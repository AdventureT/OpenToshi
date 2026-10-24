#include "ToshiPCH.h"
#include "TPostProcess_DX11.h"
#include "TRender_DX11.h"

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

#include "TPostProcessShaders_DX11.inl"

// 0x008e0dc8, triangle strip of the screen space quad
static const FLOAT s_aQuadVertices[ 8 ] = { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f };

// $deBlob: FUNCTION 006b8350
void TPostProcess::GaussianBlur5x5( TINT a_iTexWidth, TINT a_iTexHeight, TVector4* a_pOffsets, TVector4* a_pWeights, TFLOAT a_fMultiplier )
{
	const TFLOAT fTU = 1.0f / TFLOAT( a_iTexWidth );
	const TFLOAT fTV = 1.0f / TFLOAT( a_iTexHeight );

	TFLOAT fTotalWeight = 0.0f;
	TINT   iIndex       = 0;

	for ( TINT x = -2; x <= 2; x++ )
	{
		for ( TINT y = -2; y <= 2; y++ )
		{
			// Only the 13 samples within a block distance of 2
			if ( TMath::Abs( TFLOAT( y ) ) + TMath::Abs( TFLOAT( x ) ) > 2.0f )
				continue;

			a_pOffsets[ iIndex ].Set( TFLOAT( x ) * fTU, TFLOAT( y ) * fTV, 0.0f, 0.0f );

			const TFLOAT fWeight = GaussianDistribution( TFLOAT( x ), TFLOAT( y ), 1.0f );
			a_pWeights[ iIndex ].Set( fWeight, fWeight, fWeight, fWeight );
			fTotalWeight += a_pWeights[ iIndex ].x;
			iIndex++;
		}
	}

	if ( iIndex != 0 )
	{
		const TFLOAT fScale = 1.0f / fTotalWeight;

		for ( TINT i = 0; i < iIndex; i++ )
		{
			a_pWeights[ i ].Set( a_pWeights[ i ].x * fScale * a_fMultiplier, a_pWeights[ i ].y * fScale * a_fMultiplier, a_pWeights[ i ].z * fScale * a_fMultiplier, a_pWeights[ i ].w * fScale * a_fMultiplier );
		}
	}
}

// The offsets and weights of a 15 tap gaussian blur along a direction (a_fAngle) for a texture of a_uiTexWidth x
// a_uiTexHeight: the centre, 7 taps one way and the mirrored 7
// (after the DirectX SDK's GetSampleOffsets_Bloom)
// $deBlob: FUNCTION 006b8540
void TPostProcess::GetSampleOffsetsDirectionalBlur( TUINT a_uiTexWidth, TUINT a_uiTexHeight, TFLOAT a_fAngle, TVector4* a_pOffsets, TVector4* a_pWeights, TFLOAT a_fDeviation, TFLOAT a_fMultiplier )
{
	const TFLOAT fTU = TFLOAT( cos( TDOUBLE( a_fAngle ) ) ) / TFLOAT( a_uiTexWidth );
	const TFLOAT fTV = TFLOAT( sin( TDOUBLE( a_fAngle ) ) ) / TFLOAT( a_uiTexHeight );

	TFLOAT fWeight = GaussianDistribution( 0.0f, 0.0f, a_fDeviation ) * a_fMultiplier;
	a_pWeights[ 0 ].Set( fWeight, fWeight, fWeight, 1.0f );
	a_pOffsets[ 0 ].Set( 0.0f, 0.0f, 0.0f, 0.0f );

	for ( TUINT i = 1; i < 8; i++ )
	{
		const TFLOAT fX = TFLOAT( i );

		fWeight = GaussianDistribution( fX, 0.0f, a_fDeviation ) * a_fMultiplier;
		a_pWeights[ i ].Set( fWeight, fWeight, fWeight, 1.0f );
		a_pOffsets[ i ].Set( fX * fTU, fX * fTV, 0.0f, 0.0f );
	}

	for ( TINT i = 8; i < 15; i++ )
	{
		a_pWeights[ i ] = a_pWeights[ i - 7 ];
		a_pOffsets[ i ].Set( 0.0f - a_pOffsets[ i - 7 ].x, 0.0f - a_pOffsets[ i - 7 ].y, 0.0f - a_pOffsets[ i - 7 ].z, 0.0f - a_pOffsets[ i - 7 ].w );
	}
}

// The same horizontally for a texture a_uiTexSize wide, without a multiplier
// (after the DirectX SDK's GetSampleOffsets_Bloom)
// $deBlob: FUNCTION 006b8750
void TPostProcess::GetSampleOffsetsBloom( TUINT a_uiTexSize, TVector4* a_pOffsets, TVector4* a_pWeights, TFLOAT a_fDeviation )
{
	const TFLOAT fTU = 1.0f / TFLOAT( a_uiTexSize );

	TFLOAT fWeight = GaussianDistribution( 0.0f, 0.0f, a_fDeviation );
	a_pWeights[ 0 ].Set( fWeight, fWeight, fWeight, 1.0f );
	a_pOffsets[ 0 ].Set( 0.0f, 0.0f, 0.0f, 0.0f );

	for ( TUINT i = 1; i < 8; i++ )
	{
		const TFLOAT fX = TFLOAT( i );

		fWeight = GaussianDistribution( fX, 0.0f, a_fDeviation );
		a_pWeights[ i ].Set( fWeight, fWeight, fWeight, 1.0f );
		a_pOffsets[ i ].Set( fX * fTU, 0.0f, 0.0f, 0.0f );
	}

	for ( TINT i = 8; i < 15; i++ )
	{
		a_pWeights[ i ] = a_pWeights[ i - 7 ];
		a_pOffsets[ i ].Set( 0.0f - a_pOffsets[ i - 7 ].x, 0.0f - a_pOffsets[ i - 7 ].y, 0.0f - a_pOffsets[ i - 7 ].z, 0.0f - a_pOffsets[ i - 7 ].w );
	}
}

// The 16 sample offsets of a 4x4 down scale of a texture of a_uiTexWidth x a_uiTexHeight
// After the DirectX SDK's GetSampleOffsets_DownScale4x4
// $deBlob: FUNCTION 006b8900
void TPostProcess::GetSampleOffsetsDownScale4x4( TUINT a_uiTexWidth, TUINT a_uiTexHeight, TVector4* a_pOffsets )
{
	for ( TINT y = 0; y < 4; y++ )
	{
		const TFLOAT fY = ( TFLOAT( y ) - 1.5f ) * ( 1.0f / TFLOAT( a_uiTexHeight ) );

		for ( TINT x = 0; x < 4; x++ )
		{
			const TFLOAT fX = ( TFLOAT( x ) - 1.5f ) * ( 1.0f / TFLOAT( a_uiTexWidth ) );
			( a_pOffsets++ )->Set( fX, fY, 0.0f, 0.0f );
		}
	}
}

// The 9 sample offsets of a 3x3 down scale (NOTE: the outer loop is the x one)
// $deBlob: FUNCTION 006b89b0
void TPostProcess::GetSampleOffsetsDownScale3x3( TUINT a_uiTexWidth, TUINT a_uiTexHeight, TVector4* a_pOffsets )
{
	for ( TINT x = 0; x < 3; x++ )
	{
		const TFLOAT fX = ( TFLOAT( x ) - 1.0f ) * ( 1.0f / TFLOAT( a_uiTexWidth ) );

		for ( TINT y = 0; y < 3; y++ )
		{
			const TFLOAT fY = ( TFLOAT( y ) - 1.0f ) * ( 1.0f / TFLOAT( a_uiTexHeight ) );
			( a_pOffsets++ )->Set( fX, fY, 0.0f, 0.0f );
		}
	}
}

// The 4 sample offsets of a 2x2 down scale
// $deBlob: FUNCTION 006b8a50
void TPostProcess::GetSampleOffsetsDownScale2x2( TUINT a_uiTexWidth, TUINT a_uiTexHeight, TVector4* a_pOffsets )
{
	for ( TINT y = 0; y < 2; y++ )
	{
		const TFLOAT fY = ( TFLOAT( y ) - 0.5f ) * ( 1.0f / TFLOAT( a_uiTexHeight ) );

		for ( TINT x = 0; x < 2; x++ )
		{
			const TFLOAT fX = ( TFLOAT( x ) - 0.5f ) * ( 1.0f / TFLOAT( a_uiTexWidth ) );
			( a_pOffsets++ )->Set( fX, fY, 0.0f, 0.0f );
		}
	}
}

// $deBlob: FUNCTION 006b8b00
void TPostProcess::RenderImpl( ID3D11PixelShader* a_pPS, ID3D11ShaderResourceView* a_pSRV, TFLOAT a_fIntensity, ID3D11ShaderResourceView* a_pStencilSRV )
{
	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	ID3D11Resource*  pResource = TNULL;
	ID3D11Texture2D* pTexture  = TNULL;
	a_pSRV->GetResource( &pResource );
	pResource->QueryInterface( __uuidof( ID3D11Texture2D ), (void**)&pTexture );

	D3D11_TEXTURE2D_DESC oDesc;
	pTexture->GetDesc( &oDesc );
	pTexture->Release();
	pResource->Release();

	TVector4 aSampleOffsets[ MAX_SAMPLES ];
	TVector4 aSampleWeights[ MAX_SAMPLES ];
	GaussianBlur5x5( TUINT32( TFLOAT( oDesc.Width ) / a_fIntensity ), TUINT32( TFLOAT( oDesc.Height ) / a_fIntensity ), aSampleOffsets, aSampleWeights, 1.0f );

	pRender->SetVec4InPSBuffer( 29, aSampleOffsets, MAX_SAMPLES );
	pRender->SetVec4InPSBuffer( 45, aSampleWeights, MAX_SAMPLES );

	if ( a_pStencilSRV )
	{
		// NOTE: retail leaves z and w uninitialised
		TVector4 vStencilSize( TFLOAT( oDesc.Width ), TFLOAT( oDesc.Height ), 0.0f, 0.0f );
		pRender->SetVec4InPSBuffer( 61, &vStencilSize, 1 );
	}

	pDeviceContext->PSSetShader( a_pPS, TNULL, 0 );
	pDeviceContext->PSSetShaderResources( 0, 1, &a_pSRV );
	pRender->SetSamplerState( 0, 0, TRUE );

	if ( a_pStencilSRV )
	{
		pDeviceContext->PSSetShaderResources( 1, 1, &a_pStencilSRV );
		pRender->SetSamplerState( 1, 0, TRUE );
	}

	DrawScreenOverlay();

	if ( a_pStencilSRV )
	{
		ID3D11ShaderResourceView* pNullSRV = TNULL;
		pDeviceContext->PSSetShaderResources( 1, 1, &pNullSRV );
	}
}

// $deBlob: FUNCTION 006b8d60
TPostProcess::TPostProcess()
{
	m_iUnk68 = 8;
	m_iUnk6C = 0;
	m_pUnk64 = new TUINT32[ 8 ];
	m_iUnk74 = 16;
	m_iUnk78 = 0;
	m_pUnk70 = new TUINT64[ m_iUnk68 ];

	TRenderDX11* pRender = TRenderDX11::Interface();

	ID3DBlob* pShader = TRenderDX11::CompileShader( s_szScreenSpaceVS, "main", "vs_4_0_level_9_3", TNULL );
	pRender->CreateVertexShader( pShader->GetBufferPointer(), pShader->GetBufferSize(), &m_pScreenSpaceVS );

	D3D11_INPUT_ELEMENT_DESC oInputDesc;
	oInputDesc.SemanticName         = "POSITION";
	oInputDesc.SemanticIndex        = 0;
	oInputDesc.Format               = DXGI_FORMAT_R32G32_FLOAT;
	oInputDesc.InputSlot            = 0;
	oInputDesc.AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	oInputDesc.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	oInputDesc.InstanceDataStepRate = 0;
	pRender->GetDevice()->CreateInputLayout( &oInputDesc, 1, pShader->GetBufferPointer(), pShader->GetBufferSize(), &m_pQuadVtxDecl );
	pShader->Release();

	m_pQuadVB = pRender->CreateBuffer( TRenderDX11::BUFFERTYPE_VERTEX, sizeof( s_aQuadVertices ), s_aQuadVertices, D3D11_USAGE_IMMUTABLE, 0 );

	pShader = TRenderDX11::CompileShader( s_szGaussBlur5x5PS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pShader->GetBufferPointer(), pShader->GetBufferSize(), &m_pPSGaussBlur5x5 );
	pShader->Release();

	pShader = TRenderDX11::CompileShader( s_szGaussBlurMultByAlphaPS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pShader->GetBufferPointer(), pShader->GetBufferSize(), &m_pPSGaussBlurAlpha );
	pShader->Release();

	pShader = TRenderDX11::CompileShader( s_szGaussBlurMultByAlphaStencilPS, "main", "ps_4_0", TNULL );
	pRender->CreatePixelShader( pShader->GetBufferPointer(), pShader->GetBufferSize(), &m_pPSGaussBlurAlphaStencil );
	pShader->Release();
}

// $deBlob: FUNCTION 006b8f40
// $deBlob: FUNCTION 006aa8c0
TPostProcess::~TPostProcess()
{
	// NOTE: retail doesn't release the quad and the shaders
	delete[] m_pUnk64;
	delete[] m_pUnk70;
}

// $deBlob: FUNCTION 006b9190
void TPostProcess::ApplyGaussBlur( ID3D11ShaderResourceView* a_pSRV, TFLOAT a_fIntensity )
{
	RenderImpl( m_pPSGaussBlur5x5, a_pSRV, a_fIntensity, TNULL );
}

// $deBlob: FUNCTION 006b91b0
void TPostProcess::ApplyGaussBlurWithAlpha( ID3D11ShaderResourceView* a_pSRV, TFLOAT a_fIntensity, ID3D11ShaderResourceView* a_pStencilSRV )
{
	RenderImpl( a_pStencilSRV ? m_pPSGaussBlurAlphaStencil : m_pPSGaussBlurAlpha, a_pSRV, a_fIntensity, a_pStencilSRV );
}

// $deBlob: FUNCTION 006b9420
void TPostProcess::DrawScreenOverlay()
{
	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	TASSERT( m_pQuadVtxDecl );
	TASSERT( m_pQuadVB );
	TASSERT( m_pScreenSpaceVS );

	// Cull mode: none
	TUINT8& rRasterizerFlags = TREINTERPRETCAST( TUINT8*, &pRender->GetCurrentRasterizer().uiFlags )[ 0 ];
	rRasterizerFlags         = ( rRasterizerFlags & 0xF3 ) | ( D3D11_CULL_NONE << TRenderDX11::RASTERIZERFLAGS_CULLSHIFT );

	pRender->SetZMode( TFALSE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
	pDeviceContext->IASetInputLayout( m_pQuadVtxDecl );
	pDeviceContext->VSSetShader( m_pScreenSpaceVS, TNULL, 0 );
	pRender->DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pQuadVB, 4, 8, 0, 0 );
}

// $deBlob: FUNCTION 006b9540
TFLOAT TPostProcess::GaussianDistribution( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fRho )
{
	const TFLOAT fExp = TFLOAT( exp( -( ( a_fX * a_fX + a_fY * a_fY ) / ( a_fRho * 2.0f * a_fRho ) ) ) );
	return fExp / TFLOAT( sqrt( a_fRho * TMath::TWO_PI * a_fRho ) );
}

// $deBlob: FUNCTION 006b9610
void TPostProcess::Unk136( void* a_pUnk1, void* a_pUnk2 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b95d0
IUnknown* TPostProcess::Unk157( void* a_pUnk )
{
	TASSERT( TFALSE );
	return TNULL;
}

// $deBlob: FUNCTION 006b8f60
void TPostProcess::Unk452( ID3D11ShaderResourceView* pTexture, void* a_pUnk )
{
	TASSERT( pTexture );
	TRenderDX11::Interface()->CreateRenderTargetView( pTexture );

	Unk136( TNULL, TNULL );
	TASSERT( TFALSE );

	// NOTE: Unk157 returns TNULL, so this would crash
	IUnknown* pSurface = Unk157( TNULL );
	pSurface->Release();
}

// $deBlob: FUNCTION 006b93e0
void TPostProcess::Unk477( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b9220
void TPostProcess::Unk551( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b9320
void TPostProcess::Unk604( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b92e0
void TPostProcess::Unk673( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b9150
void TPostProcess::Unk720( void* a_pUnk1, void* a_pUnk2 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b91e0
void TPostProcess::Unk830( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b9110
void TPostProcess::Unk867( void* a_pUnk1, void* a_pUnk2 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b9050
void TPostProcess::Unk912( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b9090
void TPostProcess::Unk954( void* a_pUnk )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b90d0
void TPostProcess::Unk1010( void* a_pUnk1, void* a_pUnk2 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b9360
void TPostProcess::Unk1064( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4, void* a_pUnk5 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b93a0
void TPostProcess::Unk1118( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4, void* a_pUnk5, void* a_pUnk6, void* a_pUnk7, void* a_pUnk8, void* a_pUnk9 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b92a0
void TPostProcess::Unk1179( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b9260
void TPostProcess::Unk1226( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3 )
{
	TASSERT( TFALSE );
}

TOSHI_NAMESPACE_END

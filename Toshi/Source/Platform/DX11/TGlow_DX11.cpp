#include "ToshiPCH.h"
#include "TGlow_DX11.h"
#include "TPostProcess_DX11.h"
#include "TRender_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

#include "TGlowShader_DX11.inl"

// $deBlob: FUNCTION 006b45f0
TGlow::TGlow()
{
	m_pTexture      = TNULL;
	m_fIntensity    = 0.0f;
	m_fDist         = 2.0f;
	m_pGlowSRV      = TNULL;
	m_pTexture2     = TNULL;
	m_pGlowSRV2     = TNULL;
	m_pRenderTarget = TNULL;
	m_pPixelShader  = TNULL;

	TRenderDX11* pRender = TRenderDX11::Interface();
	m_uiWidth            = pRender->GetSwapChainDesc()->BufferDesc.Width / 2;
	m_uiHeight           = pRender->GetSwapChainDesc()->BufferDesc.Height / 2;

	m_pGlowSRV = pRender->CreateTexture( m_uiWidth, m_uiHeight, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, 0, D3D11_USAGE_DEFAULT, 0, 1 );
	TASSERT( m_pGlowSRV );

	if ( m_pGlowSRV )
	{
		ID3D11Resource* pResource = TNULL;
		m_pGlowSRV->GetResource( &pResource );
		pResource->QueryInterface( __uuidof( ID3D11Texture2D ), (void**)&m_pTexture );

		if ( pResource )
			pResource->Release();
	}

	m_pGlowSRV2     = pRender->CreateTexture( m_uiWidth, m_uiHeight, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, TRenderDX11::TEXTUREFLAGS_RENDERTARGET, D3D11_USAGE_DEFAULT, 0, 1 );
	m_pRenderTarget = pRender->CreateRenderTargetView( m_pGlowSRV2 );
	TASSERT( m_pRenderTarget );

	if ( m_pGlowSRV2 )
	{
		ID3D11Resource* pResource = TNULL;
		m_pGlowSRV2->GetResource( &pResource );
		pResource->QueryInterface( __uuidof( ID3D11Texture2D ), (void**)&m_pTexture2 );

		if ( pResource )
			pResource->Release();
	}

	// NOTE: retail doesn't release the blob
	ID3DBlob* pShader = TRenderDX11::CompileShader( s_szGlowPS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pShader->GetBufferPointer(), pShader->GetBufferSize(), &m_pPixelShader );
}

// $deBlob: FUNCTION 006b47c0
// $deBlob: FUNCTION 006aa860
TGlow::~TGlow()
{
	if ( m_pTexture )
	{
		m_pTexture->Release();
		m_pTexture = TNULL;
	}

	if ( m_pGlowSRV )
	{
		m_pGlowSRV->Release();
		m_pGlowSRV = TNULL;
	}

	if ( m_pTexture2 )
	{
		m_pTexture2->Release();
		m_pTexture2 = TNULL;
	}

	if ( m_pGlowSRV2 )
	{
		m_pGlowSRV2->Release();
		m_pGlowSRV2 = TNULL;
	}

	if ( m_pRenderTarget )
	{
		m_pRenderTarget->Release();
		m_pRenderTarget = TNULL;
	}

	if ( m_pPixelShader )
	{
		m_pPixelShader->Release();
		m_pPixelShader = TNULL;
	}
}

// $deBlob: FUNCTION 006b4840
void TGlow::Render( ID3D11ShaderResourceView* a_pSRV, ID3D11ShaderResourceView* a_pStencilSRV )
{
	if ( m_fIntensity <= 0.0f )
		return;

	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	ID3D11RenderTargetView* pOldRenderTarget;
	ID3D11DepthStencilView* pOldDepthStencil;
	pDeviceContext->OMGetRenderTargets( 1, &pOldRenderTarget, &pOldDepthStencil );

	UINT           uiNumViewports = 1;
	D3D11_VIEWPORT oOldViewport;
	pDeviceContext->RSGetViewports( &uiNumViewports, &oOldViewport );

	const FLOAT aClearColour[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pDeviceContext->ClearRenderTargetView( m_pRenderTarget, aClearColour );
	pDeviceContext->OMSetRenderTargets( 1, &m_pRenderTarget, TNULL );

	D3D11_VIEWPORT oViewport;
	oViewport.TopLeftX = 0.0f;
	oViewport.TopLeftY = 0.0f;
	oViewport.Width    = FLOAT( TINT( m_uiWidth ) );
	oViewport.Height   = FLOAT( TINT( m_uiHeight ) );
	oViewport.MinDepth = 0.0f;
	oViewport.MaxDepth = 1.0f;
	pDeviceContext->RSSetViewports( 1, &oViewport );

	pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE );

	TPostProcess* pPostProcess = pRender->GetPostProcess();
	pPostProcess->ApplyGaussBlurWithAlpha( a_pSRV, m_fDist * 4.1f, a_pStencilSRV );
	pDeviceContext->CopyResource( m_pTexture, m_pTexture2 );

	pPostProcess->ApplyGaussBlur( m_pGlowSRV, TMath::LERP( m_fDist, 1.0f, 0.5f ) * 2.7f );
	pDeviceContext->CopyResource( m_pTexture, m_pTexture2 );

	pPostProcess->ApplyGaussBlur( m_pGlowSRV, TMath::LERP( m_fDist, 1.0f, 0.25f ) * 1.3f );
	pDeviceContext->CopyResource( m_pTexture, m_pTexture2 );

	pDeviceContext->OMSetRenderTargets( 1, &pOldRenderTarget, pOldDepthStencil );

	if ( pOldRenderTarget )
	{
		pOldRenderTarget->Release();
		pOldRenderTarget = TNULL;
	}

	if ( pOldDepthStencil )
	{
		pOldDepthStencil->Release();
		pOldDepthStencil = TNULL;
	}

	pDeviceContext->RSSetViewports( 1, &oOldViewport );

	const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc = pRender->GetSwapChainDesc();

	TVector4 vUVST;
	vUVST.x = oOldViewport.Width / TFLOAT( pSwapChainDesc->BufferDesc.Width );
	vUVST.y = oOldViewport.Height / TFLOAT( pSwapChainDesc->BufferDesc.Height );
	vUVST.z = oOldViewport.TopLeftX / TFLOAT( pSwapChainDesc->BufferDesc.Width );
	vUVST.w = oOldViewport.TopLeftY / TFLOAT( pSwapChainDesc->BufferDesc.Height );
	TRenderDX11::RenderOverlay( 0.0f, 0.0f, oOldViewport.Width, oOldViewport.Height, a_pSRV, TNULL, &vUVST );

	pRender->SetDstAlpha( 0.0f );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE );

	TVector4 vGlowMag( m_fIntensity, m_fIntensity, m_fIntensity, m_fIntensity );
	pRender->SetVec4InPSBuffer( 28, &vGlowMag, 1 );

	vUVST.x = oOldViewport.Width / TFLOAT( pSwapChainDesc->BufferDesc.Width );
	vUVST.y = oOldViewport.Height / TFLOAT( pSwapChainDesc->BufferDesc.Height );
	vUVST.z = oOldViewport.TopLeftX / TFLOAT( pSwapChainDesc->BufferDesc.Width );
	vUVST.w = oOldViewport.TopLeftY / TFLOAT( pSwapChainDesc->BufferDesc.Height );
	TRenderDX11::RenderOverlay( 0.0f, 0.0f, oOldViewport.Width, oOldViewport.Height, m_pGlowSRV, m_pPixelShader, &vUVST );
}

// $deBlob: FUNCTION 006b4cc0
void TGlow::SetIntensity( TFLOAT a_fIntensity )
{
	TASSERT( a_fIntensity >= 0.0f );
	m_fIntensity = a_fIntensity;
}

// $deBlob: FUNCTION 006b4c60
void TGlow::SetDist( TFLOAT a_fDist )
{
	TASSERT( a_fDist >= 0.0f );
	m_fDist = a_fDist;
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TFXAA_DX11.h"
#include "TRender_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

#include "TFXAAShader_DX11.inl"

// $deBlob: FUNCTION 006b4d30
TFXAA::TFXAA()
{
	m_pTexture      = TNULL;
	m_pTextureView  = TNULL;
	m_pRenderTarget = TNULL;
	m_uiWidth       = 1280;
	m_uiHeight      = 720;
	m_pShader       = TNULL;

	D3D_SHADER_MACRO aMacros[ 5 ];
	aMacros[ 0 ].Name       = "FXAA_PC";
	aMacros[ 0 ].Definition = "1";
	aMacros[ 1 ].Name       = "FXAA_HLSL_4";
	aMacros[ 1 ].Definition = "1";
	aMacros[ 2 ].Name       = "FXAA_QUALITY__PRESET";
	aMacros[ 2 ].Definition = "12";
	aMacros[ 3 ].Name       = "FXAA_GREEN_AS_LUMA";
	aMacros[ 3 ].Definition = "1";
	aMacros[ 4 ].Name       = TNULL;
	aMacros[ 4 ].Definition = TNULL;

	// NOTE: retail compiles one string literal; it's stored in parts here (see TFXAAShader_DX11.inl)
	TCHAR* pszSource = new TCHAR[ s_uiFXAAShaderSize + 1 ];
	TSIZE  uiOffset  = 0;

	for ( TSIZE i = 0; i < TARRAYSIZE( s_apFXAAShaderParts ); i++ )
	{
		TSIZE uiLength = TStringManager::String8Length( s_apFXAAShaderParts[ i ] );
		TUtil::MemCopy( pszSource + uiOffset, s_apFXAAShaderParts[ i ], uiLength );
		uiOffset += uiLength;
	}

	TASSERT( uiOffset == s_uiFXAAShaderSize );
	pszSource[ uiOffset ] = '\0';

	// NOTE: retail doesn't release the blob
	ID3DBlob* pShader = TRenderDX11::CompileShader( pszSource, "main", "ps_4_0", aMacros );
	delete[] pszSource;

	TRenderDX11::Interface()->CreatePixelShader( pShader->GetBufferPointer(), pShader->GetBufferSize(), &m_pShader );
	Initialise();
}

// $deBlob: FUNCTION 006b4e20
// $deBlob: FUNCTION 006aa800
TFXAA::~TFXAA()
{
	if ( m_pShader )
	{
		m_pShader->Release();
		m_pShader = TNULL;
	}

	if ( m_pRenderTarget )
	{
		m_pRenderTarget->Release();
		m_pRenderTarget = TNULL;
	}

	if ( m_pTextureView )
	{
		m_pTextureView->Release();
		m_pTextureView = TNULL;
	}

	if ( m_pTexture )
	{
		m_pTexture->Release();
		m_pTexture = TNULL;
	}
}

// $deBlob: FUNCTION 006b4e80
void TFXAA::Render( ID3D11ShaderResourceView* a_pShaderResourceView )
{
	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	ID3D11RenderTargetView* pOldRenderTarget;
	ID3D11DepthStencilView* pOldDepthStencil;
	pDeviceContext->OMGetRenderTargets( 1, &pOldRenderTarget, &pOldDepthStencil );

	pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
	pDeviceContext->OMSetRenderTargets( 1, &m_pRenderTarget, TNULL );

	TVector4 vScreenSize( TFLOAT( m_uiWidth ), TFLOAT( m_uiHeight ), 0.0f, 0.0f );
	pRender->SetVec4InPSBuffer( 26, &vScreenSize, 1 );
	TRenderDX11::RenderOverlay( 0.0f, 0.0f, TFLOAT( m_uiWidth ), TFLOAT( m_uiHeight ), a_pShaderResourceView, m_pShader, TNULL );

	// NOTE: retail never releases the views OMGetRenderTargets returned
	pDeviceContext->OMSetRenderTargets( 1, &pOldRenderTarget, pOldDepthStencil );
	TRenderDX11::RenderOverlay( 0.0f, 0.0f, TFLOAT( m_uiWidth ), TFLOAT( m_uiHeight ), m_pTextureView, TNULL, TNULL );
}

// $deBlob: FUNCTION 006b4fb0
void TFXAA::Initialise()
{
	TRenderDX11* pRender = TRenderDX11::Interface();
	m_uiWidth            = pRender->GetSwapChainDesc()->BufferDesc.Width;
	m_uiHeight           = pRender->GetSwapChainDesc()->BufferDesc.Height;

	m_pTextureView  = pRender->CreateTexture( m_uiWidth, m_uiHeight, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, TRenderDX11::TEXTUREFLAGS_RENDERTARGET, D3D11_USAGE_DEFAULT, 0, 1 );
	m_pRenderTarget = pRender->CreateRenderTargetView( m_pTextureView );
	TASSERT( m_pRenderTarget );

	if ( m_pTextureView )
	{
		ID3D11Resource* pResource = TNULL;
		m_pTextureView->GetResource( &pResource );
		pResource->QueryInterface( __uuidof( ID3D11Texture2D ), (void**)&m_pTexture );

		if ( pResource )
			pResource->Release();
	}

	m_pTextureView->SetPrivateData( WKPDID_D3DDebugObjectName, 8, "FXAA SRV" );
	m_pRenderTarget->SetPrivateData( WKPDID_D3DDebugObjectName, 7, "FXAA RT" );
}

TOSHI_NAMESPACE_END

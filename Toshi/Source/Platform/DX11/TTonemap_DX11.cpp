#include "ToshiPCH.h"
#include "TTonemap_DX11.h"
#include "TRender_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006ba990
TToneMap::TToneMap()
    : m_pTextureView( TNULL )
    , m_pRenderTarget( TNULL )
    , m_uiUnk08( 0 )
    , m_uiUnk0C( 0 )
    , m_fBlend( 0.0f )
    , m_pUnk14( TNULL )
    , m_pUnk18( TNULL )
{
	m_pTextureView = TRenderDX11::Interface()->CreateTexture( 128, 128, DXGI_FORMAT_R8G8B8A8_UNORM, TNULL, TRenderDX11::TEXTUREFLAGS_RENDERTARGET, D3D11_USAGE_DEFAULT, 0, 1 );

	if ( m_pTextureView )
		m_pRenderTarget = TRenderDX11::Interface()->CreateRenderTargetView( m_pTextureView );

	TASSERT( m_pRenderTarget );
}

// $deBlob: FUNCTION 006baa30
// $deBlob: FUNCTION 006aa980
TToneMap::~TToneMap()
{
	if ( m_pTextureView )
	{
		m_pTextureView->Release();
		m_pTextureView = TNULL;
	}

	if ( m_pRenderTarget )
	{
		m_pRenderTarget->Release();
		m_pRenderTarget = TNULL;
	}

	if ( m_pUnk14 )
	{
		m_pUnk14->Release();
		m_pUnk14 = TNULL;
	}

	if ( m_pUnk18 )
	{
		m_pUnk18->Release();
		m_pUnk18 = TNULL;
	}
}

// $deBlob: FUNCTION 006bab30
void TToneMap::Render( ID3D11ShaderResourceView* a_pSRV )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006baa90
void TToneMap::SetBlend( TFLOAT a_fBlend )
{
	TASSERT( a_fBlend >= 0.0f );
	TASSERT( a_fBlend <= 1.0f );
	m_fBlend = a_fBlend;
}

TOSHI_NAMESPACE_END

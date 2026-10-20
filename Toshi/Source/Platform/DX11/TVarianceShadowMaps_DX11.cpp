#include "ToshiPCH.h"
#include "TVarianceShadowMaps_DX11.h"
#include "Render/TRenderParamTable.h"
#include "TTexture_DX11.h"
#include "TRender_DX11.h"

#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL TVarianceShadowMaps::s_bUnk582 = TTRUE;
TBOOL TVarianceShadowMaps::s_bUnk800 = TTRUE;

// $deBlob: FUNCTION 006b7140
TVarianceShadowMaps::TVarianceShadowMaps()
{
	m_vPosition.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	m_pTexture       = TNULL;
	m_uiUnkF8        = 0;
	m_uiUnk10C       = 0;
	m_iNumFrustums   = 1;
	m_iShadowMapSize = 1024;
	m_iActiveFrustum = 0;
	m_uiUnkD8        = 0;
	m_fUnkDC         = 1.0f;
	m_fUnkE4         = 0.04f;
	m_fUnkE0         = 0.04f;

	SetOrthographicProjection( 10.0f, 10.0f, 0.0f, 1.0f );
	Initialise();
}

// $deBlob: FUNCTION 006b7200
TVarianceShadowMaps::~TVarianceShadowMaps()
{
	if ( m_pTexture )
	{
		TTextureManager::GetSingleton()->DestroyTexture( m_pTexture );
		m_pTexture = TNULL;
	}

	delete[] m_pUnkF4;
	delete[] m_pUnkEC;
	delete[] m_pUnkF0;
}

// $deBlob: FUNCTION 006b7640
void TVarianceShadowMaps::SetPosition( const TVector3& a_rcPosition )
{
	m_vPosition.Set( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
}

// $deBlob: FUNCTION 006b76b0
void TVarianceShadowMaps::SetOrthographicProjection( TFLOAT a_fWidth, TFLOAT a_fHeight, TFLOAT a_fNear, TFLOAT a_fFar )
{
	const TFLOAT fDepthScale = 1.0f / ( a_fFar - a_fNear );

	m_oProjection.Set(
	    2.0f / a_fWidth, 0.0f, 0.0f, 0.0f,
	    0.0f, 2.0f / a_fHeight, 0.0f, 0.0f,
	    0.0f, 0.0f, fDepthScale, 0.0f,
	    0.0f, 0.0f, -( fDepthScale * a_fNear ), 1.0f
	);
}

// $deBlob: FUNCTION 006b7760
void TVarianceShadowMaps::SetActiveFrustum( TINT a_iActiveFrustum )
{
	TASSERT( a_iActiveFrustum >= 0 );
	TASSERT( a_iActiveFrustum < m_iNumFrustums );
	m_iActiveFrustum = a_iActiveFrustum;
}

// $deBlob: FUNCTION 006b77e0
TINT TVarianceShadowMaps::Initialise()
{
	m_uiUnkFC  = 0;
	m_iUnk100  = 1;
	m_fUnk104  = 0.001f;
	m_iUnk108  = 1;
	m_pUnkEC   = TNULL;
	m_pUnkF0   = TNULL;
	m_pUnkF4   = TNULL;
	m_uiUnk134 = 0;

	m_pUnkF0 = new TUINT32[ m_iNumFrustums ];
	m_pUnkEC = new TUINT32[ m_iNumFrustums ];
	m_pUnkF4 = new TUINT32[ m_iNumFrustums ];

	for ( TINT i = 0; i < m_iNumFrustums; i++ )
	{
		m_pUnkF0[ i ] = 0;
		m_pUnkEC[ i ] = 0;
		m_pUnkF4[ i ] = 0;
	}

	m_vUnk80.Set( 0.0f, 0.28024039f, -0.55869102f, 1.0f );
	m_vUnk90.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	m_vUnkA0.Set( 0.0f, 1.0f, 0.0f, 0.0f );
	m_vUnkC0.Set( 0.6041047f, 0.53163457f, -0.58546913f, 0.0f );

	return 0;
}

// $deBlob: FUNCTION 006b7920
void TVarianceShadowMaps::SetShaderConstants( const TMatrix44& a_rcMatrix1, const TMatrix44& a_rcMatrix2 )
{
	DirectX::XMMATRIX mMatrix1 = DirectX::XMMatrixTranspose( DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &a_rcMatrix1 ) ) );
	DirectX::XMMATRIX mMatrix2 = DirectX::XMMatrixTranspose( DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &a_rcMatrix2 ) ) );

	// NOTE: retail transforms both vectors by the inverse of the identity matrix
	DirectX::XMVECTOR vDeterminant;
	DirectX::XMMATRIX mInverse = DirectX::XMMatrixInverse( &vDeterminant, DirectX::XMMatrixIdentity() );

	DirectX::XMVECTOR vUnkC0 = DirectX::XMVector4Normalize( DirectX::XMVector4Transform( DirectX::XMLoadFloat4( TREINTERPRETCAST( const DirectX::XMFLOAT4*, &m_vUnkC0 ) ), mInverse ) );
	DirectX::XMVECTOR vUnk80 = DirectX::XMVector4Transform( DirectX::XMLoadFloat4( TREINTERPRETCAST( const DirectX::XMFLOAT4*, &m_vUnk80 ) ), mInverse );

	TRenderDX11* pRender = TRenderDX11::Interface();
	pRender->SetVec4InVSBuffer( 0, &mMatrix1, 4 );
	pRender->SetVec4InVSBuffer( 4, &mMatrix2, 4 );
	pRender->SetVec4InVSBuffer( 8, &vUnkC0, 1 );
	pRender->SetVec4InVSBuffer( 9, &vUnk80, 1 );
}

// Only read by UpdateUnkC0 (initialised data)
static TFLOAT s_fUnk9820B4 = 5.0f;
static TFLOAT s_fUnk9820B8 = 0.1f;
static TFLOAT s_fUnk9820BC = 10.0f;

// $deBlob: FUNCTION 006b7250
TINT TVarianceShadowMaps::UpdateUnkC0()
{
	// NOTE: the function statics are written once and never read
	static TFLOAT s_fUnk9A8EFC = -0.47123894f;
	static TFLOAT s_fUnk9A8F04 = 0.0f;

	const TVector4& rcParam = TRender::GetSingleton()->GetParamTable()->GetParameterV4( 2 );
	m_vUnkC0.Set( rcParam.x, rcParam.y, rcParam.z, 0.0f );

	static TFLOAT s_fUnk9A8F08 = s_fUnk9820BC * s_fUnk9820B4 + s_fUnk9820B8;

	TRender::GetSingleton()->GetCurrentContext();

	static TVector4  s_vUnk9A8F10( 0.0f, 1.0f, 0.0001f, 0.0f );
	static TMatrix44 s_mUnk9A8F20(
	    1.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 1.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, -1.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 1.0f
	);

	(void)s_fUnk9A8EFC;
	(void)s_fUnk9A8F04;
	(void)s_fUnk9A8F08;
	(void)s_vUnk9A8F10;
	(void)s_mUnk9A8F20;

	return 0;
}

// $deBlob: FUNCTION 006b7670
void TVarianceShadowMaps::Unk409( void* a_pUnk )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b73c0
void TVarianceShadowMaps::Unk492()
{
	// NOTE: the views OMGetRenderTargets returns are never released (DX9 leftover)
	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11DepthStencilView* pDepthStencilView;
	TRenderDX11::Interface()->GetDeviceContext()->OMGetRenderTargets( 1, &pRenderTargetView, &pDepthStencilView );

	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b74a0
void TVarianceShadowMaps::Unk520()
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b7510
void TVarianceShadowMaps::Unk557()
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b74d0
void TVarianceShadowMaps::Unk582( void* a_pUnk )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b75e0
void TVarianceShadowMaps::Unk608()
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b7610
void TVarianceShadowMaps::Unk641()
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b7540
void TVarianceShadowMaps::Unk653()
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b75b0
void TVarianceShadowMaps::Unk787()
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b7570
void TVarianceShadowMaps::Unk800( void* a_pUnk )
{
	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 006b7420
TINT TVarianceShadowMaps::Unk582And800( void* a_pUnk )
{
	if ( s_bUnk582 )
		Unk582( a_pUnk );

	if ( s_bUnk800 )
		Unk800( a_pUnk );

	return 0;
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TRenderContext_DX11.h"
#include "Render/TRender.h"
#include "Render/TRenderParamTable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006b9680
TRenderContextDX11::TRenderContextDX11( TRender* a_pRender )
    : TRenderContext( a_pRender )
{
}

// $deBlob: FUNCTION 006ba450
// $deBlob: FUNCTION 006b96f0
TRenderContextDX11::~TRenderContextDX11()
{
}

// $deBlob: FUNCTION 006b96a0
void TRenderContextDX11::Update()
{
	if ( IsDirty() )
	{
		if ( m_eCameraMode == CameraMode_Perspective )
		{
			ComputePerspectiveProjection();
			ComputePerspectiveFrustum();
		}
		else
		{
			ComputeOrthographicProjection();
			ComputeOrthographicFrustum();
		}

		TRender::GetSingleton()->GetParamTable()->SetParameterM44( TRenderParamTable::M44PARAM_PROJ, m_oProjection );
	}
}

// $deBlob: FUNCTION 006b9700
void TRenderContextDX11::ComputePerspectiveProjection()
{
	m_oProjection.m_f11 = ( m_oProjParams.m_Proj.x * 2.0f ) / m_oViewportParams.fWidth;
	m_oProjection.m_f12 = 0.0f;
	m_oProjection.m_f13 = 0.0f;
	m_oProjection.m_f14 = 0.0f;
	m_oProjection.m_f21 = 0.0f;
	m_oProjection.m_f22 = -( ( m_oProjParams.m_Proj.y * 2.0f ) / m_oViewportParams.fHeight );
	m_oProjection.m_f23 = 0.0f;
	m_oProjection.m_f24 = 0.0f;
	m_oProjection.m_f31 = ( m_oProjParams.m_Centre.x * 2.0f ) / m_oViewportParams.fWidth - 1.0f;
	m_oProjection.m_f32 = -( ( m_oProjParams.m_Centre.y * 2.0f ) / m_oViewportParams.fHeight - 1.0f );
	m_oProjection.m_f33 = m_oProjParams.m_fFarClip / ( m_oProjParams.m_fFarClip - m_oProjParams.m_fNearClip );
	m_oProjection.m_f34 = 1.0f;
	m_oProjection.m_f41 = 0.0f;
	m_oProjection.m_f42 = 0.0f;
	m_oProjection.m_f43 = -( ( m_oProjParams.m_fNearClip * m_oProjParams.m_fFarClip ) / ( m_oProjParams.m_fFarClip - m_oProjParams.m_fNearClip ) );
	m_oProjection.m_f44 = 0.0f;
}

// $deBlob: FUNCTION 006b9820
void TRenderContextDX11::ComputePerspectiveFrustum()
{
	TRenderContext::ComputePerspectiveFrustum( m_aFrustumPlanes1, m_oViewportParams, m_oProjParams );

	for ( TINT i = 0; i < WORLDPLANE_NUMOF; i++ )
		m_aGuardPlanes[ i ] = m_aFrustumPlanes1[ i ];
}

// $deBlob: FUNCTION 006b9c30
void TRenderContextDX11::ComputeOrthographicProjection()
{
	TASSERT( m_oProjParams.m_Proj.x != 0.0f );
	TASSERT( m_oProjParams.m_Proj.y != 0.0f );
	TASSERT( m_oViewportParams.fWidth != 0.0f );
	TASSERT( m_oViewportParams.fHeight != 0.0f );

	TASSERT( TMath::IsFinite( m_oProjParams.m_Proj.x ) && ( !TMath::IsNaN( m_oProjParams.m_Proj.x ) ) );
	TASSERT( TMath::IsFinite( m_oProjParams.m_Proj.y ) && ( !TMath::IsNaN( m_oProjParams.m_Proj.y ) ) );
	TASSERT( TMath::IsFinite( m_oProjParams.m_Centre.x ) && ( !TMath::IsNaN( m_oProjParams.m_Centre.x ) ) );
	TASSERT( TMath::IsFinite( m_oProjParams.m_Centre.y ) && ( !TMath::IsNaN( m_oProjParams.m_Centre.y ) ) );

	m_oProjection.m_f11 = ( m_oProjParams.m_Proj.x * 2.0f ) / m_oViewportParams.fWidth;
	m_oProjection.m_f12 = 0.0f;
	m_oProjection.m_f13 = 0.0f;
	m_oProjection.m_f14 = 0.0f;
	m_oProjection.m_f21 = 0.0f;
	m_oProjection.m_f22 = -( ( m_oProjParams.m_Proj.y * 2.0f ) / m_oViewportParams.fHeight );
	m_oProjection.m_f23 = 0.0f;
	m_oProjection.m_f24 = 0.0f;
	m_oProjection.m_f31 = 0.0f;
	m_oProjection.m_f32 = 0.0f;
	m_oProjection.m_f33 = 1.0f / ( m_oProjParams.m_fFarClip - m_oProjParams.m_fNearClip );
	m_oProjection.m_f34 = 0.0f;
	m_oProjection.m_f41 = ( m_oProjParams.m_Centre.x * 2.0f ) / m_oViewportParams.fWidth - 1.0f;
	m_oProjection.m_f42 = -( ( m_oProjParams.m_Centre.y * 2.0f ) / m_oViewportParams.fHeight - 1.0f );
	m_oProjection.m_f43 = -( m_oProjParams.m_fNearClip / ( m_oProjParams.m_fFarClip - m_oProjParams.m_fNearClip ) );
	m_oProjection.m_f44 = 1.0f;
}

// $deBlob: FUNCTION 006b9ff0
void TRenderContextDX11::ComputeOrthographicFrustum()
{
	TRenderContext::ComputeOrthographicFrustum( m_aFrustumPlanes1, m_oViewportParams, m_oProjParams );

	for ( TINT i = 0; i < WORLDPLANE_NUMOF; i++ )
		m_aGuardPlanes[ i ] = m_aFrustumPlanes1[ i ];
}

TOSHI_NAMESPACE_END

#include "pch.h"
#include "AXYZViewportManager.h"
#include "Memory/AMemory.h"
#include "Cameras/ACamera.h"
#include "Input/AInputManager2.h"

#include <Math/TQuaternion.h>
#include <Render/TCameraObject.h>
#include <Render/TRenderContext.h>
#include <Render/TViewport.h>

#include <Render/TRender.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005f1030
AXYZViewportManager::AXYZViewportManager()
{
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

	TFLOAT fWidth      = TFLOAT( TINT( pDisplayParams->uiWidth ) );
	TFLOAT fHeight     = TFLOAT( TINT( pDisplayParams->uiHeight ) );
	TFLOAT fHalfWidth  = fWidth * 0.5f;
	TFLOAT fHalfHeight = fHeight * 0.5f;
	TFLOAT fThirdWidth = fWidth / 3.0f;
	TFLOAT fHalfX      = fWidth * 0.5f + 0.0f;
	TFLOAT fHalfY      = fHeight * 0.5f + 0.0f;
	TFLOAT fThirdX     = fWidth / 3.0f + 0.0f;
	TFLOAT fTwoThirdsX = fThirdX + fWidth / 3.0f;

	m_apViewports[ VIEWPORT_FullScreen ]  = CreateViewport( 0.0f, 0.0f, fWidth, fHeight );
	m_apViewports[ VIEWPORT_TopLeft ]     = CreateViewport( 0.0f, 0.0f, fHalfWidth, fHalfHeight );
	m_apViewports[ VIEWPORT_TopRight ]    = CreateViewport( fHalfX, 0.0f, fHalfWidth, fHalfHeight );
	m_apViewports[ VIEWPORT_BottomLeft ]  = CreateViewport( 0.0f, fHalfY, fHalfWidth, fHalfHeight );
	m_apViewports[ VIEWPORT_BottomRight ] = CreateViewport( fHalfX, fHalfY, fHalfWidth, fHalfHeight );
	m_apViewports[ VIEWPORT_Top ]         = CreateViewport( 0.0f, 0.0f, fWidth, fHalfHeight );
	m_apViewports[ VIEWPORT_Bottom ]      = CreateViewport( 0.0f, fHalfY, fWidth, fHalfHeight );
	m_apViewports[ VIEWPORT_Left ]        = CreateViewport( 0.0f, 0.0f, fHalfWidth, fHeight );
	m_apViewports[ VIEWPORT_Right ]       = CreateViewport( fHalfX, 0.0f, fHalfWidth, fHeight );
	m_apViewports[ VIEWPORT_ThirdLeft ]   = CreateViewport( 0.0f, 0.0f, fThirdWidth, fHeight );
	m_apViewports[ VIEWPORT_ThirdMiddle ] = CreateViewport( fThirdX, 0.0f, fThirdWidth, fHeight );
	m_apViewports[ VIEWPORT_ThirdRight ]  = CreateViewport( fTwoThirdsX, 0.0f, fThirdWidth, fHeight );

	TMatrix44 mCamera = TMatrix44::IDENTITY;
	TVector4  vTarget( 0.0f, 0.0f, 1.0f, 1.0f );
	mCamera.LookAtTarget( TVector4::VEC_ZERO, vTarget );
	mCamera.m_f41 = 0.0f;
	mCamera.m_f42 = 0.0f;
	mCamera.m_f43 = 25.0f;

	for ( TINT i = 0; i < VIEWPORT_Count; i++ )
	{
		m_apCameras[ i ] = TNULL;
		m_abUsed[ i ]    = TFALSE;

		m_apCameraObjects[ i ] = new TCameraObject();
		m_apCameraObjects[ i ]->SetProjectionCentreX( 0.5f );
		m_apCameraObjects[ i ]->SetProjectionCentreY( 0.5f );
		m_apCameraObjects[ i ]->SetFOV( 1.0471976f );
		m_apCameraObjects[ i ]->GetTransformObject().SetMatrix( TMatrix44::IDENTITY );
		m_apCameraObjects[ i ]->SetFar( 200.0f );
		m_apCameraObjects[ i ]->SetNear( 1.0f );
		m_apCameraObjects[ i ]->GetTransformObject().SetMatrix( mCamera );
	}

	m_uiCurrentViewport             = 0;
	m_eRenderingViewport            = VIEWPORT_NONE;
	m_abUsed[ VIEWPORT_FullScreen ] = TTRUE;
}

// $deBlob: FUNCTION 005f14b0
// $deBlob: FUNCTION 005f0170
AXYZViewportManager::~AXYZViewportManager()
{
	for ( TINT i = 0; i < VIEWPORT_Count; i++ )
	{
		if ( m_apViewports[ i ] != TNULL )
			delete m_apViewports[ i ];
	}

	TINT            i              = VIEWPORT_Count;
	TCameraObject** ppCameraObject = m_apCameraObjects;
	while ( i != 0 )
	{
		TCameraObject* pCameraObject = *ppCameraObject;
		if ( pCameraObject != TNULL )
			delete pCameraObject;

		ppCameraObject++;
		i--;
	}
}

// $deBlob: FUNCTION 005f1500
void AXYZViewportManager::DisableAllViewports()
{
	for ( TINT i = VIEWPORT_Count; i != 0; )
		m_abUsed[ --i ] = TFALSE;
}

// Forgets the cameras of all the viewports
// $deBlob: FUNCTION 005f1520
void AXYZViewportManager::ClearCameras()
{
	for ( TINT i = VIEWPORT_Count; i != 0; )
		m_apCameras[ --i ] = TNULL;
}

// $deBlob: FUNCTION 005f1540
void AXYZViewportManager::RemoveCamera( ACamera* a_pCamera )
{
	for ( TINT i = VIEWPORT_Count; i != 0; )
	{
		i--;
		if ( m_apCameras[ i ] == a_pCamera )
			m_apCameras[ i ] = TNULL;
	}
}

// $deBlob: FUNCTION 005f1570
void AXYZViewportManager::SetCamera( VIEWPORT a_eViewport, ACamera* a_pCamera )
{
	TASSERT( a_eViewport >= VIEWPORT_FullScreen && a_eViewport < VIEWPORT_Count );
	m_apCameras[ a_eViewport ] = a_pCamera;
}

// $deBlob: FUNCTION 005f15d0
void AXYZViewportManager::SetViewportUsed( VIEWPORT a_eViewport, TBOOL a_bUsed )
{
	TASSERT( a_eViewport >= VIEWPORT_FullScreen && a_eViewport < VIEWPORT_Count );
	m_abUsed[ a_eViewport ] = a_bUsed;
}

// $deBlob: FUNCTION 005f1640
void AXYZViewportManager::UseViewport( VIEWPORT a_eViewport, TBOOL a_bExclusive )
{
	if ( a_bExclusive )
	{
		for ( TINT i = VIEWPORT_Count; i != 0; )
			m_abUsed[ --i ] = TFALSE;
	}

	TASSERT( a_eViewport >= VIEWPORT_FullScreen && a_eViewport < VIEWPORT_Count );
	m_abUsed[ a_eViewport ] = TTRUE;
}

// $deBlob: FUNCTION 005f16c0
void AXYZViewportManager::EnableViewport( VIEWPORT a_eViewport, ACamera* a_pCamera )
{
	TASSERT( a_eViewport >= VIEWPORT_FullScreen && a_eViewport < VIEWPORT_Count );
	m_abUsed[ a_eViewport ] = TTRUE;
	SetCamera( a_eViewport, a_pCamera );
}

// $deBlob: FUNCTION 005f1750
TBOOL AXYZViewportManager::IsSphereVisible( const TSphere& a_rcSphere ) const
{
	for ( TINT i = VIEWPORT_Count; i != 0; )
	{
		i--;
		if ( m_abUsed[ i ] && m_apCameras[ i ] && TRenderContext::CullSphereToFrustumSimple( a_rcSphere, m_apCameras[ i ]->GetViewPlanes(), 6 ) )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005f17a0
TBOOL AXYZViewportManager::GetCursorRay( TVector4& a_rOutOrigin, TVector4& a_rOutDirection )
{
	TINT iX, iY;
	AInputManager2::GetCursorXY( iX, iY );

	VIEWPORT eViewport = GetViewportAt( iX, iY );

	if ( eViewport == -1 )
		return TFALSE;

	TViewport*      pViewport     = m_apViewports[ eViewport ];
	TRenderContext* pContext      = pViewport->GetRenderContext();
	TCameraObject*  pCameraObject = m_apCameraObjects[ eViewport ];

	TRenderContext* pOldContext = TRender::GetSingleton()->GetCurrentContext();
	TRender::GetSingleton()->SetCurrentRenderContext( pContext );
	pCameraObject->Render();
	TRender::GetSingleton()->SetCurrentRenderContext( pOldContext );

	const TVector3& rvTranslation = pCameraObject->GetTransformObject().GetTranslation();

	TQuaternion qRotation;
	TMatrix44::MatToQuat( qRotation, pCameraObject->GetTransformObject().GetMatrix() );

	iX -= TINT( pViewport->GetX() );
	iY -= TINT( pViewport->GetY() );

	const TRenderContext::PROJECTIONPARAMS& rParams = pContext->GetProjectionParameters();
	TVector4                                vScreen( ( TFLOAT( iX ) - rParams.m_Centre.x ) / rParams.m_Proj.x, ( TFLOAT( iY ) - rParams.m_Centre.y ) / rParams.m_Proj.y, 1.0f, 1.0f );

	TMatrix44 mCamera;
	mCamera.SetFromQuaternion( qRotation );
	mCamera.m_f41 = rvTranslation.x;
	mCamera.m_f42 = rvTranslation.y;
	mCamera.m_f43 = rvTranslation.z;

	TMatrix44::TransformVector( a_rOutDirection, mCamera, vScreen );

	a_rOutOrigin = TVector4( TVector3::VEC_ZERO.x, TVector3::VEC_ZERO.y, TVector3::VEC_ZERO.z, 1.0f );
	TMatrix44::TransformVector( a_rOutOrigin, mCamera, a_rOutOrigin );

	a_rOutDirection.x = a_rOutDirection.x - a_rOutOrigin.x;
	a_rOutDirection.y = a_rOutDirection.y - a_rOutOrigin.y;
	a_rOutDirection.z = a_rOutDirection.z - a_rOutOrigin.z;
	a_rOutDirection.Normalise();

	return TTRUE;
}

// $deBlob: FUNCTION 005f1aa0
TViewport* AXYZViewportManager::CreateViewport( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TViewport* pViewport = new TViewport();
	pViewport->SetMemoryAllocatorBlock( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) );
	pViewport->SetX( a_fX );
	pViewport->SetY( a_fY );
	pViewport->SetWidth( a_fWidth );
	pViewport->SetHeight( a_fHeight );
	pViewport->SetMinZ( 0.2f );
	pViewport->SetMaxZ( 1.0f );
	pViewport->AllowBackgroundClear( TTRUE );
	pViewport->AllowDepthClear( TTRUE );
	pViewport->Enable( TTRUE );
	pViewport->EnableDefaultBeginRender( TTRUE );
	pViewport->SetBackgroundColor( 64, 64, 64, 255 );

	return pViewport;
}

// $deBlob: FUNCTION 005f1980
AXYZViewportManager::VIEWPORT AXYZViewportManager::GetViewportAt( TINT a_iX, TINT a_iY ) const
{
	for ( TINT i = 0; i < VIEWPORT_Count; i++ )
	{
		TViewport* pViewport = m_apViewports[ i ];

		if ( pViewport->GetX() <= TFLOAT( a_iX ) && TFLOAT( a_iX ) <= pViewport->GetWidth() + pViewport->GetX() &&
		     pViewport->GetY() <= TFLOAT( a_iY ) && TFLOAT( a_iY ) <= pViewport->GetHeight() + pViewport->GetY() )
		{
			return VIEWPORT( i );
		}
	}

	return VIEWPORT_NONE;
}

// The rectangle of the full screen viewport (the display's size)
// $deBlob: FUNCTION 005f1bc0
void GetFullScreenRect( TFLOAT* a_pfX, TFLOAT* a_pfY, TFLOAT* a_pfWidth, TFLOAT* a_pfHeight )
{
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

	*a_pfX      = 0.0f;
	*a_pfY      = 0.0f;
	*a_pfWidth  = TFLOAT( TINT( pDisplayParams->uiWidth ) );
	*a_pfHeight = TFLOAT( TINT( pDisplayParams->uiHeight ) );
}

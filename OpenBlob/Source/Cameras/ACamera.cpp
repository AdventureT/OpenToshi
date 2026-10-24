#include "pch.h"
#include "ACamera.h"

#include <Math/TQuaternion.h>
#include <Math/TMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TFLOAT ACamera::sm_fDefaultFOV = 1.0471976f;

// $deBlob: FUNCTION 0058d0c0
// $deBlob: FUNCTION 0058d0d0
// $deBlob: FUNCTION 0058d0e0
// $deBlob: FUNCTION 0058d0f0
// $deBlob: FUNCTION 0058d100
// $deBlob: FUNCTION 0053b930
// $deBlob: FUNCTION 00857f70
TDEFINE_CLASS_NORUNTIME( ACamera );

const TVector4 ACamera::sm_vWorldUp = { 0.0f, -1.0f, 0.0f, 1.0f };

// The default camera (CT_DEFAULT) of ACameraManager::CreateCamera
// $deBlob: FUNCTION 0058d110
ACamera::ACamera()
    : m_bActive( TFALSE )
    , m_bUnk05( TFALSE )
    , m_eCamType( CT_DEFAULT )
{
	ResetCameraMatrix( m_Matrix );
}

// $deBlob: FUNCTION 0058d140
ACamera::ACamera( CameraType a_eCamType )
    : m_bActive( TFALSE )
    , m_bUnk05( TFALSE )
    , m_eCamType( a_eCamType )
{
	ResetCameraMatrix( m_Matrix );
	TASSERT( a_eCamType >= 0 && a_eCamType < CT_TYPECOUNT );
}

// $deBlob: FUNCTION 0058d1b0
// $deBlob: FUNCTION 0058da20
ACamera::~ACamera()
{
}

// $deBlob: FUNCTION 0058d210
void ACamera::SetMatrix( const TMatrix44& a_rMatrix )
{
	m_Matrix.m_mMatrix = a_rMatrix;
}

// $deBlob: FUNCTION 0058d250
void ACamera::SetShakeMatrix( const TMatrix44& a_rShakeMatrix )
{
	m_Matrix.m_mShakeMatrix = a_rShakeMatrix;
}

// $deBlob: FUNCTION 0058d420
void ACamera::LookAtPoint( const TVector4& a_rvEye, const TVector4& a_rvTarget )
{
	TVector4 vDir = a_rvTarget - a_rvEye;
	vDir.Normalise();
	m_Matrix.m_mMatrix.LookAtDirection( vDir, sm_vWorldUp );
	m_Matrix.m_mMatrix.SetTranslation( a_rvEye );
}

// $deBlob: FUNCTION 0058d4a0
void ACamera::LookAtPoint( const TVector4& a_rvTarget )
{
	TVector4 vDir = a_rvTarget - m_Matrix.m_mMatrix.GetTranslation();
	vDir.w        = a_rvTarget.w;
	vDir.Normalise();
	m_Matrix.m_mMatrix.LookAtDirection( vDir, sm_vWorldUp );
}

// $deBlob: FUNCTION 0058d510
void ACamera::LookAtDirection( const TVector4& a_rvDir )
{
	m_Matrix.m_mMatrix.LookAtDirection( a_rvDir, sm_vWorldUp );
}

// $deBlob: FUNCTION 0058d530
void ACamera::LookAtDirection( const TVector4& a_rvDir, const TVector4& a_rvUp )
{
	m_Matrix.m_mMatrix.LookAtDirection( a_rvDir, a_rvUp );
}

// $deBlob: FUNCTION 0058d540
TBOOL ACamera::IsInViewCone( const TVector4& a_rvPos, TFLOAT a_fSphereRadius ) const
{
	TVector4 vConePosition  = m_Matrix.m_mMatrix.GetTranslation();
	TVector4 vConeDirection = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vConeDirection.w        = 1.0f;

	return TMath::ConeVsSphere( vConePosition, vConeDirection, m_Matrix.m_fFOV * 0.5f, a_fSphereRadius, a_rvPos );
}

// $deBlob: FUNCTION 0058d5a0
TBOOL ACamera::IsInViewCone( const TVector4& a_rvPos, TFLOAT a_fSphereRadius, TFLOAT a_fConeLength ) const
{
	TVector4 vConePosition  = m_Matrix.m_mMatrix.GetTranslation();
	TVector4 vConeDirection = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vConeDirection.w        = 1.0f;

	return TMath::ConeVsSphere( vConePosition, vConeDirection, m_Matrix.m_fFOV * 0.5f, a_fConeLength, a_rvPos, a_fSphereRadius );
}

// Rotates the look direction around the world up axis
// $deBlob: FUNCTION 0058d610
void ACamera::RotateAroundWorldUp( TFLOAT a_fRotation )
{
	TVector4 vDir = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vDir.w        = 1.0f;

	TQuaternion quat;
	quat.SetRotation( sm_vWorldUp.AsVector3(), a_fRotation );
	TQuaternion::RotateVector( vDir, quat, vDir );

	m_Matrix.m_mMatrix.LookAtDirection( vDir, sm_vWorldUp );
}

// $deBlob: FUNCTION 0058d820
void ACamera::RotateAroundAxis( const TVector4& a_rvAxis, TFLOAT a_fRotation )
{
	TVector4 vDir = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vDir.w        = 1.0f;

	// NOTE: retail rotates around the world up, not a_rvAxis
	TQuaternion quat;
	quat.SetRotation( sm_vWorldUp.AsVector3(), a_fRotation );
	TQuaternion::RotateVector( vDir, quat, vDir );

	m_Matrix.m_mMatrix.LookAtDirection( vDir, sm_vWorldUp );
}

// Rotates the look direction around the camera's right axis, keeping it at least a_fLimit away from the world up/down
// $deBlob: FUNCTION 0058d670
void ACamera::RotateAroundRight( TFLOAT a_fRotation, TFLOAT a_fLimit )
{
	TVector4 vAxis = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vAxis.w        = 1.0f;

	TVector4 vRight;
	vRight.CrossProduct( vAxis, sm_vWorldUp );
	TASSERT( vRight.MagnitudeSq() > 0.0001f );
	vRight.Normalise();

	TQuaternion quat;
	quat.SetRotation( vRight.AsVector3(), a_fRotation );
	TQuaternion::RotateVector( vAxis, quat, vAxis );

	TFLOAT fDot   = sm_vWorldUp.x * vAxis.x + sm_vWorldUp.y * vAxis.y + sm_vWorldUp.z * vAxis.z;
	TFLOAT fLimit = 1.0f - a_fLimit;

	if ( fDot <= fLimit )
	{
		if ( a_fLimit - 1.0f > fDot )
		{
			TVector4 vDown = sm_vWorldUp;
			vDown.x        = -sm_vWorldUp.x;
			vDown.y        = -sm_vWorldUp.y;
			vDown.z        = -sm_vWorldUp.z;

			quat.SetRotation( vRight.AsVector3(), TMath::ACos( fLimit ) );
			TQuaternion::RotateVector( vAxis, quat, vDown );
		}
	}
	else
	{
		quat.SetRotation( vRight.AsVector3(), -TMath::ACos( fLimit ) );
		TQuaternion::RotateVector( vAxis, quat, sm_vWorldUp );
	}

	m_Matrix.m_mMatrix.LookAtDirection( vAxis, sm_vWorldUp );
}

// $deBlob: FUNCTION 0058d880
void ACamera::SetViewPlanes( const TPlane* a_pPlanes, TINT a_iNumPlanes )
{
	for ( TINT i = 0; i < a_iNumPlanes; i++ )
	{
		if ( i >= 6 ) return;

		m_aViewPlanes[ i ] = a_pPlanes[ i ];
	}
}

// $deBlob: FUNCTION 0058d8b0
TBOOL ACamera::IsPointInsideFrustum( const TVector4& a_rvPoint ) const
{
	for ( TINT i = 0; i < 6; i++ )
	{
		const TPlane& rPlane = m_aViewPlanes[ i ];

		if ( !( rPlane.GetNormal().x * a_rvPoint.x + rPlane.GetNormal().y * a_rvPoint.y + rPlane.GetNormal().z * a_rvPoint.z - rPlane.GetD() <= 0.0f ) )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0058d920
void ACamera::ResetCameraMatrix( CameraMatrix& a_rCamMat )
{
	a_rCamMat.m_mMatrix            = TMatrix44::IDENTITY;
	a_rCamMat.m_mShakeMatrix       = TMatrix44::IDENTITY;
	a_rCamMat.m_fNear              = 5.0f;
	a_rCamMat.m_fFar               = 500.0f;
	a_rCamMat.m_fFOV               = sm_fDefaultFOV;
	a_rCamMat.m_fProjectionCentreX = 0.5f;
	a_rCamMat.m_fProjectionCentreY = 0.5f;
	a_rCamMat.m_fAspect            = TFLOAT( tan( sm_fDefaultFOV * 0.5f ) );
}

// $deBlob: FUNCTION 0058d290
void ACamera::GetFlatForward( TVector4& a_rOutForward ) const
{
	TVector4 vForward( GetMatrix().m_f31, GetMatrix().m_f32, GetMatrix().m_f33, 1.0f );
	a_rOutForward.ProjectNormalToPlane( vForward, sm_vWorldUp );
}

// $deBlob: FUNCTION 0058d360
void ACamera::GetAppSideEuler( TFLOAT& a_rfRoll, TFLOAT& a_rfPitch, TFLOAT& a_rfYaw ) const
{
	const TVector4& rvForward = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_FORWARD );

	a_rfRoll  = 0.0f;
	a_rfPitch = TMath::ASin( -rvForward.y );
	a_rfYaw   = TMath::ATan2( rvForward.x, rvForward.z );
}

// $deBlob: FUNCTION 0058d2d0
void ACamera::GetHorizRight( TVector4& a_rvHorizRight ) const
{
	TVector4 vAxis = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vAxis.w        = 1.0f;

	a_rvHorizRight.CrossProduct( vAxis, sm_vWorldUp );
	TASSERT( a_rvHorizRight.MagnitudeSq() > 0.0001f );
	a_rvHorizRight.Normalise();
}

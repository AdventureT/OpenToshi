#include "pch.h"
#include "Cheats/ACheatManager.h"
#include "ABlobFollowCamera.h"
#include "ACameraManager.h"
#include "Buildings/ABuildingManager.h"
#include "Challenges/AChallengeManager.h"
#include "ARevolutionary/ARevolutionary.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Entities/APropTriggerHelp.h"
#include "Entities/APropTriggerLevelComplete.h"
#include "AScriptedCamera.h"
#include "GameInterface/APlayerManager.h"
#include "GameController/AGameStatus.h"
#include "Options/AOptions.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"

#include "Input/ABlobInputMap.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/AGameStateController.h"
#include <Input/TInputDeviceKeyboard.h>
#include <Physics/Havok/TPhysicsManager.h>
#include "Entities/ABoss/ABoss.h"
#include <Physics/TSphereData.h>
#include <Render/TViewport.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TFLOAT ABlobFollowCamera::ms_fCamSpeedOption      = 1.0f;
TFLOAT ABlobFollowCamera::ms_fInvertY             = 1.0f;
TFLOAT ABlobFollowCamera::ms_fCollisionRadius     = 1.5f;
TFLOAT ABlobFollowCamera::ms_fDefaultMaxDist      = 45.0f;
TFLOAT ABlobFollowCamera::ms_fDefaultIdealDist    = 40.0f;
TFLOAT ABlobFollowCamera::ms_fAcceptableIdealDist = 25.0f;
TFLOAT ABlobFollowCamera::ms_fDefaultMinDist      = 2.0f;
TFLOAT ABlobFollowCamera::ms_fRotSpeed            = 3.5f;
TFLOAT ABlobFollowCamera::ms_fUnkBC2C             = 0.25f;
TFLOAT ABlobFollowCamera::ms_fUnkBC30             = 0.25f;
TFLOAT ABlobFollowCamera::ms_fPitchFactor         = 0.55f;
TFLOAT ABlobFollowCamera::ms_fLookAngleFactor     = 0.8f;
TFLOAT ABlobFollowCamera::ms_afLookSpeeds[ 8 ]    = { 6.0f, 15.0f, 6.5f, 7.5f, 10.0f, 15.0f, 7.5f, 15.0f };
TFLOAT ABlobFollowCamera::ms_fAnimCamDist         = 32.0f;

// $deBlob: FUNCTION 0058c230
TFLOAT ABlobFollowCamera::GetMAXCamSpeedOption()
{
	return 1.0f;
}

// $deBlob: FUNCTION 0058c240
TFLOAT ABlobFollowCamera::GetMINCamSpeedOption()
{
	return 0.5f;
}
TFLOAT ABlobFollowCamera::ms_fAnimCamHeight       = 32.0f;
TFLOAT ABlobFollowCamera::ms_fAnimCamLookDown     = 19.0f;
TFLOAT ABlobFollowCamera::ms_fUnkBC68             = 1.5f;
TFLOAT ABlobFollowCamera::ms_fRotBlendDown        = 50.0f;
TFLOAT ABlobFollowCamera::ms_fRotBlendUp          = 1.0f;
TFLOAT ABlobFollowCamera::ms_fRotBlendTime        = 1.0f;
TFLOAT ABlobFollowCamera::ms_fYawFactor;
// $deBlob: FUNCTION 0053b7d0
TFLOAT ABlobFollowCamera::ms_fOrbitSpeed = Toshi::TMath::HALF_PI * 0.25f;
// $deBlob: FUNCTION 0053b880
TFLOAT ABlobFollowCamera::ms_fFOVMin = Toshi::TMath::HALF_PI * 60.0f / 90.0f;
// $deBlob: FUNCTION 0053b8b0
TFLOAT ABlobFollowCamera::ms_fMinPitch = Toshi::TMath::HALF_PI - Toshi::TMath::HALF_PI * 25.0f / 90.0f;
// $deBlob: FUNCTION 0053b850
TFLOAT ABlobFollowCamera::ms_fMaxPitch = Toshi::TMath::HALF_PI * 50.0f / 90.0f + Toshi::TMath::HALF_PI;
// $deBlob: FUNCTION 0053b7b0
TFLOAT ABlobFollowCamera::ms_fFOVBlendSpeed = Toshi::TMath::HALF_PI * 0.25f;
// $deBlob: FUNCTION 0053b910
TFLOAT ABlobFollowCamera::ms_fUnkF18 = Toshi::TMath::HALF_PI * 0.6f;
// $deBlob: FUNCTION 0053b7f0
TFLOAT ABlobFollowCamera::ms_fOrbitPitch = Toshi::TMath::HALF_PI * 25.0f / 90.0f + Toshi::TMath::HALF_PI;
// $deBlob: FUNCTION 0053b820
TFLOAT ABlobFollowCamera::ms_fFOVMax = Toshi::TMath::HALF_PI * 100.0f / 90.0f;
// $deBlob: FUNCTION 0053b8e0
TFLOAT                       ABlobFollowCamera::ms_fLiquidPitch = Toshi::TMath::HALF_PI * 15.0f / 90.0f + Toshi::TMath::HALF_PI;
TBOOL                        ABlobFollowCamera::ms_bUnderCam;
ABlobFollowCamera::PosSpeeds ABlobFollowCamera::ms_aPosSpeeds[ PS_NUMOF ];

// A camera region of ACameraManager (+0x4, not ported)

// $deBlob: FUNCTION 00582fd0
// $deBlob: FUNCTION 00582fe0
// $deBlob: FUNCTION 00583050
// $deBlob: FUNCTION 005830b0
// $deBlob: FUNCTION 005830c0
// $deBlob: FUNCTION 0053b770
// $deBlob: FUNCTION 00857f60
TDEFINE_CLASS( ABlobFollowCamera );

// $deBlob: FUNCTION 005889a0
ABlobFollowCamera::ABlobFollowCamera()
    : ACamera( CT_BLOBFOLLOW )
{
	m_pOverrideCam    = TNULL;
	m_mPrevMatrix     = TMatrix44::IDENTITY;
	m_bRotBlend       = TFALSE;
	m_fRotBlend       = 100.0f;
	m_fRotBlendTarget = 1000.0f;
	m_fRotBlendTimer  = 1000.0f;
	m_iUnk3B0         = 1;
	m_uiFlags         = 1;
	m_iPosType        = POS_HANDLE_NONE;
	m_pNedFixedCam    = TNULL;
	m_pBlob           = TNULL;
	m_bUnk1C1         = TFALSE;

	for ( TINT i = MAX_ALPHA_INSTANCES; i != 0; )
	{
		i--;
		m_afAlphaInstances[ i ] = 0.0f;
		m_afAlpha[ i ]          = 1.0f;
		m_afAlphaTarget[ i ]    = 1.0f;
		m_abUnk344[ i ]         = TFALSE;
	}

	m_pPhantom1   = TNULL;
	m_pPhantom2   = TNULL;
	m_bBlobVision = TTRUE;
}

// $deBlob: FUNCTION 00588ab0
// $deBlob: FUNCTION 0058c790
ABlobFollowCamera::~ABlobFollowCamera()
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( m_pPhantom1 != TNULL )
	{
		pPhysicsManager->DestroyPhantom( TSTATICCAST( TPhysicsPhantomContainer, m_pPhantom1 ) );
		m_pPhantom1 = TNULL;
	}

	if ( m_pPhantom2 != TNULL )
	{
		pPhysicsManager->DestroyPhantom( TSTATICCAST( TPhysicsPhantomContainer, m_pPhantom2 ) );
		m_pPhantom2 = TNULL;
	}
}

// $deBlob: FUNCTION 005830d0
void ABlobFollowCamera::UpdateFollowPosition( TFLOAT a_fDeltaTime, TVector4& a_rvPos, const TVector4& a_rvCamPos, const TVector4& a_rvSmoothPos, const TVector4& a_rvFacing, const TVector4& a_rvTarget, const TVector4& a_rvBlobPos, TFLOAT a_fSpeed, TFLOAT a_fMaxSpeed, TFLOAT a_fRadius, TFLOAT a_fMinDist, TFLOAT a_fIdealDist, TFLOAT a_fMaxDist, TBOOL a_bGrounded, TFLOAT* a_pfTimer1, TFLOAT* a_pfTimer2, void* a_pPhantom1, void* a_pPhantom2 )
{
	TVector4 vOldPos = a_rvPos;

	// The direction from the target to the camera
	TVector4 vDir;
	vDir.x       = a_rvPos.x - a_rvTarget.x;
	vDir.y       = a_rvPos.y - a_rvTarget.y;
	vDir.z       = a_rvPos.z - a_rvTarget.z;
	vDir.w       = a_rvPos.w;
	TFLOAT fDist = vDir.NormaliseMag();

	TVector4 vAxis;
	vAxis.CrossProduct( TVector4::VEC_POSY, vDir );
	vAxis.Normalise();

	TVector4    vRotated = TVector4::VEC_POSY;
	TQuaternion qFrom;
	qFrom.SetVectorDirection( vDir.AsVector3(), TVector3::VEC_POSZ );

	TFLOAT fSpeedRatio = a_fSpeed / a_fMaxSpeed;
	TMath::Clip( fSpeedRatio, 0.0f, 1.0f );

	TBOOL       bOrbit = TTRUE;
	TFLOAT      fBlend;
	TQuaternion qTo;

	if ( fDist > a_fMaxDist && fSpeedRatio > 0.2f )
	{
		// Too far and moving: turn to the side of the blob
		qTo.SetRotation( vAxis.AsVector3(), a_bGrounded ? ms_fLiquidPitch : TMath::HALF_PI );
		TQuaternion::RotateVector( vRotated, qTo, vRotated );
		vRotated.Normalise();
		qTo.SetVectorDirection( vRotated.AsVector3(), TVector3::VEC_POSZ );
		fBlend = ms_fOrbitSpeed * a_fDeltaTime;
	}
	else if ( fSpeedRatio < 0.2f || fDist < a_fMaxDist * 0.9f )
	{
		qTo.SetRotation( vAxis.AsVector3(), ms_fOrbitPitch );
		TQuaternion::RotateVector( vRotated, qTo, vRotated );
		vRotated.Normalise();
		qTo.SetVectorDirection( vRotated.AsVector3(), TVector3::VEC_POSZ );
		fBlend = ms_fOrbitSpeed * a_fDeltaTime * 0.3f;
	}
	else
	{
		bOrbit = TFALSE;
	}

	if ( bOrbit )
	{
		TQuaternion qBlend;
		qBlend.Lerp( qFrom, qTo, fBlend );
		TQuaternion::RotateVector( vDir, qBlend, TVector4::VEC_POSZ );
		vDir.Normalise();
	}

	// Clamp the pitch
	TVector4 vPitchDir = vDir;
	TFLOAT   fPitch    = vPitchDir.GetAngleBetweenVector( TVector4::VEC_POSY );
	TMath::Clip( fPitch, a_bGrounded ? ms_fLiquidPitch : ms_fMinPitch, ms_fMaxPitch );

	TVector4 vPitchAxis;
	vPitchAxis.CrossProduct( TVector4::VEC_POSY, vPitchDir );
	vPitchAxis.Normalise();

	TQuaternion qPitch;
	qPitch.SetRotation( vPitchAxis.AsVector3(), fPitch );
	vPitchDir = TVector4::VEC_POSY;
	TQuaternion::RotateVector( vPitchDir, qPitch, vPitchDir );
	vPitchDir.Normalise();

	TFLOAT fCamDist = fDist;
	TMath::Clip( fCamDist, a_fIdealDist, a_fMaxDist );

	TVector4 vHit;
	CastRay( a_rvTarget, vPitchDir, vHit, fCamDist, 0x10, TNULL );

	fCamDist -= ms_fCollisionRadius;
	TMath::Clip( fCamDist, 0.0f, a_fMaxDist );

	TVector4 vOffset;
	vOffset.Multiply( vPitchDir, fCamDist );

	a_rvPos.x = a_rvTarget.x + vOffset.x;
	a_rvPos.y = a_rvTarget.y + vOffset.y;
	a_rvPos.z = a_rvTarget.z + vOffset.z;
	a_rvPos.w = a_rvTarget.w;

	TVector4 vMove;
	vMove.x          = a_rvPos.x - vOldPos.x;
	vMove.y          = a_rvPos.y - vOldPos.y;
	vMove.z          = a_rvPos.z - vOldPos.z;
	vMove.w          = a_rvPos.w;
	TFLOAT fMoveDist = vMove.NormaliseMag();

	TBOOL bBlocked = SphereCast( vOldPos, vMove, vHit, fMoveDist, ms_fCollisionRadius, 0x10 );

	TBOOL bAvoided = TFALSE;

	if ( fCamDist < a_fMinDist || *a_pfTimer2 > 0.0f )
	{
		bAvoided = AvoidCeiling( a_rvTarget, a_rvBlobPos, a_rvPos, a_pfTimer2, a_fMinDist, a_fIdealDist, a_fMaxDist, a_fRadius, ms_fCollisionRadius );
	}

	TBOOL bFlag;

	if ( ( fCamDist < a_fMinDist || bBlocked ) && !bAvoided )
	{
		FindOpenDirection( a_rvTarget, a_rvFacing, a_rvPos, a_fMinDist, a_fIdealDist, a_fMaxDist, ms_fCollisionRadius );
		bFlag       = TTRUE;
		*a_pfTimer1 = 1.0f;
	}
	else
	{
		bFlag = *a_pfTimer1 > 0.0f;
	}

	AvoidSideWalls( a_fDeltaTime, a_rvTarget, a_rvPos, a_fMaxDist, a_fIdealDist, ms_fCollisionRadius, bFlag, a_pPhantom1, a_pPhantom2 );

	TVector4 vFinal;
	vFinal.x          = a_rvPos.x - a_rvTarget.x;
	vFinal.y          = a_rvPos.y - a_rvTarget.y;
	vFinal.z          = a_rvPos.z - a_rvTarget.z;
	vFinal.w          = a_rvPos.w;
	TFLOAT fFinalDist = vFinal.NormaliseMag();

	CastRay( a_rvTarget, vFinal, vHit, fFinalDist, 0x10, TNULL );

	vOffset.Multiply( vFinal, fFinalDist );
	a_rvPos.x = a_rvTarget.x + vOffset.x;
	a_rvPos.y = a_rvTarget.y + vOffset.y;
	a_rvPos.z = a_rvTarget.z + vOffset.z;
	a_rvPos.w = a_rvTarget.w;
}

// $deBlob: FUNCTION 00585d20
void ABlobFollowCamera::LerpPosition( TFLOAT a_fDeltaTime, TVector4& a_rvPos, const TVector4& a_rvTo, TFLOAT a_fSpeed )
{
	TVector4 vFrom = a_rvPos;
	TFLOAT   fT    = a_fDeltaTime * a_fSpeed;
	TMath::Clip( fT, 0.0f, 1.0f );
	a_rvPos.Lerp4( vFrom, a_rvTo, fT );
}

// $deBlob: FUNCTION 00585d80
void ABlobFollowCamera::UpdateLookDirection( TFLOAT a_fDeltaTime, const TVector4& a_rvCamPos, const TVector4& a_rvTarget, const TVector4& a_rvFacing, TVector4& a_rvDir, TVector4& a_rvUp, const TVector4& a_rvRight, TFLOAT* a_pfRotSpeed, TFLOAT* a_pfPitch, TFLOAT* a_pfYaw, TFLOAT a_fFOV, TFLOAT a_fVFOV, TBOOL a_bNoOffset, TBOOL a_bSnap, TFLOAT a_fMinBlend )
{
	TASSERT( a_rvDir.IsNormalised() );

	TVector4 vToTarget;
	vToTarget.x = a_rvTarget.x - a_rvCamPos.x;
	vToTarget.y = a_rvTarget.y - a_rvCamPos.y;
	vToTarget.z = a_rvTarget.z - a_rvCamPos.z;
	vToTarget.w = a_rvTarget.w;
	vToTarget.Normalise();

	TVector4 vLook = vToTarget;

	TVector4 vSide;
	vSide.CrossProduct( vToTarget, TVector4::VEC_NEGY );
	vSide.NormaliseXZ();

	// Look ahead of the blob in the direction it faces
	TFLOAT fForward = 0.0f;
	TFLOAT fSide    = 0.0f;

	if ( !a_bNoOffset )
	{
		fForward = a_rvFacing.x * vToTarget.x + a_rvFacing.z * vToTarget.z;
		fSide    = a_rvFacing.x * vSide.x + a_rvFacing.z * vSide.z;
	}

	TMath::Clip( fForward, -1.0f, 1.0f );
	TMath::Clip( fSide, -1.0f, 1.0f );

	TFLOAT fOffsetBlend = a_fDeltaTime * 1.3f;
	TFLOAT fPitch       = fForward * a_fVFOV * 0.5f * ms_fPitchFactor;
	TFLOAT fYaw         = fSide * a_fFOV * 0.5f * ms_fYawFactor;
	TMath::Clip( fOffsetBlend, 0.0f, 1.0f );

	if ( a_fMinBlend <= 0.0f )
	{
		*a_pfPitch = TMath::LERP( *a_pfPitch, fPitch, fOffsetBlend );
		*a_pfYaw   = TMath::LERP( *a_pfYaw, fYaw, fOffsetBlend );
	}

	TQuaternion qRot;
	qRot.SetRotation( a_rvRight.AsVector3(), *a_pfPitch );
	TQuaternion::RotateVector( vLook, qRot, vLook );
	vLook.Normalise();
	qRot.SetRotation( TVector3::VEC_POSY, *a_pfYaw );
	TQuaternion::RotateVector( vLook, qRot, vLook );
	vLook.Normalise();

	if ( !a_bSnap )
	{
		if ( a_fVFOV <= a_fFOV )
			a_fFOV = a_fVFOV;

		TFLOAT fMaxAngle   = ms_fLookAngleFactor * a_fFOV;
		TFLOAT fAngle      = vToTarget.GetAngleBetweenVector( a_rvDir );
		TFLOAT fRotSpeed   = ms_afLookSpeeds[ 0 ];
		TFLOAT fSpeedBlend = a_fFOV;

		if ( fAngle + fAngle <= fMaxAngle )
		{
			fSpeedBlend = ms_afLookSpeeds[ 2 ];

			if ( fAngle + fAngle <= fMaxAngle && a_bNoOffset )
			{
				fRotSpeed   = ms_afLookSpeeds[ 4 ];
				fSpeedBlend = ms_afLookSpeeds[ 6 ];
			}
		}
		else if ( !a_bNoOffset )
		{
			fRotSpeed   = ms_afLookSpeeds[ 1 ];
			fSpeedBlend = ms_afLookSpeeds[ 3 ];
		}
		else
		{
			fRotSpeed   = ms_afLookSpeeds[ 5 ];
			fSpeedBlend = ms_afLookSpeeds[ 7 ];
		}

		fSpeedBlend *= a_fDeltaTime;
		TMath::Clip( fSpeedBlend, 0.0f, 1.0f );
		*a_pfRotSpeed = TMath::LERP( *a_pfRotSpeed, fRotSpeed, fSpeedBlend );

		TFLOAT fBlend = *a_pfRotSpeed * a_fDeltaTime;
		if ( fBlend < a_fMinBlend )
			fBlend = a_fMinBlend;

		TMath::Clip( fBlend, 0.0f, 1.0f );

		TQuaternion qFrom;
		TQuaternion qTo;
		TQuaternion qBlend;
		qFrom.SetVectorDirection( a_rvDir.AsVector3(), TVector3::VEC_POSZ );
		qTo.SetVectorDirection( vLook.AsVector3(), TVector3::VEC_POSZ );
		TQuaternion::Slerp( qBlend, qFrom, qTo, fBlend );
		TQuaternion::RotateVector( a_rvDir, qBlend, TVector4::VEC_POSZ );
	}
	else
	{
		a_rvDir = vLook;
	}

	a_rvDir.Normalise();
	a_rvUp = sm_vWorldUp;
}

// Used by the manual camera (POS_HANDLE_3) to turn the view towards the target.
// $deBlob: FUNCTION 005863b0
void ABlobFollowCamera::UpdateBlendedLookAt( TFLOAT a_fDeltaTime, TFLOAT a_fSpeed, TFLOAT* a_pfBlend, const TVector4& a_rvTarget, TMatrix44& a_rmOut )
{
	TQuaternion qStart;
	TQuaternion qEnd;
	TMatrix44::MatToQuat( qStart, a_rmOut );

	// The look at matrix keeps the position of the output
	TMatrix44 mLookAt = a_rmOut;
	mLookAt.LookAtTarget( a_rvTarget, mLookAt.GetTranslation() );
	TMatrix44::MatToQuat( qEnd, mLookAt );

	*a_pfBlend = a_fDeltaTime * a_fSpeed + *a_pfBlend;
	TMath::Clip( *a_pfBlend, 0.0f, 1.0f );
	TQuaternion::Slerp( qStart, qStart, qEnd, *a_pfBlend );

	TFLOAT fLength = TMath::Sqrt( qStart.x * qStart.x + qStart.y * qStart.y + qStart.z * qStart.z + qStart.w * qStart.w );
	qStart /= fLength;
	a_rmOut.SetFromQuaternion( qStart );
}

// Turns a_rmOut (keeping its position) towards a_rvTarget: its look point (the target projected on its view line) is
// lerped to the target by a_fDeltaTime * a_fSpeed
// $deBlob: FUNCTION 005861a0
void ABlobFollowCamera::UpdateTurnTowards( TFLOAT a_fDeltaTime, TFLOAT a_fSpeed, const TVector4& a_rvTarget, TMatrix44& a_rmOut )
{
	TVector4 vUpDir( a_rmOut.m_f21, a_rmOut.m_f22, a_rmOut.m_f23, 1.0f );
	TVector4 vLookDir( a_rmOut.m_f31, a_rmOut.m_f32, a_rmOut.m_f33, 1.0f );
	TASSERT( vLookDir.IsNormalised() );
	TASSERT( vUpDir.IsNormalised() );

	TVector4 vPos = a_rmOut.GetTranslation();

	TVector4 vLine;
	vLine.x = ( vPos.x + vLookDir.x ) - vPos.x;
	vLine.y = ( vPos.y + vLookDir.y ) - vPos.y;
	vLine.z = ( vPos.z + vLookDir.z ) - vPos.z;
	vLine.w = a_rmOut.m_f44;

	TFLOAT fDot = vLine.y * ( a_rvTarget.y - vPos.y ) + vLine.x * ( a_rvTarget.x - vPos.x ) + vLine.z * ( a_rvTarget.z - vPos.z );

	TVector4 vOffset;
	vOffset.Multiply( vLookDir, fDot / vLine.Magnitude() );

	TVector4 vLookAt = vPos;
	vLookAt.x += vOffset.x;
	vLookAt.y += vOffset.y;
	vLookAt.z += vOffset.z;

	TFLOAT fBlend = a_fDeltaTime * a_fSpeed;
	TMath::Clip( fBlend, 0.0f, 1.0f );
	vLookAt.Lerp4( vLookAt, a_rvTarget, fBlend );

	a_rmOut.LookAtTarget( vLookAt, a_rmOut.GetTranslation() );
}

// Slerps the orientation of a_rmOut to a_rqOrient by a_fDeltaTime * a_fSpeed
// $deBlob: FUNCTION 005864c0
void ABlobFollowCamera::UpdateBlendedOrient( TFLOAT a_fDeltaTime, TFLOAT a_fSpeed, const TQuaternion& a_rqOrient, TMatrix44& a_rmOut )
{
	TQuaternion qOrient;
	TMatrix44::MatToQuat( qOrient, a_rmOut );

	TFLOAT fBlend = a_fDeltaTime * a_fSpeed;
	TMath::Clip( fBlend, 0.0f, 1.0f );
	TQuaternion::Slerp( qOrient, qOrient, a_rqOrient, fBlend );
	qOrient /= TMath::Sqrt( qOrient.x * qOrient.x + qOrient.y * qOrient.y + qOrient.z * qOrient.z + qOrient.w * qOrient.w );
	a_rmOut.SetFromQuaternion( qOrient );
}

// Turns a_rmOut towards a_rvFrom + ( a_rvTo - a_rvFrom ) rotated by -a_fAngle * 0.3 around Y; a_rvUnused isn't read;
// Used by the challenge camera (POS_HANDLE_1)
// $deBlob: FUNCTION 00586580
void ABlobFollowCamera::UpdateOffsetLookAt( TFLOAT a_fDeltaTime, TFLOAT a_fSpeed, TFLOAT a_fAngle, const TVector4& a_rvFrom, const TVector4& a_rvTo, const TVector4& a_rvUnused, TMatrix44& a_rmOut )
{
	TVector4 vOffset( a_rvTo.x - a_rvFrom.x, a_rvTo.y - a_rvFrom.y, a_rvTo.z - a_rvFrom.z, a_rvTo.w );
	vOffset.RotateY( -( a_fAngle * 0.3f ) );

	TVector4 vLookAt( a_rvFrom.x + vOffset.x, a_rvFrom.y + vOffset.y, a_rvFrom.z + vOffset.z, a_rvFrom.w );
	UpdateTurnTowards( a_fDeltaTime, a_fSpeed, vLookAt, a_rmOut );
}

// Orientation of the blended fixed camera: slerps from the last orientation (a_rmLast, its translation is the
// last position) to the fixed camera's a_rqOrient, by how far the camera has got from the last position towards the
// ideal position. Returns FALSE (no matrix) when the look at the target turns more than 90 degrees from the last
// position: the caller then looks at the target. NOTE: retail also builds a look-at matrix there whose result is never
// used; retail's fixed-camera time/speed inputs (a1, a2) are unused as well.
// $deBlob: FUNCTION 00586640
TBOOL ABlobFollowCamera::UpdateBlendedFixedCam( const TVector4& a_rvTarget, const TVector4& a_rvCamPos, const TVector4& a_rvIdealPos, const TMatrix44& a_rmLast, const TQuaternion& a_rqOrient, TMatrix44& a_rmOut, TBOOL a_bBlend )
{
	TVector4 vLast = a_rmLast.GetTranslation();

	if ( !a_bBlend )
	{
		TVector4 vLastDir;
		vLastDir.x = vLast.x - a_rvTarget.x;
		vLastDir.y = vLast.y - a_rvTarget.y;
		vLastDir.z = vLast.z - a_rvTarget.z;
		vLastDir.w = vLast.w;
		vLastDir.Normalise();

		TVector4 vIdealDir;
		vIdealDir.x = a_rvIdealPos.x - a_rvTarget.x;
		vIdealDir.y = a_rvIdealPos.y - a_rvTarget.y;
		vIdealDir.z = a_rvIdealPos.z - a_rvTarget.z;
		vIdealDir.w = a_rvIdealPos.w;
		vIdealDir.Normalise();

		if ( vIdealDir.GetAngleBetweenVector( vLastDir ) > 1.5707964f )
		{
			// NOTE: retail builds a look-at matrix from the midpoint of the last and the ideal positions towards the
			// target here and then discards it (it returns TFALSE below, after the fraction): kept for faithfulness
			TVector4 vMid;
			vMid.x = vLast.x + a_rvIdealPos.x;
			vMid.y = vLast.y + a_rvIdealPos.y;
			vMid.z = vLast.z + a_rvIdealPos.z;
			vMid.w = a_rvIdealPos.w;
			vMid.Multiply( 0.5f );

			TVector4 vMidDir;
			vMidDir.x = a_rvTarget.x - vMid.x;
			vMidDir.y = a_rvTarget.y - vMid.y;
			vMidDir.z = a_rvTarget.z - vMid.z;
			vMidDir.w = a_rvTarget.w;
			vMidDir.Normalise();

			TMatrix44 mUnused = TMatrix44::IDENTITY;
			mUnused.LookAtDirection( vMidDir, TVector4::VEC_NEGY );
			return TFALSE;
		}
	}

	// How far the camera has got from the last position towards the ideal position (NormaliseMag normalises in place)
	TVector4 vToIdeal;
	vToIdeal.x     = a_rvIdealPos.x - vLast.x;
	vToIdeal.y     = a_rvIdealPos.y - vLast.y;
	vToIdeal.z     = a_rvIdealPos.z - vLast.z;
	vToIdeal.w     = a_rvIdealPos.w;
	TFLOAT fLength = vToIdeal.NormaliseMag();

	TFLOAT fFraction = 1.0f;

	if ( fLength > 0.0001f )
	{
		fFraction = ( ( a_rvCamPos.x - vLast.x ) * vToIdeal.x + ( a_rvCamPos.y - vLast.y ) * vToIdeal.y + ( a_rvCamPos.z - vLast.z ) * vToIdeal.z ) / fLength;
		TMath::Clip( fFraction, 0.0f, 1.0f );
	}

	TQuaternion qFrom;
	TMatrix44::MatToQuat( qFrom, a_rmLast );

	TQuaternion qBlend;
	TQuaternion::Slerp( qBlend, qFrom, a_rqOrient, fFraction );
	qBlend /= TMath::Sqrt( qBlend.x * qBlend.x + qBlend.y * qBlend.y + qBlend.z * qBlend.z + qBlend.w * qBlend.w );
	a_rmOut.SetFromQuaternion( qBlend );
	return TTRUE;
}

// The free look (the right stick while m_bFreeLook): pitches a_rvDir around a_rvRight by a_pfInput[ 1 ] (kept 20-160
// degrees from up) and turns it around Y by a_pfInput[ 0 ], both past a dead zone; a_rvUp becomes the world up;
// $deBlob: FUNCTION 00586970
void ABlobFollowCamera::UpdateFreeLook( TFLOAT a_fDeltaTime, TVector4& a_rvDir, TVector4& a_rvUp, const TVector4& a_rvRight, const TFLOAT* a_pfInput, TBOOL a_bActive )
{
	TASSERT( a_rvDir.IsNormalised() );

	if ( a_bActive )
	{
		TFLOAT fX = TMath::Abs( a_pfInput[ 0 ] ) - ms_fUnkBC2C;
		TFLOAT fY = TMath::Abs( a_pfInput[ 1 ] ) - ms_fUnkBC30;

		if ( fY > 0.0f )
		{
			static TFLOAT s_fMaxAngle = TMath::HALF_PI + TMath::HALF_PI * 70.0f / 90.0f;
			static TFLOAT s_fMinAngle = TMath::HALF_PI - TMath::HALF_PI * 70.0f / 90.0f;

			TFLOAT fAngle = a_rvDir.GetAngleBetweenVector( TVector4::VEC_POSY );
			TFLOAT fStep  = ( ( ms_fUnkF18 * a_fDeltaTime ) / ( 1.0f - ms_fUnkBC30 ) ) * fY;
			TBOOL  bTurn  = TFALSE;

			if ( a_pfInput[ 1 ] > 0.0f )
			{
				if ( fAngle > s_fMinAngle )
				{
					fAngle -= fStep;
					bTurn = TTRUE;
				}
			}
			else if ( fAngle < s_fMaxAngle )
			{
				fAngle += fStep;
				bTurn = TTRUE;
			}

			if ( bTurn )
			{
				TMath::Clip( fAngle, s_fMinAngle, s_fMaxAngle );

				if ( fStep != 0.0f )
				{
					TQuaternion qRotation;
					qRotation.SetRotation( a_rvRight.AsVector3(), fAngle );
					a_rvDir = TVector4::VEC_POSY;
					TQuaternion::RotateVector( a_rvDir, qRotation, a_rvDir );
					a_rvDir.Normalise();
				}
			}
		}

		if ( fX > 0.0f )
		{
			TFLOAT fStep = ( ( ms_fUnkF18 * a_fDeltaTime ) / ( 1.0f - ms_fUnkBC2C ) ) * fX;

			if ( a_pfInput[ 0 ] < 0.0f )
				fStep = -fStep;

			a_rvDir.RotateY( fStep );
		}
	}

	a_rvUp = sm_vWorldUp;
}

// $deBlob: FUNCTION 00586be0
void ABlobFollowCamera::UpdateFollowFOV( const TVector4& a_rvCamPos, const TVector4& a_rvBlobPos, TVector4 a_vVelocity, TFLOAT a_fMaxPlayerSpeed, TFLOAT* a_pfFOV, TBOOL a_bNarrow )
{
	TVector4 vDir;
	vDir.x = a_rvBlobPos.x - a_rvCamPos.x;
	vDir.y = a_rvBlobPos.y - a_rvCamPos.y;
	vDir.z = a_rvBlobPos.z - a_rvCamPos.z;
	vDir.w = a_rvBlobPos.w;
	vDir.Normalise();

	TASSERT( a_fMaxPlayerSpeed > 0.0f );

	TFLOAT fSpeed = TMath::Pow( a_vVelocity.NormaliseMag() / a_fMaxPlayerSpeed, 0.25f );
	TMath::Clip( fSpeed, 0.0f, 1.0f );

	TFLOAT fDot = a_vVelocity.x * vDir.x + a_vVelocity.y * vDir.y + vDir.z * a_vVelocity.z;
	TMath::Clip( fDot, 0.0f, 1.0f );

	if ( a_bNarrow )
	{
		*a_pfFOV = fDot * 0.2617994f * fSpeed + 0.87266463f;
		return;
	}

	*a_pfFOV = ( ms_fFOVMax - ms_fFOVMin ) * fDot * fSpeed + ms_fFOVMin;
}

// $deBlob: FUNCTION 00586d70
TBOOL ABlobFollowCamera::SphereCast( const TVector4& a_rvFrom, const TVector4& a_rvDir, TVector4& a_rvOutNormal, TFLOAT& a_rfDist, TFLOAT a_fRadius, TUINT32 a_uiCollisionLayer )
{
	TASSERT( a_rvDir.IsNormalised() );
	TASSERT( a_rfDist >= 0.0f );

	TSphereData oSphere;
	oSphere.SetRadius( a_fRadius );

	TMatrix44 mFrom = TMatrix44::IDENTITY;
	mFrom.SetTranslation( a_rvFrom );

	TVector4 vOffset;
	vOffset.Multiply( a_rvDir, a_rfDist );
	TVector4 vTo( a_rvFrom.x + vOffset.x, a_rvFrom.y + vOffset.y, a_rvFrom.z + vOffset.z, a_rvFrom.w );

	TPhysicsShapeContact oContact;
	TBOOL                bHit = TPhysicsManager::GetSingleton()->SweepShapeClosest( &oSphere, mFrom, vTo, a_uiCollisionLayer, oContact, TNULL );

	if ( bHit )
	{
		a_rvOutNormal = oContact.m_vNormal;
		a_rfDist      = TVector4( a_rvFrom.x - oContact.m_vPosition.x, a_rvFrom.y - oContact.m_vPosition.y, a_rvFrom.z - oContact.m_vPosition.z, a_rvFrom.w ).Magnitude();
	}

	return bHit;
}

// $deBlob: FUNCTION 00583670
TBOOL ABlobFollowCamera::AvoidCeiling( const TVector4& a_rvTarget, const TVector4& a_rvBlobPos, TVector4& a_rvPos, TFLOAT* a_pfTimer, TFLOAT a_fMinDist, TFLOAT a_fIdealDist, TFLOAT a_fMaxDist, TFLOAT a_fRadius, TFLOAT a_fCollisionRadius )
{
	const TFLOAT fAngle = 1.8064158f;
	TVector4     vNormal;

	if ( *a_pfTimer <= 0.0f )
	{
		// Anything above the blob? (-Y is up)
		TFLOAT fDist = a_fRadius + 5.0f;

		TVector4 vDir;
		vDir.x = a_rvPos.x - a_rvBlobPos.x;
		vDir.y = a_rvPos.y - a_rvBlobPos.y;
		vDir.z = a_rvPos.z - a_rvBlobPos.z;
		vDir.w = a_rvPos.w;
		vDir.Normalise();

		TVector4 vDirXZ = vDir;
		vDirXZ.NormaliseXZ();

		TFLOAT fSin = TMath::Sin( vDirXZ.GetAngleBetweenVector( vDir ) );

		if ( fDist < fSin * a_fMinDist )
			fDist = fSin * a_fMinDist;

		if ( SphereCast( a_rvBlobPos, TVector4::VEC_NEGY, vNormal, fDist, a_fCollisionRadius, 0x10 ) )
			*a_pfTimer = 1.5f;
		else if ( *a_pfTimer <= 0.0f )
			return TFALSE;
	}

	// Low behind the blob, under the ceiling
	TVector4 vDir;
	vDir.x = a_rvPos.x - a_rvTarget.x;
	vDir.y = a_rvPos.y - a_rvTarget.y;
	vDir.z = a_rvPos.z - a_rvTarget.z;
	vDir.w = a_rvPos.w;

	TVector4 vAxis;
	vAxis.CrossProduct( TVector4::VEC_POSY, vDir );
	vAxis.Normalise();

	TQuaternion qRotation;
	qRotation.SetRotation( vAxis.AsVector3(), fAngle );

	TVector4 vRotated = TVector4::VEC_POSY;
	TQuaternion::RotateVector( vRotated, qRotation, vRotated );
	vRotated.Normalise();

	TFLOAT fDist = a_fRadius + 5.0f;
	SphereCast( a_rvBlobPos, vRotated, vNormal, fDist, a_fCollisionRadius, 0x10 );

	a_rvPos = a_rvBlobPos;
	TVector4 vOffset;
	vOffset.Multiply( vRotated, fDist );
	a_rvPos.x += vOffset.x;
	a_rvPos.y += vOffset.y;
	a_rvPos.z += vOffset.z;

	return TTRUE;
}

// Moves a_rvPos towards a_rvTarget along a Catmull-Rom curve (a_rvVelocity is the tangent at a_rvPos, a_fDeltaTime *
// a_fSpeed the step along the curve) and sets a_rvVelocity to the velocity of the move
// $deBlob: FUNCTION 00583890
void MoveAlongSpline( TFLOAT a_fDeltaTime, TVector4& a_rvPos, TVector4& a_rvVelocity, const TVector4& a_rvTarget, TFLOAT a_fSpeed )
{
	if ( 0.0f >= a_fDeltaTime )
		return;

	TVector4 vDir = a_rvTarget;
	vDir.x        = vDir.x - a_rvPos.x;
	vDir.y        = vDir.y - a_rvPos.y;
	vDir.z        = vDir.z - a_rvPos.z;
	TFLOAT fDist  = vDir.NormaliseMag();

	TVector4 vTangent = a_rvVelocity;
	TFLOAT   fSpeed   = vTangent.NormaliseMag();

	if ( !( 0.0f < fSpeed ) )
		vTangent = vDir;

	TVector4 vOffset;
	TVector4 vP0 = a_rvPos;
	vOffset.Multiply( vTangent, -fDist );
	vP0.x = vOffset.x + vP0.x;
	vP0.y = vOffset.y + vP0.y;
	vP0.z = vOffset.z + vP0.z;

	TVector4 vP3 = a_rvTarget;
	vOffset.Multiply( vDir, fDist );
	vP3.x = vOffset.x + vP3.x;
	vP3.y = vOffset.y + vP3.y;
	vP3.z = vOffset.z + vP3.z;

	TFLOAT fT = a_fDeltaTime * a_fSpeed;
	TMath::Clip( fT, 0.0f, 1.0f );

	TVector4 vNewPos;
	ASSCameraFunction::CatmullRom( vNewPos, vP0, a_rvPos, a_rvTarget, vP3, fT );

	a_rvVelocity   = vNewPos;
	a_rvVelocity.x = a_rvVelocity.x - a_rvPos.x;
	a_rvVelocity.y = a_rvVelocity.y - a_rvPos.y;
	a_rvVelocity.z = a_rvVelocity.z - a_rvPos.z;
	a_rvVelocity.Multiply( a_rvVelocity.NormaliseMag() / a_fDeltaTime );

	a_rvPos = vNewPos;
}

// $deBlob: FUNCTION 00583a60
void ABlobFollowCamera::FindOpenDirection( const TVector4& a_rvTarget, const TVector4& a_rvFacing, TVector4& a_rvPos, TFLOAT a_fMinDist, TFLOAT a_fIdealDist, TFLOAT a_fMaxDist, TFLOAT a_fCollisionRadius )
{
	const TFLOAT fStep = TMath::TWO_PI * 0.0625f;

	TVector4 vPCDir;
	vPCDir.x = a_rvPos.x - a_rvTarget.x;
	vPCDir.y = a_rvPos.y - a_rvTarget.y;
	vPCDir.z = a_rvPos.z - a_rvTarget.z;
	vPCDir.w = a_rvPos.w;
	vPCDir.NormaliseMag();

	TASSERT( vPCDir.y > -1.0f && vPCDir.y < 1.0f );

	// 16 directions alternating right/left of the current one: the first two open beyond a_fMinDist (1, 2) and the
	// first two open beyond a_fIdealDist (3, 4)
	TBOOL    bDoneMin = TFALSE;
	TBOOL    bFound1  = TFALSE;
	TBOOL    bFound2  = TFALSE;
	TBOOL    bFound3  = TFALSE;
	TBOOL    bFound4  = TFALSE;
	TVector4 vDir1;
	TVector4 vDir2;
	TVector4 vDir3;
	TVector4 vDir4;
	TFLOAT   fDist1 = 0.0f;
	TFLOAT   fDist2 = 0.0f;
	TFLOAT   fDist3 = 0.0f;
	TFLOAT   fDist4 = 0.0f;
	TINT     iStep  = 0;

	for ( TINT i = 0; i < 16; i++ )
	{
		TVector4 vDir  = vPCDir;
		TFLOAT   fDist = a_fMaxDist + a_fCollisionRadius;

		if ( i % 2 != 0 )
		{
			vDir.RotateY( iStep * fStep );
		}
		else
		{
			vDir.RotateY( -( iStep * fStep ) );
			iStep++;
		}

		TVector4 vNormal;
		SphereCast( a_rvTarget, vDir, vNormal, fDist, a_fCollisionRadius, 0x10 );

		if ( !bDoneMin )
		{
			if ( fDist <= a_fMinDist + a_fCollisionRadius )
			{
				if ( bFound1 )
					bDoneMin = TTRUE;
			}
			else if ( !bFound1 )
			{
				vDir1   = vDir;
				fDist1  = fDist;
				bFound1 = TTRUE;
			}
			else
			{
				vDir2    = vDir;
				fDist2   = fDist;
				bFound2  = TTRUE;
				bDoneMin = TTRUE;
			}
		}

		if ( fDist <= a_fIdealDist + a_fCollisionRadius )
		{
			if ( bFound3 )
			{
				TASSERT( bFound1 );
				break;
			}
		}
		else if ( bFound3 )
		{
			vDir4   = vDir;
			fDist4  = fDist;
			bFound4 = TTRUE;
			break;
		}
		else
		{
			vDir3   = vDir;
			fDist3  = fDist;
			bFound3 = TTRUE;
		}
	}

	if ( !bFound1 )
		return;

	// Prefer the directions behind the blob
	TVector4 vIdealDir  = vDir3;
	TFLOAT   fIdealDist = fDist3;

	if ( bFound3 && bFound4 && a_rvFacing.x * vDir3.x + a_rvFacing.z * vDir3.z > a_rvFacing.z * vDir4.z + a_rvFacing.x * vDir4.x )
	{
		vIdealDir  = vDir4;
		fIdealDist = fDist4;
	}

	TVector4 vBest = vDir1;
	TFLOAT   fBest = fDist1;

	if ( bFound2 && vDir1.x * a_rvFacing.x + vDir1.z * a_rvFacing.z > vDir2.x * a_rvFacing.x + vDir2.z * a_rvFacing.z )
	{
		vBest = vDir2;
		fBest = fDist2;
	}

	if ( bFound3 && a_rvFacing.z * vIdealDir.z + a_rvFacing.x * vIdealDir.x < 0.0f )
	{
		vBest = vIdealDir;
		fBest = fIdealDist;
	}

	a_rvPos = a_rvTarget;
	TVector4 vOffset;
	vOffset.Multiply( vBest, fBest );
	a_rvPos.x += vOffset.x;
	a_rvPos.y += vOffset.y;
	a_rvPos.z += vOffset.z;
}

// $deBlob: FUNCTION 00584260
void ABlobFollowCamera::AvoidSideWalls( TFLOAT a_fDeltaTime, const TVector4& a_rvTarget, TVector4& a_rvPos, TFLOAT a_fMaxDist, TFLOAT a_fIdealDist, TFLOAT a_fCollisionRadius, TBOOL a_bSnap, void* a_pPhantom1, void* a_pPhantom2 )
{
	// sfCamAccepatableIdealDist < sfCamIdealDist
	TASSERT( ms_fAcceptableIdealDist < ms_fDefaultIdealDist );

	TFLOAT fStep = 0.3926991f;

	TVector4 vPCDir;
	vPCDir.x     = a_rvPos.x - a_rvTarget.x;
	vPCDir.y     = a_rvPos.y - a_rvTarget.y;
	vPCDir.z     = a_rvPos.z - a_rvTarget.z;
	vPCDir.w     = a_rvPos.w;
	TFLOAT fDist = vPCDir.NormaliseMag();

	TASSERT( vPCDir.y > -1.0f && vPCDir.y < 1.0f );

	TFLOAT fDistLeft  = a_fMaxDist;
	TFLOAT fDistRight = a_fMaxDist;

	if ( ms_fAcceptableIdealDist <= fDist )
		fDist = ms_fAcceptableIdealDist;

	TVector4 vNormal;
	TVector4 vDir = vPCDir;
	vDir.RotateY( fStep );
	CastRay( a_rvTarget, vDir, vNormal, fDistLeft, 0x10, TNULL );

	vDir            = vPCDir;
	TFLOAT fNegStep = -fStep;
	vDir.RotateY( fNegStep );
	CastRay( a_rvTarget, vDir, vNormal, fDistRight, 0x10, TNULL );

	// Only one side blocked?
	if ( ( fDistLeft < fDist || fDistRight < fDist ) && ( fDist <= fDistLeft || fDist <= fDistRight ) )
	{
		TFLOAT fCheckAngle = fStep * 2.0f;

		if ( fDistLeft < fDistRight )
		{
			fCheckAngle = -fCheckAngle;
			fDistLeft   = fDistRight;
			fStep       = fNegStep;
		}

		vDir = vPCDir;
		vDir.RotateY( fCheckAngle );
		TFLOAT fCheckDist = a_fMaxDist;
		CastRay( a_rvTarget, vDir, vNormal, fCheckDist, 0x10, TNULL );

		if ( fDist <= fCheckDist )
		{
			vDir = vPCDir;
			vDir.RotateY( fStep );

			TVector4 vNewPos = a_rvTarget;
			TVector4 vOffset;
			vOffset.Multiply( vDir, fDistLeft );
			vNewPos.x += vOffset.x;
			vNewPos.y += vOffset.y;
			vNewPos.z += vOffset.z;

			if ( a_bSnap )
			{
				a_rvPos = vNewPos;
				return;
			}

			TVector4 vMove;
			vMove.x          = vNewPos.x - a_rvPos.x;
			vMove.y          = vNewPos.y - a_rvPos.y;
			vMove.z          = vNewPos.z - a_rvPos.z;
			vMove.w          = vNewPos.w;
			TFLOAT fMoveDist = vMove.NormaliseMag();

			TFLOAT fFactor = fMoveDist * 0.5f;
			TMath::Clip( fFactor, 0.0f, 1.0f );

			TFLOAT fMove = a_fDeltaTime * 16.0f * fFactor;
			TMath::Clip( fMove, 0.0f, fMoveDist );

			vOffset.Multiply( vMove, fMove );
			a_rvPos.x += vOffset.x;
			a_rvPos.y += vOffset.y;
			a_rvPos.z += vOffset.z;
		}
	}
}

// $deBlob: FUNCTION 00586fb0
void ABlobFollowCamera::SlideAroundObstacle( TVector4& a_rvPos, const TVector4& a_rvFrom, const TVector4& a_rvIdealPos, const TVector4& a_rvTarget, TFLOAT a_fCollisionRadius )
{
	TVector4 vDir;
	vDir.x       = a_rvPos.x - a_rvFrom.x;
	vDir.y       = a_rvPos.y - a_rvFrom.y;
	vDir.z       = a_rvPos.z - a_rvFrom.z;
	vDir.w       = a_rvPos.w;
	TFLOAT fDist = vDir.NormaliseMag();

	TVector4 vNormal;
	TFLOAT   fFullDist = fDist;

	if ( !( 0.0f < fDist ) || !CastRay( a_rvFrom, vDir, vNormal, fDist, 0x10, TNULL ) )
		return;

	// Blocked: the first of 9 points from the ideal position towards the target that can be reached
	TVector4 vLine;
	vLine.x      = a_rvTarget.x - a_rvIdealPos.x;
	vLine.y      = a_rvTarget.y - a_rvIdealPos.y;
	vLine.z      = a_rvTarget.z - a_rvIdealPos.z;
	vLine.w      = a_rvTarget.w;
	TFLOAT fStep = vLine.NormaliseMag() / 10.0f;

	TVector4 vToPoint;
	TFLOAT   fPointDist;

	for ( TINT i = 0;; )
	{
		TVector4 vOffset;
		vOffset.Multiply( vLine, i * fStep );
		vToPoint.x = ( vOffset.x + a_rvIdealPos.x ) - a_rvFrom.x;
		vToPoint.y = ( vOffset.y + a_rvIdealPos.y ) - a_rvFrom.y;
		vToPoint.z = ( vOffset.z + a_rvIdealPos.z ) - a_rvFrom.z;
		vToPoint.w = a_rvIdealPos.w;
		fPointDist = vToPoint.NormaliseMag();

		if ( !CastRay( a_rvFrom, vToPoint, vNormal, fPointDist, 0x10, TNULL ) )
			break;

		if ( 8 < ++i )
			return;
	}

	// As far as before towards it, the rest towards the ideal position
	TFLOAT fRest = 0.0f;
	TFLOAT fMove = fFullDist;

	if ( fPointDist < fFullDist )
	{
		fRest = fFullDist - fPointDist;
		fMove = fPointDist;
	}

	a_rvPos = a_rvFrom;
	TVector4 vOffset;
	vOffset.Multiply( vToPoint, fMove );
	a_rvPos.x += vOffset.x;
	a_rvPos.y += vOffset.y;
	a_rvPos.z += vOffset.z;

	if ( 0.0f < fRest )
	{
		TVector4 vToIdeal;
		vToIdeal.x        = a_rvIdealPos.x - a_rvPos.x;
		vToIdeal.y        = a_rvIdealPos.y - a_rvPos.y;
		vToIdeal.z        = a_rvIdealPos.z - a_rvPos.z;
		vToIdeal.w        = a_rvIdealPos.w;
		TFLOAT fIdealDist = vToIdeal.NormaliseMag();
		TMath::Clip( fRest, 0.0f, fIdealDist );

		vOffset.Multiply( vToIdeal, fRest );
		a_rvPos.x += vOffset.x;
		a_rvPos.y += vOffset.y;
		a_rvPos.z += vOffset.z;
	}
}

// $deBlob: FUNCTION 00587250
TBOOL ABlobFollowCamera::CastRay( const TVector4& a_rvFrom, const TVector4& a_rvDir, TVector4& a_rvOutNormal, TFLOAT& a_rfDist, TUINT32 a_uiCollisionLayer, TPhysicsPhantomContainer* a_pPhantom )
{
	TASSERT( a_rvDir.IsNormalised() );
	TASSERT( a_rfDist >= 0.0f );

	TVector4 vOffset;
	vOffset.Multiply( a_rvDir, a_rfDist );

	TPhysicsRay oRay;
	oRay.m_vFrom            = a_rvFrom;
	oRay.m_vTo              = TVector4( a_rvFrom.x + vOffset.x, a_rvFrom.y + vOffset.y, a_rvFrom.z + vOffset.z, a_rvFrom.w );
	oRay.m_uiCollisionLayer = a_uiCollisionLayer;

	TPhysicsShapeContact oContact;
	TBOOL                bHit = TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, a_pPhantom );

	if ( bHit )
	{
		a_rvOutNormal = oContact.m_vNormal;
		a_rfDist      = TVector4( a_rvFrom.x - oContact.m_vPosition.x, a_rvFrom.y - oContact.m_vPosition.y, a_rvFrom.z - oContact.m_vPosition.z, a_rvFrom.w ).Magnitude();
	}

	return bHit;
}

// $deBlob: FUNCTION 005873e0
void ABlobFollowCamera::UpdateRegionDistances( TFLOAT a_fDeltaTime, ANedCameraFunction* a_pRegions, const TVector4& a_rvTarget, TFLOAT a_fRadius, TFLOAT* a_pfMinDist, TFLOAT* a_pfIdealDist, TFLOAT* a_pfMaxDist, TFLOAT* a_pfTimer, TUINT32* a_puiRegionData, void** a_ppRegion, TBOOL a_bReset )
{
	ANedCameraFunction::Region* pCurrent = a_pRegions->FindRegionAt( a_rvTarget, ANedCameraFunction::RS_ENABLED );

	TFLOAT fMinDist   = ms_fDefaultMinDist + a_fRadius;
	TFLOAT fIdealDist = ms_fDefaultIdealDist + a_fRadius;
	TFLOAT fMaxDist   = ms_fDefaultMaxDist + a_fRadius;
	*a_puiRegionData  = 0;

	if ( pCurrent != TNULL )
	{
		*a_puiRegionData = pCurrent->m_uiData;
		TASSERT( pCurrent->m_fCameraMinDist <= pCurrent->m_fCameraIdealDist );
		TASSERT( pCurrent->m_fCameraIdealDist <= pCurrent->m_fCameraMaxDist );

		if ( pCurrent->m_fCameraMaxDist < 0.0f || pCurrent->m_fCameraMinDist < 0.0f || pCurrent->m_fCameraIdealDist < 0.0f ||
		     pCurrent->m_fCameraIdealDist < pCurrent->m_fCameraMinDist || pCurrent->m_fCameraMaxDist < pCurrent->m_fCameraIdealDist )
		{
			pCurrent = TNULL;
		}
		else
		{
			fMinDist   = pCurrent->m_fCameraMinDist + a_fRadius;
			fIdealDist = pCurrent->m_fCameraIdealDist + a_fRadius;
			fMaxDist   = pCurrent->m_fCameraMaxDist + a_fRadius;
		}
	}

	if ( a_bReset )
	{
		*a_ppRegion    = pCurrent;
		*a_pfMaxDist   = fMaxDist;
		*a_pfMinDist   = fMinDist;
		*a_pfIdealDist = fIdealDist;
		*a_pfTimer     = 0.0f;
		return;
	}

	TFLOAT fTimer = *a_pfTimer;
	*a_pfTimer    = fTimer - a_fDeltaTime;

	ANedCameraFunction::Region* pOld     = TSTATICCAST( ANedCameraFunction::Region, *a_ppRegion );
	TBOOL                       bKeepOld = TFALSE;

	if ( pOld != TNULL )
	{
		if ( pOld == pCurrent )
		{
			*a_pfTimer = 0.5f;
			bKeepOld   = TTRUE;
		}
		else if ( fTimer - a_fDeltaTime > 0.0f )
		{
			fMinDist   = pOld->m_fCameraMinDist + a_fRadius;
			fIdealDist = pOld->m_fCameraIdealDist + a_fRadius;
			fMaxDist   = pOld->m_fCameraMaxDist + a_fRadius;
			bKeepOld   = TTRUE;
		}
	}

	if ( !bKeepOld )
		*a_ppRegion = pCurrent;

	ANedCameraFunction::Region* pRegion = TSTATICCAST( ANedCameraFunction::Region, *a_ppRegion );

	if ( pRegion != TNULL && ( pRegion->m_uiFlags & ANedCameraFunction::RS_BLEND ) == 0 )
	{
		*a_pfMinDist   = fMinDist;
		*a_pfIdealDist = fIdealDist;
		*a_pfMaxDist   = fMaxDist;
		return;
	}

	TFLOAT fBlend = a_fDeltaTime * 3.0f;
	TMath::Clip( fBlend, 0.0f, 1.0f );
	*a_pfMinDist   = TMath::LERP( *a_pfMinDist, fMinDist, fBlend );
	*a_pfIdealDist = TMath::LERP( *a_pfIdealDist, fIdealDist, fBlend );
	*a_pfMaxDist   = TMath::LERP( *a_pfMaxDist, fMaxDist, fBlend );
	TMath::Clip( *a_pfMinDist, 0.0f, *a_pfIdealDist );
	TMath::Clip( *a_pfIdealDist, *a_pfMinDist, *a_pfMaxDist );
	TMath::Clip( *a_pfMaxDist, *a_pfIdealDist, TMath::MAXFLOAT );
}

// Moves a_rvFrom by a_fDist towards a_rvTo on a straight line (a_rvTo if it's closer)
static TFLOAT ABlobFollowCamera_MoveStraight( const TVector4& a_rvFrom, const TVector4& a_rvTo, const TVector4& a_rvDir, TVector4& a_rvOut, TFLOAT a_fDist, TFLOAT a_fTotalDist )
{
	if ( a_fDist <= a_fTotalDist )
	{
		a_rvOut = a_rvFrom;
		TVector4 vStep;
		vStep.Multiply( a_rvDir, a_fDist );
		a_rvOut.x += vStep.x;
		a_rvOut.y += vStep.y;
		a_rvOut.z += vStep.z;
		return a_fTotalDist;
	}

	a_rvOut = a_rvTo;
	return a_fTotalDist;
}

// Moves a_rvFrom by a_fDist towards a_rvTo, around a_rvTarget when the line passes close to it; returns the length
// of the path
// $deBlob: FUNCTION 00587f60
TFLOAT ABlobFollowCamera::MoveTowards( const TVector4& a_rvTarget, const TVector4& a_rvFrom, const TVector4& a_rvTo, TVector4& a_rvOut, TFLOAT a_fDist )
{
	a_rvOut = a_rvFrom;

	TVector4 vDir;
	vDir.x            = a_rvTo.x - a_rvFrom.x;
	vDir.y            = a_rvTo.y - a_rvFrom.y;
	vDir.z            = a_rvTo.z - a_rvFrom.z;
	vDir.w            = a_rvTo.w;
	TFLOAT fTotalDist = vDir.NormaliseMag();

	if ( fTotalDist <= TMath::MINFLOAT )
		return 0.0f;

	static TFLOAT s_fNearDistSq = 49.0f;

	if ( TVector4::DistanceSq( a_rvTarget, a_rvFrom ) < s_fNearDistSq )
		return ABlobFollowCamera_MoveStraight( a_rvFrom, a_rvTo, vDir, a_rvOut, a_fDist, fTotalDist );

	TVector4 vDirXZ  = vDir;
	TFLOAT   fDistXZ = vDirXZ.MagnitudeXZ();
	vDirXZ.NormaliseXZ();

	if ( fDistXZ <= TMath::MINFLOAT )
		return ABlobFollowCamera_MoveStraight( a_rvFrom, a_rvTo, vDir, a_rvOut, a_fDist, fTotalDist );

	// Plane containing the path that faces the vertical line through the target
	TVector4 vSide;
	vSide.CrossProduct( TVector4::VEC_NEGY, vDir );
	TVector4 vPlaneNormal;
	vPlaneNormal.CrossProduct( vSide, vDir );
	vPlaneNormal.Normalise();

	TPlane   oPlane( vPlaneNormal.AsVector3(), a_rvFrom.x * vPlaneNormal.x + a_rvFrom.y * vPlaneNormal.y + a_rvFrom.z * vPlaneNormal.z );
	TVector3 vLineStart( a_rvTarget.x, a_rvTarget.y - 999.0f, a_rvTarget.z );
	TVector3 vLineEnd( a_rvTarget.x, a_rvTarget.y + 999.0f, a_rvTarget.z );
	TVector3 vIntersection;

	if ( !TMath::LineIntersectsPlane3D( oPlane, vLineStart, vLineEnd, vIntersection ) )
		return ABlobFollowCamera_MoveStraight( a_rvFrom, a_rvTo, vDir, a_rvOut, a_fDist, fTotalDist );

	// vCentre: the point on the plane below/above the target; the path goes around it at the distance of the closer end
	TVector4 vCentre( vIntersection.x, vIntersection.y, vIntersection.z, 1.0f );
	TFLOAT   fFromDistSq = TVector4::DistanceSq( vCentre, a_rvFrom );
	TFLOAT   fToDistSq   = TVector4::DistanceSq( vCentre, a_rvTo );
	TBOOL    bFromIsNear = fFromDistSq <= fToDistSq;

	TVector4 vNear = bFromIsNear ? a_rvFrom : a_rvTo;
	TVector4 vFar  = bFromIsNear ? a_rvTo : a_rvFrom;

	TVector4 vNearDir( vNear.x - vCentre.x, vNear.y - vCentre.y, vNear.z - vCentre.z, vNear.w );
	TVector4 vFarDir( vFar.x - vCentre.x, vFar.y - vCentre.y, vFar.z - vCentre.z, vFar.w );

	TFLOAT fRadius  = vNearDir.NormaliseMag();
	TFLOAT fFarDist = vFarDir.NormaliseMag();
	TMath::Clip( fFarDist, fRadius, TMath::MAXFLOAT );

	TFLOAT fTangentDist = TFLOAT( sqrt( TDOUBLE( fFarDist * fFarDist - fRadius * fRadius ) ) );
	TMath::Clip( fTangentDist, 0.0f, fFarDist );

	TVector4 vAxis;
	vAxis.CrossProduct( vNearDir, vFarDir );
	vAxis.Normalise();

	TVector4 vTangentPoint = vNear;
	TFLOAT   fArcLength    = 0.0f;
	TFLOAT   fArcAngle     = 0.0f;
	TFLOAT   fPathLength   = fTotalDist;
	TVector4 vStepDir;

	if ( fFarDist > 0.0f )
	{
		TFLOAT fCANgle = TMath::ASin( fTangentDist / fFarDist );
		TASSERT( fCANgle >= 0.0f );

		if ( vNearDir.GetAngleBetweenVector( vFarDir ) > fCANgle )
		{
			// Tangent point of the line from the far end to the circle
			TQuaternion qRotation;
			qRotation.SetRotation( vAxis.AsVector3(), -fCANgle );
			vStepDir = vFarDir;
			TQuaternion::RotateVector( vStepDir, qRotation, vStepDir );
			vStepDir.Normalise();

			vTangentPoint = vCentre;
			TVector4 vOffset;
			vOffset.Multiply( vStepDir, fRadius );
			vTangentPoint.x += vOffset.x;
			vTangentPoint.y += vOffset.y;
			vTangentPoint.z += vOffset.z;

			TFLOAT fDistTemp = TVector4::Distance( vTangentPoint, vFar );
			TFLOAT fLDist    = fTangentDist;
			TASSERT( TMath::Abs( fDistTemp - fLDist ) < 0.1f );

			fPathLength = fTangentDist;
			fArcAngle   = vNearDir.GetAngleBetweenVector( vStepDir );
			fArcLength  = fArcAngle * fRadius;
		}
	}

	fPathLength += fArcLength;

	TFLOAT fTraveldistance = a_fDist;

	if ( !bFromIsNear )
		fTraveldistance = fPathLength - a_fDist;

	TASSERT( fTraveldistance >= 0.0f );
	TMath::Clip( fTraveldistance, 0.0f, TMath::MAXFLOAT );

	TFLOAT fStep;

	if ( fTraveldistance < fArcLength )
	{
		if ( fArcLength <= 0.0f )
		{
			a_rvOut = vNear;
			return fPathLength;
		}

		// On the arc around the centre
		TQuaternion qRotation;
		qRotation.SetRotation( vAxis.AsVector3(), ( fTraveldistance * fArcAngle ) / fArcLength );
		vStepDir = vNearDir;
		TQuaternion::RotateVector( vStepDir, qRotation, vStepDir );
		vStepDir.Normalise();

		a_rvOut = vCentre;
		fStep   = fRadius;
	}
	else
	{
		// On the straight line from the tangent point to the far end
		fTraveldistance -= fArcLength;
		vStepDir.x = vFar.x - vTangentPoint.x;
		vStepDir.y = vFar.y - vTangentPoint.y;
		vStepDir.z = vFar.z - vTangentPoint.z;
		vStepDir.w = vFar.w;

		TFLOAT fDist = vStepDir.NormaliseMag();
		TASSERT( fTraveldistance <= ( fDist + .1f ) );

		a_rvOut = vTangentPoint;
		fStep   = fTraveldistance;
	}

	TVector4 vOffset;
	vOffset.Multiply( vStepDir, fStep );
	a_rvOut.x += vOffset.x;
	a_rvOut.y += vOffset.y;
	a_rvOut.z += vOffset.z;
	return fPathLength;
}

// $deBlob: FUNCTION 00587cc0
TFLOAT ABlobFollowCamera::GetDampenedTravelDistance( TFLOAT a_fSecs, TFLOAT a_fAccel, TFLOAT a_fMaxSpeed, TFLOAT a_fDampeningDist, TFLOAT a_fDist, TFLOAT* a_pfCurrentSpeed )
{
	TASSERT( a_fDampeningDist > 0.0f );

	if ( 0.0f < a_fSecs )
	{
		if ( TMath::MINFLOAT < a_fDist )
		{
			TFLOAT fRequiredSpeed = ( a_fMaxSpeed * a_fDist ) / a_fDampeningDist;
			TASSERT( fRequiredSpeed > 0.0f );

			if ( *a_pfCurrentSpeed < fRequiredSpeed )
			{
				*a_pfCurrentSpeed = a_fSecs * a_fAccel + *a_pfCurrentSpeed;
				TMath::Clip( *a_pfCurrentSpeed, 0.0f, fRequiredSpeed );
			}

			TFLOAT fCurrentSpeed = *a_pfCurrentSpeed;
			TFLOAT fAcc          = -( ( fCurrentSpeed * fCurrentSpeed ) / ( a_fDist * 2.0f ) );
			TASSERT( *a_pfCurrentSpeed > 0.0f );

			TFLOAT fTime = a_fSecs;
			TMath::Clip( fTime, 0.0f, ( a_fDist * 2.0f ) / *a_pfCurrentSpeed );

			TFLOAT fTravelDist = fAcc * 0.5f * fTime * fTime + *a_pfCurrentSpeed * fTime;

			if ( a_fDist < fTravelDist )
			{
				*a_pfCurrentSpeed = 0.0f;
				return a_fDist;
			}

			TASSERT( a_fDist - fTravelDist >= 0.0f );

			if ( ( ( a_fDist - fTravelDist ) * a_fMaxSpeed ) / a_fDampeningDist < *a_pfCurrentSpeed )
			{
				TASSERT( fAcc <= 0.0f );
				*a_pfCurrentSpeed = fTime * fAcc + *a_pfCurrentSpeed;
			}

			return fTravelDist;
		}

		*a_pfCurrentSpeed = 0.0f;
	}

	return 0.0f;
}

// $deBlob: FUNCTION 005876f0
TFLOAT ABlobFollowCamera::GetTravelDistance( TFLOAT a_fSecs, TFLOAT a_fAccel, TFLOAT a_fMaxSpeed, TFLOAT a_fDampeningDist, TFLOAT a_fDist, TFLOAT* a_pfCurrentSpeed )
{
	TMath::Clip( *a_pfCurrentSpeed, 0.0f, a_fMaxSpeed );

	if ( a_fSecs <= 0.0f )
		return 0.0f;

	TFLOAT fVi = *a_pfCurrentSpeed;

	if ( a_fDist <= a_fDampeningDist )
		return GetDampenedTravelDistance( a_fSecs, a_fAccel, a_fMaxSpeed, a_fDampeningDist, a_fDist, a_pfCurrentSpeed );

	TFLOAT fTotalAllowedDistance = a_fDist - a_fDampeningDist;
	TFLOAT fVf                   = a_fSecs * a_fAccel + fVi;
	TMath::Clip( fVf, 0.0f, a_fMaxSpeed );

	TFLOAT fAccTime      = ( fVf - fVi ) / a_fAccel;
	TFLOAT fTimeBalance1 = a_fSecs - fAccTime;
	TASSERT( fTimeBalance1 >= -0.0001f );
	TMath::Clip( fTimeBalance1, 0.0f, 99999.99f );

	TFLOAT fHalfAccel = a_fAccel * 0.5f;
	TFLOAT fAccDist   = fHalfAccel * fAccTime * fAccTime + fAccTime * fVi;

	if ( fTotalAllowedDistance < fAccDist )
	{
		fVf = TMath::Sqrt( a_fAccel * 2.0f * fTotalAllowedDistance + fVi * fVi );
		TASSERT( ( fVf + fVi ) > 0.0f );

		TFLOAT ft = fAccTime;
		if ( TMath::MINFLOAT < fVf + fVi )
			ft = ( fTotalAllowedDistance * 2.0f ) / ( fVf + fVi );

		TFLOAT fTravelDist = fHalfAccel * ft * ft + ft * fVi;
		TASSERT( ft <= a_fSecs );
		TASSERT( TMath::Abs( fTravelDist - fTotalAllowedDistance ) < 0.01f );
		TASSERT( TMath::Abs( ( a_fDist - fTotalAllowedDistance ) - a_fDampeningDist ) < 0.0001f );

		*a_pfCurrentSpeed = fVf;
		TFLOAT fResult    = GetDampenedTravelDistance( a_fSecs - ft, a_fAccel, a_fMaxSpeed, a_fDampeningDist, a_fDampeningDist, a_pfCurrentSpeed ) + fTotalAllowedDistance;
		TMath::Clip( fResult, 0.0f, a_fDist );
		return fResult;
	}

	TFLOAT fBalanceDist   = a_fDist - fAccDist;
	fTotalAllowedDistance = fBalanceDist - a_fDampeningDist;
	TASSERT( fTotalAllowedDistance >= 0.0f );

	TFLOAT fTravelDist = fTimeBalance1 * fVf;

	if ( fTravelDist <= fTotalAllowedDistance )
	{
		*a_pfCurrentSpeed = fVf;
		return fTravelDist + fAccDist;
	}

	TASSERT( fVf > 0.0f );
	TFLOAT ft           = fTotalAllowedDistance / fVf;
	TFLOAT fTravelDist2 = ft * fVf;
	TASSERT( ft <= fTimeBalance1 );
	TASSERT( TMath::Abs( fTravelDist2 - fTotalAllowedDistance ) < 0.001f );

	*a_pfCurrentSpeed = fVf;
	return fTotalAllowedDistance + fAccDist + GetDampenedTravelDistance( fTimeBalance1 - ft, a_fAccel, a_fMaxSpeed, a_fDampeningDist, fBalanceDist - fTotalAllowedDistance, a_pfCurrentSpeed );
}

// $deBlob: FUNCTION 00583f00
void ABlobFollowCamera::SmoothPosition( TFLOAT a_fDeltaTime, const TVector4& a_rvCamPos, TVector4& a_rvSmoothPos, const TVector4& a_rvTarget, const TVector4& a_rvIdealPos, TVector4& a_rvVelocity, TFLOAT a_fAccel, TFLOAT a_fMaxSpeed, TFLOAT a_fDampenDist, TFLOAT a_fRadius, TBOOL a_bCheckCollision )
{
	TVector4 vHit   = a_rvSmoothPos;
	TFLOAT   fSpeed = a_rvVelocity.Magnitude();

	TVector4 vNewPos;
	TFLOAT   fPathDist = MoveTowards( a_rvTarget, a_rvSmoothPos, a_rvIdealPos, vNewPos, 0.0f );
	TFLOAT   fStep     = GetTravelDistance( a_fDeltaTime, a_fAccel, a_fMaxSpeed, a_fDampenDist, fPathDist, &fSpeed );
	TMath::Clip( fStep, 0.0f, fPathDist );
	MoveTowards( a_rvTarget, a_rvSmoothPos, a_rvIdealPos, vNewPos, fStep );

	a_rvVelocity = vNewPos;
	a_rvVelocity.x -= a_rvSmoothPos.x;
	a_rvVelocity.y -= a_rvSmoothPos.y;
	a_rvVelocity.z -= a_rvSmoothPos.z;
	a_rvVelocity.Normalise();
	a_rvVelocity.Multiply( fSpeed );

	a_rvSmoothPos = vNewPos;

	TVector4 vFromTarget;
	vFromTarget.x      = vNewPos.x - a_rvTarget.x;
	vFromTarget.y      = vNewPos.y - a_rvTarget.y;
	vFromTarget.z      = vNewPos.z - a_rvTarget.z;
	vFromTarget.w      = vNewPos.w;
	TFLOAT fSmoothDist = vFromTarget.NormaliseMag();

	TVector4 vIdealDir;
	vIdealDir.x       = a_rvIdealPos.x - a_rvTarget.x;
	vIdealDir.y       = a_rvIdealPos.y - a_rvTarget.y;
	vIdealDir.z       = a_rvIdealPos.z - a_rvTarget.z;
	vIdealDir.w       = a_rvIdealPos.w;
	TFLOAT fIdealDist = vIdealDir.NormaliseMag();
	TFLOAT fAngle     = vFromTarget.GetAngleBetweenVector( vIdealDir );

	if ( !a_bCheckCollision )
	{
		TFLOAT fDist = fSmoothDist;
		SphereCast( a_rvTarget, vFromTarget, vHit, fDist, a_fRadius, 0x10 );

		if ( fDist <= 0.1f )
			fDist = a_fRadius;

		a_rvSmoothPos = a_rvTarget;
		TVector4 vOffset;
		vOffset.Multiply( vFromTarget, fDist );
		a_rvSmoothPos.x += vOffset.x;
		a_rvSmoothPos.y += vOffset.y;
		a_rvSmoothPos.z += vOffset.z;
		return;
	}

	TVector4 vFromIdeal;
	vFromIdeal.x = a_rvSmoothPos.x - a_rvIdealPos.x;
	vFromIdeal.y = a_rvSmoothPos.y - a_rvIdealPos.y;
	vFromIdeal.z = a_rvSmoothPos.z - a_rvIdealPos.z;
	vFromIdeal.w = a_rvSmoothPos.w;
	TFLOAT fDist = vFromIdeal.NormaliseMag();

	if ( fDist < a_fRadius )
		return;

	if ( fAngle < 0.15707964f )
	{
		if ( fIdealDist < fSmoothDist )
		{
			TBOOL bHit = SphereCast( a_rvIdealPos, vFromIdeal, vHit, fDist, a_fRadius, 0x10 );

			if ( !bHit )
				return;

			a_rvSmoothPos = a_rvIdealPos;
			TVector4 vOffset;
			vOffset.Multiply( vFromIdeal, fDist );
			a_rvSmoothPos.x += vOffset.x;
			a_rvSmoothPos.y += vOffset.y;
			a_rvSmoothPos.z += vOffset.z;
			a_rvVelocity = TVector4::VEC_ZERO;
			return;
		}

		return;
	}

	// NOTE: vHit still holds the smoothed position from before this update here (retail shares the slot)
	SlideAroundObstacle( a_rvSmoothPos, vHit, a_rvIdealPos, a_rvTarget, ms_fCollisionRadius );
}

// $deBlob: FUNCTION 0058c610
TUINT32 ABlobFollowCamera::GetRegionsForCategory( TINT a_eCategory )
{
	switch ( a_eCategory )
	{
		case 0:
			return 0x1E7E;
		case 1:
			return 1;
		default:
			return 0;
	}
}

// Searches the cameras of the category a_uiFlag at a_rvAt
static void ABlobFollowCamera_SearchCategory( TUINT32 a_uiFlag, const TVector4& a_rvAt, const TVector4& a_rvViewPos, ANedCameraFunction* a_pNed, TBOOL a_bSpline, TUINT32* a_puiSearched, TUINT32* a_puiFlag, ANedCameraFunction::FixedCam** a_ppFixedCam, ANedCameraFunction::SplineCam** a_ppSplineCam )
{
	*a_puiFlag = a_uiFlag;
	*a_puiSearched |= a_uiFlag;

	if ( a_bSpline )
		*a_ppSplineCam = a_pNed->FindSplineCamAt( a_rvAt, a_uiFlag );
	else
		*a_ppFixedCam = a_pNed->FindFixedCamAt( a_rvAt, a_rvViewPos, a_uiFlag );
}

// $deBlob: FUNCTION 00584c30
void ABlobFollowCamera::SearchCameras( TINT a_eCategory, ABlobEntity* a_pBlob, ANedCameraFunction* a_pNed, const TVector4& a_rvPos, const TVector4& a_rvViewPos, TUINT32* a_puiSearched, TUINT32* a_puiFlag, ANedCameraFunction::FixedCam** a_ppFixedCam, ANedCameraFunction::SplineCam** a_ppSplineCam, TBOOL a_bSpline )
{

	*a_ppFixedCam  = TNULL;
	*a_ppSplineCam = TNULL;
	*a_puiSearched = 0;
	*a_puiFlag     = ANedCameraFunction::RS_ENABLED;


	if ( a_eCategory != 0 )
	{
		if ( a_eCategory == 1 && *a_ppFixedCam == TNULL && *a_ppSplineCam == TNULL )
			ABlobFollowCamera_SearchCategory( 1, a_rvPos, a_rvViewPos, a_pNed, a_bSpline, a_puiSearched, a_puiFlag, a_ppFixedCam, a_ppSplineCam );
	}
	else
	{
		*a_puiSearched |= 0xC;

		if ( *a_ppSplineCam == TNULL && ( a_pBlob->GetBlobFlags() & 0x10000000 ) )
			ABlobFollowCamera_SearchCategory( 0x800, a_rvPos, a_rvViewPos, a_pNed, a_bSpline, a_puiSearched, a_puiFlag, a_ppFixedCam, a_ppSplineCam );

		if ( *a_ppFixedCam != TNULL )
			return;

		// A help message is shown
		if ( *a_ppSplineCam == TNULL && ( ( APropTriggerHelp::IsHelpShown() && !APropTriggerHelp::IsHelpBlocking() && !APropTriggerHelp::IsHelpHudShown() ) || APropTriggerHelp::IsUnk9B720C() ) )
			ABlobFollowCamera_SearchCategory( 0x400, a_rvPos, a_rvViewPos, a_pNed, a_bSpline, a_puiSearched, a_puiFlag, a_ppFixedCam, a_ppSplineCam );

		if ( *a_ppFixedCam != TNULL )
			return;

		if ( *a_ppSplineCam == TNULL )
		{
			// A challenge with its camera: the cameras around the challenge's position
			AChallengeManager* pChallengeManager = AChallengeManager::GetSingleton();

			if ( pChallengeManager->IsWaitingForPrompt() && pChallengeManager->GetNearbyChallenge() != TNULL )
			{
				*a_puiFlag = 0x200;
				*a_puiSearched |= 0x200;

				TVector4 vPos = a_rvPos;

				// Around the challenge's marker
				AChallenge* pChallenge = pChallengeManager->GetNearbyChallenge();

				if ( pChallenge != TNULL )
				{
					ABaseEntity* pRevolutionary = TSTATICCAST( ABaseEntity, pChallenge->GetRevolutionary() );

					if ( pRevolutionary != TNULL && pChallenge->GetData() != TNULL && ( pChallenge->GetData()->m_uiFlags & AChallenge::Data::Flags_HasCamera ) )
						vPos = pRevolutionary->GetMatrix().GetTranslation();
				}

				if ( !a_bSpline )
					*a_ppFixedCam = a_pNed->FindFixedCamAt( vPos, a_rvViewPos, *a_puiFlag );
				else
					*a_ppSplineCam = a_pNed->FindSplineCamAt( vPos, *a_puiFlag );
			}
		}

		if ( *a_ppFixedCam != TNULL )
			return;

		if ( *a_ppSplineCam == TNULL )
		{
			*a_puiFlag = 0x40;
			*a_puiSearched |= 0x40;

			if ( !a_bSpline )
				*a_ppFixedCam = TSTATICCAST( ANedCameraFunction::FixedCam, APlayerManager::GetSingleton()->GetVistaCamera() );
		}

		if ( *a_ppFixedCam != TNULL )
			return;

		if ( *a_ppSplineCam == TNULL )
		{
			TINT iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( a_pBlob );

			if ( APlayerManager::GetSingleton()->IsPlayerUnk60( iPlayer ) )
			{
				*a_puiFlag = 0x40;
				*a_puiSearched |= 0x40;

				if ( !a_bSpline )
					*a_ppFixedCam = TREINTERPRETCAST( ANedCameraFunction::FixedCam*, APlayerManager::GetSingleton()->GetBlobData( TUINT8( iPlayer ) )->m_uiUnk64 );
			}
		}

		if ( *a_ppFixedCam != TNULL )
			return;

		// The landmark of the current region
		if ( *a_ppSplineCam == TNULL )
		{
			*a_puiFlag = ANedCameraFunction::RS_LANDMARK;
			*a_puiSearched |= ANedCameraFunction::RS_LANDMARK;

			if ( !a_bSpline )
			{
				*a_ppFixedCam = a_pNed->FindCurrentFixedCam( a_rvPos, a_rvViewPos );

				if ( *a_ppFixedCam != TNULL )
				{
					TASSERT( ( *a_ppFixedCam )->m_iFLAGS & ANedCameraFunction::RS_LANDMARK );
					( *a_ppFixedCam )->m_iFLAGS |= ANedCameraFunction::RS_LANDMARK;
					return;
				}
			}
		}

		if ( *a_ppSplineCam == TNULL )
		{
			if ( ABuildingManager::GetSingleton()->IsBlobOnLandmark() )
				ABlobFollowCamera_SearchCategory( 2, a_rvPos, a_rvViewPos, a_pNed, a_bSpline, a_puiSearched, a_puiFlag, a_ppFixedCam, a_ppSplineCam );
		}

		if ( *a_ppFixedCam == TNULL )
		{
			if ( *a_ppSplineCam == TNULL && ( a_pBlob->GetBlobState() == ABlobEntity::STATE_CANNON || ( a_pBlob->GetBlobFlags() & 0x1000000 ) ) )
				ABlobFollowCamera_SearchCategory( 0x1000, a_rvPos, a_rvViewPos, a_pNed, a_bSpline, a_puiSearched, a_puiFlag, a_ppFixedCam, a_ppSplineCam );

			if ( *a_ppFixedCam == TNULL && *a_ppSplineCam == TNULL )
			{
				ABlobZJumpComponent* pZJump = a_pBlob->GetZJumpComponent();
				TINT                 eState = pZJump->GetState();

				if ( eState == ABlobZJumpComponent::STATE_POINT || eState == ABlobZJumpComponent::STATE_AIMNODE || eState == ABlobZJumpComponent::STATE_AIM || eState == ABlobZJumpComponent::STATE_NODEFLIGHT )
				{
					TBOOL bUnk120 = pZJump->IsUnk120();

					if ( !bUnk120 && *a_ppFixedCam == TNULL && *a_ppSplineCam == TNULL )
						ABlobFollowCamera_SearchCategory( 0x10, a_rvPos, a_rvViewPos, a_pNed, a_bSpline, a_puiSearched, a_puiFlag, a_ppFixedCam, a_ppSplineCam );

					if ( bUnk120 )
					{
						if ( *a_ppFixedCam != TNULL )
							return;

						if ( *a_ppSplineCam == TNULL )
							ABlobFollowCamera_SearchCategory( 0x20, a_rvPos, a_rvViewPos, a_pNed, a_bSpline, a_puiSearched, a_puiFlag, a_ppFixedCam, a_ppSplineCam );
					}
				}
			}
		}
	}

	if ( *a_ppFixedCam == TNULL && *a_ppSplineCam == TNULL )
		*a_puiFlag = ANedCameraFunction::RS_ENABLED;
}

// $deBlob: FUNCTION 00585070
TBOOL ABlobFollowCamera::UpdateNedFixedCam( TFLOAT a_fDeltaTime, TINT a_eCategory, ABlobEntity* a_pBlob, ANedCameraFunction* a_pNed, const TVector4& a_rvPos, const TVector4& a_rvViewPos, TVector4& a_rvOutPos, TQuaternion& a_rqOutOrient, TFLOAT* a_pfFOV, TFLOAT* a_pfFOVSpeed, ANedCameraFunction::FixedCam** a_ppLast, ANedCameraFunction::FixedCam** a_ppCurrent, ANedCameraFunction::FixedCam** a_ppPending, TFLOAT* a_pfTimer, TUINT32* a_puiFlags )
{
	TUINT32                        uiFlag              = ANedCameraFunction::RS_ENABLED;
	TUINT32                        iFlagRegionSearched = 0;
	ANedCameraFunction::FixedCam*  pFixedCam           = TNULL;
	ANedCameraFunction::SplineCam* pSplineCam          = TNULL;
	SearchCameras( a_eCategory, a_pBlob, a_pNed, a_rvPos, a_rvViewPos, &iFlagRegionSearched, &uiFlag, &pFixedCam, &pSplineCam, TFALSE );

	TASSERT( ( iFlagRegionSearched | GetRegionsForCategory( a_eCategory ) ) == GetRegionsForCategory( a_eCategory ) );

	if ( pFixedCam == TNULL && ( *a_ppCurrent == TNULL || ( GetRegionsForCategory( a_eCategory ) & *a_puiFlags ) == 0 ) )
		return TFALSE;

	TBOOL                         bSwitch  = TTRUE;
	ANedCameraFunction::FixedCam* pCurrent = *a_ppCurrent;

	if ( pCurrent == TNULL )
	{
		if ( pFixedCam == TNULL )
		{
			*a_ppPending = TNULL;
			*a_pfTimer   = 0.0f;
			bSwitch      = TFALSE;
		}
		else if ( *a_ppPending != pFixedCam )
		{
			*a_ppPending = pFixedCam;
			*a_pfTimer   = pFixedCam->m_fStartDelay;
		}
	}
	else if ( pFixedCam != TNULL )
	{
		if ( pFixedCam == pCurrent )
		{
			// Keep it
			*a_pfTimer   = pFixedCam->m_fHoldTime;
			*a_ppPending = pFixedCam;
			bSwitch      = TFALSE;
		}
		else if ( *a_ppPending != pFixedCam )
		{
			*a_ppPending = pFixedCam;
			*a_pfTimer   = pFixedCam->m_fStartDelay;
		}
	}
	else
	{
		if ( ( pCurrent->m_iFLAGS & ANedCameraFunction::RS_UNK20000 ) && ( pCurrent->m_iFLAGS & iFlagRegionSearched ) )
		{
			// It stays while its category was searched
			*a_pfTimer   = pCurrent->m_fHoldTime;
			*a_ppPending = *a_ppCurrent;
			bSwitch      = TFALSE;
		}
		else
		{
			*a_ppPending = TNULL;

			if ( ( pCurrent->m_iFLAGS & 0x30 ) && !( iFlagRegionSearched & 0x30 ) )
				*a_pfTimer = 0.0f;

			if ( ( pCurrent->m_iFLAGS & 0x1000 ) && !( iFlagRegionSearched & 0x1000 ) )
				*a_pfTimer = 0.0f;
		}
	}

	if ( bSwitch )
	{
		*a_pfTimer -= a_fDeltaTime;

		if ( *a_pfTimer <= 0.0f )
		{
			ANedCameraFunction::FixedCam* pOld = *a_ppCurrent;

			if ( pOld != TNULL && pOld != *a_ppPending )
				pOld->m_iNumUses++;

			*a_ppLast    = *a_ppCurrent;
			*a_ppCurrent = *a_ppPending;

			if ( *a_ppCurrent == TNULL )
				return TFALSE;

			*a_pfTimer  = ( *a_ppCurrent )->m_fHoldTime;
			*a_puiFlags = uiFlag;
		}
	}

	if ( *a_ppCurrent != TNULL && ( GetRegionsForCategory( a_eCategory ) & *a_puiFlags ) )
	{
		a_rvOutPos    = ( *a_ppCurrent )->m_vPosition;
		a_rqOutOrient = ( *a_ppCurrent )->m_qOrientation;
		*a_pfFOV      = ( *a_ppCurrent )->m_fFOV;
		*a_pfFOVSpeed = ( *a_ppCurrent )->m_fFOVSpeed;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00585290
TBOOL ABlobFollowCamera::UpdateSplineCam( TFLOAT a_fDeltaTime, TINT a_eCategory, ABlobEntity* a_pBlob, ANedCameraFunction* a_pNed, ASSCameraFunction* a_pSSCams, const TVector4& a_rvTarget, const TVector4& a_rvViewPos, TVector4& a_rvOutPos, TVector4* a_pvFollowPos, TQuaternion* a_pqOutOrient, TFLOAT* a_pfFOV, TFLOAT* a_pfFOVSpeed, ANedCameraFunction::SplineCam** a_ppLast, ANedCameraFunction::SplineCam** a_ppCurrent, ANedCameraFunction::SplineCam** a_ppPending, TFLOAT* a_pfTimer, TUINT32* a_puiFlags )
{
	TUINT32                        uiFlag              = ANedCameraFunction::RS_ENABLED;
	TUINT32                        iFlagRegionSearched = 0;
	ANedCameraFunction::FixedCam*  pFixedCam           = TNULL;
	ANedCameraFunction::SplineCam* pSplineCam          = TNULL;
	SearchCameras( a_eCategory, a_pBlob, a_pNed, a_rvTarget, a_rvViewPos, &iFlagRegionSearched, &uiFlag, &pFixedCam, &pSplineCam, TTRUE );

	TASSERT( ( iFlagRegionSearched | GetRegionsForCategory( a_eCategory ) ) == GetRegionsForCategory( a_eCategory ) );

	if ( pSplineCam == TNULL && ( *a_ppCurrent == TNULL || ( GetRegionsForCategory( a_eCategory ) & *a_puiFlags ) == 0 ) )
		return TFALSE;

	TBOOL                          bSwitch  = TTRUE;
	ANedCameraFunction::SplineCam* pCurrent = *a_ppCurrent;

	if ( pCurrent != TNULL )
	{
		if ( pSplineCam != TNULL )
		{
			if ( pSplineCam == pCurrent )
			{
				*a_pfTimer   = pSplineCam->m_fHoldTime;
				*a_ppPending = pSplineCam;
				bSwitch      = TFALSE;
			}
			else if ( *a_ppPending != pSplineCam )
			{
				*a_ppPending = pSplineCam;
				*a_pfTimer   = pSplineCam->m_fStartDelay;
			}
		}
		else if ( ( pCurrent->m_uiFlags & ANedCameraFunction::RS_UNK20000 ) && ( pCurrent->m_uiFlags & iFlagRegionSearched ) )
		{
			*a_pfTimer   = pCurrent->m_fHoldTime;
			*a_ppPending = *a_ppCurrent;
			bSwitch      = TFALSE;
		}
		else
		{
			*a_ppPending = TNULL;
		}
	}
	else if ( pSplineCam == TNULL )
	{
		bSwitch = TFALSE;
	}
	else if ( *a_ppPending != pSplineCam )
	{
		*a_ppPending   = pSplineCam;
		*a_pfTimer     = pSplineCam->m_fStartDelay;
		*a_pvFollowPos = a_rvTarget;
	}

	if ( bSwitch )
	{
		*a_pfTimer -= a_fDeltaTime;

		if ( *a_pfTimer <= 0.0f )
		{
			*a_ppLast    = *a_ppCurrent;
			*a_ppCurrent = *a_ppPending;

			if ( *a_ppCurrent == TNULL )
				return TFALSE;

			*a_pfTimer  = ( *a_ppCurrent )->m_fHoldTime;
			*a_puiFlags = uiFlag;
		}
	}

	if ( *a_ppCurrent != TNULL && ( GetRegionsForCategory( a_eCategory ) & *a_puiFlags ) )
	{
		TFLOAT fBlend = ( *a_ppCurrent )->m_fBlendSpeed * a_fDeltaTime;
		TMath::Clip( fBlend, 0.0f, 1.0f );
		a_pvFollowPos->Lerp4( *a_pvFollowPos, a_rvTarget, fBlend );

		const ASSCameraFunction::Camera* pCam = a_pSSCams->FindCamera( ( *a_ppCurrent )->m_iSSCam );
		*a_pfFOVSpeed                         = ( *a_ppCurrent )->m_fFOVSpeed;

		ANedCameraFunction::SplineCam* pSpline = *a_ppCurrent;
		ASSCameraFunction::GetCameraTransform( pCam, pCam->m_vLineStart, pCam->m_vLineEnd, *a_pvFollowPos, a_rvViewPos, a_rvOutPos, *a_pqOutOrient, *a_pfFOV, pSpline->m_fRadius, pSpline->m_fMinOffset, pSpline->m_fOffsetAngle, 0.0f, pSpline->m_uiFlags );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00584600
TBOOL ABlobFollowCamera::UpdateChallengeCam( TFLOAT a_fDeltaTime, ABlobEntity* a_pBlob, TVector4* a_pvTarget, TVector4& a_rvOutPos, TFLOAT* a_pfFOV, TFLOAT* a_pfFOVSpeed )
{
	AChallengeManager* pChallengeManager = AChallengeManager::GetSingleton();

	if ( !pChallengeManager->IsWaitingForPrompt() || pChallengeManager->GetNearbyChallenge() == TNULL )
		return TFALSE;

	AChallenge*      pChallenge = pChallengeManager->GetNearbyChallenge();
	AChallenge::Type eType      = pChallenge->GetType();

	if ( eType == AChallenge::Type_Null )
	{
		// NOTE: 0x38 is past the end of AChallenge::Data as far as its known fields go (the size is unknown)
		switch ( *TREINTERPRETCAST( TINT*, TREINTERPRETCAST( TUINT8*, pChallenge->GetData() ) + 0x38 ) )
		{
			case 0: eType = AChallenge::Type_Race; break;
			case 1: eType = AChallenge::Type_Paint; break;
			case 2: eType = AChallenge::Type_Landmark; break;
			case 3: eType = AChallenge::Type_Combat; break;
			default: break;
		}
	}

	// Types 1-6 have their own camera settings, everything else uses the default entry 0
	TINT iType     = TINT( eType );
	TINT iSettings = ( iType >= 1 && iType <= 6 ) ? iType : 0;

	const ABlobEntity::CommonProperties::ChallengeCamSettings& rSettings = ABlobEntity::GetCommonProperties()->m_aChallengeCams[ iSettings ];

	// The challenge's marker is the target; the camera looks 1.5 below it
	*a_pvTarget = pChallenge->GetData()->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );
	a_pvTarget->y -= 1.5f;

	*a_pfFOV      = rSettings.m_fFOV * 0.017453292f;
	*a_pfFOVSpeed = rSettings.m_fFOVSpeed;

	TFLOAT fAngle   = rSettings.m_fAngle * 0.017453292f;
	TFLOAT fMinDist = rSettings.m_fMinDist;

	// The flat (XZ) direction from the marker to Blob
#ifdef DEBLOB_COMMUNITY_PATCH
	TVector4 vBlobPos = a_pBlob->GetRenderPosition();
#else
	TVector4 vBlobPos = a_pBlob->GetBodyPosition();
#endif
	TVector4 vDir( vBlobPos.x - a_pvTarget->x, vBlobPos.y - a_pvTarget->y, vBlobPos.z - a_pvTarget->z, vBlobPos.w );
	vDir.NormaliseXZ();

	// The camera distance: the distance to Blob over cos( angle ), at least the minimum
	TFLOAT fDist    = TVector4::Distance( vBlobPos, *a_pvTarget );
	TFLOAT fCamDist = fMinDist;

	if ( fDist > 0.0f )
	{
		fCamDist = fDist / TFLOAT( cos( static_cast<double>( fAngle ) ) ) + 0.5f;

		if ( fMinDist > fCamDist )
			fCamDist = fMinDist;
	}

	// The direction turned around the vertical axis by the angle
	TVector4 vCross;
	vCross.CrossProduct( vDir, TVector4( 0.0f, -1.0f, 0.0f, 1.0f ) );

	TQuaternion qRot;
	qRot.SetRotation( TVector3( vCross.x, vCross.y, vCross.z ), fAngle );
	TQuaternion::RotateVector( vDir, qRot, vDir );
	vDir.Normalise();

	// The camera stops at the first wall on the way out (CastRay clamps fCamDist)
	TVector4 vOffset;
	CastRay( *a_pvTarget, vDir, vOffset, fCamDist, 16, TNULL );
	vOffset.Multiply( vDir, fCamDist );

	a_rvOutPos = *a_pvTarget;
	a_rvOutPos.x += vOffset.x;
	a_rvOutPos.y += vOffset.y;
	a_rvOutPos.z += vOffset.z;

	return TTRUE;
}

// $deBlob: FUNCTION 005849d0
TBOOL ABlobFollowCamera::UpdateAnimationCam( ABlobEntity* a_pBlob, TVector4& a_rvPos, const TVector4& a_rvTarget, TQuaternion* a_pqOrient, TFLOAT* a_pfFOV, TFLOAT* a_pfFOVSpeed, TBOOL* a_pbSnap )
{
	ABlobAnimationComponent* pAnimation = a_pBlob->GetAnimationComponent();

	if ( ( pAnimation->GetCharModelInstance()->GetFlags() & AModelInstance2::FLAG_Visible ) == 0 )
		return TFALSE;

	ABlobAnimationComponent::Anim* pAnim = pAnimation->GetCurrentAnim();

	if ( pAnim == TNULL || ( pAnim->m_uiFlags & 2 ) == 0 )
		return TFALSE;

	TMatrix44 mMatrix;
	pAnimation->GetCharModelInstance()->GetBoneTransform( ABlobAnimationComponent::ms_iBoneSBBody, mMatrix );
	TVector4 vLookAt = mMatrix.GetTranslation();

	// Above, behind
	TVector4 vDir = a_rvPos - a_rvTarget;
	vDir.w        = a_rvPos.w;
	vDir.NormaliseXZ();

	a_rvPos = a_rvTarget;
	TVector4 vOffset;
	vOffset.Multiply( vDir, ms_fAnimCamDist );
	a_rvPos.x += vOffset.x;
	a_rvPos.z += vOffset.z;
	a_rvPos.y = ( a_rvPos.y + vOffset.y ) - ms_fAnimCamHeight;

	vDir         = a_rvPos - a_rvTarget;
	vDir.w       = a_rvPos.w;
	TFLOAT fDist = vDir.NormaliseMag();

	TBOOL bHit = CastRay( a_rvTarget, vDir, vOffset, fDist, 0x10, TNULL );

	a_rvPos = a_rvTarget;
	vOffset.Multiply( vDir, fDist );
	a_rvPos.x += vOffset.x;
	a_rvPos.y += vOffset.y;
	a_rvPos.z += vOffset.z;

	mMatrix.Identity();
	mMatrix.GetTranslation() = a_rvPos;

	if ( !bHit )
	{
		vLookAt   = a_rvTarget;
		vLookAt.y = a_rvTarget.y - ms_fAnimCamLookDown;
	}

	mMatrix.LookAtTarget( vLookAt, mMatrix.GetTranslation() );
	TMatrix44::MatToQuat( *a_pqOrient, mMatrix );
	*a_pqOrient /= TMath::Sqrt( a_pqOrient->x * a_pqOrient->x + a_pqOrient->y * a_pqOrient->y + a_pqOrient->z * a_pqOrient->z + a_pqOrient->w * a_pqOrient->w );
	return TTRUE;
}

// $deBlob: FUNCTION 00585510
TBOOL ABlobFollowCamera::UpdateManualCam( TFLOAT a_fDeltaTime, TVector4& a_rvPos, const TVector4& a_rvCamPos, const TVector4& a_rvSmoothPos, const TVector4& a_rvTarget, const TVector4& a_rvFacing, TVector4* a_pvLastTarget, TBOOL* a_pbActive, TUINT32 a_uiInput, TUINT32* a_puiLastInput, TFLOAT a_fMinDist, TFLOAT a_fCollisionRadius, TFLOAT a_fMaxDist, TFLOAT* a_pfTurnSpeed, TFLOAT* a_pfBlendTime )
{
	TVector4 vNormal;

#ifdef DEBLOB_COMMUNITY_PATCH
	// CamUp and CamDown (4, 8) pitch the camera too; retail ignores them, so the mouse's vertical motion did nothing
	if ( ( a_uiInput & 0xF ) == 0 )
#else
	if ( ( a_uiInput & 1 ) == 0 && ( a_uiInput & 2 ) == 0 )
#endif
	{
		if ( a_uiInput & 0x80 )
		{
			// Behind the blob, looking down by the liquid pitch
			TVector4 vAxis;
			vAxis.CrossProduct( a_rvFacing, TVector4::VEC_POSY );
			vAxis.Normalise();

			TVector4    vDir = TVector4::VEC_POSY;
			TQuaternion qRotation;
			qRotation.SetRotation( vAxis.AsVector3(), ms_fLiquidPitch );
			TQuaternion::RotateVector( vDir, qRotation, vDir );
			vDir.Normalise();

			TFLOAT fDist = a_fMaxDist;
			CastRay( a_rvTarget, vDir, vNormal, fDist, 0x10, TNULL );
			TMath::Clip( fDist, a_fMinDist, a_fMaxDist );

			a_rvPos = a_rvTarget;
			TVector4 vOffset;
			vOffset.Multiply( vDir, fDist );
			a_rvPos.x += vOffset.x;
			a_rvPos.y += vOffset.y;
			a_rvPos.z += vOffset.z;

			*a_pfBlendTime  = 2.0f;
			*a_pvLastTarget = a_rvTarget;
			*a_pbActive     = TTRUE;
			*a_puiLastInput = a_uiInput;
			return TTRUE;
		}
	}
	else
	{
		TVector4 vDir = a_rvPos - a_rvTarget;
		vDir.w        = a_rvPos.w;

		TFLOAT fAngle  = vDir.GetAngleBetweenVector( sm_vWorldUp );
		*a_pfBlendTime = 0.5f;

		if ( 0.0f < fAngle && fAngle < TMath::PI )
		{
			// Turning around the target
#ifdef DEBLOB_COMMUNITY_PATCH
			if ( a_uiInput & 3 )
			{
#endif
			TFLOAT fMin;
			TFLOAT fMax;
			TFLOAT fSpeed;

			if ( ( a_uiInput & 1 ) == 0 )
			{
				fMax   = 0.0f;
				fMin   = -1.0f;
				fSpeed = *a_pfTurnSpeed - a_fDeltaTime * 5.0f;
			}
			else
			{
				fMax   = 1.0f;
				fMin   = 0.0f;
				fSpeed = a_fDeltaTime * 5.0f + *a_pfTurnSpeed;
			}

			*a_pfTurnSpeed = fSpeed;
			TMath::Clip( *a_pfTurnSpeed, fMin, fMax );

			vDir.RotateY( *a_pfTurnSpeed * TMath::PI * a_fDeltaTime * ms_fInvertY * ms_fCamSpeedOption );
#ifdef DEBLOB_COMMUNITY_PATCH
			}

			if ( ( a_uiInput & 0xC ) != 0 && ( a_uiInput & 0xC ) != 0xC )
				PitchManualCam( vDir, fAngle, ( a_uiInput & 4 ) ? -a_fDeltaTime : a_fDeltaTime );
#endif
			vDir.NormaliseMag();

			TFLOAT fDist = a_fMaxDist;
			TMath::Clip( fDist, 0.0f, a_fMaxDist );
			CastRay( a_rvTarget, vDir, vNormal, fDist, 0x10, TNULL );
			fDist -= a_fCollisionRadius;
			TMath::Clip( fDist, a_fMinDist, a_fMaxDist );

			a_rvPos = a_rvTarget;
			TVector4 vOffset;
			vOffset.Multiply( vDir, fDist );
			a_rvPos.x += vOffset.x;
			a_rvPos.y += vOffset.y;
			a_rvPos.z += vOffset.z;

			*a_pvLastTarget = a_rvTarget;
			*a_pbActive     = TTRUE;
			*a_puiLastInput = a_uiInput;
			return TTRUE;
		}
	}

	if ( !*a_pbActive )
	{
		*a_pfTurnSpeed  = TMath::LERP( *a_pfTurnSpeed, 0.0f, a_fDeltaTime * 3.0f );
		*a_puiLastInput = a_uiInput;
		return TFALSE;
	}

	// Kept until the target moves away or something blocks the view
	if ( 100.0f < TVector4::DistanceSq( *a_pvLastTarget, a_rvTarget ) )
	{
		*a_pbActive     = TFALSE;
		*a_puiLastInput = a_uiInput;
		return *a_pbActive;
	}

	TVector4 vDir = a_rvTarget - a_rvPos;
	vDir.w        = a_rvTarget.w;
	TFLOAT fDist  = vDir.NormaliseMag();

	if ( CastRay( a_rvPos, vDir, vNormal, fDist, 0x10, TNULL ) )
	{
		*a_pbActive     = TFALSE;
		*a_puiLastInput = a_uiInput;
	}

	return *a_pbActive;
}

#ifdef DEBLOB_COMMUNITY_PATCH
// Pitches the manual camera's direction from the target: CamUp (a negative time) raises the camera, CamDown lowers it
// (the inverted camera option swaps them), at PI / 2 per second, between 20 and 95 degrees from the world's up
void ABlobFollowCamera::PitchManualCam( TVector4& a_rvDir, TFLOAT a_fAngle, TFLOAT a_fDeltaTime )
{
	const TFLOAT fMinAngle = TMath::PI * ( 20.0f / 180.0f );
	const TFLOAT fMaxAngle = TMath::PI * ( 95.0f / 180.0f );

	TFLOAT fLength = a_rvDir.Magnitude();

	TVector4 vFlat = a_rvDir;
	vFlat.y        = 0.0f;
	TFLOAT fFlat   = vFlat.Magnitude();

	if ( fLength <= 0.0f || fFlat <= 0.0001f )
		return;

	TFLOAT fDelta = a_fDeltaTime * TMath::PI * 0.5f * ms_fInvertY * ms_fCamSpeedOption;
	TFLOAT fAngle = a_fAngle + fDelta;

	// Don't snap a camera that is already outside the limits, only stop it going further
	if ( fDelta < 0.0f )
		fAngle = TMath::Max( fAngle, TMath::Min( a_fAngle, fMinAngle ) );
	else
		fAngle = TMath::Min( fAngle, TMath::Max( a_fAngle, fMaxAngle ) );

	TFLOAT fSin = TMath::Sin( fAngle ) * fLength / fFlat;
	TFLOAT fCos = TMath::Cos( fAngle ) * fLength;

	a_rvDir.x = vFlat.x * fSin + sm_vWorldUp.x * fCos;
	a_rvDir.y = sm_vWorldUp.y * fCos;
	a_rvDir.z = vFlat.z * fSin + sm_vWorldUp.z * fCos;
}
#endif

// $deBlob: FUNCTION 00585920
TBOOL ABlobFollowCamera::UpdateUnderCam( ABlobEntity* a_pBlob, TVector4& a_rvPos, const TVector4& a_rvCamPos, const TVector4& a_rvSmoothPos, const TVector4& a_rvTarget, const TVector4& a_rvFacing, TFLOAT a_fRadius, TFLOAT a_fCollisionRadius )
{
	if ( !ms_bUnderCam || ( a_pBlob->GetBlobFlags() & 0x40 ) == 0 )
		return TFALSE;

	TVector4 vDir = a_rvSmoothPos - a_rvTarget;
	vDir.w        = a_rvSmoothPos.w;
	vDir.NormaliseXZ();

	a_rvPos = a_rvTarget;
	TVector4 vOffset;
	vOffset.Multiply( vDir, 15.0f );
	a_rvPos.x += vOffset.x;
	a_rvPos.z += vOffset.z;
	a_rvPos.y = ( a_rvPos.y + vOffset.y ) - 130.0f;

	TVector4 vToPos = a_rvPos - a_rvTarget;
	vToPos.w        = a_rvPos.w;
	TFLOAT fDist    = vToPos.NormaliseMag();

	CastRay( a_rvTarget, vToPos, vOffset, fDist, 0x10, TNULL );

	a_rvPos = a_rvTarget;
	vOffset.Multiply( vToPos, fDist );
	a_rvPos.x += vOffset.x;
	a_rvPos.y += vOffset.y;
	a_rvPos.z += vOffset.z;
	return TTRUE;
}

// $deBlob: FUNCTION 00588980
TBOOL ABlobFollowCamera::ToggleUnderCam( void* a_pUnused )
{
	ms_bUnderCam = !ms_bUnderCam;
	return TTRUE;
}

// $deBlob: FUNCTION 00585a90
TBOOL ABlobFollowCamera::UpdateCloseCam( ABlobEntity* a_pBlob, const TVector4& a_rvTarget, TVector4& a_rvPos )
{
	if ( a_pBlob->m_bInBlobVision == 0 )
		return TFALSE;

	TVector4 vDir = a_rvPos - a_rvTarget;
	vDir.w        = a_rvPos.w;
	vDir.NormaliseXZ();

	a_rvPos = a_rvTarget;
	TVector4 vOffset;
	vOffset.Multiply( vDir, 0.1f );
	a_rvPos.x += vOffset.x;
	a_rvPos.z += vOffset.z;
	a_rvPos.y = ( a_rvPos.y + vOffset.y ) - 0.5f;
	return TTRUE;
}

// $deBlob: FUNCTION 00585b50
TBOOL ABlobFollowCamera::UpdateBusyCam( ABlobEntity* a_pBlob, const TVector4& a_rvTarget, TVector4& a_rvPos, POS_HANDLE a_iPosType )
{
	TINT eState = a_pBlob->m_eState;

	if ( ( ( !AChallengeManager::GetSingleton()->IsEnding() && ( a_pBlob->GetBlobFlags() & 0x10000000 ) == 0 ) || eState != ABlobEntity::STATE_ROLL ) &&
	     ( 0.0f < AGameStatus::GetSingleton()->GetTimeLeft() || ( eState != ABlobEntity::STATE_FAIL && eState != ABlobEntity::STATE_VICTORY ) ) )
	{
		if ( 0 < TINT( APlayerManager::GetSingleton()->GetBlobDataOf( a_pBlob )->m_uiLives ) )
			return TFALSE;
	}

	if ( a_pBlob->GetBlobFlags() & 0x10000000 )
	{
		ACamera* pCamera = a_pBlob->GetViewCamera();

		if ( ACameraManager::GetSingleton()->GetNedCameras().FindFixedCamAt( a_rvTarget, pCamera->GetMatrix().GetTranslation(), 0x800 ) != TNULL )
			return TFALSE;
	}

	if ( a_iPosType != POS_HANDLE_2 )
	{
		TVector4 vFacing = a_pBlob->m_vFacing;
		FindOpenDirection( a_rvTarget, vFacing, a_rvPos, ms_fDefaultIdealDist, ms_fDefaultIdealDist, ms_fDefaultMaxDist, ms_fCollisionRadius );
	}

	TFLOAT fDist = ( TFLOAT( a_pBlob->m_iSize ) / 9.0f + 1.0f ) * 10.0f;

	TVector4 vDir = a_rvPos - a_rvTarget;
	vDir.w        = a_rvPos.w;
	vDir.NormaliseXZ();

	TVector4 vNormal;
	CastRay( a_rvTarget, vDir, vNormal, fDist, 0x10, TNULL );

	a_rvPos = a_rvTarget;
	TVector4 vOffset;
	vOffset.Multiply( vDir, fDist );
	a_rvPos.x += vOffset.x;
	a_rvPos.y += vOffset.y;
	a_rvPos.z += vOffset.z;
	return TTRUE;
}

// $deBlob: FUNCTION 0058c540
static TFLOAT GetFOVBlendFraction( const TVector4& a_rvFrom, const TVector4& a_rvTo, const TVector4& a_rvAt )
{
	TFLOAT   fFraction = 1.0f;
	TVector4 vDir( a_rvTo.x - a_rvFrom.x, a_rvTo.y - a_rvFrom.y, a_rvTo.z - a_rvFrom.z, a_rvTo.w );
	TFLOAT   fLength = vDir.NormaliseMag();

	if ( 0.0001f < fLength )
	{
		fFraction = ( ( a_rvAt.x - a_rvFrom.x ) * vDir.x + ( a_rvAt.y - a_rvFrom.y ) * vDir.y + ( a_rvAt.z - a_rvFrom.z ) * vDir.z ) / fLength;
		TMath::Clip( fFraction, 0.0f, 1.0f );
	}

	return fFraction;
}

#ifdef DEBLOB_COMMUNITY_PATCH

// The liquid surface point under the drawn position a_rvRenderPos: the hit's height at the drawn position's X/Z (the
// hit comes from a vertical ray cast at the start of each fixed step, so its X/Z moved in 60 Hz steps)
static TVector4 GetLiquidViewTarget( ABlobEntity* a_pBlob, const TVector4& a_rvRenderPos )
{
	TVector4 vTarget = a_pBlob->GetLiquidHitPosition();
	vTarget.x        = a_rvRenderPos.x;
	vTarget.z        = a_rvRenderPos.z;
	return vTarget;
}

#endif // DEBLOB_COMMUNITY_PATCH

// $deBlob: FUNCTION 00588b50
void ABlobFollowCamera::Update( TFLOAT a_fDeltaTime )
{
	if ( m_pBlob == TNULL || a_fDeltaTime <= 0.0f )
	{
		ACamera::Update( a_fDeltaTime );
		return;
	}

	SetMatrix( m_mPrevMatrix );

	TBOOL  bNoRotBlend = TFALSE;
	TBOOL  bGrounded   = m_pBlob->m_eState == ABlobEntity::STATE_ROLL || m_pBlob->m_eState == ABlobEntity::STATE_FLOAT;
	TFLOAT fRadius     = ABlobEntity::GetProperties( m_pBlob->m_iSize )->m_fRadius;

	TVector4 vVelocity;
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBlob->m_pBody, vVelocity );

#ifdef DEBLOB_COMMUNITY_PATCH
	// The view follows the position Blob is drawn at (between the body's last two fixed steps, see
	// ABlobEntity::GetRenderPhysicsMatrix), not the body's last step: above 60 fps the stepped target made the cameras
	// that snap to it (fixed cameras, look-ats) twitch
	TVector4 vBlobPos = m_pBlob->GetRenderPosition();
	TVector4 vTarget  = vBlobPos;

	if ( m_pBlob->IsInLiquid() )
		vTarget = GetLiquidViewTarget( m_pBlob, vBlobPos );
#else
	TVector4 vBlobPos = m_pBlob->m_oPhysicsMatrix.GetTranslation();
	TVector4 vTarget  = vBlobPos;

	if ( m_pBlob->IsInLiquid() )
		vTarget = m_pBlob->GetLiquidHitPosition();
#endif

	TBOOL        bOverride      = TFALSE;
	AGameStatus* pGameStatus    = AGameStatus::GetSingleton();
	TVector4     vFacing        = m_pBlob->m_vFacing;
	TFLOAT       fOverrideBlend = 1.0f;
	TMatrix44    mOverride;

	if ( pGameStatus->IsLevelStart() )
	{
		bOverride = pGameStatus->GetLevelStartCamera( mOverride, fOverrideBlend );

		if ( bOverride )
		{
			pGameStatus->GetLevelStartBlobPosition( vTarget );
			GetFlatForward( vFacing );
		}
	}
	else if ( m_pBlob->m_eState == ABlobEntity::STATE_LEVEL_EXIT && APlayerManager::GetSingleton()->GetVistaCamera() == TNULL )
	{
		// Exiting the level: the camera of the exit door
		bOverride      = TTRUE;
		fOverrideBlend = TSTATICCAST( APropTriggerLevelComplete, m_pBlob->m_pLevelExitTrigger )->GetDoorCameraMatrix( mOverride );
	}
	else if ( m_pOverrideCam != TNULL )
	{
		// A scripted camera (e.g. the respawn camera) blending out into this one
		TBOOL bKeep = TFALSE;

		if ( TSTATICCAST( ACamera, m_pOverrideCam )->GetCameraType() == CT_SCRIPTED )
		{
			AScriptedCamera* pScriptedCam = TSTATICCAST( AScriptedCamera, m_pOverrideCam );
			TASSERT( ( pScriptedCam == TNULL ) || pScriptedCam->IsA( &TGetClass( AScriptedCamera ) ) );
			pScriptedCam->Update( a_fDeltaTime );

			if ( pScriptedCam->IsAnimating() )
			{
				TFLOAT fLength     = pScriptedCam->GetAnimationLength();
				TFLOAT fTime       = pScriptedCam->GetAnimationTime();
				TFLOAT fBlendStart = pScriptedCam->GetBlendOutStart() * fLength;
				TFLOAT fBlendEnd   = pScriptedCam->GetBlendOutEnd() * fLength;
				pScriptedCam->GetMatrix( mOverride );
				bOverride = TTRUE;
				bKeep     = TTRUE;

				if ( fTime > fBlendStart )
				{
					if ( fBlendEnd - fTime < 0.0f )
					{
						m_pOverrideCam = TNULL;
						fOverrideBlend = 0.0f;
					}
					else
					{
						fOverrideBlend = ( fBlendEnd - fTime ) / ( fBlendEnd - fBlendStart );
					}
				}
			}
		}

		if ( !bKeep )
			m_pOverrideCam = TNULL;
	}

	vTarget.y -= fRadius;
	TFLOAT fMaxSpeed = TPhysicsManager::GetSingleton()->GetMaxLinearVelocity( m_pBlob->m_pBody );
	TFLOAT fSpeed    = vVelocity.Magnitude();
	TBOOL  bStill    = fSpeed < 0.03f;

	TBOOL bNoOffset = APlayerManager::GetSingleton()->IsBlobInState15( m_pBlob );

	if ( !bStill || bNoOffset )
		m_fIdleTime = 0.0f;
	else
		m_fIdleTime = a_fDeltaTime * 0.5f + m_fIdleTime;

	TMath::Clip( m_fIdleTime, 0.0f, 1.0f );

	if ( fMaxSpeed < fSpeed )
		fMaxSpeed = fSpeed;

	TMath::Clip( fMaxSpeed, 1.0f, 200.0f );

	TVector4                      vCamPos          = GetMatrix().GetTranslation();
	AXYZViewportManager*          pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
	AXYZViewportManager::VIEWPORT eViewport        = pViewportManager->GetCameraViewport( this );
	void*                         pRegion          = m_pRegion;
	TBOOL                         bSnap            = TFALSE;

	UpdateRegionDistances( a_fDeltaTime, &ACameraManager::GetSingleton()->GetNedCameras(), vTarget, fRadius, &m_fMinDist, &m_fIdealDist, &m_fMaxDist, &m_fRegionTimer, &m_uiRegionData, &pRegion, TFALSE );

	if ( m_pRegion != pRegion )
	{
		m_pRegion = pRegion;

		if ( pRegion != TNULL && ( TSTATICCAST( ANedCameraFunction::Region, pRegion )->m_uiFlags & ANedCameraFunction::RS_BLEND ) == 0 )
		{
			bSnap       = TTRUE;
			bNoRotBlend = TTRUE;
		}
	}

	TFLOAT fMinDist   = m_fMinDist;
	TFLOAT fIdealDist = m_fIdealDist;
	TFLOAT fMaxDist   = m_fMaxDist;

	m_fUnk1F0 -= a_fDeltaTime;
	TMath::Clip( m_fUnk1F0, 0.0f, TMath::MAXFLOAT );
	m_fUnk1F4 -= a_fDeltaTime;
	TMath::Clip( m_fUnk1F4, 0.0f, TMath::MAXFLOAT );
	m_fUnk1F8 -= a_fDeltaTime;
	TMath::Clip( m_fUnk1F8, 0.0f, TMath::MAXFLOAT );

	if ( !m_bUnk1C1 && ( m_iUnk3B0 & 1 ) )
	{
		ReadInput( a_fDeltaTime );
	}

	TFLOAT      fFOV        = m_Matrix.m_fFOV;
	TUINT32     uiPrevFlags = m_uiFlags;
	TFLOAT      fTargetFOV  = sm_fDefaultFOV;
	TFLOAT      fFOVSpeed   = ms_fFOVBlendSpeed;
	TQuaternion qOrient;
	POS_HANDLE  iPosType;
	PosSpeeds   oSpeeds;

	// The position handlers in order of priority (each one returns whether it handles the camera)
	TBOOL bHandler9 = TFALSE;

	bHandler9 = ABoss::GetSingleton() != TNULL && ABoss::GetSingleton()->GetStunCamera( &m_vIdealPos, &qOrient, &fTargetFOV, &fFOVSpeed, &bSnap );

	TBOOL bReset = TTRUE;

	if ( bHandler9 )
	{
		iPosType = POS_HANDLE_9;
		oSpeeds  = ms_aPosSpeeds[ PS_9 ];
	}
	else
	{
		TBOOL bHandler2 = TFALSE;
		bHandler2       = UpdateBusyCam( m_pBlob, vTarget, m_vIdealPos, m_iPosType );

		TBOOL bNedFixed  = TFALSE;
		TBOOL bSpline    = TFALSE;
		TBOOL bHandler1  = TFALSE;
		TBOOL bHandler7  = TFALSE;
		TBOOL bHandler8  = TFALSE;
		TBOOL bHandler10 = TFALSE;

		if ( bHandler2 )
		{
			iPosType = POS_HANDLE_2;
			oSpeeds  = ms_aPosSpeeds[ PS_2 ];
		}
		else
		{
			bNedFixed = UpdateNedFixedCam( a_fDeltaTime, 0, m_pBlob, &ACameraManager::GetSingleton()->GetNedCameras(), vBlobPos, vCamPos, m_vIdealPos, qOrient, &fTargetFOV, &fFOVSpeed, &m_pLastFixedCam, &m_pNedFixedCam, &m_pPendingFixedCam, &m_fFixedCamTimer, &m_uiFlags );

			if ( !bNedFixed )
			{
				bSpline = UpdateSplineCam( a_fDeltaTime, 0, m_pBlob, &ACameraManager::GetSingleton()->GetNedCameras(), &ACameraManager::GetSingleton()->GetSSCameras(), vTarget, vCamPos, m_vIdealPos, &m_vUnk118, &qOrient, &fTargetFOV, &fFOVSpeed, &m_pLastSpline, &m_pCurrSpline, &m_pPendingSpline, &m_fSplineTimer, &m_uiSplineFlags );
			}

			if ( !bNedFixed && !bSpline )
			{
				bHandler1 = UpdateChallengeCam( a_fDeltaTime, m_pBlob, &m_vUnk184, m_vIdealPos, &fTargetFOV, &fFOVSpeed );
			}

			if ( !bNedFixed && !bSpline && !bHandler1 )
			{
				bHandler7 = UpdateCloseCam( m_pBlob, vTarget, m_vIdealPos );
			}

			if ( !bNedFixed && !bSpline && !bHandler1 && !bHandler7 )
			{
				bHandler8 = UpdateUnderCam( m_pBlob, m_vIdealPos, vCamPos, m_vSmoothPos, vTarget, vFacing, fRadius, ms_fCollisionRadius );
			}

			if ( !bNedFixed && !bSpline && !bHandler1 && !bHandler7 && !bHandler8 )
			{
				bHandler10 = UpdateAnimationCam( m_pBlob, m_vIdealPos, vTarget, &qOrient, &fTargetFOV, &fFOVSpeed, &bSnap );
			}

			if ( bNedFixed )
			{
				iPosType = POS_HANDLE_NEDFIXED;
				oSpeeds  = ms_aPosSpeeds[ m_uiFlags == 2 ? PS_NEDFIXED_2 : ( m_uiFlags == 0x10 || m_uiFlags == 0x20 ) ? PS_NEDFIXED_10_20 :
				                                                                                                        PS_NEDFIXED ];
			}
			else if ( bSpline )
			{
				iPosType = POS_HANDLE_SPLINE;
				oSpeeds  = ms_aPosSpeeds[ PS_SPLINE ];
			}
			else if ( bHandler1 )
			{
				iPosType = POS_HANDLE_1;
				oSpeeds  = ms_aPosSpeeds[ PS_1 ];
			}
			else if ( bHandler7 )
			{
				iPosType = POS_HANDLE_7;
				oSpeeds  = ms_aPosSpeeds[ PS_7 ];
			}
			else if ( bHandler8 )
			{
				iPosType = POS_HANDLE_8;
				oSpeeds  = ms_aPosSpeeds[ PS_8 ];
			}
			else if ( bHandler10 )
			{
				iPosType = POS_HANDLE_10;
				oSpeeds  = ms_aPosSpeeds[ PS_10 ];
			}
			else
			{
				// These don't reset m_fUnk1F0 and m_bUnk1B4
				bReset = TFALSE;

				TBOOL bHandler3 = TFALSE;

				if ( !( m_iPosType == POS_HANDLE_7 || pGameStatus->IsLevelStart() || ( m_iPosType == POS_HANDLE_NEDFIXED && m_uiFlags == 0x200 ) ) )
				{
					bHandler3 = UpdateManualCam( a_fDeltaTime, m_vIdealPos, vCamPos, m_vSmoothPos, vTarget, vFacing, &m_vUnk1A4, &m_bUnk1B4, m_uiControls, &m_uiUnk1DC, fRadius, ms_fCollisionRadius, ms_fDefaultMaxDist, &m_fManualRotationSpeed, &m_fUnk1F0 );
				}

				if ( bHandler3 )
				{
					iPosType = POS_HANDLE_3;
					oSpeeds  = ms_aPosSpeeds[ ( m_uiControls & 0x80 ) ? PS_3_FLAG80 : PS_3 ];
				}
				else
				{
					bNedFixed = UpdateNedFixedCam( a_fDeltaTime, 1, m_pBlob, &ACameraManager::GetSingleton()->GetNedCameras(), vBlobPos, vCamPos, m_vIdealPos, qOrient, &fTargetFOV, &fFOVSpeed, &m_pLastFixedCam, &m_pNedFixedCam, &m_pPendingFixedCam, &m_fFixedCamTimer, &m_uiFlags );

					if ( !bNedFixed )
					{
						bSpline = UpdateSplineCam( a_fDeltaTime, 1, m_pBlob, &ACameraManager::GetSingleton()->GetNedCameras(), &ACameraManager::GetSingleton()->GetSSCameras(), vTarget, vCamPos, m_vIdealPos, &m_vUnk118, &qOrient, &fTargetFOV, &fFOVSpeed, &m_pLastSpline, &m_pCurrSpline, &m_pPendingSpline, &m_fSplineTimer, &m_uiSplineFlags );
					}

					if ( bNedFixed )
					{
						iPosType = POS_HANDLE_NEDFIXED;
						oSpeeds  = ms_aPosSpeeds[ m_uiFlags == 2 ? PS_NEDFIXED_2 : ( m_uiFlags == 0x10 || m_uiFlags == 0x20 ) ? PS_NEDFIXED_10_20 :
						                                                                                                        PS_NEDFIXED ];
					}
					else if ( bSpline )
					{
						iPosType = POS_HANDLE_SPLINE;
						oSpeeds  = ms_aPosSpeeds[ PS_SPLINE ];
					}
					else
					{
						// The normal follow
						TASSERT( TNULL == m_pNedFixedCam );
						TASSERT( TNULL == m_pCurrSpline );

						if ( m_iPosType == POS_HANDLE_NEDFIXED && m_uiFlags == 0x200 )
							m_vIdealPos = m_vLastPos;

						UpdateFollowPosition( a_fDeltaTime, m_vIdealPos, vCamPos, m_vSmoothPos, vFacing, vTarget, vBlobPos, fSpeed, fMaxSpeed, fRadius, fMinDist, fIdealDist, fMaxDist, bGrounded, &m_fUnk1F4, &m_fUnk1F8, m_pPhantom2, m_pPhantom1 );
						UpdateFollowFOV( vCamPos, vBlobPos, vVelocity, fMaxSpeed, &fTargetFOV, eViewport == AXYZViewportManager::VIEWPORT_Left || eViewport == AXYZViewportManager::VIEWPORT_Right );

						iPosType = POS_HANDLE_FOLLOW;
						oSpeeds  = ms_aPosSpeeds[ PS_FOLLOW ];
					}
				}
			}
		}
	}

	if ( bReset )
	{
		m_fUnk1F0 = 0.0f;
		m_bUnk1B4 = TFALSE;
	}

	TASSERT( iPosType != POS_HANDLE_NONE );

	if ( m_iPosType != iPosType )
	{
		m_vLastPos = vCamPos;

		if ( m_iPosType == POS_HANDLE_1 )
		{
			bSnap       = TTRUE;
			bNoRotBlend = TTRUE;
		}

		if ( m_iPosType == POS_HANDLE_NEDFIXED )
		{
			if ( uiPrevFlags == 0x200 )
			{
				bSnap       = TTRUE;
				bNoRotBlend = TTRUE;
			}

			if ( iPosType == POS_HANDLE_FOLLOW )
				StartRotationBlend();
		}

		m_fIdleTime = 0.0f;
	}

	// The FOV
	const TFLOAT fFOVSnapAngle = 0.17453294f;
	TFLOAT       fFOVBlend;

	switch ( iPosType )
	{
		case POS_HANDLE_1:
		case POS_HANDLE_3:
		case POS_HANDLE_FOLLOW:
		case POS_HANDLE_8:
			fFOVBlend = fFOVSpeed * a_fDeltaTime;
			TMath::Clip( fFOVBlend, 0.0f, 1.0f );

			if ( m_fIdleTime < 0.25f || fFOVSnapAngle <= TMath::Abs( fFOV - fTargetFOV ) )
				fFOV = TMath::LERP( fFOV, fTargetFOV, fFOVBlend );

			break;
		case POS_HANDLE_2:
		case POS_HANDLE_7:
			fFOV = fTargetFOV;
			break;
		case POS_HANDLE_NEDFIXED:
		case POS_HANDLE_SPLINE:
		{
			TUINT32 uiCamFlags = ( iPosType == POS_HANDLE_NEDFIXED ) ? m_pNedFixedCam->m_iFLAGS : m_pCurrSpline->m_uiFlags;

			if ( ( uiCamFlags & 0x2000 ) == 0 )
			{
				fFOV = fTargetFOV;
				break;
			}

			if ( iPosType == POS_HANDLE_NEDFIXED )
			{
				if ( m_pLastFixedCam != m_pNedFixedCam )
					m_fLastFOV = m_Matrix.m_fFOV;

				if ( ( uiCamFlags & 0x30 ) != 0 && ( uiCamFlags & 0x4000 ) == 0 )
				{
					// Blend the FOV by how far the camera got from where it was to the fixed camera
					TVector4 vLastPos = m_mLastOrient.GetTranslation();
					fFOV              = ( fTargetFOV - m_fLastFOV ) * GetFOVBlendFraction( vLastPos, m_vIdealPos, vCamPos ) + m_fLastFOV;
					break;
				}
			}

			fFOVBlend = fFOVSpeed * a_fDeltaTime;
			TMath::Clip( fFOVBlend, 0.0f, 1.0f );

			if ( ( uiCamFlags & 0x4000 ) == 0 || m_fIdleTime < 0.25f || fFOVSnapAngle <= TMath::Abs( fFOV - fTargetFOV ) )
				fFOV = TMath::LERP( fFOV, fTargetFOV, fFOVBlend );

			break;
		}
		case POS_HANDLE_9:
		case POS_HANDLE_10:
			fFOVBlend = fFOVSpeed * a_fDeltaTime;
			TMath::Clip( fFOVBlend, 0.0f, 1.0f );
			fFOV = TMath::LERP( fFOV, fTargetFOV, fFOVBlend );
			break;
		default:
			break;
	}

	m_Matrix.m_fFOV    = fFOV;
	m_Matrix.m_fAspect = TFLOAT( tan( fFOV * 0.5f ) );

	TFLOAT     fVFOV     = fFOV;
	TViewport* pViewport = pViewportManager->GetCameraTViewport( this );

	if ( pViewport != TNULL && eViewport != AXYZViewportManager::VIEWPORT_Left && eViewport != AXYZViewportManager::VIEWPORT_Right )
	{
		TASSERT( pViewport->GetHeight() > 0.0f && pViewport->GetWidth() > 0.0f );
		fVFOV = ( pViewport->GetHeight() / pViewport->GetWidth() ) * fFOV;
	}

	// The speeds
	TBOOL bCheckCollision;
	TBOOL bScale;

	if ( m_fUnk1F0 <= 0.0f )
	{
		bCheckCollision = TTRUE;
		m_fUnk1FC       = 0.0f;
		bScale          = iPosType == POS_HANDLE_3 || iPosType == POS_HANDLE_FOLLOW;
	}
	else
	{
		iPosType        = POS_HANDLE_3;
		bCheckCollision = TFALSE;
		oSpeeds         = ms_aPosSpeeds[ ( m_uiUnk1DC & 0x80 ) ? PS_3_FLAG80 : PS_3 ];
		bScale          = TTRUE;
	}

	if ( bScale )
	{
		if ( 1.0f < ms_fCamSpeedOption )
			oSpeeds.m_fLerpSpeed *= ms_fCamSpeedOption;

		oSpeeds.m_fAccel *= ms_fCamSpeedOption;
		oSpeeds.m_fMaxSpeed *= ms_fCamSpeedOption;

		if ( 0.0f < ms_fCamSpeedOption )
			oSpeeds.m_fDampenDist /= ms_fCamSpeedOption;
	}

	if ( iPosType == POS_HANDLE_NEDFIXED )
	{
		// The fixed camera's own speeds (0x48)
		const TFLOAT* pSpeeds = TREINTERPRETCAST( const TFLOAT*, m_pNedFixedCam->m_aUnk48 );

		if ( 0.0f < pSpeeds[ 0 ] ) oSpeeds.m_fAccel = pSpeeds[ 0 ];
		if ( 0.0f < pSpeeds[ 1 ] ) oSpeeds.m_fMaxSpeed = pSpeeds[ 1 ];
		if ( 0.0f < pSpeeds[ 2 ] ) oSpeeds.m_fDampenDist = pSpeeds[ 2 ];
		if ( 0.0f < pSpeeds[ 3 ] ) oSpeeds.m_fLerpSpeed = pSpeeds[ 3 ];

		m_pCurrSpline = TNULL;
	}
	else
	{
		m_pNedFixedCam = TNULL;

		if ( iPosType == POS_HANDLE_SPLINE )
		{
			// The spline camera's own speeds (0x28)
			const TFLOAT* pSpeeds = TREINTERPRETCAST( const TFLOAT*, m_pCurrSpline->m_aUnk28 );

			if ( 0.0f < pSpeeds[ 0 ] ) oSpeeds.m_fAccel = pSpeeds[ 0 ];
			if ( 0.0f < pSpeeds[ 1 ] ) oSpeeds.m_fMaxSpeed = pSpeeds[ 1 ];
			if ( 0.0f < pSpeeds[ 2 ] ) oSpeeds.m_fDampenDist = pSpeeds[ 2 ];
			if ( 0.0f < pSpeeds[ 3 ] ) oSpeeds.m_fLerpSpeed = pSpeeds[ 3 ];
		}
		else
		{
			m_pCurrSpline = TNULL;
		}
	}

	// The position
	TBOOL bNewSpline = TFALSE;

	if ( !bSnap )
	{
		switch ( iPosType )
		{
			case POS_HANDLE_1:
			case POS_HANDLE_7:
			case POS_HANDLE_8:
				SmoothPosition( a_fDeltaTime, vCamPos, m_vSmoothPos, vTarget, m_vIdealPos, m_vVelocity, oSpeeds.m_fAccel, oSpeeds.m_fMaxSpeed, oSpeeds.m_fDampenDist, ms_fCollisionRadius, TFALSE );
				LerpPosition( a_fDeltaTime, vCamPos, m_vSmoothPos, oSpeeds.m_fLerpSpeed );
				break;
			case POS_HANDLE_2:
			case POS_HANDLE_9:
			case POS_HANDLE_10:
				LerpPosition( a_fDeltaTime, m_vSmoothPos, m_vIdealPos, oSpeeds.m_fLerpSpeed );
				LerpPosition( a_fDeltaTime, vCamPos, m_vSmoothPos, oSpeeds.m_fLerpSpeed );
				break;
			case POS_HANDLE_3:
				if ( ( m_uiUnk1DC & 0x80 ) == 0 )
					LerpPosition( a_fDeltaTime, m_vSmoothPos, m_vIdealPos, oSpeeds.m_fLerpSpeed );
				else
					SmoothPosition( a_fDeltaTime, vCamPos, m_vSmoothPos, vTarget, m_vIdealPos, m_vVelocity, oSpeeds.m_fAccel, oSpeeds.m_fMaxSpeed, oSpeeds.m_fDampenDist, ms_fCollisionRadius, bCheckCollision );

				LerpPosition( a_fDeltaTime, vCamPos, m_vSmoothPos, oSpeeds.m_fLerpSpeed );
				break;
			case POS_HANDLE_NEDFIXED:
			case POS_HANDLE_SPLINE:
			{
				TUINT32 uiCamFlags;

				if ( iPosType == POS_HANDLE_NEDFIXED )
				{
					uiCamFlags = m_pNedFixedCam->m_iFLAGS;
				}
				else
				{
					bNewSpline = m_pLastSpline != m_pCurrSpline;

					if ( bNewSpline )
						m_pLastSpline = m_pCurrSpline;

					uiCamFlags = m_pCurrSpline->m_uiFlags;
				}

				if ( uiCamFlags & 0x2000 )
				{
					SmoothPosition( a_fDeltaTime, vCamPos, m_vSmoothPos, vTarget, m_vIdealPos, m_vVelocity, oSpeeds.m_fAccel, oSpeeds.m_fMaxSpeed, oSpeeds.m_fDampenDist, ms_fCollisionRadius, TTRUE );
					LerpPosition( a_fDeltaTime, vCamPos, m_vSmoothPos, oSpeeds.m_fLerpSpeed );
				}
				else
				{
					// No blending: cut to it
					vCamPos      = m_vIdealPos;
					m_vSmoothPos = m_vIdealPos;
				}
				break;
			}
			case POS_HANDLE_FOLLOW:
				SmoothPosition( a_fDeltaTime, vCamPos, m_vSmoothPos, vTarget, m_vIdealPos, m_vVelocity, oSpeeds.m_fAccel, oSpeeds.m_fMaxSpeed, oSpeeds.m_fDampenDist, ms_fCollisionRadius, bCheckCollision );
				LerpPosition( a_fDeltaTime, vCamPos, m_vSmoothPos, oSpeeds.m_fLerpSpeed );
				break;
			default:
				break;
		}
	}
	else
	{
		vCamPos      = m_vIdealPos;
		m_vSmoothPos = m_vIdealPos;
	}

	m_Matrix.m_mMatrix.SetTranslation( vCamPos );

#ifdef DEBLOB_COMMUNITY_PATCH
	if ( !m_pBlob->IsInLiquid() )
		vTarget = m_pBlob->GetLastRenderMatrix().GetTranslation();
	else
		vTarget = GetLiquidViewTarget( m_pBlob, m_pBlob->GetLastRenderMatrix().GetTranslation() );
#else
	if ( !m_pBlob->IsInLiquid() )
		vTarget = m_pBlob->m_oPhysicsMatrix.GetTranslation();
	else
		vTarget = m_pBlob->GetLiquidHitPosition();
#endif

	// The orientation
	TMatrix44 mView = m_Matrix.m_mMatrix;
	TVector4  vDir( mView.m_f31, mView.m_f32, mView.m_f33, 1.0f );
	TVector4  vUp( -mView.m_f21, -mView.m_f22, -mView.m_f23, 1.0f );
	TVector4  vRight( mView.m_f11, mView.m_f12, mView.m_f13, 1.0f );

	TVector4 vToTargetXZ;
	vToTargetXZ.x = vTarget.x - vCamPos.x;
	vToTargetXZ.y = vTarget.y - vCamPos.y;
	vToTargetXZ.z = vTarget.z - vCamPos.z;
	vToTargetXZ.w = vTarget.w;
	vToTargetXZ.NormaliseXZ();

	TBOOL bSaveOrient   = TTRUE;
	TBOOL bSetView      = TFALSE;
	TBOOL bSkipRotBlend = TFALSE;

	switch ( iPosType )
	{
		case POS_HANDLE_2:
		{
			TVector4 vLookAt( vBlobPos.x, vBlobPos.y - ms_fUnkBC68, vBlobPos.z, vBlobPos.w );
			UpdateLookDirection( a_fDeltaTime, vCamPos, vLookAt, vFacing, vDir, vUp, vRight, &m_fLookSpeed, &m_fPitch, &m_fYaw, fFOV, fVFOV, TTRUE, bSnap, 0.0f );
			LookAtDirection( vDir, vUp );
			break;
		}
		case POS_HANDLE_NEDFIXED:
		{
			ANedCameraFunction::FixedCam* pFixedCam = m_pNedFixedCam;
			TUINT32                       uiFlags   = pFixedCam->m_iFLAGS;
			TBOOL                         bLookAt   = TFALSE;

			if ( ( uiFlags & 0x2000 ) != 0 && ( uiFlags & 0x4000 ) == 0 )
			{
				// The blended fixed camera (flags 0x2000 without 0x4000): a new fixed camera saves the current orientation
				if ( m_pLastFixedCam != pFixedCam )
				{
					m_pLastFixedCam = pFixedCam;
					m_mLastOrient   = m_Matrix.m_mMatrix;
					m_fLastFOV      = m_Matrix.m_fFOV;
				}

				TMatrix44 mBlend;

				if ( UpdateBlendedFixedCam( vTarget, vCamPos, m_vIdealPos, m_mLastOrient, qOrient, mBlend, ( uiFlags & 0x30 ) != 0 ) )
				{
					// The blend is in the last orientation's space: the last orientation is not saved this update
					mBlend.SetTranslation( vCamPos );
					SetMatrix( mBlend );
					bSaveOrient = TFALSE;
				}
				else
				{
					UpdateLookDirection( a_fDeltaTime, vCamPos, vTarget, vFacing, vDir, vUp, vRight, &m_fLookSpeed, &m_fPitch, &m_fYaw, fFOV, fVFOV, bNoOffset, TFALSE, 0.0f );
					LookAtDirection( vDir, vUp );
				}

				if ( bSnap )
				{
					TQuaternion qFixed = qOrient;
					qFixed /= TMath::Sqrt( qFixed.x * qFixed.x + qFixed.y * qFixed.y + qFixed.z * qFixed.z + qFixed.w * qFixed.w );
					mView.SetFromQuaternion( qFixed );
					mView.SetTranslation( vCamPos );
					SetMatrix( mView );
					bSkipRotBlend = TTRUE;
				}
				break;
			}

			if ( ( uiFlags & 0x2000 ) == 0 && ( uiFlags & 0x4000 ) == 0 )
			{
				// Cut to the fixed camera's orientation
				TQuaternion qFixed = qOrient;
				qFixed /= TMath::Sqrt( qFixed.x * qFixed.x + qFixed.y * qFixed.y + qFixed.z * qFixed.z + qFixed.w * qFixed.w );
				mView.SetFromQuaternion( qFixed );
				mView.SetTranslation( vCamPos );
				SetMatrix( mView );
				bSkipRotBlend = TTRUE;
				break;
			}

			if ( ( uiFlags & 0x2000 ) == 0 && m_pLastFixedCam != pFixedCam )
			{
				// Look at the target once when the camera is new
				m_pLastFixedCam = pFixedCam;
				LookAtPoint( vTarget );
				bSkipRotBlend = TTRUE;
				break;
			}

			UpdateLookDirection( a_fDeltaTime, vCamPos, vTarget, vFacing, vDir, vUp, vRight, &m_fLookSpeed, &m_fPitch, &m_fYaw, fFOV, fVFOV, bNoOffset, bSnap, m_fIdleTime );
			LookAtDirection( vDir, vUp );
			break;
		}
		case POS_HANDLE_3:
		{
			// The manual camera: the view turns towards the target (the orbit target, before the blob centre below)
			TFLOAT fSpeed = 2.0f;

			if ( 1.0f < ms_fCamSpeedOption )
				fSpeed = ms_fCamSpeedOption * 2.0f;

			UpdateBlendedLookAt( a_fDeltaTime, fSpeed, &m_fUnk1FC, vTarget, mView );
			m_fLookSpeed = 0.0f;
			m_fPitch     = 0.0f;
			m_fYaw       = 0.0f;
			bSetView     = TTRUE;
			break;
		}
		case POS_HANDLE_1:
			// The challenge camera
			UpdateOffsetLookAt( a_fDeltaTime, ms_fRotSpeed * 2.0f, fTargetFOV, m_vIdealPos, m_vUnk184, vTarget, mView );
			bSetView = TTRUE;
			break;
		case POS_HANDLE_SPLINE:
		{
			ANedCameraFunction::SplineCam* pSpline = m_pCurrSpline;
			TUINT32                        uiFlags = pSpline->m_uiFlags;
			TFLOAT                         fSpeed  = pSpline->m_fLookSpeed;

			if ( ( uiFlags & 0x2000 ) != 0 && ( uiFlags & 0x4000 ) == 0 )
			{
				// The blended spline camera
				if ( bNewSpline )
				{
					GetMatrix( m_mLastOrient );
					m_fLastFOV     = m_Matrix.m_fFOV;
					m_fSplineBlend = 0.0f;
				}

				if ( fSpeed <= 0.0f )
					fSpeed = 2.0f;

				m_fSplineBlend = fSpeed * a_fDeltaTime + m_fSplineBlend;
				TMath::Clip( m_fSplineBlend, 0.0f, 9999.0f );

				if ( ( m_pCurrSpline->m_uiFlags & 0x400000 ) == 0 )
				{
					if ( m_pCurrSpline->m_fLookSpeed > 0.0f )
					{
						UpdateBlendedOrient( a_fDeltaTime, m_fSplineBlend, qOrient, mView );
						bSetView = TTRUE;
						break;
					}

					if ( UpdateBlendedFixedCam( vTarget, vCamPos, m_vIdealPos, m_mLastOrient, qOrient, mView, TTRUE ) )
					{
						bSetView = TTRUE;
						break;
					}

					UpdateLookDirection( a_fDeltaTime, vCamPos, vTarget, vFacing, vDir, vUp, vRight, &m_fLookSpeed, &m_fPitch, &m_fYaw, fFOV, fVFOV, bNoOffset, bSnap, 0.0f );
					LookAtDirection( vDir, vUp );
					break;
				}

				// Towards the start of the spline camera's line
				const ASSCameraFunction::Camera* pCam = ACameraManager::GetSingleton()->GetSSCameras().FindCamera( m_pCurrSpline->m_iSSCam );
				UpdateTurnTowards( a_fDeltaTime, m_fSplineBlend, pCam->m_vLineStart, mView );
				bSetView = TTRUE;
				break;
			}

			if ( ( uiFlags & 0x2000 ) != 0 )
			{
				// 0x2000 and 0x4000: blends a look at the target
				if ( fSpeed <= 0.0f )
				{
					UpdateLookDirection( a_fDeltaTime, vCamPos, vTarget, vFacing, vDir, vUp, vRight, &m_fLookSpeed, &m_fPitch, &m_fYaw, fFOV, fVFOV, bNoOffset, bSnap, m_fIdleTime );
					LookAtDirection( vDir, vUp );
					break;
				}

				TFLOAT fBlend = 0.0f;
				UpdateBlendedLookAt( a_fDeltaTime, fSpeed, &fBlend, vTarget, mView );
				bSetView = TTRUE;
				break;
			}

			if ( ( uiFlags & 0x4000 ) != 0 )
			{
				// Looks at the target once when the camera is new
				if ( bNewSpline )
				{
					LookAtPoint( vTarget );
					bSkipRotBlend = TTRUE;
					break;
				}

				UpdateLookDirection( a_fDeltaTime, vCamPos, vTarget, vFacing, vDir, vUp, vRight, &m_fLookSpeed, &m_fPitch, &m_fYaw, fFOV, fVFOV, bNoOffset, bSnap, m_fIdleTime );
				LookAtDirection( vDir, vUp );
				break;
			}

			if ( uiFlags & 0x400000 )
			{
				// Looks at the start of the spline camera's line
				const ASSCameraFunction::Camera* pCam = ACameraManager::GetSingleton()->GetSSCameras().FindCamera( pSpline->m_iSSCam );
				mView.LookAtTarget( pCam->m_vLineStart, mView.GetTranslation() );
			}
			else
			{
				// Cut to the spline camera's orientation
				TQuaternion qSpline = qOrient;
				qSpline /= TMath::Sqrt( qSpline.x * qSpline.x + qSpline.y * qSpline.y + qSpline.z * qSpline.z + qSpline.w * qSpline.w );
				mView.SetFromQuaternion( qSpline );
			}

			mView.SetTranslation( vCamPos );
			SetMatrix( mView );
			bSkipRotBlend = TTRUE;
			break;
		}
		case POS_HANDLE_7:
		{
			// The close camera
			if ( !m_bBlobVision )
				break;

			TFLOAT fDX = vCamPos.x - vTarget.x;
			TFLOAT fDZ = vCamPos.z - vTarget.z;

			if ( 25.0f <= fDZ * fDZ + fDX * fDX )
			{
				if ( m_bBlobVision )
				{
					TVector4 vOffset;
					vOffset.Multiply( vToTargetXZ, 5.0f );

					TVector4 vLookAt( vOffset.x + vTarget.x, ( vTarget.y + vOffset.y ) - 3.0f, vOffset.z + vTarget.z, vTarget.w );
					UpdateLookDirection( a_fDeltaTime, vCamPos, vLookAt, vFacing, vDir, vUp, vRight, &m_fLookSpeed, &m_fPitch, &m_fYaw, fFOV, fVFOV, TTRUE, bSnap, 0.0f );
					LookAtDirection( vDir, vUp );
				}

				break;
			}

			// The free look
			UpdateFreeLook( a_fDeltaTime, vDir, vUp, vRight, m_afFreeLookInput, m_bFreeLook );
			LookAtDirection( vDir, vUp );
			break;
		}
		case POS_HANDLE_9:
		case POS_HANDLE_10:
			// The boss and animation cameras
			if ( !bSnap )
			{
				UpdateBlendedOrient( a_fDeltaTime, ms_fRotSpeed, qOrient, mView );
			}
			else
			{
				TQuaternion qAnim = qOrient;
				qAnim /= TMath::Sqrt( qAnim.x * qAnim.x + qAnim.y * qAnim.y + qAnim.z * qAnim.z + qAnim.w * qAnim.w );
				mView.SetFromQuaternion( qAnim );
				bNoRotBlend = TTRUE;
			}

			bSetView = TTRUE;
			break;
		default:
			UpdateLookDirection( a_fDeltaTime, vCamPos, vTarget, vFacing, vDir, vUp, vRight, &m_fLookSpeed, &m_fPitch, &m_fYaw, fFOV, fVFOV, bNoOffset, bSnap, m_fIdleTime );
			LookAtDirection( vDir, vUp );
			break;
	}

	if ( bSetView )
	{
		mView.SetTranslation( vCamPos );
		SetMatrix( mView );
	}

	if ( !bNoRotBlend && !bSkipRotBlend )
	{
		// Blend the rotation from the last update's
		if ( ms_fRotBlendTime <= m_fRotBlendTimer )
			m_fRotBlendTarget = 1000.0f;
		else
			m_fRotBlendTimer = m_fRotBlendTimer + a_fDeltaTime;

		TFLOAT fTarget = m_fRotBlendTarget;
		TFLOAT fDiff   = fTarget - m_fRotBlend;

		if ( 0.0f <= fDiff )
		{
			if ( 0.0f < fDiff )
			{
				TFLOAT fNew = ms_fRotBlendUp * fDiff * a_fDeltaTime + m_fRotBlend;
				m_fRotBlend = fNew;

				if ( !( fNew < fTarget ) && fTarget != fNew )
					m_fRotBlend = fTarget;
			}
			else
			{
				m_fRotBlend = fTarget;
			}
		}
		else
		{
			TFLOAT fNew = ms_fRotBlendDown * fDiff * a_fDeltaTime + m_fRotBlend;
			m_fRotBlend = fNew;

			if ( !( fTarget < fNew ) && fTarget != fNew )
				m_fRotBlend = fTarget;
		}

		TFLOAT fBlend = m_fRotBlend * a_fDeltaTime;
		TMath::Clip( fBlend, 0.0f, 1.0f );

		TMatrix44   mCurrent = m_Matrix.m_mMatrix;
		TQuaternion qPrev;
		TQuaternion qCurrent;
		TQuaternion qBlend;
		TMatrix44::MatToQuat( qPrev, m_mPrevMatrix );
		TMatrix44::MatToQuat( qCurrent, mCurrent );
		TQuaternion::Slerp( qBlend, qPrev, qCurrent, fBlend );
		qBlend /= TMath::Sqrt( qBlend.x * qBlend.x + qBlend.y * qBlend.y + qBlend.z * qBlend.z + qBlend.w * qBlend.w );

		TVector4 vTranslation = m_Matrix.m_mMatrix.GetTranslation();
		mCurrent.SetFromQuaternion( qBlend );
		mCurrent.SetTranslation( vTranslation );
		SetMatrix( mCurrent );
	}

	if ( bSaveOrient )
	{
		m_fLastFOV    = m_Matrix.m_fFOV;
		m_mLastOrient = m_Matrix.m_mMatrix;
	}

	m_mPrevMatrix = m_Matrix.m_mMatrix;

	if ( bOverride )
	{
		if ( 1.0f <= fOverrideBlend )
		{
			SetMatrix( mOverride );
		}
		else
		{
			TMatrix44 mCurrent;
			GetMatrix( mCurrent );

			TQuaternion qCurrent;
			TQuaternion qOverride;
			TQuaternion qBlend;
			TMatrix44::MatToQuat( qCurrent, mCurrent );
			TMatrix44::MatToQuat( qOverride, mOverride );
			TQuaternion::Slerp( qBlend, qCurrent, qOverride, fOverrideBlend );
			qBlend /= TMath::Sqrt( qBlend.x * qBlend.x + qBlend.y * qBlend.y + qBlend.z * qBlend.z + qBlend.w * qBlend.w );

			TMatrix44 mBlend;
			mBlend.SetFromQuaternion( qBlend );

			TVector4 vFrom = mCurrent.GetTranslation();
			TVector4 vTo   = mOverride.GetTranslation();
			TVector4 vPos;
			vPos.Lerp4( vFrom, vTo, fOverrideBlend );
			mBlend.SetTranslation( vPos );
			SetMatrix( mBlend );
		}
	}

	// The near plane
	SetNearClip( 5.0f );

	if ( iPosType == POS_HANDLE_7 )
	{
		SetNearClip( 0.1f );
	}
	else
	{
		TFLOAT fDistSq = TVector4::DistanceSq( m_Matrix.m_mMatrix.GetTranslation(), vBlobPos );
		TFLOAT fMax    = fRadius + 5.0f + 1.0f;

		if ( fDistSq <= fMax * fMax && iPosType != POS_HANDLE_NONE )
		{
			TFLOAT fNear = ( TMath::Sqrt( fDistSq ) - fRadius ) - 1.0f;
			TMath::Clip( fNear, 0.1f, 5.0f );
			SetNearClip( fNear );
		}
	}

	m_iPosType = iPosType;
	ACamera::Update( a_fDeltaTime );
}

// $deBlob: FUNCTION 0058b110
void ABlobFollowCamera::ReadInput( TFLOAT a_fDeltaTime )
{
	m_uiControls = 0;
	TASSERT( APlayerManager::IsSingletonCreated() );

	if ( m_pBlob == TNULL )
		return;

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	TINT            iPlayer        = pPlayerManager->GetPlayerIndex( m_pBlob );

	if ( iPlayer < 0 )
		return;

	TASSERT( iPlayer >= 0 && iPlayer < APlayerManager::MAX_PLAYERS );
	TINT iController = pPlayerManager->GetController( TUINT8( iPlayer ) );

	if ( iController < 0 )
		return;

	ABlobInputMap* pInputMap = ABlobInputMap::GetSingleton();

	if ( iController == 0 )
	{
		// A cheat sequence is being entered on a gamepad
		TBOOL bCheatSequence = ACheatManager::GetSingleton()->IsAllShoulderButtonsDown();

		if ( bCheatSequence && TInputDevice::GetLastGamepadType() != 0 )
			return;
	}

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();

	if ( pState->IsA( &TGetClass( AInGameState ) ) && TSTATICCAST( AInGameState, pState )->IsPaused() )
		return;

	if ( m_pBlob->m_bInBlobVision != 0 )
	{
		// The movement stick (keyboard: the movement keys at full speed) and the mouse turn the camera
		TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
		const TVector2&       rvStick   = pInputMap->GetMoveStick( iPlayer );

		TFLOAT fSign = 1.0f;
		TFLOAT fX    = rvStick.x;
		TFLOAT fY    = rvStick.y;

		if ( fX != 0.0f )
		{
			if ( fX < 0.0f )
				fSign = -1.0f;

			if ( pKeyboard != TNULL && ( pKeyboard->IsDown( ABlobInputMap::ms_aKeyboardBindings[ ABlobInputMap::BINDING_LEFT ].m_iButton ) || pKeyboard->IsDown( ABlobInputMap::ms_aKeyboardBindings[ ABlobInputMap::BINDING_RIGHT ].m_iButton ) ) )
				fX = fSign;

			fX = TMath::Abs( fX ) * ms_fCamSpeedOption * fSign;
		}

		m_afFreeLookInput[ 0 ] = fX;

		if ( fY != 0.0f )
		{
			fSign = 1.0f;

			if ( 0.0f <= fY )
				fSign = -1.0f;

			if ( pKeyboard != TNULL && ( pKeyboard->IsDown( ABlobInputMap::ms_aKeyboardBindings[ ABlobInputMap::BINDING_UP ].m_iButton ) || pKeyboard->IsDown( ABlobInputMap::ms_aKeyboardBindings[ ABlobInputMap::BINDING_DOWN ].m_iButton ) ) )
				fY = fSign;

			fY = TMath::Abs( fY ) * ms_fCamSpeedOption * fSign * ms_fInvertY;
		}

		m_afFreeLookInput[ 1 ] = fY;
		m_bFreeLook            = TTRUE;

		const ABlobInputMap::PlayerInput& rInput = pInputMap->GetPlayerInput( iPlayer );

		if ( rInput.m_bCursorActive )
		{
			m_afFreeLookInput[ 1 ] = rInput.m_vCursor.y + m_afFreeLookInput[ 1 ];
			m_afFreeLookInput[ 0 ] = rInput.m_vCursor.x + m_afFreeLookInput[ 0 ];
		}

		m_uiControls |= 0x20;
		return;
	}

	// The camera buttons (CamLeft, CamRight, CamUp, CamDown, CenterCamera)
	if ( pInputMap->IsButtonDown( iController, ABlobInputMap::BINDING_CAMLEFT ) )
		m_uiControls |= 0x11;

	if ( pInputMap->IsButtonDown( iController, ABlobInputMap::BINDING_CAMRIGHT ) )
		m_uiControls |= 0x12;

	if ( pInputMap->IsButtonDown( iController, ABlobInputMap::BINDING_CAMUP ) )
		m_uiControls |= 0x14;

	if ( pInputMap->IsButtonDown( iController, ABlobInputMap::BINDING_CAMDOWN ) )
		m_uiControls |= 0x18;

	if ( pInputMap->IsButtonPressed( iController, ABlobInputMap::BINDING_CENTERCAMERA ) )
		m_uiControls |= 0x90;
}

// $deBlob: FUNCTION 0058b460
void ABlobFollowCamera::SetTarget( ABlobEntity* a_pBlob )
{
	m_pBlob                = a_pBlob;
	m_vVelocity        = TVector4::VEC_ZERO;
	m_vUnk184          = TVector4::VEC_ZERO;
	m_vLastPos         = TVector4::VEC_ZERO;
	m_bUnk1B4          = TFALSE;
	m_vUnk1A4              = TVector4::VEC_ZERO;
	m_vIdealPos            = m_Matrix.m_mMatrix.GetTranslation();
	m_fFixedCamTimer       = 0.0f;
	m_fPitch               = 0.0f;
	m_vSmoothPos           = m_Matrix.m_mMatrix.GetTranslation();
	m_fYaw                 = 0.0f;
	m_uiControls           = 0;
	m_pNedFixedCam         = TNULL;
	m_pPendingFixedCam     = TNULL;
	m_pLastFixedCam        = TNULL;
	m_uiFlags              = 0x10000;
	m_uiSplineFlags        = 0x10000;
	m_afFreeLookInput[ 0 ] = TVector2::VEC_ZERO.x;
	m_fLookSpeed           = ms_fRotSpeed;
	m_fMaxDist             = ms_fDefaultMaxDist;
	m_fIdealDist           = ms_fDefaultIdealDist;
	m_afFreeLookInput[ 1 ] = TVector2::VEC_ZERO.y;
	m_bFreeLook            = TFALSE;
	m_bUnk1C1              = TFALSE;
	m_fManualRotationSpeed = 0.0f;
	m_fSplineBlend         = 0.0f;
	m_uiUnk1EC             = 0;
	m_uiUnk1E4             = 0;
	m_pRegion              = TNULL;
	m_fMinDist             = ms_fDefaultMinDist;
	m_fRegionTimer         = 0.0f;
	m_fUnk1F0          = 0.0f;
	m_fUnk1F4          = 0.0f;
	m_fUnk1F8          = 0.0f;
	m_uiRegionData     = 0;
	m_pLastSpline      = TNULL;
	m_pCurrSpline      = TNULL;
	m_pPendingSpline   = TNULL;
	m_fSplineTimer     = 0.0f;
	m_vUnk118          = a_pBlob->m_oPhysicsMatrix.GetTranslation();
	m_mLastOrient      = TMatrix44::IDENTITY;
	m_mPrevMatrix      = TMatrix44::IDENTITY;
	m_fLastFOV         = 1.0471976f;
	m_fUnk1FC          = 0.0f;
	m_uiUnk1DC             = 0;
	m_fIdleTime            = 0.0f;
	m_iPosType             = POS_HANDLE_NONE;
	m_pOverrideCam         = TNULL;
	m_bRotBlend            = TFALSE;

	for ( TINT i = MAX_ALPHA_INSTANCES; i != 0; )
	{
		i--;
		m_afAlphaInstances[ i ] = 0.0f;
		m_afAlpha[ i ]          = 1.0f;
		m_afAlphaTarget[ i ]    = 1.0f;
		m_abUnk344[ i ]         = TFALSE;
	}

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( m_pPhantom1 == TNULL )
	{
		m_pPhantom1 = pPhysicsManager->CreateAabbPhantom();
		pPhysicsManager->SetPhantomCollisionFilterInfo( TSTATICCAST( TPhysicsPhantomContainer, m_pPhantom1 ), 0x10000 );
		pPhysicsManager->AddPhantomToSimulation( TSTATICCAST( TPhysicsPhantomContainer, m_pPhantom1 ), a_pBlob->GetTranslation(), TNULL );
	}

	if ( m_pPhantom2 == TNULL )
	{
		m_pPhantom2 = pPhysicsManager->CreateAabbPhantom();
		pPhysicsManager->SetPhantomCollisionFilterInfo( TSTATICCAST( TPhysicsPhantomContainer, m_pPhantom2 ), 0x10000 );
		pPhysicsManager->AddPhantomToSimulation( TSTATICCAST( TPhysicsPhantomContainer, m_pPhantom2 ), a_pBlob->GetTranslation(), TNULL );
	}

	// The camera speed option
	TFLOAT fSpeed = AOptions::GetSingleton()->GetCameraSpeed();
	if ( fSpeed <= 0.5f ) fSpeed = 0.5f;
	if ( 1.0f <= fSpeed ) fSpeed = 1.0f;

	TASSERT( fSpeed >= 0.5f && fSpeed <= 1.0f );
	ms_fCamSpeedOption = fSpeed;
	ms_fInvertY        = AOptions::GetSingleton()->IsCameraInverted() ? -1.0f : 1.0f;
}

// $deBlob: FUNCTION 0058b940
void ABlobFollowCamera::SetUnk1C1( TBOOL a_bUnk1C1 )
{
	if ( m_pBlob != TNULL && a_bUnk1C1 )
	{
		m_bUnk1C1    = TTRUE;
		m_uiControls = 4;
	}
	else
	{
		m_bUnk1C1 = TFALSE;
	}
}

// Snaps the camera to a start position for the blob: the player's start point (a_bLevelStart), else a Ned fixed or
// spline camera of the blob's position (flags 4 for a level start, 8 for a respawn, else 1), else behind the blob
// $deBlob: FUNCTION 0058b980
void ABlobFollowCamera::Reset( TBOOL a_bLevelStart, TBOOL a_bRespawn )
{
	if ( m_pBlob == TNULL )
		return;

	TVector4 vBlobPos = m_pBlob->m_oPhysicsMatrix.GetTranslation();
	TVector4 vCamPos  = m_Matrix.m_mMatrix.GetTranslation();
	m_pNedFixedCam    = TNULL;
	m_fFixedCamTimer  = 0.0f;

	TBOOL  bStartPoint = TFALSE;
	TBOOL  bDefaultFOV = TFALSE;
	TFLOAT fFOV        = sm_fDefaultFOV;

	if ( a_bLevelStart )
	{
		TINT8 iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( m_pBlob );

		if ( iPlayer >= 0 )
		{
			APointDataFunction::Point* pPoint = AGameStatus::GetSingleton()->GetPointData().GetPoint( iPlayer + 5 );

			if ( pPoint != TNULL )
			{
				m_vIdealPos = pPoint->m_vPosition;
				m_Matrix.m_mMatrix.SetTranslation( pPoint->m_vPosition );
				bStartPoint = TTRUE;
			}
		}
	}

	if ( !bStartPoint )
	{
		ANedCameraFunction& rNedCameras = ACameraManager::GetSingleton()->GetNedCameras();
		TUINT32             uiFlags     = a_bLevelStart ? 4 : ( a_bRespawn ? 8 : 1 );

		m_pNedFixedCam = rNedCameras.FindFixedCamAt( vBlobPos, vCamPos, uiFlags );

		if ( m_pNedFixedCam == TNULL && uiFlags != 1 )
		{
			uiFlags        = 1;
			m_pNedFixedCam = rNedCameras.FindFixedCamAt( vBlobPos, vCamPos, uiFlags );
		}

		if ( m_pNedFixedCam != TNULL )
		{
			m_vIdealPos      = m_pNedFixedCam->m_vPosition;
			m_fFixedCamTimer = m_pNedFixedCam->m_fHoldTime;
			m_uiFlags        = uiFlags;

			if ( m_pNedFixedCam->m_iFLAGS & 0x4000 )
			{
				m_Matrix.m_mMatrix.SetTranslation( m_vIdealPos );
				LookAtPoint( vBlobPos );
			}
			else
			{
				TMatrix44 mMatrix;
				mMatrix.SetFromQuaternion( m_pNedFixedCam->m_qOrientation );
				mMatrix.SetTranslation( m_vIdealPos );
				SetMatrix( mMatrix );
			}

			fFOV = m_pNedFixedCam->m_fFOV;
		}
		else
		{
			uiFlags       = a_bLevelStart ? 4 : ( a_bRespawn ? 8 : 1 );
			m_pCurrSpline = rNedCameras.FindSplineCamAt( vBlobPos, uiFlags );

			if ( m_pCurrSpline == TNULL && uiFlags != 1 )
			{
				uiFlags       = 1;
				m_pCurrSpline = rNedCameras.FindSplineCamAt( vBlobPos, uiFlags );
			}

			if ( m_pCurrSpline == TNULL )
			{
				// Behind the blob
				TVector4 vBehind = m_pBlob->m_vFacing;
				vBehind.NormaliseXZ();
				vBehind.x = -vBehind.x;
				vBehind.y = -vBehind.y;
				vBehind.z = -vBehind.z;

				TVector4 vOffset;
				vOffset.Multiply( vBehind, 15.0f );

				// 130 units below the blob
				TVector4 vPos( vOffset.x + vBlobPos.x, ( vOffset.y + vBlobPos.y ) - 130.0f, vOffset.z + vBlobPos.z, vBlobPos.w );
				m_Matrix.m_mMatrix.SetTranslation( vPos );
				LookAtPoint( vBlobPos );
				bDefaultFOV = TTRUE;
			}
			else
			{
				m_uiSplineFlags = uiFlags;
				fFOV            = TMath::HALF_PI;

				const ASSCameraFunction::Camera* pCam = ACameraManager::GetSingleton()->GetSSCameras().FindCamera( m_pCurrSpline->m_iSSCam );
				TQuaternion                      qOrient;
				ASSCameraFunction::GetCameraTransform( pCam, pCam->m_vLineStart, pCam->m_vLineEnd, vBlobPos, vCamPos, m_vIdealPos, qOrient, fFOV, m_pCurrSpline->m_fRadius, m_pCurrSpline->m_fMinOffset, m_pCurrSpline->m_fOffsetAngle, 0.0f, m_pCurrSpline->m_uiFlags );
				m_vUnk118 = vBlobPos;

				if ( m_pCurrSpline->m_uiFlags & 0x4000 )
				{
					m_Matrix.m_mMatrix.SetTranslation( m_vIdealPos );
					LookAtPoint( vBlobPos );
				}
				else
				{
					TMatrix44 mMatrix;
					mMatrix.SetFromQuaternion( qOrient );
					mMatrix.SetTranslation( m_vIdealPos );
					SetMatrix( mMatrix );
				}
			}
		}
	}
	else
	{
		// The player's start point
		LookAtPoint( vBlobPos );
		bDefaultFOV = TTRUE;
	}

	if ( bDefaultFOV )
	{
		m_Matrix.m_fFOV    = sm_fDefaultFOV;
		m_Matrix.m_fAspect = TFLOAT( tan( TDOUBLE( sm_fDefaultFOV * 0.5f ) ) );
	}
	else
	{
		m_Matrix.m_fFOV    = fFOV;
		m_Matrix.m_fAspect = TFLOAT( tan( fFOV * 0.5f ) );
	}

	m_fLastFOV     = m_Matrix.m_fFOV;
	m_vIdealPos    = m_Matrix.m_mMatrix.GetTranslation();
	m_pOverrideCam = TNULL;
	m_bRotBlend    = TFALSE;
	m_vSmoothPos   = m_Matrix.m_mMatrix.GetTranslation();
	m_mPrevMatrix  = m_Matrix.m_mMatrix;
	m_mLastOrient  = m_Matrix.m_mMatrix;
}

// Takes over the matrix of a_pCamera (e.g. the scripted camera it follows)
// $deBlob: FUNCTION 0058be10
void ABlobFollowCamera::SetFromCamera( ACamera* a_pCamera )
{
	const TMatrix44& rMatrix = a_pCamera->GetMatrix();
	m_mLastOrient            = rMatrix;
	m_mPrevMatrix            = rMatrix;
	m_vIdealPos              = rMatrix.GetTranslation();
	m_vSmoothPos             = rMatrix.GetTranslation();
	SetMatrix( rMatrix );
}

// $deBlob: FUNCTION 0058beb0
void ABlobFollowCamera::SetOverrideCamera( ACamera* a_pCamera )
{
	m_pOverrideCam = a_pCamera;
}

// $deBlob: FUNCTION 0058bec0
void ABlobFollowCamera::StartRotationBlend()
{
	m_bRotBlend       = TTRUE;
	m_fRotBlend       = 2.0f;
	m_fRotBlendTarget = 30.0f;
	m_fRotBlendTimer  = 0.0f;
}

// $deBlob: FUNCTION 0058bef0
void ABlobFollowCamera::InitialiseSettings()
{
	ms_fCollisionRadius     = 0.5f;
	ms_fDefaultMaxDist      = 45.0f;
	ms_fDefaultIdealDist    = 40.0f;
	ms_fUnkF18              = 0.94247788f;
	ms_fAcceptableIdealDist = 25.0f;
	ms_fDefaultMinDist      = 7.0f;
	ms_fRotSpeed            = 3.5f;
	ms_fUnkBC2C             = 0.25f;
	ms_fUnkBC30             = 0.25f;
	ms_fLiquidPitch         = 2.04f;
	ms_fMinPitch            = 2.04f;
	ms_fMaxPitch            = 2.04f;
	ms_fOrbitPitch          = 2.04f;
	ms_fYawFactor           = 0.3f;
	ms_fPitchFactor         = 0.75f;
	ms_fFOVMin              = 1.0471976f;

	ms_aPosSpeeds[ PS_FOLLOW ]         = { 150.0f, 500.0f, 1.0f, 5.0f };
	ms_aPosSpeeds[ PS_3 ]              = { 200.0f, 200.0f, 0.5f, 6.0f };
	ms_aPosSpeeds[ PS_3_FLAG80 ]       = { 400.0f, 300.0f, 2.0f, 6.0f };
	ms_aPosSpeeds[ PS_NEDFIXED ]       = { 350.0f, 300.0f, 1.0f, 6.0f };
	ms_aPosSpeeds[ PS_NEDFIXED_10_20 ] = { 450.0f, 400.0f, 0.35f, 12.0f };
	ms_aPosSpeeds[ PS_NEDFIXED_2 ]     = { 450.0f, 400.0f, 0.35f, 12.0f };
	ms_aPosSpeeds[ PS_SPLINE ]         = { 100.0f, 150.0f, 5.0f, 3.0f };
	ms_aPosSpeeds[ PS_1 ]              = { 100.0f, 150.0f, 5.0f, 3.0f };
	ms_aPosSpeeds[ PS_2 ]              = { 100.0f, 150.0f, 5.0f, 3.0f };
	ms_aPosSpeeds[ PS_7 ]              = { 400.0f, 450.0f, 1.0f, 10.0f };
	ms_aPosSpeeds[ PS_UNUSED ]         = { 200.0f, 200.0f, 5.0f, 3.0f };
	ms_aPosSpeeds[ PS_8 ]              = { 400.0f, 450.0f, 1.0f, 10.0f };
	ms_aPosSpeeds[ PS_9 ]              = { 400.0f, 450.0f, 1.0f, 10.0f };

	ms_fOrbitSpeed    = 0.39269909f;
	ms_fFOVMax        = 1.7453293f;
	ms_fFOVBlendSpeed = 0.39269909f;

	ms_aPosSpeeds[ PS_10 ] = { 400.0f, 450.0f, 1.0f, 10.0f };
}

// $deBlob: FUNCTION 0058b900
void ABlobFollowCamera::OnSuspend()
{
	Reset( TTRUE, TFALSE );
}

// $deBlob: FUNCTION 0058b910
void ABlobFollowCamera::Invalidate()
{
	m_bUnk1C1 = TFALSE;
}

// $deBlob: FUNCTION 0058b920
void ABlobFollowCamera::OnDeactivate()
{
}

// $deBlob: FUNCTION 0058b930
void ABlobFollowCamera::OnDestroy()
{
}

// $deBlob: FUNCTION 0058b8b0
void ABlobFollowCamera::DestroyPhantoms()
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( m_pPhantom1 != TNULL )
	{
		pPhysicsManager->DestroyPhantom( TSTATICCAST( TPhysicsPhantomContainer, m_pPhantom1 ) );
		m_pPhantom1 = TNULL;
	}

	if ( m_pPhantom2 != TNULL )
	{
		pPhysicsManager->DestroyPhantom( TSTATICCAST( TPhysicsPhantomContainer, m_pPhantom2 ) );
		m_pPhantom2 = TNULL;
	}

	m_pBlob = TNULL;
}

// $deBlob: FUNCTION 0058c410
void ABlobFollowCamera::GetNedFixedSpeeds( TFLOAT* a_pfAccel, TFLOAT* a_pfMaxSpeed, TFLOAT* a_pfDampenDist, TFLOAT* a_pfLerpSpeed, TINT a_iFlags )
{
	*a_pfAccel      = ms_aPosSpeeds[ PS_NEDFIXED ].m_fAccel;
	*a_pfMaxSpeed   = ms_aPosSpeeds[ PS_NEDFIXED ].m_fMaxSpeed;
	*a_pfDampenDist = ms_aPosSpeeds[ PS_NEDFIXED ].m_fDampenDist;
	*a_pfLerpSpeed  = ms_aPosSpeeds[ PS_NEDFIXED ].m_fLerpSpeed;

	switch ( a_iFlags )
	{
		case 2:
			*a_pfAccel      = ms_aPosSpeeds[ PS_NEDFIXED_2 ].m_fAccel;
			*a_pfMaxSpeed   = ms_aPosSpeeds[ PS_NEDFIXED_2 ].m_fMaxSpeed;
			*a_pfDampenDist = ms_aPosSpeeds[ PS_NEDFIXED_2 ].m_fDampenDist;
			*a_pfLerpSpeed  = ms_aPosSpeeds[ PS_NEDFIXED_2 ].m_fLerpSpeed;
			break;
		case 0x10:
		case 0x20:
			*a_pfAccel      = ms_aPosSpeeds[ PS_NEDFIXED_10_20 ].m_fAccel;
			*a_pfMaxSpeed   = ms_aPosSpeeds[ PS_NEDFIXED_10_20 ].m_fMaxSpeed;
			*a_pfDampenDist = ms_aPosSpeeds[ PS_NEDFIXED_10_20 ].m_fDampenDist;
			*a_pfLerpSpeed  = ms_aPosSpeeds[ PS_NEDFIXED_10_20 ].m_fLerpSpeed;
			break;
	}
}

// $deBlob: FUNCTION 0058c4d0
TBOOL ABlobFollowCamera::MoveTowards( TFLOAT* a_pfOut, TFLOAT a_fValue, TFLOAT a_fTarget, TFLOAT a_fSpeedDown, TFLOAT a_fSpeedUp, TFLOAT a_fDeltaTime )
{
	TFLOAT fDiff = a_fTarget - a_fValue;

	if ( 0.0f <= fDiff )
	{
		if ( 0.0f < fDiff )
		{
			*a_pfOut = fDiff * a_fSpeedUp * a_fDeltaTime + a_fValue;

			if ( *a_pfOut <= a_fTarget )
				return TFALSE;
		}
	}
	else
	{
		*a_pfOut = fDiff * a_fSpeedDown * a_fDeltaTime + a_fValue;

		if ( a_fTarget <= *a_pfOut )
			return TFALSE;
	}

	*a_pfOut = a_fTarget;
	return TTRUE;
}

#include "pch.h"
#include "AKeyFrameCamera.h"
#include "ACameraManager.h"
#include "GameInterface/APlayerManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameController/AGameStatus.h"
#include "Entities/AEntityHash.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/APropTriggerHelp.h"
#include "Input/ABlobInputMap.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Challenges/AChallengeManager.h"

#include <Math/TMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The smoothing speeds of UpdateKeyFrame (per second) and UpdateEntityHash's interval
static TCONSTEXPR_VAR TFLOAT s_fMinRateSpeed      = 8.0f;
static TCONSTEXPR_VAR TFLOAT s_fOrientSpeed       = 2.5f;
static TCONSTEXPR_VAR TFLOAT s_fCameraOrientSpeed = 2.5f;
static TCONSTEXPR_VAR TFLOAT s_fPosSpeed          = 2.0f;
static TCONSTEXPR_VAR TFLOAT s_fHelpInterval      = 0.2f;
static TCONSTEXPR_VAR TFLOAT s_fHelpRadius        = 10.0f;

static TCONSTEXPR_VAR TINT s_iSkipButton = ABlobInputMap::BINDING_SKIP;

// The helps of UpdateEntityHash (its local AEntityHashFilterFunctorHelpOnly)
class AEntityHashFilterFunctorHelpOnly : public AEntityHashFilter
{
public:
	// $deBlob: FUNCTION 005932a0
	virtual TBOOL operator()( ABaseEntity* a_pEntity ) override
	{
		return a_pEntity->GetClass() == &TGetClass( APropTriggerHelp );
	}
};

// The key frames are the same (position and orientation within a_fEpsilon)
// $deBlob: FUNCTION 00593120
static TBOOL IsSameKeyFrame( const ASSCameraFunction::KeyFrame* a_pFrame1, const ASSCameraFunction::KeyFrame* a_pFrame2, TFLOAT a_fEpsilon )
{
	if ( a_pFrame1 != TNULL && a_pFrame2 != TNULL &&
	     TMath::Abs( a_pFrame1->m_qOrient.x - a_pFrame2->m_qOrient.x ) < a_fEpsilon &&
	     TMath::Abs( a_pFrame1->m_qOrient.y - a_pFrame2->m_qOrient.y ) < a_fEpsilon &&
	     TMath::Abs( a_pFrame1->m_qOrient.z - a_pFrame2->m_qOrient.z ) < a_fEpsilon &&
	     TMath::Abs( a_pFrame1->m_qOrient.w - a_pFrame2->m_qOrient.w ) < a_fEpsilon )
	{
		if ( TVector4::DistanceSq( a_pFrame1->m_vPos, a_pFrame2->m_vPos ) < a_fEpsilon )
			return TTRUE;
	}

	return TFALSE;
}

// The orientations are the same within a_fEpsilon
// $deBlob: FUNCTION 005931e0
static TBOOL IsSameOrientation( const TQuaternion& a_rq1, const TQuaternion& a_rq2, TFLOAT a_fEpsilon )
{
	return TMath::Abs( a_rq1.x - a_rq2.x ) < a_fEpsilon && TMath::Abs( a_rq1.y - a_rq2.y ) < a_fEpsilon &&
	    TMath::Abs( a_rq1.z - a_rq2.z ) < a_fEpsilon && TMath::Abs( a_rq1.w - a_rq2.w ) < a_fEpsilon;
}

// The positions are the same within a_fEpsilon
// $deBlob: FUNCTION 00593270
TBOOL IsSamePosition( const TVector4& a_rv1, const TVector4& a_rv2, TFLOAT a_fEpsilon )
{
	return TVector4::DistanceSq( a_rv1, a_rv2 ) < a_fEpsilon;
}

// Normalises a slerped quaternion
static void NormaliseQuat( TQuaternion& a_rq )
{
	a_rq /= TMath::Sqrt( a_rq.x * a_rq.x + a_rq.y * a_rq.y + a_rq.z * a_rq.z + a_rq.w * a_rq.w );
}

// $deBlob: FUNCTION 00591960
// $deBlob: FUNCTION 00591970
// $deBlob: FUNCTION 005919e0
// $deBlob: FUNCTION 00591a40
// $deBlob: FUNCTION 00591a50
// $deBlob: FUNCTION 0053b9b0
// $deBlob: FUNCTION 00857f90
TDEFINE_CLASS( AKeyFrameCamera );

TSTATICASSERT( sizeof( AKeyFrameCamera ) == 0x184 );
TSTATICASSERT( sizeof( ASSCameraFunction::KeyFrame ) == 0x50 && sizeof( ASSCameraFunction::Camera ) == 0x50 );

// $deBlob: FUNCTION 00591a60
AKeyFrameCamera::AKeyFrameCamera()
    : ACamera( CT_KEYFRAME )
{
	m_pSSCam      = TNULL;
	m_fTime       = 0.0f;
	m_fTotalTime  = 0.0f;
	m_bPlaying    = TFALSE;
	m_bSkipped    = TFALSE;
	m_bSnap       = TFALSE;
	m_bPath       = TFALSE;
	m_pCurrentKF  = TNULL;
	m_pNextKF     = TNULL;
	m_pNextNextKF = TNULL;
	m_fHelpTimer  = -1.0f;

	TMatrix44::MatToQuat( m_oKeyFrame.m_qOrient, m_Matrix.m_mMatrix );
	m_oKeyFrame.m_fFOV = m_Matrix.m_fFOV;
	m_qOrient          = m_oKeyFrame.m_qOrient;
}

// $deBlob: FUNCTION 005932f0
// $deBlob: FUNCTION 00591b30
AKeyFrameCamera::~AKeyFrameCamera()
{
	m_pSSCam = TNULL;
}

// $deBlob: FUNCTION 00591b50
void AKeyFrameCamera::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bPlaying )
	{
		// The time only runs while the cells around the streaming position are loaded
		TBOOL bAreaReady = ATerrain2Interface::GetInstance()->IsCurrentCellAreaReady();

		if ( bAreaReady )
			m_fTime += a_fDeltaTime;

		if ( m_fTotalTime <= m_fTime )
		{
			if ( m_pSSCam == TNULL || m_pSSCam->m_iNextCamera < 0 )
			{
				m_fTime    = m_fTotalTime;
				m_bPlaying = TFALSE;
			}
			else
			{
				// Plays the chained camera
				TBOOL bSkipped = m_bSkipped;
				SetCamera( m_pSSCam->m_iNextCamera );
				Play();
				m_bSkipped = bSkipped;
				bAreaReady = ATerrain2Interface::GetInstance()->IsCurrentCellAreaReady();
			}
		}

		UpdateEntityHash( a_fDeltaTime );
		SetTime( m_fTime );

		if ( m_bPath )
			UpdatePath( a_fDeltaTime );
		else
			UpdateKeyFrame( a_fDeltaTime );

		UpdateFade( bAreaReady );
	}

	UpdateSkip();
}

// $deBlob: FUNCTION 00591c50
void AKeyFrameCamera::UpdateKeyFrame( TFLOAT a_fDeltaTime )
{
	TMatrix44 mat;
	TVector4  vPos;

	if ( m_bSnap )
	{
		m_qOrient = m_oKeyFrame.m_qOrient;
		mat.SetFromQuaternion( m_oKeyFrame.m_qOrient );
		vPos    = m_oKeyFrame.m_vPos;
		m_bSnap = TFALSE;
	}
	else
	{
		// The camera keeps up with the key frames at least at this rate: while the next two key frames are the same
		// (a hold) it closes in as the key frame gets there, when the current and next ones are the same it snaps
		TFLOAT fMinRequiredRate = 0.0f;

		if ( m_pNextKF != m_pNextNextKF && IsSameKeyFrame( m_pNextKF, m_pNextNextKF, 0.001f ) )
		{
			TFLOAT fDistLeft  = TVector4::Distance( m_oKeyFrame.m_vPos, m_pNextKF->m_vPos );
			TFLOAT fDistTotal = TVector4::Distance( m_pCurrentKF->m_vPos, m_pNextKF->m_vPos );

			if ( fDistLeft < fDistTotal )
			{
				if ( 0.0f < fDistTotal )
					fMinRequiredRate = 1.0f - fDistLeft / fDistTotal;
			}
			else
			{
				fMinRequiredRate = 0.0f;
			}
		}
		else if ( IsSameKeyFrame( m_pNextKF, m_pCurrentKF, 0.001f ) )
		{
			fMinRequiredRate = 1.0f;
		}

		TASSERT( fMinRequiredRate >= 0.0f && fMinRequiredRate <= 1.0f );

		TFLOAT fMinRate = fMinRequiredRate * a_fDeltaTime * s_fMinRateSpeed;
		TMath::Clip( fMinRate, 0.0f, 1.0f );

		// The smoothed orientation follows the key frame's
		TFLOAT fOrientRate = s_fOrientSpeed * a_fDeltaTime;

		if ( fOrientRate < fMinRate )
			fOrientRate = fMinRate;

		TMath::Clip( fOrientRate, 0.0f, 1.0f );

		TQuaternion qOrient;
		TQuaternion::Slerp( qOrient, m_qOrient, m_oKeyFrame.m_qOrient, fOrientRate );
		NormaliseQuat( qOrient );
		m_qOrient = qOrient;

		// The camera follows the smoothed orientation
		TMatrix44   mCurrent = m_Matrix.m_mMatrix;
		TQuaternion qCurrent;
		TMatrix44::MatToQuat( qCurrent, mCurrent );

		TFLOAT fCameraRate = s_fCameraOrientSpeed * a_fDeltaTime;

		if ( fCameraRate < fMinRate )
			fCameraRate = fMinRate;

		TMath::Clip( fCameraRate, 0.0f, 1.0f );

		TQuaternion qCamera;
		TQuaternion::Slerp( qCamera, qCurrent, m_qOrient, fCameraRate );
		NormaliseQuat( qCamera );
		mat.SetFromQuaternion( qCamera );

		// And the key frame's position
		TVector4 vCurrentPos = mCurrent.GetTranslation();
		TFLOAT   fPosRate    = s_fPosSpeed * a_fDeltaTime;

		if ( fPosRate < fMinRate )
			fPosRate = fMinRate;

		TMath::Clip( fPosRate, 0.0f, 1.0f );
		vPos.Lerp4( vCurrentPos, m_oKeyFrame.m_vPos, fPosRate );
	}

	mat.SetTranslation( vPos );
	TASSERT( mat.IsOrthogonal() );
	TASSERT( mat.IsOrthonormal() );
	SetMatrix( mat );

	m_Matrix.m_fFOV    = m_oKeyFrame.m_fFOV;
	m_Matrix.m_fAspect = TFLOAT( tan( m_oKeyFrame.m_fFOV * 0.5f ) );
}

// $deBlob: FUNCTION 00592250
void AKeyFrameCamera::UpdatePath( TFLOAT a_fDeltaTime )
{
	TMatrix44 mCurrent = m_Matrix.m_mMatrix;
	TMatrix44 mat;

	if ( m_bSnap )
	{
		m_qOrient = m_oKeyFrame.m_qOrient;
		mat.SetFromQuaternion( m_oKeyFrame.m_qOrient );
		mat.SetTranslation( m_oKeyFrame.m_vPos );
		TASSERT( mat.IsOrthogonal() );
		TASSERT( mat.IsOrthonormal() );
		SetMatrix( mat );

		m_Matrix.m_fFOV    = m_oKeyFrame.m_fFOV;
		m_bSnap            = TFALSE;
		m_Matrix.m_fAspect = TFLOAT( tan( m_oKeyFrame.m_fFOV * 0.5f ) );
	}
	else
	{
		TQuaternion qCurrent;
		TMatrix44::MatToQuat( qCurrent, mCurrent );
		TVector4 vCurrentPos = mCurrent.GetTranslation();

		// Close enough to the key frame (and the key frames don't move) the camera stays, else it follows them
		TBOOL bFar        = m_pSSCam->m_fSnapDistSq <= TVector4::DistanceSq( vCurrentPos, m_oKeyFrame.m_vPos );
		TBOOL bSameOrient = IsSameOrientation( qCurrent, m_oKeyFrame.m_qOrient, m_pSSCam->m_fSnapOrient );
		TBOOL bHold       = IsSameKeyFrame( m_pCurrentKF, m_pNextKF, 0.001f );

		TVector4    vPos;
		TQuaternion qOrient;

		if ( bFar || !bSameOrient || !bHold )
		{
			TFLOAT fPosRate = m_pSSCam->m_fPosRate * a_fDeltaTime;
			TMath::Clip( fPosRate, 0.0f, 1.0f );
			vPos.Lerp4( vCurrentPos, m_oKeyFrame.m_vPos, fPosRate );

			TFLOAT fOrientRate = m_pSSCam->m_fOrientRate * a_fDeltaTime;
			TMath::Clip( fOrientRate, 0.0f, 1.0f );
			TQuaternion::Slerp( qOrient, qCurrent, m_oKeyFrame.m_qOrient, fOrientRate );
		}
		else
		{
			vPos    = vCurrentPos;
			qOrient = qCurrent;
		}

		NormaliseQuat( qOrient );
		mat.SetFromQuaternion( qOrient );
		mat.SetTranslation( vPos );
	}

	TASSERT( mat.IsOrthogonal() );
	TASSERT( mat.IsOrthonormal() );
	SetMatrix( mat );

	m_Matrix.m_fFOV    = m_oKeyFrame.m_fFOV;
	m_Matrix.m_fAspect = TFLOAT( tan( m_oKeyFrame.m_fFOV * 0.5f ) );
}

// $deBlob: FUNCTION 00592750
void AKeyFrameCamera::UpdateEntityHash( TFLOAT a_fDeltaTime )
{
	if ( m_pSSCam == TNULL || !( m_pSSCam->m_uiFlags & ASSCameraFunction::CF_HELP ) )
		return;

	m_fHelpTimer -= a_fDeltaTime;

	if ( 0.0f < m_fHelpTimer )
		return;

	AEntityHashFilterFunctorHelpOnly oFilter;
	TINT                             iCount;
	ABaseEntity**                    ppEntities = AEntityHash::GetSingleton()->Query( m_Matrix.m_mMatrix.GetTranslation(), s_fHelpRadius, &iCount, &oFilter, TTRUE, AEntityHash::MAX_QUERY_RESULTS, TFALSE, TTRUE, TFALSE );

	for ( TUINT i = 0; i < TUINT( iCount ); i++ )
	{
		ABaseEntity* pEntity = ppEntities[ i ];
		TASSERT( pEntity->IsExactly( &TGetClass( APropTriggerHelp ) ) );

		APropTriggerHelp* pHelp = TSTATICCAST( APropTriggerHelp, pEntity );

		// The letterbox helps of this camera (or of any)
		if ( pHelp->GetHelpType() == APropTriggerHelp::HELPTYPE_LetterBox && ( pHelp->GetSSCamID() == -1 || pHelp->GetSSCamID() == m_pSSCam->m_iID ) )
		{
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
			oPacket.m_iData    = 0;
			pHelp->HandleEntityIO( TNULL, oPacket );
		}
	}

	m_fHelpTimer = s_fHelpInterval;
}

// $deBlob: FUNCTION 00592880
void AKeyFrameCamera::UpdateSkip()
{
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	ABaseEntity*    pBlob          = pPlayerManager->GetBlobOfCamera( this );

	if ( pBlob == TNULL )
		return;

	TINT iController = pPlayerManager->GetBlobController( pBlob );

	if ( iController >= 0 && ABlobInputMap::GetSingleton()->IsButtonPressed( iController, s_iSkipButton ) )
		Skip();
}

// $deBlob: FUNCTION 005928e0
void AKeyFrameCamera::UpdateFade( TBOOL a_bAreaReady )
{
	if ( m_pSSCam == TNULL || m_pCurrentKF == TNULL || m_bSkipped )
		return;

	TINT iViewport = ARenderer::GetSingleton()->GetViewportManager()->GetCameraViewport( this );

	if ( iViewport < 0 )
		return;

	AGameStateController* pController = AGameStateController::GetSingleton();

	TBOOL bIsFadingtoBlack   = pController->IsFadingToBlack( iViewport );
	TBOOL bIsFadingFromBlack = pController->IsFadingFromBlack( iViewport );
	TASSERT( !( bIsFadingtoBlack && bIsFadingFromBlack ) );

	TFLOAT fFade = m_oKeyFrame.m_fFade;

	if ( bIsFadingtoBlack )
		return;

	if ( bIsFadingFromBlack )
	{
		// A fade from black only gets darker through the key frame
		if ( fFade <= pController->GetFade( iViewport ) )
			return;

		pController->StopFade( iViewport );
	}

	pController->SetFade( fFade, iViewport );
}

// $deBlob: FUNCTION 005929e0
TBOOL AKeyFrameCamera::SetCamera( TINT a_iCamera )
{
	const ASSCameraFunction::Camera* pCam = TNULL;

	if ( a_iCamera >= 0 && ACameraManager::GetSingleton() != TNULL )
		pCam = ACameraManager::GetSingleton()->GetSSCameras().FindCamera( a_iCamera );

	return SetCamera( pCam );
}

// $deBlob: FUNCTION 00592a20
TBOOL AKeyFrameCamera::SetCamera( const ASSCameraFunction::Camera* a_pCam )
{
	m_pSSCam     = TNULL;
	m_fTime      = 0.0f;
	m_fTotalTime = 0.0f;
	m_bSnap      = TFALSE;
	m_bPath      = TFALSE;

	if ( a_pCam == TNULL )
		return TFALSE;

	m_bPath = ( a_pCam->m_uiFlags & ASSCameraFunction::CF_PATH ) != 0;

	if ( !m_bPath )
	{
		const ASSCameraFunction::KeyFrame* pCurrent = TNULL;
		const ASSCameraFunction::KeyFrame* pPrev    = TNULL;

		if ( !ASSCameraFunction::GetKeyFrame( a_pCam, 0.0f, &m_oKeyFrame, &pCurrent, &pPrev, &m_pNextKF, &m_pNextNextKF ) )
			return TFALSE;

		// NOTE: m_pSSCam was cleared above, so the previous camera is always TNULL
		APlayerManager::GetSingleton()->OnSSCameraChanged( m_pSSCam, a_pCam );
		m_pSSCam     = a_pCam;
		m_fTotalTime = ASSCameraFunction::GetTotalTime( a_pCam );

		if ( m_pCurrentKF != pCurrent )
		{
			m_pCurrentKF = pCurrent;

			// A key frame that starts the camera or has no time to the previous one is jumped to
			if ( pCurrent != TNULL && ( pCurrent->m_fTime <= 0.0f || ( pPrev != TNULL && pCurrent->m_fTime - pPrev->m_fTime <= 0.0f ) ) )
			{
				m_bSnap   = TTRUE;
				m_qOrient = m_oKeyFrame.m_qOrient;
			}
		}

		SetTime( m_fTime );
		m_bSnap = TTRUE;
		UpdateKeyFrame( 0.0f );
		return TTRUE;
	}

	m_pNextNextKF = TNULL;
	m_pNextKF     = TNULL;
	m_pCurrentKF  = TNULL;
	m_bSnap       = TTRUE;

	if ( !ASSCameraFunction::GetPathKeyFrame( a_pCam, 0.0f, &m_oKeyFrame, &m_pCurrentKF, &m_pNextKF, &m_pNextNextKF ) )
		return TFALSE;

	m_pSSCam = a_pCam;

	if ( 0 < a_pCam->m_iCount )
	{
		// The time of the path without the skipped start and end
		m_fTotalTime = a_pCam->m_pKeyFrames[ a_pCam->m_iCount - 1 ].m_fTime;

		if ( 1 < a_pCam->m_iCount )
		{
			m_fTotalTime -= a_pCam->m_pKeyFrames[ 1 ].m_fUnk28;
			m_fTotalTime -= a_pCam->m_pKeyFrames[ a_pCam->m_iCount - 1 ].m_fUnk28;
		}

		if ( m_fTotalTime <= 0.0f )
			m_fTotalTime = 0.0f;
	}

	SetTime( m_fTime );
	m_bSnap = TTRUE;
	UpdatePath( 0.0f );
	return TTRUE;
}

// $deBlob: FUNCTION 00592c40
void AKeyFrameCamera::Play()
{
	m_pCurrentKF  = TNULL;
	m_pNextKF     = TNULL;
	m_pNextNextKF = TNULL;
	m_fTime       = 0.0f;
	m_bPlaying    = TTRUE;
	m_bSkipped    = TFALSE;
	m_fHelpTimer  = s_fHelpInterval;

	SetTime( 0.0f );
	m_bSnap = TTRUE;

	if ( m_bPath )
		UpdatePath( 0.0f );
	else
		UpdateKeyFrame( 0.0f );
}

// $deBlob: FUNCTION 00592cc0
void AKeyFrameCamera::Stop()
{
	m_fHelpTimer  = -1.0f;
	m_bPlaying    = TFALSE;
	m_bSkipped    = TFALSE;
	m_pCurrentKF  = TNULL;
	m_pNextKF     = TNULL;
	m_pNextNextKF = TNULL;
	m_fTime       = 0.0f;
}

// $deBlob: FUNCTION 00592d00
void AKeyFrameCamera::Pause()
{
	m_bPlaying = TFALSE;
}

// $deBlob: FUNCTION 00592d10
void AKeyFrameCamera::Resume()
{
	m_fHelpTimer = s_fHelpInterval;
	m_bPlaying   = TTRUE;
	m_bSkipped   = TFALSE;

	SetTime( m_fTime );
	m_bSnap = TTRUE;

	if ( m_bPath )
		UpdatePath( 0.0f );
	else
		UpdateKeyFrame( 0.0f );
}

// $deBlob: FUNCTION 00592d70
void AKeyFrameCamera::Rewind()
{
	m_bPlaying    = TFALSE;
	m_bSkipped    = TFALSE;
	m_pCurrentKF  = TNULL;
	m_pNextKF     = TNULL;
	m_pNextNextKF = TNULL;
	m_fTime       = 0.0f;
}

// $deBlob: FUNCTION 00592db0
void AKeyFrameCamera::Skip()
{
	const ASSCameraFunction::Camera* pCam = m_pSSCam;
	m_bSkipped                            = TTRUE;

	if ( pCam != TNULL )
	{
		// The helps of the chained cameras
		const ASSCameraFunction::Camera* pNext;

		do
		{
			TINT iNext = pCam->m_iNextCamera;

			if ( iNext < 0 )
				break;

			pNext = ACameraManager::GetSingleton()->GetSSCameras().FindCamera( iNext );

			if ( pNext != TNULL )
				APlayerManager::GetSingleton()->OnSSCameraChanged( pCam, pNext );

			pCam = pNext;
		} while ( pNext != TNULL );
	}

	AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;

	AChallengeManager::GetSingleton()->SendSkipToCurrentChallenge();
}

// $deBlob: FUNCTION 00592e10
void AKeyFrameCamera::SetKeyFrame( TINT a_iKeyFrame )
{
	TASSERT( a_iKeyFrame < m_pSSCam->m_iCount && a_iKeyFrame >= 0 );

	if ( m_pSSCam != TNULL )
	{
		// NOTE: retail clips to m_iCount, not m_iCount - 1
		TMath::Clip( a_iKeyFrame, 0, m_pSSCam->m_iCount );
		m_oKeyFrame = m_pSSCam->m_pKeyFrames[ a_iKeyFrame ];
		m_fTime     = m_oKeyFrame.m_fTime;
	}
}

// $deBlob: FUNCTION 00592ee0
void AKeyFrameCamera::SetTime( TFLOAT a_fTime )
{
	TASSERT( a_fTime >= 0 && a_fTime <= m_fTotalTime );

	if ( m_pSSCam == TNULL )
		return;

	TMath::Clip( a_fTime, 0.0f, m_fTotalTime );
	m_fTime = a_fTime;

	const ASSCameraFunction::KeyFrame* pCurrent;

	if ( !m_bPath )
	{
		const ASSCameraFunction::KeyFrame* pPrev = TNULL;

		TBOOL bOk = ASSCameraFunction::GetKeyFrame( m_pSSCam, a_fTime, &m_oKeyFrame, &pCurrent, &pPrev, &m_pNextKF, &m_pNextNextKF );
		TASSERT( bOk );

		if ( m_pCurrentKF == pCurrent )
			return;

		m_pCurrentKF = pCurrent;

		if ( pCurrent == TNULL )
			return;

		// A key frame that starts the camera or has no time to the previous one is jumped to
		if ( 0.0f < pCurrent->m_fTime )
		{
			if ( pPrev == TNULL )
				return;

			if ( 0.0f < pCurrent->m_fTime - pPrev->m_fTime )
				return;
		}
	}
	else
	{
		// The path's time starts after the second key frame's skipped time
		TFLOAT fPathTime = a_fTime;

		if ( 1 < m_pSSCam->m_iCount )
			fPathTime = m_pSSCam->m_pKeyFrames[ 1 ].m_fUnk28 + a_fTime;

		m_pNextKF = TNULL;

		TBOOL bOk = ASSCameraFunction::GetPathKeyFrame( m_pSSCam, fPathTime, &m_oKeyFrame, &pCurrent, &m_pNextKF, &m_pNextNextKF );
		TASSERT( bOk );

		if ( m_pCurrentKF == pCurrent )
			return;

		m_pCurrentKF = pCurrent;

		if ( pCurrent == TNULL )
			return;

		if ( 0.0f < pCurrent->m_fUnk28 )
			return;
	}

	m_bSnap = TTRUE;
}

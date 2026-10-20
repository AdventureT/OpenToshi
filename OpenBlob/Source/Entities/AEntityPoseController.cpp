#include "pch.h"
#include "AEntityPoseController.h"
#include "AEntityStateComponent.h"
#include "AEntityModelComponent.h"
#include "AEntityData.h"
#include "AEntityFactory.h"
#include "ABaseEntity.h"
#include "ARandom.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "Animation/AAnimationPose.h"

#include <Render/TAnimation.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007c4f40
AEntityPoseController::AEntityPoseController( ABaseEntity* a_pEntity, AAnimationPoseList* a_pPoseInformation )
    : AEntityComponent2( a_pEntity )
{
	m_iCurrentState     = -1;
	m_iCurrentPose      = -1;
	m_iCurrentAnim      = -1;
	m_CurrentMult       = 1.0f;
	m_pPoseInformation  = a_pPoseInformation;
	m_bNoBlend          = TFALSE;
	m_ManualOverlayAnim = -1;

	AEntityTypeData* pTypeData = a_pEntity->GetTypeData();

	TCHAR szSymbol[ 1024 ];
	T2String8::Format( szSymbol, "%s_PoseMap_Main", AEntityData::GetDirectoryName( pTypeData->m_strName.GetString() ) );

	m_pPoseMap = TSTATICCAST( PoseMap, AEntityData::GetSingleton()->GetPoseMapData( szSymbol ) );
}

// The model's animation interface
AToshiAnimInterface2* AEntityPoseController::GetAnimInterface() const
{
	return TSTATICCAST( AEntityModelComponent, m_pEntity->GetModelComponent() )->GetModelInstance()->GetAnimInterface();
}

// $deBlob: FUNCTION 007c5060
void AEntityPoseController::Tick( TFLOAT a_fDeltaTime )
{
	TINT iCurrentState = m_iCurrentState;

	if ( iCurrentState != m_pEntity->GetState() )
	{
		SetPoseForState( iCurrentState, m_pEntity->GetState() );
		m_iCurrentState = m_pEntity->GetState();
		m_bNoBlend      = TFALSE;
		return;
	}

	if ( iCurrentState >= 0 )
	{
		AEntityStateComponent* pStateComponent = TSTATICCAST( AEntityStateComponent, m_pEntity->GetStateComponent() );

		if ( pStateComponent->IsStateLooping( iCurrentState ) )
			UpdateAnimLoop();
	}
}

// $deBlob: FUNCTION 007c5100
TFLOAT AEntityPoseController::GetPoseLength()
{
	TASSERT( TMath::Abs( m_CurrentMult ) > 0 );

	if ( m_iCurrentAnim >= 0 )
		return GetAnimInterface()->GetAnimationLength( TINT16( m_iCurrentAnim ) ) / m_CurrentMult;

	if ( m_iCurrentPose >= 0 )
	{
		AToshiAnimInterface2* pAnimInterface = GetAnimInterface();
		return pAnimInterface->GetPoseLength( m_pPoseInformation->GetPose( m_iCurrentPose ) ) / m_CurrentMult;
	}

	return 0.0f;
}

// $deBlob: FUNCTION 007c51b0
void AEntityPoseController::SetStateDirty()
{
	m_iCurrentState = -1;
	m_iCurrentAnim  = -1;
	m_iCurrentPose  = -1;
}

// $deBlob: FUNCTION 007c51d0
TAnimation* AEntityPoseController::PlayManualOverlayAnim( const TCHAR* a_szName, TFLOAT a_fBlendInSpeed, TFLOAT a_fWeight, TFLOAT a_fSpeed )
{
	TASSERT( 0 > m_ManualOverlayAnim );

	AToshiAnimInterface2* pAnimInterface = GetAnimInterface();
	TINT16                iAnim          = TINT16( pAnimInterface->GetAnimID( a_szName ) );
	m_ManualOverlayAnim                  = iAnim;

	if ( iAnim >= 0 )
	{
		TAnimation* pAnim = pAnimInterface->AddAnim( iAnim, a_fBlendInSpeed, a_fWeight, a_fSpeed );
		TASSERT( TNULL != pAnim );
		return pAnim;
	}

	return TNULL;
}

// $deBlob: FUNCTION 007c52a0
void AEntityPoseController::StopManualOverlayAnim( TFLOAT a_fBlendOutSpeed )
{
	if ( m_ManualOverlayAnim >= 0 )
	{
		GetAnimInterface()->StopAnim( TINT16( m_ManualOverlayAnim ), a_fBlendOutSpeed );
		m_ManualOverlayAnim = -1;
	}
}

// $deBlob: FUNCTION 007c52e0
void AEntityPoseController::SetManualOverlayAnimWeight( TFLOAT a_fWeight )
{
	TASSERT( m_ManualOverlayAnim >= 0 );

	TSkeletonInstance* pSkeletonInstance = GetAnimInterface()->GetSkeletonInstance();

	if ( pSkeletonInstance )
	{
		TAnimation* pAnim = pSkeletonInstance->IsAnimating( TINT16( m_ManualOverlayAnim ) );

		if ( pAnim )
			pAnim->SetWeight( a_fWeight );
	}
}

// $deBlob: FUNCTION 007c5360
void AEntityPoseController::SetManualOverlayAnimTime( TFLOAT a_fTime )
{
	TASSERT( m_ManualOverlayAnim >= 0 );

	TSkeletonInstance* pSkeletonInstance = GetAnimInterface()->GetSkeletonInstance();

	if ( pSkeletonInstance )
	{
		TAnimation* pAnim = pSkeletonInstance->IsAnimating( TINT16( m_ManualOverlayAnim ) );

		if ( pAnim )
			pAnim->SetTime( a_fTime );
	}
}

// $deBlob: FUNCTION 007c53e0
void AEntityPoseController::ResetAnimIDs()
{
	for ( TINT i = 0; i < m_pPoseMap->iNumStates; i++ )
	{
		StatePose& rState = m_pPoseMap->pStates[ i ];

		for ( TINT k = 0; k < rState.iNumOptions; k++ )
			rState.pOptions[ k ].iAnim = -1;
	}
}

// $deBlob: FUNCTION 007c5450
AEntityPoseController::PoseMap* AEntityPoseController::ClonePoseMap( const PoseMap* a_pPoseMap )
{
	PoseMap* pPoseMap    = new PoseMap;
	pPoseMap->iNumStates = a_pPoseMap->iNumStates;

	if ( pPoseMap->iNumStates == 0 )
	{
		pPoseMap->pStates = TNULL;
		return pPoseMap;
	}

	pPoseMap->pStates = new StatePose[ pPoseMap->iNumStates ];

	for ( TINT i = 0; i < pPoseMap->iNumStates; i++ )
	{
		const StatePose& rSrc  = a_pPoseMap->pStates[ i ];
		StatePose&       rDest = pPoseMap->pStates[ i ];

		rDest.iPose       = rSrc.iPose;
		rDest.fMult       = rSrc.fMult;
		rDest.iNumOptions = rSrc.iNumOptions;

		if ( rDest.iNumOptions == 0 )
		{
			rDest.pOptions = TNULL;
		}
		else
		{
			rDest.pOptions = new AnimOption[ rDest.iNumOptions ];

			for ( TINT k = 0; k < rDest.iNumOptions; k++ )
			{
				rDest.pOptions[ k ].iAnim        = -1;
				rDest.pOptions[ k ].fProbability = rSrc.pOptions[ k ].fProbability;
				rDest.pOptions[ k ].szAnimName   = rSrc.pOptions[ k ].szAnimName;
			}
		}
	}

	return pPoseMap;
}

// $deBlob: FUNCTION 007c5570
void AEntityPoseController::DestroyPoseMap( PoseMap* a_pPoseMap )
{
	if ( a_pPoseMap )
	{
		if ( a_pPoseMap->iNumStates != 0 )
		{
			for ( TINT i = 0; i < a_pPoseMap->iNumStates; i++ )
			{
				if ( a_pPoseMap->pStates[ i ].iNumOptions != 0 )
					delete[] a_pPoseMap->pStates[ i ].pOptions;
			}

			delete[] a_pPoseMap->pStates;
		}

		delete a_pPoseMap;
	}
}

// $deBlob: FUNCTION 007c55d0
void AEntityPoseController::UpdateAnimLoop()
{
	if ( m_iCurrentAnim < 0 )
		return;

	AToshiAnimInterface2* pAnimInterface = GetAnimInterface();
	TAnimation*           pAnim          = pAnimInterface->GetSkeletonInstance()->IsAnimating( TINT16( m_iCurrentAnim ) );

	if ( pAnim && pAnim->GetSequencePtr()->GetDuration() <= pAnim->GetTotalTime() )
	{
		// The animation is over: pick the state's next one
		TINT   iNewAnim = -1;
		TFLOAT fMult    = 1.0f;

		if ( GetPoseForState( m_iCurrentState, iNewAnim, fMult ) == POSETYPE_Anim && iNewAnim != m_iCurrentAnim )
		{
			TFLOAT fBlendTime;

			if ( !m_bNoBlend )
			{
				AEntityStateComponent* pStateComponent = TSTATICCAST( AEntityStateComponent, m_pEntity->GetStateComponent() );
				fBlendTime                             = pStateComponent->GetStateAnimBlendTime( m_iCurrentState, m_iCurrentState );
			}
			else
			{
				fBlendTime = 0.0f;
			}

			pAnimInterface->StopAnim( TINT16( m_iCurrentAnim ), fBlendTime );

			if ( pAnimInterface->GetSkeletonInstance() )
			{
				TAnimation* pNewAnim = pAnimInterface->GetSkeletonInstance()->IsAnimating( TINT16( iNewAnim ) );

				if ( pNewAnim )
				{
					pNewAnim->SetWeight( 1.0f );
					pNewAnim->SetSpeed( fMult );
					m_iCurrentAnim = iNewAnim;
					return;
				}
			}

			pAnimInterface->AddAnim( TINT16( iNewAnim ), fBlendTime, 1.0f, fMult );
			m_iCurrentAnim = iNewAnim;
			return;
		}

		// Loop it
		pAnim->SetTime( pAnim->GetSeqTime() );
	}
}

// $deBlob: FUNCTION 007c5730
TINT AEntityPoseController::PickAnimOption( PoseMap* a_pPoseMap, TINT a_iState )
{
	StatePose& rState = a_pPoseMap->pStates[ a_iState ];

	if ( rState.iNumOptions > 1 )
	{
		TFLOAT fRandom = ARandom::GetSingleton()->GetRandom().GetFloat();

		for ( TINT i = 0; i < rState.iNumOptions; i++ )
		{
			if ( fRandom < rState.pOptions[ i ].fProbability )
				return i;
		}
	}

	return 0;
}

// $deBlob: FUNCTION 007c57d0
void AEntityPoseController::SetPoseForState( TINT a_iCurState, TINT a_iNewState )
{
	TINT     iID;
	TFLOAT   fMult;
	POSETYPE eType = GetPoseForState( a_iNewState, iID, fMult );

	TASSERT( eType == POSETYPE_None || eType == POSETYPE_Anim || ( eType == POSETYPE_Pose && m_pPoseInformation ) );

	if ( eType == POSETYPE_Anim )
	{
		if ( iID < 0 )
		{
			RestartManualOverlayAnim();
		}
		else if ( m_iCurrentAnim != iID || m_CurrentMult != fMult )
		{
			TFLOAT fBlendTime;

			if ( !m_bNoBlend )
			{
				AEntityStateComponent* pStateComponent = TSTATICCAST( AEntityStateComponent, m_pEntity->GetStateComponent() );
				fBlendTime                             = pStateComponent->GetStateAnimBlendTime( a_iCurState, a_iNewState );
			}
			else
			{
				fBlendTime = 0.0f;
			}

			StopOldAnims( iID, fBlendTime );

			AToshiAnimInterface2* pAnimInterface = GetAnimInterface();

			if ( pAnimInterface->GetSkeletonInstance() )
			{
				TAnimation* pAnim = pAnimInterface->GetSkeletonInstance()->IsAnimating( TINT16( iID ) );

				if ( pAnim )
				{
					pAnim->SetWeight( 1.0f );
					pAnim->SetSpeed( fMult );
					m_iCurrentAnim = iID;
					m_iCurrentPose = -1;
					m_CurrentMult  = fMult;
					return;
				}
			}

			pAnimInterface->AddAnim( TINT16( iID ), fBlendTime, 1.0f, fMult );
		}

		m_iCurrentAnim = iID;
		m_iCurrentPose = -1;
		m_CurrentMult  = fMult;
		return;
	}

	if ( eType == POSETYPE_Pose )
	{
		if ( m_pPoseInformation->GetPose( iID ) )
		{
			if ( m_iCurrentPose != iID )
			{
				TFLOAT fBlendTime;

				if ( !m_bNoBlend )
				{
					AEntityStateComponent* pStateComponent = TSTATICCAST( AEntityStateComponent, m_pEntity->GetStateComponent() );
					fBlendTime                             = pStateComponent->GetStateAnimBlendTime( a_iCurState, a_iNewState );
				}
				else
				{
					fBlendTime = 0.0f;
				}

				AAnimationPose* pPose = m_pPoseInformation->GetPose( iID );
				StopOldAnims( pPose, fBlendTime );
				GetAnimInterface()->SetPose( pPose, fBlendTime, fMult );
			}

			m_iCurrentPose = iID;
			m_CurrentMult  = fMult;
			m_iCurrentAnim = -1;
			return;
		}
	}
	else if ( eType != POSETYPE_None )
	{
		return;
	}

	RestartManualOverlayAnim();
	m_iCurrentPose = -1;
	m_CurrentMult  = 1.0f;
	m_iCurrentAnim = -1;
}

// $deBlob: FUNCTION 007c5a30
AEntityPoseController::POSETYPE AEntityPoseController::GetPoseForState( TINT a_State, TINT& a_rOutID, TFLOAT& a_rOutMult )
{
	TASSERT( a_State >= 0 );

	PoseMap* pPoseMap = m_pPoseMap;
	a_rOutID          = -1;
	a_rOutMult        = 1.0f;

	if ( a_State >= pPoseMap->iNumStates )
		return POSETYPE_None;

	StatePose& rState = pPoseMap->pStates[ a_State ];
	a_rOutID          = rState.iPose;
	a_rOutMult        = rState.fMult;

	if ( a_rOutID >= 0 )
		return POSETYPE_Pose;

	if ( rState.iNumOptions == 0 )
		return POSETYPE_None;

	TINT        iOption = PickAnimOption( pPoseMap, a_State );
	AnimOption& rOption = rState.pOptions[ iOption ];

	if ( rOption.iAnim < 0 )
	{
		if ( rOption.szAnimName == TNULL )
			rOption.iAnim = -1;
		else
			rOption.iAnim = TINT16( GetAnimInterface()->GetAnimID( rOption.szAnimName ) );
	}

	a_rOutID = rOption.iAnim;
	return POSETYPE_Anim;
}

// $deBlob: FUNCTION 007c5b50
void AEntityPoseController::RestartManualOverlayAnim()
{
	AToshiAnimInterface2* pAnimInterface = GetAnimInterface();

	if ( m_ManualOverlayAnim >= 0 )
	{
		TAnimation* pAnim = pAnimInterface->GetSkeletonInstance() ? pAnimInterface->GetSkeletonInstance()->IsAnimating( TINT16( m_ManualOverlayAnim ) ) : TNULL;
		TASSERT( TNULL != pAnim );

		if ( pAnim )
		{
			TFLOAT fSeqTime = pAnim->GetSeqTime();
			TFLOAT fSpeed   = pAnim->GetSpeed();
			TFLOAT fWeight  = pAnim->GetWeight();

			pAnimInterface->SetInvalidPose();

			TAnimation* pNewAnim = pAnimInterface->AddAnim( TINT16( m_ManualOverlayAnim ), 0.0f, fWeight, fSpeed );
			TASSERT( TNULL != pNewAnim );

			if ( pNewAnim )
				pNewAnim->SetTime( fSeqTime );

			return;
		}
	}

	pAnimInterface->SetInvalidPose();
}

// $deBlob: FUNCTION 007c5c70
void AEntityPoseController::StopOldAnims( TINT a_iNewAnim, TFLOAT a_fBlendOutSpeed )
{
	AToshiAnimInterface2* pAnimInterface = GetAnimInterface();

	if ( m_iCurrentAnim >= 0 )
	{
		if ( m_iCurrentAnim != a_iNewAnim )
			pAnimInterface->StopAnim( TINT16( m_iCurrentAnim ), a_fBlendOutSpeed );

		return;
	}

	if ( m_iCurrentPose >= 0 )
	{
		AAnimationPose* pPose = m_pPoseInformation->GetPose( m_iCurrentPose );

		for ( TINT i = 0; i < pPose->GetComponentCount(); i++ )
		{
			TINT iSequence = pPose->GetComponent( i ).iSequence;

			if ( iSequence != a_iNewAnim )
				pAnimInterface->StopAnim( TINT16( iSequence ), a_fBlendOutSpeed );
		}

		return;
	}

	if ( m_ManualOverlayAnim < 0 )
	{
		pAnimInterface->StopAllAnims( 0.0f );
		return;
	}

	if ( pAnimInterface->GetSkeletonInstance() )
	{
		TAnimation* pAnim = pAnimInterface->GetSkeletonInstance()->IsAnimating( TINT16( m_ManualOverlayAnim ) );

		if ( pAnim )
		{
			TFLOAT fSeqTime = pAnim->GetSeqTime();
			TFLOAT fSpeed   = pAnim->GetSpeed();
			TFLOAT fWeight  = pAnim->GetWeight();

			pAnimInterface->StopAllAnims( 0.0f );

			TAnimation* pNewAnim = pAnimInterface->AddAnim( TINT16( m_ManualOverlayAnim ), 0.0f, fWeight, fSpeed );
			TASSERT( TNULL != pNewAnim );

			if ( pNewAnim )
				pNewAnim->SetTime( fSeqTime );
		}
	}
}

// $deBlob: FUNCTION 007c5e40
void AEntityPoseController::StopOldAnims( AAnimationPose* a_pNewPose, TFLOAT a_fBlendOutSpeed )
{
	AToshiAnimInterface2* pAnimInterface = GetAnimInterface();

	if ( m_iCurrentAnim >= 0 )
	{
		for ( TINT i = 0; i < a_pNewPose->GetComponentCount(); i++ )
		{
			if ( m_iCurrentAnim == a_pNewPose->GetComponent( i ).iSequence )
				return;
		}

		pAnimInterface->StopAnim( TINT16( m_iCurrentAnim ), a_fBlendOutSpeed );
		return;
	}

	if ( m_iCurrentPose >= 0 )
	{
		AAnimationPose* pPose = m_pPoseInformation->GetPose( m_iCurrentPose );

		for ( TINT i = 0; i < pPose->GetComponentCount(); i++ )
		{
			TINT  iSequence  = pPose->GetComponent( i ).iSequence;
			TBOOL bInNewPose = TFALSE;

			for ( TINT k = 0; k < a_pNewPose->GetComponentCount(); k++ )
			{
				if ( iSequence == a_pNewPose->GetComponent( k ).iSequence )
				{
					bInNewPose = TTRUE;
					break;
				}
			}

			if ( !bInNewPose )
				pAnimInterface->StopAnim( TINT16( iSequence ), a_fBlendOutSpeed );
		}

		return;
	}

	if ( m_ManualOverlayAnim >= 0 )
	{
		TAnimation* pAnim = pAnimInterface->GetSkeletonInstance() ? pAnimInterface->GetSkeletonInstance()->IsAnimating( TINT16( m_ManualOverlayAnim ) ) : TNULL;
		TASSERT( TNULL != pAnim );

		if ( pAnim )
		{
			TFLOAT fSeqTime = pAnim->GetSeqTime();
			TFLOAT fSpeed   = pAnim->GetSpeed();
			TFLOAT fWeight  = pAnim->GetWeight();

			pAnimInterface->StopAllAnims( 0.0f );

			TAnimation* pNewAnim = pAnimInterface->AddAnim( TINT16( m_ManualOverlayAnim ), 0.0f, fWeight, fSpeed );
			TASSERT( TNULL != pNewAnim );

			if ( pNewAnim )
				pNewAnim->SetTime( fSeqTime );

			return;
		}
	}

	pAnimInterface->StopAllAnims( 0.0f );
}

// $deBlob: FUNCTION 007c6120
// $deBlob: FUNCTION 007c5050
AEntityPoseController::~AEntityPoseController()
{
}

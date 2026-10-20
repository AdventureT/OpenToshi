#include "pch.h"
#include "APropSwitchPressure.h"
#include "APropDoor.h"
#include "AEntityModelComponent.h"
#include "AEntityParams.h"
#include "AEntityFactory.h"
#include "AEntityLoader.h"
#include "AEntityIOPacket.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AModel2.h"
#include "Cameras/ACameraManager.h"
#include "Challenges/AChallengeManager.h"
#include "Collision/ACollisionInstance2.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Buildings/ABuildingInstance.h"
#include "Buildings/ATreeInstance.h"
#include "Terrain2/TerrainTypes.h"
#include "Locale/ALocaleManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "XUI/InGame/HUD/ABlobGoalHud.h"
#include "XUI/InGame/HUD/ABlobLetterBoxHud.h"
#include "XUI/InGame/HUD/ABlobChallengeEndHud.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Physics/TPhysicsMotionBase.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <Toshi/TStringManager.h>
#include <T2Locale/T2Locale.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TParticleEffect* APropSwitchPressure::sm_explosionEffect;
Toshi::TParticleEffect* APropSwitchPressure::sm_despawnEffect;
TINT                    APropSwitchPressure::sm_iSoundMode = 2;

TSTATICASSERT( sizeof( APropSwitchPressure ) == 0x2F0 );

// The level's list of sector switches (AGameStatus::LevelData+0x784, T2DList of the switches' nodes)
static T2DList<APropSwitchPressure::SectorSwitchNode>& GetSectorSwitchList()
{
	return *TREINTERPRETCAST( T2DList<APropSwitchPressure::SectorSwitchNode>*, &AGameStatus::GetSingleton()->GetLevelData().m_oList784 );
}

// The last switch of the level's list of sector switches (the one with the highest unlock score), TNULL without one
// NOTE: retail's is a member of AGameStatus::LevelData (thiscall)
// $deBlob: FUNCTION 005bc1f0
static APropSwitchPressure* GetLastSectorSwitch()
{
	T2DList<APropSwitchPressure::SectorSwitchNode>& rList = GetSectorSwitchList();

	if ( rList.IsEmpty() )
		return TNULL;

	APropSwitchPressure::SectorSwitchNode* pLast = rList.Back();
	return pLast != TNULL ? pLast->m_pSwitch : TNULL;
}

// $deBlob: FUNCTION 007e1270
APropSwitchPressure::SectorSwitchNode::SectorSwitchNode( APropSwitchPressure* a_pSwitch, ABaseEntity* a_pDoor )
    : m_pSwitch( a_pSwitch )
    , m_pDoor( a_pDoor )
{
	// Inserts the node before the first switch with an unlock score that isn't lower
	T2DList<SectorSwitchNode>& rList = GetSectorSwitchList();

	auto itWhere = rList.Begin();

	while ( itWhere.Get() != rList.End().Get() )
	{
		if ( m_pSwitch->m_iUnlockScore <= itWhere.Get()->m_pSwitch->m_iUnlockScore )
			break;

		itWhere = itWhere.Next();
	}

	InsertBefore( itWhere.Get() );
}

// $deBlob: FUNCTION 007e1370
// $deBlob: FUNCTION 0079f380
APropSwitchPressure::SectorSwitchNode::~SectorSwitchNode()
{
	if ( IsLinked() )
		Remove();
}

// $deBlob: FUNCTION 0079d010
void APropSwitchPressure::APropSwitchPressureCollisionEventHandler::OnTriggerEnter( TPhysicsObjectContainer* a_pBody, TPhysicsObjectContainer* a_pOther )
{
	APropSwitchPressure* pSwitch = m_pSwitch;

	if ( pSwitch == TNULL )
		return;

	TUINT32 uiLayers = ( a_pOther != TNULL ) ? a_pOther->m_uiCollisionFilterInfo & 0xFFF : 0;

	if ( !( uiLayers & 6 ) )
		return;

	struct
	{
		TVector4 vPosition;
		TVector4 vDirection;
	} oContact;

	oContact.vPosition = pSwitch->GetMatrix().GetTranslation();

	AEntityIOPacket oTouch;
	oTouch.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
	oTouch.m_iData    = TREINTERPRETCAST( TINT, &oContact );

	AEntityIOPacket oTouchedBy;
	oTouchedBy.m_eMessage = AEntityIOPacket::MESSAGE_TOUCHEDBY;
	oTouchedBy.m_iData    = TREINTERPRETCAST( TINT, &oContact );

	if ( a_pOther != TNULL && a_pOther->m_pUserData != TNULL )
	{
		ABaseEntity*    pOther    = TSTATICCAST( ABaseEntity, TSTATICCAST( ACollisionInstance2::OwnerInfo, a_pOther->m_pUserData )->m_pOwner );
		const TVector4& rOtherPos = pOther->GetMatrix().GetTranslation();

		oContact.vDirection.x = rOtherPos.x - pSwitch->GetMatrix().GetTranslation().x;
		oContact.vDirection.y = rOtherPos.y - pSwitch->GetMatrix().GetTranslation().y;
		oContact.vDirection.z = rOtherPos.z - pSwitch->GetMatrix().GetTranslation().z;
		oContact.vDirection.w = rOtherPos.w;
		oContact.vDirection.Normalise();
		oContact.vPosition = rOtherPos;

		pSwitch->HandleEntityIO( pOther, oTouch );
		pOther->HandleEntityIO( pSwitch, oTouchedBy );
		pSwitch->UpdateUnlockMessage();
	}
}

// $deBlob: FUNCTION 0079d100
void APropSwitchPressure::APropSwitchPressureCollisionEventHandler::OnTriggerLeave( TPhysicsObjectContainer* a_pBody, TPhysicsObjectContainer* a_pOther )
{
	APropSwitchPressure* pSwitch = m_pSwitch;

	if ( pSwitch == TNULL || a_pOther == TNULL || a_pOther->m_pUserData == TNULL )
		return;

	struct
	{
		TVector4 vPosition;
		TVector4 vDirection;
	} oContact;

	oContact.vPosition  = pSwitch->GetMatrix().GetTranslation();
	oContact.vDirection = TVector4::VEC_NEGY;

	AEntityIOPacket oUntouch;
	oUntouch.m_eMessage = AEntityIOPacket::MESSAGE_UNTOUCH;
	oUntouch.m_iData    = TREINTERPRETCAST( TINT, &oContact );

	pSwitch->HandleEntityIO( TSTATICCAST( ABaseEntity, TSTATICCAST( ACollisionInstance2::OwnerInfo, a_pOther->m_pUserData )->m_pOwner ), oUntouch );
}

// $deBlob: FUNCTION 0079cf10
// $deBlob: FUNCTION 0079cf20
// $deBlob: FUNCTION 0079cf90
// $deBlob: FUNCTION 0079cff0
// $deBlob: FUNCTION 0079d000
// $deBlob: FUNCTION 00540ee0
// $deBlob: FUNCTION 00859e40
TDEFINE_CLASS( APropSwitchPressure );

// $deBlob: FUNCTION 0079d160
APropSwitchPressure::APropSwitchPressure()
{
	m_iPressAnim         = -1;
	m_iReleaseAnim       = -1;
	m_iUnlockScore       = -1;
	m_iScriptedSequence  = -1;
	m_eState             = STATE_Idle;
	m_fDeactivationDelay = 3.0f;
	m_fDeactivationTimer = 0.0f;
	m_fPressTimer        = 0.0f;
	m_pSound             = TNULL;
	m_pPhysicsData       = TNULL;
	m_pSectorNode        = TNULL;
	m_pShield            = TNULL;
	m_pPresser           = TNULL;
	m_uiSwitchFlags      = 0;
	m_iSectorIndex       = -1;
	m_uiUnlockType       = 0;
	m_fUnlockDelay       = 0.0f;
	m_fUnlockTimer       = 0.0f;
	TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType )
	[1] = 2;
}

// $deBlob: FUNCTION 0079d220
// $deBlob: FUNCTION 0079f320
// $deBlob: FUNCTION 0079f307
APropSwitchPressure::~APropSwitchPressure()
{
	sm_despawnEffect   = TNULL;
	sm_explosionEffect = TNULL;

	if ( m_pSectorNode != TNULL )
	{
		m_pSectorNode->~SectorSwitchNode();
		::operator delete( m_pSectorNode );
	}

	delete m_pShield;

	KillManualAllAnimations( -1.0f );
	DestroyPhysics();
}

// $deBlob: FUNCTION 0079d950
void APropSwitchPressure::Tick( TFLOAT a_fDeltaTime )
{
	AProp::Tick( a_fDeltaTime );

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	AGameStatus*    pGameStatus    = AGameStatus::GetSingleton();
	TXUI*           pXUI           = TXUI::GetSingleton();

	if ( m_uiSwitchFlags & SWITCHFLAG_UnlockWaitsForCamera )
	{
		APlayerManager::BlobData* pBlobData = pPlayerManager->GetBlobData( 0 );

		if ( !pBlobData->IsSSCameraPlaying( m_iScriptedSequence ) )
		{
			if ( !pPlayerManager->IsSSCameraQueued( 0, m_iScriptedSequence ) && pXUI->GetActiveScene() != TNULL && pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() )
			{
				if ( pXUI->GetActiveScene()->IsA( &TGetClass( ABlobHud ) ) )
				{
					TSTATICCAST( ABlobHud, pXUI->GetActiveScene() )->QueueUnk260Banner();
					m_fUnlockTimer = 0.0f;
					Unlock();
					m_uiSwitchFlags &= ~SWITCHFLAG_UnlockWaitsForCamera;
				}
			}
		}
		else if ( pBlobData->m_iCameraState == 5 && pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() && pXUI->GetActiveScene() != TNULL )
		{
			if ( pXUI->GetActiveScene()->IsA( &TGetClass( ABlobLetterBoxHud ) ) )
			{
				m_fUnlockTimer -= a_fDeltaTime;

				if ( m_fUnlockTimer < 0.0f )
				{
					m_fUnlockTimer = 0.0f;
					Unlock();
					m_uiSwitchFlags &= ~SWITCHFLAG_UnlockWaitsForCamera;
					TSTATICCAST( ABlobLetterBoxHud, pXUI->GetActiveScene() )->PlayTopDisplay();
				}
			}
		}
	}

	if ( ( m_uiSwitchFlags & SWITCHFLAG_ChallengeEndPending ) && m_eState == STATE_Pressed )
	{
		ABaseEntity*              pBlob     = pPlayerManager->GetPlayerBlob( 0 );
		APlayerManager::BlobData* pBlobData = pPlayerManager->GetBlobData( 0 );

		if ( TSTATICCAST( ABlobEntity, pBlob )->GetBlobState() == ABlobEntity::STATE_ROLL && pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() && pBlobData->m_eCameraMode == 1 &&
		     ( pBlobData->m_iNextCameraMode == -1 || pBlobData->m_iNextCameraMode == 1 ) && !AChallengeManager::GetSingleton()->IsEnding() )
		{
			m_uiSwitchFlags &= ~SWITCHFLAG_ChallengeEndPending;

			TXUIResource* pResource = pXUI->FindResource( "challengeend.xur" );
			TXUIScene*    pScene    = pXUI->CreateScene( pResource, TUINT32( 0 ) );
			TASSERT( pScene->IsA( &TGetClass( ABlobChallengeEndHud ) ) );

			ABlobChallengeEndHud* pHud = TSTATICCAST( ABlobChallengeEndHud, pScene );

			AGameStatus::LevelData& rLevelData = pGameStatus->GetLevelData();
			pHud->SetCount( rLevelData.m_bGoalCheat ? 0 : rLevelData.m_iGoal );

			pXUI->TransitionTo( TNULL, pScene );
		}
	}

	m_fPressTimer -= a_fDeltaTime;

	if ( m_fPressTimer < 0.0f && !( m_uiSwitchFlags & SWITCHFLAG_Pressed ) )
	{
		if ( m_eState == STATE_Pressed )
		{
			if ( !( m_uiSwitchFlags & SWITCHFLAG_SingleUse ) )
			{
				if ( m_uiSwitchFlags & SWITCHFLAG_TriggerOnRelease )
					StartCountdownSound();

				m_fDeactivationTimer = m_fDeactivationDelay;
				m_eState             = STATE_WaitRelease;
			}
		}
		else if ( m_eState == STATE_WaitRelease )
		{
			UpdateCountdownSound();

			m_fDeactivationTimer -= a_fDeltaTime;

			if ( m_fDeactivationTimer < 0.0f )
			{
				StopCountdownSound();
				AddManualAnimation( m_iReleaseAnim, 0.0f, 0.0f, 1.0f, 1.0f );
				m_eState = STATE_Releasing;
			}
		}
	}

	if ( m_eState == STATE_Pressing )
	{
		if ( IsManualAnimationFinished( m_iPressAnim ) )
		{
			m_eState = STATE_Pressed;
			TriggerAllTargets( this );
			m_uiSwitchFlags |= SWITCHFLAG_Triggered;
		}
	}
	else if ( m_eState == STATE_Releasing && IsManualAnimationFinished( m_iReleaseAnim ) )
	{
		if ( m_uiSwitchFlags & SWITCHFLAG_TriggerOnRelease )
			TriggerAllTargets( this );

		if ( !( m_uiSwitchFlags & SWITCHFLAG_Pressed ) )
		{
			m_eState = STATE_Idle;
			SetTicking( TFALSE );
			AddManualAnimation( m_iReleaseAnim, -1.0f, 0.15f, 1.0f, 1.0f );
		}
		else
		{
			AddManualAnimation( m_iPressAnim, 0.0f, 0.0f, 1.0f, 1.0f );
			m_eState = STATE_Pressing;
		}
	}

	ALevelInformation::LevelProperties* pLevelProps = pGameStatus->GetLevelProperties();

	if ( pPlayerManager->GetNumPlayers() == 1 && m_eState == STATE_Locked && m_uiUnlockType == 0 )
	{
		AGameStatus::LevelData& rLevelData = pGameStatus->GetLevelData();
		TINT                    iScore     = -1;
		TINT                    eLevelMode = pGameStatus->GetLevelMode();

		if ( eLevelMode == -1 || eLevelMode == 0 || eLevelMode == 3 )
		{
			iScore = rLevelData.GetPoints( 0 );
		}
		else if ( eLevelMode == 2 )
		{
			const AGameStatus::PlayerData& rPlayerData = rLevelData.m_aPlayerData[ 0 ];
			iScore                                     = TINT( ( TFLOAT( TINT( rPlayerData.m_auiProgress[ 4 ] ) + TINT( rPlayerData.m_iBuildingsPainted ) ) / TFLOAT( TINT( rLevelData.m_auiLevelCounts[ 2 ] ) + TINT( rLevelData.m_auiLevelCounts[ 0 ] ) ) ) * 100.0f );
		}

		TASSERT( iScore != -1 );

		if ( pLevelProps->m_sprintLevel == 0 )
		{
			if ( !rLevelData.m_bUnk00C )
			{
				TBOOL bUnlock = TTRUE;

				if ( !( m_uiSwitchFlags & SWITCHFLAG_SectorSwitch ) )
				{
					if ( iScore < m_iUnlockScore )
						bUnlock = TFALSE;
				}
				else if ( iScore < m_iUnlockScore && !rLevelData.m_bGoalCheat )
				{
					bUnlock = TFALSE;
				}
				else
				{
					T2DList<SectorSwitchNode>& rList = GetSectorSwitchList();
					TASSERT( !rList.IsEmpty() );

					SectorSwitchNode* pFront = rList.Front();

					if ( m_pSectorNode != pFront && TSTATICCAST( SectorSwitchNode, m_pSectorNode->Prev() )->m_pSwitch->m_eState == STATE_Locked )
						bUnlock = TFALSE;
				}

				if ( bUnlock )
				{
					if ( m_iScriptedSequence == -1 )
					{
						Unlock();
						ABlobHud::GetSingleton()->QueueUnk260Banner();
					}
					else if ( pPlayerManager->GetNumPlayers() == 1 && !( m_uiSwitchFlags & SWITCHFLAG_UnlockWaitsForCamera ) )
					{
						if ( pGameStatus->GetLevelMode() != AGameStatus::Mode_FreePaint )
						{
							pPlayerManager->QueueSSCamera( 0, m_iScriptedSequence );
						}

						m_uiSwitchFlags |= SWITCHFLAG_UnlockWaitsForCamera;
						m_fUnlockTimer = m_fUnlockDelay;
					}
				}
			}
		}
		else if ( ( m_uiSwitchFlags & SWITCHFLAG_SectorSwitch ) && m_iUnlockScore <= AChallengeManager::GetSingleton()->GetCounter() && ( AChallengeManager::GetSingleton()->GetCurrentChallenge() == TNULL || AChallengeManager::GetSingleton()->GetCurrentChallenge()->GetData()->m_uiNumLinkedChallenges == 0 ) )
		{
			Unlock();
		}
	}

	if ( ( m_uiSwitchFlags & SWITCHFLAG_Locked ) && IsFlagged( FLAG_Visible ) && IsFlagged( FLAG_HighLOD ) && ( m_pShield->GetModelInstance()->GetFlags() & AModelInstance2::FLAG_Visible ) )
		m_pShield->Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0079ebb0
void APropSwitchPressure::OnLODChanging( TINT a_iLOD )
{
	if ( m_uiSwitchFlags & SWITCHFLAG_Locked )
	{
		if ( !( a_iLOD & 0x800 ) || m_eState != STATE_Locked )
		{
			m_pShield->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		}
		else
		{
			m_pShield->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, ( m_Flags >> 2 ) & 1 );

			TMatrix44 mTransform = m_oMatrix;
			mTransform.RotateZ( 1.5707964f );
			m_pShield->SetTransform( mTransform );
		}
	}

	if ( ( a_iLOD & 0x200 ) && ( m_uiSwitchFlags & SWITCHFLAG_SoundPlaying ) )
	{
		if ( m_pSound )
		{
			m_pSound->stop( false );
			m_pSound = TNULL;
		}

		m_uiSwitchFlags &= ~SWITCHFLAG_SoundPlaying;
	}

	AVisibleEntity::OnLODChanging( a_iLOD );
}

// $deBlob: FUNCTION 0079d850
// $deBlob: FUNCTION 0079f48b
void APropSwitchPressure::PostCreate()
{
	AProp::PostCreate();

	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
	{
		if ( GetNumEntityTargets() == 0 || TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModel()->GetFileName().CompareNoCase( "prop_trap_pressure_switch.trb", -1 ) == 0 )
			Despawn( TFALSE );
	}

	if ( m_uiSwitchFlags & SWITCHFLAG_SectorSwitch )
	{
		// The first entity target that is an APropDoor
		ABaseEntity* pDoor = TNULL;

		if ( GetNumEntityTargets() != 0 )
		{
			TINT iTarget = 0;

			do
			{
				pDoor = GetEntityTarget( iTarget );

				if ( pDoor->GetClass() == &TGetClass( APropDoor ) )
					break;

				iTarget++;
			} while ( iTarget < GetNumEntityTargets() );

			if ( iTarget >= GetNumEntityTargets() )
				pDoor = TNULL;
		}

		m_pSectorNode = new SectorSwitchNode( this, pDoor );
	}
}

// $deBlob: FUNCTION 0079d2e0
// $deBlob: FUNCTION 0079f496
void APropSwitchPressure::ProcessParams( const AEntityParams& a_rParams )
{
	const TCHAR* szDespawnEffect = m_pTypeData->m_pTRBData->m_szDespawnEffect;

	if ( szDespawnEffect && *szDespawnEffect && sm_despawnEffect == TNULL )
	{
		sm_despawnEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( szDespawnEffect );
		TASSERT( TDebug_IsValidAddress( sm_despawnEffect ) && "Couldn't find despawn particle effect %s for pressure switch!" );
	}

	AProp::ProcessParams( a_rParams );

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	TBOOL        bValue;

	a_rParams.GetValue( bValue, "TriggerOnce" );
	if ( bValue ) m_uiSwitchFlags |= SWITCHFLAG_TriggerOnce;

	a_rParams.GetValue( bValue, "TriggerOnRelease" );
	if ( bValue ) m_uiSwitchFlags |= SWITCHFLAG_TriggerOnRelease;

	a_rParams.GetValue( bValue, "StateDependent" );
	if ( bValue ) m_uiSwitchFlags |= SWITCHFLAG_StateDependent;

	a_rParams.GetValue( bValue, "Slam" );
	if ( bValue ) m_uiSwitchFlags |= SWITCHFLAG_Slam;

	a_rParams.GetValue( bValue, "Targetable" );
	if ( bValue ) m_uiSwitchFlags |= SWITCHFLAG_Targetable;

	a_rParams.GetValue( bValue, "TargetableFromBelow" );
	if ( bValue ) m_uiSwitchFlags |= SWITCHFLAG_TargetableFromBelow;

	if ( pGameStatus->GetLevelMode() != AGameStatus::Mode_FreePaint )
	{
		TBOOL bLocked = TFALSE;

		if ( a_rParams.GetValue( bLocked, "Locked" ) && bLocked )
		{
			m_uiSwitchFlags |= SWITCHFLAG_Locked;
			m_eState = STATE_Locked;
			a_rParams.GetValue( TREINTERPRETCAST( TINT8&, m_uiUnlockType ), "Unlock Type" );
		}
	}

	a_rParams.GetValue( bValue, "SingleUse" );
	if ( bValue ) m_uiSwitchFlags |= SWITCHFLAG_SingleUse;

	TBOOL bSectorSwitch = TFALSE;

	if ( a_rParams.GetValue( bSectorSwitch, "Sector Switch" ) && bSectorSwitch )
		m_uiSwitchFlags |= SWITCHFLAG_SectorSwitch;

	if ( bSectorSwitch )
	{
		pGameStatus->GetLevelData().m_uiUnk780++;
		m_Flags &= ~FLAG_DontTick;
		SetTicking( TTRUE );

		a_rParams.GetValue( m_iSectorIndex, "SectorIndex" );
		TASSERT( m_iSectorIndex >= 0 && m_iSectorIndex < 10 );

		ALevelInformation::LevelProperties* pLevelProps = pGameStatus->GetLevelProperties();
		TINT                                eLevelMode  = pGameStatus->GetLevelMode();

		if ( pLevelProps->m_sprintLevel == 0 )
		{
			if ( eLevelMode == -1 || eLevelMode == 0 || eLevelMode == 3 )
			{
				// NOTE: the level's unlock scores per sector (LevelProperties+0x5C)
				m_iUnlockScore = TREINTERPRETCAST( TINT*, pLevelProps->m_auiGoals[ 0 ] )[ m_iSectorIndex ];
				TASSERT( m_iUnlockScore > 0 );
			}
			else if ( eLevelMode == 2 )
			{
				// NOTE: the level's unlock percentages per sector (LevelProperties+0x60)
				m_iUnlockScore = TREINTERPRETCAST( TINT*, pLevelProps->m_auiGoals[ 1 ] )[ m_iSectorIndex ];
				TASSERT( m_iUnlockScore >= 0 && m_iUnlockScore <= 100 );
			}
		}

		if ( eLevelMode != 1 )
		{
			m_eState = STATE_Locked;
			m_uiSwitchFlags |= SWITCHFLAG_Locked;
		}

		m_uiSwitchFlags |= SWITCHFLAG_SingleUse;

		if ( pGameStatus->GetLevelMode() != AGameStatus::Mode_FreePaint )
		{
			a_rParams.GetValue( m_iScriptedSequence, "Sector Unlock Scripted Sequence" );

			if ( m_iScriptedSequence != -1 )
				a_rParams.GetValue( m_fUnlockDelay, "Unlock delay" );
		}
	}

	a_rParams.GetValue( m_fDeactivationDelay, "DeactivationDelay" );

	if ( m_uiSwitchFlags & SWITCHFLAG_Slam )
		m_Flags |= 0x40;

	m_iPressAnim   = CreateManualAnimation( "anim_press" );
	m_iReleaseAnim = CreateManualAnimation( "anim_release" );

	if ( m_uiSwitchFlags & SWITCHFLAG_Locked )
	{
		m_pShield = new AEntityModelComponent( this, "Landmark_target_shield.trb" );
		m_pShield->GetModelInstance()->SetAmbientScale( m_fAmbientScale );

		TModelInstance*      pModelInstance = m_pShield->GetModelInstance()->GetModelInstance();
		ACollisionInstance2* pCollision     = new ACollisionInstance2();
		pCollision->SetModelInstance( pModelInstance, TNULL );

		ACollisionInstance2::OwnerInfo oOwner;
		oOwner.m_eType  = 2;
		oOwner.m_pOwner = this;
		pCollision->Create( pModelInstance, &oOwner, TFALSE, 0, TNULL );

		for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
		{
			TPhysicsObjectContainer* pBody = pCollision->GetPhysicsInfo( i ).m_pBody;

			if ( pBody != TNULL && !( pBody->m_flags & 0x128 ) )
				TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, 2 );
		}

		m_pShield->GetModelInstance()->SetCollisionInstance( pCollision );

		if ( sm_explosionEffect == TNULL )
			sm_explosionEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_Explosion" );
	}

	AddManualAnimation( ( m_uiSwitchFlags & SWITCHFLAG_Locked ) ? m_iPressAnim : m_iReleaseAnim, -1.0f, 0.15f, 1.0f, 1.0f );
	CreateModelCollision( TTRUE );
	CreatePhysics( a_rParams );
}

// $deBlob: FUNCTION 0079eaf0
void APropSwitchPressure::SetVisible( TBOOL a_bVisible )
{
	AVisibleEntity::SetVisible( a_bVisible );

	if ( m_uiSwitchFlags & SWITCHFLAG_Locked )
	{
		if ( a_bVisible && ( m_Flags & FLAG_HighLOD ) && m_eState == STATE_Locked )
		{
			m_pShield->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );

			TMatrix44 mTransform = m_oMatrix;
			mTransform.RotateZ( 1.5707964f );
			m_pShield->SetTransform( mTransform );
			return;
		}

		m_pShield->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	}
}

// $deBlob: FUNCTION 0079e7d0
void APropSwitchPressure::ReleaseByDoor()
{
	m_fPressTimer = 0.2f;
	KillManualAllAnimations( -1.0f );
	// NOTE: retail plays the press animation here, not the release one
	AddManualAnimation( m_iPressAnim, 0.0f, 0.15f, 1.0f, 1.0f );
	m_uiSwitchFlags |= SWITCHFLAG_Triggered;
	m_eState = STATE_Pressed;
	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 0079e990
void APropSwitchPressure::SetPosition( const TVector4& a_rPosition )
{
	AProp::SetPosition( a_rPosition );
	UpdatePhysics();
}

// $deBlob: FUNCTION 0079e9b0
void APropSwitchPressure::SetMatrix( const TMatrix44& a_rMatrix )
{
	AProp::SetMatrix( a_rMatrix );
	UpdatePhysics();
}

// The targets of a pressure switch: a state dependent switch opens (state 1, MESSAGE_OPEN) or closes (state 3, MESSAGE_CLOSE)
// its doors; a blob pressing it in multiplayer scores the instance targets
// $deBlob: FUNCTION 0079e290
void APropSwitchPressure::TriggerAllTargets( ABaseEntity* a_pCaller )
{
	if ( ( m_uiSwitchFlags & ( SWITCHFLAG_TriggerOnce | SWITCHFLAG_Triggered ) ) == ( SWITCHFLAG_TriggerOnce | SWITCHFLAG_Triggered ) )
		return;

	for ( TINT i = 0; i < GetNumEntityTargets(); i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( pTarget->GetFlags() & FLAG_Despawned )
			continue;

		AEntityIOPacket oPacket;

		if ( !( m_uiSwitchFlags & SWITCHFLAG_StateDependent ) )
		{
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
			oPacket.m_iData    = 0;
			pTarget->HandleEntityIO( a_pCaller, oPacket );
		}
		else if ( pTarget->IsA( &TGetClass( APropDoor ) ) )
		{
			APropDoor* pDoor = pTarget->CastTo<APropDoor>();
			pDoor->SetSwitch( this );

			if ( m_eState == STATE_Pressed )
			{
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_OPEN;
				oPacket.m_iData    = 0;
				pDoor->HandleEntityIO( a_pCaller, oPacket );
			}
			else if ( m_eState == STATE_Idle )
			{
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_CLOSE;
				oPacket.m_iData    = 0;
				pDoor->HandleEntityIO( a_pCaller, oPacket );
			}
		}
		else
		{
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
			oPacket.m_iData    = 0;
			pTarget->HandleEntityIO( a_pCaller, oPacket );
		}
	}

	// Multiplayer: the blob that pressed it scores the buildings and trees it paints (repainted ones count less)
	TINT iPoints = 0;

	if ( m_eState == STATE_Pressed && m_pPresser != TNULL && m_pPresser->IsA( &TGetClass( ABlobEntity ) ) && AGameStatus::GetSingleton()->GetMultiPlayerGameType() != AGameStatus::GAMETYPE_MP_NONE )
	{
		const TINT*   piScoring = TREINTERPRETCAST( const TINT*, AGameStatus::GetBlobScoringProperties() );
		const TFLOAT* pfScoring = TREINTERPRETCAST( const TFLOAT*, AGameStatus::GetBlobScoringProperties() );

		TINT iBuildingRepaint = TINT( TFLOAT( piScoring[ 0x0C / 4 ] ) * pfScoring[ 0x04 / 4 ] );
		TINT iTreeRepaint     = TINT( TFLOAT( piScoring[ 0x9C / 4 ] ) * pfScoring[ 0x04 / 4 ] );

		for ( TINT i = 0; i < GetNumInstances(); i++ )
		{
			TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, GetInstanceTarget( i ) );
			TUINT32                     uiType    = pInstance->m_uiFlags & 7;

			if ( uiType == TerrainTypes::InstanceType_Tree )
			{
				if ( TSTATICCAST( ATreeInstance, pInstance->m_pModelInstance2 )->IsTransformed() )
					iPoints += iTreeRepaint;
				else
					iPoints += piScoring[ 0x9C / 4 ];
			}
			else if ( uiType == TerrainTypes::InstanceType_Building )
			{
				if ( TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 )->GetBuildingFlags() & ABuildingInstance::Flags_Painted )
					iPoints += iBuildingRepaint;
				else
					iPoints += piScoring[ 0x0C / 4 ];
			}
		}
	}

	TriggerInstanceTargets( m_pPresser );
	StartCameraShakes();

	if ( iPoints > 0 )
	{
		TINT    iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( m_pPresser );
		TWCHAR* wszText = TStringManager::GetTempString16();
		TStringManager::String16Format( wszText, T2Locale::GetSingleton()->GetString( 0x75 ), iPoints );

		TVector4 vPosition = m_pPresser->GetMatrix().GetTranslation();
		vPosition.y -= 2.0f;

		AGameStatus::GetSingleton()->GetLevelData().ShowMessage( wszText, 2, &vPosition, iPlayer );
	}
}

// $deBlob: FUNCTION 0079e020
void APropSwitchPressure::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( m_Flags & FLAG_Despawned )
		return;

	AProp::HandleEntityIO( a_pCaller, a_rPacket );

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	TINT eMessage = a_rPacket.m_eMessage;

	if ( ( eMessage == AEntityIOPacket::MESSAGE_TOUCH || eMessage == AEntityIOPacket::MESSAGE_TRIGGER ) && a_pCaller->IsFlagged( ABaseEntity::FLAG_Player ) )
	{
		m_pPresser = a_pCaller;

		if ( !( m_uiSwitchFlags & SWITCHFLAG_Slam ) )
		{
			m_fPressTimer = 0.2f;

			if ( m_eState == STATE_WaitRelease )
				m_eState = STATE_Pressed;

			if ( m_eState == STATE_Idle )
			{
				AddManualAnimation( m_iPressAnim, 0.0f, 0.0f, 1.0f, 1.0f );
				m_eState = STATE_Pressing;
				ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 4, m_oMatrix.GetTranslation(), 1.0f, -1 );
				SetTicking( TTRUE );

				if ( m_uiSwitchFlags & SWITCHFLAG_SectorSwitch )
				{
					if ( this != GetLastSectorSwitch() )
						AMusicManager::GetSingleton()->ClampScore();

					TASSERT( m_iSectorIndex + 1 < 10 );

					ALevelInformation::LevelProperties* pLevelProps = pGameStatus->GetLevelProperties();
					TINT                                iNextScore  = TREINTERPRETCAST( TINT*, pLevelProps->m_auiGoals[ 0 ] )[ m_iSectorIndex + 1 ];

					ABlobHud* pHud = ABlobHud::GetSingleton();

					if ( iNextScore == 0 )
					{
						if ( pHud != TNULL )
							TSTATICCAST( ABlobGoalHud, pHud->GetGoalHud() )->SetUnk280( TTRUE );
					}
					else
					{
						if ( pHud != TNULL )
							TSTATICCAST( ABlobGoalHud, pHud->GetGoalHud() )->SetState( 0 );

						pGameStatus->GetLevelData().SetGoal( iNextScore, m_iSectorIndex + 1 );
					}

					if ( pLevelProps->m_sprintLevel == 0 && iNextScore != 0 && pGameStatus->GetLevelMode() != AGameStatus::Mode_FreePaint )
					{
						m_uiSwitchFlags |= SWITCHFLAG_ChallengeEndPending;
						TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) )->AddBlobFlags( 0x10000000 );
					}
				}

				if ( m_pSectorNode != TNULL )
				{
					pGameStatus->GetPointData().SetUnk20( pGameStatus->GetLevelData().m_uiUnk774, TTRUE );
				}
			}

			if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TRIGGER )
				m_uiSwitchFlags |= SWITCHFLAG_Pressed;
		}
	}
	else if ( eMessage == AEntityIOPacket::MESSAGE_UNTOUCH )
	{
		m_uiSwitchFlags &= ~SWITCHFLAG_Pressed;
	}
	else if ( eMessage == AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE && m_eState == STATE_Locked && m_uiUnlockType == 1 )
	{
		Unlock();
	}
}

// $deBlob: FUNCTION 0079e9d0
void APropSwitchPressure::AddToPhysics()
{
	if ( ( m_uiSwitchFlags & SWITCHFLAG_Locked ) && m_eState == STATE_Locked && m_pShield->GetModelInstance()->GetCollisionInstance() )
		TSTATICCAST( ACollisionInstance2, m_pShield->GetModelInstance()->GetCollisionInstance() )->AddToWorld( 0x60 );

	TPhysicsManager::GetSingleton()->AddToSimulation( m_pPhysicsData->m_pBody, TFALSE );

	AVisibleEntity::AddToPhysics();
}

// $deBlob: FUNCTION 0079ea30
void APropSwitchPressure::RemoveFromPhysics()
{
	if ( ( m_uiSwitchFlags & SWITCHFLAG_Locked ) && m_eState == STATE_Locked && m_pShield->GetModelInstance()->GetCollisionInstance() )
		TSTATICCAST( ACollisionInstance2, m_pShield->GetModelInstance()->GetCollisionInstance() )->RemoveFromWorld( 0x60 );

	TPhysicsManager::GetSingleton()->RemoveFromSimulation( m_pPhysicsData->m_pBody, TFALSE );

	AVisibleEntity::RemoveFromPhysics();
}

// $deBlob: FUNCTION 0079f450
TBOOL APropSwitchPressure::IsTargetable()
{
	return ( m_uiSwitchFlags & SWITCHFLAG_Targetable ) && m_eState == STATE_Idle;
}

// $deBlob: FUNCTION 0079f3c0
TBOOL APropSwitchPressure::CanBeTargetedFromBelow()
{
	return ( m_uiSwitchFlags >> 5 ) & 1;
}

// $deBlob: FUNCTION 0079f4d0
TVector4 APropSwitchPressure::GetTargetOffset()
{
	TVector4 vResult;
	vResult.Multiply( TVector4( m_oMatrix.m_f21, m_oMatrix.m_f22, m_oMatrix.m_f23, m_oMatrix.m_f24 ), -m_vBBox.y );
	vResult.w = 1.0f;

	return vResult;
}

// $deBlob: FUNCTION 0079e840
void APropSwitchPressure::Reset( ABaseEntity* a_pEntity )
{
	AProp::Reset( a_pEntity );

	m_Flags &= ~FLAG_DontTick;
	m_fUnlockTimer = 0.0f;
	m_pPresser     = TNULL;
	KillManualAllAnimations( -1.0f );

	if ( m_uiSwitchFlags & SWITCHFLAG_SoundPlaying )
	{
		if ( m_pSound )
		{
			m_pSound->stop( false );
			m_pSound = TNULL;
		}

		m_uiSwitchFlags &= ~SWITCHFLAG_SoundPlaying;
	}

	TINT16 iAnim;

	if ( !( m_uiSwitchFlags & SWITCHFLAG_Locked ) )
	{
		iAnim    = m_iReleaseAnim;
		m_eState = STATE_Idle;
	}
	else
	{
		iAnim    = m_iPressAnim;
		m_eState = STATE_Locked;
	}

	AddManualAnimation( iAnim, -1.0f, 0.15f, 1.0f, 1.0f );
	m_fPressTimer = 0.0f;
	m_uiSwitchFlags &= 0xCF3F;
	SetTicking( ( m_uiSwitchFlags & SWITCHFLAG_SectorSwitch ) != 0 );
	SetVisible( TTRUE );

	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
	{
		if ( GetNumEntityTargets() == 0 || TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModel()->GetFileName().CompareNoCase( "prop_trap_pressure_switch.trb", -1 ) == 0 )
			Despawn( TFALSE );
	}
}

// $deBlob: FUNCTION 0079ecb0
void APropSwitchPressure::Despawn( TBOOL a_bPlayEffect )
{
	if ( m_uiSwitchFlags & SWITCHFLAG_SoundPlaying )
	{
		if ( m_pSound )
		{
			m_pSound->stop( false );
			m_pSound = TNULL;
		}

		m_uiSwitchFlags &= ~SWITCHFLAG_SoundPlaying;
	}

	KillManualAllAnimations( -1.0f );
	AVisibleEntity::Despawn( a_bPlayEffect );
}

// $deBlob: FUNCTION 0079f410
void* APropSwitchPressure::GetDespawnEffect()
{
	return sm_despawnEffect;
}

// Creates the fixed body from the entity data's matrix and box
// $deBlob: FUNCTION 0079e500
void APropSwitchPressure::CreatePhysics( const AEntityParams& a_rParams )
{
	const AEntityLoader::EntityData& rEntityData = static_cast<const AEntityLoader::EntityData&>( a_rParams );

	TASSERT( TNULL == m_pPhysicsData );

	m_pPhysicsData                        = new PhysicsData;
	m_pPhysicsData->m_oOwnerInfo.m_eType  = 0;
	m_pPhysicsData->m_oOwnerInfo.m_pOwner = TNULL;
	m_pPhysicsData->m_pHandler            = new APropSwitchPressureCollisionEventHandler( this );
	m_pPhysicsData->m_pMover              = TNULL;
	m_pPhysicsData->m_oOwnerInfo.m_eType  = 0;
	m_pPhysicsData->m_oOwnerInfo.m_pOwner = this;
	m_pPhysicsData->m_oMatrix             = *rEntityData.m_pMatrix;

	// The box's origin is its centre: moves the body down by the half height along the matrix's up axis
	TMatrix44& rMatrix = m_pPhysicsData->m_oMatrix;
	TVector4   vUp( rMatrix.m_f21, rMatrix.m_f22, rMatrix.m_f23, rMatrix.m_f24 );
	vUp.Multiply( rEntityData.m_pBBox->y );
	rMatrix.m_f41 -= vUp.x;
	rMatrix.m_f42 -= vUp.y;
	rMatrix.m_f43 -= vUp.z;

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
	m_pPhysicsData->m_pBody          = pPhysicsManager->CreateFixedBody( &m_pPhysicsData->m_oMatrix, TNULL );
	pPhysicsManager->SetBox( m_pPhysicsData->m_pBody, *rEntityData.m_pBBox );
	pPhysicsManager->SetListener( m_pPhysicsData->m_pBody, m_pPhysicsData->m_pHandler );
	pPhysicsManager->SetCollisionLayer( m_pPhysicsData->m_pBody, 0x200 );
	pPhysicsManager->SetCollisionGroup( m_pPhysicsData->m_pBody, 6 );
	pPhysicsManager->UpdateBodyTransform( m_pPhysicsData->m_pBody, TFALSE );

	if ( m_pPhysicsData->m_pBody )
		m_pPhysicsData->m_pBody->m_pUserData = &m_pPhysicsData->m_oOwnerInfo;

	pPhysicsManager->AddToSimulation( m_pPhysicsData->m_pBody, TFALSE );
}

// $deBlob: FUNCTION 0079e710
void APropSwitchPressure::DestroyPhysics()
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( m_pPhysicsData == TNULL )
		return;

	if ( m_pPhysicsData->m_pMover )
		pPhysicsManager->RemoveCustomMover( m_pPhysicsData->m_pMover );

	pPhysicsManager->RemoveFromSimulation( m_pPhysicsData->m_pBody, TFALSE );
	pPhysicsManager->DestroyBody( m_pPhysicsData->m_pBody );

	delete m_pPhysicsData->m_pHandler;
	m_pPhysicsData->m_pHandler = TNULL;

	delete m_pPhysicsData->m_pMover;
	m_pPhysicsData->m_pMover = TNULL;

	::operator delete( m_pPhysicsData );
	m_pPhysicsData = TNULL;
}

// $deBlob: FUNCTION 0079eef0
void APropSwitchPressure::UpdatePhysics()
{
	if ( m_pPhysicsData != TNULL )
	{
		m_pPhysicsData->m_oMatrix = m_oMatrix;

		TMatrix44& rMatrix = m_pPhysicsData->m_oMatrix;
		TVector4   vUp( rMatrix.m_f21, rMatrix.m_f22, rMatrix.m_f23, rMatrix.m_f24 );
		vUp.Multiply( m_vBBox.y );
		rMatrix.m_f41 -= vUp.x;
		rMatrix.m_f42 -= vUp.y;
		rMatrix.m_f43 -= vUp.z;

		TPhysicsManager::GetSingleton()->UpdateBodyTransform( m_pPhysicsData->m_pBody, TFALSE );
	}

	if ( m_uiSwitchFlags & SWITCHFLAG_Locked )
	{
		TMatrix44 mTransform = m_oMatrix;
		mTransform.RotateZ( 1.5707964f );
		m_pShield->SetTransform( mTransform );
	}
}

// $deBlob: FUNCTION 0079f000
void APropSwitchPressure::Unlock()
{
	m_eState = STATE_Idle;
	TASSERT( m_uiSwitchFlags & SWITCHFLAG_Locked );

	m_pShield->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	TSTATICCAST( ACollisionInstance2, m_pShield->GetModelInstance()->GetCollisionInstance() )->RemoveFromWorld( 0x60 );

	TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( sm_explosionEffect );

	if ( pInstance != TNULL )
	{
		// The effect and the sound play 2.5 units in front of the switch
		TVector4 vOffset;
		vOffset.Multiply( TVector4( 0.0f, 0.0f, 1.0f, 1.0f ), 2.5f );

		TVector4 vPosition;
		TMatrix44::RotateVector( vPosition, m_oMatrix, vOffset );
		vPosition.x += m_oMatrix.m_f41;
		vPosition.y += m_oMatrix.m_f42;
		vPosition.z += m_oMatrix.m_f43;

		TMatrix44 mTransform = TMatrix44::IDENTITY;
		mTransform.SetTranslation( vPosition );
		pInstance->GetTransform() = mTransform;

		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 0x20 ), vPosition, FMOD_EVENT_DEFAULT );
	}

	KillManualAllAnimations( -1.0f );
	AddManualAnimation( m_iReleaseAnim, -1.0f, 0.15f, 1.0f, 1.0f );

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	T2LocalisedString wszString = T2Locale::GetSingleton()->GetString( 0x1A );
	pGameStatus->GetLevelData().ShowMessage( wszString, 6, TNULL, 0 );

	if ( pGameStatus->GetLevelProperties()->m_sprintLevel == 0 )
		AMusicManager::GetSingleton()->PlayMusicSFX( "gate_open", 0x2041, &TVector4::VEC_ZERO, TNULL );
}

// The sector switch is touched while locked: "N more points" in m_wszUnlockMessage
// $deBlob: FUNCTION 0079ea90
void APropSwitchPressure::UpdateUnlockMessage()
{
	if ( m_eState == STATE_Locked && m_uiUnlockType == 0 )
	{
		TINT iPoints = AGameStatus::GetSingleton()->GetLevelData().GetPoints( 0 );

		if ( iPoints < m_iUnlockScore )
		{
			T2LocalisedString wszFormat = T2Locale::GetSingleton()->GetString( 0xBF );
			TStringManager::String16Format( m_wszUnlockMessage, 0x100, wszFormat, m_iUnlockScore - iPoints );
		}
	}
}

// $deBlob: FUNCTION 0079ed10
void APropSwitchPressure::StartCountdownSound()
{
	if ( !( m_uiSwitchFlags & SWITCHFLAG_SoundPlaying ) )
	{
		if ( m_pSound )
			m_pSound->stop( false );

		m_pSound = ASoundInterface::PlayEvent( ASoundMap::GROUPID( 8 ), ASoundMap::EVENTID( 0x92 ), FMOD_EVENT_DEFAULT, TTRUE );
		m_uiSwitchFlags |= SWITCHFLAG_SoundPlaying;
	}
}

// $deBlob: FUNCTION 0079ed60
void APropSwitchPressure::UpdateCountdownSound()
{
	if ( !( m_uiSwitchFlags & SWITCHFLAG_SoundPlaying ) || m_pSound == TNULL )
		return;

	TFLOAT fVelocity = 1.0f;
	TFLOAT fRatio    = 1.0f;

	if ( m_fDeactivationDelay > 0.0f )
		fRatio = ( m_fDeactivationDelay - m_fDeactivationTimer ) / m_fDeactivationDelay;

	switch ( sm_iSoundMode )
	{
		case 0:
		{
			fVelocity = ( ( m_fDeactivationDelay - m_fDeactivationTimer ) / m_fDeactivationDelay ) * 3.0f + 1.0f;
		}
		break;
		case 1:
		{
			TMath::Clip( fRatio, 0.0f, 1.0f );

			if ( 0.5f > fRatio )
				fVelocity = 1.0f;
			else if ( 0.75f > fRatio )
				fVelocity = 2.0f;
			else
				fVelocity = 4.0f;
		}
		break;
		case 2:
		{
			fVelocity = fRatio * fRatio * 3.0f + 1.0f;
		}
		break;
	}

	FMOD::EventParameter* pParameter = TNULL;

	// NOTE: retail asks for the parameter index -1
	if ( m_pSound->getParameterByIndex( -1, &pParameter ) == FMOD_OK )
		pParameter->setVelocity( fVelocity );
}

// $deBlob: FUNCTION 0079eeb0
void APropSwitchPressure::StopCountdownSound()
{
	if ( m_uiSwitchFlags & SWITCHFLAG_SoundPlaying )
	{
		if ( m_pSound )
		{
			m_pSound->stop( false );
			m_pSound = TNULL;
		}

		m_uiSwitchFlags &= ~SWITCHFLAG_SoundPlaying;
	}
}

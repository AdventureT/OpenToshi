#include "pch.h"
#include "Sound/APositionalSoundManager.h"
#include "APropGrassBomb.h"
#include "AEntityPhysicsMover.h"
#include <Physics/TSphereData.h>
#include "AEntityModelComponent.h"
#include "AEntityParams.h"
#include "Animation/AModelInstance2.h"
#include "APickupManager.h"
#include "ATriggerUtil.h"
#include "ABlobEntity/ABlobEntity.h"
#include "ABlobEntity/ABlobColourComponent.h"
#include "ABlobEntity/ABlobAnimationComponent.h"
#include "ABlobEntity/ABlobZJumpComponent.h"
#include "AEntityIOPacket.h"
#include "Cameras/ACameraManager.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "GameInterface/AGameStateController.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "XUI/InGame/HUD/ABlobLetterBoxHud.h"
#include <XUI/TXUI.h>
#include <XUI/TXUIScene.h>
#include <Toshi/TStringManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>

#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TParticleEffect* APropGrassBomb::sm_pIdleEffect;
Toshi::TParticleEffect* APropGrassBomb::sm_pPickupEffect;

TSTATICASSERT( sizeof( APropGrassBomb ) == 0xF4 );

// $deBlob: FUNCTION 00786cd0
// $deBlob: FUNCTION 00786ce0
// $deBlob: FUNCTION 00786d50
// $deBlob: FUNCTION 00786db0
// $deBlob: FUNCTION 00786dc0
// $deBlob: FUNCTION 00540be0
// $deBlob: FUNCTION 00859d70
TDEFINE_CLASS( APropGrassBomb );

// $deBlob: FUNCTION 00786dd0
APropGrassBomb::APropGrassBomb()
{
	m_uiBombFlags       = 0;
	m_iRequiredScore    = 0;
	m_iCameraRegion     = -1;
	m_iScriptedSequence = -1;
	m_fTimer            = -1.0f;
	m_oNode.m_pOwner    = this;
	m_pIdleParticles    = TNULL;
	m_uiState           = 0;
	m_fLetterboxDelay   = 3.0f;
	TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType )
	[1] = 0xD;

	if ( sm_pIdleEffect == TNULL )
	{
		TParticleEffectManager* pEffectManager = TParticleSystem::GetSingleton()->GetEffectManager();
		sm_pIdleEffect                         = pEffectManager->FindEffect( "Blob_Grassbomb_Idle" );
		sm_pPickupEffect                       = pEffectManager->FindEffect( "Blob_Pickup_GrassBomb" );
	}
}

// $deBlob: FUNCTION 00786ec0
// $deBlob: FUNCTION 00788230
// $deBlob: FUNCTION 0078821b
APropGrassBomb::~APropGrassBomb()
{
	sm_pPickupEffect = TNULL;
	sm_pIdleEffect   = TNULL;

	if ( m_pIdleParticles != TNULL )
	{
		m_pIdleParticles->StopInstant();
		m_pIdleParticles = TNULL;
	}

	if ( m_oNode.IsLinked() )
		m_oNode.Remove();

	if ( m_uiBombFlags & BOMBFLAG_BossCompletion )
		AGameStatus::GetSingleton()->SetBossGrassBomb( TNULL );
}

// $deBlob: FUNCTION 00786fe0
void APropGrassBomb::Tick( TFLOAT a_fDeltaTime )
{
	AProp::Tick( a_fDeltaTime );

	if ( GetLOD() == FLAG_HighLOD && ( TUINT8( m_Flags >> 2 ) & 1 ) && m_pModelComponent != TNULL && m_pModelComponent->GetModelInstance() != TNULL )
	{
		AModelInstance2* pModel    = m_pModelComponent->GetModelInstance();
		TVector4         vPosition = GetTranslation();

		pModel->SetTransform( *APickupManager::GetSingleton()->GetPickupRotation( 0, TFALSE ) );
		pModel->SetPosition( vPosition );
	}

	// The bomb's scripted camera plays: the letterbox shows its bottom display after a delay
	if ( ( TUINT8( m_Flags >> 2 ) & 1 ) && ( m_uiBombFlags & BOMBFLAG_Active ) && !( m_uiBombFlags & BOMBFLAG_BottomDisplayStarted ) && m_iScriptedSequence != -1 )
	{
		APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobData( 0 );
		TXUI*                     pXUI      = TXUI::GetSingleton();

		if ( pBlobData->IsSSCameraPlaying( m_iScriptedSequence ) && !AGameStateController::GetSingleton()->IsFadingFromBlack( 0 ) &&
		     pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() && pXUI->GetActiveScene() != TNULL )
		{
			TXUIScene* pScene = pXUI->GetActiveScene();

			if ( pScene->IsA( &TGetClass( ABlobLetterBoxHud ) ) )
			{
				m_fLetterboxDelay -= a_fDeltaTime;

				if ( m_fLetterboxDelay <= 0.0f )
				{
					m_uiBombFlags |= BOMBFLAG_BottomDisplayStarted;
					TSTATICCAST( ABlobLetterBoxHud, pScene )->PlayBottomDisplay();
				}
			}
		}
	}

	TFLOAT fOldTimer = m_fTimer;

	if ( fOldTimer >= 0.0f )
	{
		m_fTimer = fOldTimer + a_fDeltaTime;

		// The arrow that points at the bomb (the current player's blob)
		if ( 2.5f < fOldTimer + a_fDeltaTime && APlayerManager::IsSingletonCreated() )
		{
			ABlobEntity* pArrowBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() );

			if ( pArrowBlob != TNULL )
			{
				TVector4 vOffset;
				vOffset.Multiply( TVector4::VEC_NEGY, 20.0f );

				const TVector4& rPosition = GetTranslation();
				TVector4        vTarget( rPosition.x + vOffset.x, rPosition.y + vOffset.y, rPosition.z + vOffset.z, 0.0f );

				const TMatrix44& rCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
				TVector4         vCamera( rCamera.m_f41, rCamera.m_f42, rCamera.m_f43, rCamera.m_f44 );
				TVector4         vNormal( -rCamera.m_f31, -rCamera.m_f32, -rCamera.m_f33, 1.0f );

				TASSERT( vNormal.IsNormalised() );

				TVector4 vDirection( vTarget.x - vCamera.x, vTarget.y - vCamera.y, vTarget.z - vCamera.z, 1.0f );
				vDirection.Normalise();

				TVector4 vScaled;
				vScaled.Multiply( vDirection, 5.0f );

				TVector4 vArrow( vScaled.x + vCamera.x, vScaled.y + vCamera.y, vScaled.z + vCamera.z, vCamera.w );

				if ( 2.5f < fOldTimer )
				{
					pArrowBlob->UpdateShockwave( vArrow, vNormal );
				}
				else
				{
					pArrowBlob->StartShockwave( vArrow, vNormal, 1.5f, 0.6f, TTRUE );
				}
			}
		}

		switch ( m_uiState )
		{
			case 0:
			{
				// Collected: the pickup effect, the blob is held
				TParticleEffectInstance* pPickup = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( sm_pPickupEffect );

				if ( pPickup != TNULL )
					pPickup->GetTransform() = m_oMatrix;

				SetVisible( TFALSE );
				m_oNode.Remove();

				if ( m_pIdleParticles != TNULL )
				{
					m_pIdleParticles->StopInstant();
					m_pIdleParticles = TNULL;
				}

				AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TTRUE;

				ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
				pBlob->SetState( 8 );
				m_uiState = 1;
				break;
			}
			case 1:
			{
				ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

				if ( m_fTimer < m_fStopPlayerTime )
					break;

				if ( pBlob->GetBlobState() != ABlobEntity::STATE_GRASSBOMB )
					pBlob->SetState( 8 );

				TXUI* pXUI = TXUI::GetSingleton();

				if ( !Toshi::TPhysicsObjectContainer::IsInSimulation( pBlob->m_pBody ) && pXUI->GetActiveScene() != TNULL && pXUI->GetActiveScene()->IsA( &TGetClass( ABlobHud ) ) &&
				     pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() )
				{
					// The letterbox
					pXUI->TransitionTo( TNULL, pXUI->CreateScene( pXUI->FindResource( "letterbox.xur" ), 0 ) );
					pBlob->GetAnimationComponent()->SetState( ABlobAnimationComponent::STATE_GRASSBOMB );
					m_uiState                                                                       = 2;
					APlayerManager::GetSingleton()->GetBlobDataOf( pBlob )->m_bKeyFrameCameraActive = TTRUE;
					break;
				}

				// Waits: the timer doesn't advance
				m_fTimer = fOldTimer;
				break;
			}
			case 2:
				if ( m_fStartTransitionTime <= m_fTimer )
				{
					ACameraManager::GetSingleton()->GetNedCameras().SetCurrentRegion( m_iCameraRegion );
					m_uiState = 3;
				}
				break;
			case 3:
				if ( m_fTriggerTime <= m_fTimer )
				{
					TriggerAllTargets( this );

					ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

					if ( pBlob->GetColourComponent()->GetCurrentColour() & ABlobColourComponent::COLOUR_BLACK )
						pBlob->WashOff();

					m_uiState = 4;

					if ( m_uiBombFlags & BOMBFLAG_BossCompletion )
					{
						AGameStateController::GetSingleton()->SetFadeColour( 0xFFFFFFFF, 0 );
						AGameStateController::GetSingleton()->SetFade( 0.0f, 0 );

						TFLOAT fDuration = m_fReturnToGameTime - m_fTimer;

						if ( fDuration < 0.0f )
							fDuration = 0.01f;

						AGameStateController::GetSingleton()->SetRenderFadesFirst( TTRUE );
						AGameStateController::GetSingleton()->FadeToBlack( fDuration, 0 );
					}
				}
				break;
			case 4:
			{
				if ( m_fTimer < m_fEndTransitionTime )
					break;

				TXUI* pXUI = TXUI::GetSingleton();

				TASSERT( 0 == TStringManager::String16Compare( pXUI->GetActiveScene()->GetID(), L"LetterBox" ) );

				if ( pXUI->GetActiveScene() != TNULL && pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() &&
				     TStringManager::String16Compare( pXUI->GetActiveScene()->GetID(), L"LetterBox" ) == 0 )
				{
					if ( !( m_uiBombFlags & BOMBFLAG_BossCompletion ) )
						pXUI->TransitionBack( pXUI->GetActiveScene() );

					ACameraManager::GetSingleton()->GetNedCameras().SetCurrentRegion( -1 );
					m_uiState = 5;
					break;
				}

				// Waits: the timer doesn't advance
				m_fTimer = fOldTimer;
				break;
			}
			case 5:
				if ( m_fReturnToGameTime <= m_fTimer )
				{
					AMusicManager::GetSingleton()->ReleaseSnapshot();
					m_fTimer = -1.0f;
					SetTicking( TFALSE );
					m_Flags |= FLAG_DontTick;

					ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
					pBlob->SetState( 1 );

					AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
					APlayerManager::GetSingleton()->DecrementSSCameraDelay();

					if ( m_uiBombFlags & BOMBFLAG_BossCompletion )
						AGameStatus::GetSingleton()->SetUnk00C( TTRUE );

					APlayerManager::GetSingleton()->GetBlobDataOf( pBlob )->m_bKeyFrameCameraActive = TFALSE;
				}
				break;
		}
	}

	// Waiting for its scripted sequence: activates once the camera is playing (or no sequence is queued)
	if ( m_uiBombFlags & BOMBFLAG_WaitsForSequence )
	{
		TASSERT( m_iScriptedSequence >= 0 && AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint );

		APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobData( 0 );
		TBOOL                     bActivate;

		if ( pBlobData->IsSSCameraPlaying( m_iScriptedSequence ) )
			bActivate = pBlobData->m_iCameraState == 5;
		else
			bActivate = !APlayerManager::GetSingleton()->IsSSCameraQueued( 0, m_iScriptedSequence );

		if ( bActivate )
			Activate();
	}
}

// $deBlob: FUNCTION 00787ec0
void APropGrassBomb::OnLODChanging( TINT a_iLOD )
{
	if ( !( a_iLOD & FLAG_LowLOD ) && ( TUINT8( m_Flags >> 2 ) & 1 ) && ( m_uiBombFlags & BOMBFLAG_Active ) )
	{
		if ( !( m_uiBombFlags & BOMBFLAG_SoundPlaying ) )
		{
			APositionalSoundManager::GetSingleton()->AddEntitySound( this, 1, 0x25 );
			m_uiBombFlags |= BOMBFLAG_SoundPlaying;
		}
	}
	else if ( m_uiBombFlags & BOMBFLAG_SoundPlaying )
	{
		m_uiBombFlags &= ~BOMBFLAG_SoundPlaying;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, 1, 0x25 );
	}

	AVisibleEntity::OnLODChanging( a_iLOD );

	if ( ( m_uiBombFlags & BOMBFLAG_BossCompletion ) && !( m_uiBombFlags & BOMBFLAG_Active ) )
		m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
}

// $deBlob: FUNCTION 00787d80
void APropGrassBomb::Create()
{
	AProp::Create();
	SetVisible( TTRUE );

	AGameStatus::GetSingleton()->GetLevelData().AddGrassBomb();
}

// $deBlob: FUNCTION 00786fc0
// $deBlob: FUNCTION 00788318
void APropGrassBomb::PostCreate()
{
	AProp::PostCreate();

	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
		TryActivate();
}

// $deBlob: FUNCTION 00787980
// $deBlob: FUNCTION 00788323
void APropGrassBomb::ProcessParams( const AEntityParams& a_rParams )
{
	AProp::ProcessParams( a_rParams );

	AEntityParam::Collision** ppCollision = TNULL;

	if ( !a_rParams.GetCollision( ppCollision ) )
	{
		// A sphere (radius 3.5) centred 3.5 below the origin
		TSphereData oSphere;
		oSphere.SetRadius( 3.5f );
		TShapeBaseClass* pShape = &oSphere;

		TMatrix44 matOffset = TMatrix44::IDENTITY;
		matOffset.SetTranslation( TVector4( 0.0f, -3.5f, 0.0f, 1.0f ) );

		CreatePhysics( m_pTypeData->m_pTRBData, &pShape, &matOffset, 1, TNULL, TFALSE );
		SetPhysicsMover( new AEntityPhysicsMover( this ) );
	}
	else
	{
		CreatePhysics( m_pTypeData->m_pTRBData, *ppCollision, TNULL, TFALSE );
		SetPhysicsMover( new AEntityPhysicsMover( this ) );
	}

	SetModelFade( 0.25f );
	SetPhysicsEnabled( TFALSE );

	a_rParams.GetValue( m_iCameraRegion, "Camera Region" );
	a_rParams.GetFloat( m_fStopPlayerTime, "StopPlayerTime" );
	a_rParams.GetFloat( m_fStartTransitionTime, "StartTransitionTime" );
	a_rParams.GetFloat( m_fTriggerTime, "TriggerTime" );
	a_rParams.GetFloat( m_fEndTransitionTime, "EndTransitionTime" );
	a_rParams.GetFloat( m_fReturnToGameTime, "ReturnToGameTime" );
	a_rParams.GetValue( m_iRequiredScore, "Required Score" );

	TBOOL bBossCompletion = TFALSE;

	if ( a_rParams.GetValue( bBossCompletion, "Trigger Boss Level Completion" ) && bBossCompletion )
	{
		m_uiBombFlags |= BOMBFLAG_BossCompletion;

		AGameStatus::GetSingleton()->SetBossGrassBomb( this );

		m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	}

	a_rParams.GetValue( m_iScriptedSequence, "Scripted Sequence" );

	TASSERT( m_fReturnToGameTime > m_fEndTransitionTime );
	TASSERT( m_fEndTransitionTime > m_fTriggerTime );
	TASSERT( m_fTriggerTime > m_fStartTransitionTime );
	TASSERT( m_fStartTransitionTime > m_fStopPlayerTime );

	if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
		AddToWaitingList();
}

// $deBlob: FUNCTION 00787840
void APropGrassBomb::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_SLAM )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );

		TASSERT( pBlob->GetZJumpComponent()->GetState() == ABlobZJumpComponent::STATE_ATTACKSLAM );

		// NOTE: the packet's data is a pointer to the type of the collision (3: the blob's body)
		if ( !( pBlob->GetBlobFlags() & 0x8000 ) && *TREINTERPRETCAST( TINT*, a_rPacket.m_iData ) == 3 && ( TUINT8( m_Flags >> 2 ) & 1 ) &&
		     ( m_uiBombFlags & BOMBFLAG_Active ) && m_fTimer <= 0.0f && m_fTimer != 0.0f )
		{
			SetTicking( TTRUE );
			m_fTimer = 0.0f;

			AMusicManager::GetSingleton()->PlaySnapshot( "grassbomb" );
			ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 0x23 ), GetTranslation(), FMOD_EVENT_MODE( 0 ) );
			SetVisible( TFALSE );

			AGameStatus::GetSingleton()->GetLevelData().AddGrassBombCollected( 0 );
			APlayerManager::GetSingleton()->IncrementSSCameraDelay();
		}
	}

	AProp::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 00787ea0
TBOOL APropGrassBomb::IsTargetable()
{
	return ( TUINT8( m_Flags >> 2 ) & 1 ) && ( m_uiBombFlags & BOMBFLAG_Active );
}

// $deBlob: FUNCTION 00787da0
void APropGrassBomb::Reset( ABaseEntity* a_pEntity )
{
	APositionalSoundManager::GetSingleton()->RemoveSound( this, 1, 0x25 );

	m_uiBombFlags &= ~( BOMBFLAG_Active | BOMBFLAG_BottomDisplayStarted | BOMBFLAG_WaitsForSequence );

	SetVisible( TTRUE );
	SetModelFade( 0.25f );
	SetPhysicsEnabled( TFALSE );
	SetTicking( TFALSE );
	m_Flags &= ~FLAG_DontTick;
	m_uiState = 0;

	if ( m_oNode.IsLinked() )
		m_oNode.Remove();

	if ( m_pIdleParticles != TNULL )
	{
		m_pIdleParticles->StopInstant();
		m_pIdleParticles = TNULL;
	}

	m_fTimer = -1.0f;
	ACameraManager::GetSingleton()->GetNedCameras().SetCurrentRegion( -1 );

	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
		TryActivate();
	else
		AddToWaitingList();

	m_fLetterboxDelay = 3.0f;
}

void APropGrassBomb::SetModelFade( TFLOAT a_fFade )
{
	TModelInstance* pModelInstance = m_pModelComponent->GetModelInstance()->GetModelInstance();

	pModelInstance->SetForceFade( TTRUE );
	pModelInstance->SetFade( a_fFade );
}

// $deBlob: FUNCTION 00787fc0
void APropGrassBomb::Activate()
{
	m_uiBombFlags = ( m_uiBombFlags & ~BOMBFLAG_WaitsForSequence ) | BOMBFLAG_Active;

	SetModelFade( 1.0f );
	SetPhysicsEnabled( TTRUE );

	// NOTE: AGameStatus's list of the active bombs is a T2GenericDList of BombNodes
	TREINTERPRETCAST( BombList*, &AGameStatus::GetSingleton()->GetLevelData().m_oList78C )->PushBack( &m_oNode );

	if ( m_pIdleParticles == TNULL )
		m_pIdleParticles = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( sm_pIdleEffect );

	if ( m_pIdleParticles != TNULL )
	{
		TMatrix44 matTransform = TMatrix44::IDENTITY;
		matTransform.SetTranslation( GetBasePosition() );

		m_pIdleParticles->GetTransform() = matTransform;
	}

	if ( m_uiBombFlags & BOMBFLAG_BossCompletion )
		m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, ( m_Flags & FLAG_Visible ) != 0 );
}

// $deBlob: FUNCTION 00787f60
void APropGrassBomb::TryActivate()
{
	if ( !( m_uiBombFlags & ( BOMBFLAG_Active | BOMBFLAG_WaitsForSequence ) ) && ( TUINT8( m_Flags >> 2 ) & 1 ) )
	{
		if ( m_oNode.IsLinked() )
			m_oNode.Remove();

		if ( m_iScriptedSequence < 0 || AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
		{
			Activate();
			return;
		}

		APlayerManager::GetSingleton()->QueueSSCamera( 0, m_iScriptedSequence );
		m_uiBombFlags |= BOMBFLAG_WaitsForSequence;
	}
}

// $deBlob: FUNCTION 00788330
void APropGrassBomb::AddToWaitingList()
{
	// NOTE: AGameStatus's list of the waiting bombs is a T2GenericDList of BombNodes
	BombList& rList = *TREINTERPRETCAST( BombList*, &AGameStatus::GetSingleton()->GetLevelData().m_oList794 );

	BombList::Iterator it = rList.Begin();

	while ( it != rList.End() )
	{
		if ( CompareRequiredScore( &m_oNode, it.Get() ) < 0 )
			break;

		++it;
	}

	rList.InsertBefore( it, &m_oNode );
}

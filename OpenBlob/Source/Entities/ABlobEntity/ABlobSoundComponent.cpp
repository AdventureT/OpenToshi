#include "pch.h"
#include "ABlobSoundComponent.h"
#include "ABlobEntity.h"
#include "ABlobColourComponent.h"
#include "ABlobAnimationComponent.h"
#include "Animation/AModelInstance2.h"
#include "Sound/ASoundBreakpointComponent.h"
#include "Sound/ASoundInterface.h"
#include "Sound/AMusicManager.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "ALevelInformation.h"

#include <Sound/TSound.h>
#include <Physics/Havok/TPhysicsManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TFLOAT s_fLockedBuildingStopTime = 0.25f;

// $deBlob: FUNCTION 0075d490
ABlobSoundComponent::ABlobSoundComponent( ABlobEntity* a_pBlob )
    : AEntityComponent2( a_pBlob )
{
	TASSERT( ( a_pBlob == TNULL ) || a_pBlob->IsA( &TGetClass( ABlobEntity ) ) );

	m_pBlob                = a_pBlob;
	m_bBouncePlayed        = TFALSE;
	m_pRollEvent           = TNULL;
	m_pZJumpEvent          = TNULL;
	m_pSwimEvent           = TNULL;
	m_pHotPlateEvent       = TNULL;
	m_pElectrocuteEvent    = TNULL;
	m_pHurtLoopA           = TNULL;
	m_pHurtLoopB           = TNULL;
	m_pLockedBuildingEvent = TNULL;
	m_uiZJumpNodeCount     = 0;
	m_fAirTimer            = 0.0f;
	m_fTime                = 0.0f;
	m_uiFlags              = 0;

	if ( TSound::GetSingleton()->IsInitialised() )
	{
		m_pRollEvent = ASoundInterface::PlayEvent( ASoundMap::GROUPID_BLOB_SFX, ASoundMap::EVENTID( 6 ), FMOD_EVENT_DEFAULT, TTRUE );
		m_pSwimEvent = ASoundInterface::PlayEvent( ASoundMap::GROUPID_BLOB_SFX, ASoundMap::EVENTID( 7 ), FMOD_EVENT_DEFAULT, TTRUE );

		m_pRollEvent->start();
		m_pRollEvent->setMute( true );
		m_uiFlags |= FLAGS_ROLL_MUTED;

		m_pSwimEvent->start();
		m_pSwimEvent->setMute( true );
		m_uiFlags |= FLAGS_SWIM_MUTED;
	}
}

// $deBlob: FUNCTION 0075e3e0
void ABlobSoundComponent::PlayElectrocutionSnapshot()
{
	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE && !AMusicManager::GetSingleton()->IsInked() )
		AMusicManager::GetSingleton()->PlaySnapshot( "electrocution" );
}

// $deBlob: FUNCTION 0075e410
void ABlobSoundComponent::ReleaseElectrocutionSnapshot()
{
	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE && !AMusicManager::GetSingleton()->IsInked() )
		AMusicManager::GetSingleton()->ReleaseSnapshot();
}

// $deBlob: FUNCTION 0075d5f0
// $deBlob: FUNCTION 0075e440
ABlobSoundComponent::~ABlobSoundComponent()
{
	// NOTE: retail stops only the rolling loop
	if ( m_pRollEvent )
	{
		m_pRollEvent->stop( true );
		m_pRollEvent = TNULL;
	}
}

// $deBlob: FUNCTION 0075d660
void ABlobSoundComponent::Tick( TFLOAT a_fDeltaTime )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return;

	TBOOL bRolling  = TFALSE;
	TBOOL bSwimming = TFALSE;

	m_fTime += a_fDeltaTime;

	if ( m_fTime > s_fLockedBuildingStopTime && m_pLockedBuildingEvent )
	{
		m_pLockedBuildingEvent->stop( false );
		m_pLockedBuildingEvent = TNULL;
	}

	switch ( m_pBlob->m_eState )
	{
		case ABlobEntity::STATE_ROLL:
		case ABlobEntity::STATE_STUCKWALL:
			bRolling = TTRUE;
			break;
		case ABlobEntity::STATE_BALLISTIC:
			if ( m_uiFlags & FLAGS_AIR_TIMER )
			{
				m_fAirTimer -= a_fDeltaTime;

				if ( m_fAirTimer < 0.0f )
					m_uiFlags &= ~FLAGS_AIR_TIMER;
			}
			break;
		case ABlobEntity::STATE_FLOAT:
			bSwimming = TTRUE;
			break;
	}

	TVector4 vVelocity;
	TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pBlob->m_pBody, vVelocity );
	TFLOAT fSpeed = vVelocity.Magnitude();

	if ( bRolling && m_pRollEvent )
	{
		if ( fSpeed >= 10.0f )
		{
			if ( m_uiFlags & FLAGS_ROLL_MUTED )
			{
				m_pRollEvent->setMute( false );
				m_uiFlags &= ~FLAGS_ROLL_MUTED;
			}

			TMath::Clip( fSpeed, 10.0f, 30.0f );
			fSpeed = ( fSpeed - 20.0f ) / 10.0f;
			m_pRollEvent->setPitch( fSpeed, FMOD_EVENT_PITCHUNITS_RAW );
			ASoundInterface::SetPosition( m_pRollEvent, m_pBlob->GetBodyPosition() );
		}
		else if ( !( m_uiFlags & FLAGS_ROLL_MUTED ) )
		{
			m_pRollEvent->setMute( true );
			m_uiFlags |= FLAGS_ROLL_MUTED;
		}
	}

	if ( bSwimming )
	{
		if ( m_pSwimEvent )
		{
			if ( fSpeed >= 5.0f )
			{
				if ( m_uiFlags & FLAGS_SWIM_MUTED )
				{
					m_pSwimEvent->setMute( false );
					m_uiFlags &= ~FLAGS_SWIM_MUTED;
				}

				TMath::Clip( fSpeed, 5.0f, 30.0f );
				fSpeed = ( fSpeed - 30.0f ) / 25.0f;
				m_pSwimEvent->setPitch( fSpeed, FMOD_EVENT_PITCHUNITS_RAW );
				ASoundInterface::SetPosition( m_pSwimEvent, m_pBlob->GetBodyPosition() );
			}
			else if ( !( m_uiFlags & FLAGS_SWIM_MUTED ) )
			{
				m_pSwimEvent->setMute( true );
				m_uiFlags |= FLAGS_SWIM_MUTED;
			}
		}
	}
	else
	{
		if ( m_pHurtLoopB )
			ASoundInterface::SetPosition( m_pHurtLoopB, m_pBlob->GetBodyPosition() );

		if ( m_pHurtLoopA )
			ASoundInterface::SetPosition( m_pHurtLoopA, m_pBlob->GetBodyPosition() );
	}
}

// $deBlob: FUNCTION 0075d8c0
void ABlobSoundComponent::SetSoundsEnabled( TBOOL a_bEnabled )
{
	AModelInstance2* pModel = m_pBlob->GetAnimationComponent()->GetCharModelInstance();

	if ( pModel != TNULL && pModel->GetSoundBreakpointComponent() != TNULL )
		TSTATICCAST( ASoundBreakpointComponent, pModel->GetSoundBreakpointComponent() )->SetMuted( !a_bEnabled );
}

// $deBlob: FUNCTION 0075da40
void ABlobSoundComponent::PlayLiquidJump()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_BLOB_SFX, ASoundMap::EVENTID( 4 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075db50
void ABlobSoundComponent::PlayZJumpLaunch( TFLOAT a_fFlightTime )
{
	if ( ( m_uiFlags & FLAGS_ZJUMP_FLIGHT ) == 0 && 1.25f < a_fFlightTime )
	{
		m_uiFlags |= FLAGS_ZJUMP_FLIGHT | FLAGS_AIR_TIMER;
		m_fAirTimer = ( a_fFlightTime - 1.25f ) - 0.033333335f;

		if ( m_fAirTimer < 0.0f )
			m_fAirTimer = 0.0f;
	}

	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_BLOB_SFX, ASoundMap::EVENTID( 4 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075daa0
void ABlobSoundComponent::PlayZJumpLand()
{
	m_uiZJumpNodeCount = 0;

	if ( m_pZJumpEvent )
	{
		m_pZJumpEvent->stop( true );
		m_pZJumpEvent = TNULL;
	}

	m_uiFlags &= ~FLAGS_ZJUMP_FLIGHT;
	ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 50 ), FMOD_EVENT_DEFAULT, TTRUE );
}

// $deBlob: FUNCTION 0075da60
void ABlobSoundComponent::StartZJumpPointLoop()
{
	if ( m_pZJumpEvent )
	{
		m_pZJumpEvent->stop( true );
		m_pZJumpEvent = TNULL;
	}

	m_uiFlags &= ~FLAGS_ZJUMP_FLIGHT;
	m_pZJumpEvent      = ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x3B ), FMOD_EVENT_DEFAULT, TTRUE );
	m_uiZJumpNodeCount = 0;
}

// $deBlob: FUNCTION 0075dae0
void ABlobSoundComponent::PlayZJumpEnd()
{
	if ( m_pZJumpEvent )
	{
		m_pZJumpEvent->stop( true );
		m_pZJumpEvent = TNULL;
	}

	m_uiFlags &= ~FLAGS_ZJUMP_FLIGHT;
	ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x3A ), FMOD_EVENT_DEFAULT, TTRUE );
}

// $deBlob: FUNCTION 0075dbb0
void ABlobSoundComponent::PlayZJumpNode()
{
	TUINT32 auiEvents[ 8 ] = { 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0 };
	ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( auiEvents[ m_uiZJumpNodeCount ] ), FMOD_EVENT_DEFAULT, TTRUE );

	m_uiZJumpNodeCount++;

	if ( 7 < TINT( m_uiZJumpNodeCount ) )
		m_uiZJumpNodeCount = 7;
}

// $deBlob: FUNCTION 0075dc10
void ABlobSoundComponent::OnZJumpPoint()
{
}

// $deBlob: FUNCTION 0075dc20
void ABlobSoundComponent::PlayLockAim()
{
	ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x30 ), FMOD_EVENT_DEFAULT, TTRUE );
}

// $deBlob: FUNCTION 0075dc30
void ABlobSoundComponent::PlayLockOn()
{
	m_uiZJumpNodeCount = 0;
	ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x2F ), FMOD_EVENT_DEFAULT, TTRUE );
}

// $deBlob: FUNCTION 0075dc60
void ABlobSoundComponent::PlayLockChange()
{
	ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x30 ), FMOD_EVENT_DEFAULT, TTRUE );
}

// $deBlob: FUNCTION 0075d900
void ABlobSoundComponent::UpdateSquashBounce()
{
	TFLOAT fSquash = m_pBlob->GetContactSquash();

	if ( fSquash < 0.7f )
	{
		if ( !m_bBouncePlayed )
		{
			m_bBouncePlayed = TTRUE;
			ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_BLOB_SFX, ASoundMap::EVENTID( 0 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
			AMusicManager::GetSingleton()->PlayMusicSFX( "blob_bounce", AMusicManager::SS_START_POSITION, &m_pBlob->GetBodyPosition(), TNULL );
		}
	}
	else if ( fSquash > 0.8f )
	{
		m_bBouncePlayed = TFALSE;
	}
}

// $deBlob: FUNCTION 0075e2a0
void ABlobSoundComponent::PlayLockedBuilding( void* a_pUnused )
{
	m_fTime = 0.0f;

	if ( m_pLockedBuildingEvent == TNULL )
		m_pLockedBuildingEvent = ASoundInterface::PlayEvent( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 0x1D ), FMOD_EVENT_MODE( 0 ), TTRUE );
}

// $deBlob: FUNCTION 0075e0d0
// The pattern pickup's music sound, at Blob's body position (APickupPattern::OnCollected)
void ABlobSoundComponent::PlayPatternPickup()
{
	AMusicManager::GetSingleton()->PlayMusicSFX( "pattern_pickup", AMusicManager::SS_START_POSITION, &m_pBlob->GetBodyPosition(), TNULL );
}

// $deBlob: FUNCTION 0075e120
// The life pickup's sound at Blob's body position (APickupLife::OnCollected)
void ABlobSoundComponent::PlayLifePickup()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 39 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075e190
// The energy pickup's sound at Blob's body position (APickupEnergy::OnCollected)
void ABlobSoundComponent::PlayEnergyPickup()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 30 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075dc70
void ABlobSoundComponent::OnColourChanging( TUINT32 a_uiColour )
{
	if ( a_uiColour & 0x10 )
	{
		AMusicManager::GetSingleton()->ResetScore();

		TUINT32 uiColour = m_pBlob->m_pColourComponent->GetCurrentColour();

		if ( uiColour & 0x10 )
			uiColour = 0x10;

		if ( uiColour != a_uiColour )
			ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_BLOB_SFX, ASoundMap::EVENTID( 24 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
	}
	else
	{
		if ( AMusicManager::GetSingleton()->IsPitchChangeKept() )
			AMusicManager::GetSingleton()->ResetPitch( 1.5f );

		AMusicManager::GetSingleton()->ReleaseSnapshot();
	}
}

// $deBlob: FUNCTION 0075e140
void ABlobSoundComponent::PlayEvent38()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 38 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// DOES_NOTHING
void ABlobSoundComponent::NoEvent38()
{
}

// $deBlob: FUNCTION 0075e170
void ABlobSoundComponent::PlayPaintLost()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 40 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075d9f0
void ABlobSoundComponent::PlayLiquidEnter()
{
	TINT iSize = m_pBlob->m_iSize;

	if ( iSize >= 3 )
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_BLOB_SFX, ASoundMap::EVENTID( ( iSize < 6 ) + 8 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
	else
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_BLOB_SFX, ASoundMap::EVENTID( 10 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075db20
void ABlobSoundComponent::StopZJumpEvent()
{
	if ( m_pZJumpEvent )
	{
		m_pZJumpEvent->stop( true );
		m_pZJumpEvent = TNULL;
	}

	m_uiFlags &= ~FLAGS_ZJUMP_FLIGHT;
}

// $deBlob: FUNCTION 0075d970
void ABlobSoundComponent::PlaySizeUp()
{
	AMusicManager::GetSingleton()->PlayMusicSFX( "blob_size_up", 0x100, &m_pBlob->GetBodyPosition(), TNULL );
}

// $deBlob: FUNCTION 0075d9a0
void ABlobSoundComponent::PlaySizeDown()
{
	AMusicManager::GetSingleton()->PlayMusicSFX( "blob_size_down", 0x100, &m_pBlob->GetBodyPosition(), TNULL );

	TUINT32 uiColour = m_pBlob->GetPaintColour();

	if ( ( uiColour & 0x10 ) != 0 || uiColour == 0x10 )
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_BLOB_SFX, ASoundMap::EVENTID( 0x18 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075dcf0
void ABlobSoundComponent::StartHotPlate()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 0x5A ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );

	if ( m_pHotPlateEvent )
		m_pHotPlateEvent->stop( false );

	m_pHotPlateEvent = ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 0x5B ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );

	PlayElectrocutionSnapshot();
}

// $deBlob: FUNCTION 0075dd60
void ABlobSoundComponent::StopHotPlate()
{
	if ( m_pHotPlateEvent )
	{
		m_pHotPlateEvent->stop( false );
		m_pHotPlateEvent = TNULL;
	}

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE && !AMusicManager::GetSingleton()->IsInked() )
		AMusicManager::GetSingleton()->ReleaseSnapshot();
}

// $deBlob: FUNCTION 0075e020
void ABlobSoundComponent::PlayCannonFire()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 0x6A ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075dda0
void ABlobSoundComponent::StartElectrocution()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 88 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );

	if ( m_pElectrocuteEvent )
		m_pElectrocuteEvent->stop( false );

	m_pElectrocuteEvent = ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 87 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE && !AMusicManager::GetSingleton()->IsInked() )
		AMusicManager::GetSingleton()->PlaySnapshot( "electrocution" );
}

// $deBlob: FUNCTION 0075de10
void ABlobSoundComponent::StopElectrocution()
{
	if ( m_pElectrocuteEvent )
	{
		m_pElectrocuteEvent->stop( false );
		m_pElectrocuteEvent = TNULL;
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 86 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
	}

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE && !AMusicManager::GetSingleton()->IsInked() )
		AMusicManager::GetSingleton()->ReleaseSnapshot();
}

// $deBlob: FUNCTION 0075de70
void ABlobSoundComponent::PlayHurtHard()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 0x65 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075de90
void ABlobSoundComponent::StartHurtLoops()
{
	m_pHurtLoopB = ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 0x61 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
	m_pHurtLoopA = ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 0x60 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075ded0
void ABlobSoundComponent::StopHurtLoops()
{
	if ( m_pHurtLoopA )
	{
		m_pHurtLoopA->stop( false );
		m_pHurtLoopA = TNULL;
	}

	if ( m_pHurtLoopB )
	{
		m_pHurtLoopB->stop( false );
		m_pHurtLoopB = TNULL;
	}
}

// $deBlob: FUNCTION 0075df10
void ABlobSoundComponent::PlayHurt()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 0x62 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

// $deBlob: FUNCTION 0075e040
void ABlobSoundComponent::MuteLoopsForState()
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return;

	switch ( m_pBlob->GetBlobState() )
	{
		case ABlobEntity::STATE_BALLISTIC:
		case ABlobEntity::STATE_HOTPLATE:
		case ABlobEntity::STATE_LMARK:
		case ABlobEntity::STATE_BOSS:
		case ABlobEntity::STATE_CANNON:
		case ABlobEntity::STATE_GRASSBOMB:
		case ABlobEntity::STATE_RESPAWN:
		case ABlobEntity::STATE_LEVEL_EXIT:
		case ABlobEntity::STATE_DEAD:
		case ABlobEntity::STATE_FAIL:
		case ABlobEntity::STATE_VICTORY:
		case ABlobEntity::STATE_LOSTLASTLIFE:
			if ( m_pRollEvent )
			{
				m_pRollEvent->setMute( true );
				m_uiFlags |= FLAGS_ROLL_MUTED;
			}
			TFALLTHROUGH;
		case ABlobEntity::STATE_ROLL:
		case ABlobEntity::STATE_STUCKWALL:
			if ( m_pSwimEvent )
			{
				m_pSwimEvent->setMute( true );
				m_uiFlags |= FLAGS_SWIM_MUTED;
			}
			break;
		case ABlobEntity::STATE_FLOAT:
			if ( m_pRollEvent )
			{
				m_pRollEvent->setMute( true );
				m_uiFlags |= FLAGS_ROLL_MUTED;
			}
			break;
	}
}

// $deBlob: FUNCTION 0075dfd0
void ABlobSoundComponent::OnRespawn()
{
	AMusicManager::GetSingleton()->ReleaseSnapshot();

	if ( AMusicManager::GetSingleton()->IsPitchChangeKept() )
		AMusicManager::GetSingleton()->ResetPitch( 1.5f );
}

// $deBlob: FUNCTION 0075df30
void ABlobSoundComponent::PlayDeath()
{
	if ( m_pZJumpEvent )
	{
		m_pZJumpEvent->stop( true );
		m_pZJumpEvent = TNULL;
	}

	m_uiFlags &= ~FLAGS_ZJUMP_FLIGHT;
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_BLOB_SFX, ASoundMap::EVENTID( 5 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );

	static const TFLOAT s_fSprintEndTime = 2.0f;

	TBOOL bSlowDown = TTRUE;

	if ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel != 0 )
		bSlowDown = s_fSprintEndTime <= AGameStatus::GetSingleton()->GetTimeLeft();

	if ( APlayerManager::GetSingleton()->GetNumPlayers() <= 1 && bSlowDown )
		AMusicManager::GetSingleton()->SetPitch( 0.25f, 3.0f, TFALSE );
}

// $deBlob: FUNCTION 0075e1b0
void ABlobSoundComponent::PlayShakerSingle()
{
	AMusicManager::GetSingleton()->PlayMusicSFX( "shaker_single", AMusicManager::SS_START_POSITION, &m_pBlob->GetBodyPosition(), TNULL );
}

// $deBlob: FUNCTION 0075e1e0
void ABlobSoundComponent::StartShaker()
{
	TUINT32 uiFlags = AMusicManager::SS_PLOOP | AMusicManager::SS_START;

	if ( !AMusicManager::GetSingleton()->IsMusicPaused() )
		uiFlags = AMusicManager::SS_PLOOP | AMusicManager::SS_DEFAULT;

	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_LEVELS_LM_COMMON, ASoundMap::EVENTID( 211 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
	AMusicManager::GetSingleton()->PlayMusicSFX( "shaker_loop", uiFlags, &TVector4::VEC_ZERO, TNULL ); // X0Y0Z0W1
}

// $deBlob: FUNCTION 0075e230
void ABlobSoundComponent::SetShakerOff()
{
	AMusicManager::GetSingleton()->SetPLoopParameter( "shaker_loop", 0.0f );
}

// $deBlob: FUNCTION 0075e250
void ABlobSoundComponent::SetShakerOn()
{
	AMusicManager::GetSingleton()->SetPLoopParameter( "shaker_loop", 1.0f );
}

// $deBlob: FUNCTION 0075e270
void ABlobSoundComponent::StopShaker()
{
	AMusicManager::GetSingleton()->RemoveMusicSFXSyncSounds( "shaker_loop" );
	AMusicManager::GetSingleton()->StopPLoop( "shaker_loop" );
}

// $deBlob: FUNCTION 0075e2d0
void ABlobSoundComponent::PlaySqueeze()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 16 ), ASoundMap::EVENTID( 230 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

static const TCHAR* s_szRainbowSnapshot = "rainbow";

// $deBlob: FUNCTION 0075e310
void ABlobSoundComponent::PlayRainbow()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 42 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
	AMusicManager::GetSingleton()->PlaySnapshot( s_szRainbowSnapshot );
}

// $deBlob: FUNCTION 0075e340
void ABlobSoundComponent::ReleaseRainbowSnapshot()
{
	if ( AMusicManager::GetSingleton()->IsSnapshot( s_szRainbowSnapshot ) )
		AMusicManager::GetSingleton()->ReleaseSnapshot();
}

// $deBlob: FUNCTION 0075e370
void ABlobSoundComponent::StopLoops()
{
	if ( m_pZJumpEvent )
	{
		m_pZJumpEvent->stop( true );
		m_pZJumpEvent = TNULL;
	}

	m_uiFlags &= ~FLAGS_ZJUMP_FLIGHT;
	AMusicManager::GetSingleton()->RemoveMusicSFXSyncSounds( "shaker_loop" );
	AMusicManager::GetSingleton()->StopPLoop( "shaker_loop" );

	if ( m_pHurtLoopA )
	{
		m_pHurtLoopA->stop( false );
		m_pHurtLoopA = TNULL;
	}

	if ( m_pHurtLoopB )
	{
		m_pHurtLoopB->stop( false );
		m_pHurtLoopB = TNULL;
	}
}

// $deBlob: FUNCTION 0075e2f0
void ABlobSoundComponent::PlayShieldHit()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_LEVELS_MULTIPLAYER, ASoundMap::EVENTID( 231 ), m_pBlob->GetBodyPosition(), FMOD_EVENT_DEFAULT );
}

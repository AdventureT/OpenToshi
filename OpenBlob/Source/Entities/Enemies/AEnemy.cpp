#include "pch.h"
#include "AEnemy.h"
#include "Console/AStaticCVars.h"
#include <TParticleSystem/TParticleEffectInstance.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleSystem.h>
#include "Entities/ASquad/ASquad.h"
#include "Entities/AEntityComponent2.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityModelComponent.h"
#include "Animation/AModelInstance2.h"
#include <Render/TModel.h>
#include "Collision/ACollisionObject.h"
#include "AEnemyManager.h"
#include "AEnemyBehaviour.h"
#include "AEnemySchedule.h"
#include "Entities/AGndMotorComponent.h"
#include "Entities/AGndMotorManager.h"
#include "Entities/PathFinding/APathfindingComponent.h"
#include "Entities/PathFinding/AAIMoveProbe.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Render/Decal/ADecalManager.h"
#include "ARandom.h"
#include "Entities/APickup.h"
#include "Entities/APickupData.h"
#include "Entities/APickupSolid.h"
#include "Console/AConsoleVar.h"
#include "Entities/AAttackMessage.h"
#include "Fx/Shadows/AShadow.h"
#include <Physics/Havok/TPhysicsManager.h>
#include "Challenges/AChallengeManager.h"
#include "Entities/AEnemyWrapper/AEnemyWrapper.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The size of the ink splats of SpawnDecal
static TCONSTEXPR_VAR TFLOAT s_fInkDecalSize = 5.0f;

// NOTE: the motor (AGndMotorComponent, unported) flags at 0x25C: bit 0x200 is set while the enemy is held (EndAtomState)
static TUINT32& GetMotorFlags25C( void* a_pMotor )
{
	return *TREINTERPRETCAST( TUINT32*, TREINTERPRETCAST( TUINT8*, a_pMotor ) + 0x25C );
}

// $deBlob: FUNCTION 007a6830
// $deBlob: FUNCTION 00541160
// $deBlob: FUNCTION 00859ee0
TDEFINE_CLASS_NORUNTIME( AEnemy );

// $deBlob: FUNCTION 007a6880
AEnemy::AEnemy()
{
	m_uiPickupType              = 0xFF;
	m_uiMusicMode               = 0;
	m_fFallTime                 = 0.0f;
	m_vSpawnPosition            = TVector4::VEC_ZERO;
	m_iNumPickups               = 1;
	m_uiEnemyFlags              = 0;
	m_pSquad                    = TNULL;
	m_fClosestEnemyDist         = TMath::MAXFLOAT;
	m_fActiveTime               = 0.0f;
	m_pWrapper                  = TNULL;
	m_pEnemyLiquidRayCastHelper = TNULL;
	m_fAmbientScale             = 1.0f;

	// NOTE: 0x20 and 0x8 aren't named in ABaseEntity::FLAG yet
	m_Flags |= FLAG_Pooled | 0x20 | 0x8;
	SetTicking( TTRUE );
	m_Flags |= FLAG_NoComponentDelete;

	CreateShadow( 3.0f, 0.7f );
}

// $deBlob: FUNCTION 007a6980
// $deBlob: FUNCTION 007a8a40
AEnemy::~AEnemy()
{
	if ( m_pEnemyLiquidRayCastHelper )
	{
		TPhysicsManager::GetSingleton()->DestroyPhantom( m_pEnemyLiquidRayCastHelper );
		m_pEnemyLiquidRayCastHelper = TNULL;
	}
}

// $deBlob: FUNCTION 007a6a00
void AEnemy::PostCreate()
{
	TSTATICCAST( AEnemyBehaviour, GetBehaviourComponent() )->CacheComponents();

	SetBBox( *GetEnemyData()->m_pBBox );
}

// $deBlob: FUNCTION 007a6a40
void AEnemy::SetActive( TBOOL a_bActive, TBOOL a_bUnk )
{
	if ( !a_bActive )
		m_uiEnemyFlags &= ~EFLAG_Active;
	else
		m_uiEnemyFlags |= EFLAG_Active;

	SetHash( a_bActive );
	SetVisible( a_bActive );

	if ( a_bActive == TTRUE )
	{
		if ( TUINT8( m_Flags >> 24 ) & 1 )
		{
			m_Flags &= ~0x4000;
			SetTicking( TTRUE );
			m_Flags &= ~0x1000000;
		}

		InitIdleState();
		InitialiseComponents();

		TINT16 iMaxLife = TINT16( GetMaxLife() );
		m_Flags &= ~FLAG_Dead;
		m_iLife                  = iMaxLife;
		m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & ~0xFF ) | 2 );
		m_pCollisionObject->SetGroupMask( 0xFB );

		SetAlpha( 1.0f );
		UpdateMusicMode( GetLOD(), TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->GetSelectedEnemy() != TNULL );

		if ( m_pSquad != TNULL )
			TSTATICCAST( ASquad, m_pSquad )->AddMember( this );

		if ( GetMotorComponent() != TNULL )
			TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetQueryHelpersActive( TTRUE );

		if ( GetPathfinding() != TNULL && TSTATICCAST( APathfindingComponent, GetPathfinding() )->GetMoveProbe() != TNULL )
			TSTATICCAST( APathfindingComponent, GetPathfinding() )->GetMoveProbe()->SetPhantomActive( TTRUE );
	}
	else
	{
		ForceEndStatesForTask();

		if ( GetFXBreakpointComponent() != TNULL )
			TSTATICCAST( AEntityComponent2, GetFXBreakpointComponent() )->ResetComponent();

		UpdateMusicMode( GetLOD(), TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->GetSelectedEnemy() != TNULL );

		if ( m_pSquad != TNULL )
			TSTATICCAST( ASquad, m_pSquad )->RemoveMember( this );

		if ( GetMotorComponent() != TNULL )
			TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetQueryHelpersActive( TFALSE );

		if ( GetPathfinding() != TNULL && TSTATICCAST( APathfindingComponent, GetPathfinding() )->GetMoveProbe() != TNULL )
			TSTATICCAST( APathfindingComponent, GetPathfinding() )->GetMoveProbe()->SetPhantomActive( TFALSE );

		RemoveFromPhysics();
	}
}

// $deBlob: FUNCTION 007a6c20
void AEnemy::ReleaseToWrapper()
{
	if ( m_pSquad )
		TSTATICCAST( ASquad, m_pSquad )->RemoveMember( this );

	m_pSquad = TNULL;

	ABaseEntity* pWrapper = TREINTERPRETCAST( ABaseEntity*, GetPoolOwner() );

	if ( pWrapper )
	{
		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_ENEMYRELEASED;
		oPacket.m_iData    = 0;
		pWrapper->HandleEntityIO( this, oPacket );
		return;
	}

	AEnemyManager::GetSingleton()->ReleaseEnemy( this );
}

// $deBlob: FUNCTION 007a6c90
void AEnemy::Reset( ABaseEntity* a_pEntity )
{
	m_uiEnemyFlags &= ~EFLAG_Unk100;
	AVisibleEntity::Reset( a_pEntity );
	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 007a6cc0
void* AEnemy::CreateComponent( ComponentType a_eType )
{

	switch ( a_eType )
	{
		case ComponentType_Pose:
			CreatePoseController();
			return GetPoseController();
		case ComponentType_Model:
			CreateModelComponent( TNULL );

			if ( GetModelComponent() != TNULL && TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance() != TNULL &&
			     TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance() != TNULL )
			{
				TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance()->GetTransform().SetMatrix( TMatrix44::IDENTITY );
				return GetModelComponent();
			}
			else
			{
				// NOTE: retail formats the message and drops it
				TString8 strError;
				strError.Format( "Failed to load Enemy Model [%s]\n", GetModelName() );
			}

			break;
		case ComponentType_FXBreakpoint:
			CreateFXBreakpointComponent( TNULL );
			return GetFXBreakpointComponent();
		case ComponentType_State:
			CreateStateComponent();
			return GetStateComponent();
		case ComponentType_Motor:
			CreateMotorComponent();
			return m_pMotorComponent;
		case ComponentType_Pathfinding:
		{
			CreatePathfindingComponent();

			APathfindingComponent* pPathfinding = TSTATICCAST( APathfindingComponent, GetPathfinding() );
			pPathfinding->SetFlags( APathfindingComponent::FLAG_Walk );
			pPathfinding->SetFlags( APathfindingComponent::FLAG_Network );

			TFLOAT fSpeed = GetRunSpeed();
			TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( fSpeed );
			TSTATICCAST( APathfindingComponent, GetPathfinding() )->SetUnk30( 0 );

			if ( TUINT8( m_Flags >> 8 ) & 1 )
			{
				AGndMotorManager::GetSingleton()->RegisterMotorComponent( TSTATICCAST( AGndMotorComponent, GetMotorComponent() ) );
			}

			return GetPathfinding();
		}
		case ComponentType_Behaviour:
			CreateBehaviourComponent();
			return GetBehaviourComponent();
		case ComponentType_Schedule:
			CreateScheduleComponent();
			return GetScheduleComponent();
	}

	return TNULL;
}

// $deBlob: FUNCTION 007a6f40
void* AEnemy::GetComponent( ComponentType a_eType )
{
	switch ( a_eType )
	{
		case ComponentType_Pose:
			return GetPoseController();
		case ComponentType_Model:
			return GetModelComponent();
		case ComponentType_FXBreakpoint:
			return GetFXBreakpointComponent();
		case ComponentType_State:
			return GetStateComponent();
		case ComponentType_Motor:
			return GetMotorComponent();
		case ComponentType_Pathfinding:
			return GetPathfinding();
		case ComponentType_Behaviour:
			return GetBehaviourComponent();
		case ComponentType_Schedule:
			return GetScheduleComponent();
		default:
			return TNULL;
	}
}

// $deBlob: FUNCTION 007a6fd0
void AEnemy::Tick( TFLOAT a_fDeltaTime )
{
	if ( !IsActive() )
		return;


	UpdateActiveTime( a_fDeltaTime );

	TUINT32 uiLOD = GetLOD();

	if ( uiLOD == FLAG_LowLOD )
	{
		UpdateLowLOD( a_fDeltaTime );
		return;
	}

	if ( uiLOD == FLAG_MediumLOD )
		UpdateMediumLOD( a_fDeltaTime );

	if ( TUINT8( m_uiEnemyFlags >> 8 ) & 1 )
	{
		ABaseEntity*    pWrapper    = TREINTERPRETCAST( ABaseEntity*, GetPoolOwner() );
		const TVector4& rWrapperPos = pWrapper->GetMatrix().GetTranslation();
		const TVector4& rPosition   = m_oMatrix.GetTranslation();

		if ( rWrapperPos.y < rPosition.y )
		{
			TVector4 vPosition( rPosition.x, rWrapperPos.y - 0.5f, rPosition.z, rPosition.w );
			SetPosition( vPosition );
		}

		if ( TVector4::DistanceSq( m_oMatrix.GetTranslation(), rWrapperPos ) < 300.0f )
		{
			m_uiEnemyFlags &= ~EFLAG_Unk100;
			GetMotorFlags25C( GetMotorComponent() ) &= ~0x200;
		}
	}

	ACharEntity::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 007a70f0
void AEnemy::UpdateMovement( TFLOAT a_fDeltaTime )
{
	if ( TSTATICCAST( AEntityComponent2, GetMotorComponent() )->IsEnabled() )
	{
		UpdateFacing( a_fDeltaTime );
		UpdateFallTime( a_fDeltaTime );
	}
}

// $deBlob: FUNCTION 007a7140
void AEnemy::OnLODChanging( TINT a_iLOD )
{
	if ( GetLOD() != TUINT32( a_iLOD ) )
	{
		if ( a_iLOD == FLAG_LowLOD )
		{
			TSTATICCAST( AEnemyBehaviour, GetBehaviourComponent() )->SetSelectedEnemy( TNULL );
		}
		else if ( a_iLOD == FLAG_MediumLOD || a_iLOD == FLAG_HighLOD )
		{
			AEnemyManager::GetSingleton()->OnLODChanging( this );
		}

		UpdateMusicMode( a_iLOD, TSTATICCAST( AEnemyBehaviour, GetBehaviourComponent() )->GetSelectedEnemy() != TNULL );
	}

	ACharEntity::OnLODChanging( a_iLOD );
}

// $deBlob: FUNCTION 007a7200
TBOOL AEnemy::UpdateMusicMode( TINT a_iLOD, TBOOL a_bHasEnemy )
{
	if ( !IsActive() || a_iLOD == FLAG_LowLOD )
	{
		m_uiMusicMode = 2;
	}
	else if ( a_iLOD == FLAG_MediumLOD )
	{
		TINT iMotorState = TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->GetState();
		m_uiMusicMode    = ( a_bHasEnemy || iMotorState != 1 ) ? 1 : 2;
	}
	else if ( a_iLOD == FLAG_HighLOD )
	{
		m_uiMusicMode = 0;
	}

	AEnemyManager::GetSingleton()->UpdateEnemyMusicMode( this, m_uiMusicMode );

	return m_uiMusicMode;
}

// $deBlob: FUNCTION 007a7290
void AEnemy::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( TUINT8( m_Flags >> 22 ) & 1 )
		return;


	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_PLAYERATTACK:
		{
			if ( IsInvulnerable() )
				return;

			// The attack's direction, from its position when it has none
			AAttackMessage* pAttack = TREINTERPRETCAST( AAttackMessage*, a_rPacket.m_iData );

			if ( !pAttack->m_vDirection.IsNormalised() )
			{
				pAttack->m_vDirection = TVector4( m_oMatrix.m_f41 - pAttack->m_vPosition.x, m_oMatrix.m_f42 - pAttack->m_vPosition.y, m_oMatrix.m_f43 - pAttack->m_vPosition.z, m_oMatrix.m_f44 );
				pAttack->m_vDirection.NormaliseXZ();
			}
		}
		break;
		case AEntityIOPacket::MESSAGE_PROPATTACK:
		{
			if ( IsInvulnerable() )
				return;

			AAttackMessage* pAttack = TREINTERPRETCAST( AAttackMessage*, a_rPacket.m_iData );

			if ( !pAttack->m_vDirection.IsNormalised() )
			{
				pAttack->m_vDirection = TVector4( m_oMatrix.m_f41 - pAttack->m_vPosition.x, m_oMatrix.m_f42 - pAttack->m_vPosition.y, m_oMatrix.m_f43 - pAttack->m_vPosition.z, m_oMatrix.m_f44 );
				pAttack->m_vDirection.NormaliseXZ();
			}

			TakeDamage( TINT16( AStaticCVars::cvEnemyTakeDamageMultiplier.GetInt() * TINT( pAttack->m_uiDamage ) ) );
		}
		break;
		case AEntityIOPacket::MESSAGE_DEATHZONE:
		{
			m_iLife = 0;
			m_Flags |= FLAG_Dead;
		}
		break;
	}

	if ( TUINT8( m_Flags >> 22 ) & 1 )
		OnDeath( a_rPacket, a_pCaller );

	ACharEntity::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007a7420
TBOOL AEnemy::HandleFallenThroughWorld()
{
	ReleaseToWrapper();
	return TTRUE;
}

// $deBlob: FUNCTION 007a7490
void AEnemy::TakeDamage( TINT16 a_iDamage )
{
	m_iLife -= a_iDamage;

	if ( m_iLife <= 0 && GetMaxLife() > 0 )
	{
		m_Flags |= FLAG_Dead;
		return;
	}

	m_Flags &= ~FLAG_Dead;
}

// $deBlob: FUNCTION 007a74d0
void AEnemy::OnDeath( const AEntityIOPacket& a_rPacket, ABaseEntity* a_pKiller )
{
	m_Flags |= FLAG_Dead;
	m_iLife = 0;

	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 && GetKillScore() != -1 )
	{
		if ( ( !IsA( &TGetClass( ALeechBot2 ) ) || GetEnemyType() == 2 ) && a_pKiller != TNULL && a_pKiller->IsA( &TGetClass( ABlobEntity ) ) )
		{
			TINT                    iPlayer    = APlayerManager::GetSingleton()->GetPlayerIndex( a_pKiller );
			AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();

			if ( rLevelData.GetComboState( iPlayer ) == 2 )
				rLevelData.IncrementComboCount( iPlayer );
			else
				rLevelData.StartCombo( iPlayer, 2 );

			// The kill's score pops up above the enemy (+Y is down)
			const TVector4& rcPosition = GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
			TVector4        vPosition( rcPosition.x, rcPosition.y - GetBBox().y * 2.0f, rcPosition.z, rcPosition.w );
			rLevelData.AddKillScore( iPlayer, vPosition, GetKillScore() );
		}
	}

	m_iCollisionGroupAndType = 4;
	GetCollisionObject()->SetGroupMask( 8 );

	if ( a_rPacket.m_iData != 0 )
	{
		TASSERT( IsA( &TGetClass( ALeechBot2 ) ) );
		return;
	}

	ABaseEntity* pSquad   = m_pSquad;
	ABaseEntity* pWrapper = TREINTERPRETCAST( ABaseEntity*, GetPoolOwner() );
	ABaseEntity* pTrigger = TNULL;

	if ( pWrapper != TNULL && pWrapper->IsA( &TGetClass( AEnemyWrapper ) ) )
		pTrigger = TSTATICCAST( AEnemyWrapper, pWrapper )->GetOwnerTrigger();

	ReleaseToWrapper();

	// The combat challenge hears about the kill (the enemy group trigger's ID, else the wrapper's)
	TINT16 iID;

	if ( pTrigger != TNULL )
	{
		pTrigger->Tick( 0.0f );
		TASSERT( !pTrigger->IsPooled() );
		iID = TINT16( pTrigger->GetID() );
		AChallengeManager::GetSingleton()->OnCombatEvent( TREINTERPRETCAST( void*, TUINTPTR( TINT( iID ) ) ) );
	}
	else if ( pWrapper != TNULL )
	{
		iID = pWrapper->GetHashID();
		AChallengeManager::GetSingleton()->OnCombatEvent( TREINTERPRETCAST( void*, TUINTPTR( TINT( iID ) ) ) );
	}

	if ( pSquad != TNULL )
		TSTATICCAST( ASquad, pSquad )->OnMemberKilled();
}

// A random drop velocity: X and Z between a_fHorizontal / a_iDivB and a_fHorizontal / a_iDivA (each with a random sign),
// Y between -a_fVertical and -a_fVertical / a_iDivA (up)
// $deBlob: FUNCTION 007e4d80
static TVector4& GetRandomDropVelocity( TVector4& a_rOutVelocity, TFLOAT a_fHorizontal, TFLOAT a_fVertical, TINT a_iDivA, TINT a_iDivB )
{
	TRandom& rRandom = ARandom::GetSingleton()->GetRandom();
	a_rOutVelocity   = TVector4::VEC_ZERO;

	TFLOAT fNear = a_fHorizontal / TFLOAT( a_iDivB );
	TFLOAT fFar  = a_fHorizontal / TFLOAT( a_iDivA );

	if ( rRandom.GetInt( 2 ) != 0 )
		a_rOutVelocity.x = rRandom.GetFloatMinMax( fNear, fFar );
	else
		a_rOutVelocity.x = rRandom.GetFloatMinMax( -fFar, -fNear );

	a_rOutVelocity.y = rRandom.GetFloatMinMax( -a_fVertical, -( a_fVertical / TFLOAT( a_iDivA ) ) );

	if ( rRandom.GetInt( 2 ) != 0 )
		a_rOutVelocity.z = rRandom.GetFloatMinMax( fNear, fFar );
	else
		a_rOutVelocity.z = rRandom.GetFloatMinMax( -fFar, -fNear );

	return a_rOutVelocity;
}

// $deBlob: FUNCTION 007a7710
// NOTE: APickupData::CreatePickup is empty on PC (returns TNULL), so enemies never drop pickups
void AEnemy::SpawnPickups()
{
	for ( TINT i = 0; i < m_iNumPickups; i++ )
	{
		TVector4 vVelocity;
		GetRandomDropVelocity( vVelocity, 4.0f, 10.0f, 2, 4 );

		APickup* pPickup = APickupData::CreatePickup( m_uiPickupType );

		if ( pPickup != TNULL )
		{
			pPickup->SetPosition( GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) );
			pPickup->SetMatrix( GetMatrix() );
			pPickup->Drop( this, vVelocity );
			pPickup->SetVisible( TTRUE );

			if ( pPickup->IsA( &TGetClass( APickupSolid ) ) )
				pPickup->CastTo<APickupSolid>()->StartDying();
		}
	}
}

// $deBlob: FUNCTION 007a7820
void AEnemy::SetBBox( TVector4 a_vBBox )
{
	ABaseEntity::SetBBox( a_vBBox );
	GetCollisionObject()->SetFlags( ACollisionObject::FLAG( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty ) );
}

// $deBlob: FUNCTION 007a7850
void AEnemy::SetVisible( TBOOL a_bVisible )
{
	AVisibleEntity::SetVisible( a_bVisible );
}

// $deBlob: FUNCTION 007a7870
TBOOL AEnemy::IsRecoverState( TINT a_iState )
{
	return TUINT32( a_iState - COMMONSTATE_Recover ) <= 2 ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 007a7890
TBOOL AEnemy::ShouldIgnoreBlob( void* a_pEntity )
{
	// Ignores a blob that is dead, in state 8, flagged 0x80000 or with its 0x5A4 timer running
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pEntity );

	if ( pBlob->GetBlobState() != ABlobEntity::STATE_DEAD && pBlob->GetBlobState() != ABlobEntity::STATE_GRASSBOMB &&
	     !( pBlob->GetBlobFlags() & 0x80000 ) && pBlob->GetUntargetableTime() <= 0.0f )
		return TFALSE;

	return TTRUE;
}

// $deBlob: FUNCTION 007a78d0
TFLOAT AEnemy::GetFleeRange()
{
	return AEnemyManager::GetSingleton()->GetEnemiesData()->m_fFleeRange;
}

// $deBlob: FUNCTION 007a78e0
void AEnemy::EndAtomState()
{
	void* pMotor = GetMotorComponent();

	if ( pMotor )
	{
		m_uiEnemyFlags |= EFLAG_Unk100;
		GetMotorFlags25C( pMotor ) |= 0x200;
	}
}

// $deBlob: FUNCTION 007a7910
TBOOL AEnemy::IsInSpawnRange( const TVector4& a_rPosition, TBOOL a_bReduced )
{
	TFLOAT fRange = GetSpawnRange();

	if ( a_bReduced )
		fRange *= 0.8f;

	if ( 0.0f < fRange &&
	     fRange * fRange < ( m_vSpawnPosition.x - a_rPosition.x ) * ( m_vSpawnPosition.x - a_rPosition.x ) + ( m_vSpawnPosition.z - a_rPosition.z ) * ( m_vSpawnPosition.z - a_rPosition.z ) )
		return TFALSE;

	return TTRUE;
}

// $deBlob: FUNCTION 007a7990
void AEnemy::Despawn( TBOOL a_bPlayEffect )
{
	AVisibleEntity::Despawn( a_bPlayEffect );
	ReleaseToWrapper();
}

// $deBlob: FUNCTION 007a7a00
void AEnemy::CreateBehaviourComponent()
{
	m_pBehaviourComponent = new AEnemyBehaviour( this );
}

// $deBlob: FUNCTION 007a7a80
void AEnemy::CreateScheduleComponent()
{
	m_pScheduleComponent = new AEnemySchedule( this );
}

// $deBlob: FUNCTION 007a7b00
void AEnemy::UpdateLowLOD( TFLOAT a_fDeltaTime )
{
	if ( TUINT8( m_Flags >> 22 ) & 1 )
		ReleaseToWrapper();
}

// $deBlob: FUNCTION 007a7b20
void AEnemy::UpdateMediumLOD( TFLOAT a_fDeltaTime )
{
	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->SetTransform( m_oMatrix );

	if ( TUINT8( m_Flags >> 22 ) & 1 )
		ReleaseToWrapper();
}

// $deBlob: FUNCTION 007a7b50
void AEnemy::UpdateFacing( TFLOAT a_fDeltaTime )
{
	TVector4 vFacing = GetDesiredFacing();
	vFacing.NormaliseXZ();
	TurnToFace( a_fDeltaTime, vFacing, *GetTurnSettings() );
	Unk218( a_fDeltaTime );
}

// $deBlob: FUNCTION 007a7bb0
void AEnemy::UpdateFallTime( TFLOAT a_fDeltaTime )
{
	// Falling for more than 0.35 seconds: the fall schedule
	TFLOAT fVelocityY = TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->GetVelocity().y;

	if ( TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->GetState() == 2 || TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->GetState() == 1 || fVelocityY < 2.5f )
	{
		m_fFallTime = 0.0f;
		return;
	}

	m_fFallTime += a_fDeltaTime;

	if ( m_fFallTime <= 0.35f )
		return;

	if ( CanPlayFallSchedule() )
		TSTATICCAST( AAISchedule, GetScheduleComponent() )->StartSchedule( 0x16 );
}

// $deBlob: FUNCTION 007a7c60
void AEnemy::UpdateActiveTime( TFLOAT a_fDeltaTime )
{
	TASSERT( TTRUE == IsActive() );

	if ( GetLOD() == FLAG_HighLOD )
	{
		if ( TUINT8( m_uiEnemyFlags >> 7 ) & 1 )
		{
			m_fActiveTime = 0.0f;
			m_uiEnemyFlags &= ~EFLAG_InactiveTimedOut;
		}
	}
	else if ( !( TUINT8( m_uiEnemyFlags >> 7 ) & 1 ) )
	{
		// Out of the high LOD for too long
		m_fActiveTime += a_fDeltaTime;

		if ( AEnemyManager::GetSingleton()->GetEnemiesData()->m_fMaxInactiveTime <= m_fActiveTime )
		{
			OnInactiveTimeOut();
			m_fActiveTime = 0.0f;
		}
	}
}

// $deBlob: FUNCTION 007a7d80
void AEnemy::OnEnterState( TINT a_iState )
{
	TFLOAT fSpeed;

	if ( a_iState == COMMONSTATE_Walk )
	{
		fSpeed = GetWalkSpeed();
	}
	else if ( a_iState == COMMONSTATE_Run )
	{
		fSpeed = GetRunSpeed();
	}
	else
	{
		if ( a_iState != COMMONSTATE_Fall )
			return;

		// In liquid: its ray cast helper
		if ( m_pEnemyLiquidRayCastHelper == TNULL )
		{
			m_pEnemyLiquidRayCastHelper = TPhysicsManager::GetSingleton()->CreateAabbPhantom();
			TPhysicsManager::SetCollisionTestFlags( m_pEnemyLiquidRayCastHelper, 0x20 );
		}

		TASSERT( m_pEnemyLiquidRayCastHelper, "phantomContainer" );
		TASSERT( !m_pEnemyLiquidRayCastHelper->IsInSimulation(), "!pPM->IsQueryHelperInSimulation(m_pEnemyLiquidRayCastHelper)" );
		TPhysicsManager::GetSingleton()->AddPhantomToSimulation( m_pEnemyLiquidRayCastHelper, m_oMatrix.GetTranslation(), TNULL );
		return;
	}

	TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( fSpeed );
}

// $deBlob: FUNCTION 007a7ea0
void AEnemy::OnLeaveState( TINT a_iState )
{
	if ( a_iState == COMMONSTATE_Walk )
	{
		TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( GetRunSpeed() );
	}
	else if ( a_iState == COMMONSTATE_Fall )
	{
		TASSERT( m_pEnemyLiquidRayCastHelper, "phantomContainer" );
		TASSERT( m_pEnemyLiquidRayCastHelper->IsInSimulation(), "pPM->IsQueryHelperInSimulation(m_pEnemyLiquidRayCastHelper)" );
		TPhysicsManager::GetSingleton()->RemovePhantomFromSimulation( m_pEnemyLiquidRayCastHelper, TNULL );
	}
}

// $deBlob: FUNCTION 007a7f90
void AEnemy::StartCurrentStateTask()
{
	switch ( m_iCurrentTask )
	{
		case 0xB:
			Unk1F8();
			break;
		case 0x1B:
			Unk208();
			break;
		case 0x1E:
			InitAngryState();
			break;
		case 0x20:
			InitMoveState();
			break;
		case 0x21:
			Unk210();
			break;
		case 0x23:
			InitFallState();
			break;
	}

	ACharEntity::StartCurrentStateTask();

	if ( m_iCurrentTask == 0 )
	{
		TFLOAT fTaskTime = GetEnemyData()->m_fTaskTime;
		TSTATICCAST( AAISchedule, GetScheduleComponent() )->OverrideTaskTime( fTaskTime );
	}
}

// $deBlob: FUNCTION 007a8060
TBOOL AEnemy::UpdateCurrentStateTask( TFLOAT a_fDeltaTime )
{
	switch ( m_iCurrentTask )
	{
		case 0xB:
			return Unk1FC( a_fDeltaTime );
		case 0x1B:
			return Unk20C( a_fDeltaTime );
		case 0x1E:
			return UpdateAngryState( a_fDeltaTime );
		case 0x20:
			return UpdateMoveState( a_fDeltaTime );
		case 0x21:
			return Unk214( a_fDeltaTime );
		case 0x22:
			return TFALSE;
		case 0x23:
			return UpdateFallState( a_fDeltaTime );
		default:
			return ACharEntity::UpdateCurrentStateTask( a_fDeltaTime );
	}
}

// $deBlob: FUNCTION 007a8170
TBOOL AEnemy::CanPlayFallSchedule()
{
	return TSTATICCAST( AAISchedule, GetScheduleComponent() )->GetActiveSchedule() != 0x16;
}

// $deBlob: FUNCTION 007a8190
void AEnemy::SetAlpha( TFLOAT a_fUnk )
{
	// The fade of the model (the shadow is hidden at 0)
	m_pShadow->SetHidden( a_fUnk == 0.0f );

	TModelInstance* pInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance();
	pInstance->SetForceFade( TTRUE );
	pInstance->SetFade( a_fUnk );
}

// $deBlob: FUNCTION 007a8220
void AEnemy::SetTimeRatio( TFLOAT a_fCurrentTime, TFLOAT a_fMaxTime )
{
	TASSERT( a_fCurrentTime >= 0 );
	TASSERT( a_fMaxTime >= 0 );

	if ( 0.0f < a_fMaxTime )
	{
		TFLOAT fRatio = ( a_fCurrentTime < a_fMaxTime ) ? a_fCurrentTime / a_fMaxTime : 1.0f;
		SetAlpha( 1.0f - fRatio );
	}
}

// $deBlob: FUNCTION 007a82e0
TBOOL AEnemy::UpdateKnock( TFLOAT a_fDeltaTime )
{
	TINT iState = m_iState;

	if ( iState >= 0xA )
	{
		if ( iState > 0xB )
		{
			if ( iState == 0xC )
				return HasStateFinished();
		}
		else if ( HasStateFinished() )
		{
			SetState( m_iState + 1 );
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007a8320
void AEnemy::InitIdleState()
{
	// NOTE: a value of the behaviour (AEnemyBehaviour, unported) at 0x2C
	if ( GetBehaviourComponent() && *TREINTERPRETCAST( TINT*, TREINTERPRETCAST( TUINT8*, GetBehaviourComponent() ) + 0x2C ) != 0 )
	{
		SetState( COMMONSTATE_IdleMoving );
		return;
	}

	SetState( COMMONSTATE_Idle );
}

// $deBlob: FUNCTION 007a8360
TBOOL AEnemy::UpdateIdleState( TFLOAT a_fDeltaTime )
{
	TBOOL bResult = TFALSE;

	if ( 0 != m_iState )
		bResult = TTRUE;

	TSTATICCAST( AEnemyBehaviour, GetBehaviourComponent() )->FaceEnemyPredictedPosition( 0.5f, TFALSE, TNULL );

	return bResult;
}

// $deBlob: FUNCTION 007a83e0
TBOOL AEnemy::UpdateFidgetState( TFLOAT a_fDeltaTime )
{
	TBOOL bResult = TFALSE;

	switch ( m_iState )
	{
		case COMMONSTATE_Fidget:
			if ( HasPoseFinished() )
				InitIdleState();
			break;
		default:
			bResult = TTRUE;
			break;
	}

	return bResult;
}

// $deBlob: FUNCTION 007a8410
void AEnemy::InitMoveState()
{
	// Walks (3) while patrolling or without an enemy, else runs (4)
	TINT eSchedule = TSTATICCAST( AAISchedule, GetScheduleComponent() )->GetActiveSchedule();

	if ( eSchedule != 0x17 )
	{
		AAIBehaviour* pBehaviour = TSTATICCAST( AAIBehaviour, GetBehaviourComponent() );

		if ( pBehaviour->GetSelectedEnemy() == TNULL || ( eSchedule == 3 && !TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->HasPathFailed() ) )
		{
			SetState( COMMONSTATE_Walk );
			return;
		}
	}

	SetState( COMMONSTATE_Run );
}

// $deBlob: FUNCTION 007a8470
void AEnemy::InitDeathState()
{
	ReleaseToWrapper();
}

// $deBlob: FUNCTION 007a84e0
void AEnemy::InitAngryState()
{
	SetState( COMMONSTATE_Angry );

	TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->SetTaunted();
}

// $deBlob: FUNCTION 007a8500
TBOOL AEnemy::UpdateAngryState( TFLOAT a_fDeltaTime )
{
	TASSERT( COMMONSTATE_Angry == GetState() );
	return HasStateFinished();
}

// $deBlob: FUNCTION 007a8560
TBOOL AEnemy::UpdateFallState( TFLOAT a_fDeltaTime )
{
	if ( m_iState == COMMONSTATE_Fall )
	{
		// Falling: landed (state 6), or a ray along the last move finds liquid (killed, with a splash)
		AGndMotorComponent* pMotor = TSTATICCAST( AGndMotorComponent, GetMotorComponent() );

		if ( pMotor->IsOnGround( 0.15f ) )
		{
			SetState( COMMONSTATE_Land );
			return TFALSE;
		}

		if ( !( ( TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->GetFlags() >> 9 ) & 1 ) )
		{
			TPhysicsRay oRay;
			oRay.m_vFrom = m_oMatrix.GetTranslation();
			oRay.m_vTo   = m_oMatrix.GetTranslation();

			TVector4 vMove;
			vMove.Multiply( TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->GetVelocity(), a_fDeltaTime );
			oRay.m_vFrom.x -= vMove.x;
			oRay.m_vFrom.z -= vMove.z;
			oRay.m_vFrom.y          = ( oRay.m_vFrom.y - vMove.y ) - m_vBBox.y;
			oRay.m_uiCollisionLayer = 0x20;

			// The phantom is the box around the ray
			TVector4 vSum( oRay.m_vTo.x + oRay.m_vFrom.x, oRay.m_vTo.y + oRay.m_vFrom.y, oRay.m_vTo.z + oRay.m_vFrom.z, oRay.m_vTo.w );
			TVector4 avBox[ 2 ];
			avBox[ 0 ].Multiply( vSum, 0.5f );

			TVector4 vDiff( oRay.m_vFrom.x - oRay.m_vTo.x, oRay.m_vFrom.y - oRay.m_vTo.y, oRay.m_vFrom.z - oRay.m_vTo.z, oRay.m_vFrom.w );
			avBox[ 1 ].Multiply( vDiff, 0.5f );
			avBox[ 1 ].Abs4();

			TPhysicsManager* pPM = TPhysicsManager::GetSingleton();
			pPM->SetAabbPhantomBox( m_pEnemyLiquidRayCastHelper, avBox );
			TASSERT( pPM->GetCollisionTestFlags( m_pEnemyLiquidRayCastHelper ) == oRay.m_uiCollisionLayer );

			TPhysicsShapeContact oContact;

			if ( pPM->CollideRay( oRay, oContact, m_pEnemyLiquidRayCastHelper ) )
			{
				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_DEATHZONE;
				oPacket.m_iData    = 0;
				HandleEntityIO( TNULL, oPacket );

				TParticleEffectInstance* pSplash = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ABlobEntity::ms_pWaterSplashEffect );

				if ( pSplash != TNULL )
					pSplash->GetTransform().SetTranslation( oContact.m_vPosition );
			}
		}

		return TFALSE;
	}

	if ( m_iState != COMMONSTATE_Land )
		return TTRUE;

	return HasStateFinished();
}

// $deBlob: FUNCTION 007a8830
void AEnemy::InitAlarmState()
{
	TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->ClearSelectedEnemyNew();
	SetState( COMMONSTATE_Alarm );

	if ( m_pSquad != TNULL )
		TSTATICCAST( ASquad, m_pSquad )->TellTargets( TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->GetSelectedEnemy() );
}

// $deBlob: FUNCTION 007a8870
TBOOL AEnemy::UpdateAlarmState( TFLOAT a_fDeltaTime )
{
	TASSERT( COMMONSTATE_Alarm == GetState() );
	return HasStateFinished();
}

// $deBlob: FUNCTION 007a88c0
void AEnemy::SpawnDecal()
{
	// An ink splat under the enemy, randomly rotated
	TFLOAT fRandom = ARandom::GetSingleton()->GetRandom().GetFloat();
	ADecalManager::GetSingleton()->SetRotation( fRandom * TMath::TWO_PI );

	TVector4 vPos = GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
	vPos.y -= 0.1f;

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	ADecalManager::GetSingleton()->AddDecal( &vPos, s_fInkDecalSize, s_fInkDecalSize, 0xFF000000, pBlob->GetSplatMaterial(), ( fRandom <= 0.5f ) ? ADecalManager::Type_BottomRight : ADecalManager::Type_TopRight );
}

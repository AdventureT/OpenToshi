#include "pch.h"
#include "AEnemyBehaviour.h"
#include "Console/AStaticCVars.h"
#include "Entities/ASquad/ASquad.h"
#include "Entities/AI/Schedule/AAIBaseSchedules.h"
#include "Entities/AI/Schedule/AAISchedule.h"
#include "ARandom.h"
#include "AEnemy.h"
#include "AEnemyManager.h"
#include "AEnemyTarget.h"
#include "AEnemySchedule.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AGndMotorComponent.h"
#include "Entities/PathFinding/AAINavigator.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Collision/ACollisionObject.h"
#include "Console/AConsoleState.h"
#include "GameInterface/APlayerManager.h"
#include "Platform/DX11/AStencilShadow_DX11.h"
#include <Physics/Havok/TPhysicsManager.h>
#include "Entities/PathFinding/APathfindingComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void* AEnemyBehaviour::sm_pLOSCollisionFunctor;
TBOOL AEnemyBehaviour::sm_bPausedForCutscene;

TSTATICASSERT( sizeof( AEnemyBehaviour ) == 0x44 );

// $deBlob: FUNCTION 00541910
// $deBlob: FUNCTION 0085a0d0
static AConsoleVar s_cvEnemyShowSurroundPosition( "enemy_show_surround_position", TFALSE ); // 0x009b8d3c
// $deBlob: FUNCTION 005418f0
// $deBlob: FUNCTION 0085a0c0
static AConsoleVar s_cvEnemyShowEnemyList( "enemy_show_enemy_list", TFALSE ); // 0x009b8d68

// $deBlob: FUNCTION 007cee00
AEnemyBehaviour::AEnemyBehaviour( ABaseEntity* a_pEntity )
    : AAIBehaviour( a_pEntity )
{
	m_pSchedule       = TNULL;
	m_pNavigator      = TNULL;
	m_iSurroundResult = -1;
	m_pTargetChar     = TNULL;
}

// $deBlob: FUNCTION 007d0ae0
// $deBlob: FUNCTION 007cee40
AEnemyBehaviour::~AEnemyBehaviour()
{
}

// $deBlob: FUNCTION 007cee50
void AEnemyBehaviour::InitialiseStatic()
{
	TASSERT( TNULL == sm_pLOSCollisionFunctor );
	sm_pLOSCollisionFunctor = new AEnemyLOSCollisionFunctor();

	AConsoleState::RegisterCVar( &s_cvEnemyShowSurroundPosition );
	AConsoleState::RegisterCVar( &s_cvEnemyShowEnemyList );
}

// $deBlob: FUNCTION 007ceec0
void AEnemyBehaviour::DeinitialiseStatic()
{
	AConsoleState::UnregisterCVar( &s_cvEnemyShowEnemyList );
	AConsoleState::UnregisterCVar( &s_cvEnemyShowSurroundPosition );

	// NOTE: retail frees the functor without its destructor
	::operator delete( sm_pLOSCollisionFunctor );
	sm_pLOSCollisionFunctor = TNULL;
}

// $deBlob: FUNCTION 007cef00
void AEnemyBehaviour::CacheComponents()
{
	m_pSchedule  = GetEnemyOwner()->GetScheduleComponent();
	m_pNavigator = TSTATICCAST( APathfindingComponent, GetEnemyOwner()->GetPathfinding() )->GetNavigator();
}

// $deBlob: FUNCTION 007cefc0
void AEnemyBehaviour::ResetComponent()
{
	AAIBehaviour::ResetComponent();
	m_iAttackType     = -1;
	m_iSurroundResult = -1;
}

// $deBlob: FUNCTION 007cefe0
void AEnemyBehaviour::Tick( TFLOAT a_fDeltaTime )
{
	AAIBehaviour::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 007cf000
void AEnemyBehaviour::UpdateEnemyList( TFLOAT a_fDeltaTime )
{
	if ( sm_bPausedForCutscene )
		return;

	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
	AEnemy* pOwner = TSTATICCAST( AEnemy, m_pEntity );

	TFLOAT fMaxDistance    = 0.0f;
	TFLOAT fMaxFrustration = 0.0f;
	TFLOAT fRange          = pOwner->GetVisualRange();

	// NOTE: retail gets the collision object without using it
	pOwner->GetCollisionObject();

	// NOTE: retail also counts m_Enemies and oUpdated (T2GenericDList::Size) without using the counts
	EnemyList oUpdated;
	m_uiFlags &= ~FLAG_EnemyInSight;

	for ( auto it = m_Enemies.Begin(); it != m_Enemies.End(); )
	{
		Enemy* pNode = it;
		it++;

		TASSERT( pNode->m_pEnemy->IsA( &TGetClass( ABlobEntity ) ) );
		ABaseEntity* pEnemy = pNode->m_pEnemy;
		TBOOL        bKeep  = TFALSE;

		if ( !( pEnemy->GetFlags() & ABaseEntity::FLAG_Dead ) && !pOwner->ShouldIgnoreBlob( pEnemy ) )
		{
			if ( pNode->m_fFrustration > 0.0f )
				pNode->m_fFrustration -= a_fDeltaTime;

			if ( pNode->m_fFrustration < 0.0f )
				pNode->m_fFrustration = 0.0f;

			if ( fMaxFrustration < pNode->m_fFrustration )
				fMaxFrustration = pNode->m_fFrustration;

			TFLOAT fDistance = TVector4::Distance( pEnemy->GetMatrix().GetTranslation(), m_pEntity->GetMatrix().GetTranslation() );

			if ( fRange <= fDistance || ( !( pEnemy->GetFlags() & ABaseEntity::FLAG_Player ) && !CanSeeEnemy( pEnemy ) ) )
			{
				// Out of range or sight: forgotten after a while (unless its squad still fights it)
				pNode->m_fLostTime += a_fDeltaTime;

				if ( GetLOSMemoryTime() < pNode->m_fLostTime )
				{
					// Kept while the squad still has it as a target
					if ( GetEnemyOwner()->GetSquad() != TNULL )
						bKeep = TSTATICCAST( ASquad, GetEnemyOwner()->GetSquad() )->IsTarget( pEnemy );
				}
				else
				{
					bKeep = TTRUE;
				}
			}
			else
			{
				pNode->m_fLostTime   = 0.0f;
				pNode->m_fDistanceSq = fDistance;
				m_uiFlags |= FLAG_EnemyInSight;

				if ( fMaxDistance < fDistance )
					fMaxDistance = fDistance;

				if ( pOwner->GetSquad() != TNULL )
				{
					ASquad* pSquad = TSTATICCAST( ASquad, GetEnemyOwner()->GetSquad() );
					pSquad->AddTarget( pEnemy, GetLOSMemoryTime() );
				}

				bKeep = TTRUE;
			}
		}

		if ( bKeep )
		{
			pNode->Remove();
			oUpdated.Push( pNode );
		}
		else
		{
			RemoveFromEnemyList( pNode );
		}
	}

	TASSERT( m_Enemies.IsEmpty() );

	for ( auto it = oUpdated.Begin(); it != oUpdated.End(); it++ )
		it->m_fThreatRating = CalculateEnemyThreat( it, fMaxDistance, fMaxFrustration );

	while ( !oUpdated.IsEmpty() )
		m_Enemies.Push( oUpdated.PopBack() );

	TASSERT( m_Enemies.IsEmpty() || m_Enemies.Front()->m_fThreatRating >= m_Enemies.Back()->m_fThreatRating );

	if ( m_Enemies.IsEmpty() )
		m_uiFlags &= ~FLAG_HasEnemies;
	else
		m_uiFlags |= FLAG_HasEnemies;

	ShouldSelectNewEnemy( TTRUE );
}

// $deBlob: FUNCTION 007cf640
void AEnemyBehaviour::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_PLAYERATTACK:
		case AEntityIOPacket::MESSAGE_PROPATTACK:
			if ( GetEnemyOwner()->IsInvulnerable() )
				return;
			TFALLTHROUGH;
		case AEntityIOPacket::MESSAGE_DEATHZONE:
			BeenHit( a_pCaller, a_rPacket );
			break;
		case AEntityIOPacket::MESSAGE_ENEMYATTACK:
			if ( !GetEnemyOwner()->IsInvulnerable() && a_pCaller != m_pEntity && *TREINTERPRETCAST( TINT*, a_rPacket.m_iData ) == 0xB )
			{
				BeenHit( a_pCaller, a_rPacket );
				return;
			}
			break;
	}
}

// $deBlob: FUNCTION 007cf6f0
TBOOL AEnemyBehaviour::CanChooseSchedule()
{
	if ( m_fScheduleTimer > 0.0f )
		return TFALSE;

	return !HASANYFLAG( GetEnemyOwner()->GetFlags(), ABaseEntity::FLAG_Dead );
}

// $deBlob: FUNCTION 007cf760
void AEnemyBehaviour::SelectSchedule()
{
	if ( !CanChooseSchedule() )
		return;

	TINT eSchedule;

	if ( m_pSelectedEnemy == TNULL )
	{
		eSchedule = SelectIdleSchedule();
	}
	else
	{
		if ( !GetEnemyOwner()->IsInSpawnRange( m_pSelectedEnemy->GetMatrix().GetTranslation(), TTRUE ) )
		{
			// Out of its area: it taunts the enemy (once FLAG_Taunted is set, it idles)
			if ( !( TUINT8( m_uiFlags >> 3 ) & 1 ) )
			{
				TSTATICCAST( AAISchedule, TSTATICCAST( AEnemy, m_pEntity )->GetScheduleComponent() )->StartSchedule( 0x12 );
				return;
			}

			eSchedule = SelectIdleSchedule();

			if ( eSchedule != -1 )
			{
				TSTATICCAST( AAISchedule, TSTATICCAST( AEnemy, m_pEntity )->GetScheduleComponent() )->StartSchedule( eSchedule );
				return;
			}
		}

		if ( IsSelectedEnemyNew() )
			eSchedule = AAIBaseSchedules::BASE_Alert;
		else if ( GetEnemyOwner()->ShouldFlee( TREINTERPRETCAST( TINT, m_pSelectedEnemy ) ) )
			eSchedule = AAIBaseSchedules::BASE_Flee;
		else
			eSchedule = SelectAttackSchedule();
	}

	TSTATICCAST( AAISchedule, TSTATICCAST( AEnemy, m_pEntity )->GetScheduleComponent() )->StartSchedule( eSchedule );
}

// $deBlob: FUNCTION 007cf860
void AEnemyBehaviour::OnScheduleFinished( TINT a_iSchedule, TINT a_iResult, TINT a_iTask )
{
	if ( a_iSchedule == 3 )
	{
		m_uiFlags &= ~FLAG_PathFailed;
	}
	else if ( a_iSchedule == 5 && a_iResult == 0 && m_pTargetChar != TNULL )
	{
		m_pTargetChar->ReleaseSurroundPoint( GetEnemyOwner() );
	}

	AAIBehaviour::OnScheduleFinished( a_iSchedule, a_iResult, a_iTask );
}

// $deBlob: FUNCTION 007cf8b0
TBOOL AEnemyBehaviour::ShouldSelectNewEnemy( TBOOL a_bSelect )
{
	if ( a_bSelect || m_pSelectedEnemy != TNULL )
	{
		ABaseEntity* pSelected = m_pSelectedEnemy;

		// Keeps the one in its attack range
		if ( pSelected != TNULL && WithinAttackRange( pSelected->GetMatrix().GetTranslation(), pSelected->GetBoundingSphereRadius(), TFALSE, -1 ) )
			return TFALSE;

		ABaseEntity* pEnemy = SelectEnemy();

		if ( pEnemy == m_pSelectedEnemy )
			return TFALSE;

		if ( a_bSelect )
			SetSelectedEnemy( pEnemy );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 007cf910
void AEnemyBehaviour::SetSelectedEnemy( ABaseEntity* a_pEnemy )
{
	// NOTE: m_pTargetChar is kept when the enemy is deselected
	if ( m_pTargetChar != TNULL )
		m_pTargetChar->RemoveTargetter( GetEnemyOwner() );

	if ( a_pEnemy != TNULL )
	{
		m_pTargetChar = AEnemyManager::GetSingleton()->FindTarget( a_pEnemy );
		TASSERT( TNULL != m_pTargetChar, "No targetfound.  Who are you trying to target?" );

		m_pTargetChar->AddTargetter( GetEnemyOwner() );
	}

	if ( IsEnemy( a_pEnemy ) && !IsKnownEnemy( a_pEnemy ) )
		AddToEnemyList( a_pEnemy );

	m_pSelectedEnemy = a_pEnemy;

	AEnemy* pOwner = GetEnemyOwner();
	pOwner->UpdateMusicMode( TINT( m_pEntity->GetLOD() ), a_pEnemy != TNULL );
}

// $deBlob: FUNCTION 007cfaf0
TBOOL AEnemyBehaviour::IsEnemy( ABaseEntity* a_pEntity )
{
	if ( a_pEntity != TNULL && a_pEntity->IsA( &TGetClass( ABaseEntity ) ) )
		return !( a_pEntity->GetFlags() & 0x20 );

	return TFALSE;
}

// $deBlob: FUNCTION 007cfb30
TVector4 AEnemyBehaviour::GetEnemyPredictedPosition( ABaseEntity* a_pEnemy, TFLOAT a_fTime, const TVector4* a_pFrom )
{
	if ( a_pEnemy == TNULL )
	{
		a_pEnemy = m_pSelectedEnemy;

		if ( a_pEnemy == TNULL )
		{
			// Without an enemy: ahead of the owner, at its attack range
			AEnemy::EnemyData* pData = TSTATICCAST( AEnemy::EnemyData, m_pEntity->GetTypeData()->m_pTRBData );
			TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );

			TVector4 vAhead;
			vAhead.Multiply( m_pEntity->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD ), pData->m_fAttackRangeMax );

			TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
			const TVector4& rcOwnerPos = m_pEntity->GetMatrix().GetTranslation();

			return TVector4( rcOwnerPos.x + vAhead.x, rcOwnerPos.y + vAhead.y, rcOwnerPos.z + vAhead.z, rcOwnerPos.w );
		}
	}

	TASSERT( a_pEnemy->IsA( &TGetClass( ABlobEntity ) ) );
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pEnemy );

	// Where the blob will be in a_fTime (from a_pFrom, else its position), unless it moves away from the owner
	TVector4 vOffset;
	TPhysicsManager::GetSingleton()->GetLinearVelocity( AEnemy::GetBlobBody( pBlob ), vOffset );
	vOffset.Multiply( a_fTime );

	const TVector4& rcBlobPos  = pBlob->GetMatrix().GetTranslation();
	const TVector4& rcFrom     = ( a_pFrom != TNULL ) ? *a_pFrom : rcBlobPos;
	const TVector4& rcOwnerPos = m_pEntity->GetMatrix().GetTranslation();

	if ( ( ( rcFrom.y + vOffset.y ) - rcOwnerPos.y ) * ( rcBlobPos.y - rcOwnerPos.y ) + ( ( vOffset.x + rcFrom.x ) - rcOwnerPos.x ) * ( rcBlobPos.x - rcOwnerPos.x ) + ( ( rcFrom.z + vOffset.z ) - rcOwnerPos.z ) * ( rcBlobPos.z - rcOwnerPos.z ) <= 0.0f )
		return rcBlobPos;

	return TVector4( vOffset.x + rcFrom.x, rcFrom.y + vOffset.y, rcFrom.z + vOffset.z, vOffset.w );
}

// $deBlob: FUNCTION 007cfe10
TINT AEnemyBehaviour::GetSurroundPoint( TVector4& a_rvOutPoint, ABaseEntity* a_pEnemy, TFLOAT a_fUnk3, TBOOL a_bUnk4, TBOOL a_bUnk5, TBOOL a_bUnk6, TBOOL a_bUnk7, const TVector4& a_rcUnk8 )
{
	// NOTE: retail only uses a_rvOutPoint and a_pEnemy
	if ( !AEnemyManager::GetSingleton()->CanUpdate() )
	{
		m_iSurroundResult = 1;
		return 1;
	}

	if ( a_pEnemy == TNULL )
	{
		a_pEnemy = m_pSelectedEnemy;

		if ( a_pEnemy == TNULL )
		{
			m_iSurroundResult = 4;
			return 4;
		}
	}

	GetEnemyOwner();

	if ( !( a_pEnemy->GetFlags() & ABaseEntity::FLAG_Player ) )
	{
		m_iSurroundResult = 5;
		return 5;
	}

	// NOTE: retail takes the position of stencil shadow 0 (player 0's blob shadow on the ground, whatever a_pEnemy is)
	AStencilShadow::GetSingleton()->GetShadowPosition( 0, a_rvOutPoint );

	return 0;
}

// $deBlob: FUNCTION 007cfe90
TBOOL AEnemyBehaviour::FaceEnemyPredictedPosition( TFLOAT a_fTime, TBOOL a_bImmediate, ABaseEntity* a_pEnemy )
{
	if ( a_pEnemy == TNULL )
	{
		a_pEnemy = m_pSelectedEnemy;

		if ( a_pEnemy == TNULL )
			return TFALSE;
	}

	const TVector4& rcOwnerPos = m_pEntity->GetMatrix().GetTranslation();
	TVector4        vPredicted = GetEnemyPredictedPosition( a_pEnemy, a_fTime, TNULL );

	TVector4 vFacing( vPredicted.x - rcOwnerPos.x, vPredicted.y - rcOwnerPos.y, vPredicted.z - rcOwnerPos.z, vPredicted.w );
	vFacing.NormaliseXZ();

	AEnemy* pOwner = GetEnemyOwner();

	if ( a_bImmediate )
	{
		TSTATICCAST( AGndMotorComponent, pOwner->GetMotorComponent() )->SetFacing( vFacing );
		return TTRUE;
	}

	return !TSTATICCAST( APathfindingComponent, pOwner->GetPathfinding() )->GetNavigator()->FaceDirection( vFacing );
}

// $deBlob: FUNCTION 007cff60
TBOOL AEnemyBehaviour::CanSeeEnemy( ABaseEntity* a_pEnemy )
{
	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
	TVector4 vEye = TSTATICCAST( AEnemy, m_pEntity )->GetEyePosition();

	TVector4 vTarget = a_pEnemy->GetMatrix().GetTranslation();
	TASSERT( a_pEnemy->IsA( &TGetClass( ABlobEntity ) ) );

	TPhysicsRay oRay;
	oRay.m_vFrom            = vEye;
	oRay.m_vTo              = vTarget;
	oRay.m_uiCollisionLayer = 0x81;

	TPhysicsShapeContact oContact;
	return !TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, TNULL );
}

// $deBlob: FUNCTION 007d0080
void AEnemyBehaviour::UpdateFrustration( TBOOL a_bReset )
{
	if ( m_pSelectedEnemy != TNULL )
	{
		Enemy* pNode = FindInEnemyList( m_pSelectedEnemy );

		if ( pNode != TNULL )
		{
			if ( a_bReset )
			{
				pNode->m_fFrustration = 0.0f;
				return;
			}

			EnemiesData* pData    = AEnemyManager::GetSingleton()->GetEnemiesData();
			pNode->m_fFrustration = TMath::Min( pData->m_fFrustrationStep + pNode->m_fFrustration, pData->m_fFrustrationMax );
		}
	}
}

// $deBlob: FUNCTION 007d00e0
void AEnemyBehaviour::PauseForCutscene( TBOOL a_bPause )
{
	sm_bPausedForCutscene = a_bPause;
}

// $deBlob: FUNCTION 007d00f0
void AEnemyBehaviour::UpdateEnvironment()
{
	if ( sm_bPausedForCutscene )
		return;

	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
	AEnemy* pOwner = TSTATICCAST( AEnemy, m_pEntity );
	TFLOAT  fRange = pOwner->GetVisualRange();

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );

	// The visible player blobs in range and sight become known enemies
	for ( TINT i = 0; i < pPlayerManager->GetNumPlayers(); i++ )
	{
		ABaseEntity* pBlob = pPlayerManager->GetPlayerBlob( TUINT8( i ) );

		if ( ( pBlob->GetFlags() & ABaseEntity::FLAG_Visible ) && !pOwner->ShouldIgnoreBlob( pBlob ) &&
		     TVector4::DistanceSq( m_pEntity->GetMatrix().GetTranslation(), pBlob->GetMatrix().GetTranslation() ) <= fRange * fRange &&
		     !IsKnownEnemy( pBlob ) && CanSeeEnemy( pBlob ) )
		{
			AddToEnemyList( pBlob );
		}
	}
}

// $deBlob: FUNCTION 007d0260
void AEnemyBehaviour::BeenHit( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
	TASSERT( HASANYFLAG( m_pEntity->GetFlags(), ABaseEntity::FLAG_Dead ) );

	OnDeath();
	TSTATICCAST( AAISchedule, m_pSchedule )->StartSchedule( AAIBaseSchedules::BASE_Death );
}

// $deBlob: FUNCTION 007d0330
TBOOL AEnemyBehaviour::GetAttackPoint( ABaseEntity* a_pEntity, TVector4* a_pPoint, TFLOAT a_fGroundDistance )
{
	TASSERT( a_pEntity != TNULL );
	return AAIBehaviour::GetAttackPoint( a_pEntity, a_pPoint, a_fGroundDistance );
}

// $deBlob: FUNCTION 007d0390
void AEnemyBehaviour::JustBeenAttacked( ABaseEntity* a_pAttacker )
{
	if ( a_pAttacker->IsA( &TGetClass( ABaseEntity ) ) && !( TUINT8( a_pAttacker->GetFlags() >> 5 ) & 1 ) )
	{
		ABaseEntity* pEnemy = a_pAttacker;

		if ( !IsKnownEnemy( pEnemy ) )
			AddToEnemyList( pEnemy );

		SetSelectedEnemy( pEnemy );
		AAIBehaviour::JustBeenAttacked( a_pAttacker );
	}
}

// $deBlob: FUNCTION 007d03f0
ABaseEntity* AEnemyBehaviour::SelectEnemy()
{
	if ( !m_Enemies.IsEmpty() )
	{
		Enemy* pNode;
		TINT   iMobBehaviourType = GetMobBehaviourType();

		// The most threatening one, else the closest
		if ( TUINT( iMobBehaviourType - 1 ) < 2 )
			pNode = m_Enemies.Head();
		else
			pNode = GetClosestEnemyNode( TFALSE );

		if ( pNode != TNULL )
			return pNode->m_pEnemy;
	}

	return TNULL;
}

// $deBlob: FUNCTION 007d0470
TINT AEnemyBehaviour::SelectIdleSchedule()
{
	if ( sm_bPausedForCutscene )
		return AAIBaseSchedules::BASE_Idle;

	AEnemy*  pOwner  = GetEnemyOwner();
	TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

	if ( rRandom.GetFloat() <= pOwner->GetFidgetFreq() )
		return AAIBaseSchedules::BASE_Fidget;

	return ( rRandom.GetFloat() <= 0.5f ) ? AAIBaseSchedules::BASE_Patrol : AAIBaseSchedules::BASE_Idle;
}

// $deBlob: FUNCTION 007d05a0
TINT AEnemyBehaviour::SelectAttackSchedule()
{
	// Too many failed paths: it patrols instead
	if ( TINT( TSTATICCAST( AEnemySchedule, m_pSchedule )->GetPathFailTime() ) < 2 )
	{
		ClearTaunted();

		TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
		return TSTATICCAST( AEnemy, m_pEntity )->GetAttackSchedule();
	}

	m_uiFlags |= FLAG_PathFailed;
	return AAIBaseSchedules::BASE_Patrol;
}

// $deBlob: FUNCTION 007d0620
TFLOAT AEnemyBehaviour::CalculateEnemyThreat( Enemy* a_pEnemy, TFLOAT a_fMaxDistance, TFLOAT a_fMaxFrustration )
{
	TINT iMobBehaviourType = GetMobBehaviourType();

	if ( iMobBehaviourType != 1 && iMobBehaviourType != 2 )
		return AAIBehaviour::CalculateEnemyThreat( a_pEnemy, a_fMaxDistance, a_fMaxFrustration );

	// NOTE: retail gets the threat factors without using them
	TFLOAT afFactors[ 6 ];
	GetThreatFactors( &afFactors[ 0 ], &afFactors[ 1 ], &afFactors[ 2 ], &afFactors[ 3 ], &afFactors[ 4 ], &afFactors[ 5 ] );

	return 1.0f;
}

// $deBlob: FUNCTION 007d0690
void AEnemyBehaviour::AddedNewEnemy( Enemy* a_pEnemy )
{
	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
	AEnemy* pOwner = TSTATICCAST( AEnemy, m_pEntity );

	if ( a_pEnemy->m_fDistanceSq < pOwner->GetClosestEnemyDist() )
		pOwner->SetClosestEnemyDist( a_pEnemy->m_fDistanceSq );
}

// $deBlob: FUNCTION 007d0760
void AEnemyBehaviour::GetThreatFactors( TFLOAT* a_pUnk1, TFLOAT* a_pUnk2, TFLOAT* a_pUnk3, TFLOAT* a_pUnk4, TFLOAT* a_pUnk5, TFLOAT* a_pUnk6 )
{
	EnemiesData* pData = AEnemyManager::GetSingleton()->GetEnemiesData();

	*a_pUnk1 = pData->m_afThreatFactors[ 0 ];
	*a_pUnk2 = pData->m_afThreatFactors[ 4 ];
	*a_pUnk3 = pData->m_afThreatFactors[ 1 ];
	*a_pUnk4 = pData->m_afThreatFactors[ 5 ];
	*a_pUnk5 = pData->m_afThreatFactors[ 2 ];
	*a_pUnk6 = pData->m_afThreatFactors[ 3 ];
}

// $deBlob: FUNCTION 007d07a0
AEnemy* AEnemyBehaviour::GetEnemyOwner()
{
	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
	return TSTATICCAST( AEnemy, m_pEntity );
}

// $deBlob: FUNCTION 007d0800
TBOOL AEnemyBehaviour::GetAttackRange( TFLOAT* a_pMin, TFLOAT* a_pMax, TINT a_iAttackType )
{
	*a_pMin = TSTATICCAST( AEnemy::EnemyData, m_pEntity->GetTypeData()->m_pTRBData )->m_fAttackRangeMin;
	*a_pMax = TSTATICCAST( AEnemy::EnemyData, m_pEntity->GetTypeData()->m_pTRBData )->m_fAttackRangeMax;
	return TTRUE;
}

// $deBlob: FUNCTION 007d0830
TBOOL AEnemyBehaviour::GetChooseAttackRange( TFLOAT* a_pMin, TFLOAT* a_pMax, TINT a_iAttackType )
{
	*a_pMin = TSTATICCAST( AEnemy::EnemyData, m_pEntity->GetTypeData()->m_pTRBData )->m_fChooseAttackRangeMin;
	*a_pMax = TSTATICCAST( AEnemy::EnemyData, m_pEntity->GetTypeData()->m_pTRBData )->m_fChooseAttackRangeMax;
	return TTRUE;
}

// $deBlob: FUNCTION 007d0860
TFLOAT AEnemyBehaviour::GetAttackRangeOptimal()
{
	return TSTATICCAST( AEnemy::EnemyData, m_pEntity->GetTypeData()->m_pTRBData )->m_fAttackRangeOptimal;
}

// $deBlob: FUNCTION 007d0870
TFLOAT AEnemyBehaviour::GetAttackedTimeOut( TINT a_iSchedule, TINT a_iTask )
{
	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );

	if ( TSTATICCAST( AEnemy, m_pEntity )->GetAttackedTimeOut( a_iSchedule, a_iTask ) > 0.0f )
		return TSTATICCAST( AEnemy, m_pEntity )->GetAttackedTimeOut( a_iSchedule, a_iTask );

	return 0.3f;
}

// $deBlob: FUNCTION 007d0960
TFLOAT AEnemyBehaviour::GetBeenAttackedTime()
{
	return TSTATICCAST( AEnemy::EnemyData, m_pEntity->GetTypeData()->m_pTRBData )->m_fUnk78;
}

// $deBlob: FUNCTION 007d0d60
TBOOL AEnemyBehaviour::IsEnabled()
{
	return !AStaticCVars::cvDisableEnemyAI.GetBool() && m_bEnabled;
}

// $deBlob: FUNCTION 007d0cb0
TBOOL AEnemyLOSCollisionFunctor::HitIgnorableObject( ACollisionObject* a_pObject )
{
	if ( a_pObject != TNULL && a_pObject->GetOwner() != TNULL )
	{
		TObject* pOwner = TSTATICCAST( TObject, a_pObject->GetOwner() );

		if ( pOwner->IsA( &TGetClass( ABaseEntity ) ) )
		{
			// The visible entities block the sight
			TUINT32 uiFlags = TSTATICCAST( ABaseEntity, pOwner )->GetFlags();
			return !( !HASANYFLAG( uiFlags, 0x20 ) && !HASANYFLAG( uiFlags, 0x10 ) && HASANYFLAG( uiFlags, ABaseEntity::FLAG_Visible ) );
		}
	}

	return TFALSE;
}

#include "pch.h"
#include "AEnemySchedule.h"
#include "Console/AStaticCVars.h"
#include "AEnemy.h"
#include "AEnemyBehaviour.h"
#include "AEnemyTarget.h"
#include "Entities/PathFinding/AAINavigator.h"
#include "Entities/PathFinding/ABuildPathJob.h"
#include "Entities/AGndMotorComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AAIScheduleList<AEnemySchedule::NUM_SCHEDULES> AEnemySchedule::sm_EnemySchedules;

TSTATICASSERT( sizeof( AEnemySchedule ) == 0x6C );

// $deBlob: FUNCTION 007d1540
AEnemySchedule::AEnemySchedule( ABaseEntity* a_pEntity )
    : AAISchedule( a_pEntity )
{
	m_pEnemyBehaviour = TNULL;
	m_iNumPaths       = 0;
	m_fPathFailTime   = 0.0f;
	m_vLastPathGoal   = TVector4::VEC_ZERO;

	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
	m_pEnemyBehaviour = TSTATICCAST( AEnemyBehaviour, TSTATICCAST( AEnemy, m_pEntity )->GetBehaviourComponent() );
}

// $deBlob: FUNCTION 007d26e0
AGndMotorComponent* AEnemySchedule::GetMotorComponent()
{
	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
	return TSTATICCAST( AGndMotorComponent, TSTATICCAST( AEnemy, m_pEntity )->GetMotorComponent() );
}

// $deBlob: FUNCTION 007d2860
// $deBlob: FUNCTION 007d1610
AEnemySchedule::~AEnemySchedule()
{
}

using Task = AAIScheduleManager::ScheduleList::Task;

// The tasks of the schedules (AAITasks::AITASKS, parameter)
static const Task s_aSpawnTasks[]            = { { 27, -1.0f }, { 12, 0.5f } };
static const Task s_aStationaryAttackTasks[] = { { 28, 0.0f }, { 8, 0.0f }, { 10, -1.0f }, { 11, -1.0f } };
static const Task s_aSurroundEnemyTasks[]    = { { 9, 0.0f }, { 7, -1.0f }, { 8, 0.0f }, { 12, 1.0f } };
static const Task s_aTauntTasks[]            = { { 8, 0.0f }, { 30, -1.0f }, { 0, -1.0f } };
static const Task s_aStareAtEnemyTasks[]     = { { 31, 2.0f } };
static const Task s_aGotoScoutTasks[]        = { { 32, -1.0f } };
static const Task s_aHiddenTasks[]           = { { 34, 10.0f } };
static const Task s_aStunnedTasks[]          = { { 33, -1.0f } };
static const Task s_aFallTasks[]             = { { 35, -1.0f } };
static const Task s_aGrappleTasks[]          = { { 36, -1.0f } };
static const Task s_aInkyPenAttackTasks[]    = { { 37, -1.0f }, { 9, 0.0f }, { 7, -1.0f }, { 8, 0.05f }, { 10, -1.0f }, { 11, -1.0f } };

// $deBlob: FUNCTION 007d1620
void AEnemySchedule::InitialiseStatic()
{
	sm_EnemySchedules.SetScheduleOffset( 14 );
	sm_EnemySchedules.AddSchedule( 14, s_aSpawnTasks, 2, 3, "ENEMYSCHEDULE_Spawn" );
	sm_EnemySchedules.AddSchedule( 15, s_aStationaryAttackTasks, 4, 2, "ENEMYSCHEDULE_StationaryAttack" );
	sm_EnemySchedules.AddSchedule( 16, s_aSurroundEnemyTasks, 4, 1, "ENEMYSCHEDULE_SurroundEnemy" );
	sm_EnemySchedules.AddSchedule( 18, s_aTauntTasks, 3, 2, "ENEMYSCHEDULE_Taunt" );
	sm_EnemySchedules.AddSchedule( 17, s_aStareAtEnemyTasks, 1, 0, "ENEMYSCHEDULE_StareAtEnemy" );
	sm_EnemySchedules.AddSchedule( 19, s_aGotoScoutTasks, 1, 1, "ENEMYSCHEDULE_GotoScout" );
	sm_EnemySchedules.AddSchedule( 20, s_aHiddenTasks, 1, 2, "ENEMYSCHEDULE_Hidden" );
	sm_EnemySchedules.AddSchedule( 21, s_aStunnedTasks, 1, 0, "ENEMYSCHEDULE_Stunned" );
	sm_EnemySchedules.AddSchedule( 22, s_aFallTasks, 1, 3, "ENEMYSCHEDULE_Fall" );
	sm_EnemySchedules.AddSchedule( 24, s_aGrappleTasks, 1, 4, "ENEMYSCHEDULE_Grapple" );
	sm_EnemySchedules.AddSchedule( 25, s_aInkyPenAttackTasks, 6, 2, "ENEMYSCHEDULE_InkyPen_Attack" );
}

// $deBlob: FUNCTION 007d1750
TBOOL AEnemySchedule::StartSchedule( TINT a_eSchedule )
{
	if ( m_bScheduleActive && CanScheduleInterrupt( a_eSchedule ) )
		OnTaskFailed( m_pScheduleManager->GetCurrentTask() );

	return AAISchedule::StartSchedule( a_eSchedule );
}

// $deBlob: FUNCTION 007d17a0
AAIScheduleManager::ScheduleList* AEnemySchedule::TranslateSchedule( TINT a_eSchedule )
{
	if ( sm_EnemySchedules.IsScheduleInRange( a_eSchedule ) )
		return sm_EnemySchedules.GetSchedule( a_eSchedule );

	return AAISchedule::TranslateSchedule( a_eSchedule );
}

// $deBlob: FUNCTION 007d17d0
TBOOL AEnemySchedule::StartTask( TINT a_eTask )
{
	m_iNumPaths = 0;

	switch ( a_eTask )
	{
		case 0:
		{
			// Patrol: a random position around the owner, brought back towards the spawn position when it's
			// further away than the patrol radius
			AEnemy* pEnemy        = GetEnemy();
			TFLOAT  fPatrolRadius = pEnemy->GetPatrolRadius();

			if ( TSTATICCAST( AAIBehaviour, GetEnemy()->GetBehaviourComponent() )->HasPathFailed() || fPatrolRadius <= 0.0f )
			{
				CreatePathBuildJob( GetRandMoveLocation() );
				return TTRUE;
			}

			const TVector4& rcSpawn    = pEnemy->GetSpawnPosition();
			const TVector4& rcPosition = pEnemy->GetMatrix().GetTranslation();

			// NOTE: retail compares the squared XZ distance with the radius itself
			if ( fPatrolRadius < ( rcPosition.z - rcSpawn.z ) * ( rcPosition.z - rcSpawn.z ) + ( rcPosition.x - rcSpawn.x ) * ( rcPosition.x - rcSpawn.x ) )
			{
				TVector4 vToSpawn( rcSpawn.x - rcPosition.x, rcSpawn.y - rcPosition.y, rcSpawn.z - rcPosition.z, rcSpawn.w );
				TVector4 vLocation = GetRandMoveLocation();
				CreatePathBuildJob( TVector4( vToSpawn.x + vLocation.x, vLocation.y + vToSpawn.y, vLocation.z + vToSpawn.z, rcSpawn.w ) );
				return TTRUE;
			}

			CreatePathBuildJob( GetRandMoveLocation() );
			return TTRUE;
		}
		case 1:
		case 0x20:
			break;
		case 2:
		{
			if ( m_GroundGoalJob == TNULL )
			{
				// Flee: 5 away from the selected enemy while it's within the flee range
				AEnemy*  pEnemy1    = GetEnemy();
				AEnemy*  pEnemy2    = GetEnemy();
				TVector4 vGrounded  = GetMotorComponent()->GetGroundedPosition();
				TVector4 vPredicted = m_pEnemyBehaviour->GetEnemyPredictedPosition( TNULL, 0.5f, TNULL );

				TFLOAT fDistSq = TVector4::DistanceSq( vPredicted, vGrounded );
				TFLOAT fRange  = pEnemy1->GetFleeRange();
				fRange         = pEnemy2->GetFleeRange() * fRange;

				if ( fDistSq < fRange )
				{
					ABaseEntity*    pTarget     = m_pEnemyBehaviour->GetSelectedEnemy();
					const TVector4& rcTargetPos = pTarget->GetMatrix().GetTranslation();
					const TVector4& rcEnemyPos  = GetEnemy()->GetMatrix().GetTranslation();

					TVector4 vAway( rcEnemyPos.x - rcTargetPos.x, rcEnemyPos.y - rcTargetPos.y, rcEnemyPos.z - rcTargetPos.z, rcEnemyPos.w );
					vAway.NormaliseXZ();

					TVector4 vOffset;
					vOffset.Multiply( vAway, 5.0f );

					const TVector4& rcEnemyPos2 = GetEnemy()->GetMatrix().GetTranslation();
					CreatePathBuildJob( TVector4( vOffset.x + rcEnemyPos2.x, rcEnemyPos2.y + vOffset.y, rcEnemyPos2.z + vOffset.z, rcEnemyPos2.w ) );
					return TTRUE;
				}

				OnTaskFinished( 2, TFALSE );
				return TTRUE;
			}

			if ( !m_GroundGoalJob->IsReady() )
				return TTRUE;

			TBOOL bSuccessful = m_GroundGoalJob->IsSuccessful();
			CancelPathBuildJob();

			if ( !bSuccessful )
			{
				OnTaskFailed( 2 );
				return TTRUE;
			}

			GetEnemy()->StartStatesForTask( 2 );
			return TTRUE;
		}
		case 7:
		{
			// Surround the selected enemy
			ABaseEntity* pTarget = m_pEnemyBehaviour->GetSelectedEnemy();

			if ( pTarget != TNULL )
			{
				if ( m_pEnemyBehaviour->WithinAttackRange( pTarget->GetMatrix().GetTranslation(), pTarget->GetBoundingSphereRadius(), TFALSE, -1 ) )
				{
					OnTaskFinished( 7, TFALSE );
					return TTRUE;
				}

				// NOTE: zeroed; retail's local is uninitialised (GetSurroundPoint fills it when it returns 0)
				TVector4 vPoint = TVector4::VEC_ZERO;

				if ( m_pEnemyBehaviour->GetSurroundPoint( vPoint, TNULL, -1.0f, TTRUE, TTRUE, TFALSE, TTRUE, TVector4::VEC_ZERO ) == 0 )
				{
					CreatePathBuildJob( vPoint );
					return TTRUE;
				}
			}

			break;
		}
		case 9:
			if ( m_pEnemyBehaviour->GetSelectedEnemy() != TNULL )
				return m_pEnemyBehaviour->ChooseAttack( TTRUE, TTRUE ) != -1;

			break;
		case 10:
			// The attack (mob type 1 reserves the target first)
			m_pEnemyBehaviour->UpdateEnemyList( 0.0f );

			if ( m_pEnemyBehaviour->GetSelectedEnemy() != TNULL )
			{
				if ( m_pEnemyBehaviour->GetSelectEnemyType() == 1 && !m_pEnemyBehaviour->GetTargetChar()->ReserveTarget( GetEnemy(), 1.5f ) )
					return TFALSE;

				GetEnemy()->StartStatesForTask( 10 );

				if ( !( GetEnemy()->GetTurnFlags() & ACharEntity::TURNFLAG_AttackFailed ) )
					return TTRUE;

				GetEnemy()->ClearTurnFlags( ACharEntity::TURNFLAG_AttackFailed );
				OnTaskFailed( 10 );
			}

			break;
		case 0x10:
			AAISchedule::StartTask( 0x10 );
			return TTRUE;
		case 0x1B:
		case 0x1E:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
			GetEnemy()->StartStatesForTask( a_eTask );
			return TTRUE;
		case 0x1C:
			if ( m_pEnemyBehaviour->GetSelectedEnemy() != TNULL )
				return m_pEnemyBehaviour->ChooseAttack( TTRUE, TFALSE ) != -1;

			break;
		case 0x1D:
			return TTRUE;
		case 0x1F:
			return m_pEnemyBehaviour->FaceSelectedEnemy( TFALSE );
		case 0x25:
			m_pEnemyBehaviour->UpdateEnemyList( 0.0f );

			if ( m_pEnemyBehaviour->GetSelectedEnemy() != TNULL )
			{
				GetEnemy()->StartStatesForTask( 0x25 );
				return TTRUE;
			}

			break;
		default:
			return AAISchedule::StartTask( a_eTask );
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007d1d30
void AEnemySchedule::UpdateTask( TINT a_eTask, TFLOAT a_fDeltaTime )
{
	switch ( a_eTask )
	{
		case 12:
			// Waits, unless there's an enemy to deal with
			if ( IsTaskTimedOut() || m_pEnemyBehaviour->GetSelectedEnemy() != TNULL )
				OnTaskFinished( 12, TFALSE );

			return;
		case 14:
			if ( TSTATICCAST( AEnemy, GetCharOwner() )->GetCurrentTask() != 14 || m_pEnemyBehaviour->GetSelectedEnemy() != TNULL )
				OnTaskFinished( 14, TFALSE );

			return;
		case 0:
		{
			// Patrol: until the time is up, or there's a target within the spawn range
			if ( IsTaskTimedOut() )
			{
				OnTaskFinished( 0, TFALSE );
				return;
			}

			ABaseEntity* pTarget = m_pEnemyBehaviour->GetSelectedEnemy();

			if ( !TSTATICCAST( AAIBehaviour, GetEnemy()->GetBehaviourComponent() )->HasPathFailed() && pTarget != TNULL )
			{
				if ( GetEnemy()->IsInSpawnRange( pTarget->GetMatrix().GetTranslation(), TTRUE ) )
				{
					OnTaskFinished( 0, TFALSE );
					return;
				}
			}

			if ( !HasPathBuildJob( 0 ) )
				UpdateNavigatorGoal( a_fDeltaTime, TTRUE, TFALSE );

			return;
		}
		case 1:
			OnTaskFinished( 1, TFALSE );
			return;
		case 7:
		{
			if ( HasPathBuildJob( 7 ) )
				break;

			if ( GetEnemy()->ShouldFaceEnemy() )
				m_pEnemyBehaviour->FaceEnemyPredictedPosition( 0.05f, TTRUE, TNULL );

			if ( !IsTaskTimedOut() )
			{
				ABaseEntity* pTarget = m_pEnemyBehaviour->GetSelectedEnemy();
				TFLOAT       fRadius;

				if ( pTarget == TNULL )
				{
					fRadius = 0.0f;
				}
				else
				{
					fRadius = pTarget->GetBoundingSphereRadius();

					if ( m_pEnemyBehaviour->WithinAttackRange( pTarget->GetMatrix().GetTranslation(), fRadius, TTRUE, -1 ) )
					{
						m_pNavigator->ClearGoal( TTRUE );
						OnTaskFinished( m_pScheduleManager->GetCurrentTask(), TFALSE );
						return;
					}
				}

				// The surround point is checked every m_fRepathTimer seconds
				m_fRepathTimer -= a_fDeltaTime;

				if ( 0.0f < m_fRepathTimer )
				{
					UpdateNavigatorGoal( a_fDeltaTime, TTRUE, TFALSE );
					return;
				}

				// NOTE: zeroed; retail's local is uninitialised (GetSurroundPoint fills it when it returns 0)
				TVector4 vPoint  = TVector4::VEC_ZERO;
				TINT     iResult = m_pEnemyBehaviour->GetSurroundPoint( vPoint, TNULL, -1.0f, TTRUE, TTRUE, TFALSE, TTRUE, TVector4::VEC_ZERO );

				if ( iResult == 0 )
				{
					// Still the same goal: goes on
					TVector4 vEndGoal;

					if ( m_pNavigator->GetEndGoal( vEndGoal ) && TVector4::DistanceSq( vPoint, vEndGoal ) < fRadius * fRadius )
					{
						UpdateNavigatorGoal( a_fDeltaTime, TTRUE, TFALSE );
						m_fRepathTimer = GetEnemy()->GetEnemyData()->m_fRepathTime;
						return;
					}

					if ( GetEnemy()->IsInSpawnRange( vPoint, TFALSE ) )
					{
						CreatePathBuildJob( vPoint );
						return;
					}

					// Out of its area: taunts
					StartSchedule( 0x12 );
					return;
				}

				if ( iResult == 1 )
				{
					UpdateNavigatorGoal( a_fDeltaTime, TTRUE, TFALSE );
					return;
				}
			}

			OnTaskFailed( 7 );
			return;
		}
		case 10:
			if ( GetEnemy()->GetTurnFlags() & ACharEntity::TURNFLAG_AttackFailed )
			{
				GetEnemy()->ClearTurnFlags( ACharEntity::TURNFLAG_AttackFailed );
				OnTaskFailed( 10 );
				m_pEnemyBehaviour->SetSurroundResult( -1 );
			}

			if ( GetEnemy()->GetCurrentTask() != 10 )
			{
				OnTaskFinished( 10, TFALSE );
				return;
			}

			break;
		case 0xB:
			if ( IsTaskTimedOut() )
			{
				OnTaskFinished( 0xB, TFALSE );
				return;
			}

			if ( m_pEnemyBehaviour->GetSelectedEnemy() != TNULL )
			{
				m_pEnemyBehaviour->FaceEnemyPredictedPosition( 0.0f, TFALSE, TNULL );
				return;
			}

			break;
		case 0x1B:
		case 0x23:
			if ( GetEnemy()->GetCurrentTask() != a_eTask )
			{
				OnTaskFinished( a_eTask, TFALSE );
				return;
			}

			break;
		case 0x1C:
			OnTaskFinished( 0x1C, TFALSE );
			return;
		case 0x1E:
			m_pEnemyBehaviour->FaceSelectedEnemy( TFALSE );

			if ( GetEnemy()->GetCurrentTask() != 0x1E )
			{
				OnTaskFinished( 0x1E, TFALSE );
				return;
			}

			break;
		case 0x1F:
			m_pEnemyBehaviour->FaceSelectedEnemy( TFALSE );

			if ( m_pEnemyBehaviour->ChooseAttack( TTRUE, TFALSE ) != -1 )
			{
				OnTaskFinished( 0x1F, TTRUE );
				return;
			}

			if ( IsTaskTimedOut() )
			{
				OnTaskFinished( 0x1F, TFALSE );
				return;
			}

			break;
		case 0x20:
			OnTaskFinished( 0x20, TFALSE );
			return;
		case 0x21:
			if ( GetEnemy()->GetCurrentTask() != 0x21 )
			{
				OnTaskFinished( 0x21, TFALSE );
				return;
			}

			break;
		case 0x22:
		{
			// Until an enemy is close
			m_fRepathTimer -= a_fDeltaTime;

			if ( m_fRepathTimer <= 0.0f )
			{
				m_fRepathTimer = TSTATICCAST( AEnemy::EnemyData, m_pEntity->GetTypeData()->m_pTRBData )->m_fRepathTime;

				TFLOAT fDistSq = m_pEnemyBehaviour->GetDistanceSqToClosestEnemy( TTRUE );
				TFLOAT fRange  = GetEnemy()->GetVisualRange();

				if ( 0.0f <= fDistSq && fDistSq <= fRange * fRange )
				{
					OnTaskFinished( 0x22, TFALSE );
					return;
				}
			}

			if ( IsTaskTimedOut() )
			{
				OnTaskFinished( 0x22, TFALSE );
				return;
			}

			break;
		}
		case 0x24:
			if ( GetEnemy()->GetCurrentTask() != 0x24 )
			{
				OnTaskFinished( 0x24, TFALSE );
				return;
			}

			break;
		case 0x25:
			if ( GetEnemy()->GetCurrentTask() != 0x25 )
			{
				OnTaskFinished( 0x25, TFALSE );
				return;
			}

			break;
		default:
			AAISchedule::UpdateTask( a_eTask, a_fDeltaTime );
			break;
	}
}

// $deBlob: FUNCTION 007d22e0
void AEnemySchedule::OnTaskFailed( TINT a_eTask )
{
	m_fTaskTimeOverride = -1.0f;

	if ( a_eTask == 2 )
	{
		m_pNavigator->ClearGoal( TTRUE );
	}
	else if ( a_eTask == 7 )
	{
		m_pEnemyBehaviour->UpdateFrustration( TFALSE );
		m_pNavigator->ClearGoal( TTRUE );
	}
	else if ( a_eTask == 10 && m_pEnemyBehaviour->GetSelectEnemyType() == 1 )
	{
		AEnemyTarget* pTarget = m_pEnemyBehaviour->GetTargetChar();
		TASSERT( pTarget, "Enemy has no target" );
		pTarget->UnreserveTarget( GetEnemy() );
		AAISchedule::OnTaskFailed( 10 );
		return;
	}

	AAISchedule::OnTaskFailed( a_eTask );
}

// $deBlob: FUNCTION 007d2390
void AEnemySchedule::OnTaskFinished( TINT a_eTask, TBOOL a_bNoAdvance )
{
	m_fTaskTimeOverride = -1.0f;

	switch ( a_eTask )
	{
		case 2:
			m_pNavigator->ClearGoal( TTRUE );
			break;
		case 7:
			m_pEnemyBehaviour->UpdateFrustration( TTRUE );
			m_pNavigator->ClearGoal( TTRUE );
			break;
		case 10:
			if ( m_pEnemyBehaviour->GetSelectEnemyType() == 1 )
			{
				AEnemyTarget* pTarget = m_pEnemyBehaviour->GetTargetChar();
				TASSERT( pTarget, "Enemy has no target" );
				pTarget->UnreserveTarget( GetEnemy() );
			}
			break;
		case 0x10:
			GetEnemy()->ReleaseToWrapper();
			break;
	}

	AAISchedule::OnTaskFinished( a_eTask, a_bNoAdvance );
}

// $deBlob: FUNCTION 007d24b0
void AEnemySchedule::PostCreatePathBuildJob( const TVector4& a_rvGoal )
{
	// The same goal again soon after: the time grows
	if ( m_fPathFailTime == 0.0f && m_vLastPathGoal.x == TVector4::VEC_ZERO.x && m_vLastPathGoal.y == TVector4::VEC_ZERO.y && m_vLastPathGoal.z == TVector4::VEC_ZERO.z && m_vLastPathGoal.w == TVector4::VEC_ZERO.w )
	{
		m_fPathFailTime = 0.5f;
		m_vLastPathGoal = a_rvGoal;
		return;
	}

	if ( TVector4::DistanceSq( a_rvGoal, m_vLastPathGoal ) <= 0.09f )
	{
		m_fPathFailTime += 0.5f;
		return;
	}

	m_fPathFailTime = 0.5f;
	m_vLastPathGoal = a_rvGoal;
}

// $deBlob: FUNCTION 007d2540
void AEnemySchedule::HandleFailedPathBuildJob( TINT a_eTask, const TVector4& a_rvGoal )
{
	m_fPathFailTime += 0.5f;
	OnTaskFailed( a_eTask );
}

// $deBlob: FUNCTION 007d2570
void AEnemySchedule::HandleSuccessfulPathBuildJob( TINT a_eTask, const TVector4& a_rvGoal )
{
	AEnemy* pEnemy = TSTATICCAST( AEnemy, m_pEntity );
	m_fRepathTimer = pEnemy->GetEnemyData()->m_fRepathTime;

	if ( m_iNumPaths <= 0 )
	{
		m_fTaskTimeOverride = pEnemy->GetEnemyData()->m_fPathTaskTime;
		pEnemy->StartStatesForTask( a_eTask );
	}

	m_iNumPaths++;
	TSTATICCAST( AEnemy, m_pEntity )->StartStatesForTask( a_eTask );

	m_fPathFailTime = 0.0f;
	m_vLastPathGoal = TVector4::VEC_ZERO;
}

// $deBlob: FUNCTION 007d2670
TFLOAT AEnemySchedule::GetAttackRecoverTime()
{
	return TSTATICCAST( AEnemy::EnemyData, m_pEntity->GetTypeData()->m_pTRBData )->m_fAttackRecoverTime;
}

// $deBlob: FUNCTION 007d2680
AEnemy* AEnemySchedule::GetEnemy() const
{
	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( AEnemy ) ) );
	return TSTATICCAST( AEnemy, m_pEntity );
}

// $deBlob: FUNCTION 007d2a20
TBOOL AEnemySchedule::IsEnabled()
{
	return !AStaticCVars::cvDisableEnemyAI.GetBool() && m_bEnabled;
}

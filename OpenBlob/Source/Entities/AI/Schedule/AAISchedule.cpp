#include "pch.h"
#include "AAISchedule.h"
#include "AAIBaseSchedules.h"
#include "Entities/ACharEntity.h"
#include "Entities/AI/Behaviour/AAIBehaviour.h"
#include "Entities/PathFinding/APathfindingComponent.h"
#include "Entities/PathFinding/AAINavigator.h"
#include "Entities/PathFinding/ABuildPathJob.h"
#include "Entities/AGndMotorComponent.h"
#include "JobSystem/AJobQueue.h"
#include "ARandom.h"
#include "Console/AConsoleState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00541a10
// $deBlob: FUNCTION 0085a130
static AConsoleVar s_cvScheduleShow( "schedule_show", TFALSE ); // 0x009b9078
// $deBlob: FUNCTION 00541a30
// $deBlob: FUNCTION 0085a140
static AConsoleVar s_cvScheduleShowTask( "schedule_show_task", TFALSE ); // 0x009b90a0

// $deBlob: FUNCTION 007e29d0
AAISchedule::AAISchedule( ABaseEntity* a_pEntity )
    : AAISchedulePure( a_pEntity )
{
	m_pScheduleManager  = TNULL;
	m_bScheduleActive   = TFALSE;
	m_pBehaviour        = TNULL;
	m_pNavigator        = TNULL;
	m_fTaskTimeOverride = -1.0f;
	m_fGotoAttackTimer  = 0.0f;


	m_GroundGoalJob = TNULL;
	TASSERT( a_pEntity->IsA( &TGetClass( ACharEntity ) ) );

	m_pScheduleManager = new AAIScheduleManager();
	AAISchedule::ResetComponent();

	AConsoleState::RegisterCVar( &s_cvScheduleShow );
	AConsoleState::RegisterCVar( &s_cvScheduleShowTask );
}

// $deBlob: FUNCTION 007e2af0
// $deBlob: FUNCTION 007e4300
AAISchedule::~AAISchedule()
{
	m_CustomInterrupts.Clear();
	delete m_pScheduleManager;
}

// $deBlob: FUNCTION 007e2b90
void AAISchedule::ResetComponent()
{
	ACharEntity* pOwner = GetCharOwner();
	m_pBehaviour        = TSTATICCAST( AAIBehaviour, pOwner->GetBehaviourComponent() );

	pOwner       = GetCharOwner();
	m_pNavigator = TSTATICCAST( APathfindingComponent, pOwner->GetPathfinding() )->GetNavigator();

	m_bEnabled        = TTRUE;
	m_bScheduleActive = TFALSE;
	m_pScheduleManager->Reset();
}

// $deBlob: FUNCTION 007e2c60
TBOOL AAISchedule::StartSchedule( TINT a_eSchedule )
{
	if ( m_bScheduleActive && !CanScheduleInterrupt( a_eSchedule ) )
		return TFALSE;

	m_pScheduleManager->SetScheduleList( TranslateSchedule( a_eSchedule ), a_eSchedule );
	m_bScheduleActive = TTRUE;

	if ( m_GroundGoalJob != TNULL )
	{
		m_GroundGoalJob->SetDone();
		AJobQueue::GetInstance()->RemoveJob( m_GroundGoalJob );
		m_GroundGoalJob = TNULL;
	}

	m_pBehaviour->OnScheduleStarted( a_eSchedule );
	m_pScheduleManager->ResetTaskTime();

	if ( !StartTask( m_pScheduleManager->GetCurrentTask() ) )
	{
		m_bScheduleActive = TFALSE;
		m_pBehaviour->OnScheduleFinished( m_pScheduleManager->GetSchedule(), 1, m_pScheduleManager->GetCurrentTask() );
	}

	return m_bScheduleActive;
}

// $deBlob: FUNCTION 007e2e50
void AAISchedule::AppendCustomInterruptToSchedule( TINT a_eSchedule, const TINT* a_pSchedules, TINT a_iNumSchedules )
{
	CustomInterruptMap::Iterator it = m_CustomInterrupts.Find( a_eSchedule );
	CustomInterruptList*         pList;

	if ( it == m_CustomInterrupts.End() )
		pList = m_CustomInterrupts.Insert( a_eSchedule, CustomInterruptList() );
	else
		pList = &it.GetValue()->GetSecond();

	TASSERT( pList->Size() < MAX_CUSTOM_INTERRUPTS );

	CustomInterrupt oInterrupt;
	oInterrupt.m_pSchedules    = a_pSchedules;
	oInterrupt.m_iNumSchedules = a_iNumSchedules;
	pList->PushBack( oInterrupt );
}

// $deBlob: FUNCTION 007e2d70
TBOOL AAISchedule::CanScheduleInterrupt( TINT a_eSchedule )
{
	if ( !m_bScheduleActive )
		return TTRUE;

	// The custom interrupts of the current schedule
	CustomInterruptMap::Iterator it = m_CustomInterrupts.Find( m_pScheduleManager->GetSchedule() );

	if ( it != m_CustomInterrupts.End() )
	{
		CustomInterruptList& rList = it.GetValue()->GetSecond();

		for ( TINT i = 0; i < rList.Size(); i++ )
		{
			for ( TINT j = 0; j < rList[ i ].m_iNumSchedules; j++ )
			{
				if ( rList[ i ].m_pSchedules[ j ] == a_eSchedule )
					return TTRUE;
			}
		}
	}

	// Else one of a higher (or the same) priority
	AAIScheduleManager::ScheduleList* pList = TranslateSchedule( a_eSchedule );

	if ( pList != TNULL )
		return m_pScheduleManager->GetScheduleList()->m_iPriority <= pList->m_iPriority;

	return TFALSE;
}

// $deBlob: FUNCTION 007e2f20
TINT AAISchedule::GetActiveSchedule() const
{
	return m_pScheduleManager->GetSchedule();
}

// $deBlob: FUNCTION 007e2f70
AAIScheduleManager::ScheduleList* AAISchedule::TranslateSchedule( TINT a_eSchedule )
{
	TASSERT( AAIBaseSchedules::sm_BaseSchedules.IsScheduleInRange( a_eSchedule ), "You must implement this for your derived classes" );
	return AAIBaseSchedules::sm_BaseSchedules.GetSchedule( a_eSchedule );
}

// $deBlob: FUNCTION 007e2fd0
void AAISchedule::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bScheduleActive )
	{
		m_pScheduleManager->Update( a_fDeltaTime );
		UpdateTask( m_pScheduleManager->GetCurrentTask(), a_fDeltaTime );
		return;
	}

	m_pBehaviour->SelectSchedule();
}

// $deBlob: FUNCTION 007e3030
TINT AAISchedule::UpdateNavigatorGoal( TFLOAT a_fDeltaTime, TBOOL a_bFinishTask, TBOOL a_bAuxPath )
{
	TINT iResult = 0;
	TINT eState  = m_pNavigator->GetState();

	if ( eState != AAINavigator::STATE_Moving )
	{
		iResult = ( m_pNavigator->GetResult() == AAINavigator::RESULT_Reached ) ? 1 : -1;
		m_pNavigator->ClearGoal( TTRUE );
	}

	if ( a_bFinishTask )
	{
		if ( iResult == -1 )
		{
			OnTaskFailed( m_pScheduleManager->GetCurrentTask() );
			return -1;
		}

		if ( iResult == 1 )
		{
			if ( a_bAuxPath && m_pNavigator->IsOtherPathValid() )
			{
				m_pNavigator->ActivateAuxPath();
				return 2;
			}

			OnTaskFinished( m_pScheduleManager->GetCurrentTask(), TFALSE );
			return 1;
		}
	}

	return iResult;
}

// $deBlob: FUNCTION 007e3140
TBOOL AAISchedule::StartTask( TINT a_eTask )
{
	switch ( a_eTask )
	{
		case 0:
			// Patrol: somewhere around
			CreatePathBuildJob( GetRandMoveLocation() );
			return TTRUE;
		case 1:
			break;
		case 2:
		{
			// Flee: away from the selected enemy, out of the owner's flee range
			ABaseEntity*    pEnemy     = m_pBehaviour->GetSelectedEnemy();
			const TVector4& rcOwnerPos = m_pEntity->GetMatrix().GetTranslation();
			const TVector4& rcEnemyPos = pEnemy->GetMatrix().GetTranslation();

			TVector4 vAway( rcOwnerPos.x - rcEnemyPos.x, rcOwnerPos.y - rcEnemyPos.y, rcOwnerPos.z - rcEnemyPos.z, rcOwnerPos.w );
			TFLOAT   fRange = GetCharOwner()->GetFleeRange();

			if ( fRange * fRange <= vAway.MagnitudeSq() )
			{
				OnTaskFinished( 2, TFALSE );
				return TTRUE;
			}

			vAway.Normalise();
			vAway.Multiply( fRange );

			const TVector4& rcEnemyPos2 = pEnemy->GetMatrix().GetTranslation();
			CreatePathBuildJob( TVector4( rcEnemyPos2.x + vAway.x, rcEnemyPos2.y + vAway.y, rcEnemyPos2.z + vAway.z, rcEnemyPos2.w ) );
			return TTRUE;
		}
		case 3:
			GetCharOwner()->StartInvalidTask();
			m_pNavigator->ClearGoal( TTRUE );
			return TTRUE;
		case 4:
		case 5:
		case 6:
		case 12:
		case 14:
		case 15:
		case 16:
			// Tasks that only run the owner's states
			GetCharOwner()->StartStatesForTask( a_eTask );
			return TTRUE;
		case 7:
		{
			// Goto attack: to the optimal attack range of the selected enemy
			ABaseEntity* pEnemy = m_pBehaviour->GetSelectedEnemy();

			if ( pEnemy != TNULL )
			{
				if ( m_pBehaviour->WithinAttackRange( pEnemy->GetMatrix().GetTranslation(), pEnemy->GetBoundingSphereRadius(), TFALSE, -1 ) )
				{
					OnTaskFinished( 7, TFALSE );
					return TTRUE;
				}

				TVector4 vGoal;

				if ( m_pBehaviour->GetOptimalAttackPosition( vGoal ) )
					CreatePathBuildJob( vGoal );

				return TTRUE;
			}

			break;
		}
		case 8:
			GetCharOwner()->StartStatesForTask( 8 );
			return m_pBehaviour->FaceSelectedEnemy( TFALSE );
		case 9:
			if ( m_pBehaviour->GetSelectedEnemy() != TNULL )
				return m_pBehaviour->ChooseAttack( TTRUE, TTRUE ) != -1;

			break;
		case 10:
			GetCharOwner()->StartStatesForTask( 10 );

			if ( !( GetCharOwner()->GetTurnFlags() & ACharEntity::TURNFLAG_AttackFailed ) )
				return TTRUE;

			GetCharOwner()->ClearTurnFlags( ACharEntity::TURNFLAG_AttackFailed );
			OnTaskFailed( 10 );
			return TTRUE;
		case 0xB:
			m_fTaskTimeOverride = GetAttackRecoverTime();
			GetCharOwner()->StartStatesForTask( 0xB );
			return TTRUE;
		case 0xD:
			if ( TSTATICCAST( AGndMotorComponent, GetCharOwner()->GetMotorComponent() )->IsOnGround( 0.03f ) )
				OnTaskFinished( 0xD, TFALSE );

			return TTRUE;
		default:
			TASSERT( !"Don't know how to start task\n" );
			break;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007e3470
void AAISchedule::UpdateTask( TINT a_eTask, TFLOAT a_fDeltaTime )
{
	switch ( a_eTask )
	{
		case 0:
			if ( IsTaskTimedOut() )
			{
				OnTaskFinished( 0, TFALSE );
				return;
			}

			if ( !HasPathBuildJob( 0 ) )
				UpdateNavigatorGoal( a_fDeltaTime, TTRUE, TFALSE );

			break;
		case 2:
			if ( !HasPathBuildJob( 2 ) )
			{
				if ( IsTaskTimedOut() )
				{
					OnTaskFinished( 2, TFALSE );
					return;
				}

				UpdateNavigatorGoal( a_fDeltaTime, TTRUE, TFALSE );
			}

			break;
		case 3:
		case 9:
			OnTaskFinished( a_eTask, TFALSE );
			return;
		case 4:
		case 6:
		case 10:
		case 0xE:
		case 0xF:
		case 0x10:
			// Until the owner's states are done with it
			if ( GetCharOwner()->GetCurrentTask() != a_eTask )
			{
				OnTaskFinished( a_eTask, TFALSE );
				return;
			}

			break;
		case 5:
		case 11:
		case 12:
			// Finished once their time is up
			if ( IsTaskTimedOut() )
				OnTaskFinished( a_eTask, TFALSE );

			return;
		case 7:
			if ( !HasPathBuildJob( 7 ) )
			{
				if ( IsTaskTimedOut() )
				{
					OnTaskFailed( 7 );
					return;
				}

				// The goal follows the selected enemy every GetGotoAttackTimeUpdate seconds
				m_fGotoAttackTimer -= a_fDeltaTime;

				if ( m_fGotoAttackTimer <= 0.0f )
				{
					m_fGotoAttackTimer = GetGotoAttackTimeUpdate();

					TVector4 vGoal;

					if ( m_pBehaviour->GetOptimalAttackPosition( vGoal ) )
					{
						CreatePathBuildJob( vGoal );
						return;
					}

					OnTaskFailed( 7 );
					return;
				}

				UpdateNavigatorGoal( a_fDeltaTime, TTRUE, TFALSE );
				return;
			}

			break;
		case 8:
			// Faces the selected enemy until it's turned
			if ( m_pBehaviour->GetSelectedEnemy() == TNULL )
			{
				OnTaskFailed( 8 );
				return;
			}

			if ( IsTaskTimedOut() || !( GetCharOwner()->GetTurnFlags() & ACharEntity::TURNFLAG_Mask ) )
			{
				OnTaskFinished( 8, TFALSE );
				return;
			}

			break;
		case 0xD:
			if ( TSTATICCAST( AGndMotorComponent, GetCharOwner()->GetMotorComponent() )->IsOnGround( 0.03f ) )
			{
				OnTaskFinished( 0xD, TFALSE );
				return;
			}

			break;
		default:
			TASSERT( !"Unhandled task" );
			break;
	}
}

// $deBlob: FUNCTION 007e36e0
void AAISchedule::OnTaskFailed( TINT a_eTask )
{
	m_fTaskTimeOverride = -1.0f;
	m_pNavigator->ClearGoal( TTRUE );

	if ( m_GroundGoalJob != TNULL )
	{
		m_GroundGoalJob->SetDone();
		AJobQueue::GetInstance()->RemoveJob( m_GroundGoalJob );
		m_GroundGoalJob = TNULL;
	}

	m_bScheduleActive = TFALSE;
	m_pBehaviour->OnScheduleFinished( m_pScheduleManager->GetSchedule(), 1, a_eTask );
	GetCharOwner()->HandleTaskFailed( a_eTask );
}

// $deBlob: FUNCTION 007e37a0
void AAISchedule::OnTaskFinished( TINT a_eTask, TBOOL a_bNoAdvance )
{
	m_fTaskTimeOverride = -1.0f;

	if ( !m_bScheduleActive )
		return;

	if ( a_eTask >= 0 && ( a_eTask < 2 || a_eTask == 7 ) )
	{
		m_pNavigator->ClearGoal( TTRUE );
	}

	if ( m_GroundGoalJob != TNULL )
	{
		m_GroundGoalJob->SetDone();
		AJobQueue::GetInstance()->RemoveJob( m_GroundGoalJob );
		m_GroundGoalJob = TNULL;
	}

	AAIScheduleManager::ScheduleList* pList = m_pScheduleManager->GetScheduleList();

	if ( ( pList == TNULL || m_pScheduleManager->GetTaskIndex() != pList->m_iNumTasks - 1 ) && a_bNoAdvance != TTRUE )
	{
		// The next task of the schedule
		m_pScheduleManager->AdvanceTask();
		m_pScheduleManager->ResetTaskTime();

		if ( !StartTask( m_pScheduleManager->GetCurrentTask() ) )
		{
			OnTaskFailed( m_pScheduleManager->GetCurrentTask() );
			m_pBehaviour->OnScheduleFinished( m_pScheduleManager->GetSchedule(), 1, m_pScheduleManager->GetCurrentTask() );
			GetCharOwner()->HandleTaskFinished( a_eTask, TFALSE );
		}
	}
	else
	{
		// The schedule is done
		m_bScheduleActive = TFALSE;
		m_pBehaviour->OnScheduleFinished( m_pScheduleManager->GetSchedule(), 0, m_pScheduleManager->GetCurrentTask() );
		GetCharOwner()->HandleTaskFinished( a_eTask, TTRUE );
	}
}

// $deBlob: FUNCTION 007e3920
TBOOL AAISchedule::IsTaskTimedOut() const
{
	AAIScheduleManager::ScheduleList* pList = m_pScheduleManager->GetScheduleList();
	TFLOAT                            fLimit;

	if ( pList != TNULL && pList->m_pTasks[ m_pScheduleManager->GetTaskIndex() ].m_fParam == -1.0f )
	{
		fLimit = m_fTaskTimeOverride;

		if ( fLimit == -1.0f )
			return TFALSE;
	}
	else
	{
		if ( m_fTaskTimeOverride != -1.0f )
			return m_pScheduleManager->GetTaskTime() > m_fTaskTimeOverride;

		if ( pList == TNULL )
			return TFALSE;

		fLimit = 0.0f;

		if ( pList->m_pTasks[ m_pScheduleManager->GetTaskIndex() ].m_fParam <= 0.0f )
			return TFALSE;

		fLimit = pList->m_pTasks[ m_pScheduleManager->GetTaskIndex() ].m_fParam;
	}

	if ( m_pScheduleManager->GetTaskTime() > fLimit )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 007e39c0
TVector4 AAISchedule::GetRandMoveLocation()
{
	TASSERT( GetEntity()->IsA( &TGetClass( ACharEntity ) ) );
	ACharEntity* pOwner = GetCharOwner();

	TFLOAT fMinDist = pOwner->GetAttackRangeOptimal();
	TFLOAT fMaxDist = pOwner->GetAttackRangeMax();
	TASSERT( fMinDist <= fMaxDist );

	TVector4 vLocation = TSTATICCAST( AGndMotorComponent, pOwner->GetMotorComponent() )->GetGroundedPosition();
	TFLOAT   fRange    = fMaxDist - fMinDist;
	TRandom& rRandom   = ARandom::GetSingleton()->GetRandom();

	// fMinDist to fMaxDist away on X and on Z, either way
	TFLOAT fOffset = rRandom.GetFloatMinMax( -fRange, fRange );
	vLocation.x += ( fOffset >= 0.0f ) ? fOffset + fMinDist : fOffset - fMinDist;

	fOffset = rRandom.GetFloatMinMax( -fRange, fRange );
	vLocation.z += ( fOffset >= 0.0f ) ? fOffset + fMinDist : fOffset - fMinDist;

	return vLocation;
}

// $deBlob: FUNCTION 007e3c00
void AAISchedule::CreatePathBuildJob( const TVector4& a_rvGoal )
{
	ACharEntity*  pOwner     = GetCharOwner();
	AAINavigator* pNavigator = TSTATICCAST( APathfindingComponent, pOwner->GetPathfinding() )->GetNavigator();
	TVector4      vPosition  = TSTATICCAST( AGndMotorComponent, pOwner->GetMotorComponent() )->GetGroundedPosition();

	ABuildPathJob* pJob = pNavigator->BuildPath( vPosition, a_rvGoal, 0.01f, TFALSE, TNULL, AAINavigator::GROUNDGOAL_Ground, AAINavigator::DOUBLEBUFFER_Current );

	SetGroundGoalJob( pJob );

	PostCreatePathBuildJob( a_rvGoal );
}

// $deBlob: FUNCTION 007e3ce0
TBOOL AAISchedule::HasPathBuildJob( TINT a_eTask )
{
	if ( m_GroundGoalJob == TNULL )
		return TFALSE;

	if ( m_GroundGoalJob->IsReady() )
	{
		TVector4 vGoal       = m_GroundGoalJob->GetGoal();
		TBOOL    bSuccessful = m_GroundGoalJob->IsSuccessful();

		m_GroundGoalJob->SetDone();
		AJobQueue::GetInstance()->RemoveJob( m_GroundGoalJob );
		m_GroundGoalJob = TNULL;

		if ( !bSuccessful )
		{
			HandleFailedPathBuildJob( a_eTask, vGoal );
			OnTaskFailed( a_eTask );
			return TTRUE;
		}

		HandleSuccessfulPathBuildJob( a_eTask, vGoal );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 007e3d60
void AAISchedule::CancelPathBuildJob()
{
	if ( m_GroundGoalJob != TNULL )
	{
		m_GroundGoalJob->SetDone();
		AJobQueue::GetInstance()->RemoveJob( m_GroundGoalJob );
		m_GroundGoalJob = TNULL;
	}
}

// $deBlob: FUNCTION 007e3d90
ACharEntity* AAISchedule::GetCharOwner() const
{
	TASSERT( ( m_pEntity == TNULL ) || m_pEntity->IsA( &TGetClass( ACharEntity ) ) );
	return TSTATICCAST( ACharEntity, m_pEntity );
}

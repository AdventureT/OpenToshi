#include "pch.h"
#include "AAIBaseSchedules.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AAIScheduleList<AAIBaseSchedules::NUM_SCHEDULES> AAIBaseSchedules::sm_BaseSchedules;

using Task = AAIScheduleManager::ScheduleList::Task;

// The tasks of the schedules (AAITasks::AITASKS, parameter)
static const Task s_aIdleTasks[]       = { { 12, 0.3f } };
static const Task s_aFidgetTasks[]     = { { 14, -1.0f } };
static const Task s_aPatrolTasks[]     = { { 0, -1.0f }, { 12, 0.8f } };
static const Task s_aRecoilTasks[]     = { { 3, 0.0f }, { 4, -1.0f }, { 5, 0.5f } };
static const Task s_aAttackTasks[]     = { { 9, 0.0f }, { 7, -1.0f }, { 8, 0.05f }, { 10, -1.0f }, { 11, -1.0f } };
static const Task s_aBlockTasks[]      = { { 3, 0.0f }, { 15, -1.0f } };
static const Task s_aGotoLeaderTasks[] = { { 1, -1.0f } };
static const Task s_aFleeTasks[]       = { { 2, 3.0f } };
static const Task s_aDeathTasks[]      = { { 3, 0.0f }, { 16, -1.0f } };
static const Task s_aAlertTasks[]      = { { 8, 0.05f }, { 6, -1.0f } };

// $deBlob: FUNCTION 007ea530
// $deBlob: FUNCTION 007ea540
// $deBlob: FUNCTION 007ea5b0
// $deBlob: FUNCTION 00541ad0
// $deBlob: FUNCTION 0085a170
TDEFINE_CLASS_CORE( AAIBaseSchedules );
Toshi::TClass AAIBaseSchedules::TClassObjectName = Toshi::TClass( "AAIBaseSchedules", AAIBaseSchedules::GetParentClassStatic(), AAIBaseSchedules::CreateTObject, AAIBaseSchedules::CreateTObjectInPlace, AAIBaseSchedules::InitialiseClass, AAIBaseSchedules::DeinitialiseClass, 0, 1, sizeof( AAIBaseSchedules ), TALIGNOF( AAIBaseSchedules ) );

// $deBlob: FUNCTION 007ea620
void AAIBaseSchedules::InitialiseStatic()
{
	sm_BaseSchedules.SetScheduleOffset( BASE_Idle );
	sm_BaseSchedules.AddSchedule( BASE_Idle, s_aIdleTasks, 1, 0, "BASE_Idle" );
	sm_BaseSchedules.AddSchedule( BASE_Fidget, s_aFidgetTasks, 1, 0, "BASE_Fidget" );
	sm_BaseSchedules.AddSchedule( BASE_Patrol, s_aPatrolTasks, 2, 1, "BASE_Patrol" );
	sm_BaseSchedules.AddSchedule( BASE_Recoil, s_aRecoilTasks, 3, 4, "BASE_Recoil" );
	sm_BaseSchedules.AddSchedule( BASE_Attack, s_aAttackTasks, 5, 2, "BASE_Attack" );
	sm_BaseSchedules.AddSchedule( BASE_Block, s_aBlockTasks, 2, 5, "BASE_Block" );
	sm_BaseSchedules.AddSchedule( BASE_GotoLeader, s_aGotoLeaderTasks, 1, 1, "BASE_GotoLeader" );
	sm_BaseSchedules.AddSchedule( BASE_Flee, s_aFleeTasks, 1, 1, "BASE_Flee" );
	sm_BaseSchedules.AddSchedule( BASE_Death, s_aDeathTasks, 2, 6, "BASE_Death" );
	sm_BaseSchedules.AddSchedule( BASE_Alert, s_aAlertTasks, 2, 2, "BASE_Alert" );
}

// NOTE: retail's TClass has InitialiseStatic itself as InitialiseClass
void AAIBaseSchedules::InitialiseClass()
{
	InitialiseStatic();
}

// $deBlob: FUNCTION 007ea730
void AAIBaseSchedules::DeinitialiseClass()
{
}

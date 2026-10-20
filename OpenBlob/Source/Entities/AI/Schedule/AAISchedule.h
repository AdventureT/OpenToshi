#pragma once
#include "AAISchedulePure.h"
#include "AAIScheduleManager.h"

#include <Math/TVector4.h>
#include <Toshi/T2Map.h>
#include <Toshi/T2Vector.h>

class AAIBehaviour;
class ACharEntity;
class AAINavigator;
class ABuildPathJob;

//-----------------------------------------------------------------------------
// Purpose: the base of the AI schedules (abstract: +0x3C-+0x44 are pure): runs the tasks of the current schedule
// (AAIScheduleManager) and asks the behaviour for the next schedule once it's finished.
//-----------------------------------------------------------------------------
class AAISchedule
    : public AAISchedulePure
{
public:
	// Schedules that may interrupt a schedule besides those of a higher priority (8 bytes)
	struct CustomInterrupt
	{
		const TINT* m_pSchedules;    // 0x00
		TINT        m_iNumSchedules; // 0x04
	};

	static TCONSTEXPR_VAR TINT MAX_CUSTOM_INTERRUPTS = 10;

	using CustomInterruptList = Toshi::T2Vector<CustomInterrupt, MAX_CUSTOM_INTERRUPTS>;
	using CustomInterruptMap  = Toshi::T2Map<TINT, CustomInterruptList>;

public:
	AAISchedule( ABaseEntity* a_pEntity );
	virtual ~AAISchedule() override; // +0x00

	virtual void                              Tick( TFLOAT a_fDeltaTime ) override;               // +0x04
	virtual void                              ResetComponent() override;                          // +0x08
	virtual AAIScheduleManager::ScheduleList* TranslateSchedule( TINT a_eSchedule ) override;     // +0x14
	virtual TBOOL                             StartSchedule( TINT a_eSchedule );                  // +0x18
	virtual TBOOL                             CanScheduleInterrupt( TINT a_eSchedule );           // +0x1C
	virtual TBOOL                             StartTask( TINT a_eTask );                          // +0x20
	virtual void                              UpdateTask( TINT a_eTask, TFLOAT a_fDeltaTime );    // +0x24
	virtual void                              OnTaskFailed( TINT a_eTask );                       // +0x28
	virtual void                              OnTaskFinished( TINT a_eTask, TBOOL a_bNoAdvance ); // +0x2C
	// $deBlob: FUNCTION 007e4570
	virtual TFLOAT GetAttackRecoverTime() { return 0.5f; } // +0x30
	// $deBlob: FUNCTION 007d29f0
	virtual TFLOAT GetGotoAttackTimeLimit() { return 3.0f; } // +0x34
	// $deBlob: FUNCTION 007d2a00
	virtual TFLOAT GetGotoAttackTimeUpdate() { return 1.0f; }                                         // +0x38
	virtual void   PostCreatePathBuildJob( const Toshi::TVector4& a_rvGoal )                     = 0; // +0x3C
	virtual void   HandleFailedPathBuildJob( TINT a_eTask, const Toshi::TVector4& a_rvGoal )     = 0; // +0x40
	virtual void   HandleSuccessfulPathBuildJob( TINT a_eTask, const Toshi::TVector4& a_rvGoal ) = 0; // +0x44

	ACharEntity* GetCharOwner() const;

	// a_pSchedules may interrupt a_eSchedule (nothing calls it in retail)
	void AppendCustomInterruptToSchedule( TINT a_eSchedule, const TINT* a_pSchedules, TINT a_iNumSchedules );

	// Has the current task run out of time (m_fTaskTimeOverride, else the task's parameter if it's > 0)?
	TBOOL IsTaskTimedOut() const;

	// A random position around the owner, between its optimal and maximum attack range on X and Z
	Toshi::TVector4 GetRandMoveLocation();

	// Builds the path to a_rvGoal (a ground goal job, m_GroundGoalJob)
	void CreatePathBuildJob( const Toshi::TVector4& a_rvGoal );

	// Is a path build job running for a_eTask (when it's ready, the result goes to HandleSuccessfulPathBuildJob or
	// HandleFailedPathBuildJob)?
	TBOOL HasPathBuildJob( TINT a_eTask );

	void CancelPathBuildJob();

	// When the navigator is done: -1 failed, 1 reached (2: the aux path goes on), 0 still moving; a_bFinishTask
	// fails or finishes the current task
	TINT UpdateNavigatorGoal( TFLOAT a_fDeltaTime, TBOOL a_bFinishTask, TBOOL a_bAuxPath );

	// $deBlob: FUNCTION 007e3120
	void  OverrideTaskTime( TFLOAT a_fTime ) { m_fTaskTimeOverride = a_fTime; }
	TBOOL IsScheduleActive() const { return m_bScheduleActive; }
	TINT                GetActiveSchedule() const;
	AAIScheduleManager* GetScheduleManager() const { return m_pScheduleManager; }
	// $deBlob: FUNCTION 007d2a10
	ABuildPathJob* GetGroundGoalJob() const { return m_GroundGoalJob; }
	// $deBlob: FUNCTION 007e2f40
	TFLOAT GetTaskTime() const { return m_pScheduleManager->GetTaskTime(); }

	// $deBlob: FUNCTION 007e4830
	void SetGroundGoalJob( ABuildPathJob* a_pJob )
	{
		TASSERT( TNULL == m_GroundGoalJob );
		m_GroundGoalJob = a_pJob;
	}

protected:
	AAIScheduleManager* m_pScheduleManager;  // 0x14
	TBOOL               m_bScheduleActive;   // 0x18
	AAIBehaviour*       m_pBehaviour;        // 0x1C the owner's behaviour
	AAINavigator*       m_pNavigator;        // 0x20 the owner's AAINavigator (APathfindingComponent)
	TFLOAT              m_fTaskTimeOverride; // 0x24 -1.0f: none
	TFLOAT              m_fGotoAttackTimer;  // 0x28 the goal of the goto attack task is updated when it runs out
	CustomInterruptMap  m_CustomInterrupts;  // 0x2C AppendCustomInterruptToSchedule
	ABuildPathJob*      m_GroundGoalJob;     // 0x48
};

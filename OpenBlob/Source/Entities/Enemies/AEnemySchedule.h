#pragma once
#include "Entities/AI/Schedule/AAISchedule.h"
#include "Entities/AI/Schedule/AAIScheduleList.h"

#include <Math/TVector4.h>

class AEnemyBehaviour;
class AEnemy;
class AGndMotorComponent;

//-----------------------------------------------------------------------------
// Purpose: the schedules of the INKT enemies (Enemies\AEnemySchedule.cpp): spawn, attack, surround, taunt, stare,
// scout, hide, stunned, fall, grapple (ENEMYSCHEDULE_*).
//-----------------------------------------------------------------------------
class AEnemySchedule
    : public AAISchedule
{
public:
	AEnemySchedule( ABaseEntity* a_pEntity );
	virtual ~AEnemySchedule() override; // +0x00

	virtual TBOOL                             IsEnabled() override;                                                                   // +0x0C
	virtual AAIScheduleManager::ScheduleList* TranslateSchedule( TINT a_eSchedule ) override;                                         // +0x14
	virtual TBOOL                             StartSchedule( TINT a_eSchedule ) override;                                             // +0x18
	virtual TBOOL                             StartTask( TINT a_eTask ) override;                                                     // +0x20
	virtual void                              UpdateTask( TINT a_eTask, TFLOAT a_fDeltaTime ) override;                               // +0x24
	virtual void                              OnTaskFailed( TINT a_eTask ) override;                                                  // +0x28
	virtual void                              OnTaskFinished( TINT a_eTask, TBOOL a_bNoAdvance ) override;                            // +0x2C
	virtual TFLOAT                            GetAttackRecoverTime() override;                                                        // +0x30
	virtual void                              PostCreatePathBuildJob( const Toshi::TVector4& a_rvGoal ) override;                     // +0x3C
	virtual void                              HandleFailedPathBuildJob( TINT a_eTask, const Toshi::TVector4& a_rvGoal ) override;     // +0x40
	virtual void                              HandleSuccessfulPathBuildJob( TINT a_eTask, const Toshi::TVector4& a_rvGoal ) override; // +0x44

	static void InitialiseStatic();

	AEnemy*             GetEnemy() const;
	AGndMotorComponent* GetMotorComponent();

	// Grows with every failed path build (AEnemyBehaviour::SelectAttackSchedule patrols from 2 on)
	TFLOAT GetPathFailTime() const { return m_fPathFailTime; }

	static TCONSTEXPR_VAR TINT NUM_SCHEDULES = 12;

	// ENEMYSCHEDULE_Spawn = 14...ENEMYSCHEDULE_InkyPen_Attack = 25
	static AAIScheduleList<NUM_SCHEDULES> sm_EnemySchedules;

private:
	AEnemyBehaviour* m_pEnemyBehaviour; // 0x4C
	TFLOAT           m_fRepathTimer;    // 0x50 the surround point is checked again when it runs out
	TINT             m_iNumPaths;       // 0x54
	TFLOAT           m_fPathFailTime;   // 0x58
	Toshi::TVector4  m_vLastPathGoal;   // 0x5C
};

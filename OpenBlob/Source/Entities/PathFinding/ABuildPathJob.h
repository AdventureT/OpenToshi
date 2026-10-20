#pragma once
#include "JobSystem/AJob.h"
#include "AAINavigator.h"

//-----------------------------------------------------------------------------
// Purpose: builds the path of an AAINavigator (PathFinding\ABuildPathJob.cpp): from the ground under the goal, then the
// local probes, then the AI network, in one Run; the schedule that created it (AAISchedule::CreatePathBuildJob) reads
// m_eResult when it's ready.
//-----------------------------------------------------------------------------
class ABuildPathJob
    : public AJob
{
public:
	// m_eResult
	enum RESULT
	{
		RESULT_Success = 0,
		RESULT_Failed  = 1,
	};

	// m_eStage
	enum STAGE
	{
		STAGE_CheckGround = 0,
		STAGE_Local       = 1,
		STAGE_Network     = 2,
	};

public:
	ABuildPathJob();
	virtual ~ABuildPathJob() override; // +0x00

	virtual TBOOL Run() override;   // +0x0C
	virtual void  Begin() override; // +0x10

	// Sets the job up
	void Init( AAINavigator* a_pNavigator, const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, TFLOAT a_fRadius, TBOOL a_bUnk, const Toshi::TVector4* a_pFacing, AAINavigator::eGroundGoal a_eGroundGoal, AAINavigator::eDoubleBuffer a_eDoubleBuffer );

	TBOOL                  IsSuccessful() const { return m_eResult == RESULT_Success; }
	const Toshi::TVector4& GetGoal() const { return m_vGoal; }

public:
	AAINavigator*                m_pNavigator;    // 0x10
	Toshi::TVector4              m_vGoal;         // 0x14
	TFLOAT                       m_fRadius;       // 0x24
	TBOOL                        m_bUnk28;        // 0x28
	AAINavigator::eGroundGoal    m_eGroundGoal;   // 0x29
	TBOOL                        m_bHasFacing;    // 0x2A
	AAINavigator::eDoubleBuffer  m_eDoubleBuffer; // 0x2B
	Toshi::TVector4              m_vFacing;       // 0x2C
	TINT                         m_eResult;       // 0x3C RESULT
	TINT                         m_eStage;        // 0x40 STAGE
	AAINavigator::BuildPathState m_oState;        // 0x44
};

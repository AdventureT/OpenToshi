#pragma once
#include "JobSystem/AJobQueue.h"
#include "ABuildPathJob.h"

//-----------------------------------------------------------------------------
// Purpose: the queue of the AI path builds (PathFinding\ABuildPathJob.cpp, created by AEntityFramework::Startup): 256
// jobs, one run per frame (AJobQueue::Update).
//-----------------------------------------------------------------------------
class ABuildPathJobQueue
    : public AJobQueue
{
public:
	static TCONSTEXPR_VAR TINT MAX_JOBS = 256;

public:
	ABuildPathJobQueue();
	virtual ~ABuildPathJobQueue() override; // +0x00

	virtual void InitFreeLists() override; // +0x04

	ABuildPathJob* AddNewJob( AAINavigator* a_pNavigator, const Toshi::TVector4& a_rvStart, const Toshi::TVector4& a_rvGoal, TFLOAT a_fRadius, TBOOL a_bUnk, const Toshi::TVector4* a_pFacing, AAINavigator::eGroundGoal a_eGroundGoal, AAINavigator::eDoubleBuffer a_eDoubleBuffer );

	static ABuildPathJobQueue* GetBuildPathJobQueue() { return TSTATICCAST( ABuildPathJobQueue, ms_Instance ); }

private:
	ABuildPathJob m_aJobs[ MAX_JOBS ]; // 0x20
};

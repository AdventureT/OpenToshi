#pragma once
#include "AJob.h"

//-----------------------------------------------------------------------------
// Purpose: a queue of jobs run one at a time: the pending jobs are begun and run in order (Update, from AInGameState),
// the finished ones wait until their owner is done with them and go back to the free list. The instance is the AI path
// builder (ABuildPathJobQueue).
//-----------------------------------------------------------------------------
class AJobQueue
{
public:
	using JobList = Toshi::T2DList<AJob>;

public:
	AJobQueue();
	virtual ~AJobQueue(); // +0x00

	virtual void InitFreeLists() = 0; // +0x04

	void Update( TFLOAT a_fDeltaTime );

	// Gives a finished (or running) job back to the free list
	void RemoveJob( AJob* a_pJob );

	void DestroyFreeLists();

	// $deBlob: FUNCTION 005c2ef0
	static AJobQueue* GetInstance() { return ms_Instance; }

protected:
	static AJobQueue* ms_Instance;

protected:
	JobList m_FreeJobs;    // 0x04
	JobList m_PendingJobs; // 0x0C
	JobList m_DoneJobs;    // 0x14 finished, until the owner calls SetDone
	AJob*   m_pCurrentJob; // 0x1C
};

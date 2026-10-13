#pragma once
#include "TNodeTree.h"
#include "THPTimer.h"
#include "TSystem.h"
#include "TTask.h"

TOSHI_NAMESPACE_START

class TTask;

//-----------------------------------------------------------------------------
// Purpose: owns and updates the task tree.
// NOTE: de Blob's TScheduler is not a TObject (its vtable only has the destructor), keeps its own timer and is
// updated by TSystemManager::Update. Barnyard's delta time calculator doesn't exist in this branch.
//-----------------------------------------------------------------------------
class TScheduler
{
public:
	TScheduler();
	virtual ~TScheduler();

	TTask* CreateTask( TClass* a_pClass, TTask* a_pParent = TNULL );

	template <class T>
	T* CreateTask( TTask* a_pParent = TNULL )
	{
		return TSTATICCAST( T, CreateTask( &TGetClass( T ), a_pParent ) );
	}

	// Called by TSystemManager::Update (inlined there in retail)
	void Update( TFLOAT a_fDeltaTime );

	// Marks the task and all its children as dying, they get destroyed on the next update
	static void KillTask( TTask* a_pTask );

	void DestroyAllTasks();

	void SetDebugSlowTime( TBOOL a_bSlowTime );
	void SetDebugDeltaTime( TBOOL a_bUseDebugDeltaTime, TFLOAT a_fDebugDeltaTime );
	void SetDebugDeltaTimeMult( TBOOL a_bUseDebugDeltaTimeMult, TFLOAT a_fDebugDeltaTimeMult );
	void SetDebugPause( TBOOL a_bPause );
	void SetFixedFps( TBOOL a_bFixedFps, TFLOAT a_fFixedFps );

	// NOTE: the name is unknown, TTRUE if the debug delta time or the debug slow time is used
	TBOOL IsDebugTime() const;

	TFLOAT GetDebugDeltaTime() const;
	TBOOL  IsFixedFps() const;

	TNodeTree<TTask>* GetTree();

	TFLOAT GetCurrentDeltaTime() const { return m_fDeltaTime; }
	TFLOAT GetFrameTime() const { return m_fFrameTime; }
	TFLOAT GetTotalTime() const { return m_fTotalTime; }
	TUINT  GetFrameCount() const { return m_uiNumFrames; }

	TFLOAT GetDebugDeltaTimeMult() const { return m_fDebugDeltaTimeMult; }
	TBOOL  IsDebugDeltaTimeMultUsed() const { return m_bUseDebugDeltaTimeMult; }

	// Destroys a dying task right now
	void DestroyTask( TTask* a_pTask );

	// NOTE: the printing has been compiled out of retail
	static void DumpTaskTree( TTask* a_pTask, TINT a_iDepth );

private:
	static void DestroyTaskRecurse( TTask* a_pTask );
	void        DeleteTaskAtomic( TTask* a_pTask );
	void        DeleteTaskRecurse( TTask* a_pTask );

	// Destroys all the dying tasks from the last one to the first one
	void DestroyDyingTasks( TTask* a_pTask );

	// Updates all the active tasks from the first one to the last one
	void UpdateActiveTasks( TTask* a_pTask );

private:
	static TFLOAT s_DebugSlowMaxTimeDeltaAllowed;
	static TFLOAT s_MaxTimeDeltaAllowed;

private:
	TUINT32          m_Unk1;                   // 0x04
	TNodeTree<TTask> m_TaskTree;               // 0x08
	TFLOAT           m_fDeltaTime;             // 0x20
	TFLOAT           m_fFrameTime;             // 0x24 time the last update took
	TFLOAT           m_fTotalTime;             // 0x28
	TFLOAT           m_fMaxDeltaTime;          // 0x2C
	TUINT            m_uiNumFrames;            // 0x30
	THPTimer         m_oTimer;                 // 0x38
	TBOOL            m_bUseDebugDeltaTime;     // 0x60
	TFLOAT           m_fDebugDeltaTime;        // 0x64
	TFLOAT           m_fPausedMaxDeltaTime;    // 0x68
	TBOOL            m_bDebugPause;            // 0x6C
	TBOOL            m_bUseDebugDeltaTimeMult; // 0x6D
	TFLOAT           m_fDebugDeltaTimeMult;    // 0x70
	TBOOL            m_bFixedFps;              // 0x74
	TFLOAT           m_fFixedFrameTime;        // 0x78
}; // 0x80

TOSHI_NAMESPACE_END

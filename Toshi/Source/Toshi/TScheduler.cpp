#include "ToshiPCH.h"
#include "TScheduler.h"
#include "TSystem.h"
#include "TTask.h"

#include <windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TFLOAT TScheduler::s_MaxTimeDeltaAllowed          = 0.25f;
TFLOAT TScheduler::s_DebugSlowMaxTimeDeltaAllowed = 0.005f;

// $deBlob: FUNCTION 006603d0
// $Barnyard: FUNCTION 006bccd0
TScheduler::TScheduler()
    : m_fDeltaTime( 0.0f )
    , m_fFrameTime( 0.0f )
    , m_fTotalTime( 0.0f )
    , m_fMaxDeltaTime( 0.25f )
    , m_uiNumFrames( 0 )
{
	m_fPausedMaxDeltaTime    = 0.0f;
	m_bDebugPause            = TFALSE;
	m_bUseDebugDeltaTimeMult = TFALSE;
	m_bFixedFps              = TFALSE;
	m_fFixedFrameTime        = 0.0f;
	m_bUseDebugDeltaTime     = TFALSE;
	m_fDebugDeltaTime        = 0.01f;
	m_fDebugDeltaTimeMult    = 1.0f;
}

// $deBlob: FUNCTION 00660dd0
// $deBlob: FUNCTION 00660460
// $Barnyard: FUNCTION 006bcdf0
// $Barnyard: FUNCTION 006bce40
TScheduler::~TScheduler()
{
	DestroyAllTasks();
}

// $deBlob: FUNCTION 00660580
// $Barnyard: FUNCTION 006bcbf0
TTask* TScheduler::CreateTask( TClass* a_rTaskClass, TTask* a_pParent )
{
	TClass& a_rTaskClassRef = *a_rTaskClass;
	TASSERT( a_rTaskClassRef.IsA( &TGetClass( TTask ) ) == TTRUE );

	TTask* pTask = static_cast<TTask*>( a_rTaskClass->CreateObject() );
	TASSERT( pTask != TNULL );

	m_TaskTree.InsertAtRootHead( pTask );

	if ( a_pParent != TNULL )
	{
		m_TaskTree.Remove( pTask, TFALSE );
		m_TaskTree.Insert( a_pParent, pTask );
	}

	return pTask;
}

void TScheduler::Update( TFLOAT a_fDeltaTime )
{
	// The scheduler part of TSystemManager::Update
	m_uiNumFrames += 1;
	m_fDeltaTime = a_fDeltaTime;
	m_fTotalTime += a_fDeltaTime;

	if ( a_fDeltaTime >= 0.0f )
	{
		if ( a_fDeltaTime > m_fMaxDeltaTime )
			m_fDeltaTime = m_fMaxDeltaTime;
	}
	else
	{
		m_fDeltaTime = 0.0f;
	}

	if ( m_bUseDebugDeltaTime )
		m_fDeltaTime = m_fDebugDeltaTime;

	if ( m_bUseDebugDeltaTimeMult )
		m_fDeltaTime *= m_fDebugDeltaTimeMult;

	m_oTimer.Update();
	DestroyDyingTasks( m_TaskTree.ChildOfRoot() );
	UpdateActiveTasks( m_TaskTree.ChildOfRoot() );
	m_oTimer.Update();

	m_fFrameTime = m_oTimer.GetDelta();

	if ( m_bFixedFps && m_fFixedFrameTime > m_fFrameTime )
		Sleep( TINT( ( m_fFixedFrameTime - m_fFrameTime ) * 1000.0f ) );
}

// $deBlob: FUNCTION 00660640
void TScheduler::KillTask( TTask* a_pTask )
{
	if ( !( a_pTask->m_ulFlags & TTask::FLAGS_DYING ) )
	{
		a_pTask->m_ulFlags |= TTask::FLAGS_DYING;
		DestroyTaskRecurse( a_pTask->Child() );
	}
}

// $deBlob: FUNCTION 00660690
// $Barnyard: FUNCTION 006bcd20
void TScheduler::DestroyAllTasks()
{
	if ( m_TaskTree.ChildOfRoot() != TNULL )
	{
		DestroyTaskRecurse( m_TaskTree.ChildOfRoot() );
		DestroyDyingTasks( m_TaskTree.ChildOfRoot() );
	}
}

// $deBlob: FUNCTION 006606d0
// $Barnyard: FUNCTION 006bc9c0
void TScheduler::SetDebugSlowTime( TBOOL a_bSlowTime )
{
	m_fMaxDeltaTime = a_bSlowTime ? s_DebugSlowMaxTimeDeltaAllowed : s_MaxTimeDeltaAllowed;
}

// $deBlob: FUNCTION 00660700
// $Barnyard: FUNCTION 006bc9e0
void TScheduler::SetDebugDeltaTime( TBOOL a_bUseDebugDeltaTime, TFLOAT a_fDebugDeltaTime )
{
	m_bUseDebugDeltaTime = a_bUseDebugDeltaTime;
	m_fDebugDeltaTime    = a_fDebugDeltaTime;
}

// $deBlob: FUNCTION 00660720
// $Barnyard: FUNCTION 006bca00
void TScheduler::SetDebugDeltaTimeMult( TBOOL a_bUseDebugDeltaTimeMult, TFLOAT a_fDebugDeltaTimeMult )
{
	m_bUseDebugDeltaTimeMult = a_bUseDebugDeltaTimeMult;
	m_fDebugDeltaTimeMult    = a_fDebugDeltaTimeMult;
	TMath::Clip( m_fDebugDeltaTimeMult, 0.0f, 100.0f );
}

// NOTE: de Blob pauses by setting the max delta time to 0 (Barnyard used the debug delta time)
// $deBlob: FUNCTION 00660760
void TScheduler::SetDebugPause( TBOOL a_bPause )
{
	if ( a_bPause )
	{
		if ( !m_bDebugPause )
		{
			m_fPausedMaxDeltaTime = m_fMaxDeltaTime;
			m_fMaxDeltaTime       = 0.0f;
		}
	}
	else if ( m_bDebugPause )
	{
		m_fMaxDeltaTime = m_fPausedMaxDeltaTime;
	}

	m_bDebugPause = a_bPause;
}

// $deBlob: FUNCTION 006607a0
void TScheduler::SetFixedFps( TBOOL a_bFixedFps, TFLOAT a_fFixedFps )
{
	if ( a_bFixedFps )
	{
		TASSERT( a_fFixedFps > 0.0f );
		m_fFixedFrameTime = 1.0f / a_fFixedFps;
	}

	m_bFixedFps = a_bFixedFps;
}

// $deBlob: FUNCTION 00660810
TBOOL TScheduler::IsDebugTime() const
{
	return m_bUseDebugDeltaTime || m_fMaxDeltaTime == s_DebugSlowMaxTimeDeltaAllowed;
}

// $deBlob: FUNCTION 00660830
TFLOAT TScheduler::GetDebugDeltaTime() const
{
	return m_fDebugDeltaTime;
}

// $deBlob: FUNCTION 00660840
TBOOL TScheduler::IsFixedFps() const
{
	return m_bFixedFps;
}

// $deBlob: FUNCTION 00660860
TNodeTree<TTask>* TScheduler::GetTree()
{
	return &m_TaskTree;
}

// $deBlob: FUNCTION 006608d0
void TScheduler::DestroyTask( TTask* a_pTask )
{
	TASSERT( a_pTask->m_ulFlags & TTask::FLAGS_DYING );
	a_pTask->OnPreDestroy();
	DeleteTaskRecurse( a_pTask->Child() );
	DeleteTaskAtomic( a_pTask );
}

// $deBlob: FUNCTION 006609a0
// $Barnyard: FUNCTION 006bca80
inline void TScheduler::DestroyTaskRecurse( TTask* a_pTask )
{
	TTask* pStart = a_pTask;

	if ( a_pTask == TNULL )
		return;

	do
	{
		a_pTask->m_ulFlags |= TTask::FLAGS_DYING;

		if ( a_pTask->Child() != TNULL )
			DestroyTaskRecurse( a_pTask->Child() );

		a_pTask = a_pTask->Next();
	} while ( a_pTask != pStart && a_pTask != TNULL );
}

// $deBlob: FUNCTION 00660b00
// $Barnyard: FUNCTION 006bcb40
void TScheduler::DeleteTaskAtomic( TTask* a_pTask )
{
	if ( a_pTask != TNULL )
	{
		TTask* pParent = TNULL;

		if ( a_pTask->IsChildOfDefaultRoot() == TFALSE )
		{
			pParent = a_pTask->Parent();

			if ( pParent->IsDying() == TFALSE && pParent->OnChildDying( a_pTask ) == TFALSE )
			{
				// The parent is not dying and doesn't allow us to kill the task
				return;
			}
		}

		if ( a_pTask->Child() != TNULL )
			DeleteTaskRecurse( a_pTask->Child() );

		const TClass* pClass = a_pTask->GetClass();
		a_pTask->OnDestroy();
		m_TaskTree.Remove( a_pTask, TFALSE );

		// NOTE: de Blob calls the deleting destructor (Barnyard: Delete())
		delete a_pTask;

		if ( pParent != TNULL )
			pParent->OnChildDied( pClass, a_pTask );
	}
}

// NOTE: retail inlines DeleteTaskAtomic here
// $deBlob: FUNCTION 006609e0
// $Barnyard: FUNCTION 006bcac0
void TScheduler::DeleteTaskRecurse( TTask* a_pTask )
{
	if ( a_pTask != TNULL )
	{
		TTask* pTask = a_pTask->Prev();

		while ( pTask != TNULL )
		{
			// Deleting the tasks removes them from the list, stop once the last one is deleted
			TTask* pNextTask = ( pTask->Prev() != pTask ) ? pTask->Prev() : TNULL;
			DeleteTaskAtomic( pTask );
			pTask = pNextTask;
		}
	}
}

// $deBlob: FUNCTION 00660870
// $Barnyard: FUNCTION 006bcc80
void TScheduler::DestroyDyingTasks( TTask* a_pTask )
{
	if ( a_pTask != TNULL )
	{
		TTask* pTask = a_pTask->Prev();

		while ( pTask != TNULL )
		{
			// Go from the last task to the first one (a_pTask)
			TTask* pNextTask = ( pTask != a_pTask ) ? pTask->Prev() : TNULL;

			if ( !pTask->IsDying() )
			{
				if ( pTask->Child() != TNULL )
					DestroyDyingTasks( pTask->Child() );
			}
			else
			{
				pTask->OnPreDestroy();
				DeleteTaskRecurse( pTask->Child() );
				DeleteTaskAtomic( pTask );
			}

			pTask = pNextTask;
		}
	}
}

// $deBlob: FUNCTION 00660930
// $Barnyard: FUNCTION 006bca20
void TScheduler::UpdateActiveTasks( TTask* a_pTask )
{
	TTask* pTask = a_pTask;

	while ( pTask != TNULL )
	{
		TTask* pNextTask = ( pTask->Next() != a_pTask ) ? pTask->Next() : TNULL;

		TBOOL bRecurse = TTRUE;
		if ( pTask->IsCreated() == TTRUE && pTask->IsActive() )
			bRecurse = pTask->OnUpdate( m_fDeltaTime );

		if ( pTask->Child() != TNULL && bRecurse == TTRUE )
			UpdateActiveTasks( pTask->Child() );

		pTask = pNextTask;
	}
}

// $deBlob: FUNCTION 00660c10
void TScheduler::DumpTaskTree( TTask* a_pTask, TINT a_iDepth )
{
	TTask* pTask = a_pTask;

	while ( pTask != TNULL )
	{
		TTask* pNextTask = ( pTask->Next() != a_pTask ) ? pTask->Next() : TNULL;

		if ( pTask->Child() != TNULL )
			DumpTaskTree( pTask->Child(), a_iDepth + 1 );

		pTask = pNextTask;
	}
}

TOSHI_NAMESPACE_END

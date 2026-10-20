#include "pch.h"
#include "AJobQueue.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AJobQueue* AJobQueue::ms_Instance;

// $deBlob: FUNCTION 005e1670
AJobQueue::AJobQueue()
{
	m_pCurrentJob = TNULL;

	TASSERT( 0 == ms_Instance );
	ms_Instance = this;
}

// $deBlob: FUNCTION 005e1720
// $deBlob: FUNCTION 005e1b20
AJobQueue::~AJobQueue()
{
	TASSERT( this == ms_Instance );
	ms_Instance = TNULL;
}

// $deBlob: FUNCTION 005e1910
void AJobQueue::Update( TFLOAT a_fDeltaTime )
{
	if ( m_pCurrentJob == TNULL && !m_PendingJobs.IsEmpty() )
	{
		m_pCurrentJob = m_PendingJobs.PopFront();
		m_pCurrentJob->Begin();
	}

	if ( m_pCurrentJob != TNULL && m_pCurrentJob->Run() )
	{
		m_pCurrentJob->SetReady();
		m_DoneJobs.PushBack( m_pCurrentJob );
		m_pCurrentJob = TNULL;
	}

	// The jobs their owner is done with are free again
	for ( auto it = m_DoneJobs.Begin(); it != m_DoneJobs.End(); )
	{
		AJob* pJob = it++;

		if ( pJob->IsDone() )
		{
			pJob->Remove();
			pJob->ClearFlags();
			m_FreeJobs.PushBack( pJob );
		}
	}
}

// $deBlob: FUNCTION 005e17d0
void AJobQueue::RemoveJob( AJob* a_pJob )
{
	if ( m_pCurrentJob == a_pJob )
	{
		m_pCurrentJob->ClearFlags();
		m_FreeJobs.PushBack( m_pCurrentJob );
		m_pCurrentJob = TNULL;
		return;
	}

	for ( auto it = m_DoneJobs.Begin(); it != m_DoneJobs.End(); )
	{
		AJob* pJob = it++;

		if ( pJob == a_pJob )
		{
			pJob->Remove();
			pJob->ClearFlags();
			m_FreeJobs.PushBack( pJob );
			return;
		}
	}
}

// $deBlob: FUNCTION 005e1a50
void AJobQueue::DestroyFreeLists()
{
	m_FreeJobs.Clear();
	m_PendingJobs.Clear();
	m_DoneJobs.Clear();
}

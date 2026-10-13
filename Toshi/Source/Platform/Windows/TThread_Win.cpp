#include "ToshiPCH.h"
#include "TThread_Win.h"

#include <process.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

// NOTE: retail's Toshi/TThread_Win.cpp also holds TSemaphore, TMutex and TGenericFifo (see their files)

TOSHI_NAMESPACE_START

static TThreadManager oThreadManager;

// $deBlob: FUNCTION 006622e0
// $Barnyard: FUNCTION 006bb9a0
TBOOL TThread::Create( TSIZE a_iStackSize, PRIORITY a_ePriority, FLAGS a_eFlags )
{
	m_iThreadID  = -1;
	m_hThreadHnd = NULL;
	m_hThreadHnd = (void*)_beginthreadex( NULL, TUINT( a_iStackSize ), Entry, this, CREATE_SUSPENDED, (unsigned int*)&m_iThreadID );
	TASSERT( m_hThreadHnd != NULL );

	BOOL bResult = SetThreadPriority( m_hThreadHnd, a_ePriority );
	TASSERT( bResult != FALSE );

	TThreadManager::GetThreadManager()->InsertThread( this );

	if ( ( a_eFlags & FLAGS_SUSPENDED ) == 0 )
	{
		DWORD iResult = ResumeThread( m_hThreadHnd );
		TASSERT( iResult != 0xffffffff );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006623e0
// $Barnyard: FUNCTION 006bb8b0
TBOOL TThread::Destroy()
{
	if ( m_hThreadHnd == NULL )
		return TFALSE;

	TASSERT( m_iThreadID != GetCurrentThreadId() );

	BOOL bResult = TerminateThread( m_hThreadHnd, 0 );
	TASSERT( bResult != FALSE );

	bResult = CloseHandle( m_hThreadHnd );
	TASSERT( bResult != FALSE );

	TThreadManager::GetThreadManager()->RemoveThread( this );
	m_hThreadHnd = NULL;
	m_iThreadID  = -1;

	return TTRUE;
}

// $deBlob: FUNCTION 00662640
// $Barnyard: FUNCTION 006bb980
unsigned int __stdcall TThread::Entry( void* a_pThread )
{
	TThread* pThread = TSTATICCAST( TThread, a_pThread );
	pThread->Main();
	Exit( pThread );

	return 0;
}

// $deBlob: FUNCTION 00662660
// $Barnyard: FUNCTION 006bb920
void TThread::Exit( TThread* a_pThread )
{
	TASSERT( a_pThread->m_iThreadID == GetCurrentThreadId() );

	BOOL bResult = CloseHandle( a_pThread->m_hThreadHnd );
	TASSERT( bResult != FALSE );

	TThreadManager::GetThreadManager()->RemoveThread( a_pThread );
	a_pThread->m_hThreadHnd = NULL;
	a_pThread->m_iThreadID  = -1;

	_endthreadex( 0 );
}

// $deBlob: FUNCTION 006625d0
void TThread::Exit()
{
	Exit( TThreadManager::GetThreadManager()->FindThread( GetCurrentThreadId() ) );
}

// $deBlob: FUNCTION 006625f0
TBOOL TThread::Yield()
{
	::Sleep( 0 );
	return TTRUE;
}

// $deBlob: FUNCTION 00662580
void TThread::Sleep( TINT a_iMilliseconds )
{
	::Sleep( a_iMilliseconds );
}

// $deBlob: FUNCTION 006624e0
TBOOL TThread::Suspend()
{
	TASSERT( m_hThreadHnd != NULL );
	return SuspendThread( m_hThreadHnd ) != 0xffffffff;
}

// $deBlob: FUNCTION 00662530
TBOOL TThread::Resume()
{
	TASSERT( m_hThreadHnd != NULL );
	return ResumeThread( m_hThreadHnd ) != 0xffffffff;
}

// $deBlob: FUNCTION 00662590
TBOOL TThread::SetPriority( PRIORITY a_ePriority )
{
	return SetPriority( m_hThreadHnd, a_ePriority );
}

// $deBlob: FUNCTION 006625b0
TBOOL TThread::GetPriority( PRIORITY& a_rePriority )
{
	return GetPriority( m_hThreadHnd, a_rePriority );
}

// $deBlob: FUNCTION 00662600
TBOOL TThread::SetCurrentPriority( PRIORITY a_ePriority )
{
	return SetPriority( GetCurrentThread(), a_ePriority );
}

// $deBlob: FUNCTION 00662620
TBOOL TThread::GetCurrentPriority( PRIORITY& a_rePriority )
{
	return GetPriority( GetCurrentThread(), a_rePriority );
}

// $deBlob: FUNCTION 00662730
TBOOL TThread::SetPriority( void* a_hThreadHnd, PRIORITY a_ePriority )
{
	TASSERT( a_hThreadHnd != NULL );

	BOOL bResult = SetThreadPriority( a_hThreadHnd, a_ePriority );
	TASSERT( bResult != FALSE );

	return TTRUE;
}

// $deBlob: FUNCTION 006627b0
TBOOL TThread::GetPriority( void* a_hThreadHnd, PRIORITY& a_rePriority )
{
	TASSERT( a_hThreadHnd != NULL );

	TINT iPriority = GetThreadPriority( a_hThreadHnd );
	TASSERT( iPriority != THREAD_PRIORITY_ERROR_RETURN );

	a_rePriority = iPriority;
	return TTRUE;
}

// $deBlob: FUNCTION 00662a30
TThreadManager::TThreadManager()
{
	Create();
}

// $deBlob: FUNCTION 00662ab0
TThreadManager::~TThreadManager()
{
	Destroy();
}

// $deBlob: FUNCTION 00662840
void TThreadManager::Create()
{
	TASSERT( s_pThreadManager == TNULL );
	s_pThreadManager = this;
	InitializeCriticalSection( &m_CriticalSection );
}

// $deBlob: FUNCTION 00662890
void TThreadManager::Destroy()
{
	TASSERT( s_pThreadManager != TNULL );

	if ( s_pThreadManager )
	{
		s_pThreadManager = TNULL;
		DeleteCriticalSection( &m_CriticalSection );
	}
}

// $deBlob: FUNCTION 006628f0
void TThreadManager::InsertThread( TThread* a_pThread )
{
	EnterCriticalSection( &m_CriticalSection );
	m_Threads.InsertHead( a_pThread );
	LeaveCriticalSection( &m_CriticalSection );
}

// $deBlob: FUNCTION 00662960
void TThreadManager::RemoveThread( TThread* a_pThread )
{
	EnterCriticalSection( &m_CriticalSection );
	a_pThread->Remove();
	LeaveCriticalSection( &m_CriticalSection );
}

// $deBlob: FUNCTION 006629a0
TThread* TThreadManager::FindThread( unsigned long a_iThreadID )
{
	TThread* pResult = TNULL;
	EnterCriticalSection( &m_CriticalSection );

	for ( auto it = m_Threads.Begin(); it != m_Threads.End(); it++ )
	{
		if ( it->m_iThreadID == a_iThreadID )
		{
			pResult = it;
			break;
		}
	}

	LeaveCriticalSection( &m_CriticalSection );
	return pResult;
}

TOSHI_NAMESPACE_END

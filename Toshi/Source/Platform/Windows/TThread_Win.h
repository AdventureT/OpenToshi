#pragma once

#include <Windows.h>

#undef THREAD_MODE_BACKGROUND_BEGIN
#undef THREAD_MODE_BACKGROUND_END
#undef THREAD_PRIORITY_ABOVE_NORMAL
#undef THREAD_PRIORITY_BELOW_NORMAL
#undef THREAD_PRIORITY_HIGHEST
#undef THREAD_PRIORITY_IDLE
#undef THREAD_PRIORITY_LOWEST
#undef THREAD_PRIORITY_NORMAL
#undef THREAD_PRIORITY_TIME_CRITICAL
#undef Yield

#include "Toshi/TDList.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a Win32 thread (_beginthreadex) running Main(); every thread is linked into the thread manager.
// NOTE: in de Blob's branch TThread/TThreadManager use TDList (Barnyard: T2DList) and the manager is a static object
// with s_pThreadManager (Barnyard: a TSingleton).
//-----------------------------------------------------------------------------
class TThread : public TDList<TThread>::TNode
{
public:
	typedef TINT PRIORITY;
	enum PRIORITY_ : PRIORITY
	{
		THREAD_MODE_BACKGROUND_BEGIN  = 0x00010000,
		THREAD_MODE_BACKGROUND_END    = 0x00020000,
		THREAD_PRIORITY_ABOVE_NORMAL  = 1,
		THREAD_PRIORITY_BELOW_NORMAL  = -1,
		THREAD_PRIORITY_HIGHEST       = 2,
		THREAD_PRIORITY_IDLE          = -15,
		THREAD_PRIORITY_LOWEST        = -2,
		THREAD_PRIORITY_NORMAL        = 0,
		THREAD_PRIORITY_TIME_CRITICAL = 15
	};

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE      = 0,
		FLAGS_SUSPENDED = BITFLAG( 0 ), // don't resume the thread after creating it
	};

public:
	virtual void Main() = 0;
	virtual ~TThread() {}

	TBOOL Create( TSIZE a_iStackSize, PRIORITY a_ePriority, FLAGS a_eFlags );
	TBOOL Destroy();

	TBOOL Suspend();
	TBOOL Resume();

	TBOOL SetPriority( PRIORITY a_ePriority );
	TBOOL GetPriority( PRIORITY& a_rePriority );

	TBOOL IsCreated() const { return m_hThreadHnd != NULL; }

	static unsigned int __stdcall Entry( void* a_pThread );

	// Closes a_pThread (must be the current thread) and ends it
	static void Exit( TThread* a_pThread );

	// Ends the current thread
	static void Exit();

	static TBOOL Yield();
	static void  Sleep( TINT a_iMilliseconds );

	static TBOOL SetCurrentPriority( PRIORITY a_ePriority );
	static TBOOL GetCurrentPriority( PRIORITY& a_rePriority );

	static TBOOL SetPriority( void* a_hThreadHnd, PRIORITY a_ePriority );
	static TBOOL GetPriority( void* a_hThreadHnd, PRIORITY& a_rePriority );

public:
	void*         m_hThreadHnd = NULL; // 0x0C
	unsigned long m_iThreadID  = -1;   // 0x10
}; // 0x14

class TThreadManager
{
public:
	TThreadManager();
	~TThreadManager();

	void Create();
	void Destroy();

	void     InsertThread( TThread* a_pThread );
	void     RemoveThread( TThread* a_pThread );
	TThread* FindThread( unsigned long a_iThreadID );

	// $deBlob: FUNCTION 00662b50
	static TThreadManager* GetThreadManager() { return s_pThreadManager; }

private:
	inline static TThreadManager* s_pThreadManager;

private:
	CRITICAL_SECTION m_CriticalSection; // 0x00
	TDList<TThread>  m_Threads;         // 0x18
}; // 0x20

TOSHI_NAMESPACE_END

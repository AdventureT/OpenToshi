#include "ToshiPCH.h"
#include "TMutex.h"

#ifdef USE_WIN32_MUTEX
#  include <windows.h>
#endif // USE_WIN32_MUTEX

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TMutex::TMutex()
{
}

// NOTE: like retail, the destructor doesn't destroy the mutex (TMutex is created and destroyed explicitly)
TMutex::~TMutex()
{
}

// $deBlob: FUNCTION 00661d10
// $Barnyard: FUNCTION 006bb660
TBOOL TMutex::Create()
{
#ifdef USE_WIN32_MUTEX
	m_Handle = CreateMutexA( NULL, FALSE, NULL );
	TASSERT( m_Handle != NULL );
#endif // !USE_WIN32_MUTEX
	return TTRUE;
}

// $deBlob: FUNCTION 00661d60
// $Barnyard: FUNCTION 006bb680
TBOOL TMutex::Destroy()
{
#ifdef USE_WIN32_MUTEX
	TASSERT( m_Handle != NULL );
	BOOL result = CloseHandle( m_Handle );
	m_Handle    = NULL;
	return result != 0;
#else  // USE_WIN32_MUTEX
	return TTRUE;
#endif // !USE_WIN32_MUTEX
}

TMutex& TMutex::operator=( const TMutex& mutex )
{
#ifdef USE_WIN32_MUTEX
	m_Handle = mutex.m_Handle;
#endif // !USE_WIN32_MUTEX
	return *this;
}

TMutex& TMutex::operator=( void* handle )
{
#ifdef USE_WIN32_MUTEX
	m_Handle = handle;
#endif // !USE_WIN32_MUTEX
	return *this;
}

// $deBlob: FUNCTION 00661e10
// $Barnyard: FUNCTION 006bb6e0
TBOOL TMutex::Unlock()
{
#ifdef USE_WIN32_MUTEX
	TASSERT( m_Handle != NULL );
	return ReleaseMutex( m_Handle ) != 0;
#else  // !USE_WIN32_MUTEX
	m_Atomic.Unlock();
	return TTRUE;
#endif // USE_WIN32_MUTEX
}

// $deBlob: FUNCTION 00661db0
// $Barnyard: FUNCTION 006bb6a0
TBOOL TMutex::Lock( FLAG a_eFlags /*= FLAG_NONE */ )
{
#ifdef USE_WIN32_MUTEX
	TASSERT( m_Handle != NULL );
	DWORD result;
	if ( a_eFlags & FLAG_NO_WAIT )
		result = WaitForSingleObject( m_Handle, 0 );
	else
		result = WaitForSingleObject( m_Handle, INFINITE );

	return result == WAIT_OBJECT_0;
#else  // !USE_WIN32_MUTEX
	if ( a_eFlags & FLAG_NO_WAIT )
		return m_Atomic.TryLock();

	m_Atomic.Lock();
	return TTRUE;
#endif // USE_WIN32_MUTEX
}

TOSHI_NAMESPACE_END

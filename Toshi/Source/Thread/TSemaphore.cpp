#include "ToshiPCH.h"
#include "TSemaphore.h"

#if defined( USE_WIN32_SEMAPHORE )
#  include <windows.h>
#endif // USE_WIN32_SEMAPHORE

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

// NOTE: in retail the Win32 versions are in Toshi/TThread_Win.cpp

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00661b10
TBOOL TSemaphore::Create( TINT a_iInitCount, TINT a_iMaxCount )
{
	TASSERT( a_iInitCount >= 0 );
	TASSERT( a_iMaxCount >= a_iInitCount );

#if !defined( USE_WIN32_SEMAPHORE )
	m_iCounter = a_iInitCount;
#else  // !USE_WIN32_SEMAPHORE
	m_hSemaHnd = CreateSemaphoreA( NULL, a_iInitCount, a_iMaxCount, NULL );
	TASSERT( m_hSemaHnd != NULL );
#endif // USE_WIN32_SEMAPHORE

	return TTRUE;
}

// $deBlob: FUNCTION 00661bd0
TBOOL TSemaphore::Destroy()
{
#if !defined( USE_WIN32_SEMAPHORE )
	return TTRUE;
#else  // !USE_WIN32_SEMAPHORE
	TASSERT( m_hSemaHnd != NULL );
	BOOL bResult = CloseHandle( m_hSemaHnd );
	m_hSemaHnd   = NULL;

	return bResult != 0;
#endif // USE_WIN32_SEMAPHORE
}

// $deBlob: FUNCTION 00661c20
// $Barnyard: FUNCTION 006bb640
TBOOL TSemaphore::Signal()
{
#if !defined( USE_WIN32_SEMAPHORE )
	m_iCounter += 1;
	m_iCounter.Signal();
	return TTRUE;
#else  // !USE_WIN32_SEMAPHORE
	TASSERT( m_hSemaHnd != NULL );
	return ReleaseSemaphore( m_hSemaHnd, 1, NULL ) != 0;
#endif // USE_WIN32_SEMAPHORE
}

// $deBlob: FUNCTION 00661c70
TBOOL TSemaphore::Wait()
{
#if !defined( USE_WIN32_SEMAPHORE )
	while ( TTRUE )
	{
		// Wait until the counter is non-zero
		m_iCounter.WaitForChange( 0 );

		// Decrement the counter
		TINT32 iState = m_iCounter.Add( -1 ) - 1;

		// See if it actually had some non-zero value
		if ( iState >= 0 )
			return TTRUE;

		// It had zero value, increment it back and wait until it's free to use again
		iState = m_iCounter.Add( 1 ) + 1;
	}
#else  // !USE_WIN32_SEMAPHORE
	TASSERT( m_hSemaHnd != NULL );
	return WaitForSingleObject( m_hSemaHnd, INFINITE ) == WAIT_OBJECT_0;
#endif // USE_WIN32_SEMAPHORE
}

// $deBlob: FUNCTION 00661cc0
TBOOL TSemaphore::Poll()
{
#if !defined( USE_WIN32_SEMAPHORE )
	if ( m_iCounter.Load() > 0 )
	{
		// Decrement the counter
		TINT32 iState = m_iCounter.Add( -1 ) - 1;

		// See if it actually had some non-zero value
		if ( iState >= 0 )
			return TTRUE;

		// It had zero value, increment it back and wait until it's free to use again
		iState = m_iCounter.Add( 1 ) + 1;
	}

	return TFALSE;
#else  // !USE_WIN32_SEMAPHORE
	TASSERT( m_hSemaHnd != NULL );
	return WaitForSingleObject( m_hSemaHnd, 0 ) == WAIT_OBJECT_0;
#endif // USE_WIN32_SEMAPHORE
}

TOSHI_NAMESPACE_END

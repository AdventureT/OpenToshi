#include "ToshiPCH.h"
#include "T2Mutex_Win.h"

#include <windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

#undef CreateMutex

// $deBlob: FUNCTION 006fc960
TBOOL T2Mutex_Create( T2MutexHandle* a_pMutex )
{
	*a_pMutex = CreateMutexA( NULL, FALSE, NULL );
	TASSERT( *a_pMutex != NULL );
	return TTRUE;
}

// $deBlob: FUNCTION 006fc9b0
TBOOL T2Mutex_Destroy( T2MutexHandle* a_pMutex )
{
	TASSERT( *a_pMutex != NULL );
	TBOOL bResult = CloseHandle( *a_pMutex );
	*a_pMutex     = NULL;
	return bResult;
}

// $deBlob: FUNCTION 006fca10
TBOOL T2Mutex_Lock( T2MutexHandle* a_pMutex, TUINT32 a_uiFlags )
{
	TASSERT( *a_pMutex != NULL );

	DWORD dwResult = ( a_uiFlags & T2MUTEX_LOCK_NOWAIT ) ?
	    WaitForSingleObject( *a_pMutex, 0 ) :
	    WaitForSingleObject( *a_pMutex, INFINITE );

	return dwResult == WAIT_OBJECT_0;
}

// $deBlob: FUNCTION 006fca70
TBOOL T2Mutex_Unlock( T2MutexHandle* a_pMutex )
{
	TASSERT( *a_pMutex != NULL );
	return ReleaseMutex( *a_pMutex );
}

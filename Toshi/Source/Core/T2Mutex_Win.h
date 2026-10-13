#pragma once
#include "Toshi/Typedefs.h"

//-----------------------------------------------------------------------------
// Purpose: plain C mutex of de Blob's Core library (Source\Core\T2Mutex_Win.c), used by the memory system.
// NOTE: retail calls the type T2Mutex (a HANDLE); it is named T2MutexHandle here so it doesn't clash with the C++
// Toshi::T2Mutex class.
//-----------------------------------------------------------------------------
typedef void* T2MutexHandle;

enum T2MutexLockFlags_ : TUINT32
{
	T2MUTEX_LOCK_NOWAIT = BITFLAG( 0 ), // don't block, only try to acquire the mutex
};

TBOOL T2Mutex_Create( T2MutexHandle* a_pMutex );
TBOOL T2Mutex_Destroy( T2MutexHandle* a_pMutex );
TBOOL T2Mutex_Lock( T2MutexHandle* a_pMutex, TUINT32 a_uiFlags );
TBOOL T2Mutex_Unlock( T2MutexHandle* a_pMutex );

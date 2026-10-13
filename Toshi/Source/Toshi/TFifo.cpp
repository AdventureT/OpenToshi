#include "ToshiPCH.h"
#include "TFifo.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: in retail these are in Toshi/TThread_Win.cpp

#ifdef USE_WIN32_CRITICAL_SECTION
#  define TFIFO_LOCK()   EnterCriticalSection( &m_CriticalSection )
#  define TFIFO_UNLOCK() LeaveCriticalSection( &m_CriticalSection )
#else // USE_WIN32_CRITICAL_SECTION
#  define TFIFO_LOCK()   m_Mutex.Lock()
#  define TFIFO_UNLOCK() m_Mutex.Unlock()
#endif // !USE_WIN32_CRITICAL_SECTION

// $deBlob: FUNCTION 00661e60
// $Barnyard: FUNCTION 006bb6f0
TBOOL TGenericFifo::Create( TCHAR* a_pBuffer, TINT a_iMaxItems, TINT a_iItemSize )
{
	TPROFILER_SCOPE();

	TASSERT( a_iMaxItems > 0 );
	TASSERT( a_iItemSize > 0 );

	m_iItemSize       = a_iItemSize;
	m_iMaxItems       = a_iMaxItems;
	m_pDataPopCursor  = a_pBuffer;
	m_pDataPushCursor = a_pBuffer;
	m_pDataBegin      = a_pBuffer;
	m_pDataEnd        = a_pBuffer + ( a_iMaxItems * a_iItemSize );

	TBOOL bResult;
	bResult = m_semNumFreeItems.Create( m_iMaxItems, m_iMaxItems );
	TASSERT( bResult != TFALSE );

	bResult = m_semNumUsedItems.Create( 0, m_iMaxItems );
	TASSERT( bResult != TFALSE );

#ifdef USE_WIN32_CRITICAL_SECTION
	InitializeCriticalSection( &m_CriticalSection );
#endif // USE_WIN32_CRITICAL_SECTION

	return TTRUE;
}

// $deBlob: FUNCTION 00661f80
// $Barnyard: FUNCTION 006bb750
TBOOL TGenericFifo::Destroy()
{
	TPROFILER_SCOPE();

#ifdef USE_WIN32_CRITICAL_SECTION
	DeleteCriticalSection( &m_CriticalSection );
#endif // USE_WIN32_CRITICAL_SECTION

	TBOOL bResult;
	bResult = m_semNumUsedItems.Destroy();
	TASSERT( bResult != TFALSE );

	bResult = m_semNumFreeItems.Destroy();
	TASSERT( bResult != TFALSE );

	return TTRUE;
}

// $deBlob: FUNCTION 00662080
// $Barnyard: FUNCTION 006bb780
TBOOL TGenericFifo::Push( void* a_pItem, Flags a_iFlags )
{
	TPROFILER_SCOPE();

	if ( !( a_iFlags & Flags_NoSemaphore ) )
	{
		TBOOL bResult = ( a_iFlags & Flags_PollSemaphore ) ? m_semNumFreeItems.Poll() : m_semNumFreeItems.Wait();
		if ( !bResult ) return TFALSE;
	}

	TFIFO_LOCK();

	// Copy data to the buffer and move cursor
	TUtil::MemCopy( m_pDataPushCursor, a_pItem, m_iItemSize );
	m_pDataPushCursor += m_iItemSize;

	// Set cursor to the beginning if reached the end
	if ( m_pDataPushCursor == m_pDataEnd )
		m_pDataPushCursor = m_pDataBegin;

	TFIFO_UNLOCK();

	if ( !( a_iFlags & Flags_NoSemaphore ) )
	{
		TBOOL bResult = m_semNumUsedItems.Signal();
		TASSERT( bResult != TFALSE );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006621a0
// $Barnyard: FUNCTION 006bb810
TBOOL TGenericFifo::Pop( void* a_pOut, Flags a_iFlags )
{
	TPROFILER_SCOPE();

	if ( a_iFlags & Flags_NoSemaphore )
	{
		TFIFO_LOCK();

		// Nothing to pop
		if ( m_pDataPopCursor == m_pDataPushCursor )
		{
			TFIFO_UNLOCK();
			return TFALSE;
		}
	}
	else
	{
		TBOOL bResult = ( a_iFlags & Flags_PollSemaphore ) ? m_semNumUsedItems.Poll() : m_semNumUsedItems.Wait();
		if ( !bResult ) return TFALSE;

		TFIFO_LOCK();
	}

	// Copy data to the output and move cursor
	TUtil::MemCopy( a_pOut, m_pDataPopCursor, m_iItemSize );
	m_pDataPopCursor += m_iItemSize;

	// Set cursor to the beginning if reached the end
	if ( m_pDataPopCursor == m_pDataEnd )
		m_pDataPopCursor = m_pDataBegin;

	TFIFO_UNLOCK();

	if ( !( a_iFlags & Flags_NoSemaphore ) )
	{
		TBOOL bResult = m_semNumFreeItems.Signal();
		TASSERT( bResult != TFALSE );
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END

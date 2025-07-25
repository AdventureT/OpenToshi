#include "ToshiPCH.h"
#include "TFifo.h"

TOSHI_NAMESPACE_START

TBOOL TGenericFifo::Create(TCHAR* a_pBuffer, TINT a_iMaxItems, TINT a_iItemSize)
{
	TASSERT(a_iMaxItems > 0, "Max items is less than zero");
	TASSERT(a_iItemSize > 0, "Item size is less than zero");

	m_iItemSize       = a_iItemSize;
	m_iMaxItems       = a_iMaxItems;
	m_pDataPopCursor  = a_pBuffer;
	m_pDataPushCursor = a_pBuffer;
	m_pDataBegin      = a_pBuffer;
	m_pDataEnd        = a_pBuffer + (a_iMaxItems * a_iItemSize);

	TBOOL bResult;
	bResult = m_Semaphore1.Create(m_iMaxItems, m_iMaxItems);
	TASSERT(bResult != TFALSE, "Unable to create semaphore for TGenericFifo");
	bResult = m_Semaphore2.Create(0, m_iMaxItems);
	TASSERT(bResult != TFALSE, "Unable to create semaphore for TGenericvFifo");

	InitializeCriticalSection(&m_CriticalSection);
	return TTRUE;
}

TBOOL TGenericFifo::Destroy()
{
	DeleteCriticalSection(&m_CriticalSection);
	m_Semaphore1.Destroy();
	m_Semaphore2.Destroy();
	return TTRUE;
}

TBOOL TGenericFifo::Push(void* a_pItem, Flags a_iFlags)
{
	TBOOL noSemaphore = a_iFlags & Flags_NoSemaphore;

	if (!noSemaphore)
	{
		TBOOL bResult = (a_iFlags & Flags_PollSemaphore) ? m_Semaphore1.Poll() : m_Semaphore1.Wait();
		if (!bResult) return TFALSE;
	}

	EnterCriticalSection(&m_CriticalSection);

	// Copy data to the buffer and move cursor
	Toshi::TUtil::MemCopy(m_pDataPushCursor, a_pItem, m_iItemSize);
	m_pDataPushCursor += m_iItemSize;

	// Set cursor to the beginning if reached the end
	if (m_pDataPushCursor == m_pDataEnd) m_pDataPushCursor = m_pDataBegin;

	LeaveCriticalSection(&m_CriticalSection);

	if (!noSemaphore)
	{
		TBOOL bResult = m_Semaphore2.Signal();
		TASSERT(bResult != TFALSE, "TSemaphore::Signal returned TFALSE");
	}

	return TTRUE;
}

TBOOL TGenericFifo::Pop(void* a_pOut, Flags a_iFlags)
{
	TBOOL noSemaphore = a_iFlags & Flags_NoSemaphore;

	if (!noSemaphore)
	{
		TBOOL bResult = (a_iFlags & Flags_PollSemaphore) ? m_Semaphore2.Poll() : m_Semaphore2.Wait();
		if (!bResult) return TFALSE;
	}
	else
	{
		// Check if FIFO is not empty
		if (m_pDataPopCursor == m_pDataPushCursor)
		{
			return TFALSE;
		}
	}

	EnterCriticalSection(&m_CriticalSection);

	// Copy data to the output and move cursor
	TUtil::MemCopy(a_pOut, m_pDataPopCursor, m_iItemSize);
	m_pDataPopCursor += m_iItemSize;

	// Set cursor to the beginning if reached the end
	if (m_pDataPopCursor == m_pDataEnd) m_pDataPopCursor = m_pDataBegin;

	LeaveCriticalSection(&m_CriticalSection);

	if (!noSemaphore)
	{
		TBOOL bResult = m_Semaphore1.Signal();
		TASSERT(bResult != TFALSE, "TSemaphore::Signal returned TFALSE");
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END

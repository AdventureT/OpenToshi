#include "pch.h"
#include "ACellBucket.h"

#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00625880
void ACellBucket::AllocateMemory( TINT a_iIndex )
{
	TASSERT( !m_heap, "Cell has already allocated memory" );

	T2String8::Format( m_szHeapName, sizeof( m_szHeapName ), "terrain%i", a_iIndex );
	m_heap = TMemory_CreateHeap( 0xA00000, TMEMORYHEAP_USEMUTEX, m_szHeapName );
}

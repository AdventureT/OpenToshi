/**
 * @file TMemory.cpp
 * de Blob's Source\Core\TMemory.c: allocation entry points, heap wrappers and the global memory mutex.
 */

#include "ToshiPCH.h"
#include "TMemory.h"
#include "TMemory_dlmalloc.h"
#include "dlmalloc.h"

#include "TMemoryDebugOff.h"

TMemoryHeap*  TMemory_GlobalHeap;
T2MutexHandle TMemory_GlobalMutex;

// $deBlob: FUNCTION 006fbb50
TMemoryHeap* TMemory_CreateHeap( TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName )
{
	return TMemory_dlheapcreate( a_uiSize, a_eFlags, a_szName );
}

// $deBlob: FUNCTION 006fbb60
TMemoryHeap* TMemory_CreateSubHeap( TMemoryHeap* a_pParent, TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName )
{
	return TMemory_dlheapcreatesubheap( a_pParent, a_uiSize, a_eFlags, a_szName );
}

// $deBlob: FUNCTION 006fbb70
TMemoryHeap* TMemory_CreateHeapInPlace( void* a_pMem, TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName )
{
	return TMemory_dlheapcreateinplace( a_pMem, a_uiSize, a_eFlags, a_szName );
}

// $deBlob: FUNCTION 006fbb80
void TMemory_DestroyHeap( TMemoryHeap* a_pHeap )
{
	TMemory_dlheapdestroy( a_pHeap );
}

// $deBlob: FUNCTION 006fbb90
void* TMalloc( TSIZE a_uiSize )
{
	return TMemory_Context.cbMalloc( a_uiSize );
}

// $deBlob: FUNCTION 006fbba0
void* TCalloc( TSIZE a_uiNumItems, TSIZE a_uiSize )
{
	return TMemory_Context.cbCalloc( a_uiNumItems, a_uiSize );
}

// $deBlob: FUNCTION 006fbbb0
void* TRealloc( void* a_pMem, TSIZE a_uiSize )
{
	return TMemory_Context.cbRealloc( a_pMem, a_uiSize );
}

// $deBlob: FUNCTION 006fbbc0
void* TReallocAligned( void* a_pMem, TSIZE a_uiSize, TSIZE a_uiAlign )
{
	return TMemory_Context.cbReallocAligned( a_pMem, a_uiSize, a_uiAlign );
}

// $deBlob: FUNCTION 006fbbd0
void* TMemalign( TSIZE a_uiAlign, TSIZE a_uiSize )
{
	return TMemory_Context.cbMemalign( a_uiAlign, a_uiSize );
}

// $deBlob: FUNCTION 006fbbe0
void TFree( void* a_pMem )
{
	TMemory_Context.cbFree( a_pMem );
}

// $deBlob: FUNCTION 006fbbf0
void* TMemoryHeap_Malloc( TMemoryHeap* a_pHeap, TSIZE a_uiSize )
{
	return TMemory_dlheapmalloc( a_pHeap, a_uiSize );
}

// $deBlob: FUNCTION 006fbc00
void* TMemoryHeap_Calloc( TMemoryHeap* a_pHeap, TSIZE a_uiNumItems, TSIZE a_uiSize )
{
	return TMemory_dlheapcalloc( a_pHeap, a_uiNumItems, a_uiSize );
}

// $deBlob: FUNCTION 006fbc10
void* TMemoryHeap_Memalign( TMemoryHeap* a_pHeap, TSIZE a_uiAlign, TSIZE a_uiSize )
{
	return TMemory_dlheapmemalign( a_pHeap, a_uiAlign, a_uiSize );
}

// $deBlob: FUNCTION 006fbc20
void* TMemoryHeap_Realloc( TMemoryHeap* a_pHeap, void* a_pMem, TSIZE a_uiSize )
{
	return TMemory_dlheaprealloc( a_pHeap, a_pMem, a_uiSize );
}

// $deBlob: FUNCTION 006fbc30
void TMemoryHeap_Free( TMemoryHeap* a_pHeap, void* a_pMem )
{
	TMemory_dlheapfree( a_pHeap, a_pMem );
}

// $deBlob: FUNCTION 006fbdc0
void TMemory_FillMemory( void* a_pMem, TINT a_iValue, TSIZE a_uiSize )
{
	TBYTE* pMem   = TSTATICCAST( TBYTE, a_pMem );
	TSIZE  uiLeft = a_uiSize;

	if ( a_uiSize >= 4 )
	{
		TUINT32  uiValue = TUINT32( a_iValue );
		TUINT32* pDWords = TREINTERPRETCAST( TUINT32*, pMem );
		TSIZE    uiNum   = a_uiSize >> 2;

		// NOTE: the value is stored as is (not replicated over the 4 bytes), like retail's rep stosd
		for ( TSIZE i = 0; i < uiNum; i++ )
			pDWords[ i ] = uiValue;

		pMem += uiNum * 4;
		uiLeft -= uiNum * 4;
	}

	switch ( uiLeft )
	{
		case 1:
			pMem[ 0 ] = TBYTE( a_iValue );
			break;
		case 2:
			*TREINTERPRETCAST( TUINT16*, pMem ) = TUINT16( a_iValue );
			break;
		case 3:
			*TREINTERPRETCAST( TUINT16*, pMem ) = TUINT16( a_iValue );
			pMem[ 2 ]                           = TBYTE( a_iValue );
			break;
	}
}

// $deBlob: FUNCTION 006fbe10
TMemoryHeap* TMemory_GetGlobalHeap()
{
	return TMemory_GlobalHeap;
}

// $deBlob: FUNCTION 006fbe20
void TMemory_SetGlobalHeap( TMemoryHeap* a_pHeap )
{
	T2Mutex_Lock( &TMemory_GlobalMutex, 0 );
	TMemory_GlobalHeap = a_pHeap;
	T2Mutex_Unlock( &TMemory_GlobalMutex );
}

// $deBlob: FUNCTION 006fbe50
void TMemoryHeap_GetInfo( TMemoryHeap* a_pHeap, TMemoryHeapInfo* a_pInfo )
{
	struct mallinfo oInfo = mspace_mallinfo( a_pHeap->mspace );

	a_pInfo->uiHeapSize         = oInfo.usmblks;
	a_pInfo->uiFree             = oInfo.fordblks;
	a_pInfo->uiUsed             = oInfo.uordblks;
	a_pInfo->uiNumAllocations   = oInfo.numallocs;
	a_pInfo->uiNumFreeBlocks    = oInfo.ordblks;
	a_pInfo->uiLargestFreeBlock = oInfo.largestfree;
}

// $deBlob: FUNCTION 006fbec0
void TMemoryHeapInfo_DebugDump( const TMemoryHeapInfo* a_pInfo )
{
	TDebug_FinalPrintf( "Heap size:               %d\n", a_pInfo->uiHeapSize );
	TDebug_FinalPrintf( "Heap Mem Free:           %d (~%dKB)\n", a_pInfo->uiFree, ( a_pInfo->uiFree + 512 ) >> 10 );
	TDebug_FinalPrintf( "Heap Mem Used:           %d (~%dKB)\n", a_pInfo->uiUsed, ( a_pInfo->uiUsed + 512 ) >> 10 );
	TDebug_FinalPrintf( "Num Allocations:         %d\n", a_pInfo->uiNumAllocations );
	TDebug_FinalPrintf( "Heap Num Free Blocks:    %d\n", a_pInfo->uiNumFreeBlocks );
	TDebug_FinalPrintf( "Heap Largest Free Block: %d\n", a_pInfo->uiLargestFreeBlock );
}

// $deBlob: FUNCTION 006fbf40
void TMemory_CreateMutex()
{
	T2Mutex_Create( &TMemory_GlobalMutex );
}

// $deBlob: FUNCTION 006fbf50
void TMemory_DestroyMutex()
{
	T2Mutex_Destroy( &TMemory_GlobalMutex );
}

// $deBlob: FUNCTION 006fbf60
void TMemory_AcquireMutex()
{
	T2Mutex_Lock( &TMemory_GlobalMutex, 0 );
}

// $deBlob: FUNCTION 006fbf70
void TMemory_ReleaseMutex()
{
	T2Mutex_Unlock( &TMemory_GlobalMutex );
}

// $deBlob: FUNCTION 006fbf80
void TMemoryHeap_CreateMutex( TMemoryHeap* a_pHeap )
{
	T2Mutex_Create( &a_pHeap->mutex );
}

// $deBlob: FUNCTION 006fbfa0
void TMemoryHeap_DestroyMutex( TMemoryHeap* a_pHeap )
{
	T2Mutex_Destroy( &a_pHeap->mutex );
}

// $deBlob: FUNCTION 006fbfc0
void TMemoryHeap_AcquireMutex( TMemoryHeap* a_pHeap )
{
	// NOTE: retail locks the global memory mutex, the heap's own mutex is created but never locked
	T2Mutex_Lock( &TMemory_GlobalMutex, 0 );
}

// $deBlob: FUNCTION 006fbfd0
void TMemoryHeap_ReleaseMutex( TMemoryHeap* a_pHeap )
{
	T2Mutex_Unlock( &TMemory_GlobalMutex );
}

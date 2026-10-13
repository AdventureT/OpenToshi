/**
 * @file TMemory_Win.cpp
 * de Blob's Source\Core\TMemory_Win.c: TMemory_Context setup on Windows.
 */

#include "ToshiPCH.h"
#include "TMemory.h"
#include "TMemory_dlmalloc.h"

#include <windows.h>

#include "TMemoryDebugOff.h"

#undef FillMemory

TMemoryContext TMemory_Context; // 0x009b1fa0

// CRT callbacks (TMEMORY_FLAGS_NATIVE)

// $deBlob: FUNCTION 006fbc40
static void* TMemory_NativeMalloc( TSIZE a_uiSize )
{
	return malloc( a_uiSize );
}

// $deBlob: FUNCTION 006fbc60
static void* TMemory_NativeCalloc( TSIZE a_uiNumItems, TSIZE a_uiSize )
{
	return calloc( a_uiNumItems, a_uiSize );
}

// $deBlob: FUNCTION 006fbc80
static void* TMemory_NativeRealloc( void* a_pMem, TSIZE a_uiSize )
{
	return realloc( a_pMem, a_uiSize );
}

// $deBlob: FUNCTION 006fbca0
static void* TMemory_NativeReallocAligned( void* a_pMem, TSIZE a_uiSize, TSIZE a_uiAlign )
{
	// NOTE: retail mixes _aligned_realloc with the malloc/free callbacks (it never gets called)
	return _aligned_realloc( a_pMem, a_uiSize, a_uiAlign );
}

// $deBlob: FUNCTION 006fbcc0
static void* TMemory_NativeMemalign( TSIZE a_uiAlign, TSIZE a_uiSize )
{
	// NOTE: the alignment is ignored in retail
	return malloc( a_uiSize );
}

// $deBlob: FUNCTION 006fbce0
static void TMemory_NativeFree( void* a_pMem )
{
	free( a_pMem );
}

// Global heap callbacks

// $deBlob: FUNCTION 006fbd00
static void* TMemory_GlobalMalloc( TSIZE a_uiSize )
{
	return TMemory_dlheapmalloc( TMemory_GlobalHeap, a_uiSize );
}

// $deBlob: FUNCTION 006fbd20
static void* TMemory_GlobalCalloc( TSIZE a_uiNumItems, TSIZE a_uiSize )
{
	return TMemory_dlheapcalloc( TMemory_GlobalHeap, a_uiNumItems, a_uiSize );
}

// $deBlob: FUNCTION 006fbd40
static void* TMemory_GlobalRealloc( void* a_pMem, TSIZE a_uiSize )
{
	return TMemory_dlheaprealloc( TMemory_GlobalHeap, a_pMem, a_uiSize );
}

// $deBlob: FUNCTION 006fbd60
static void* TMemory_GlobalReallocAligned( void* a_pMem, TSIZE a_uiSize, TSIZE a_uiAlign )
{
	return TMemory_dlheapreallocaligned( TMemory_GlobalHeap, a_pMem, a_uiSize, a_uiAlign );
}

// $deBlob: FUNCTION 006fbd80
static void* TMemory_GlobalMemalign( TSIZE a_uiAlign, TSIZE a_uiSize )
{
	return TMemory_dlheapmemalign( TMemory_GlobalHeap, a_uiAlign, a_uiSize );
}

// $deBlob: FUNCTION 006fbda0
static void TMemory_GlobalFree( void* a_pMem )
{
	TMemory_dlheapfree( TMemory_GlobalHeap, a_pMem );
}

// $deBlob: FUNCTION 006fb9d0
TINT TMemory_Init( const TMemoryParams* a_pParams )
{
	TASSERT( TNULL == TMemory_Context.sysheap );

	TMemory_CreateMutex();

	TMemory_Context.sysheap = GetProcessHeap();

	if ( TMemory_Context.sysheap == TNULL )
		return 1;

	if ( a_pParams->flags & TMEMORY_FLAGS_NATIVE )
	{
		TMemory_Context.cbMalloc         = TMemory_NativeMalloc;
		TMemory_Context.cbCalloc         = TMemory_NativeCalloc;
		TMemory_Context.cbRealloc        = TMemory_NativeRealloc;
		TMemory_Context.cbReallocAligned = TMemory_NativeReallocAligned;
		TMemory_Context.cbMemalign       = TMemory_NativeMemalign;
		TMemory_Context.cbFree           = TMemory_NativeFree;
		TMemory_GlobalHeap               = TNULL;
		return 0;
	}

	TMemory_Context.mem = HeapAlloc( TMemory_Context.sysheap, 0, a_pParams->uiSize );

	if ( TMemory_Context.mem == TNULL )
		return 1;

	TMemory_Context.cbMalloc         = TMemory_GlobalMalloc;
	TMemory_Context.cbCalloc         = TMemory_GlobalCalloc;
	TMemory_Context.cbRealloc        = TMemory_GlobalRealloc;
	TMemory_Context.cbReallocAligned = TMemory_GlobalReallocAligned;
	TMemory_Context.cbMemalign       = TMemory_GlobalMemalign;
	TMemory_Context.cbFree           = TMemory_GlobalFree;
	TMemory_GlobalHeap               = TMemory_dlheapcreateinplace( TMemory_Context.mem, a_pParams->uiSize, TMEMORYHEAP_USEMUTEX, "global" );

	return 0;
}

// $deBlob: FUNCTION 006fbb00
TINT TMemory_Shutdown()
{
	if ( TMemory_GlobalHeap != TNULL )
	{
		TMemory_dlheapdestroy( TMemory_GlobalHeap );
		TMemory_GlobalHeap = TNULL;
	}

	if ( TMemory_Context.sysheap != TNULL && TMemory_Context.mem != TNULL )
		HeapFree( TMemory_Context.sysheap, 0, TMemory_Context.mem );

	TMemory_FillMemory( &TMemory_Context, 0, sizeof( TMemory_Context ) );
	TMemory_DestroyMutex();

	return 0;
}

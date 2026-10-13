/**
 * @file TMemory.h
 * de Blob's memory system (Source\Core\TMemory*.c): a plain C module around dlmalloc mspaces.
 *
 * - TMemory_Init creates a global heap inside one big system allocation (or forwards everything to the CRT when
 *   TMEMORY_FLAGS_NATIVE is set) and fills TMemory_Context with the allocation callbacks used by TMalloc & co.
 * - Heaps (TMemoryHeap) are dlmalloc mspaces created in place, as sub heaps of another heap, or "piles" (linear
 *   allocators that never free).
 *
 * NOTE: this replaces Barnyard's TMemory class (MemBlock/MemNode allocator), which de Blob's branch doesn't have.
 */

#pragma once
#include "Toshi/Typedefs.h"
#include "T2Mutex_Win.h"

#ifdef TOSHI_PROFILER_MEMORY
#  include "TMemoryDebug.h"
#endif // TOSHI_PROFILER_MEMORY

#undef CreateMutex
#undef FillMemory

#define TMEMORYHEAP_MAXNAME 15

typedef TUINT32 TMemoryHeapFlags;
enum TMemoryHeapFlags_ : TMemoryHeapFlags
{
	TMEMORYHEAP_USEMUTEX    = BITFLAG( 0 ),
	TMEMORYHEAP_ALLOCASPILE = BITFLAG( 2 ),
};

struct TMemoryHeap
{
	TMemoryHeapFlags flags;                           // 0x00
	T2MutexHandle    mutex;                           // 0x04
	void*            subheap;                         // 0x08 memory this heap was allocated in (sub heaps only)
	void*            mspace;                          // 0x0C dlmalloc mspace (TNULL for piles)
	TBYTE*           pile;                            // 0x10 current pile pointer
	TUINT            pilesize;                        // 0x14
	TCHAR            name[ TMEMORYHEAP_MAXNAME + 1 ]; // 0x18
}; // 0x28

struct TMemoryHeapInfo
{
	TUINT uiHeapSize;         // 0x00 (dlmalloc max_footprint)
	TUINT uiFree;             // 0x04
	TUINT uiUsed;             // 0x08
	TUINT uiNumAllocations;   // 0x0C
	TUINT uiNumFreeBlocks;    // 0x10
	TUINT uiLargestFreeBlock; // 0x14
}; // 0x18

typedef TUINT32 TMemoryFlags;
enum TMemoryFlags_ : TMemoryFlags
{
	TMEMORY_FLAGS_NATIVE = BITFLAG( 0 ), // use the CRT allocator instead of the global heap
};

struct TMemoryParams
{
	TMemoryFlags flags;  // 0x00
	TUINT        uiSize; // 0x04 size of the global heap
};

struct TMemoryContext
{
	void* ( *cbMalloc )( TSIZE a_uiSize );                                         // 0x00
	void* ( *cbCalloc )( TSIZE a_uiNumItems, TSIZE a_uiSize );                     // 0x04
	void* ( *cbRealloc )( void* a_pMem, TSIZE a_uiSize );                          // 0x08
	void* ( *cbReallocAligned )( void* a_pMem, TSIZE a_uiSize, TSIZE a_uiAlign ); // 0x0C
	void* ( *cbMemalign )( TSIZE a_uiAlign, TSIZE a_uiSize );                     // 0x10
	void ( *cbFree )( void* a_pMem );                                              // 0x14
	void* sysheap;                                                                 // 0x18 HANDLE of the process heap
	void* mem;                                                                     // 0x1C memory of the global heap
}; // 0x20

extern TMemoryContext TMemory_Context;
extern TMemoryHeap*   TMemory_GlobalHeap;
extern T2MutexHandle  TMemory_GlobalMutex;

//-----------------------------------------------------------------------------
// TMemory_Win.c
//-----------------------------------------------------------------------------
TINT TMemory_Init( const TMemoryParams* a_pParams );
TINT TMemory_Shutdown();

//-----------------------------------------------------------------------------
// TMemory.c
//-----------------------------------------------------------------------------
TMemoryHeap* TMemory_CreateHeap( TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName );
TMemoryHeap* TMemory_CreateSubHeap( TMemoryHeap* a_pParent, TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName );
TMemoryHeap* TMemory_CreateHeapInPlace( void* a_pMem, TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName );
void         TMemory_DestroyHeap( TMemoryHeap* a_pHeap );

TMemoryHeap* TMemory_GetGlobalHeap();
void         TMemory_SetGlobalHeap( TMemoryHeap* a_pHeap );

void TMemory_FillMemory( void* a_pMem, TINT a_iValue, TSIZE a_uiSize );

void TMemory_CreateMutex();
void TMemory_DestroyMutex();
void TMemory_AcquireMutex();
void TMemory_ReleaseMutex();

void* TMemoryHeap_Malloc( TMemoryHeap* a_pHeap, TSIZE a_uiSize );
void* TMemoryHeap_Calloc( TMemoryHeap* a_pHeap, TSIZE a_uiNumItems, TSIZE a_uiSize );
void* TMemoryHeap_Memalign( TMemoryHeap* a_pHeap, TSIZE a_uiAlign, TSIZE a_uiSize );
void* TMemoryHeap_Realloc( TMemoryHeap* a_pHeap, void* a_pMem, TSIZE a_uiSize );
void  TMemoryHeap_Free( TMemoryHeap* a_pHeap, void* a_pMem );

void TMemoryHeap_GetInfo( TMemoryHeap* a_pHeap, TMemoryHeapInfo* a_pInfo );
void TMemoryHeapInfo_DebugDump( const TMemoryHeapInfo* a_pInfo );

void TMemoryHeap_CreateMutex( TMemoryHeap* a_pHeap );
void TMemoryHeap_DestroyMutex( TMemoryHeap* a_pHeap );
void TMemoryHeap_AcquireMutex( TMemoryHeap* a_pHeap );
void TMemoryHeap_ReleaseMutex( TMemoryHeap* a_pHeap );

// Allocations through TMemory_Context (the global heap or the CRT)
void* TMalloc( TSIZE a_uiSize );
void* TCalloc( TSIZE a_uiNumItems, TSIZE a_uiSize );
void* TRealloc( void* a_pMem, TSIZE a_uiSize );
void* TReallocAligned( void* a_pMem, TSIZE a_uiSize, TSIZE a_uiAlign );
void* TMemalign( TSIZE a_uiAlign, TSIZE a_uiSize );
void  TFree( void* a_pMem );

// NOTE: C++ helpers kept from Barnyard's API, a TNULL heap means TMemory_Context
TFORCEINLINE void* TMalloc( TSIZE a_uiSize, TMemoryHeap* a_pHeap )
{
	return a_pHeap ? TMemoryHeap_Malloc( a_pHeap, a_uiSize ) : TMalloc( a_uiSize );
}

TFORCEINLINE void* TMemalign( TSIZE a_uiAlign, TSIZE a_uiSize, TMemoryHeap* a_pHeap )
{
	return a_pHeap ? TMemoryHeap_Memalign( a_pHeap, a_uiAlign, a_uiSize ) : TMemalign( a_uiAlign, a_uiSize );
}

#ifdef TOSHI_PROFILER_MEMORY
// Overloads used by TMemoryDebugOn.h's TMalloc macro
TFORCEINLINE void* TMalloc( TSIZE a_uiSize, const TCHAR* a_szFileName, TINT a_iLineNum ) { return TMalloc( a_uiSize ); }
TFORCEINLINE void* TMalloc( TSIZE a_uiSize, TMemoryHeap* a_pHeap, const TCHAR* a_szFileName, TINT a_iLineNum ) { return TMalloc( a_uiSize, a_pHeap ); }
#endif // TOSHI_PROFILER_MEMORY

/**
 * Constructs an object in place
 */
template <class T, class... Args>
TFORCEINLINE T* TConstruct( T* a_pMemory, Args&&... args )
{
	return new ( a_pMemory ) T( std::forward<Args>( args )... );
}

/**
 * Constructs an array of objects in place
 */
template <class T, class... Args>
TFORCEINLINE T* TConstructArray( T* a_pMemory, TSIZE a_uiNumTimes, Args&&... args )
{
	for ( TSIZE i = 0; i < a_uiNumTimes; i++ )
	{
		new ( a_pMemory + i ) T( std::forward<Args>( args )... );
	}

	return a_pMemory;
}

/**
 * Destructs an object
 */
template <class T>
TFORCEINLINE void TDestruct( T* a_pPtr )
{
	if ( a_pPtr )
	{
		a_pPtr->~T();
	}
}

void* __CRTDECL operator new( size_t a_uiSize, TMemoryHeap* a_pHeap );
void* __CRTDECL operator new[]( size_t a_uiSize, TMemoryHeap* a_pHeap );

TFORCEINLINE void __CRTDECL operator delete( void* a_pMem, TMemoryHeap* a_pHeap ) noexcept
{
	TFree( a_pMem );
}

TFORCEINLINE void __CRTDECL operator delete[]( void* a_pMem, TMemoryHeap* a_pHeap ) noexcept
{
	TFree( a_pMem );
}

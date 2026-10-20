#include "ToshiPCH.h"
#include "TMemory_dlmalloc.h"
#include "dlmalloc.h"

#include "TMemoryDebugOff.h"

static TCHAR s_szOutOfMemMessage[ 256 ];

// The assert callback of the memory code (nothing sets it in retail); NOTE: names invented
using t_MemoryAssertCallback = TINT ( * )( const TCHAR* a_szMessage, const TCHAR* a_szFileName, TINT a_iLine );
static t_MemoryAssertCallback s_fnMemoryAssertCallback = TNULL; // 0x009b1fcc

// $deBlob: FUNCTION 006fc1d0
void TMemory_SetAssertCallback( t_MemoryAssertCallback a_fnCallback )
{
	s_fnMemoryAssertCallback = a_fnCallback;
}

// $deBlob: FUNCTION 006fc1e0
t_MemoryAssertCallback TMemory_GetAssertCallback()
{
	return s_fnMemoryAssertCallback;
}

// Formats the message and passes it to the callback, or prints "<file>(<line>): Assertion failed: "<message>"" and
// returns 1; NOTE: name invented
// $deBlob: FUNCTION 006fc1f0
static TINT TMemory_Assert( const TCHAR* a_szFileName, TINT a_iLine, const TCHAR* a_szFormat, ... )
{
	TCHAR szMessage[ 4096 ];

	va_list args;
	va_start( args, a_szFormat );
	vsprintf( szMessage, a_szFormat, args );
	va_end( args );

	if ( s_fnMemoryAssertCallback != TNULL )
		return s_fnMemoryAssertCallback( szMessage, a_szFileName, a_iLine );

	TDebug_FinalPrintf( "%s(%d): Assertion failed: \"%s\"\n", a_szFileName, a_iLine, szMessage );
	return 1;
}

// $deBlob: FUNCTION 006fc7c0
void TMemory_OutOfMem( TMemoryHeap* a_pHeap, TSIZE a_uiSize )
{
	TMemoryHeapInfo oInfo;
	TMemoryHeap_GetInfo( a_pHeap, &oInfo );

	TDebug_FinalPrintf( "=========================================================================\n" );
	TDebug_FinalPrintf( "MEMORY ALLOCATION FAILED\n" );
	TDebug_FinalPrintf( "------------------------\n" );
	TDebug_FinalPrintf( "HEAP: %s\n", a_pHeap->name );
	TDebug_FinalPrintf( "------------------------\n" );
	TDebug_FinalPrintf( "failed to allocate %d bytes (~%dKB, ~%dMB)\n", a_uiSize, ( a_uiSize + 512 ) >> 10, ( a_uiSize + 0x80000 ) >> 20 );
	TMemoryHeapInfo_DebugDump( &oInfo );
	TDebug_FinalPrintf( "=========================================================================\n" );

	void* apCallStack[ 50 ];
	TDebug_GetCallStack( 50, apCallStack );
	TDebug_DumpCallStack( 50, apCallStack, 0 );

	sprintf( s_szOutOfMemMessage, "Heap %s out of memory.\nAllocation of %i bytes failed.", a_pHeap->name, a_uiSize );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: the assert handler (logged with -noassertdialog) instead of retail's TMemory_Assert ("%s(%d): Assertion
	// failed" without a callback)
	TDebug_AssertHandler( s_szOutOfMemMessage, __FILE__, 47 );
#else
	TMemory_Assert( "..\\..\\Source\\Core\\TMemory_dlmalloc.c", 47, s_szOutOfMemMessage );
#endif
	__debugbreak(); // retail: int3 in every configuration
}

// $deBlob: FUNCTION 006fc300
TMemoryHeap* TMemory_dlheapcreate( TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName )
{
	return TMemory_dlheapcreatesubheap( TMemory_GetGlobalHeap(), a_uiSize, a_eFlags, a_szName );
}

// $deBlob: FUNCTION 006fc320
TMemoryHeap* TMemory_dlheapcreatesubheap( TMemoryHeap* a_pParent, TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName )
{
	TASSERT( ( a_uiSize / 4 ) * 4 == a_uiSize );

	TSIZE        uiHeapSize = a_uiSize + sizeof( TMemoryHeap );
	void*        pMem       = TMemoryHeap_Malloc( a_pParent, uiHeapSize );
	TMemoryHeap* pHeap;

	if ( pMem == TNULL )
		return TNULL;

	if ( a_eFlags & TMEMORYHEAP_ALLOCASPILE )
	{
		pHeap        = TSTATICCAST( TMemoryHeap, pMem );
		pHeap->flags = a_eFlags;
		strncpy( pHeap->name, a_szName, TMEMORYHEAP_MAXNAME );
		pHeap->name[ TMEMORYHEAP_MAXNAME ] = '\0';
		pHeap->mspace                      = TNULL;

		if ( a_eFlags & TMEMORYHEAP_USEMUTEX )
			TMemoryHeap_CreateMutex( pHeap );
		else
			TMemory_FillMemory( &pHeap->mutex, 0, sizeof( pHeap->mutex ) );

		pHeap->pilesize = uiHeapSize;
		pHeap->pile     = TREINTERPRETCAST( TBYTE*, pHeap + 1 );
	}
	else
	{
		pHeap = TMemory_dlheapcreateinplace( pMem, uiHeapSize, a_eFlags, a_szName );
	}

	if ( pHeap != TNULL )
	{
		pHeap->subheap = pMem;
		return pHeap;
	}

	// NOTE: the memory came from a_pParent but is given back through TMemory_Context, like in retail
	TFree( pMem );
	return TNULL;
}

// $deBlob: FUNCTION 006fc400
TMemoryHeap* TMemory_dlheapcreateinplace( void* a_pMem, TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName )
{
	TMemoryHeap* pHeap = TSTATICCAST( TMemoryHeap, a_pMem );
	TSIZE        size  = a_uiSize - sizeof( TMemoryHeap );

	TASSERT( size > 0 );
	TASSERT( ( size / 4 ) * 4 == size );

	pHeap->subheap = TNULL;
	pHeap->mspace  = create_mspace_with_base( pHeap + 1, size, 1 );

	if ( pHeap->mspace == TNULL )
		return TNULL;

	pHeap->flags = a_eFlags;
	strncpy( pHeap->name, a_szName, TMEMORYHEAP_MAXNAME );
	pHeap->name[ TMEMORYHEAP_MAXNAME ] = '\0';

	if ( a_eFlags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_CreateMutex( pHeap );
	else
		TMemory_FillMemory( &pHeap->mutex, 0, sizeof( pHeap->mutex ) );

	return pHeap;
}

// $deBlob: FUNCTION 006fc4e0
void TMemory_dlheapdestroy( TMemoryHeap* a_pHeap )
{
	if ( a_pHeap == TNULL )
		return;

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_DestroyMutex( a_pHeap );

	if ( !( a_pHeap->flags & TMEMORYHEAP_ALLOCASPILE ) )
		destroy_mspace( a_pHeap->mspace );

	void* pSubHeap   = a_pHeap->subheap;
	a_pHeap->mspace = TNULL;

	if ( pSubHeap != TNULL )
		TFree( pSubHeap );
}

// $deBlob: FUNCTION 006fc8b0
void* TMemory_dlheapallocpile( TMemoryHeap* a_pHeap, TSIZE a_uiSize, TSIZE a_uiAlign )
{
	TASSERT( a_pHeap->flags & TMEMORYHEAP_ALLOCASPILE );

	a_pHeap->pile = TREINTERPRETCAST( TBYTE*, ( ( TUINTPTR( a_pHeap->pile ) + a_uiAlign - 1 ) / a_uiAlign ) * a_uiAlign );

	if ( TUINTPTR( a_pHeap->pile ) + a_uiSize - TUINTPTR( a_pHeap->subheap ) > a_pHeap->pilesize )
	{
		TDebug_FinalPrintf( "out of mem in pile '%s' when trying to alloc %d bytes\n", a_pHeap->name, a_uiSize );
		TASSERT( 0 );
	}

	void* pMem = a_pHeap->pile;
	a_pHeap->pile += a_uiSize;

	return pMem;
}

// $deBlob: FUNCTION 006fc520
void* TMemory_dlheapmalloc( TMemoryHeap* a_pHeap, TSIZE a_uiSize )
{
	if ( a_pHeap->flags & TMEMORYHEAP_ALLOCASPILE )
		return TMemory_dlheapallocpile( a_pHeap, a_uiSize, 4 );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_AcquireMutex( a_pHeap );

	void* pMem = mspace_malloc( a_pHeap->mspace, a_uiSize );

	if ( pMem == TNULL )
		TMemory_OutOfMem( a_pHeap, a_uiSize );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_ReleaseMutex( a_pHeap );

	return pMem;
}

// $deBlob: FUNCTION 006fc580
void* TMemory_dlheapcalloc( TMemoryHeap* a_pHeap, TSIZE a_uiNumItems, TSIZE a_uiSize )
{
	if ( a_pHeap->flags & TMEMORYHEAP_ALLOCASPILE )
		return TMemory_dlheapallocpile( a_pHeap, a_uiNumItems * a_uiSize, 4 );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_AcquireMutex( a_pHeap );

	void* pMem = mspace_calloc( a_pHeap->mspace, a_uiNumItems, a_uiSize );

	if ( pMem == TNULL )
		TMemory_OutOfMem( a_pHeap, a_uiNumItems * a_uiSize );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_ReleaseMutex( a_pHeap );

	return pMem;
}

// $deBlob: FUNCTION 006fc5f0
void* TMemory_dlheaprealloc( TMemoryHeap* a_pHeap, void* a_pMem, TSIZE a_uiSize )
{
	TMemoryHeap* heap = a_pHeap;
	TASSERT( 0 == ( heap->flags & TMEMORYHEAP_ALLOCASPILE ) );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_AcquireMutex( a_pHeap );

	void* pMem = mspace_realloc( a_pHeap->mspace, a_pMem, a_uiSize );

	if ( pMem == TNULL )
		TMemory_OutOfMem( a_pHeap, a_uiSize );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_ReleaseMutex( a_pHeap );

	return pMem;
}

// $deBlob: FUNCTION 006fc670
void* TMemory_dlheapreallocaligned( TMemoryHeap* a_pHeap, void* a_pMem, TSIZE a_uiSize, TSIZE a_uiAlign )
{
	TMemoryHeap* heap = a_pHeap;
	TASSERT( 0 == ( heap->flags & TMEMORYHEAP_ALLOCASPILE ) );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_AcquireMutex( a_pHeap );

	void* pMem = mspace_realloc_aligned( a_pHeap->mspace, a_pMem, a_uiSize, a_uiAlign );

	if ( pMem == TNULL )
		TMemory_OutOfMem( a_pHeap, a_uiSize );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_ReleaseMutex( a_pHeap );

	return pMem;
}

// $deBlob: FUNCTION 006fc6f0
void* TMemory_dlheapmemalign( TMemoryHeap* a_pHeap, TSIZE a_uiAlign, TSIZE a_uiSize )
{
	if ( a_pHeap->flags & TMEMORYHEAP_ALLOCASPILE )
		return TMemory_dlheapallocpile( a_pHeap, a_uiSize, a_uiAlign );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_AcquireMutex( a_pHeap );

	void* pMem = mspace_memalign( a_pHeap->mspace, a_uiAlign, a_uiSize );

	if ( pMem == TNULL )
		TMemory_OutOfMem( a_pHeap, a_uiSize );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_ReleaseMutex( a_pHeap );

	return pMem;
}

// $deBlob: FUNCTION 006fc750
void TMemory_dlheapfree( TMemoryHeap* a_pHeap, void* a_pMem )
{
	TMemoryHeap* heap = a_pHeap;
	TASSERT( 0 == ( heap->flags & TMEMORYHEAP_ALLOCASPILE ) );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_AcquireMutex( a_pHeap );

	mspace_free( a_pHeap->mspace, a_pMem );

	if ( a_pHeap->flags & TMEMORYHEAP_USEMUTEX )
		TMemoryHeap_ReleaseMutex( a_pHeap );
}

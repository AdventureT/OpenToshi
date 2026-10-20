/**
 * @file TMemoryNew.cpp
 * The global operator new/delete (de Blob's file name from Globs of Doom's TMemoryNew.o, NT08, the Wii version). A TU
 * of its own in retail: they call TMemalign/TFree (Source\Core\TMemory.c) instead of inlining them.
 */

#include "ToshiPCH.h"
#include "TMemory.h"

#include "TMemoryDebugOff.h"

// $deBlob: FUNCTION 006fb970
void* __CRTDECL operator new( TSIZE a_uiSize )
{
	return TMemalign( 16, a_uiSize );
}

void* __CRTDECL operator new( TSIZE a_uiSize, ::std::nothrow_t const& ) TNOEXCEPT
{
	return TMemalign( 16, a_uiSize );
}

// $deBlob: FUNCTION 006fb990
void* __CRTDECL operator new[]( TSIZE a_uiSize )
{
	return TMemalign( 16, a_uiSize );
}

void* __CRTDECL operator new[]( TSIZE a_uiSize, ::std::nothrow_t const& ) TNOEXCEPT
{
	return TMemalign( 16, a_uiSize );
}

// $deBlob: FUNCTION 006fb980
void __CRTDECL operator delete( void* a_pMem ) TNOEXCEPT
{
	TFree( a_pMem );
}

void __CRTDECL operator delete( void* a_pMem, ::std::nothrow_t const& ) TNOEXCEPT
{
	TFree( a_pMem );
}

void __CRTDECL operator delete( void* a_pMem, TSIZE a_uiSize ) TNOEXCEPT
{
	TFree( a_pMem );
}

// $deBlob: FUNCTION 006fb9a0
void __CRTDECL operator delete[]( void* a_pMem ) TNOEXCEPT
{
	TFree( a_pMem );
}

void __CRTDECL operator delete[]( void* a_pMem, ::std::nothrow_t const& ) TNOEXCEPT
{
	TFree( a_pMem );
}

void __CRTDECL operator delete[]( void* a_pMem, TSIZE a_uiSize ) TNOEXCEPT
{
	TFree( a_pMem );
}

// $deBlob: FUNCTION 006fb9b0
void* __CRTDECL operator new( size_t a_uiSize, TMemoryHeap* a_pHeap )
{
	return TMemoryHeap_Malloc( a_pHeap, a_uiSize );
}

// $deBlob: FUNCTION 006fb9c0
void* __CRTDECL operator new[]( size_t a_uiSize, TMemoryHeap* a_pHeap )
{
	return TMemoryHeap_Malloc( a_pHeap, a_uiSize );
}

#pragma once
#include "TMemory.h"

//-----------------------------------------------------------------------------
// de Blob's Source\Core\TMemory_dlmalloc.c: TMemoryHeap on top of dlmalloc mspaces.
//-----------------------------------------------------------------------------

void         TMemory_OutOfMem( TMemoryHeap* a_pHeap, TSIZE a_uiSize );
TMemoryHeap* TMemory_dlheapcreate( TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName );
TMemoryHeap* TMemory_dlheapcreatesubheap( TMemoryHeap* a_pParent, TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName );
TMemoryHeap* TMemory_dlheapcreateinplace( void* a_pMem, TSIZE a_uiSize, TMemoryHeapFlags a_eFlags, const TCHAR* a_szName );
void         TMemory_dlheapdestroy( TMemoryHeap* a_pHeap );
void*        TMemory_dlheapallocpile( TMemoryHeap* a_pHeap, TSIZE a_uiSize, TSIZE a_uiAlign );
void*        TMemory_dlheapmalloc( TMemoryHeap* a_pHeap, TSIZE a_uiSize );
void*        TMemory_dlheapcalloc( TMemoryHeap* a_pHeap, TSIZE a_uiNumItems, TSIZE a_uiSize );
void*        TMemory_dlheaprealloc( TMemoryHeap* a_pHeap, void* a_pMem, TSIZE a_uiSize );
void*        TMemory_dlheapreallocaligned( TMemoryHeap* a_pHeap, void* a_pMem, TSIZE a_uiSize, TSIZE a_uiAlign );
void*        TMemory_dlheapmemalign( TMemoryHeap* a_pHeap, TSIZE a_uiAlign, TSIZE a_uiSize );
void         TMemory_dlheapfree( TMemoryHeap* a_pHeap, void* a_pMem );

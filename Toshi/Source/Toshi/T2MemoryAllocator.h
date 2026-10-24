#pragma once
#include "Toshi/T2Allocator.h"
#include "Core/TMemory.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: allocator of a memory heap. NOTE: de Blob's branch adds Realloc (+0x20) after T2Allocator's virtuals.
//-----------------------------------------------------------------------------
class T2MemoryAllocator : public T2Allocator
{
public:
	T2MemoryAllocator( TMemoryHeap* a_pHeap = TNULL )
	    : m_pHeap( a_pHeap )
	{}

	// $deBlob: FUNCTION 005e5ff0
	virtual void* Malloc( size_t a_uiSize ) override { return TMemoryHeap_Malloc( m_pHeap, a_uiSize ); }
	// $deBlob: FUNCTION 005e6010
	virtual void* Malloc( size_t a_uiSize, size_t a_uiAlignment ) override { return TMemoryHeap_Memalign( m_pHeap, a_uiAlignment, a_uiSize ); }
	// $deBlob: FUNCTION 005e5fa0
	virtual void Free( void* a_pMem ) override { TFree( a_pMem ); }
	// $deBlob: FUNCTION 005e5f80
	virtual TBOOL CanAllocate( size_t a_uiSize ) const override { return TTRUE; }
	// $deBlob: FUNCTION 005e60a0
	virtual void* TryMalloc( size_t a_uiSize ) override { return TMemoryHeap_Malloc( m_pHeap, a_uiSize ); }
	// $deBlob: FUNCTION 005e60c0
	virtual void* TryMalloc( size_t a_uiSize, size_t a_uiAlignment ) override { return TMemoryHeap_Memalign( m_pHeap, a_uiAlignment, a_uiSize ); }
	// $deBlob: FUNCTION 005e6090
	virtual size_t GetUsedSize() const override { return 0; }
	// $deBlob: FUNCTION 005e5f90
	virtual size_t GetCapacity() const override { return 0; }
	// $deBlob: FUNCTION 005e6030
	virtual void* Realloc( void* a_pMem, size_t a_uiSize ) { return TRealloc( a_pMem, a_uiSize ); }

	TMemoryHeap* GetHeap() const { return m_pHeap; }
	// $deBlob: FUNCTION 005e6040
	void SetHeap( TMemoryHeap* a_pHeap ) { m_pHeap = a_pHeap; }

private:
	TMemoryHeap* m_pHeap; // 0x04
};

TOSHI_NAMESPACE_END

/**
 * @file T2DynamicStack.h
 * de Blob's paged stack (ToshiTools/T2DynamicStack.h/.cpp): elements live in blocks of a power of two size, the
 * blocks are allocated on demand and kept in a T2DynamicVector, so pushed elements never move.
 *
 * NOTE: not in Barnyard's branch; rebuilt from retail (asserts in T2DynamicStack.h/.cpp).
 */
#pragma once
#include "Toshi/T2Allocator.h"
#include "ToshiTools/T2DynamicVector.h"

#include <new>

TOSHI_NAMESPACE_START

class TOSHI_API T2GenericDynamicStack
{
public:
	T2GenericDynamicStack( T2Allocator* a_pAllocator, TINT a_iElementsPerBlock, TINT a_iElementSize );

	// Sets the allocator and the block size (rounded up to a power of two). The stack must be empty
	void Init( T2Allocator* a_pAllocator, TINT a_iElementsPerBlock, TINT a_iElementSize );

	// Adds an uninitialised element on top, allocating a new block when the current one is full
	void PushEmpty();

	// Removes the top element (it isn't destroyed), freeing its block when it was the block's first element
	void Pop();

	// Copies the elements of a_rcSource (same element size) on top of this stack
	void Append( const T2GenericDynamicStack& a_rcSource );

	// $deBlob: FUNCTION 006ebd90
	void* At( TINT a_iElement )
	{
		TASSERT( a_iElement < m_iNumElements );
		return TREINTERPRETCAST( TBYTE*, m_vecBlocks[ a_iElement >> m_iBlockShift ] ) + m_iElementSize * ( a_iElement & ( m_iElementsPerBlock - 1 ) );
	}

	// $deBlob: FUNCTION 006ebcf0
	void* Top()
	{
		TASSERT( m_iNumElements > 0 );
		return At( m_iNumElements - 1 );
	}

	TINT  Size() const { return m_iNumElements; }
	TBOOL IsEmpty() const { return m_iNumElements == 0; }

protected:
	T2Allocator*           m_pAllocator;        // 0x00
	TINT                   m_iElementSize;      // 0x04
	TINT                   m_iElementsPerBlock; // 0x08
	TINT                   m_iNumElements;      // 0x0C
	T2DynamicVector<void*> m_vecBlocks;         // 0x10
	TINT                   m_iBlockShift;       // 0x24 log2( m_iElementsPerBlock )
};                                              // 0x28

template <class T>
class T2DynamicStack : public T2GenericDynamicStack
{
public:
	T2DynamicStack( T2Allocator* a_pAllocator = GetGlobalAllocator(), TINT a_iElementsPerBlock = 16 )
	    : T2GenericDynamicStack( a_pAllocator, a_iElementsPerBlock, sizeof( T ) )
	{}

	// $deBlob: FUNCTION 006ebe00
	void Push( const T& a_rcValue )
	{
		PushEmpty();
		new ( T2GenericDynamicStack::Top() ) T( a_rcValue );
	}

	T& At( TINT a_iElement ) { return *TSTATICCAST( T, T2GenericDynamicStack::At( a_iElement ) ); }
	T& Top() { return *TSTATICCAST( T, T2GenericDynamicStack::Top() ); }
	T& operator[]( TINT a_iElement ) { return At( a_iElement ); }

	// Destroys and removes the top element
	void Pop()
	{
		Top().~T();
		T2GenericDynamicStack::Pop();
	}
};

TOSHI_NAMESPACE_END

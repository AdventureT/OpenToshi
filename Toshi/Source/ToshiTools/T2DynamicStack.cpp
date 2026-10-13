#include "ToshiPCH.h"
#include "T2DynamicStack.h"
#include "Math/TMathInline.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

static TBOOL TIsPowerOf2( TINT a_iValue )
{
	return ( -a_iValue & a_iValue ) == a_iValue;
}

// $deBlob: FUNCTION 006f3c10
T2GenericDynamicStack::T2GenericDynamicStack( T2Allocator* a_pAllocator, TINT a_iElementsPerBlock, TINT a_iElementSize )
    : m_vecBlocks( GetGlobalAllocator(), 64, 64 )
{
	m_pAllocator        = a_pAllocator;
	m_iElementSize      = 0;
	m_iElementsPerBlock = 0;
	m_iNumElements      = 0;
	Init( a_pAllocator, a_iElementsPerBlock, a_iElementSize );
}

// $deBlob: FUNCTION 006f3ca0
void T2GenericDynamicStack::PushEmpty()
{
	if ( ( m_iNumElements & ( m_iElementsPerBlock - 1 ) ) == 0 )
	{
		void* pBlock = m_pAllocator->Malloc( m_iElementsPerBlock * m_iElementSize );
		m_vecBlocks.PushBack( pBlock );
	}

	m_iNumElements++;
}

// $deBlob: FUNCTION 006f3cf0
void T2GenericDynamicStack::Pop()
{
	TASSERT( m_iNumElements > 0 );
	m_iNumElements--;

	if ( ( m_iNumElements & ( m_iElementsPerBlock - 1 ) ) == 0 )
	{
		void* pBlock = m_vecBlocks.Back().Value();
		m_vecBlocks.PopBack();
		m_pAllocator->Free( pBlock );
	}
}

// $deBlob: FUNCTION 006f3dc0
void T2GenericDynamicStack::Init( T2Allocator* a_pAllocator, TINT a_iElementsPerBlock, TINT a_iElementSize )
{
	TASSERT( a_iElementSize > 0 );
	TASSERT( m_iNumElements == 0 );

	if ( ( -a_iElementsPerBlock & a_iElementsPerBlock ) != a_iElementsPerBlock )
	{
		// Round up to the next power of two
		TINT iNumBits = 0;
		for ( TINT i = a_iElementsPerBlock; i > 0; i >>= 1 )
			iNumBits++;

		TINT roundedValue = TINT( TMath::Pow( 2.0f, TFLOAT( iNumBits ) ) );
		TASSERT( roundedValue > a_iElementsPerBlock );
		a_iElementsPerBlock = roundedValue;
	}

	TASSERT( TIsPowerOf2( a_iElementsPerBlock ) );

	m_pAllocator        = a_pAllocator;
	m_iElementSize      = a_iElementSize;
	m_iElementsPerBlock = a_iElementsPerBlock;
	m_iNumElements      = 0;
	m_iBlockShift       = 0;

	for ( TINT i = a_iElementsPerBlock; i > 1; i >>= 1 )
		m_iBlockShift++;
}

TOSHI_NAMESPACE_END

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
		void* pBlock = m_vecBlocks.Back();
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

// $deBlob: FUNCTION 006f3f20
void T2GenericDynamicStack::Append( const T2GenericDynamicStack& a_rcSource )
{
	TASSERT( a_rcSource.m_iElementSize == m_iElementSize );

	TINT   iBlockSize   = m_iElementSize * m_iElementsPerBlock;
	TINT   iUsedInBlock = m_iElementSize * ( m_iNumElements & ( m_iElementsPerBlock - 1 ) );
	TBYTE* pDst;

	if ( iUsedInBlock != 0 )
	{
		pDst = TSTATICCAST( TBYTE, m_vecBlocks.Back() ) + iUsedInBlock;
	}
	else
	{
		pDst = TSTATICCAST( TBYTE, m_pAllocator->Malloc( iBlockSize ) );
		m_vecBlocks.PushBack( pDst );
		iUsedInBlock = 0;
	}

	TINT iFreeInBlock  = iBlockSize - iUsedInBlock;
	TINT iSrcBlockSize = m_iElementSize * a_rcSource.m_iElementsPerBlock;
	TINT iRemaining    = a_rcSource.m_iNumElements;

	T2DynamicVector<void*>& rSrcBlocks = const_cast<T2DynamicVector<void*>&>( a_rcSource.m_vecBlocks );

	for ( auto it = rSrcBlocks.Begin(); it != rSrcBlocks.End(); it++ )
	{
		TINT         iSrcLeft = iSrcBlockSize;
		const TBYTE* pSrc     = TSTATICCAST( TBYTE, it.Value() );

		while ( iSrcBlockSize > 0 )
		{
			if ( iFreeInBlock == 0 )
			{
				pDst = TSTATICCAST( TBYTE, m_pAllocator->Malloc( iBlockSize ) );
				m_vecBlocks.PushBack( pDst );
				iFreeInBlock = iBlockSize;
			}

			const TINT* pMin = ( iFreeInBlock >= iSrcLeft ) ? &iSrcLeft : &iFreeInBlock;
			TINT        iRemainingSize = iRemaining * m_iElementSize;
			TINT        copySize       = ( iRemainingSize >= *pMin ) ? *pMin : iRemainingSize;

			TASSERT( copySize % m_iElementSize == 0 );
			TUtil::MemCopy( pDst, pSrc, copySize );

			iSrcLeft -= copySize;
			pDst += copySize;
			iFreeInBlock -= copySize;
			pSrc += copySize;
			iRemaining -= copySize / m_iElementSize;

			TINT remainingElements = iRemaining;
			TASSERT( remainingElements >= 0 );

			if ( iRemaining == 0 )
			{
				m_iNumElements += a_rcSource.m_iNumElements;
				return;
			}

			if ( iSrcLeft <= 0 )
				break;
		}

		if ( iRemaining == 0 )
			break;
	}

	m_iNumElements += a_rcSource.m_iNumElements;
}

TOSHI_NAMESPACE_END

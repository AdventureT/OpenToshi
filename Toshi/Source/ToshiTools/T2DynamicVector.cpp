#include "ToshiPCH.h"
#include "T2DynamicVector.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006f38f0
// $Barnyard: FUNCTION 006cbd50
void T2GenericDynamicVector::Reallocate( TINT a_iNewSize, TINT a_iElementSize )
{
	if ( a_iNewSize != m_iAllocSize )
	{
		TASSERT( m_pAllocator != TNULL, "Cannot reallocate a T2DynamicVector with no allocator specified!" );

		void* elements = TNULL;

		if ( 0 < a_iNewSize )
		{
			elements = m_pAllocator->Malloc( a_iNewSize * a_iElementSize );
			TUtil::MemCopy( elements, m_poElements, m_iNumElements <= a_iNewSize ? m_iNumElements * a_iElementSize : a_iNewSize * a_iElementSize );
		}

		m_iAllocSize   = a_iNewSize;
		m_iNumElements = TMath::Min( m_iNumElements, a_iNewSize );

		if ( m_poElements )
		{
			m_pAllocator->Free( m_poElements );
		}

		m_poElements = elements;

		TASSERT( ( m_poElements == TNULL && a_iNewSize == 0 ) || ( m_poElements != TNULL && a_iNewSize > 0 ) );
	}
}

// $deBlob: FUNCTION 006f39e0
// $Barnyard: FUNCTION 006cbf00
void T2GenericDynamicVector::Grow( TINT a_iNumElements, TINT a_iElementSize )
{
	TINT newSize = m_iNumElements + a_iNumElements;
	TINT curSize = m_iAllocSize;

	if ( curSize < newSize )
	{
		TASSERT( m_iGrowSize != 0 );

		while ( curSize < newSize )
		{
			if ( m_iGrowSize == -1 )
			{
				curSize = m_iAllocSize * 2 < 3 ? 2 : m_iAllocSize * 2;
			}
			else
			{
				curSize += m_iGrowSize;
			}
		}

		Reallocate( curSize, a_iElementSize );
	}
}

// $deBlob: FUNCTION 006f3a70
TINT T2GenericDynamicVector::InsertGap( TINT a_iIndexGap, TINT a_iGapSize, TINT a_iElementSize )
{
	TASSERT( a_iIndexGap >= 0 && a_iIndexGap <= m_iNumElements );

	if ( m_iNumElements + a_iGapSize <= m_iAllocSize )
	{
		if ( a_iIndexGap < m_iNumElements )
		{
			TUtil::MemMove(
			    TREINTERPRETCAST( TBYTE*, m_poElements ) + a_iElementSize * ( a_iIndexGap + a_iGapSize ),
			    TREINTERPRETCAST( TBYTE*, m_poElements ) + a_iElementSize * a_iIndexGap,
			    a_iElementSize * ( m_iNumElements - a_iIndexGap )
			);
		}

		m_iNumElements += a_iGapSize;
		return a_iGapSize;
	}

	TASSERT( m_pAllocator != TNULL, "Cannot resize a T2DynamicVector with no allocator specified!" );
	TASSERT( m_iGrowSize != 0 );

	TINT iMinSize  = m_iAllocSize + a_iGapSize;
	TINT iGrowSize = ( m_iGrowSize == -1 ) ? m_iAllocSize * 2 : m_iAllocSize + m_iGrowSize;
	TINT iNewSize  = ( iMinSize >= iGrowSize ) ? iMinSize : iGrowSize;

	TBYTE* pNewElements = TSTATICCAST( TBYTE, m_pAllocator->Malloc( a_iElementSize * iNewSize ) );

	if ( m_poElements )
	{
		if ( a_iIndexGap > 0 )
			TUtil::MemCopy( pNewElements, m_poElements, a_iElementSize * a_iIndexGap );

		if ( m_iNumElements - a_iIndexGap > 0 )
			TUtil::MemCopy( pNewElements + a_iElementSize * ( a_iIndexGap + a_iGapSize ), TREINTERPRETCAST( TBYTE*, m_poElements ) + a_iElementSize * a_iIndexGap, a_iElementSize * ( m_iNumElements - a_iIndexGap ) );

		m_pAllocator->Free( m_poElements );
	}

	m_iNumElements += a_iGapSize;
	m_iAllocSize = iNewSize;
	m_poElements = pNewElements;

	// NOTE: like retail, returns the new capacity here
	return iNewSize;
}

TOSHI_NAMESPACE_END

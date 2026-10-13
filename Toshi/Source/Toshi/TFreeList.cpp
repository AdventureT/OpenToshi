#include "ToshiPCH.h"
#include "TFreeList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006f2b30
// $Barnyard: FUNCTION 006c1840
TFreeList::TFreeList( TUINT a_uiItemSize, TINT a_iInitialSize, TINT a_iGrowSize, const TCHAR* a_pchName )
{
	m_uiItemSize  = a_uiItemSize;
	m_iCapacity   = 0;
	m_pFreeNode   = TNULL;
	m_pRootBlock  = TNULL;
	m_pHeapHolder = TNULL;
	TASSERT( m_iGrowSize >= 0 );
	TASSERT( a_iInitialSize >= 0 );
	SetGrowSize( a_iGrowSize );
	m_iCapacity  = a_iInitialSize;
	m_pPrevList  = ms_pLastList;
	ms_pLastList = this;
}

// $deBlob: FUNCTION 006f2bf0
TFreeList::~TFreeList()
{
	TASSERT( TNULL == m_pRootBlock );
}

// $deBlob: FUNCTION 006f2d40
// $Barnyard: FUNCTION 006c18b0
TFreeList::Node* TFreeList::Allocate( TINT a_iNumber, TINT a_iSize )
{
	TASSERT( a_iNumber > 0 );
	m_iCapacity += a_iNumber;

	// NOTE: de Blob allocates from the holder's heap (Barnyard: m_pMemoryHeap), or from the global heap
	TMemoryHeap* pHeap    = m_pHeapHolder ? m_pHeapHolder->pHeap : TMemory_GetGlobalHeap();
	Node*        pNewNode = TREINTERPRETCAST( Node*, TMemoryHeap_Malloc( pHeap, a_iNumber * a_iSize + sizeof( Node ) ) );

	pNewNode->pNext  = m_pRootBlock;
	m_pRootBlock = pNewNode;

	auto  pData = pNewNode + 1;
	Node* pNext = TNULL;

	for ( TINT i = a_iNumber - 1; i != 0; i-- )
	{
		pData->pNext = pNext;
		pNext        = pData;

		pData = TREINTERPRETCAST( Node*, TREINTERPRETCAST( uintptr_t, pData ) + a_iSize );
	}

	m_pFreeNode = pNext;
	return pData;
}

// $deBlob: FUNCTION 006f2d10
// $Barnyard: FUNCTION 006c1990
void TFreeList::SetCapacity( TINT a_iNewCapacity )
{
	if ( m_iCapacity < a_iNewCapacity )
	{
		auto pNode = Allocate( a_iNewCapacity - m_iCapacity, m_uiItemSize );

		pNode->pNext     = m_pFreeNode;
		m_pFreeNode = pNode;
	}
}

// $deBlob: FUNCTION 006f2c80
// $Barnyard: FUNCTION 006c1950
void* TFreeList::New( TUINT a_uiSize )
{
	if ( a_uiSize != m_uiItemSize )
	{
		return TMalloc( a_uiSize );
	}

	auto pNode = m_pFreeNode;

	if ( pNode != TNULL )
	{
		m_pFreeNode = pNode->pNext;
		return pNode;
	}
	else
	{
		TASSERT( ( 0 < m_iGrowSize ), "Tried to grow TFreeList with 0 grow size\n" );
		return Allocate( m_iGrowSize, a_uiSize );
	}
}

// $deBlob: FUNCTION 006f2cf0
// $Barnyard: FUNCTION 006c1880
void TFreeList::Delete( void* a_Ptr )
{
#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: static objects destroyed after the shutdown would link nodes that are in the freed chunks
	if ( ms_bDeinitialised )
		return;
#endif // DEBLOB_COMMUNITY_PATCH

	Node* pNode = TSTATICCAST( Node, a_Ptr );

	if ( m_pFreeNode != TNULL )
	{
		pNode->pNext     = m_pFreeNode;
		m_pFreeNode = pNode;
	}
	else
	{
		m_pFreeNode = pNode;
		pNode->pNext     = TNULL;
	}
}

// $deBlob: FUNCTION 006f2de0
void TFreeList::FreeChunks()
{
	while ( Node* pChunk = m_pRootBlock )
	{
		m_pRootBlock = pChunk->pNext;
		TFree( pChunk );
	}
}

// $deBlob: FUNCTION 006f2c40
// $Barnyard: FUNCTION 006c1910
void TFreeList::DeinitialiseAll()
{
	for ( TFreeList* pList = ms_pLastList; pList != TNULL; pList = pList->m_pPrevList )
	{
		while ( Node* pChunk = pList->m_pRootBlock )
		{
			pList->m_pRootBlock = pChunk->pNext;
			TFree( pChunk );
		}

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: the original keeps the free nodes and the capacity, so the list would hand out freed memory
		pList->m_pFreeNode = TNULL;
		pList->m_iCapacity      = 0;
#endif // DEBLOB_COMMUNITY_PATCH
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	ms_bDeinitialised = TTRUE;
#endif // DEBLOB_COMMUNITY_PATCH
}

TOSHI_NAMESPACE_END

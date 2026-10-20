#include "ToshiPCH.h"
#include "TDList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TGenericDList::InsertSegmentAfter( TNode* a_pNode1, TNode* a_pNode2, TNode* a_pNode3 )
{
	a_pNode1->m_pNext          = a_pNode3;
	a_pNode2->m_pPrev          = a_pNode3->m_pPrev;
	a_pNode1->m_pNext->m_pPrev = a_pNode1;
	a_pNode2->m_pPrev->m_pNext = a_pNode2;
}

void TGenericDList::InsertSegmentBefore( TNode* a_pNode1, TNode* a_pNode2, TNode* a_pNode3 )
{
	a_pNode2->m_pPrev          = a_pNode3;
	a_pNode1->m_pNext          = a_pNode3->m_pNext;
	a_pNode2->m_pPrev->m_pNext = a_pNode2;
	a_pNode1->m_pNext->m_pPrev = a_pNode1;
}

void TGenericDList::InsertSegmentAtHead( TNode* a_pNode1, TNode* a_pNode2 )
{
	a_pNode1->m_pNext          = &m_oRoot;
	a_pNode2->m_pPrev          = m_oRoot.m_pPrev;
	a_pNode1->m_pNext->m_pPrev = a_pNode1;
	a_pNode2->m_pPrev->m_pNext = a_pNode2;
}

void TGenericDList::InsertSegmentAtTail( TNode* a_pNode1, TNode* a_pNode2 )
{
	a_pNode2->m_pPrev          = &m_oRoot;
	a_pNode1->m_pNext          = m_oRoot.m_pNext;
	a_pNode2->m_pPrev->m_pNext = a_pNode2;
	a_pNode1->m_pNext->m_pPrev = a_pNode1;
}

// $Barnyard: FUNCTION 006c9bf0
// $deBlob: FUNCTION 006f3160
void TGenericDList::RemoveAll()
{
	while ( !IsEmpty() )
	{
		RemoveHead();
	}
}

// $Barnyard: FUNCTION 006c8170
TINT TGenericDList::Count()
{
	TINT iCount = 0;

	for ( TNode* pNode = m_oRoot.Next(); pNode != &m_oRoot; pNode = pNode->Next() )
		iCount++;

	return iCount;
}

TOSHI_NAMESPACE_END

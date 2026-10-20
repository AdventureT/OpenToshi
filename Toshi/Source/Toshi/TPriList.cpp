#include "ToshiPCH.h"
#include "TDList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006f3190
// $Barnyard: FUNCTION 006c9c20
void TGenericPriList::Insert( TNode* a_pNode )
{
	TINT iPriority = a_pNode->m_iPriority;

	if ( iPriority < 0 )
	{
		TNode* pCurNode = m_pNext;

		while ( pCurNode != End() && iPriority >= pCurNode->m_iPriority )
		{
			pCurNode = pCurNode->m_pNext;
		}

		a_pNode->InsertBefore( pCurNode );
	}
	else
	{
		TNode* pCurNode = m_pPrev;
		while ( pCurNode != End() && iPriority < pCurNode->m_iPriority )
		{
			pCurNode = pCurNode->m_pPrev;
		}

		a_pNode->InsertAfter( pCurNode );
	}
}

// $deBlob: FUNCTION 006f3250
void TGenericPriList::Insert( TNode* a_pNode, TINT iPriority )
{
	// NOTE: de Blob unlinks the node first (Barnyard's port didn't)
	a_pNode->Remove();
	a_pNode->SetPriority( iPriority );
	Insert( a_pNode );
}

// $deBlob: FUNCTION 006f3280
void TGenericPriList::InsertHead( TNode* a_pNode )
{
	a_pNode->m_iPriority = -0x8000;
	a_pNode->InsertAfter( End() );
}

// $deBlob: FUNCTION 006f32e0
void TGenericPriList::InsertTail( TNode* a_pNode )
{
	a_pNode->m_iPriority = 0x7FFF;
	a_pNode->InsertBefore( End() );
}

void TGenericPriList::RemoveAll()
{
	TNode* pNode = m_pNext;

	while ( pNode != End() )
	{
		pNode->Remove();
		pNode = m_pNext;
	}
}

TOSHI_NAMESPACE_END

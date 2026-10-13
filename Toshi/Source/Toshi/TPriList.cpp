#include "ToshiPCH.h"
#include "TDList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TGenericPriList::TNode::InsertAfter( TNode* a_pNode )
{
	m_iPriority = -0x8000;
	TASSERT( !IsLinked() );

	m_pPrev          = a_pNode;
	m_pNext          = a_pNode->m_pNext;
	a_pNode->m_pNext = this;
	m_pNext->m_pPrev = this;
}

void TGenericPriList::TNode::InsertBefore( TNode* a_pNode )
{
	m_iPriority = 0x7FFF;
	TASSERT( !IsLinked() );

	m_pNext          = a_pNode;
	m_pPrev          = a_pNode->m_pPrev;
	a_pNode->m_pPrev = this;
	m_pPrev->m_pNext = this;
}

// $deBlob: FUNCTION 006f3190
// $Barnyard: FUNCTION 006c9c20
void TGenericPriList::Insert( TNode* a_pNode )
{
	TINT iPriority = a_pNode->m_iPriority;

	if ( iPriority < 0 )
	{
		TNode* pCurNode = m_pNext;

		while ( pCurNode != End() && pCurNode->m_iPriority <= iPriority )
		{
			pCurNode = pCurNode->m_pNext;
		}

		TASSERT( !a_pNode->IsLinked() );

		a_pNode->m_pNext          = pCurNode;
		a_pNode->m_pPrev          = pCurNode->m_pPrev;
		pCurNode->m_pPrev         = a_pNode;
		a_pNode->m_pPrev->m_pNext = a_pNode;
	}
	else
	{
		TNode* pCurNode = m_pPrev;
		while ( pCurNode != End() && iPriority < pCurNode->m_iPriority )
		{
			pCurNode = pCurNode->m_pPrev;
		}

		TASSERT( !a_pNode->IsLinked() );
		a_pNode->m_pPrev          = pCurNode;
		a_pNode->m_pNext          = pCurNode->m_pNext;
		pCurNode->m_pNext         = a_pNode;
		a_pNode->m_pNext->m_pPrev = a_pNode;
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
	a_pNode->InsertAfter( End() );
}

// $deBlob: FUNCTION 006f32e0
void TGenericPriList::InsertTail( TNode* a_pNode )
{
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

TGenericPriList::TNode::TNode( TUninitialised )
{
}


TGenericPriList::TNode::TNode()
{
	Reset();
}

TGenericPriList::TNode::~TNode()
{
	Remove();
}

TGenericPriList::TNode& TGenericPriList::TNode::operator=( const TNode& a_pNode )
{
	m_pNext = a_pNode.m_pNext;
	m_pPrev = a_pNode.m_pPrev;
	SetPriority( a_pNode.GetPriority() );
	return *this;
}

void TGenericPriList::TNode::Reset()
{
	m_pNext = this;
	m_pPrev = this;
	SetPriority( 0 );
}

void TGenericPriList::TNode::Remove()
{
	m_pPrev->m_pNext = m_pNext;
	m_pNext->m_pPrev = m_pPrev;
	m_pNext          = this;
	m_pPrev          = this;
}

TGenericPriList::TGenericPriList()
{
	m_pNext = End();
	m_pPrev = End();
}

// NOTE: retail's destructor is the (identical) TGenericDList::RemoveAll 006f3160
TGenericPriList::~TGenericPriList()
{
	RemoveAll();
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "T2DList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006eedc0
TBOOL T2GenericDList::Node::IsLinkedCall() const
{
	return m_pNext != this;
}

// $deBlob: FUNCTION 006eedd0
void T2GenericDList::Node::InsertBefore( Node* a_pInsertBefore )
{
	TASSERT( !IsLinked() );
	m_pNext                  = a_pInsertBefore;
	m_pPrev                  = a_pInsertBefore->m_pPrev;
	a_pInsertBefore->m_pPrev = this;
	m_pPrev->m_pNext         = this;
}

// $deBlob: FUNCTION 006eee20
void T2GenericDList::Node::InsertAfter( Node* a_pInsertAfter )
{
	TASSERT( !IsLinked() );
	m_pPrev                 = a_pInsertAfter;
	m_pNext                 = a_pInsertAfter->m_pNext;
	a_pInsertAfter->m_pNext = this;
	m_pNext->m_pPrev        = this;
}

// $deBlob: FUNCTION 006eee70
void T2GenericDList::Node::Remove()
{
	TASSERT( IsLinked() );
	m_pPrev->m_pNext = m_pNext;
	m_pNext->m_pPrev = m_pPrev;
	m_pNext          = this;
	m_pPrev          = this;
}

// $deBlob: FUNCTION 006eeec0
T2GenericDList::T2GenericDList()
{
}

// $deBlob: FUNCTION 006eeed0
void T2GenericDList::Splice( Node* a_pInsertBefore, Node* a_pNode )
{
	a_pNode->m_pPrev->m_pNext = a_pNode->m_pNext;
	a_pNode->m_pNext->m_pPrev = a_pNode->m_pPrev;

	a_pNode->m_pNext         = a_pInsertBefore;
	a_pNode->m_pPrev         = a_pInsertBefore->m_pPrev;
	a_pInsertBefore->m_pPrev = a_pNode;
	a_pNode->m_pPrev->m_pNext = a_pNode;
}

// $deBlob: FUNCTION 006eef00
void T2GenericDList::Splice( Node* a_pInsertBefore, Node* a_pFirst, Node* a_pLast )
{
	a_pFirst->m_pPrev->m_pNext = a_pLast->m_pNext;
	a_pLast->m_pNext->m_pPrev  = a_pFirst->m_pPrev;

	a_pLast->m_pNext           = a_pInsertBefore;
	a_pFirst->m_pPrev          = a_pInsertBefore->m_pPrev;
	a_pLast->m_pNext->m_pPrev  = a_pLast;
	a_pFirst->m_pPrev->m_pNext = a_pFirst;
}

// $deBlob: FUNCTION 006eef40
void T2GenericDList::Erase( Node* a_pFirst, Node* a_pLast )
{
	Node* pNode = a_pFirst;

	while ( pNode != a_pLast )
	{
		Node* pNext = pNode->m_pNext;
		pNode->Remove();
		pNode = pNext;
	}
}

// $deBlob: FUNCTION 006eefb0
void T2GenericDList::Clear()
{
	while ( m_oRoot.m_pNext != &m_oRoot )
		m_oRoot.m_pNext->Remove();
}

// $deBlob: FUNCTION 006ef010
// $deBlob: FUNCTION 0059d000
// $deBlob: FUNCTION 0060e6d0
// $deBlob: FUNCTION 0060e6e0
// $deBlob: FUNCTION 006513f0
TINT T2GenericDList::Size()
{
	TINT iSize = 0;

	for ( Node* pNode = m_oRoot.m_pNext; pNode != &m_oRoot; pNode = pNode->m_pNext )
		iSize++;

	return iSize;
}

// $deBlob: FUNCTION 006ef020
TINT T2GenericDList::Size() const
{
	TINT iSize = 0;

	for ( Node* pNode = m_oRoot.m_pNext; pNode != &m_oRoot; pNode = pNode->m_pNext )
		iSize++;

	return iSize;
}

// $deBlob: FUNCTION 006ef030
TBOOL T2GenericDList::IsInList( Node* a_pNode ) const
{
	for ( Node* pNode = m_oRoot.m_pNext; pNode != &m_oRoot; pNode = pNode->m_pNext )
	{
		if ( a_pNode == pNode )
			return TTRUE;
	}

	return TFALSE;
}

TOSHI_NAMESPACE_END

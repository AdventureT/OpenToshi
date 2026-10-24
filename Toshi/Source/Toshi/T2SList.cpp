#include "ToshiPCH.h"
#include "T2SList.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006efda0
TBOOL T2GenericSList::Node::IsLinked() const
{
	return m_pNext != this;
}

// $deBlob: FUNCTION 006efdb0
// $Barnyard: FUNCTION 006c7e00
void T2GenericSList::Node::InsertAfter( Node* a_pNode )
{
	TASSERT( !IsLinked() );
	m_pNext          = a_pNode->m_pNext;
	a_pNode->m_pNext = this;
}

// $deBlob: FUNCTION 006f0100
T2GenericSList::Node* T2GenericSList::FindNodeBefore( Node* a_pNode )
{
	Node* pResult = &m_oRoot;

	for ( Node* pNode = m_oRoot.m_pNext; pNode != a_pNode; pNode = pNode->m_pNext )
	{
		TASSERT( pNode != &m_oRoot );
		pResult = pNode;
	}

	return pResult;
}

// $deBlob: FUNCTION 006efe10
T2GenericSList::Node* T2GenericSList::Back()
{
	TASSERT( m_oRoot.m_pNext != &m_oRoot );

	Node* pBack = &m_oRoot;

	for ( Node* pNode = m_oRoot.m_pNext; pNode != &m_oRoot; pNode = pNode->m_pNext )
		pBack = pNode;

	return pBack;
}

// $deBlob: FUNCTION 006efe00
T2GenericSList::T2GenericSList()
{
}

// $deBlob: FUNCTION 006efe60
void T2GenericSList::PushBack( Node* a_pNode )
{
	TASSERT( !a_pNode->IsLinked() );

	// NOTE: Back() without its assert
	Node* pBack = &m_oRoot;

	for ( Node* pNode = m_oRoot.m_pNext; pNode != &m_oRoot; pNode = pNode->m_pNext )
		pBack = pNode;

	a_pNode->InsertAfter( pBack );
}

// $deBlob: FUNCTION 006efef0
// $Barnyard: FUNCTION 006c7e50
T2GenericSList::Node* T2GenericSList::PopFront()
{
	TASSERT( m_oRoot.m_pNext != &m_oRoot );

	Node* pNode     = m_oRoot.m_pNext;
	m_oRoot.m_pNext = pNode->m_pNext;
	pNode->m_pNext  = pNode;

	return pNode;
}

// $deBlob: FUNCTION 006eff30
T2GenericSList::Node* T2GenericSList::PopBack()
{
	TASSERT( m_oRoot.m_pNext != &m_oRoot );

	Node* pNodeBefore = &m_oRoot;
	Node* pBack       = m_oRoot.m_pNext;

	while ( pBack->m_pNext != &m_oRoot )
	{
		pNodeBefore = pBack;
		pBack       = pBack->m_pNext;
	}

	pNodeBefore->m_pNext = pBack->m_pNext;
	pBack->m_pNext       = pBack;

	return pBack;
}

// $deBlob: FUNCTION 006eff80
void T2GenericSList::InsertBefore( Node* a_pInsertBefore, Node* a_pNode )
{
	TASSERT( a_pNode->m_pNext == a_pNode );

	Node* pNodeBefore    = FindNodeBefore( a_pInsertBefore );
	a_pNode->m_pNext     = a_pInsertBefore;
	pNodeBefore->m_pNext = a_pNode;
}

// $deBlob: FUNCTION 006effe0
void T2GenericSList::Splice( Node* a_pInsertBefore, T2GenericSList& a_rFromList, Node* a_pNode )
{
	Node* pNodeBefore   = a_rFromList.FindNodeBefore( a_pNode );
	Node* pInsertBefore = FindNodeBefore( a_pInsertBefore );

	pNodeBefore->m_pNext   = a_pNode->m_pNext;
	a_pNode->m_pNext       = a_pInsertBefore;
	pInsertBefore->m_pNext = a_pNode;
}

// $deBlob: FUNCTION 006f0020
void T2GenericSList::Splice( Node* a_pInsertBefore, T2GenericSList& a_rFromList, Node* a_pFirst, Node* a_pLast )
{
	Node* pFirstBefore  = a_rFromList.FindNodeBefore( a_pFirst );
	Node* pLastBefore   = a_rFromList.FindNodeBefore( a_pLast );
	Node* pInsertBefore = FindNodeBefore( a_pInsertBefore );

	pFirstBefore->m_pNext  = a_pLast;
	pInsertBefore->m_pNext = a_pFirst;
	pLastBefore->m_pNext   = a_pInsertBefore;
}

// $deBlob: FUNCTION 006f0070
// $Barnyard: FUNCTION 006c7e80
T2GenericSList::Node* T2GenericSList::Erase( Node* a_pFrom, Node* a_pTo )
{
	Node* pNodeBefore = FindNodeBefore( a_pFrom );

	while ( a_pFrom != a_pTo )
	{
		Node* pNext      = a_pFrom->m_pNext;
		a_pFrom->m_pNext = a_pFrom;
		a_pFrom          = pNext;
	}

	pNodeBefore->m_pNext = a_pTo;
	return a_pTo;
}

// $deBlob: FUNCTION 006f00a0
// $deBlob: FUNCTION 0056efd0
// $deBlob: FUNCTION 00791f60
// $Barnyard: FUNCTION 006c7ec0
void T2GenericSList::Clear()
{
	Node* pNode = m_oRoot.m_pNext;

	while ( pNode != &m_oRoot )
	{
		Node* pNext    = pNode->m_pNext;
		pNode->m_pNext = pNode;
		pNode          = pNext;
	}

	m_oRoot.m_pNext = &m_oRoot;
}

// $deBlob: FUNCTION 006f00c0
TINT T2GenericSList::Size()
{
	TINT iSize = 0;

	for ( Node* pNode = m_oRoot.m_pNext; pNode != &m_oRoot; pNode = pNode->m_pNext )
		iSize++;

	return iSize;
}

// $deBlob: FUNCTION 006f00d0
// $Barnyard: FUNCTION 006c7ee0
T2GenericSList::Node* T2GenericSList::Transfer( Node* a_pNode, T2GenericSList& a_rList )
{
	Node* pNodeBefore    = FindNodeBefore( a_pNode );
	pNodeBefore->m_pNext = a_pNode->m_pNext;
	a_pNode->m_pNext     = a_pNode;
	a_rList.PushBack( a_pNode );

	return pNodeBefore;
}

TOSHI_NAMESPACE_END

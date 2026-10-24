#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a doubly linked list of nodes sorted by priority (Source\ToshiTools\T2PriorityDList.h). The list is its
// own root node (empty: the root points at itself); the iterator walks the m_pNext chain.
//-----------------------------------------------------------------------------
template <class T>
class T2PriorityDList
{
public:
	class Node
	{
	public:
		friend T2PriorityDList;

		Node() { m_pNext = m_pPrev = this; }

		Node* GetNext() const { return m_pNext; }
		Node* GetPrev() const { return m_pPrev; }

	protected:
		Node* m_pNext;     // 0x00
		Node* m_pPrev;     // 0x04
		TINT  m_iPriority; // 0x08
	};

	class Iterator
	{
	public:
		Iterator()
		    : m_pPtr( TNULL ) {}
		Iterator( Node* a_pPtr )
		    : m_pPtr( a_pPtr ) {}

		// $deBlob: FUNCTION 005f6cd0
		Iterator& operator++()
		{
			TASSERT( m_pPtr != TNULL );
			m_pPtr = m_pPtr->m_pNext;
			return *this;
		}

		T*    operator->() const { return TSTATICCAST( T, m_pPtr ); }
		T*    Get() const { return TSTATICCAST( T, m_pPtr ); }
		TBOOL operator==( const Iterator& a_rOther ) const { return m_pPtr == a_rOther.m_pPtr; }
		TBOOL operator!=( const Iterator& a_rOther ) const { return m_pPtr != a_rOther.m_pPtr; }

	private:
		Node* m_pPtr; // 0x00
	};

public:
	TBOOL IsEmpty() const { return m_oRoot.m_pNext == &m_oRoot; }

	// $deBlob: FUNCTION 005f7080
	Node* Head() const
	{
		TASSERT( !IsEmpty() );
		return m_oRoot.m_pNext;
	}

	Iterator Begin() const { return Iterator( m_oRoot.m_pNext ); }
	Iterator End() const { return Iterator( const_cast<Node*>( &m_oRoot ) ); }

private:
	Node m_oRoot; // 0x00
};

TOSHI_NAMESPACE_END

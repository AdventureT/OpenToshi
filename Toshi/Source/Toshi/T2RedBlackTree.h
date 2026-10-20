/**
 * @file T2RedBlackTree.h
 * Red-black tree implementation for the Toshi engine
 * 
 * This file provides a red-black tree implementation that includes:
 * - Balanced binary tree operations
 * - Tree node management
 * - Tree traversal and searching
 * - Tree balancing and maintenance
 */

#pragma once
#include "Toshi/TComparator.h"
#include "T2Allocator.h"

TOSHI_NAMESPACE_START

// NOTE: de Blob's node has a vtable (virtual destructor), Barnyard's doesn't
class T2GenericRedBlackTreeNode
{
public:
	friend class T2GenericRedBlackTree;

	template <class T, class C>
	friend class T2RedBlackTree;

	enum Nil
	{
		NIL
	};

protected:
	// $deBlob: FUNCTION 006f2170
	// NOTE: like retail, the default constructor leaves the node uninitialised (Insert links it)
	T2GenericRedBlackTreeNode()
	{}

	// $deBlob: FUNCTION 006f2180
	// The nil node: links to itself
	TCONSTEXPR T2GenericRedBlackTreeNode( Nil )
	{
		red       = 0;
		m_pLeft   = this;
		m_pRight  = this;
		m_pParent = this;
	}

	T2GenericRedBlackTreeNode( T2GenericRedBlackTreeNode* pNil )
	{
		red       = 0;
		m_pLeft   = pNil;
		m_pRight  = pNil;
		m_pParent = pNil;
	}

public:
	// $deBlob: FUNCTION 006f21a0
	// $deBlob: FUNCTION 006f2a90
	virtual ~T2GenericRedBlackTreeNode() {}

protected:
	TINT                       red;       // 0x04
	T2GenericRedBlackTreeNode* m_pLeft;   // 0x08
	T2GenericRedBlackTreeNode* m_pRight;  // 0x0C
	T2GenericRedBlackTreeNode* m_pParent; // 0x10
};                                        // 0x14

class T2GenericRedBlackTree
{
protected:
	// $deBlob: FUNCTION 006f21b0
	// $Barnyard: FUNCTION 006cefc0
	T2GenericRedBlackTree( T2Allocator* pAllocator )
	    : m_oRoot( &ms_oNil )
	{
		m_pAllocator   = pAllocator;
		m_iNumElements = 0;
	}

	~T2GenericRedBlackTree();

	T2Allocator* GetAllocator() const
	{
		return m_pAllocator;
	}

	size_t GetNumElements() const
	{
		return m_iNumElements;
	}

	T2GenericRedBlackTreeNode* GetFirstNode() const;
	T2GenericRedBlackTreeNode* Insert( T2GenericRedBlackTreeNode* pNode );
	void                       DeleteFixUp( T2GenericRedBlackTreeNode* pNode );
	T2GenericRedBlackTreeNode* DeleteNode( T2GenericRedBlackTreeNode* pNode );

	void                              LeftRotate( T2GenericRedBlackTreeNode* pNode );
	void                              RightRotate( T2GenericRedBlackTreeNode* pNode );
	static T2GenericRedBlackTreeNode* GetSuccessorOf( const T2GenericRedBlackTreeNode* pNode );
	static T2GenericRedBlackTreeNode* GetPredecessorOf( const T2GenericRedBlackTreeNode* pNode );

private:
	void CheckValid();

protected:
	static TCONSTINIT T2GenericRedBlackTreeNode ms_oNil;

protected:
	T2Allocator*                      m_pAllocator;   // 0x00
	mutable T2GenericRedBlackTreeNode m_oRoot;        // 0x04
	size_t                            m_iNumElements; // 0x18
};                                                    // 0x1C

template <class T>
class T2RedBlackTreeNode : public T2GenericRedBlackTreeNode
{
public:
	template <class T, class C>
	friend class T2RedBlackTree;

public:
	// $Barnyard: FUNCTION 00581330
	// $Barnyard: FUNCTION 00581d40
	// AInputMapManager maps
	// $Barnyard: FUNCTION 00597520
	// $Barnyard: FUNCTION 00597580
	// $Barnyard: FUNCTION 00606d70
	// $Barnyard: FUNCTION 005a3b70
	// $Barnyard: FUNCTION 005d1a50
	// $Barnyard: FUNCTION 005dc7c0
	// $Barnyard: FUNCTION 006128b0
	// $Barnyard: FUNCTION 006147f0
	// $Barnyard: FUNCTION 00553bc0
	// $deBlob: FUNCTION 0075ef70
	// $deBlob: FUNCTION 007e4000
	T2RedBlackTreeNode( const T& value )
	    : m_Value( value )
	{
	}

	T2RedBlackTreeNode( T&& value )
	    : m_Value( std::move( value ) )
	{
	}

	// $Barnyard: FUNCTION 00581c90
	// $Barnyard: FUNCTION 00606d00
	// $Barnyard: FUNCTION 00614180
	// AInputMapManager maps
	// $Barnyard: FUNCTION 005974b0
	// $Barnyard: FUNCTION 005a3b00
	// $Barnyard: FUNCTION 005d19e0
	// $Barnyard: FUNCTION 005dc750
	// $Barnyard: FUNCTION 00612840
	// $deBlob: FUNCTION 005da460
	// $deBlob: FUNCTION 005e0230
	// $deBlob: FUNCTION 005e0260
	// $deBlob: FUNCTION 006aa620
	// $deBlob: FUNCTION 006aa650
	// $deBlob: FUNCTION 006aa680
	// $deBlob: FUNCTION 006dd1e0
	// $deBlob: FUNCTION 0075f390
	// $deBlob: FUNCTION 007e42d0
	// $deBlob: FUNCTION 0075f0c0
	~T2RedBlackTreeNode()
	{
	}

	T* GetValue()
	{
		return &m_Value;
	}

private:
	T m_Value;
};

template <class T, class C = TComparator<T>>
class T2RedBlackTree : public T2GenericRedBlackTree
{
public:
	using Type             = T;
	using Comparator       = C;
	using ComparatorHelper = TComparatorHelperT<Type, Comparator>;
	using Node             = T2RedBlackTreeNode<Type>;

	class Iterator
	{
	public:
		TFORCEINLINE Iterator( Node* a_pNode )
		    : m_pNode( a_pNode ) {}

		TFORCEINLINE Iterator Next()
		{
			return TSTATICCAST( Node, T2RedBlackTree::GetSuccessorOf( m_pNode ) );
		}

		TFORCEINLINE Iterator Prev()
		{
			return TSTATICCAST( Node, T2RedBlackTree::GetPredecessorOf( m_pNode ) );
		}

		TFORCEINLINE Node* GetNode() const
		{
			return m_pNode;
		}

		TFORCEINLINE Type* GetValue()
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE TBOOL operator==( const Iterator& other ) const
		{
			return m_pNode == other.m_pNode;
		}

		TFORCEINLINE Type& operator*() const
		{
			return m_pNode->m_Value;
		}

		TFORCEINLINE Type* operator->() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE operator Type*() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE Iterator operator++( TINT )
		{
			Iterator curr = *this;
			m_pNode       = TSTATICCAST( Node, T2RedBlackTree::GetSuccessorOf( m_pNode ) );
			return curr;
		}

		TFORCEINLINE Iterator& operator++()
		{
			m_pNode = TSTATICCAST( Node, T2RedBlackTree::GetSuccessorOf( m_pNode ) );
			return *this;
		}

		TFORCEINLINE Iterator operator--( TINT )
		{
			Iterator curr = *this;
			m_pNode       = TSTATICCAST( Node, T2RedBlackTree::GetPredecessorOf( m_pNode ) );
			return curr;
		}

		TFORCEINLINE Iterator& operator--()
		{
			m_pNode = TSTATICCAST( Node, T2RedBlackTree::GetPredecessorOf( m_pNode ) );
			return *this;
		}

	private:
		Node* m_pNode;

		friend class CIterator;
	};

	class CIterator
	{
	public:
		TFORCEINLINE CIterator( const Node* a_pNode )
		    : m_pNode( a_pNode ) {}

		TFORCEINLINE CIterator( const Iterator& a_rcIterator )
		    : m_pNode( a_rcIterator.m_pNode ) {}

		TFORCEINLINE CIterator Next()
		{
			CIterator next = *this;
			m_pNode        = TSTATICCAST( const Node, T2RedBlackTree::GetSuccessorOf( m_pNode ) );
			return next;
		}

		TFORCEINLINE CIterator Prev()
		{
			CIterator prev = *this;
			m_pNode        = TSTATICCAST( const Node, T2RedBlackTree::GetPredecessorOf( m_pNode ) );
			return prev;
		}

		TFORCEINLINE const Node* GetNode() const
		{
			return m_pNode;
		}

		TFORCEINLINE const T* GetValue() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE TBOOL operator==( const Iterator& other ) const
		{
			return m_pNode == other.GetNode();
		}

		TFORCEINLINE TBOOL operator==( const CIterator& other ) const
		{
			return m_pNode == other.m_pNode;
		}

		TFORCEINLINE const T& operator*() const
		{
			return m_pNode->m_Value;
		}

		TFORCEINLINE const T* operator->() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE operator const T*() const
		{
			return &m_pNode->m_Value;
		}

		TFORCEINLINE CIterator operator++( TINT )
		{
			return Next();
		}

		TFORCEINLINE CIterator& operator++()
		{
			m_pNode = TSTATICCAST( const Node, T2RedBlackTree::GetSuccessorOf( m_pNode ) );
			return *this;
		}

		TFORCEINLINE CIterator operator--( TINT )
		{
			return Prev();
		}

		TFORCEINLINE CIterator& operator--()
		{
			m_pNode = TSTATICCAST( const Node, T2RedBlackTree::GetPredecessorOf( m_pNode ) );
			return *this;
		}

	private:
		const Node* m_pNode;

		friend class Iterator;
	};

public:
	// $deBlob: FUNCTION 005da020
	// $deBlob: FUNCTION 005da0a0
	// $deBlob: FUNCTION 005dd380
	// $deBlob: FUNCTION 005dd3a0
	// $deBlob: FUNCTION 005dd3c0
	// $deBlob: FUNCTION 005dd3e0
	// $deBlob: FUNCTION 006a9ab0
	// $deBlob: FUNCTION 006a9ad0
	// $deBlob: FUNCTION 006a9af0
	// $deBlob: FUNCTION 006a9be0
	// $deBlob: FUNCTION 006a9c00
	// $deBlob: FUNCTION 006a9c20
	// $deBlob: FUNCTION 006dcbc0
	// $deBlob: FUNCTION 006dcc20
	// $deBlob: FUNCTION 0075eef0
	// $deBlob: FUNCTION 0075ef50
	// $deBlob: FUNCTION 007e3f20
	// $deBlob: FUNCTION 007e3fe0
	// $Barnyard: FUNCTION 005dc290
	T2RedBlackTree( T2Allocator* pAllocator = GetGlobalAllocator() )
	    : T2GenericRedBlackTree( pAllocator )
	{
	}

	TBOOL IsRoot( Node* pNode )
	{
		return pNode == &m_oRoot;
	}

	// NOTE: every instantiation is folded into one function
	// de Blob instances (T2Map::Begin compiles to the same code)
	// $deBlob: FUNCTION 006aaab0
	// $deBlob: FUNCTION 006dd240
	// $deBlob: FUNCTION 005da530
	// $deBlob: FUNCTION 005dd530
	// $deBlob: FUNCTION 005dd550
	// $deBlob: FUNCTION 006aaa70
	// $deBlob: FUNCTION 006aaa90
	// $deBlob: FUNCTION 0075f460
	// $deBlob: FUNCTION 007e4360
	// $Barnyard: FUNCTION 005a22e0
	Iterator Begin()
	{
		return Iterator( TSTATICCAST( Node, GetFirstNode() ) );
	}

	// NOTE: every instantiation is folded into one function
	// de Blob instances (T2Map::End compiles to the same code)
	// $deBlob: FUNCTION 006aac60
	// $deBlob: FUNCTION 005da5e0
	// $deBlob: FUNCTION 005da5f0
	// $deBlob: FUNCTION 005dd650
	// $deBlob: FUNCTION 006aac40
	// $deBlob: FUNCTION 006aac50
	// $deBlob: FUNCTION 006dd280
	// $deBlob: FUNCTION 0075f480
	// $deBlob: FUNCTION 007e43f0
	// $Barnyard: FUNCTION 00596830
	Iterator End()
	{
		return Iterator( TSTATICCAST( Node, &m_oRoot ) );
	}

	const CIterator Begin() const
	{
		return CIterator( TSTATICCAST( const Node, GetFirstNode() ) );
	}

	const CIterator End() const
	{
		return CIterator( TSTATICCAST( const Node, &m_oRoot ) );
	}

	// Deletes every node (Erase from Begin to End)
	// $deBlob: FUNCTION 005da570
	// $deBlob: FUNCTION 005dd570
	// $deBlob: FUNCTION 005dd5e0
	// $deBlob: FUNCTION 007e4380
	void Clear()
	{
		Erase( Begin().GetNode(), End().GetNode() );
	}

	void DeleteAll()
	{
		while ( GetNumElements() > 0 )
		{
			Delete( TSTATICCAST( Node, GetFirstNode() ) );
		}
	}

	void Delete( Node* a_pNode )
	{
		m_pAllocator->Delete( TSTATICCAST( Node, DeleteNode( a_pNode ) ) );
	}

	// $deBlob: FUNCTION 006dd2f0
	void Erase( Iterator a_oIter )
	{
		TASSERT( a_oIter.GetNode() != TNULL );
		TASSERT( a_oIter.GetNode() != &m_oRoot );
		TASSERT( a_oIter.GetNode() != &ms_oNil );
		Delete( a_oIter.GetNode() );
	}

	// $Barnyard: FUNCTION 00606f80
	// $Barnyard: FUNCTION 006141f0
	// $Barnyard: FUNCTION 00581fa0
	// $Barnyard: FUNCTION 00596b20
	// $Barnyard: FUNCTION 005a55f0
	// $Barnyard: FUNCTION 005d1cf0
	// $Barnyard: FUNCTION 00612ac0
	// de Blob: retail takes iterators (first, last) and asserts them
	// $deBlob: FUNCTION 005da620
	// $deBlob: FUNCTION 005dd660
	// $deBlob: FUNCTION 005dd7d0
	// $deBlob: FUNCTION 006aac80
	// $deBlob: FUNCTION 006aadf0
	// $deBlob: FUNCTION 006aaf90
	// $deBlob: FUNCTION 007e4400
	void Erase( Node* a_pFrom, Node* a_pTo )
	{
		TASSERT( a_pFrom != TNULL );
		TASSERT( a_pFrom != &m_oRoot || a_pTo == &m_oRoot );
		TASSERT( a_pFrom != &ms_oNil );
		TASSERT( a_pTo != TNULL );
		TASSERT( a_pTo != &ms_oNil );

		Node* pNode = a_pFrom;

		while ( pNode != a_pTo )
		{
			Node* pNext = TSTATICCAST( Node, GetSuccessorOf( pNode ) );
			Delete( pNode );
			pNode = pNext;
		}
	}

	// NOTE: retail takes the value by copy and destroys it after constructing the node
	// $Barnyard: FUNCTION 00606dc0
	// $Barnyard: FUNCTION 005d7110
	// $Barnyard: FUNCTION 00581420
	// $Barnyard: FUNCTION 00581e20
	// $Barnyard: FUNCTION 00596840
	// $Barnyard: FUNCTION 00596e80
	// $Barnyard: FUNCTION 005a4480
	// $Barnyard: FUNCTION 005d1b30
	// $Barnyard: FUNCTION 005dcbd0
	// $Barnyard: FUNCTION 00612900
	// $Barnyard: FUNCTION 00614840
	Node* CreateNode( const T& a_rValue )
	{
		return m_pAllocator->New<Node>( a_rValue );
	}

	// $deBlob: FUNCTION 005da850
	// $deBlob: FUNCTION 005e0330
	// $deBlob: FUNCTION 005e0460
	// $deBlob: FUNCTION 006ab280
	// $deBlob: FUNCTION 006ab370
	// $deBlob: FUNCTION 006ab4b0
	// $deBlob: FUNCTION 006dd420
	// $deBlob: FUNCTION 0075f5f0
	// $deBlob: FUNCTION 007e4670
	Iterator Insert( const T& value )
	{
		Node* pNode     = CreateNode( value );
		pNode->m_pLeft  = &ms_oNil;
		pNode->m_pRight = &ms_oNil;

		return InsertImpl( pNode );
	}

	// $Barnyard: FUNCTION 00606e40
	// $Barnyard: FUNCTION 005814a0
	// $Barnyard: FUNCTION 00581e90
	// $Barnyard: FUNCTION 00596990
	// $Barnyard: FUNCTION 00597080
	// $Barnyard: FUNCTION 006bc250
	// $Barnyard: FUNCTION 005a5210
	// $Barnyard: FUNCTION 005d1bb0
	// $Barnyard: FUNCTION 005dcc50
	// $Barnyard: FUNCTION 005d7af0
	// $Barnyard: FUNCTION 00612980
	// $Barnyard: FUNCTION 006148c0
	Iterator Insert( T&& value )
	{
		Node* pNode     = CreateNode( value );
		pNode->m_pLeft  = &ms_oNil;
		pNode->m_pRight = &ms_oNil;

		return InsertImpl( pNode );
	}

	// $Barnyard: FUNCTION 00596f80
	// $Barnyard: FUNCTION 005dc2b0
	// $deBlob: FUNCTION 005d9f30
	// $deBlob: FUNCTION 006dcb10
	// $deBlob: FUNCTION 005d9ee0
	// $deBlob: FUNCTION 005dd330
	// $deBlob: FUNCTION 006a9870
	// $deBlob: FUNCTION 006a98c0
	// $deBlob: FUNCTION 006a9930
	// $deBlob: FUNCTION 0075edf0
	// $deBlob: FUNCTION 007e3e10
	Iterator Find( const T& value ) const
	{
		Node* pCurrentNode = TSTATICCAST( Node, m_oRoot.m_pLeft );

		while ( pCurrentNode != &ms_oNil )
		{
			auto iCmpResult = Comparator()( *pCurrentNode->GetValue(), value );

			if ( TComparatorHelper::IsEqual( iCmpResult ) )
				return pCurrentNode;

			pCurrentNode = TSTATICCAST( Node, ( TComparatorHelper::IsGreater( iCmpResult ) ? pCurrentNode->m_pLeft : pCurrentNode->m_pRight ) );
		}

		return TSTATICCAST( Node, &m_oRoot );
	}

	// $Barnyard: FUNCTION 00596fd0
	// $Barnyard: FUNCTION 006bc1d0
	Iterator FindLast( const T& value ) const
	{
		Node* pCurrentNode = TSTATICCAST( Node, m_oRoot.m_pLeft );

		while ( pCurrentNode != &ms_oNil )
		{
			auto iCmpResult = Comparator()( *pCurrentNode->GetValue(), value );

			if ( TComparatorHelper::IsEqual( iCmpResult ) )
			{
				// Find the last one
				Node* pLeft = TSTATICCAST( Node, pCurrentNode->m_pLeft );
				while ( pCurrentNode->m_pLeft != &ms_oNil && ComparatorHelper::IsEqual( *pLeft->GetValue(), value ) )
				{
					pCurrentNode = pLeft;
					pLeft        = TSTATICCAST( Node, pCurrentNode->m_pLeft );
				}

				return pCurrentNode;
			}

			pCurrentNode = TSTATICCAST( Node, ( TComparatorHelper::IsLessOrEqual( iCmpResult ) ? pCurrentNode->m_pRight : pCurrentNode->m_pLeft ) );
		}

		return TSTATICCAST( Node, &m_oRoot );
	}

	// $Barnyard: FUNCTION 00597040
	Iterator FindNext( const Node* a_pNextAfter, const T& value ) const
	{
		Node* pNode = TSTATICCAST( Node, GetSuccessorOf( a_pNextAfter ) );

		if ( ComparatorHelper::IsEqual( *pNode->GetValue(), value ) )
		{
			return pNode;
		}

		return TSTATICCAST( Node, &m_oRoot );
	}

	TSIZE Size() const
	{
		return GetNumElements();
	}

	T2Allocator* GetAllocator() const
	{
		return m_pAllocator;
	}

private:
	Iterator InsertImpl( Node* pNode )
	{
		Node* pCurrentNode = TSTATICCAST( Node, m_oRoot.m_pLeft );
		Node* pInsertTo    = TSTATICCAST( Node, &m_oRoot );

		while ( pCurrentNode != &ms_oNil )
		{
			pInsertTo    = pCurrentNode;
			pCurrentNode = TSTATICCAST( Node, ( ComparatorHelper::IsLessOrEqual( *pCurrentNode->GetValue(), *pNode->GetValue() ) ? pCurrentNode->m_pRight : pCurrentNode->m_pLeft ) );
		}

		pNode->m_pParent = pInsertTo;
		if ( pInsertTo == &m_oRoot || ComparatorHelper::IsGreater( *pInsertTo->GetValue(), *pNode->GetValue() ) )
		{
			pInsertTo->m_pLeft = pNode;
		}
		else
		{
			pInsertTo->m_pRight = pNode;
		}

		TASSERT( ms_oNil.red == 0, "ms_oNil not red in T2GenericRedBlackTree::TreeInsertHelp" );
		T2GenericRedBlackTree::Insert( pNode );

		return pNode;
	}
};

TOSHI_NAMESPACE_END

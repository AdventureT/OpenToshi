#pragma once
#include "Toshi/Typedefs.h"

TOSHI_NAMESPACE_START

template <class T>
class TNodeTree
{
public:
	class TNode
	{
	public:
		friend TNodeTree;

	protected:
		// $deBlob: FUNCTION 00660cb0
		// $deBlob: FUNCTION 006915d0
		TNode()
		{
			m_Parent = TNULL;
			m_Child  = TNULL;
			m_Next   = (T*)this;
			m_Prev   = (T*)this;
			m_Tree   = TNULL;
		}

		// $deBlob: FUNCTION 00660d20
		// $deBlob: FUNCTION 006916c0
		~TNode()
		{
			TASSERT( IsLinked() == TFALSE );
		}

	public:
		TBOOL IsChildOfDefaultRoot() const
		{
			TASSERT( IsLinked() == TTRUE );
			return m_Parent == (T*)( &Tree()->m_Root );
		}

		TBOOL IsLinked() const { return m_Tree != TNULL; }
		T*    Parent() const { return m_Parent; }

		// The parent, TNULL when it's the tree's root
		// $deBlob: FUNCTION 00661260
		// $deBlob: FUNCTION 00691c60
		// $deBlob: FUNCTION 00691cc0
		T* GetParent() const
		{
			TASSERT( TTRUE == IsLinked() );

			if ( m_Tree != TNULL )
				return ( m_Parent != m_Tree->GetRoot() ) ? m_Parent : TNULL;

			return ( m_Parent != TNULL ) ? m_Parent : TNULL;
		}
		T*            Next() const { return m_Next; }
		T*            Prev() const { return m_Prev; }
		TNodeTree<T>* Tree() const { return m_Tree; }
		T*            Child() const { return m_Child; }

	protected:
		TNodeTree<T>* m_Tree;
		T*            m_Next;
		T*            m_Prev;
		T*            m_Parent;
		T*            m_Child;
	};

public:
	// Walks the siblings from a node, the iteration ends (TNULL) when it's back at the first one
	class Iterator
	{
	public:
		Iterator( T* a_pNode )
		    : m_pPtr( a_pNode ), m_pFirst( a_pNode )
		{}

		T* Get() const { return m_pPtr; }

		// $deBlob: FUNCTION 00660d80
		Iterator& operator++()
		{
			TASSERT( m_pPtr != TNULL );

			if ( m_pPtr != TNULL )
			{
				T* pNext = m_pPtr->Next();
				m_pPtr   = ( pNext == m_pFirst ) ? TNULL : pNext;
			}

			return *this;
		}

	private:
		T* m_pPtr;   // 0x00
		T* m_pFirst; // 0x04
	};

public:
	// $deBlob: FUNCTION 00660c60
	// $deBlob: FUNCTION 00691560
	TNodeTree()
	{
		m_Count = 0;
	}

	// $deBlob: FUNCTION 00660cd0
	// $deBlob: FUNCTION 00691640
	~TNodeTree()
	{
		DeleteAll();
		TASSERT( IsLinked() == TFALSE );
	}

	// Links sourceNode before a_pSibling (in the sibling's parent)
	// $deBlob: FUNCTION 00660fa0
	// $deBlob: FUNCTION 00691a30
	void InsertBefore( T* a_pSibling, T* sourceNode )
	{
		TASSERT( sourceNode->IsLinked() == TFALSE );

		sourceNode->m_Tree         = this;
		sourceNode->m_Next         = a_pSibling;
		sourceNode->m_Prev         = a_pSibling->m_Prev;
		a_pSibling->m_Prev->m_Next = sourceNode;
		a_pSibling->m_Prev         = sourceNode;
		sourceNode->m_Parent       = a_pSibling->m_Parent;
		m_Count += 1;
	}

	// $deBlob: FUNCTION 006610d0
	// $deBlob: FUNCTION 00691aa0
	// $Barnyard: FUNCTION 00403980
	// $Barnyard: FUNCTION 006beb10
	/**
		* Inserts node as a child of another node.
		* 
		* @param parentNode Pointer to the parent node.
		* @param sourceNode Pointer to the node you want to insert.
		*/
	void Insert( T* parentNode, T* sourceNode )
	{
		TASSERT( sourceNode->IsLinked() == TFALSE, "The source node shouldn't be linked" );

		// Remove the source node from the tree
		Remove( *sourceNode, TFALSE );

		// Get the first attached to parent node
		T* firstAttached = parentNode->Child();

		if ( firstAttached != TNULL )
		{
			// Attach node to other attached nodes
			T* lastAttached = firstAttached->Prev();

			lastAttached->m_Next  = sourceNode;
			firstAttached->m_Prev = sourceNode;

			sourceNode->m_Next = firstAttached;
			sourceNode->m_Prev = lastAttached;
		}
		else
		{
			// Attach node as the first one
			parentNode->m_Child = sourceNode;
		}

		sourceNode->m_Tree   = this;
		sourceNode->m_Parent = parentNode;
		m_Count += 1;
	}

	/**
		* Inserts node to the default tree.
		*
		* @param sourceNode Pointer to the node you want to insert.
		*/
	// $deBlob: FUNCTION 006611d0
	void InsertAtRoot( T* sourceNode )
	{
		Insert( GetRoot(), sourceNode );
	}

	// Inserts node as the first child of parentNode
	// $deBlob: FUNCTION 00661010
	void InsertHead( T* parentNode, T* sourceNode )
	{
		TASSERT( sourceNode->IsLinked() == TFALSE );
		Remove( *sourceNode, TFALSE );

		T* firstAttached = parentNode->Child();

		if ( firstAttached != TNULL )
			InsertBefore( firstAttached, sourceNode );
		else
			m_Count += 1;

		sourceNode->m_Parent = parentNode;
		parentNode->m_Child  = sourceNode;
		sourceNode->m_Tree   = this;
	}

	// $deBlob: FUNCTION 006611a0
	// $Barnyard: FUNCTION 00645fa0
	/**
		* Inserts node as the first child of the root.
		*
		* @param sourceNode Pointer to the node you want to insert.
		*/
	void InsertAtRootHead( T* sourceNode )
	{
		InsertHead( GetRoot(), sourceNode );
	}

	// $deBlob: FUNCTION 006613e0
	// $deBlob: FUNCTION 00692010
	// $Barnyard: FUNCTION 004030d0
	/**
		 * Tries to remove sourceNode from the tree and inserts it to the parentNode or to the root
		 */
	void ReInsert( T* parentNode, T* sourceNode )
	{
		Remove( sourceNode, TFALSE );

		if ( parentNode == TNULL )
		{
			if ( this != TNULL )
			{
				InsertAtRoot( sourceNode );
				return;
			}
		}

		Insert( parentNode, sourceNode );
	}

	// $deBlob: FUNCTION 006612d0
	// $deBlob: FUNCTION 00691e70
	// $Barnyard: FUNCTION 006bf2b0
	// TNodeTree<TTask>
	// $Barnyard: FUNCTION 00403110
	T* Remove( T& node, TBOOL flag = TFALSE )
	{
		TNodeTree<T>* nodeRoot   = node.Tree();
		T*            nodeParent = node.Parent();

		if ( nodeRoot != TNULL )
		{
			// Check if the node belongs to the current tree
			if ( nodeRoot != this )
			{
				return &node;
			}

			m_Count -= 1;
		}

		if ( flag == TTRUE )
		{
			// Move the children of the node to its parent (or to the root)
			while ( node.Child() != TNULL )
			{
				T* attachedNode = node.Child();
				T* newParent    = ( node.Parent() == ( node.Tree() ? node.Tree()->GetRoot() : TNULL ) ) ? TNULL : node.Parent();

				Remove( *attachedNode, TFALSE );
				Insert( newParent ? newParent : GetRoot(), attachedNode );
			}
		}

		if ( nodeParent != TNULL )
		{
			// If it's the first attached to the root node, set it to next or just remove
			if ( nodeParent->Child() == &node )
			{
				nodeParent->m_Child = ( node.Next() != &node ) ? node.Next() : TNULL;
			}

			node.m_Parent = TNULL;
		}

		node.m_Prev->m_Next = node.m_Next;
		node.m_Next->m_Prev = node.m_Prev;
		node.m_Next         = &node;
		node.m_Prev         = &node;
		node.m_Tree         = TNULL;
		return &node;
	}

	T* Remove( T* node, TBOOL flag = TFALSE )
	{
		return Remove( *node, flag );
	}

	// $deBlob: FUNCTION 00660ef0
	// $deBlob: FUNCTION 00691960
	// $Barnyard: FUNCTION 006bf3d0
	// $Barnyard: FUNCTION 006bcaf0
	void DeleteRecurse( T* node )
	{
		while ( node != TNULL )
		{
			T* next = ( node->Next() != node ) ? node->Next() : TNULL;

			if ( node->Child() != TNULL )
			{
				DeleteRecurse( node->Child() );
			}

			Remove( node, TFALSE );
			delete node;

			node = next;
		}
	}

	// The callbacks of Recurse (the node, the user data)
	using t_RecurseCallback = void( __stdcall* )( T* a_pNode, void* a_pUserData );

	// Calls a_cbNode for a_pStart (and its siblings if a_bSiblings) and walks into the children; a_cbEnter and
	// a_cbLeave are called around each child list. NOTE: retail passes a_pStart (not the node whose children are
	// visited) to a_cbEnter and a_cbLeave
	// $deBlob: FUNCTION 00691d70
	void Recurse( t_RecurseCallback a_cbEnter, t_RecurseCallback a_cbLeave, t_RecurseCallback a_cbNode, T* a_pStart, TBOOL a_bSiblings, void* a_pUserData )
	{
		T* pNode = a_pStart;

		while ( pNode != TNULL )
		{
			T* pNext = pNode->Next();

			if ( pNext == a_pStart || pNext == pNode || !a_bSiblings )
				pNext = TNULL;

			if ( a_cbNode != TNULL )
				a_cbNode( pNode, a_pUserData );

			if ( pNode->Child() != TNULL )
			{
				if ( a_cbEnter != TNULL )
					a_cbEnter( a_pStart, a_pUserData );

				Recurse( a_cbEnter, a_cbLeave, a_cbNode, pNode->Child(), TTRUE, a_pUserData );

				if ( a_cbLeave != TNULL )
					a_cbLeave( a_pStart, a_pUserData );
			}

			pNode = pNext;
		}
	}

	// $deBlob: FUNCTION 00660e80
	// $deBlob: FUNCTION 006918f0
	void DeleteAll()
	{
		T* node = GetRoot()->Child();

		while ( node != TNULL )
		{
			Remove( node, TFALSE );
			DeleteRecurse( node );
			node = GetRoot()->Child();
		}

		TASSERT( Count() == 0 );
	}

	T* GetRoot()
	{
		return TSTATICCAST( T, &m_Root );
	}

	T* ChildOfRoot()
	{
		return m_Root.Child();
	}

	// $deBlob: FUNCTION 00660e50
	size_t Count() const
	{
		return m_Count;
	}

	TBOOL IsLinked() const
	{
		return m_Root.IsLinked();
	}

protected:
	TNode  m_Root;  // 0x0
	size_t m_Count; // 0x14
};

TOSHI_NAMESPACE_END

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
		// <TTask>: 00660cb0
		// $deBlob: FUNCTION 00660cb0
		TNode()
		{
			m_Tree   = TNULL;
			m_Next   = (T*)this;
			m_Prev   = (T*)this;
			m_Parent = TNULL;
			m_Child  = TNULL;
		}

		// <TTask>: 00660d20
		// $deBlob: FUNCTION 00660d20
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

		TBOOL         IsLinked() const { return m_Tree != TNULL; }
		T*            Parent() const { return m_Parent; }
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
	// <TTask>: 00660c60
	// $deBlob: FUNCTION 00660c60
	TNodeTree()
	{
		m_Count = 0;
	}

	// <TTask>: 00660cd0
	// $deBlob: FUNCTION 00660cd0
	~TNodeTree()
	{
		DeleteAll();
		TASSERT( IsLinked() == TFALSE );
	}

	// Links sourceNode before a_pSibling (in the sibling's parent)
	// <TTask>: 00660fa0
	// $deBlob: FUNCTION 00660fa0
	void InsertBefore( T* a_pSibling, T* sourceNode )
	{
		TASSERT( sourceNode->IsLinked() == TFALSE );

		sourceNode->m_Tree           = this;
		sourceNode->m_Next           = a_pSibling;
		sourceNode->m_Prev           = a_pSibling->m_Prev;
		a_pSibling->m_Prev->m_Next   = sourceNode;
		a_pSibling->m_Prev           = sourceNode;
		sourceNode->m_Parent         = a_pSibling->m_Parent;
		m_Count += 1;
	}

	// <TTask>: 006610d0
	// $deBlob: FUNCTION 006610d0
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
		// Toshi::TNodeTree<Toshi::TResource>::Insert - 00691aa0
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
	// <TTask>: 006611d0
	// $deBlob: FUNCTION 006611d0
	void InsertAtRoot( T* sourceNode )
	{
		Insert( GetRoot(), sourceNode );
	}

	// Inserts node as the first child of parentNode
	// <TTask>: 00661010
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

	// <TTask>: 006611a0
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

	// <TTask>: 006613e0
	// $deBlob: FUNCTION 006613e0
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

	// <TTask>: 006612d0
	// $deBlob: FUNCTION 006612d0
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

	// <TTask>: 00660ef0
	// $deBlob: FUNCTION 00660ef0
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

	// <TTask>: 00660e80
	// $deBlob: FUNCTION 00660e80
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

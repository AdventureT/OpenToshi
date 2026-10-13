#pragma once
#include "T2Iterator.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// TOSHI 2.0 - Doubly linked list
// Usage example:
//
// class AMyClass : public Toshi::T2DList<AMyClass>::Node { ... };
//
// Toshi::T2DList<AMyClass> list;
// list.PushBack(new AMyClass);
//-----------------------------------------------------------------------------
class T2GenericDList
{
public:
	class Node
	{
	protected:
		friend T2GenericDList;
		template <class T> friend class T2DList;
		T2_DEFINE_ITERATOR_FRIEND();

	public:
		constexpr Node()
		    : m_pNext( this ), m_pPrev( this )
		{}

		// NOTE: de Blob's node doesn't unlink itself, it asserts that it was removed.
		// Out-of-line instances (the destructors of node-only classes):
		// $deBlob: FUNCTION 0055d9a0
		// $deBlob: FUNCTION 0055da10
		// $deBlob: FUNCTION 0055da50
		// $deBlob: FUNCTION 00560cf0
		// $deBlob: FUNCTION 00560d40
		// $deBlob: FUNCTION 00578390
		// $deBlob: FUNCTION 0057ae50
		// $deBlob: FUNCTION 0057aec0
		// $deBlob: FUNCTION 0057af00
		// $deBlob: FUNCTION 0057af40
		// $deBlob: FUNCTION 005995d0
		// $deBlob: FUNCTION 00599610
		// $deBlob: FUNCTION 005aa000
		// $deBlob: FUNCTION 005b6450
		// $deBlob: FUNCTION 005d2350
		// $deBlob: FUNCTION 005e15b0
		// $deBlob: FUNCTION 005e9f70
		// $deBlob: FUNCTION 005f6990
		// $deBlob: FUNCTION 005f69d0
		// $deBlob: FUNCTION 005f6a40
		// $deBlob: FUNCTION 005f6a80
		// $deBlob: FUNCTION 005f6ac0
		// $deBlob: FUNCTION 005f7ee0
		// $deBlob: FUNCTION 005f7f20
		// $deBlob: FUNCTION 005f9780
		// $deBlob: FUNCTION 005f97c0
		// $deBlob: FUNCTION 005fb830
		// $deBlob: FUNCTION 005fb870
		// $deBlob: FUNCTION 005fdc00
		// $deBlob: FUNCTION 005fdc40
		// $deBlob: FUNCTION 00600740
		// $deBlob: FUNCTION 00600790
		// $deBlob: FUNCTION 00604b30
		// $deBlob: FUNCTION 00604bb0
		// $deBlob: FUNCTION 0060bf30
		// $deBlob: FUNCTION 0060bf70
		// $deBlob: FUNCTION 0060bff0
		// $deBlob: FUNCTION 0060c030
		// $deBlob: FUNCTION 0060e0f0
		// $deBlob: FUNCTION 0060e130
		// $deBlob: FUNCTION 0060e1b0
		// $deBlob: FUNCTION 0060e1f0
		// $deBlob: FUNCTION 00610c60
		// $deBlob: FUNCTION 00613990
		// $deBlob: FUNCTION 006139d0
		// $deBlob: FUNCTION 00613a10
		// $deBlob: FUNCTION 00613a50
		// $deBlob: FUNCTION 006177a0
		// $deBlob: FUNCTION 00617860
		// $deBlob: FUNCTION 006178a0
		// $deBlob: FUNCTION 0061fb30
		// $deBlob: FUNCTION 0061fb90
		// $deBlob: FUNCTION 006598e0
		// $deBlob: FUNCTION 00659920
		// $deBlob: FUNCTION 006b42e0
		// $deBlob: FUNCTION 006b4320
		// $deBlob: FUNCTION 006be460
		// $deBlob: FUNCTION 006be4a0
		// $deBlob: FUNCTION 006c5080
		// $deBlob: FUNCTION 006c50c0
		// $deBlob: FUNCTION 006dce00
		// $deBlob: FUNCTION 006dce40
		// $deBlob: FUNCTION 006e92a0
		// $deBlob: FUNCTION 006e9dd0
		// $deBlob: FUNCTION 00735440
		// $deBlob: FUNCTION 00735530
		// $deBlob: FUNCTION 00735620
		// $deBlob: FUNCTION 0075af50
		// $deBlob: FUNCTION 0075afc0
		// $deBlob: FUNCTION 0075f1e0
		// $deBlob: FUNCTION 0075f270
		// $deBlob: FUNCTION 00763890
		// $deBlob: FUNCTION 00769fb0
		// $deBlob: FUNCTION 0076a080
		// $deBlob: FUNCTION 007881a0
		// $deBlob: FUNCTION 0078bb50
		// $deBlob: FUNCTION 0078bb90
		// $deBlob: FUNCTION 0078f570
		// $deBlob: FUNCTION 0078f5d0
		// $deBlob: FUNCTION 007914f0
		// $deBlob: FUNCTION 007a44b0
		// $deBlob: FUNCTION 007a4500
		// $deBlob: FUNCTION 007b0be0
		// $deBlob: FUNCTION 007b9e70
		// $deBlob: FUNCTION 007c42f0
		// $deBlob: FUNCTION 007cea30
		// $deBlob: FUNCTION 007cea70
		// $deBlob: FUNCTION 007de7e0
		// $deBlob: FUNCTION 007de830
		// $deBlob: FUNCTION 007dfdc0
		// $deBlob: FUNCTION 007e25c0
		// $deBlob: FUNCTION 007e2600
		~Node()
		{
			TASSERT( TFALSE == IsLinked() );
		}

		TBOOL IsLinked() const;
		void  InsertBefore( Node* a_pInsertBefore );
		void  InsertAfter( Node* a_pInsertAfter );
		void  Remove();

		Node* Next() const
		{
			return m_pNext;
		}

		Node* Prev() const
		{
			return m_pPrev;
		}

	private:
		Node* m_pNext; // 0x0
		Node* m_pPrev; // 0x4
	};

public:
	T2GenericDList();

	~T2GenericDList()
	{
		TASSERT( m_oRoot.m_pNext == &m_oRoot );
	}

	TBOOL IsLinked() const
	{
		return m_oRoot.IsLinked();
	}

	// Moves a_pNode (unlinking it from its list) before a_pInsertBefore
	static void Splice( Node* a_pInsertBefore, Node* a_pNode );

	// Moves the nodes a_pFirst...a_pLast (unlinking them from their list) before a_pInsertBefore
	static void Splice( Node* a_pInsertBefore, Node* a_pFirst, Node* a_pLast );

	// Removes the nodes from a_pFirst to a_pLast (excluded)
	static void Erase( Node* a_pFirst, Node* a_pLast );

	void ClearBefore( Node* pNode )
	{
		if ( pNode != &m_oRoot )
			Erase( m_oRoot.m_pNext, pNode );
	}

	void Clear();

	TBOOL IsInList( Node* a_pNode ) const;

	TINT Size();
	TINT Size() const;

protected:
	mutable Node m_oRoot;
};

template <class T>
class T2DList : public T2GenericDList
{
public:
	T2_DEFINE_ITERATOR( T, Node );

public:
	// Shared (identical code folding) by every instantiation: 00516030
	// $Barnyard: FUNCTION 00516030
	T2DList()
	{
		TSTATICASSERT( std::is_base_of<T2GenericDList::Node, T>::value );
	}

	// NOTE: the empty check is T2GenericDList's
	~T2DList() = default;

	Iterator Head() const
	{
		TASSERT( !IsEmpty() );
		return m_oRoot.Next();
	}

	Iterator Tail() const
	{
		TASSERT( !IsEmpty() );
		return m_oRoot.Prev();
	}

	// NOTE: retail folds every instantiation (005e94e0)
	// $Barnyard: FUNCTION 005e94e0
	Iterator Begin() const
	{
		return m_oRoot.Next();
	}

	// NOTE: retail folds the instantiations whose T has the node at offset 4 (00623810)
	// $Barnyard: FUNCTION 00623810
	Iterator End() const
	{
		return &m_oRoot;
	}

	Iterator RBegin() const
	{
		return m_oRoot.Next();
	}

	Iterator REnd() const
	{
		return &m_oRoot;
	}

	Iterator Front() const
	{
		return m_oRoot.Next();
	}

	Iterator Back() const
	{
		return m_oRoot.Prev();
	}

	void Delete( Iterator iter )
	{
		iter->Remove();
		delete TSTATICCAST( T, iter );
	}

	// $Barnyard: FUNCTION 0057c4d0
	// $Barnyard: FUNCTION 0057d520
	void DeleteAll()
	{
		while ( Begin() != End() )
		{
			Delete( Begin() );
		}
	}

	// $Barnyard: FUNCTION 00457a20
	Iterator Erase( Iterator iter )
	{
		Iterator itNext = iter.Next();
		TSTATICCAST( Node, iter.Get() )->Remove(); // T can have other bases with a Remove method

		return itNext;
	}

	// $Barnyard: FUNCTION 006c8140
	TBOOL Exists( T* a_pItem )
	{
		for ( auto it = Begin(); it != End(); it++ )
		{
			if ( it == a_pItem )
				return TTRUE;
		}

		return TFALSE;
	}

	TBOOL IsEmpty() const
	{
		return !m_oRoot.IsLinked();
	}

	// Inserts a_pItem before a_itWhere and returns an iterator to it
	// NOTE: retail folds every instantiation (005d6a20, used by T2SortedList::Push and AInteriorNetwork::PathSearch)
	// $Barnyard: FUNCTION 005d6a20
	Iterator InsertBefore( Iterator a_itWhere, T* a_pItem )
	{
		a_pItem->InsertBefore( a_itWhere );
		return a_pItem;
	}

	// T2DList<AVeggiePatchVeggie> (node at +0xdc): 004c6260
	// $Barnyard: FUNCTION 004c6260
	void PushBack( T* pItem )
	{
		pItem->InsertBefore( &m_oRoot );
	}

	void PushFront( T* pItem )
	{
		pItem->InsertAfter( &m_oRoot );
	}

	T* PopBack()
	{
		auto node = Back();
		node->Remove();
		return node;
	}

	T* PopFront()
	{
		auto node = Front();
		node->Remove();
		return node;
	}
};

//-----------------------------------------------------------------------------
// Purpose: wraps typename T making it T2DList::Node
//-----------------------------------------------------------------------------
template <typename T>
class T2DListNodeWrapper : public T2DList<T2DListNodeWrapper<T>>::Node
{
public:
	using Type = T;

public:
	// constructors/destructor
	constexpr T2DListNodeWrapper() = default;

	constexpr T2DListNodeWrapper( T* a_pValue )
	    : m_pValue( a_pValue ) {}

	~T2DListNodeWrapper() = default;

	constexpr T*       GetNodeValue() { return m_pValue; }
	constexpr const T* GetNodeValue() const { return m_pValue; }
	constexpr T*       operator->() { return m_pValue; }
	constexpr const T* operator->() const { return m_pValue; }

	constexpr void SetNodeValue( T* a_pValue ) { m_pValue = a_pValue; }

	constexpr T2DListNodeWrapper& operator=( T* a_pValue )
	{
		SetNodeValue( a_pValue );
		return *this;
	}

private:
	T* m_pValue;
};

TOSHI_NAMESPACE_END

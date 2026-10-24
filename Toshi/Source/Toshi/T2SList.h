#pragma once
#include "Toshi/Typedefs.h"

TOSHI_NAMESPACE_START

class TOSHI_API T2GenericSList
{
public:
	class Node
	{
	public:
		friend class T2GenericSList;

	public:
		Node()
		{
			Reset();
		}

		void Reset()
		{
			m_pNext = this;
		}

		Node* Next() const
		{
			return m_pNext;
		}

		TBOOL IsLinked() const;
		void  InsertAfter( Node* a_pNode );

	private:
		Node* m_pNext;
	};

public:
	T2GenericSList();
	~T2GenericSList() = default;

	// Calling this won't clear the list so the items will still be linked to each other!
	// Use this only if you're sure the nodes are freed
	void Reset()
	{
		m_oRoot.Reset();
	}

	Node* Begin() const
	{
		return m_oRoot.Next();
	}

	Node* RBegin() const
	{
		return m_oRoot.Next();
	}

	Node* End() const
	{
		return &m_oRoot;
	}

	Node* REnd() const
	{
		return &m_oRoot;
	}

	TBOOL IsLinked() const
	{
		return m_oRoot.m_pNext != &m_oRoot;
	}

	TBOOL IsEmpty() const
	{
		return m_oRoot.m_pNext == &m_oRoot;
	}

	Node* FindNodeBefore( Node* a_pNode );
	Node* Back();

	void PushBack( Node* a_pNode );

	void PushFront( Node* a_pNode )
	{
		a_pNode->InsertAfter( &m_oRoot );
	}

	Node* PopFront();
	Node* PopBack();

	// Inserts a_pNode before a_pInsertBefore
	void InsertBefore( Node* a_pInsertBefore, Node* a_pNode );

	// Moves a_pNode from a_rFromList before a_pInsertBefore
	void Splice( Node* a_pInsertBefore, T2GenericSList& a_rFromList, Node* a_pNode );

	// Moves the nodes a_pFirst...a_pLast (excluded) from a_rFromList before a_pInsertBefore
	void Splice( Node* a_pInsertBefore, T2GenericSList& a_rFromList, Node* a_pFirst, Node* a_pLast );

	// Removes the nodes from a_pFrom to a_pTo (excluded), returns a_pTo
	Node* Erase( Node* a_pFrom, Node* a_pTo );

	void Clear();
	TINT Size();

	// Moves a_pNode to the back of a_rList, returns the node that was before it
	Node* Transfer( Node* a_pNode, T2GenericSList& a_rList );

private:
	mutable Node m_oRoot;
};

//-----------------------------------------------------------------------------
// TOSHI 2.0 - Singly linked list
// Usage example:
//
// class AMyClass : public Toshi::T2SList<AMyClass>::Node { ... };
//
// Toshi::T2SList<AMyClass> list;
// list.PushBack(new AMyClass);
//-----------------------------------------------------------------------------
template <class T, TINT Unknown = 0>
class T2SList : public T2GenericSList
{
public:
	class Node : public T2GenericSList::Node
	{
	public:
		T* Next() const
		{
			return TSTATICCAST( T, T2GenericSList::Node::Next() );
		}

		T* Prev() const
		{
			TASSERT( !"Nah, you can't do this" );
			return TNULL;
		}
	};

	T2_DEFINE_ITERATOR_FRIEND();
	T2_DEFINE_ITERATOR( T, T2GenericSList::Node );

public:
	// $deBlob: FUNCTION 007e0e50
	T2SList() {}
	// $deBlob: FUNCTION 007e0ef0
	// $deBlob: FUNCTION 0061b9d0
	// $deBlob: FUNCTION 007e0ea0
	// $deBlob: FUNCTION 00792130
	~T2SList() { Clear(); }

	void Delete( Iterator iter )
	{
		iter->Remove();
		delete TSTATICCAST( T, iter );
	}

	// de Blob pops the nodes and deletes them (Barnyard removes and deletes Begin())
	// $deBlob: FUNCTION 0056f230
	// $deBlob: FUNCTION 007e1010
	void DeleteAll()
	{
		while ( !IsEmpty() )
		{
			delete TSTATICCAST( T, T2GenericSList::PopFront() );
		}
	}

	// $deBlob: FUNCTION 0056f210
	// $deBlob: FUNCTION 0056f220
	// $deBlob: FUNCTION 0061baf0
	// $deBlob: FUNCTION 00792110
	// $deBlob: FUNCTION 00792120
	// $deBlob: FUNCTION 007e1000
	// T2SList<ATerrain2::MaterialEntry> (ACellBucket::CreateInstanceMaterials, ACellBucket::m_oUsedList):
	// $deBlob: FUNCTION 0061d9f0
	Iterator Begin() const { return T2GenericSList::Begin(); }
	Iterator RBegin() const { return T2GenericSList::RBegin(); }

	// $deBlob: FUNCTION 0056f280
	// $deBlob: FUNCTION 0056f290
	// $deBlob: FUNCTION 0061bb00
	// $deBlob: FUNCTION 00792170
	// $deBlob: FUNCTION 00792180
	// $deBlob: FUNCTION 007e1030
	// T2SList<ATerrain2::MaterialEntry> (ACellBucket::CreateInstanceMaterials, ACellBucket::m_oUsedList):
	// $deBlob: FUNCTION 0061da00
	Iterator End() const { return T2GenericSList::End(); }
	Iterator REnd() const { return T2GenericSList::REnd(); }

	Iterator Head() const { return T2GenericSList::Begin(); }
	Iterator Back() { return T2GenericSList::Back(); }

	Iterator FindNodeBefore( Iterator a_Node ) { return T2GenericSList::FindNodeBefore( a_Node ); }

	// $deBlob: FUNCTION 0061bb20
	// $deBlob: FUNCTION 007e1180
	Iterator PopBack() { return T2GenericSList::PopBack(); }
	// $deBlob: FUNCTION 0056f360
	// $deBlob: FUNCTION 0056f370
	// $deBlob: FUNCTION 0061dae0
	// $deBlob: FUNCTION 007921e0
	// $deBlob: FUNCTION 007e1190
	// $Barnyard: FUNCTION 006c7e50
	Iterator PopFront() { return T2GenericSList::PopFront(); }

	// $deBlob: FUNCTION 0061bb30
	// $deBlob: FUNCTION 007921f0
	// $deBlob: FUNCTION 007e11a0
	void PushBack( T2GenericSList::Node* a_pNode ) { T2GenericSList::PushBack( a_pNode ); }

	// T2SList<T> group of AAINodeHash.cpp, which calls the base function inline
	// $deBlob: FUNCTION 007e11b0
	TINT Size() { return T2GenericSList::Size(); }

	// $deBlob: FUNCTION 00792190
	// $deBlob: FUNCTION 007e1040
	// $Barnyard: FUNCTION 0057d4c0
	// $Barnyard: FUNCTION 005fbf00
	// $Barnyard: FUNCTION 00551d40
	Iterator Erase( Iterator a_Iterator ) { return T2GenericSList::Erase( a_Iterator, a_Iterator.Next() ); }
	Iterator Erase( Iterator a_From, Iterator a_To ) { return T2GenericSList::Erase( a_From, a_To ); }

	// $deBlob: FUNCTION 0061bb40
	// $Barnyard: FUNCTION 005313a0
	Iterator Transfer( Node* a_pNode, T2GenericSList& a_rList ) { return T2GenericSList::Transfer( a_pNode, a_rList ); }
};

//-----------------------------------------------------------------------------
// Purpose: wraps typename T making it T2SList::Node
//-----------------------------------------------------------------------------
template <typename T>
class T2SListNodeWrapper : public T2SList<T2SListNodeWrapper<T>>::Node
{
public:
	using Type = T;

public:
	// constructors/destructor
	// $Barnyard: FUNCTION 005dfd40
	TCONSTEXPR T2SListNodeWrapper() = default;

	TCONSTEXPR T2SListNodeWrapper( T a_Value )
	    : m_Value( a_Value ) {}

	TCONSTEXPR T2SListNodeWrapper( const T2SListNodeWrapper& a_Other )
	    : m_Value( a_Other.m_Value ) {}

	TCONSTEXPR T2SListNodeWrapper( T2SListNodeWrapper&& a_Other )
	    : m_Value( std::move( a_Other.m_Value ) ) {}

	~T2SListNodeWrapper() = default;

	TCONSTEXPR T&       GetNodeValue() { return m_Value; }
	TCONSTEXPR const T& GetNodeValue() const { return m_Value; }
	TCONSTEXPR T&       operator->() { return m_Value; }
	TCONSTEXPR const T& operator->() const { return m_Value; }

	TCONSTEXPR void SetNodeValue( T a_Value ) { m_Value = a_Value; }

	TCONSTEXPR T2SListNodeWrapper& operator=( T a_Value )
	{
		SetNodeValue( a_Value );
		return *this;
	}

private:
	T m_Value;
};

TOSHI_NAMESPACE_END

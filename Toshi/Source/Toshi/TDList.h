#pragma once
#include "T2Iterator.h"

TOSHI_NAMESPACE_START

class TGenericDList
{
public:
	class TNode
	{
	public:
		friend TGenericDList;
		template <class T> friend class TDList;
		T2_DEFINE_ITERATOR_FRIEND();

		enum TUninitialised
		{
			Unitiailised
		};

	public:
		template <typename T>
		T* As() { return TSTATICCAST( T, this ); }

	protected:
		TNode() { Reset(); }
		TNode( TUninitialised ) {}
#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail's destructor is empty (a node destroyed while linked stays in its list); Barnyard's unlinks it
		~TNode() { Remove(); }
#else
		~TNode() {}
#endif

		TNode* Next() const { return m_pNext; }
		TNode* Prev() const { return m_pPrev; }

	public:
		// $deBlob: FUNCTION 00662c80
		// $deBlob: FUNCTION 00662ca0
		// $Barnyard: FUNCTION 006c8120
		void Remove()
		{
			m_pPrev->m_pNext = m_pNext;
			m_pNext->m_pPrev = m_pPrev;
			Reset();
		}

		// $deBlob: FUNCTION 00662b60
		// $Barnyard: FUNCTION 006c8100
		void InsertAfter( TNode* a_pNode )
		{
			TASSERT( !IsLinked(), "TNode::InsertAfter - TNode shouldn't be linked" );

			m_pPrev          = a_pNode;
			m_pNext          = a_pNode->m_pNext;
			a_pNode->m_pNext = this;
			m_pNext->m_pPrev = this;
		}

		// $deBlob: FUNCTION 00686510
		// $Barnyard: FUNCTION 006c80e0
		void InsertBefore( TNode* a_pNode )
		{
			TASSERT( !IsLinked(), "TNode::InsertBefore - TNode shouldn't be linked" );

			m_pNext          = a_pNode;
			m_pPrev          = a_pNode->m_pPrev;
			a_pNode->m_pPrev = this;
			m_pPrev->m_pNext = this;
		}

		// $deBlob: FUNCTION 00662c50
		TBOOL IsLinked() const { return m_pNext != this; }

	private:
		void Reset()
		{
			m_pNext = this;
			m_pPrev = this;
		}

	protected:
		TNode* m_pNext;
		TNode* m_pPrev;
	};

public:
	void InsertSegmentAtHead( TNode* a_pNode1, TNode* a_pNode2 );
	void InsertSegmentAtTail( TNode* a_pNode1, TNode* a_pNode2 );

	// $deBlob: FUNCTION 00662bb0
	// $deBlob: FUNCTION 00662c00
	// $deBlob: FUNCTION 00686560
	void InsertHead( TNode* a_pNode ) { a_pNode->InsertAfter( &m_oRoot ); }

	// $deBlob: FUNCTION 006865b0
	// $deBlob: FUNCTION 00686600
	void InsertTail( TNode* a_pNode ) { a_pNode->InsertBefore( &m_oRoot ); }
	void RemoveHead()
	{
		if ( !IsEmpty() ) m_oRoot.Next()->Remove();
	}
	void RemoveTail()
	{
		if ( !IsEmpty() ) m_oRoot.Prev()->Remove();
	}
	TBOOL  IsEmpty() { return m_oRoot.Next() == &m_oRoot; }
	TNode* Head() { return m_oRoot.Next(); }
	TNode* Tail() { return m_oRoot.Prev(); }
	TNode* Begin() { return m_oRoot.Next(); }
	TNode* End() { return &m_oRoot; }

	void RemoveAll();

	// Returns the number of nodes in the list
	TINT Count();

public:
	static void InsertSegmentAfter( TNode* a_pNode1, TNode* a_pNode2, TNode* a_pNode3 );
	static void InsertSegmentBefore( TNode* a_pNode1, TNode* a_pNode2, TNode* a_pNode3 );

protected:
	TGenericDList() {}

	// $deBlob: FUNCTION 00662a90
	// $deBlob: FUNCTION 00662aa0
	// $deBlob: FUNCTION 00686430
	~TGenericDList() { RemoveAll(); }

protected:
	TNode m_oRoot;
};

class TGenericPriList
{
public:
	class TNode
	{
	public:
		friend TGenericPriList;
		template <class T> friend class TPriList;
		T2_DEFINE_ITERATOR_FRIEND();

		enum TUninitialised
		{
			Unitiailised
		};

	public:
		template <typename T>
		T* As() { return TSTATICCAST( T, this ); }

	protected:
		TNode() { Reset(); }
		TNode( TUninitialised ) {}
		~TNode() {}

		TNode* Next() const { return m_pNext; }
		TNode* Prev() const { return m_pPrev; }

	public:
		void Remove()
		{
			m_pPrev->m_pNext = m_pNext;
			m_pNext->m_pPrev = m_pPrev;
			m_pNext          = this;
			m_pPrev          = this;
		}

		// NOTE: header inlines in de Blob (asserts in TDList.h)
		void InsertBefore( TNode* a_pNode )
		{
			TASSERT( !IsLinked() );
			m_pNext          = a_pNode;
			m_pPrev          = a_pNode->m_pPrev;
			a_pNode->m_pPrev = this;
			m_pPrev->m_pNext = this;
		}

		void InsertAfter( TNode* a_pNode )
		{
			TASSERT( !IsLinked() );
			m_pPrev          = a_pNode;
			m_pNext          = a_pNode->m_pNext;
			a_pNode->m_pNext = this;
			m_pNext->m_pPrev = this;
		}

		TBOOL IsLinked() const { return m_pNext != this; }

		void SetPriority( TINT priority ) { m_iPriority = priority; }
		TINT GetPriority() const { return m_iPriority; }

		TNode& operator=( const TNode& a_pNode )
		{
			m_pNext = a_pNode.m_pNext;
			m_pPrev = a_pNode.m_pPrev;
			SetPriority( a_pNode.GetPriority() );
			return *this;
		}

	private:
		void Reset()
		{
			m_pNext = this;
			m_pPrev = this;
			SetPriority( 0 );
		}

	public:
		TNode* m_pNext;
		TNode* m_pPrev;
		TINT   m_iPriority;
	};

public:
	TBOOL  IsEmpty() { return m_pNext == End(); }
	TNode* Head() { return m_pNext; }
	TNode* Tail() { return m_pPrev; }
	TNode* Begin() { return m_pNext; }
	TNode* End() { return (TNode*)this; }

	void Insert( TNode* a_pNode, TINT iPriority );
	void Insert( TNode* a_pNode );
	void InsertHead( TNode* a_pNode );
	void InsertTail( TNode* a_pNode );

	void RemoveAll();

protected:
	TGenericPriList()
	{
		m_pNext = End();
		m_pPrev = End();
	}

	~TGenericPriList() { RemoveAll(); }

private:
	TNode* m_pNext;
	TNode* m_pPrev;
};

template <class T>
class TPriList : public TGenericPriList
{
public:
	T2_DEFINE_ITERATOR( T, TNode );

	TPriList() {}

	Iterator GetNext( Iterator a_itNode ) { return a_itNode.Next(); }
	Iterator GetPrev( Iterator a_itNode ) { return a_itNode.Prev(); }
	Iterator Begin() { return TGenericPriList::Begin(); }
	Iterator End() { return TGenericPriList::End(); }
	TBOOL    IsLinked() { return !TGenericPriList::IsEmpty(); }
};

template <class T>
class TDList : public TGenericDList
{
public:
	T2_DEFINE_ITERATOR( T, TNode );

	TDList() {}

	Iterator GetNext( Iterator a_itNode ) { return a_itNode.Next(); }
	Iterator GetPrev( Iterator a_itNode ) { return a_itNode.Prev(); }
	T*       Head() { return TGenericDList::Head()->template As<T>(); }
	T*       Tail() { return TGenericDList::Tail()->template As<T>(); }
	Iterator Begin() { return TGenericDList::Begin()->template As<T>(); }
	Iterator End() { return TGenericDList::End()->template As<T>(); }
	TBOOL    IsEmpty() { return TGenericDList::IsEmpty(); }
	TBOOL    IsLinked() { return m_oRoot.IsLinked(); }
	void     RemoveHead() { TGenericDList::RemoveHead(); }
	void     RemoveTail() { TGenericDList::RemoveTail(); }
	void     Remove( T* a_pNode ) { a_pNode->Remove(); }

	void RemoveAll()
	{
		while ( !IsEmpty() )
		{
			Remove( Begin() );
		}
	}

	void DeleteAll()
	{
		while ( !IsEmpty() )
		{
			Delete( Begin() );
		}
	}

	void Delete( T* a_pNode )
	{
		a_pNode->Remove();
		delete a_pNode;
	}
};

TOSHI_NAMESPACE_END

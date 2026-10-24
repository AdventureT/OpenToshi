#pragma once

TOSHI_NAMESPACE_START

template <class T>
class TNodeList
{
public:
	class TNode
	{
	protected:
		friend TNodeList;

	public:
		// NOTE: retail stores next, prev, then the list (members are declared list first)
		TCONSTEXPR TNode()
		{
			m_pNext = TSTATICCAST( T, this );
			m_pPrev = TSTATICCAST( T, this );
			m_pList = TNULL;
		}

		TCONSTEXPR TNode( const TNode& a_rcNode )
		    : m_pList( a_rcNode.m_pList ), m_pNext( a_rcNode.m_pNext ), m_pPrev( a_rcNode.m_pPrev )
		{}

		TNode( TNode&& a_rNode )
		    : m_pList( a_rNode.m_pList ), m_pNext( a_rNode.m_pNext ), m_pPrev( a_rNode.m_pPrev )
		{
			a_rNode.m_pList = TNULL;
			a_rNode.m_pNext = TNULL;
			a_rNode.m_pPrev = TNULL;
		}

		// NOTE: de Blob's node asserts that it was removed
		// $deBlob: FUNCTION 00602eb0
		// $deBlob: FUNCTION 00602f50
		// $deBlob: FUNCTION 00603040
		// $deBlob: FUNCTION 00677d50
		// $deBlob: FUNCTION 00691680
		// $deBlob: FUNCTION 006aa1c0
		// $deBlob: FUNCTION 006aa200
		// $deBlob: FUNCTION 006ae700
		// $deBlob: FUNCTION 006ae990
		// $deBlob: FUNCTION 006aeaa0
		// $deBlob: FUNCTION 0070e770
		// $deBlob: FUNCTION 0070e7b0
		~TNode()
		{
			TASSERT( IsLinked() == TFALSE );
		}

		T* Next() const { return m_pNext; }
		T* Prev() const { return m_pPrev; }

		void Remove() { GetList()->Remove( TSTATICCAST( T, this ) ); }

		// NOTE: de Blob's returns the previous list
		// $deBlob: FUNCTION 00603550
		// $deBlob: FUNCTION 006b3c40
		// $deBlob: FUNCTION 006aefb0
		// $deBlob: FUNCTION 006ab860
		// $deBlob: FUNCTION 00678170
		TNodeList* SetList( TNodeList* list )
		{
			TNodeList* pOldList = m_pList;
			m_pList             = list;
			return pOldList;
		}
		TNodeList* GetList() const { return m_pList; }

		TBOOL IsLinked() const { return m_pList != TNULL; }

	protected:
		TNodeList* m_pList;
		T*         m_pNext;
		T*         m_pPrev;
	};

	class Iterator
	{
	public:
		// $deBlob: FUNCTION 00602e40
		TCONSTEXPR Iterator()
		    : m_pPtr( TNULL )
		{}

		// $deBlob: FUNCTION 00602e30
		TCONSTEXPR Iterator( T* a_pPtr )
		    : m_pPtr( a_pPtr )
		{}

		TCONSTEXPR Iterator( const Iterator& other )
		    : m_pPtr( other.m_pPtr )
		{}

		// $deBlob: FUNCTION 00602f90
		T* Get() const
		{
			return m_pPtr;
		}

		void operator=( const Iterator& other )
		{
			m_pPtr = other.m_pPtr;
		}

		void operator=( T* pPtr )
		{
			m_pPtr = pPtr;
		}

		// $deBlob: FUNCTION 005f0ec0
		// $deBlob: FUNCTION 00602fb0
		// $deBlob: FUNCTION 00677df0
		// $deBlob: FUNCTION 0070eab0
		// $deBlob: FUNCTION 0070eaf0
		T* operator->() const
		{
			TASSERT( m_pPtr != TNULL );
			return m_pPtr;
		}

		operator T*() const
		{
			TASSERT( m_pPtr != TNULL );
			return m_pPtr;
		}

		TBOOL operator==( const Iterator& a_rcIterator ) const
		{
			return m_pPtr == a_rcIterator.m_pPtr;
		}

		TBOOL operator!=( const Iterator& a_rcIterator ) const
		{
			return m_pPtr != a_rcIterator.m_pPtr;
		}

		// NOTE: de Blob's iterator steps only when the pointer isn't null
		// $deBlob: FUNCTION 0070eb80
		// $Barnyard: FUNCTION 0040f7f0
		Iterator operator++( TINT )
		{
			TASSERT( m_pPtr != TNULL );
			Iterator old = m_pPtr;
			if ( m_pPtr )
				m_pPtr = m_pPtr->Next();
			return old;
		}

		Iterator operator--( TINT )
		{
			TASSERT( m_pPtr != TNULL );
			Iterator old = m_pPtr;
			if ( m_pPtr )
				m_pPtr = m_pPtr->Prev();
			return old;
		}

		Iterator& operator++()
		{
			TASSERT( m_pPtr != TNULL );
			if ( m_pPtr )
				m_pPtr = m_pPtr->Next();
			return *this;
		}

		// $deBlob: FUNCTION 005ebb60
		// $deBlob: FUNCTION 005ebbb0
		// $deBlob: FUNCTION 00602ff0
		// $deBlob: FUNCTION 00677e30
		// $deBlob: FUNCTION 0070eb30
		// $deBlob: FUNCTION 0070ebe0
		Iterator& operator--()
		{
			TASSERT( m_pPtr != TNULL );
			if ( m_pPtr )
				m_pPtr = m_pPtr->Prev();
			return *this;
		}

	protected:
		T* m_pPtr;
	};

public:
	// $Barnyard: FUNCTION 0040d9f0
	// $Barnyard: FUNCTION 0040ea80
	TNodeList()
	{
		m_iCount = 0;
	}

	// $deBlob: FUNCTION 00677cb0
	// $deBlob: FUNCTION 00691600
	// $deBlob: FUNCTION 006a9ff0
	// $deBlob: FUNCTION 0070e710
	// $Barnyard: FUNCTION 00678c70
	// $Barnyard: FUNCTION 006797c0
	// $Barnyard: FUNCTION 0063aa00
	// $Barnyard: FUNCTION 006b55e0
	// $Barnyard: FUNCTION 006d9600
	// $Barnyard: FUNCTION 006c7f50
	// $Barnyard: FUNCTION 005a39b0
	// $Barnyard: FUNCTION 0040dca0
	// $Barnyard: FUNCTION 00410a90
	~TNodeList()
	{
		DeleteAll();
	}

	// $deBlob: FUNCTION 006aecf0
	// $deBlob: FUNCTION 007102f0
	// $deBlob: FUNCTION 00710350
	void InsertAfter( T* insertAfter, T* newNode )
	{
		TASSERT( !newNode->IsLinked() );
		newNode->SetList( this );
		newNode->m_pNext              = insertAfter->m_pNext;
		newNode->m_pPrev              = insertAfter;
		insertAfter->m_pNext->m_pPrev = newNode;
		insertAfter->m_pNext          = newNode;
		m_iCount++;
	}

	// $deBlob: FUNCTION 00603380
	// $deBlob: FUNCTION 00677fe0
	// $deBlob: FUNCTION 006ab5e0
	// $deBlob: FUNCTION 006aed50
	// $deBlob: FUNCTION 006b3b70
	// $deBlob: FUNCTION 007103b0
	TFORCEINLINE void InsertBefore( T* insertBefore, T* newNode )
	{
		TASSERT( !newNode->IsLinked() );
		newNode->SetList( this );
		newNode->m_pNext      = insertBefore;
		T* pPrev              = insertBefore->m_pPrev;
		newNode->m_pPrev      = pPrev;
		pPrev->m_pNext        = newNode;
		insertBefore->m_pPrev = newNode;
		m_iCount++;
	}

	// $deBlob: FUNCTION 006034e0
	// $deBlob: FUNCTION 00678100
	// $deBlob: FUNCTION 00691e00
	// $deBlob: FUNCTION 006ab7f0
	// $deBlob: FUNCTION 006aeea0
	// $deBlob: FUNCTION 0070fa10
	// $deBlob: FUNCTION 0070fac0
	// $deBlob: FUNCTION 00710620
	// $deBlob: FUNCTION 00710690
	// $Barnyard: FUNCTION 005a66b0
	T* Remove( T* pNode )
	{
		TASSERT( pNode->GetList() == this );
		pNode->SetList( TNULL );
		pNode->m_pPrev->m_pNext = pNode->m_pNext;
		pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		pNode->m_pNext          = pNode;
		pNode->m_pPrev          = pNode;
		m_iCount--;
		return pNode;
	}

	// $deBlob: FUNCTION 00710700
	// $deBlob: FUNCTION 006aef10
	// $Barnyard: FUNCTION 0061f490
	// $Barnyard: FUNCTION 005c8350
	// $Barnyard: FUNCTION 0040da20
	void RemoveAll()
	{
		while ( !IsEmpty() )
		{
			Remove( m_oRoot.Next() );
		}
	}

	// $Barnyard: FUNCTION 006b4e30
	Iterator RemoveHead()
	{
		if ( !IsEmpty() )
		{
			return Remove( m_oRoot.Next() );
		}

		return TNULL;
	}

	Iterator RemoveTail()
	{
		if ( !IsEmpty() )
		{
			return Remove( m_oRoot.Prev() );
		}

		return TNULL;
	}

	// NOTE: de Blob checks for an empty list
	// $deBlob: FUNCTION 0070fd10
	void DeleteHead()
	{
		if ( !IsEmpty() )
			Delete( Head() );
	}
	void DeleteTail() { Delete( Tail() ); }

	// $deBlob: FUNCTION 006031d0
	// $deBlob: FUNCTION 00677f00
	// $deBlob: FUNCTION 00691870
	// $deBlob: FUNCTION 006aabc0
	// $deBlob: FUNCTION 006aeba0
	// $deBlob: FUNCTION 0070fb80
	// $deBlob: FUNCTION 0070fc40
	// $Barnyard: FUNCTION 005a0190
	// $Barnyard: FUNCTION 00678290
	// $Barnyard: FUNCTION 00679770
	// $Barnyard: FUNCTION 005a3230
	// $Barnyard: FUNCTION 0063a180
	// $Barnyard: FUNCTION 0060a830
	// $Barnyard: FUNCTION 0060a880
	// $Barnyard: FUNCTION 0060a8d0
	// $Barnyard: FUNCTION 0060a8e0
	// $Barnyard: FUNCTION 005f7530
	// $Barnyard: FUNCTION 005f4690
	// $Barnyard: FUNCTION 0040dac0
	// $Barnyard: FUNCTION 0040e320
	// $Barnyard: FUNCTION 004108d0
	void DeleteAll()
	{
		while ( !IsEmpty() )
		{
			DeleteHead();
		}
	}

	// $deBlob: FUNCTION 00677e90
	// $deBlob: FUNCTION 006917e0
	// $deBlob: FUNCTION 006aeaf0
	// $deBlob: FUNCTION 006aab50
	// $deBlob: FUNCTION 00603120
	void Delete( T* pNode )
	{
		Remove( pNode );
		delete pNode;
	}

	// $deBlob: FUNCTION 0070eec0
	// Moves every node of a_rList to the end of this list
	// $Barnyard: FUNCTION 00678c80
	void InsertListTail( TNodeList& a_rList )
	{
		if ( a_rList.IsEmpty() )
			return;

		T* pFirst = a_rList.m_oRoot.m_pNext;
		T* pLast  = a_rList.m_oRoot.m_pPrev;
		T* pTail  = m_oRoot.m_pPrev;

		for ( T* pNode = pFirst; pNode != TSTATICCAST( T, &a_rList.m_oRoot ); pNode = pNode->m_pNext )
			pNode->m_pList = this;

		m_iCount += a_rList.m_iCount;

		pTail->m_pNext  = pFirst;
		pFirst->m_pPrev = pTail;
		pLast->m_pNext  = TSTATICCAST( T, &m_oRoot );
		m_oRoot.m_pPrev = pLast;

		a_rList.m_iCount        = 0;
		a_rList.m_oRoot.m_pPrev = TSTATICCAST( T, &a_rList.m_oRoot );
		a_rList.m_oRoot.m_pNext = TSTATICCAST( T, &a_rList.m_oRoot );
	}

	// $deBlob: FUNCTION 006aedb0
	// $deBlob: FUNCTION 00710410
	// $deBlob: FUNCTION 00710470
	void InsertHead( T* a_pNode ) { InsertAfter( End(), a_pNode ); }
	// $deBlob: FUNCTION 006033e0
	// $deBlob: FUNCTION 00678040
	// $deBlob: FUNCTION 006ab640
	// $deBlob: FUNCTION 006aee10
	// $deBlob: FUNCTION 006b3bd0
	// $deBlob: FUNCTION 007104d0
	void InsertTail( T* a_pNode ) { InsertBefore( End().Get(), a_pNode ); }

	// $deBlob: FUNCTION 006780c0
	// $deBlob: FUNCTION 00691bc0
	// $deBlob: FUNCTION 006ab6c0
	// $Barnyard: FUNCTION 005a66e0
	TBOOL IsEmpty() const { return Begin() == End(); }
	TBOOL IsValid( const T* a_pNode ) const { return a_pNode != TNULL && a_pNode->m_pList == this; }

	TSIZE Count() const { return m_iCount; }

	Iterator Head() const { return m_oRoot.m_pNext; }
	Iterator Tail() const { return m_oRoot.m_pPrev; }

	// $deBlob: FUNCTION 005ebc50
	// $deBlob: FUNCTION 005ebc60
	// $Barnyard: FUNCTION 0040ea70
	// $deBlob: FUNCTION 0070ef20
	// $deBlob: FUNCTION 0070ef10
	// $deBlob: FUNCTION 00677e80
	Iterator Begin() const { return m_oRoot.m_pNext; }
	// $deBlob: FUNCTION 00603280
	// $Barnyard: FUNCTION 00639c30
	// $deBlob: FUNCTION 005ebc90
	// $deBlob: FUNCTION 005ebcb0
	// $Barnyard: FUNCTION 0040d880
	// $deBlob: FUNCTION 00677f80
	Iterator End() const { return TSTATICCAST( T, &m_oRoot ); }

	// $deBlob: FUNCTION 006034d0
	Iterator RBegin() const { return m_oRoot.m_pPrev; }
	Iterator REnd() const { return TSTATICCAST( T, &m_oRoot ); }

protected:
	mutable TNode m_oRoot;
	TINT          m_iCount;
};

TOSHI_NAMESPACE_END

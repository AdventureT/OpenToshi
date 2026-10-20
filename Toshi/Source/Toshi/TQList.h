#pragma once
#include "Toshi/T2Iterator.h"

TOSHI_NAMESPACE_START

template <class T>
class TQList
{
public:
	// TQList's own iterator: unlike T2Iterator, ++ doesn't advance a null iterator
	class Iterator
	{
	public:
		Iterator()
		    : m_pPtr( TNULL )
		{}

		Iterator( T* a_pPtr )
		    : m_pPtr( a_pPtr )
		{}

		T* Get() const
		{
			return m_pPtr;
		}

		T& operator*() const
		{
			return *m_pPtr;
		}

		T* operator->() const
		{
			return m_pPtr;
		}

		operator T*() const
		{
			return m_pPtr;
		}

		// $deBlob: FUNCTION 00560e00
		// $deBlob: FUNCTION 0068f800
		Iterator& operator++()
		{
			TASSERT( m_pPtr != TNULL );

			if ( m_pPtr )
				m_pPtr = m_pPtr->Next();

			return *this;
		}

		// $deBlob: FUNCTION 0068f7b0
		Iterator operator++( TINT )
		{
			TASSERT( m_pPtr != TNULL );
			Iterator old = m_pPtr;

			if ( m_pPtr )
				m_pPtr = m_pPtr->Next();

			return old;
		}

	private:
		T* m_pPtr;
	};

	class TNode
	{
	public:
		friend TQList;
		using Iterator = typename TQList::Iterator;

	public:
		TNode()
		{
			m_pNext = TSTATICCAST( T, this );
			m_pPrev = TSTATICCAST( T, this );
		}

		~TNode()
		{
			m_pPrev->m_pNext = m_pNext;
			m_pNext->m_pPrev = m_pPrev;
			m_pNext          = TSTATICCAST( T, this );
			m_pPrev          = TSTATICCAST( T, this );
		}

		TBOOL IsLinked() const
		{
			return this != m_pNext;
		}

		T* Next() const
		{
			return m_pNext;
		}

		T* Prev() const
		{
			return m_pPrev;
		}

		void Remove()
		{
			m_pPrev->m_pNext = m_pNext;
			m_pNext->m_pPrev = m_pPrev;
			m_pNext          = TSTATICCAST( T, this );
			m_pPrev          = TSTATICCAST( T, this );
		}

	private:
		// $deBlob: FUNCTION 0068f850
		// $deBlob: FUNCTION 0068f8a0
		void InsertAfter( TNode* a_pNode )
		{
			TASSERT( TFALSE == IsLinked() );
			m_pPrev          = TSTATICCAST( T, a_pNode );
			m_pNext          = a_pNode->m_pNext;
			a_pNode->m_pNext = TSTATICCAST( T, this );
			m_pNext->m_pPrev = TSTATICCAST( T, this );
		}

		// $deBlob: FUNCTION 006ef5d0
		// $deBlob: FUNCTION 0068f8f0
		void InsertBefore( TNode* a_pNode )
		{
			TASSERT( TFALSE == IsLinked() );
			m_pNext          = TSTATICCAST( T, a_pNode );
			m_pPrev          = a_pNode->m_pPrev;
			a_pNode->m_pPrev = TSTATICCAST( T, this );
			m_pPrev->m_pNext = TSTATICCAST( T, this );
		}

	private:
		T* m_pNext;
		T* m_pPrev;
	};

public:
	TQList()  = default;
	~TQList() = default;

	// $deBlob: FUNCTION 0068f940
	// $deBlob: FUNCTION 0068f990
	void PushFront( T* a_pNode )
	{
		a_pNode->InsertAfter( &m_oRoot );
	}

	// $deBlob: FUNCTION 006ef620
	// $deBlob: FUNCTION 0068f9e0
	// $deBlob: FUNCTION 006ef710
	void Push( T* a_pNode )
	{
		a_pNode->InsertBefore( &m_oRoot );
	}

	T* Pop()
	{
		auto pFirstElement = m_oRoot.m_pNext;
		pFirstElement->Remove();
		return pFirstElement;
	}

	typename TNode::Iterator Head() const
	{
		return m_oRoot.m_pNext;
	}

	// NOTE: retail folds every instantiation whose node follows the vtable
	// $Barnyard: FUNCTION 0052cc70
	typename TNode::Iterator Begin() const
	{
		return m_oRoot.m_pNext;
	}

	const TNode* End() const
	{
		return &m_oRoot;
	}

	// NOTE: retail folds every instantiation whose node follows the vtable
	// $Barnyard: FUNCTION 0045c640
	void Clear()
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

	void Remove( T* a_pNode )
	{
		a_pNode->Remove();
	}

	void Delete( T* a_pNode )
	{
		a_pNode->Remove();
		delete a_pNode;
	}

	// NOTE: retail compares Begin() with End()
	// $Barnyard: FUNCTION 005fd4f0
	TBOOL IsEmpty() const
	{
		return !m_oRoot.IsLinked();
	}

private:
	TNode m_oRoot;
};

TOSHI_NAMESPACE_END

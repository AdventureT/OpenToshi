#pragma once

#define TARRAY_FOREACH( vecName, iteratorName ) \
	for ( auto iteratorName = vecName.Begin(); !iteratorName.IsOver(); iteratorName++ )

#define TARRAY_FOREACH_BACK( vecName, iteratorName ) \
	for ( auto iteratorName = vecName.Tail(); !iteratorName.IsOver(); iteratorName-- )

TOSHI_NAMESPACE_START

template <class T>
class TGenericArray
{
public:
	class Iterator
	{
	public:
		friend TGenericArray;
		using Storage = TGenericArray;

	public:
		Iterator()
		{
			m_iIndex  = 0;
			m_poArray = TNULL;
		}

		Iterator( const Iterator& other )
		{
			m_poArray = other.m_poArray;
			m_iIndex  = 0;
		}

		Iterator( Storage& a_poArray )
		{
			m_iIndex  = 0;
			m_poArray = &a_poArray;
		}

		Iterator( TINT a_iIndex, Storage& a_poArray )
		{
			m_iIndex  = a_iIndex;
			m_poArray = &a_poArray;
		}

		TINT Index() const
		{
			return m_iIndex;
		}

		void Push( const T& element )
		{
			TASSERT( m_poArray );
			m_poArray->Push( element );
		}

		// $deBlob: FUNCTION 0067a6f0
		// $deBlob: FUNCTION 0067a7a0
		T* operator->() const
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_poArray );
			TASSERT( m_iIndex < m_poArray->m_iNumElements );
			return &m_poArray->m_pData[ m_iIndex ];
		}

		T& Get()
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_poArray );
			TASSERT( m_iIndex < m_poArray->m_iNumElements );
			return m_poArray->m_pData[ m_iIndex ];
		}

		bool IsOver() const
		{
			return ( m_iIndex >= m_poArray->Size() || m_iIndex == -1 );
		}

		const T& Get() const
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_poArray );
			TASSERT( m_iIndex < m_poArray->m_iNumElements );
			return m_poArray->m_pData[ m_iIndex ];
		}

		TBOOL operator==( const Iterator& a_rIt ) const
		{
			return ( m_iIndex == a_rIt.m_iIndex ) && ( m_poArray == a_rIt.m_poArray );
		}

		// NOTE: de Blob's iterator turns into the end iterator (-1) when it steps past the last element
		// $deBlob: FUNCTION 0067a850
		// $deBlob: FUNCTION 0067a8a0
		Iterator& operator++()
		{
			m_iIndex++;
			TASSERT( m_poArray );

			if ( m_iIndex >= m_poArray->m_iNumElements || m_iIndex == 0 )
				m_iIndex = -1;

			return *this;
		}

		Iterator operator++( TINT )
		{
			Iterator temp = *this;
			m_iIndex++;
			return temp;
		}

		Iterator& operator--()
		{
			m_iIndex--;
			return *this;
		}

		Iterator operator--( TINT )
		{
			Iterator temp = *this;
			m_iIndex--;
			return temp;
		}

		Iterator operator+( TINT a_iValue ) const
		{
			return Iterator( m_iIndex + a_iValue, m_poArray );
		}

		Iterator operator-( TINT a_iValue ) const
		{
			return Iterator( m_iIndex - a_iValue, m_poArray );
		}

		// T2SortedVector compatibility
		Iterator( TINT a_iIndex, Storage* a_poArray )
		{
			m_iIndex  = a_iIndex;
			m_poArray = a_poArray;
		}

		T& Value()
		{
			return Get();
		}

		const T& Value() const
		{
			return Get();
		}

		TBOOL operator!=( const Iterator& a_rIt ) const
		{
			return !( *this == a_rIt );
		}

		T& operator*()
		{
			return Get();
		}

		const T& operator*() const
		{
			return Get();
		}

	private:
		TINT     m_iIndex;  // 0x0
		Storage* m_poArray; // 0x4
	};

public:
	// NOTE: de Blob allocates with new[] and has no assert for an empty array without a grow size
	// $deBlob: FUNCTION 0067a410
	// $deBlob: FUNCTION 0067a460
	// $deBlob: FUNCTION 0070e3b0
	// $deBlob: FUNCTION 0070e400
	// $deBlob: FUNCTION 0070e450
	// $deBlob: FUNCTION 0070e4a0
	// $deBlob: FUNCTION 0070e4f0
	TGenericArray( TINT a_iGrowSize, TINT a_iSize )
	{
		m_iGrowSize         = a_iGrowSize;
		m_iNumAllocElements = a_iSize;
		m_iNumElements      = 0;

		if ( m_iNumAllocElements > 0 )
		{
			m_pData = TSTATICCAST( T, TMemalign( TALIGNOF( T ), m_iNumAllocElements * sizeof( T ) ) );
		}
		else
		{
			TASSERT( m_iGrowSize != 0 );
			m_pData = TNULL;
		}
	}

	// $deBlob: FUNCTION 0067a530
	// $Barnyard: FUNCTION 005a2160
	// $deBlob: FUNCTION 0067a540
	~TGenericArray()
	{
		// NOTE: retail calls operator delete[] (see FreeData)
		if ( m_pData )
		{
			::operator delete[]( m_pData );
		}
	}

	// NOTE: retail allocates the new buffer with new[] when T has a constructor and copies the elements one by one
	// $Barnyard: FUNCTION 0057a130
	// $Barnyard: FUNCTION 0057a200
	// $Barnyard: FUNCTION 0059e720
	// $Barnyard: FUNCTION 0059e8f0
	// $Barnyard: FUNCTION 006d9830
	// $Barnyard: FUNCTION 005a4280
	// $Barnyard: FUNCTION 00410b10
	// NOTE: retail allocates with new[]/delete[] (TMemalign( 16 )/TFree) and copies the elements by assignment
	// $deBlob: FUNCTION 0067aba0
	// $deBlob: FUNCTION 0067ac80
	// $deBlob: FUNCTION 007107e0
	// $deBlob: FUNCTION 007108c0
	// $deBlob: FUNCTION 007109a0
	// $deBlob: FUNCTION 00710a80
	// $deBlob: FUNCTION 00710b60
	void Resize( TINT a_iNewSize )
	{
		if ( a_iNewSize != 0 )
		{
			T*     pNewBuffer = TSTATICCAST( T, TMemalign( TALIGNOF( T ), a_iNewSize * sizeof( T ) ) );
			size_t uiCopySize = TMath::Min( m_iNumElements, a_iNewSize );

			TUtil::MemCopy( pNewBuffer, m_pData, sizeof( T ) * uiCopySize );

			m_iNumAllocElements = a_iNewSize;
			TASSERT( m_iNumElements <= m_iNumAllocElements );

			if ( m_pData ) TFree( m_pData );
			m_pData = pNewBuffer;
		}
		else
		{
			FreeData();
		}
	}

	// Frees the buffer and empties the array
	// $deBlob: FUNCTION 0067a950
	// $deBlob: FUNCTION 0067a980
	// $deBlob: FUNCTION 0070ef30
	// $deBlob: FUNCTION 0070ef60
	// $deBlob: FUNCTION 0070ef90
	// $deBlob: FUNCTION 0070efc0
	// $Barnyard: FUNCTION 00410b80
	// $deBlob: FUNCTION 0070eff0
	void FreeData()
	{
		// NOTE: retail calls operator delete[] (no destructors, the elements are destroyed by the callers)
		if ( m_pData ) ::operator delete[]( m_pData );
		m_pData             = TNULL;
		m_iNumAllocElements = 0;
		m_iNumElements      = 0;
	}

	void Clear()
	{
		if ( m_iNumAllocElements < 0 )
		{
			if ( m_pData )
				::operator delete[]( m_pData );

			m_pData             = TNULL;
			m_iNumAllocElements = 0;
			m_iNumElements      = 0;
		}

		m_iNumElements = 0;
	}

	// $deBlob: FUNCTION 0067a8f0
	// $deBlob: FUNCTION 0067a920
	// $Barnyard: FUNCTION 005c9f20
	Iterator Begin()
	{
		return Iterator( ( m_iNumElements == 0 ) ? -1 : 0, *this );
	}

	Iterator Tail()
	{
		return Iterator( ( m_iNumElements == 0 ) ? -1 : m_iNumElements - 1, *this );
	}

	// $Barnyard: FUNCTION 005c9d40
	Iterator End()
	{
		return Iterator( -1, *this );
	}

	// $deBlob: FUNCTION 0067ad80
	TINT Size() const
	{
		return m_iNumElements;
	}

	void SetSize( TINT a_iSize )
	{
		m_iNumElements = a_iSize;
	}

	TINT SizeAllocated() const
	{
		return m_iNumAllocElements;
	}

	// $deBlob: FUNCTION 0070ea70
	TINT GetGrowSize() const
	{
		return m_iGrowSize;
	}

	void SetGrowSize( TINT a_iGrowSize )
	{
		m_iGrowSize = a_iGrowSize;
	}

	// $Barnyard: FUNCTION 0057a310
	// $Barnyard: FUNCTION 0057a920
	// $Barnyard: FUNCTION 005a3890
	// $Barnyard: FUNCTION 005a54e0
	// $deBlob: FUNCTION 0067ab50
	T* Push( const T& element = T() )
	{
		GrowBy( 1 );
		return TConstruct( &m_pData[ m_iNumElements++ ], element );
	}

	template <class... Args>
	T* EmplaceBack( Args&&... args )
	{
		GrowBy( 1 );
		return TConstruct( &m_pData[ m_iNumElements++ ], std::forward<Args>( args )... );
	}

	T& Pop()
	{
		TASSERT( m_iNumElements >= 1 );
		return m_pData[ --m_iNumElements ];
	}

	// $deBlob: FUNCTION 0067a5f0
	// $deBlob: FUNCTION 0067a670
	// $deBlob: FUNCTION 0070e7f0
	// $deBlob: FUNCTION 0070e870
	// $deBlob: FUNCTION 0070e8f0
	// $deBlob: FUNCTION 0070e970
	// $deBlob: FUNCTION 0070e9f0
	T& operator[]( TINT a_iIndex )
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < m_iNumElements );
		return m_pData[ a_iIndex ];
	}

	const T& operator[]( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < m_iNumElements );
		return m_pData[ a_iIndex ];
	}

	// T2SortedVector compatibility
	Iterator Front()
	{
		TASSERT( m_iNumElements > 0 );
		return Iterator( 0, *this );
	}

	Iterator Back()
	{
		TASSERT( m_iNumElements > 0 );
		return Iterator( m_iNumElements - 1, *this );
	}

	TINT Capacity() const
	{
		return m_iNumAllocElements;
	}

	TBOOL IsEmpty() const
	{
		return Size() == 0;
	}

	Iterator InsertBefore( Iterator a_itInsertBefore, const T& a_rcItem = T() )
	{
		TINT iIndex = a_itInsertBefore.Index();
		TASSERT( iIndex >= 0 );
		TASSERT( iIndex <= m_iNumElements );

		InsertGap( iIndex, 1 );
		TConstruct( &m_pData[ iIndex ], a_rcItem );

		return Iterator( iIndex, *this );
	}

	Iterator InsertAfter( Iterator a_itInsertAfter, const T& a_rcItem = T() )
	{
		return InsertBefore( Iterator( a_itInsertAfter.Index() + 1, *this ), a_rcItem );
	}

	void PopBack()
	{
		TASSERT( m_iNumElements >= 1 );
		m_pData[ --m_iNumElements ].~T();
	}

	void PopFront()
	{
		TASSERT( m_iNumElements >= 1 );
		Erase( Begin() );
	}

	Iterator Find( const T& a_rcValue )
	{
		for ( auto it = Begin(); !it.IsOver(); it++ )
		{
			if ( it.Value() == a_rcValue )
				return it;
		}

		return End();
	}

	void Erase( const Iterator& a_rIterator )
	{
		TINT iIndex = a_rIterator.Index();
		TASSERT( iIndex >= 0 );
		TASSERT( iIndex < m_iNumElements );

		m_pData[ iIndex ].~T();

		for ( TINT i = iIndex + 1; i < m_iNumElements; i++ )
		{
			TConstruct( &m_pData[ i - 1 ], std::move( m_pData[ i ] ) );
			m_pData[ i ].~T();
		}

		m_iNumElements--;
	}

	void FindAndErase( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			Erase( it );
	}

	void EraseFast( const Iterator& a_rIterator )
	{
		TINT iIndex = a_rIterator.Index();
		TASSERT( iIndex >= 0 );
		TASSERT( iIndex < m_iNumElements );

		m_pData[ iIndex ].~T();

		if ( iIndex != m_iNumElements - 1 )
		{
			TConstruct( &m_pData[ iIndex ], std::move( m_pData[ m_iNumElements - 1 ] ) );
			m_pData[ m_iNumElements - 1 ].~T();
		}

		m_iNumElements--;
	}

	void FindAndEraseFast( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			EraseFast( it );
	}

	T& At( TINT a_iIndex )
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < m_iNumElements );
		return m_pData[ a_iIndex ];
	}

	const T& At( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < m_iNumElements );
		return m_pData[ a_iIndex ];
	}

private:
	// T2SortedVector compatibility
	void InsertGap( TINT a_iIndex, TINT a_iCount )
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex <= m_iNumElements );
		TASSERT( a_iCount >= 0 );

		if ( a_iCount == 0 )
			return;

		GrowBy( a_iCount );

		for ( TINT i = m_iNumElements - 1; i >= a_iIndex; i-- )
		{
			TConstruct( &m_pData[ i + a_iCount ], std::move( m_pData[ i ] ) );
			m_pData[ i ].~T();
		}

		m_iNumElements += a_iCount;
	}

	// $Barnyard: FUNCTION 00410ad0
	// $deBlob: FUNCTION 0067aa00
	// $deBlob: FUNCTION 0067aa80
	// $deBlob: FUNCTION 00718590
	// $deBlob: FUNCTION 00718610
	// $deBlob: FUNCTION 00718690
	// $deBlob: FUNCTION 00718710
	void GrowBy( TINT a_iGrowBy )
	{
		if ( m_iNumElements + a_iGrowBy > m_iNumAllocElements )
		{
			TASSERT( m_iGrowSize != 0 );

			auto iNewSize = TMath::Max( m_iNumAllocElements + m_iGrowSize, m_iNumElements + a_iGrowBy );
			Resize( iNewSize );
		}
	}

private:
	TINT m_iGrowSize;         // 0x0
	TINT m_iNumElements;      // 0x4
	TINT m_iNumAllocElements; // 0x8
	T*   m_pData;             // 0xC
};

template <class T>
class TArray : public TGenericArray<T>
{
public:
	TArray( TINT a_iGrowSize = 10, TINT a_iSize = 0 )
	    : TGenericArray<T>( a_iGrowSize, a_iSize )
	{
	}
};

TOSHI_NAMESPACE_END

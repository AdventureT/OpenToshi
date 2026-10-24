#pragma once

TOSHI_NAMESPACE_START

template <typename T, TINT MaxSize>
class T2Vector
{
public:
	class Iterator
	{
	public:
		friend class T2Vector;

	public:
		TCONSTEXPR Iterator()
		    : m_iIndex( 0 ), m_pVector( TNULL ) {}
		TCONSTEXPR Iterator( T2Vector* a_pVector )
		    : m_iIndex( 0 ), m_pVector( a_pVector ) {}
		TCONSTEXPR Iterator( TINT a_iIndex, T2Vector* a_pVector )
		    : m_iIndex( a_iIndex ), m_pVector( a_pVector ) {}
		TCONSTEXPR Iterator( const Iterator& a_rOther )
		    : m_iIndex( a_rOther.m_iIndex ), m_pVector( a_rOther.m_pVector ) {}

		TCONSTEXPR TINT Index() const TNOEXCEPT
		{
			return m_iIndex;
		}

		T* Get() TNOEXCEPT
		{
			return &m_pVector->At( m_iIndex );
		}

		const T* Get() const TNOEXCEPT
		{
			return &m_pVector->At( m_iIndex );
		}

		// NOTE: de Blob's iterator checks the index against the capacity, not the size
		T& Value() TNOEXCEPT
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_iIndex < CAPACITY );
			return m_pVector->AtUnsafe( m_iIndex );
		}

		const T& Value() const TNOEXCEPT
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_iIndex < CAPACITY );
			return m_pVector->AtUnsafe( m_iIndex );
		}

		Iterator Next() const
		{
			Iterator temp = *this;
			temp.m_iIndex += 1;
			return temp;
		}

		Iterator Prev() const
		{
			Iterator temp = *this;
			temp.m_iIndex -= 1;
			return temp;
		}

		operator T*()
		{
			return &Value();
		}

		operator const T*() const
		{
			return &Value();
		}

		// $deBlob: FUNCTION 00573eb0
		// $deBlob: FUNCTION 007a3420
		T& operator*() TNOEXCEPT
		{
			return Value();
		}

		const T& operator*() const TNOEXCEPT
		{
			return Value();
		}

		// $deBlob: FUNCTION 00573f30
		// $deBlob: FUNCTION 005c6520
		// $deBlob: FUNCTION 005da270
		// $deBlob: FUNCTION 005dbba0
		// $deBlob: FUNCTION 005eba70
		// $deBlob: FUNCTION 005f6bc0
		// $deBlob: FUNCTION 007a34a0
		T* operator->() TNOEXCEPT
		{
			return &Value();
		}

		const T* operator->() const TNOEXCEPT
		{
			return &Value();
		}

		// NOTE: de Blob compares the index first
		// $deBlob: FUNCTION 005dbb60
		// $Barnyard: FUNCTION 0051bce0
		TBOOL operator==( const Iterator& a_rOther ) const TNOEXCEPT
		{
			return m_iIndex == a_rOther.m_iIndex && m_pVector == a_rOther.m_pVector;
		}

		// $deBlob: FUNCTION 007a3400
		// $deBlob: FUNCTION 005c6500
		// $Barnyard: FUNCTION 00513e50
		// $deBlob: FUNCTION 005eba20
		// $deBlob: FUNCTION 005dbb80
		// $deBlob: FUNCTION 00573e80
		TBOOL operator!=( const Iterator& a_rOther ) const TNOEXCEPT
		{
			return m_iIndex != a_rOther.m_iIndex || m_pVector != a_rOther.m_pVector;
		}

		TBOOL operator>( const Iterator& a_rOther ) const TNOEXCEPT
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex > m_iIndex;
		}

		TBOOL operator>=( const Iterator& a_rOther ) const TNOEXCEPT
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex >= m_iIndex;
		}

		TBOOL operator<( const Iterator& a_rOther ) const TNOEXCEPT
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex < m_iIndex;
		}

		TBOOL operator<=( const Iterator& a_rOther ) const TNOEXCEPT
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex <= m_iIndex;
		}

		TCONSTEXPR Iterator operator++( TINT ) TNOEXCEPT
		{
			Iterator temp = *this;
			m_iIndex++;
			return temp;
		}

		TCONSTEXPR Iterator operator--( TINT ) TNOEXCEPT
		{
			Iterator temp = *this;
			m_iIndex--;
			return temp;
		}

		// $deBlob: FUNCTION 00573fc0
		// $deBlob: FUNCTION 005c65a0
		// $deBlob: FUNCTION 005da300
		// $deBlob: FUNCTION 005dbc20
		// $deBlob: FUNCTION 005ebaf0
		// $deBlob: FUNCTION 005f6d20
		// $deBlob: FUNCTION 007a3520
		Iterator& operator++() TNOEXCEPT
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_iIndex < CAPACITY );
			m_iIndex++;
			return *this;
		}

		TCONSTEXPR Iterator& operator--() TNOEXCEPT
		{
			m_iIndex--;
			return *this;
		}

		// $deBlob: FUNCTION 00573e60
		// $deBlob: FUNCTION 005c64e0
		// $deBlob: FUNCTION 005dbb20
		// $deBlob: FUNCTION 005eba00
		// $deBlob: FUNCTION 007a33e0
		// $Barnyard: FUNCTION 00479c20
		TCONSTEXPR Iterator& operator=( const Iterator& a_rOther ) TNOEXCEPT
		{
			if ( this != &a_rOther )
			{
				m_pVector = a_rOther.m_pVector;
				m_iIndex  = a_rOther.m_iIndex;
			}

			return *this;
		}

		Iterator operator+( TINT a_uiValue ) const TNOEXCEPT
		{
			TASSERT( m_iIndex + a_uiValue < m_pVector->Size() );
			return Iterator( m_iIndex + a_uiValue, m_pVector );
		}

		Iterator operator-( TINT a_uiValue ) const TNOEXCEPT
		{
			TASSERT( m_iIndex - a_uiValue < m_pVector->Size() );
			return Iterator( m_iIndex - a_uiValue, m_pVector );
		}

		// $deBlob: FUNCTION 005f6e10
		Iterator& operator+=( TINT a_uiValue ) TNOEXCEPT
		{
			if ( a_uiValue > 0 )
			{
				TASSERT( m_iIndex >= 0 );
				TASSERT( m_iIndex < CAPACITY - a_uiValue );
			}
			else
			{
				TASSERT( m_iIndex >= a_uiValue );
				TASSERT( m_iIndex < CAPACITY );
			}

			m_iIndex += a_uiValue;
			return *this;
		}

		Iterator& operator-=( TINT a_uiValue ) TNOEXCEPT
		{
			TASSERT( m_iIndex - a_uiValue < m_pVector->Size() );
			m_iIndex -= a_uiValue;
			return *this;
		}

	private:
		TINT      m_iIndex;
		T2Vector* m_pVector;
	};

public:
	static TCONSTEXPR_VAR TINT CAPACITY = MaxSize;

public:
	// de Blob's T2Vector stores its items as a T array: the constructor sets the count, then default-constructs all
	// MaxSize items. Items with a trivial default constructor stay uninitialised. NOTE: our storage is a raw buffer
	// (items are constructed in place by the inserting functions), so the items are default-constructed here
	// explicitly; types without a default constructor are skipped.
	// $deBlob: FUNCTION 005db9d0
	T2Vector()
	{
		m_iNumElements = 0;
		DefaultConstructItems( std::integral_constant < TBOOL, std::is_default_constructible<T>::value && !std::is_trivially_default_constructible<T>::value > () );
	}

	void DefaultConstructItems( std::true_type )
	{
		for ( TINT i = 0; i < MaxSize; i++ )
			new ( &AtUnsafe( i ) ) T;
	}

	void DefaultConstructItems( std::false_type ) {}

	~T2Vector()
	{
		Clear();
	}

	// $deBlob: FUNCTION 0078fbd0
	// $Barnyard: FUNCTION 005ddb60
	Iterator InsertBefore( Iterator a_itInsertBefore, const T& a_rcItem = T(), TBOOL a_bUseMemMove = TTRUE )
	{
		TASSERT( a_itInsertBefore.m_pVector == this );
		TASSERT( a_itInsertBefore.m_iIndex >= 0 && a_itInsertBefore.m_iIndex <= m_iNumElements );

		InsertGap( a_itInsertBefore.Index(), 1, a_bUseMemMove );
		TConstruct<T>( &AtUnsafe( a_itInsertBefore.Index() ), a_rcItem );

		return Iterator( a_itInsertBefore.Index(), this );
	}

	Iterator InsertAfter( Iterator a_itInsertAfter, const T& a_rcItem = T() )
	{
		InsertGap( a_itInsertAfter.Index() + 1, 1 );
		TConstruct<T>( &AtUnsafe( a_itInsertAfter.Index() + 1 ), a_rcItem );

		return Iterator( a_itInsertAfter.Index() + 1, this );
	}

	// $deBlob: FUNCTION 00731120
	// $deBlob: FUNCTION 007e47c0
	// $deBlob: FUNCTION 007ea360
	template <class... Args>
	Iterator EmplaceBack( Args&&... args )
	{
		TASSERT( m_iNumElements < CAPACITY );
		TConstruct<T>( &AtUnsafe( m_iNumElements++ ), std::forward<Args>( args )... );

		return Iterator( m_iNumElements - 1, this );
	}

	// $deBlob: FUNCTION 005741c0
	// $deBlob: FUNCTION 005c6820
	// $deBlob: FUNCTION 005c6870
	// $deBlob: FUNCTION 005da9b0
	// $deBlob: FUNCTION 005dbfc0
	// $deBlob: FUNCTION 005e0570
	// $deBlob: FUNCTION 005ebd10
	// $deBlob: FUNCTION 005f7420
	// $deBlob: FUNCTION 006c27e0
	// $deBlob: FUNCTION 006c2840
	// $deBlob: FUNCTION 007311f0
	// $deBlob: FUNCTION 00731240
	// $deBlob: FUNCTION 00731290
	// $deBlob: FUNCTION 00733ea0
	// $deBlob: FUNCTION 00733ef0
	// $deBlob: FUNCTION 007a36f0
	// $deBlob: FUNCTION 005eb900
	// $Barnyard: FUNCTION 00573510
	void PushBack( const T& item = T() )
	{
		TASSERT( m_iNumElements < CAPACITY );
		TConstruct<T>( &AtUnsafe( m_iNumElements++ ), item );
	}

	// NOTE: de Blob assigns T() to the removed slot
	// $deBlob: FUNCTION 005c6780
	// $deBlob: FUNCTION 005c67d0
	// $deBlob: FUNCTION 006c2720
	// $deBlob: FUNCTION 006c2780
	// $deBlob: FUNCTION 0078fe70
	// $deBlob: FUNCTION 007ea290
	// $Barnyard: FUNCTION 00517bb0
	// $Barnyard: FUNCTION 0057cf00
	void PopBack()
	{
		TASSERT( m_iNumElements > 0 );
		m_iNumElements--;
		AtUnsafe( m_iNumElements ) = T();
	}

	void PopFront()
	{
		TASSERT( m_iNumElements > 0 );
		Erase( Begin() );
	}

	// $Barnyard: FUNCTION 00517c20
	// $Barnyard: FUNCTION 0057cf70
	// $Barnyard: FUNCTION 0057eca0
	void Clear()
	{
		for ( TINT i = 0; i < Size(); i++ )
			AtUnsafe( i ).~T();

		m_iNumElements = 0;
	}

	void Copy( const T2Vector<T, MaxSize>& a_rcCopyFrom )
	{
		TINT i;
		for ( i = 0; i < a_rcCopyFrom.Size(); i++ )
		{
			TConstruct<T>( &AtUnsafe( i ), a_rcCopyFrom.At( i ) );
		}

		if ( Size() > a_rcCopyFrom.Size() )
		{
			for ( ; i < Size(); i++ )
			{
				AtUnsafe( i ).~T();
			}
		}

		m_iNumElements = a_rcCopyFrom.Size();
	}

	// $deBlob: FUNCTION 005dbdd0
	// $Barnyard: FUNCTION 0062fba0
	// $Barnyard: FUNCTION 004a5430
	Iterator Find( const T& a_rValue )
	{
		for ( auto it = Begin(); it != End(); it++ )
		{
			if ( it.Value() == a_rValue )
				return it;
		}

		return End();
	}

	// Erases element preserving order
	// NOTE: de Blob takes the iterator by value, shifts the elements by assignment and assigns T() to the last slot
	// $deBlob: FUNCTION 00574100
	// $deBlob: FUNCTION 005dbd40
	// $deBlob: FUNCTION 00730400
	// $deBlob: FUNCTION 00733d00
	// $deBlob: FUNCTION 00733d80
	// $deBlob: FUNCTION 007a3640
	// $Barnyard: FUNCTION 005141e0
	// $Barnyard: FUNCTION 0051c230
	// $Barnyard: FUNCTION 0062fb60
	// $Barnyard: FUNCTION 005ddba0
	Iterator Erase( Iterator a_itItem )
	{
		TINT iIndex = a_itItem.Index();
		TASSERT( iIndex >= 0 && iIndex < m_iNumElements );

		for ( TINT i = iIndex; i < m_iNumElements - 1; i++ )
			AtUnsafe( i ) = AtUnsafe( i + 1 );

		AtUnsafe( m_iNumElements - 1 ) = T();
		m_iNumElements--;
		return Iterator( iIndex, this );
	}

	// Returns the item if it's stored in the vector or TNULL otherwise (for pointer types)
	// NOTE: a member template so that it's only instantiated when used (VS2013 can't take aligned types by value)
	// $Barnyard: FUNCTION 00573600
	template <class U = T>
	U FindItem( typename std::remove_reference<U>::type a_Item )
	{
		auto it = Find( a_Item );

		if ( it == End() )
			return TNULL;

		return it.Value();
	}

	// Finds and erases element preserving order
	// $Barnyard: FUNCTION 005735a0
	void FindAndErase( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			Erase( it );
	}

	// Erases element ignoring order but with a faster algorithm
	void EraseFast( Iterator& a_rIterator )
	{
		TINT uiItemIndex = a_rIterator.Index();
		TASSERT( uiItemIndex < m_iNumElements );

		a_rIterator.Value() = Back().Value();
		PopBack();
	}

	// Finds and erases element ignoring order but with a faster algorithm
	void FindAndEraseFast( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			EraseFast( it );
	}

	TINT Size() const
	{
		return m_iNumElements;
	}

	TCONSTEXPR TINT Capacity() const
	{
		return CAPACITY;
	}

	TBOOL IsEmpty() const
	{
		return Size() == 0;
	}

	Iterator Front()
	{
		TASSERT( m_iNumElements > 0 );
		return Iterator( 0, this );
	}

	// $deBlob: FUNCTION 005c6610
	// $deBlob: FUNCTION 005c6950
	// $deBlob: FUNCTION 006c24b0
	// $deBlob: FUNCTION 006c2b80
	T& Back()
	{
		TASSERT( m_iNumElements > 0 );
		return AtUnsafe( m_iNumElements - 1 );
	}

	// $deBlob: FUNCTION 00574060
	// $deBlob: FUNCTION 005da550
	// $deBlob: FUNCTION 005dbcc0
	// $deBlob: FUNCTION 005ebc30
	// $deBlob: FUNCTION 005f6f60
	// $deBlob: FUNCTION 007a35d0
	// $deBlob: FUNCTION 005c6650
	// $deBlob: FUNCTION 005c6670
	// $Barnyard: FUNCTION 00429680
	TCONSTEXPR Iterator Begin()
	{
		return Iterator( this );
	}

	// Returns an iterator to the element at a_iIndex
	// NOTE: Retail folds it with the same helper of the other index based containers and calls it e.g. from
	// T2SortedVector::FindInsertionPoint and ACollisionModel::CollideRay
	// $deBlob: FUNCTION 00731070
	// $deBlob: FUNCTION 00733e00
	// $deBlob: FUNCTION 00733e50
	// $deBlob: FUNCTION 0078fdd0
	// $Barnyard: FUNCTION 0061a720
	Iterator GetIterator( TINT a_iIndex )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex <= m_iNumElements );
		return Iterator( a_iIndex, this );
	}

	// $deBlob: FUNCTION 005740e0
	// $deBlob: FUNCTION 005c66b0
	// $deBlob: FUNCTION 005da600
	// $deBlob: FUNCTION 005dbd20
	// $deBlob: FUNCTION 005ebc70
	// $deBlob: FUNCTION 005f6fb0
	// $deBlob: FUNCTION 0078fa10
	// $deBlob: FUNCTION 007a3620
	// $Barnyard: FUNCTION 00575090
	// $deBlob: FUNCTION 005c6690
	// $Barnyard: FUNCTION 004296a0
	TCONSTEXPR Iterator End()
	{
		return Iterator( m_iNumElements, this );
	}

	T& At( TINT a_iIndex )
	{
		TASSERT( a_iIndex < m_iNumElements );
		return *(T*)( m_aBuffer + a_iIndex * sizeof( T ) );
	}

	const T& At( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iNumElements );
		return *(const T*)( m_aBuffer + a_iIndex * sizeof( T ) );
	}

	T& operator[]( TINT a_iIndex )
	{
		return At( a_iIndex );
	}

	const T& operator[]( TINT a_iIndex ) const
	{
		return At( a_iIndex );
	}

private:
	TFORCEINLINE TCONSTEXPR T& AtUnsafe( TINT a_iIndex )
	{
		return *( (T*)( m_aBuffer ) + a_iIndex );
	}

	TFORCEINLINE TCONSTEXPR const T& AtUnsafe( TINT a_iIndex ) const
	{
		return *( (T*)( m_aBuffer ) + a_iIndex );
	}

	// $Barnyard: FUNCTION 00639ba0
	// $deBlob: FUNCTION 0078fc90
	// $Barnyard: FUNCTION 005dd9d0
	void InsertGap( TINT a_iGapAt, TINT a_iGapSize, TBOOL a_bUseMemMove = TTRUE )
	{
		TASSERT( a_iGapAt >= 0 && a_iGapAt <= m_iNumElements );
		TASSERT( m_iNumElements + a_iGapSize <= CAPACITY );

		if ( m_iNumElements > a_iGapAt )
		{
			if ( a_bUseMemMove )
			{
				// Unsafe way but suitable with simple types
				TUtil::MemMove( &AtUnsafe( a_iGapAt + a_iGapSize ), &AtUnsafe( a_iGapAt ), ( m_iNumElements - a_iGapAt ) * sizeof( T ) );
			}
			else
			{
				// This is much safer
				for ( TINT i = m_iNumElements - 1, k = i + a_iGapSize; i >= a_iGapAt; i--, k-- )
				{
					TConstruct<T>( &AtUnsafe( k ), std::move( AtUnsafe( i ) ) );
					AtUnsafe( i ).~T();
				}
			}
		}

		m_iNumElements += a_iGapSize;
	}

private:
	TINT  m_iNumElements;
	TBYTE m_aBuffer[ MaxSize * sizeof( T ) ];
};

TOSHI_NAMESPACE_END

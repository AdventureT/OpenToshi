/**
 * @file T2DynamicVector.h
 * Dynamic vector container for the Toshi engine
 * 
 * This file provides a dynamic vector implementation that includes:
 * - Dynamic array management
 * - Memory allocation and deallocation
 * - Element access and manipulation
 * - Vector operations and utilities
 */

#pragma once
#include "Toshi/T2Allocator.h"

TOSHI_NAMESPACE_START

class TOSHI_API T2GenericDynamicVector
{
protected:
	// $deBlob: FUNCTION 006f3870
	// $Barnyard: FUNCTION 006cbce0
	T2GenericDynamicVector( T2Allocator* a_pAllocator, TINT a_iInitialSize, TINT a_iGrowSize, TINT a_iElementSize )
	{
		m_pAllocator   = a_pAllocator;
		m_iGrowSize    = a_iGrowSize;
		m_iNumElements = 0;
		m_iAllocSize   = a_iInitialSize;
		m_poElements   = ( a_iInitialSize > 0 ) ? a_pAllocator->Malloc( a_iInitialSize * a_iElementSize ) : TNULL;
	}

	// $deBlob: FUNCTION 006f38c0
	// $Barnyard: FUNCTION 006cbd20
	T2GenericDynamicVector( void* a_poElements, TINT a_iInitialSize, TINT a_iNumElements )
	{
		m_pAllocator   = TNULL;
		m_iGrowSize    = 0;
		m_iNumElements = a_iNumElements;
		m_iAllocSize   = a_iInitialSize;
		m_poElements   = a_poElements;
	}

	~T2GenericDynamicVector()
	{
		TASSERT( m_poElements == TNULL );
	}

	void Reallocate( TINT a_iNewSize, TINT a_iElementSize );
	void Grow( TINT a_iNumElements, TINT a_iElementSize );

	// Opens a gap of a_iGapSize elements at a_iIndexGap (memmoves the elements), returns the new size of the gap
	// NOTE: the templated vector has its own InsertGap, which constructs the moved elements
	TINT InsertGap( TINT a_iIndexGap, TINT a_iGapSize, TINT a_iElementSize );

protected:
	T2Allocator* m_pAllocator;   // 0x0
	TINT         m_iGrowSize;    // 0x4
	TINT         m_iNumElements; // 0x8
	TINT         m_iAllocSize;   // 0xC
	void*        m_poElements;   // 0x10
};

template <class T>
class T2DynamicVector : public T2GenericDynamicVector
{
public:
	class Iterator
	{
	public:
		friend T2DynamicVector;

	public:
		TCONSTEXPR Iterator()
		    : m_iIndex( 0 ), m_pVector( TNULL ) {}
		TCONSTEXPR Iterator( T2DynamicVector* a_pVector )
		    : m_iIndex( 0 ), m_pVector( a_pVector ) {}
		TCONSTEXPR Iterator( T2DynamicVector& a_rVector )
		    : m_iIndex( 0 ), m_pVector( &a_rVector ) {}
		TCONSTEXPR Iterator( TINT a_iIndex, T2DynamicVector* a_pVector )
		    : m_iIndex( a_iIndex ), m_pVector( a_pVector ) {}
		TCONSTEXPR Iterator( const Iterator& a_rOther )
		    : m_iIndex( a_rOther.m_iIndex ), m_pVector( a_rOther.m_pVector ) {}

		TCONSTEXPR TINT Index() const
		{
			return m_iIndex;
		}

		TBOOL IsValid() const
		{
			return m_pVector && *this != m_pVector->End();
		}

		// NOTE: de Blob's iterator checks the index against the size
		T* Get() TNOEXCEPT
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_iIndex < m_pVector->m_iNumElements );
			return &m_pVector->AtUnsafe( m_iIndex );
		}

		const T* Get() const TNOEXCEPT
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_iIndex < m_pVector->m_iNumElements );
			return &m_pVector->AtUnsafe( m_iIndex );
		}

		T& Value()
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_iIndex < m_pVector->m_iNumElements );
			return m_pVector->AtUnsafe( m_iIndex );
		}

		const T& Value() const
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_iIndex < m_pVector->m_iNumElements );
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

		// $deBlob: FUNCTION 005b0100
		// $deBlob: FUNCTION 005b0180
		// $deBlob: FUNCTION 005c0640
		// $deBlob: FUNCTION 006f4390
		// $deBlob: FUNCTION 007eb630
		T* operator->()
		{
			return &Value();
		}

		const T* operator->() const
		{
			return &Value();
		}

		// $deBlob: FUNCTION 005b0080
		// $deBlob: FUNCTION 005c05c0
		// $deBlob: FUNCTION 007eb5b0
		T& operator*() TNOEXCEPT
		{
			return Value();
		}

		const T& operator*() const TNOEXCEPT
		{
			return Value();
		}

		TBOOL operator==( const Iterator& a_rOther ) const TNOEXCEPT
		{
			return a_rOther.m_pVector == m_pVector && a_rOther.m_iIndex == m_iIndex;
		}

		// $deBlob: FUNCTION 005b0030
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

		TCONSTEXPR Iterator& operator=( const Iterator& a_rOther ) TNOEXCEPT
		{
			m_iIndex  = a_rOther.m_iIndex;
			m_pVector = a_rOther.m_pVector;
			return *this;
		}

		// $deBlob: FUNCTION 005b0200
		// $deBlob: FUNCTION 005b0280
		// $deBlob: FUNCTION 005c0710
		// $deBlob: FUNCTION 006f4410
		// $deBlob: FUNCTION 007eb6b0
		Iterator& operator++()
		{
			TASSERT( m_iIndex >= 0 );
			TASSERT( m_iIndex < m_pVector->m_iNumElements );
			m_iIndex++;
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
			return Iterator( m_iIndex + a_iValue, m_pVector );
		}

		Iterator operator-( TINT a_iValue ) const
		{
			return Iterator( m_iIndex - a_iValue, m_pVector );
		}

	private:
		TINT             m_iIndex;  // 0x0
		T2DynamicVector* m_pVector; // 0x4
	};

public:
	// $deBlob: FUNCTION 005cb3c0
	// $deBlob: FUNCTION 0072d220
	// $deBlob: FUNCTION 0072d240
	// $deBlob: FUNCTION 005afb40
	// $deBlob: FUNCTION 005afb60
	// $deBlob: FUNCTION 005afb80
	// $deBlob: FUNCTION 005afba0
	// $deBlob: FUNCTION 005afbc0
	// $deBlob: FUNCTION 005afbe0
	T2DynamicVector( T2Allocator* a_pAllocator = GetGlobalAllocator(), TINT a_iInitialSize = 0, TINT a_iGrowSize = -1 )
	    : T2GenericDynamicVector( a_pAllocator, a_iInitialSize, a_iGrowSize, sizeof( T ) )
	{}

	T2DynamicVector( T* a_poElements, TINT a_iInitialSize, TINT a_iNumElements )
	    : T2GenericDynamicVector( a_poElements, a_iInitialSize, a_iNumElements )
	{}

	// $deBlob: FUNCTION 005afd50
	// $deBlob: FUNCTION 005afde0
	// $deBlob: FUNCTION 005afea0
	// $deBlob: FUNCTION 005c0370
	// $deBlob: FUNCTION 005cb3f0
	// $deBlob: FUNCTION 006f42d0
	// $deBlob: FUNCTION 0072e1d0
	// $deBlob: FUNCTION 0072e230
	// $deBlob: FUNCTION 0072f6c0
	// $deBlob: FUNCTION 0072f730
	// $deBlob: FUNCTION 007e9560
	// $Barnyard: FUNCTION 0059dce0
	// $Barnyard: FUNCTION 00469fd0
	~T2DynamicVector()
	{
		FreeMemory();
	}

	Iterator InsertBefore( Iterator a_itInsertBefore, const T& a_rcItem = T() )
	{
		InsertGap( a_itInsertBefore.Index(), 1, sizeof( T ) );
		TConstruct<T>( &AtUnsafe( a_itInsertBefore.Index() ), a_rcItem );

		return Iterator( a_itInsertBefore.Index(), this );
	}

	Iterator InsertAfter( Iterator a_itInsertAfter, const T& a_rcItem = T() )
	{
		InsertGap( a_itInsertAfter.Index() + 1, 1, sizeof( T ) );
		TConstruct<T>( &AtUnsafe( a_itInsertAfter.Index() + 1 ), a_rcItem );

		return Iterator( a_itInsertAfter.Index() + 1, this );
	}

	// $deBlob: FUNCTION 005b0460
	// $Barnyard: FUNCTION 006ce330
	void Clear()
	{
		for ( TINT i = 0; i < m_iNumElements; i++ )
		{
			( (T*)m_poElements + i )->~T();
		}

		m_iNumElements = 0;
	}

	// $Barnyard: FUNCTION 0047b230
	void FreeMemory()
	{
		Clear();

		if ( m_pAllocator )
			Reallocate( 0, sizeof( T ) );
		else
			m_poElements = TNULL;
	}

	void Reserve( TINT a_iSize )
	{
		if ( a_iSize > m_iAllocSize )
		{
			Reallocate( a_iSize, sizeof( T ) );
		}
	}

	// Reallocates to a_iSize elements, destroying the ones beyond it
	// $deBlob: FUNCTION 005b0c70
	// $deBlob: FUNCTION 005b0cc0
	// $deBlob: FUNCTION 005b0d10
	void SetCapacity( TINT a_iSize )
	{
		if ( a_iSize < m_iNumElements )
			Erase( Iterator( a_iSize, this ), End() );

		Reallocate( a_iSize, sizeof( T ) );
	}

	// $deBlob: FUNCTION 005b0a30
	// $deBlob: FUNCTION 005b0ad0
	template <class... Args>
	T* EmplaceBack( Args&&... args )
	{
		Grow( 1, sizeof( T ) );
		return TConstruct<T>( &AtUnsafe( m_iNumElements++ ), std::forward<Args>( args )... );
	}

	// $Barnyard: FUNCTION 0059db20
	// $Barnyard: FUNCTION 00616580
	// NOTE: de Blob's instances return nothing; the reference is kept for the callers that use it
	// $deBlob: FUNCTION 005b0aa0
	// $deBlob: FUNCTION 005c0ab0
	// $deBlob: FUNCTION 005cb5a0
	// $deBlob: FUNCTION 006f4570
	// $deBlob: FUNCTION 00731190
	// $deBlob: FUNCTION 007311c0
	// $deBlob: FUNCTION 007e9720
	T& PushBack( const T& item = T() )
	{
		Grow( 1, sizeof( T ) );
		return *TConstruct<T>( &AtUnsafe( m_iNumElements++ ), item );
	}

	T& PushBack( T&& item )
	{
		Grow( 1, sizeof( T ) );
		return *TConstruct<T>( &AtUnsafe( m_iNumElements++ ), item );
	}

	// $deBlob: FUNCTION 006f4530
	void PopBack()
	{
		TASSERT( m_iNumElements > 0 );
		AtUnsafe( --m_iNumElements ).~T();
	}

	// $Barnyard: FUNCTION 0062a0a0
	Iterator Find( const T& a_rcValue )
	{
		for ( auto it = Begin(); it != End(); it++ )
		{
			if ( it.Value() == a_rcValue )
				return it;
		}

		return End();
	}

	// Erases element preserving order
	// $Barnyard: FUNCTION 005a22a0
	// $Barnyard: FUNCTION 0062a110
	// NOTE: de Blob takes the iterator by value and moves the rest down with MemMove
	// $deBlob: FUNCTION 005b0590
	// $deBlob: FUNCTION 005b06f0
	// $deBlob: FUNCTION 00730300
	// $deBlob: FUNCTION 00730380
	Iterator Erase( Iterator a_itItem )
	{
		TINT iIndex = a_itItem.Index();
		TASSERT( iIndex >= 0 && iIndex < m_iNumElements );

		AtUnsafe( iIndex ).~T();

		if ( m_iNumElements > 1 )
			TUtil::MemMove( &AtUnsafe( iIndex ), &AtUnsafe( iIndex + 1 ), ( m_iNumElements - iIndex - 1 ) * sizeof( T ) );

		m_iNumElements--;
		return Iterator( iIndex, this );
	}

	// Finds and erases element preserving order
	void FindAndErase( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			Erase( it );
	}

	// Erases element ignoring order but with a faster algorithm
	void EraseFast( const Iterator& a_rIterator )
	{
		TINT uiItemIndex = a_rIterator.Index();
		TASSERT( uiItemIndex < m_iNumElements );

		AtUnsafe( uiItemIndex ) = Back().Value();
		PopBack();
	}

	// Finds and erases element ignoring order but with a faster algorithm
	void FindAndEraseFast( const T& a_rcItem )
	{
		auto it = Find( a_rcItem );

		if ( it != End() )
			EraseFast( it );
	}

	Iterator Front()
	{
		TASSERT( m_iNumElements > 0 );
		return Iterator( 0, this );
	}

	// $deBlob: FUNCTION 006f4490
	T& Back()
	{
		TASSERT( m_iNumElements > 0 );
		return AtUnsafe( m_iNumElements - 1 );
	}

	// $deBlob: FUNCTION 005b0390
	// $deBlob: FUNCTION 005b03b0
	// $deBlob: FUNCTION 005c07e0
	// $deBlob: FUNCTION 006f44e0
	// $deBlob: FUNCTION 007eb740
	Iterator Begin()
	{
		return Iterator( 0, this );
	}

	// NOTE: retail folds every instantiation de Blob instances:
	// $deBlob: FUNCTION 005c0840
	// $deBlob: FUNCTION 006f4510
	// $deBlob: FUNCTION 005b04a0
	// $deBlob: FUNCTION 005b04c0
	// $deBlob: FUNCTION 007eb760
	// $Barnyard: FUNCTION 005d6a40
	Iterator End()
	{
		return Iterator( m_iNumElements, this );
	}

	// $deBlob: FUNCTION 00730fd0
	// $deBlob: FUNCTION 00731020
	Iterator MakeIterator( TINT a_iIndex )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex <= m_iNumElements );
		return Iterator( a_iIndex, this );
	}

	TINT Size() const
	{
		return m_iNumElements;
	}

	// Destroys the elements in the [a_itFrom; a_itTo) range and moves the rest down
	// $Barnyard: FUNCTION 005a2250
	// $Barnyard: FUNCTION 006ce1e0
	// $deBlob: FUNCTION 005b04e0
	// $deBlob: FUNCTION 005b0610
	// $deBlob: FUNCTION 005b0780
	Iterator Erase( Iterator a_itFrom, Iterator a_itTo )
	{
		TINT iFirstIndex = a_itFrom.Index();
		TINT iLastIndex  = a_itTo.Index();
		TASSERT( iFirstIndex >= 0 && iFirstIndex < m_iNumElements );
		TASSERT( iLastIndex <= m_iNumElements );

		for ( TINT i = iFirstIndex; i < iLastIndex; i++ )
			AtUnsafe( i ).~T();

		TUtil::MemMove( &AtUnsafe( iFirstIndex ), &AtUnsafe( iLastIndex ), sizeof( T ) * ( m_iNumElements - iLastIndex ) );
		m_iNumElements += iFirstIndex - iLastIndex;

		return Iterator( iFirstIndex, this );
	}

	// $Barnyard: FUNCTION 006ce280
	template <class... Args>
	void SetSize( TINT a_iNewSize, Args&&... args )
	{
		if ( a_iNewSize < m_iNumElements )
			Erase( Iterator( a_iNewSize, this ), Iterator( m_iNumElements, this ) );
		else if ( a_iNewSize > m_iAllocSize )
			Reallocate( a_iNewSize, sizeof( T ) );

		for ( TINT i = m_iNumElements; i < a_iNewSize; i++ )
			new ( &AtUnsafe( i ) ) T( std::forward<Args>( args )... );

		m_iNumElements = a_iNewSize;
	}

	TINT Capacity() const
	{
		return m_iAllocSize;
	}

	TBOOL IsEmpty() const
	{
		return Size() == 0;
	}

	T*       Data() { return TREINTERPRETCAST( T*, m_poElements ); }
	const T* Data() const { return TREINTERPRETCAST( const T*, m_poElements ); }

	T& At( TINT a_iIndex )
	{
		TASSERT( a_iIndex < m_iNumElements );
		return *( TREINTERPRETCAST( T*, m_poElements ) + a_iIndex );
	}

	const T& At( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iNumElements );
		return *( TREINTERPRETCAST( T*, m_poElements ) + a_iIndex );
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
		return *( TREINTERPRETCAST( T*, m_poElements ) + a_iIndex );
	}

	TFORCEINLINE TCONSTEXPR const T& AtUnsafe( TINT a_iIndex ) const
	{
		return *( TREINTERPRETCAST( T*, m_poElements ) + a_iIndex );
	}

	void InsertGap( TINT a_iGapAt, TINT a_iGapSize, TINT a_iElementSize, TBOOL a_bUseMemMove = TFALSE )
	{
		if ( m_iAllocSize < m_iNumElements + a_iGapSize )
		{
			TINT iNewSize = m_iAllocSize + a_iGapSize;

			if ( m_iGrowSize == -1 )
				iNewSize = TMath::Max( iNewSize, m_iAllocSize * 2 );
			else
				iNewSize = TMath::Max( iNewSize, m_iAllocSize + m_iGrowSize );

			TCHAR* pNewBuffer = (TCHAR*)m_pAllocator->Malloc( iNewSize * a_iElementSize );

			if ( m_poElements )
			{
				TINT iNumElementsBeforeGap = a_iGapAt;
				TINT iNumElementsAfterGap  = m_iNumElements - a_iGapAt;

				if ( a_bUseMemMove )
				{
					// Unsafe way but suitable with simple types

					// Copy elements before the gap
					if ( iNumElementsBeforeGap > 0 )
						TUtil::MemCopy( pNewBuffer, m_poElements, iNumElementsBeforeGap * a_iElementSize );

					// Copy elements after the gap
					if ( iNumElementsAfterGap > 0 )
						TUtil::MemCopy( pNewBuffer + ( a_iGapAt + a_iGapSize ) * a_iElementSize, TREINTERPRETCAST( TCHAR*, m_poElements ) + a_iGapAt * a_iElementSize, iNumElementsAfterGap * a_iElementSize );
				}
				else
				{
					// This is much safer

					// Copy elements before the gap
					for ( TINT i = 0; i < iNumElementsBeforeGap; i++ )
					{
						TConstruct<T>( TREINTERPRETCAST( T*, pNewBuffer + ( i * a_iElementSize ) ), std::move( AtUnsafe( i ) ) );
						AtUnsafe( i ).~T();
					}

					// Copy elements after the gap
					for ( TINT i = a_iGapAt + a_iGapSize, k = a_iGapAt; k < m_iNumElements; i++, k++ )
					{
						TConstruct<T>( TREINTERPRETCAST( T*, pNewBuffer + ( i * a_iElementSize ) ), std::move( AtUnsafe( k ) ) );
						AtUnsafe( k ).~T();
					}
				}

				// Free the old buffer
				m_pAllocator->Free( m_poElements );
			}

			m_poElements   = pNewBuffer;
			m_iAllocSize   = iNewSize;
			m_iNumElements = m_iNumElements + a_iGapSize;

			return;
		}
		else if ( m_iNumElements > a_iGapAt )
		{
			if ( a_bUseMemMove )
			{
				// Unsafe way but suitable with simple types
				// Note: Seems that in Barnyard there's no other way of adding a gap but in newer Toshi branches there is so I'm adding it here too anyways
				TUtil::MemMove( &AtUnsafe( a_iGapAt + a_iGapSize ), &AtUnsafe( a_iGapAt ), ( m_iNumElements - a_iGapAt ) * a_iElementSize );
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
};

TOSHI_NAMESPACE_END

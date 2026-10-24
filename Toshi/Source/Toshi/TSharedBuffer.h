#pragma once
#include "Toshi/Typedefs.h"
#include "Toshi/TArray.h"
#include <type_traits>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Use this to create shared buffer of type T.
// Call TSharedBuffer<>::Create() specifying number of buffers to create.
// Note: Max possible number of elements is BUFSIZE * m_iNumBuffers.
//-----------------------------------------------------------------------------
template <typename T, TINT BUFSIZE, TINT IDENTIFIER>
class TSharedBuffer
{
public:
	class TNode
	{
	public:
		// $deBlob: FUNCTION 007182e0
		// $deBlob: FUNCTION 00718330
		// $deBlob: FUNCTION 00718380
		// $deBlob: FUNCTION 0071af10
		T& GetElement( TINT a_iIndex )
		{
			TASSERT( a_iIndex < BUFSIZE );
			return TREINTERPRETCAST( T*, m_aMemoryBuffer )[ a_iIndex ];
		}

		const T& GetElement( TINT a_iIndex ) const
		{
			return GetElement( a_iIndex );
		}

		T& operator[]( TINT a_iIndex )
		{
			return GetElement( a_iIndex );
		}

		const T& operator[]( TINT a_iIndex ) const
		{
			return GetElement( a_iIndex );
		}

	private:
		// NOTE: a union of the buffer and T m_aElements[ BUFSIZE ] before; the elements are reached through the buffer
		// as VS2013 has no unions of classes with constructors (the alignment member keeps T's alignment)
		union
		{
			TUINT8                                                          m_aMemoryBuffer[ sizeof( T ) * BUFSIZE ];
			typename std::aligned_storage<sizeof( T ), TALIGNOF( T )>::type m_oAlignment;
		};
	};

public:
	//-----------------------------------------------------------------------------
	// Static definitions
	//-----------------------------------------------------------------------------

	// $deBlob: FUNCTION 0070f420
	// $deBlob: FUNCTION 0070f810
	// $deBlob: FUNCTION 0070f040
	// $deBlob: FUNCTION 0070f230
	// $deBlob: FUNCTION 0070f610
	static void CreateSharedBuffers( TINT a_iNumBuffers )
	{
		TASSERT( m_iNumBuffers == 0 && "m_iNumBuffers== 0" );
		TASSERT( m_pNodeArray == TNULL );

		m_iNumBuffers = a_iNumBuffers;

		if ( a_iNumBuffers > m_oFreeList.SizeAllocated() )
		{
			m_oFreeList.Resize( a_iNumBuffers );
		}

		m_oFreeList.SetSize( a_iNumBuffers );
		m_pNodeArray = TSTATICCAST( TNode, TMemalign( 16, sizeof( TNode ) * m_iNumBuffers ) );

		for ( TINT i = 0; i < m_iNumBuffers; i++ )
		{
			m_oFreeList[ i ] = &m_pNodeArray[ i ];
		}

		TASSERT( m_oFreeList.Size() <= m_iNumBuffers );
		TASSERT( ( 1 << TMath::IntLog2( BUFSIZE ) ) == BUFSIZE );

		m_iMainIndexShift = TMath::IntLog2( BUFSIZE );
		m_iSubIndexMask   = ( 1 << TMath::IntLog2( BUFSIZE ) ) - 1;
	}

	// $deBlob: FUNCTION 0070ff10
	// $deBlob: FUNCTION 007100c0
	// $deBlob: FUNCTION 0070fd70
	// $deBlob: FUNCTION 0070fe40
	// $deBlob: FUNCTION 0070fff0
	static void DestroySharedBuffers()
	{
		TASSERT( m_oFreeList.Size() <= m_iNumBuffers );
		TASSERT( TFALSE == IsAnyBuffersUsed() );

		// NOTE: called again after the buffers are destroyed (TParticleSystem's particle nodes), so nothing is
		// asserted about the node array
		m_oFreeList.FreeData();
		TFree( m_pNodeArray );

		m_iNumBuffers = 0;
		m_pNodeArray  = TNULL;
	}

	static TBOOL IsAnyBuffersUsed()
	{
		return m_iNumBuffers != m_oFreeList.Size();
	}

private:
	static TNode*         m_pNodeArray;
	static TINT           m_iNumBuffers;
	static TINT           m_iMainIndexShift;
	static TINT           m_iSubIndexMask;
	static TArray<TNode*> m_oFreeList;

public:
	//-----------------------------------------------------------------------------
	// Member definitions
	//-----------------------------------------------------------------------------

	// $deBlob: FUNCTION 00717b70
	// $deBlob: FUNCTION 00717c00
	// $deBlob: FUNCTION 00717c90
	// $deBlob: FUNCTION 00717d20
	TSharedBuffer()
	    : m_oArray( 4, 10 )
	{
		TASSERT( ( 1 << TMath::IntLog2( BUFSIZE ) ) == BUFSIZE );
	}

	~TSharedBuffer()
	{
		SetSize( 0 );
	}

	TUINT GetSize() const
	{
		return m_oArray.Size() << m_iMainIndexShift;
	}

	// $deBlob: FUNCTION 00719180
	// $deBlob: FUNCTION 007194a0
	// $deBlob: FUNCTION 00718b40
	// $deBlob: FUNCTION 00718e60
	TBOOL SetSize( TINT a_iSize )
	{
		TASSERT( a_iSize >= 0 );
		TASSERT( m_oFreeList.Size() <= m_iNumBuffers );
		TASSERT( TMath::IntLog2( BUFSIZE ) == m_iMainIndexShift );

		TINT iDesiredNumNodes = a_iSize >> ( m_iMainIndexShift % BUFSIZE );

		if ( 0 < ( m_iSubIndexMask & a_iSize ) )
		{
			iDesiredNumNodes += 1;
		}

		TASSERT( iDesiredNumNodes >= 0 );

		if ( m_oArray.Size() != iDesiredNumNodes )
		{
			if ( 10 < iDesiredNumNodes ) return TFALSE;

			if ( iDesiredNumNodes < m_oArray.Size() )
			{
				// Removing elements from m_oArray

				while ( iDesiredNumNodes < m_oArray.Size() )
				{
					// Remove the last element from array and store in the freelist
					m_oFreeList.Push( m_oArray.Pop() );
				}

				TASSERT( m_oArray.Size() == iDesiredNumNodes );
				TASSERT( m_oFreeList.Size() <= m_iNumBuffers );
			}
			else
			{
				// Adding new elements to m_oArray

				if ( m_oFreeList.Size() < 1 ) return TFALSE;

				while ( m_oArray.Size() < iDesiredNumNodes )
				{
					if ( m_oFreeList.Size() < 1 ) return TFALSE;

					TASSERT( m_oArray.Size() < m_oArray.SizeAllocated() );

					m_oArray.Push( m_oFreeList.Pop() );
				}

				TASSERT( m_oArray.Size() == iDesiredNumNodes );
				TASSERT( m_oFreeList.Size() <= m_iNumBuffers );
			}
		}

		return TTRUE;
	}

	// $deBlob: FUNCTION 00717fc0
	// $deBlob: FUNCTION 00718150
	// $deBlob: FUNCTION 0071ad80
	// $deBlob: FUNCTION 00717e30
	T& At( TINT a_iIndex )
	{
		TASSERT( TMath::IntLog2( BUFSIZE ) == m_iMainIndexShift );
		TASSERT( m_oFreeList.Size() <= m_iNumBuffers );
		TASSERT( a_iIndex < m_oArray.Size() * BUFSIZE );

		TINT iSubIndex  = m_iSubIndexMask & a_iIndex;
		TINT iMainIndex = a_iIndex >> ( m_iMainIndexShift % BUFSIZE );

		TASSERT( iMainIndex < m_oArray.Size() );
		TASSERT( iSubIndex < BUFSIZE );

		return m_oArray[ iMainIndex ]->GetElement( iSubIndex );
	}

	const T& At( TINT a_iIndex ) const
	{
		TASSERT( TMath::IntLog2( BUFSIZE ) == m_iMainIndexShift );
		TASSERT( m_oFreeList.Size() <= m_iNumBuffers );
		TASSERT( a_iIndex < m_oArray.Size() * BUFSIZE );

		TINT iSubIndex  = m_iSubIndexMask & a_iIndex;
		TINT iMainIndex = a_iIndex >> ( m_iMainIndexShift % BUFSIZE );

		TASSERT( iMainIndex < m_oArray.Size() );
		TASSERT( iSubIndex < BUFSIZE );

		return m_oArray[ iMainIndex ]->GetElement( iSubIndex );
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
	TArray<TNode*> m_oArray;
};

template <typename T, TINT BUFSIZE, TINT IDENTIFIER>
typename TSharedBuffer<T, BUFSIZE, IDENTIFIER>::TNode* TSharedBuffer<T, BUFSIZE, IDENTIFIER>::m_pNodeArray;
template <typename T, TINT BUFSIZE, TINT IDENTIFIER>
TINT TSharedBuffer<T, BUFSIZE, IDENTIFIER>::m_iNumBuffers;
template <typename T, TINT BUFSIZE, TINT IDENTIFIER>
TINT TSharedBuffer<T, BUFSIZE, IDENTIFIER>::m_iMainIndexShift;
template <typename T, TINT BUFSIZE, TINT IDENTIFIER>
TINT TSharedBuffer<T, BUFSIZE, IDENTIFIER>::m_iSubIndexMask;

// m_oFreeList's per-TU dynamic initializers (atexit registration) and atexit destructors,
// one pair in every .cpp that instantiates the buffer; the type follows from the free list each destructor frees
// (the one CreateSharedBuffers<T> fills)
// TFLOAT: initializers, then destructors
// $deBlob: FUNCTION 0053fe10
// $deBlob: FUNCTION 0053fe60
// $deBlob: FUNCTION 0053fed0
// $deBlob: FUNCTION 0053fef0
// $deBlob: FUNCTION 0053ff10
// $deBlob: FUNCTION 0053ff50
// $deBlob: FUNCTION 0053ff90
// $deBlob: FUNCTION 0053fff0
// $deBlob: FUNCTION 005400b0
// $deBlob: FUNCTION 005400d0
// $deBlob: FUNCTION 00540120
// $deBlob: FUNCTION 00540160
// $deBlob: FUNCTION 00859620
// $deBlob: FUNCTION 008596c0
// $deBlob: FUNCTION 00859770
// $deBlob: FUNCTION 008597b0
// $deBlob: FUNCTION 008597f0
// $deBlob: FUNCTION 00859840
// $deBlob: FUNCTION 00859890
// $deBlob: FUNCTION 008598f0
// $deBlob: FUNCTION 008599e0
// $deBlob: FUNCTION 00859a20
// $deBlob: FUNCTION 00859a90
// $deBlob: FUNCTION 00859b10
// TVector4: initializers, then destructors
// $deBlob: FUNCTION 0053fe30
// $deBlob: FUNCTION 0053fe70
// $deBlob: FUNCTION 0053fe80
// $deBlob: FUNCTION 0053ffc0
// $deBlob: FUNCTION 00540080
// $deBlob: FUNCTION 00540090
// $deBlob: FUNCTION 005400a0
// $deBlob: FUNCTION 005400c0
// $deBlob: FUNCTION 005400e0
// $deBlob: FUNCTION 00540110
// $deBlob: FUNCTION 00540130
// $deBlob: FUNCTION 00540140
// $deBlob: FUNCTION 00540170
// $deBlob: FUNCTION 00540180
// $deBlob: FUNCTION 00859660
// $deBlob: FUNCTION 008596e0
// $deBlob: FUNCTION 00859700
// $deBlob: FUNCTION 008598c0
// $deBlob: FUNCTION 00859980
// $deBlob: FUNCTION 008599a0
// $deBlob: FUNCTION 008599c0
// $deBlob: FUNCTION 00859a00
// $deBlob: FUNCTION 00859a40
// $deBlob: FUNCTION 00859a70
// $deBlob: FUNCTION 00859ab0
// $deBlob: FUNCTION 00859ad0
// $deBlob: FUNCTION 00859b30
// $deBlob: FUNCTION 00859b50
// TUINT8: initializers, then destructors
// $deBlob: FUNCTION 0053fdf0
// $deBlob: FUNCTION 0053fe40
// $deBlob: FUNCTION 0053feb0
// $deBlob: FUNCTION 0053fee0
// $deBlob: FUNCTION 008595e0
// $deBlob: FUNCTION 00859680
// $deBlob: FUNCTION 00859730
// $deBlob: FUNCTION 00859790
// TUINT32: initializers, then destructors
// $deBlob: FUNCTION 0053fe00
// $deBlob: FUNCTION 0053fe50
// $deBlob: FUNCTION 0053fec0
// $deBlob: FUNCTION 0053ff00
// $deBlob: FUNCTION 0053ff40
// $deBlob: FUNCTION 0053ff80
// $deBlob: FUNCTION 00540020
// $deBlob: FUNCTION 00540050
// $deBlob: FUNCTION 00540150
// $deBlob: FUNCTION 00859600
// $deBlob: FUNCTION 008596a0
// $deBlob: FUNCTION 00859750
// $deBlob: FUNCTION 008597d0
// $deBlob: FUNCTION 00859820
// $deBlob: FUNCTION 00859870
// $deBlob: FUNCTION 00859920
// $deBlob: FUNCTION 00859950
// $deBlob: FUNCTION 00859af0
// TQuaternion: initializers, then destructors
// $deBlob: FUNCTION 0053fe20
// $deBlob: FUNCTION 00859640
template <typename T, TINT BUFSIZE, TINT IDENTIFIER>
TArray<typename TSharedBuffer<T, BUFSIZE, IDENTIFIER>::TNode*> TSharedBuffer<T, BUFSIZE, IDENTIFIER>::m_oFreeList;

TOSHI_NAMESPACE_END

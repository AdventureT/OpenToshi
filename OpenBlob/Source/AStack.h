#pragma once

//-----------------------------------------------------------------------------
// Purpose: a fixed size stack (Nicktoons06\Source\AStack.h, shared with the other Blue Tongue games of the branch)
//-----------------------------------------------------------------------------
template <typename T, TINT C>
class AStack
{
public:
	AStack()
	    : m_iStackIndex( -1 )
	{}

	// NOTE: a push beyond the capacity only moves the index
	void Push( const T& a_rItem )
	{
		m_iStackIndex++;

		if ( m_iStackIndex < C )
			m_aItems[ m_iStackIndex ] = a_rItem;
	}

	// $deBlob: FUNCTION 005c0a30
	// $deBlob: FUNCTION 005d6e90
	// $deBlob: FUNCTION 005dbf50
	T& Pop()
	{
		TASSERT( m_iStackIndex >= 0 );
		return m_aItems[ m_iStackIndex-- ];
	}

	// $deBlob: FUNCTION 005d6f10
	// $deBlob: FUNCTION 005dc020
	T& Top()
	{
		TASSERT( !Empty() );
		return m_aItems[ m_iStackIndex ];
	}

	// $deBlob: FUNCTION 0075b0c0
	TINT  GetStackIndex() const { return m_iStackIndex; }
	TBOOL Empty() const { return m_iStackIndex < 0; }
	TINT  Size() const { return m_iStackIndex + 1; }
	void  Reset() { m_iStackIndex = -1; }

	// $deBlob: FUNCTION 0056e820
	// $deBlob: FUNCTION 0075b060
	// $deBlob: FUNCTION 0078d370
	// $deBlob: FUNCTION 007e95f0
	T& operator[]( TINT a_iIndex )
	{
		TASSERT( a_iIndex <= m_iStackIndex );
		return m_aItems[ a_iIndex ];
	}

	const T& operator[]( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex <= m_iStackIndex );
		return m_aItems[ a_iIndex ];
	}

private:
	TINT m_iStackIndex; // 0x0
	T    m_aItems[ C ]; // 0x4
};

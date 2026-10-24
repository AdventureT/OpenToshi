#pragma once

TOSHI_NAMESPACE_START

template <typename T, TINT C>
class TStack
{
public:
	TStack()  = default;
	~TStack() = default;

	// $deBlob: FUNCTION 005f0640
	// $Barnyard: FUNCTION 00471520
	void Push( const T& a_item = T() )
	{
		m_iTop++;
		if ( m_iTop < C ) AtUnsafe( m_iTop ) = a_item;
	}

	T& PushNull()
	{
		m_iTop++;
		return Top();
	}

	// de Blob's Pop returns the new top (the item below the popped one) and clamps a negative index to 0; Barnyard's
	// returns the popped item and clamps only -1
	// $deBlob: FUNCTION 005f0620
	T& Pop()
	{
		m_iTop--;
		if ( m_iTop < 0 ) m_iTop = 0;
		return AtUnsafe( m_iTop );
	}

	TBOOL IsFull() const
	{
		return m_iTop == ( C - 1 );
	}

	T& Top()
	{
		return AtUnsafe( m_iTop );
	}

	TBOOL IsEmpty() const
	{
		return m_iTop == -1;
	}

	void Reset()
	{
		m_iTop = -1;
	}

	TINT Count() const
	{
		return m_iTop + 1;
	}

	T& Begin()
	{
		return AtUnsafe( 0 );
	}

	const T& Begin() const
	{
		return AtUnsafe( 0 );
	}

	T& End()
	{
		return AtUnsafe( m_iTop );
	}

	const T& End() const
	{
		return AtUnsafe( m_iTop );
	}

private:
	TFORCEINLINE TCONSTEXPR T& AtUnsafe( TUINT a_uiIndex )
	{
		return *( (T*)( m_aBuffer ) + a_uiIndex );
	}

	TFORCEINLINE TCONSTEXPR const T& AtUnsafe( TUINT a_uiIndex ) const
	{
		return *( (const T*)( m_aBuffer ) + a_uiIndex );
	}

private:
	TINT  m_iTop = -1;
	TBYTE m_aBuffer[ sizeof( T ) * C ];
};

TOSHI_NAMESPACE_END

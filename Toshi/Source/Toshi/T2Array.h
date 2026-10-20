#pragma once

TOSHI_NAMESPACE_START

template <class T, TINT Capacity>
class T2Array
{
public:
	void InitialiseAll( const T& a_oValue )
	{
		for ( TINT i = 0; i < Capacity; i++ )
		{
			this->operator[]( i ) = a_oValue;
		}
	}

	TINT GetCapacity() const
	{
		return Capacity;
	}

	T* GetArray() const
	{
		return TREINTERPRETCAST( T*, m_pData );
	}

	// $deBlob: FUNCTION 005a2820
	// $deBlob: FUNCTION 005a2880
	// $deBlob: FUNCTION 005a28e0
	// $deBlob: FUNCTION 005a2940
	// $deBlob: FUNCTION 005a2990
	// $deBlob: FUNCTION 005a29e0
	// $deBlob: FUNCTION 005b24f0
	// $deBlob: FUNCTION 005e13d0
	// $deBlob: FUNCTION 005e55d0
	// $deBlob: FUNCTION 005e7270
	// $deBlob: FUNCTION 00679990
	// $deBlob: FUNCTION 006ad8c0
	// $deBlob: FUNCTION 006ad920
	// $deBlob: FUNCTION 006ad970
	// $deBlob: FUNCTION 007d2810
	// $deBlob: FUNCTION 007e41f0
	// $deBlob: FUNCTION 007e83b0
	// $deBlob: FUNCTION 007e8550
	T& operator[]( TINT a_iIndex )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < Capacity );
		return *( TREINTERPRETCAST( T*, m_pData ) + a_iIndex );
	}

	// $deBlob: FUNCTION 006799f0
	const T& operator[]( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < Capacity );
		return *( TREINTERPRETCAST( const T*, m_pData ) + a_iIndex );
	}

private:
	TCHAR m_pData[ sizeof( T ) * Capacity ];
};

TOSHI_NAMESPACE_END

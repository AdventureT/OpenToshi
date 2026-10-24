#pragma once
#include <cstdint>

typedef TINT Endianess;
enum Endianess_ : Endianess
{
	Endianess_Little,
	Endianess_Big
};

TCONSTEXPR_VAR Endianess CURRENT_ENDIANESS =
#ifdef TOSHI_SKU_WINDOWS
    Endianess_Little;
#endif

TFORCEINLINE TCONSTEXPR TUINT16 SWAP16( const TUINT8 bytes[ 2 ] )
{
	return ( bytes[ 1 ] << 0 ) | ( bytes[ 0 ] << 8 );
}

TFORCEINLINE TCONSTEXPR TUINT32 SWAP32( const TUINT8 bytes[ 4 ] )
{
	return ( bytes[ 3 ] << 0 ) | ( bytes[ 2 ] << 8 ) | ( bytes[ 1 ] << 16 ) | ( bytes[ 0 ] << 24 );
}

TFORCEINLINE TCONSTEXPR TUINT16 SWAP16( TUINT16 value )
{
	return TUINT16( TUINT8( value ) << 8 ) | TUINT8( value >> 8 );
}

TFORCEINLINE TCONSTEXPR TUINT32 SWAP32( TUINT32 value )
{
	return ( SWAP16( TUINT16( value ) ) << 16 ) | SWAP16( TUINT16( value >> 16 ) );
}

TFORCEINLINE TFLOAT SWAPFLOAT( const TUINT8 bytes[ 4 ] )
{
	TUINT32 result = SWAP32( bytes );
	return *(TFLOAT*)( &result );
}

TFORCEINLINE TUINT32 PARSEDWORD( const TUINT32 value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return value;
	else return SWAP32( (const TUINT8*)&value );
}

TFORCEINLINE TUINT32 PARSEDWORD( const TUINT8* value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return *(TUINT32*)value;
	else return SWAP32( value );
}

TFORCEINLINE TUINT32 PARSEDWORD_BIG( const TUINT32 value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return SWAP32( value );
	else return value;
}

TFORCEINLINE TUINT32 PARSEDWORD_BIG( const TUINT8* value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return SWAP32( value );
	else return *(TUINT32*)value;
}

TFORCEINLINE TUINT16 PARSEWORD( TUINT16 value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return value;
	else return SWAP16( (const TUINT8*)&value );
}

TFORCEINLINE TUINT16 PARSEWORD( const TUINT8* value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return *(TUINT16*)value;
	else return SWAP16( value );
}

TFORCEINLINE TUINT32 PARSEWORD_BIG( TUINT32 value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return SWAP16( (const TUINT8*)&value );
	else return value;
}

TFORCEINLINE TUINT32 PARSEWORD_BIG( const TUINT8* value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return SWAP16( value );
	else return *(TUINT16*)value;
}

TFORCEINLINE TFLOAT PARSEFLOAT( TFLOAT value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return value;
	else return SWAPFLOAT( (const TUINT8*)&value );
}

TFORCEINLINE TFLOAT PARSEFLOAT( const TUINT8* value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return *(TFLOAT*)value;
	else return SWAPFLOAT( value );
}

TFORCEINLINE TFLOAT PARSEFLOAT_BIG( TFLOAT value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return SWAPFLOAT( (const TUINT8*)&value );
	else return value;
}

TFORCEINLINE TFLOAT PARSEFLOAT_BIG( const TUINT8* value )
{
	if ( CURRENT_ENDIANESS == Endianess_Little ) return SWAPFLOAT( value );
	else return *(TFLOAT*)value;
}

// CONVERTENDIANESS's conversions by type: TFLOAT, 4 and 2 byte values; other sizes stay as they are
template <typename T, TSIZE SIZE = sizeof( T )>
struct TEndianessConverter
{
	static T Little( T a_numValue ) { return a_numValue; }
	static T Big( T a_numValue ) { return a_numValue; }
};

template <typename T>
struct TEndianessConverter<T, 4>
{
	static T Little( T a_numValue ) { return (T)PARSEDWORD( *(TUINT32*)( TREINTERPRETCAST( void*, &a_numValue ) ) ); }
	static T Big( T a_numValue ) { return (T)PARSEDWORD_BIG( *(TUINT32*)( TREINTERPRETCAST( void*, &a_numValue ) ) ); }
};

template <typename T>
struct TEndianessConverter<T, 2>
{
	static T Little( T a_numValue ) { return (T)PARSEWORD( *(TUINT16*)( TREINTERPRETCAST( void*, &a_numValue ) ) ); }
	static T Big( T a_numValue ) { return (T)PARSEWORD_BIG( *(TUINT16*)( TREINTERPRETCAST( void*, &a_numValue ) ) ); }
};

template <>
struct TEndianessConverter<TFLOAT, 4>
{
	static TFLOAT Little( TFLOAT a_numValue ) { return PARSEFLOAT( a_numValue ); }
	static TFLOAT Big( TFLOAT a_numValue ) { return PARSEFLOAT_BIG( a_numValue ); }
};

template <typename T>
TFORCEINLINE T CONVERTENDIANESS( Endianess a_eEndianess, T a_numValue )
{
	switch ( a_eEndianess )
	{
		case Endianess_Little:
			return TEndianessConverter<T>::Little( a_numValue );
		case Endianess_Big:
			return TEndianessConverter<T>::Big( a_numValue );
	}

	return a_numValue;
}

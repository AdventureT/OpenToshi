#include "ToshiPCH.h"
#include "T2String8.h"
#include "T2CharTraits.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TCHAR T2String8::ms_aScratchMem[ T2String8::SCRATCH_MEM_SIZE ];

// $deBlob: FUNCTION 006ec1f0
TINT T2String8::FormatV( TCHAR* a_pcString, const TCHAR* a_pcFormat, va_list args )
{
	// NOTE: no iResult assert in de Blob
	return vsprintf( a_pcString, a_pcFormat, args );
}

// $deBlob: FUNCTION 006ec270
// $Barnyard: FUNCTION 006bb1d0
TINT T2String8::FormatV( TCHAR* a_pcString, TINT size, const TCHAR* a_pcFormat, va_list args )
{
	TINT iResult = _vsnprintf( a_pcString, size, a_pcFormat, args );
	TASSERT( iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem" );
	a_pcString[ size - 1 ] = '\0';
	return iResult;
}

// $deBlob: FUNCTION 006ec1d0
// $Barnyard: FUNCTION 006bb180
TINT T2String8::Format( TCHAR* a_pcString, const TCHAR* a_pcFormat, ... )
{
	va_list args;

	va_start( args, a_pcFormat );
	TINT iResult = FormatV( a_pcString, a_pcFormat, args );
	va_end( args );

	return iResult;
}

// $deBlob: FUNCTION 006ec210
TINT T2String8::Format( TCHAR* a_pcString, TINT size, const TCHAR* a_pcFormat, ... )
{
	va_list args;

	va_start( args, a_pcFormat );
	TINT iResult = FormatV( a_pcString, size, a_pcFormat, args );
	va_end( args );

	return iResult;
}

// $deBlob: FUNCTION 006ebe30
TINT T2String8::Compare( const TCHAR* str1, const TCHAR* str2, TSIZE size )
{
	const TCHAR* a_String1 = str1;
	const TCHAR* a_String2 = str2;
	TASSERT( ( a_String1 != TNULL ) && ( a_String2 != TNULL ) );

	if ( size != -1 )
		return strncmp( str1, str2, size );

	return strcmp( str1, str2 );
}

// $deBlob: FUNCTION 006ebec0
// $Barnyard: FUNCTION 006bb0e0
TINT T2String8::CompareNoCase( const TCHAR* str1, const TCHAR* str2, TSIZE size /*= -1*/ )
{
	const TCHAR* a_String1 = str1;
	const TCHAR* a_String2 = str2;
	TASSERT( ( a_String1 != TNULL ) && ( a_String2 != TNULL ) );

	if ( size != -1 )
		return _strnicmp( str1, str2, size );

	return _stricmp( str1, str2 );
}

// $deBlob: FUNCTION 006ebf30
// $Barnyard: FUNCTION 006bb120
TCHAR* T2String8::Copy( TCHAR* dst, const TCHAR* src, TSIZE size /*= -1*/ )
{
	const TCHAR* a_DestinationString = dst;
	const TCHAR* a_SourceString      = src;
	TASSERT( ( a_DestinationString != TNULL ) && ( a_SourceString != TNULL ) );

	if ( size != -1 )
		return strncpy( dst, src, size );

	return strcpy( dst, src );
}

TCHAR* T2String8::Concat( TCHAR* dst, const TCHAR* src, TSIZE size /*= -1*/ )
{
	if ( size != -1 )
		return strncat( dst, src, size );

	return strcat( dst, src );
}

// $deBlob: FUNCTION 006ebfb0
TCHAR* T2String8::CopySafe( TCHAR* dst, const TCHAR* src, TSIZE size )
{
	const TCHAR* a_DestinationString = dst;
	const TCHAR* a_SourceString      = src;
	TINT         a_iDestBufferSize   = TINT( size );
	TASSERT( ( a_DestinationString != TNULL ) && ( a_SourceString != TNULL ) );
	TASSERT( a_iDestBufferSize > 1 );

	TINT iSrcLen = TINT( Length( src ) );
	size         = TSIZE( TMath::Min( a_iDestBufferSize - 1, iSrcLen ) );
	strncpy( dst, src, size );
	dst[ size ] = '\0';
	return dst;
}

// $deBlob: FUNCTION 006ec0a0
TCHAR* T2String8::Move( TCHAR* dst, const TCHAR* src, TSIZE size )
{
	const TCHAR* a_DestinationString = dst;
	const TCHAR* a_SourceString      = src;
	TASSERT( ( a_DestinationString != TNULL ) && ( a_SourceString != TNULL ) );

	if ( dst >= src )
	{
		// Copy backwards (including the terminator) so overlapping strings stay intact
		TINT iSize = ( size == -1 ) ? TINT( strlen( src ) ) : TINT( size );

		TCHAR*       pDst = dst + iSize;
		const TCHAR* pSrc = src + iSize;
		for ( ; iSize >= 0; iSize-- )
			*pDst-- = *pSrc--;

		return dst;
	}

	if ( size != -1 )
		return strncpy( dst, src, size );

	return strcpy( dst, src );
}

const TCHAR* T2String8::FindChar( const TCHAR* str, TCHAR character )
{
	while ( TTRUE )
	{
		if ( *str == '\0' ) return TNULL;
		if ( *str == character ) return str;
		str++;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006ec1a0
TCHAR* T2String8::FindChar( TCHAR* str, TCHAR character )
{
	while ( TTRUE )
	{
		if ( *str == '\0' ) return TNULL;
		if ( *str == character ) return str;
		str++;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006bb200
const TCHAR* T2String8::FindString( const TCHAR* str, const TCHAR* substr )
{
	return strstr( str, substr );
}

// $deBlob: FUNCTION 006ec2d0
TCHAR* T2String8::FindString( TCHAR* str, const TCHAR* substr )
{
	return strstr( str, substr );
}

// $deBlob: FUNCTION 006ec150
// $Barnyard: FUNCTION 006bb160
TSIZE T2String8::Length( const TCHAR* str )
{
	const TCHAR* a_String = str;
	TASSERT( a_String != TNULL );

	return strlen( str );
}

// $deBlob: FUNCTION 006ec5b0
TBOOL T2String8::IsLowerCase( const TCHAR* str )
{
	while ( *str != '\0' && T2Char8::IsLowerCase( *str ) )
		str++;

	return *str == '\0';
}

TBOOL T2String8::IsUpperCase( const TCHAR* str )
{
	while ( *str != '\0' && T2Char8::IsUpperCase( *str ) )
		str++;

	return *str == '\0';
}

// $deBlob: FUNCTION 006ec340
void T2String8::ToLowerCase( TCHAR* str )
{
	while ( *str )
	{
		*str = T2Char8::ToLowerCase( *str );
		str++;
	}
}

// $deBlob: FUNCTION 006ec370
void T2String8::ToUpperCase( TCHAR* str )
{
	while ( *str )
	{
		*str = T2Char8::ToUpperCase( *str );
		str++;
	}
}

// $Barnyard: FUNCTION 006bb220
void T2String8::IntToString( TINT value, TCHAR* dst, TINT radix )
{
	_itoa( value, dst, radix );
}

// $deBlob: FUNCTION 006ec2f0
void T2String8::IntToString( TINT value, TCHAR* dst, TINT unused, TINT radix )
{
	_itoa( value, dst, radix );
}

// $deBlob: FUNCTION 006ec460
// $Barnyard: FUNCTION 006bb2a0
TINT T2String8::StringToInt( const TCHAR* src )
{
	return atol( src );
}

// $deBlob: FUNCTION 006ec480
TUINT T2String8::StringToUInt( const TCHAR* src )
{
	return TUINT( atol( src ) );
}

// $deBlob: FUNCTION 006ec4a0
TFLOAT T2String8::StringToFloat( const TCHAR* src )
{
	return TFLOAT( atof( src ) );
}

// $deBlob: FUNCTION 006ec4c0
TBOOL T2String8::StringToBool( const TCHAR* src )
{
	if ( CompareNoCase( "True", src ) == 0 )
		return TTRUE;

	if ( CompareNoCase( "False", src ) == 0 )
		return TFALSE;

	return atol( src ) != 0;
}

// $deBlob: FUNCTION 006ec570
TBOOL T2String8::IsNumeric( const TCHAR* str )
{
	while ( isdigit( *str ) )
		str++;

	return *str == '\0';
}

// $deBlob: FUNCTION 006ec310
TUINT32 T2String8::Hash( const TCHAR* str )
{
	TUINT32 uiHash = 0;

	for ( ; *str != '\0'; str++ )
		uiHash = uiHash * 5 + *str;

	return uiHash;
}

// $deBlob: FUNCTION 006ec3a0
TCHAR T2String8::CharToLowerCase( TCHAR a_cChar )
{
	return TCHAR( tolower( a_cChar ) );
}

// $deBlob: FUNCTION 006ec400
TCHAR T2String8::CharToUpperCase( TCHAR a_cChar )
{
	return TCHAR( toupper( a_cChar ) );
}

TCHAR* T2String8::SkipSpaces( TCHAR* str )
{
	TCHAR* pszCursor = str;
	while ( T2Char8::IsSpace( *pszCursor ) ) pszCursor++;

	return pszCursor;
}

const TCHAR* T2String8::SkipSpaces( const TCHAR* str )
{
	const TCHAR* pszCursor = str;
	while ( T2Char8::IsSpace( *pszCursor ) ) pszCursor++;

	return pszCursor;
}

TCHAR* T2String8::CreateCopy( const TCHAR* a_pcString, TSIZE a_uiSize /*= -1 */ )
{
	TSIZE uiSize = ( a_uiSize == -1 ) ? T2String8::Length( a_pcString ) : a_uiSize;

	TCHAR* pcBuffer = new TCHAR[ uiSize + 1 ];
	TUtil::MemCopy( pcBuffer, a_pcString, uiSize * sizeof( TCHAR ) );
	pcBuffer[ uiSize ] = '\0';

	return pcBuffer;
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "T2CharTraits.h"
#include "T2String16.h"
#include "T2String8.h"
#include "TStringManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2Allocator* T2String16::sm_pDefaultAllocatorCB = GetDefaultAllocatorCB();

TWCHAR T2String16::ms_aScratchMem[ T2String16::SCRATCH_MEM_SIZE ];

// NOTE: de Blob's T2String16 Format functions have no iResult asserts
// $deBlob: FUNCTION 006f1d30
// $Barnyard: FUNCTION 006bb400
TINT T2String16::FormatV( TWCHAR* a_pcString, TINT size, const TWCHAR* a_pcFormat, va_list args )
{
	TINT iResult           = _vsnwprintf( a_pcString, size, a_pcFormat, args );
	a_pcString[ size - 1 ] = '\0';
	return iResult;
}

// $deBlob: FUNCTION 006f1ce0
TINT T2String16::FormatV( TWCHAR* a_pcString, const TWCHAR* a_pcFormat, va_list args )
{
	return _vswprintf( a_pcString, a_pcFormat, args );
}

// $deBlob: FUNCTION 006f1cc0
// $Barnyard: FUNCTION 006bb3e0
TINT T2String16::Format( TWCHAR* a_pcString, const TWCHAR* a_pcFormat, ... )
{
	va_list args;

	va_start( args, a_pcFormat );
	TINT iResult = FormatV( a_pcString, a_pcFormat, args );
	va_end( args );

	return iResult;
}

// $deBlob: FUNCTION 006f1d00
TINT T2String16::Format( TWCHAR* a_pcString, TINT size, const TWCHAR* a_pcFormat, ... )
{
	va_list args;

	va_start( args, a_pcFormat );
	TINT iResult = FormatV( a_pcString, size, a_pcFormat, args );
	va_end( args );

	return iResult;
}

// $deBlob: FUNCTION 006f19f0
// $Barnyard: FUNCTION 006bb2d0
TINT T2String16::Compare( const TWCHAR* str1, const TWCHAR* str2, TSIZE size )
{
	const TWCHAR* a_String1 = str1;
	const TWCHAR* a_String2 = str2;
	TASSERT( ( a_String1 != TNULL ) && ( a_String2 != TNULL ) );

	if ( size != -1 )
		return wcsncmp( str1, str2, size );

	return wcscmp( str1, str2 );
}

// $deBlob: FUNCTION 006f1a80
TINT T2String16::CompareNoCase( const TWCHAR* str1, const TWCHAR* str2, TSIZE size )
{
	const TWCHAR* a_String1 = str1;
	const TWCHAR* a_String2 = str2;
	TASSERT( ( a_String1 != TNULL ) && ( a_String2 != TNULL ) );

	if ( size != -1 )
	{
		return _wcsnicmp( str1, str2, size );
	}

	return _wcsicmp( str1, str2 );
}

// NOTE: TStringManager::String16Copy is the same plain copy; retail only has that one.
TWCHAR* T2String16::Copy( TWCHAR* dst, const TWCHAR* src, TSIZE size )
{
	if ( size != -1 )
		return wcsncpy( dst, src, size );

	return wcscpy( dst, src );
}

// $deBlob: FUNCTION 006f1bc0
// $Barnyard: FUNCTION 006bb350
TWCHAR* T2String16::Move( TWCHAR* dst, const TWCHAR* src, TSIZE size )
{
	const TWCHAR* a_DestinationString = dst;
	const TWCHAR* a_SourceString      = src;
	TASSERT( ( a_DestinationString != TNULL ) && ( a_SourceString != TNULL ) );

	if ( dst >= src )
	{
		// Copy backwards so overlapping strings stay intact
		if ( size == -1 )
			size = wcslen( src );

		TWCHAR*       pDst = dst + size;
		const TWCHAR* pSrc = src + size;

		for ( TINT i = TINT( size ); i >= 0; i-- )
			*pDst-- = *pSrc--;

		return dst;
	}

	if ( size != -1 )
		return wcsncpy( dst, src, size );

	return wcscpy( dst, src );
}

// $deBlob: FUNCTION 006f1af0
// $Barnyard: FUNCTION 006bb590
TWCHAR* T2String16::CopySafe( TWCHAR* dst, const TWCHAR* src, TSIZE size )
{
	const TWCHAR* a_DestinationString = dst;
	const TWCHAR* a_SourceString      = src;
	TINT          a_iDestBufferSize   = TINT( size );
	TASSERT( ( a_DestinationString != TNULL ) && ( a_SourceString != TNULL ) );
	TASSERT( a_iDestBufferSize > 1 );

	TINT iSrcLen = TINT( Length( src ) );
	size         = TSIZE( TMath::Min( a_iDestBufferSize - 1, iSrcLen ) );
	wcsncpy( dst, src, size );
	dst[ size ] = L'\0';
	return dst;
}

TWCHAR* T2String16::Concat( TWCHAR* dst, const TWCHAR* src, TSIZE size /*= -1 */ )
{
	if ( size != -1 )
		return wcsncat( dst, src, size );

	return wcscat( dst, src );
}

// $deBlob: FUNCTION 006f1db0
const TWCHAR* T2String16::FindChar( const TWCHAR* str, TWCHAR character )
{
	while ( TTRUE )
	{
		if ( *str == L'\0' ) return TNULL;
		if ( *str == character ) return str;
		str++;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006f1d80
TWCHAR* T2String16::FindChar( TWCHAR* str, TWCHAR character )
{
	while ( TTRUE )
	{
		if ( *str == L'\0' ) return TNULL;
		if ( *str == character ) return str;
		str++;
	}

	return TNULL;
}

const TWCHAR* T2String16::FindString( const TWCHAR* str, const TWCHAR* substr )
{
	return wcsstr( str, substr );
}

// $deBlob: FUNCTION 006f1d60
TWCHAR* T2String16::FindString( TWCHAR* str, const TWCHAR* substr )
{
	return wcsstr( str, substr );
}

// $deBlob: FUNCTION 006f1c90
// $Barnyard: FUNCTION 006bb3c0
TINT T2String16::Length( const TWCHAR* str )
{
	if ( str != TNULL ) return TINT( wcslen( str ) );
	return 0;
}

TBOOL T2String16::IsLowerCase( const TWCHAR* str )
{
	while ( *str != L'\0' && T2Char16::IsLowerCase( *str ) )
		str++;

	return ( *str == L'\0' );
}

TBOOL T2String16::IsUpperCase( const TWCHAR* str )
{
	while ( *str != L'\0' && T2Char16::IsUpperCase( *str ) )
		str++;

	return ( *str == L'\0' );
}

void T2String16::ToLowerCase( TWCHAR* str )
{
	while ( *str )
	{
		*str = T2Char16::ToLowerCase( *str );
		str++;
	}
}

void T2String16::ToUpperCase( TWCHAR* str )
{
	while ( *str )
	{
		*str = T2Char16::ToUpperCase( *str );
		str++;
	}
}

void T2String16::IntToString( TINT value, TWCHAR* dst, TINT unused, TINT radix )
{
	_itow( value, dst, radix );
}

void T2String16::IntToString( TINT value, TWCHAR* dst, TINT radix )
{
	_itow( value, dst, radix );
}

TINT T2String16::StringToInt( const TWCHAR* src )
{
	return _wtoi( src );
}

TFLOAT T2String16::StringToFloat( const TWCHAR* src )
{
	return (TFLOAT)_wtof( src );
}

TWCHAR* T2String16::SkipSpaces( TWCHAR* str )
{
	TWCHAR* pszCursor = str;
	while ( T2Char16::IsSpace( *pszCursor ) ) pszCursor++;

	return pszCursor;
}

const TWCHAR* T2String16::SkipSpaces( const TWCHAR* str )
{
	const TWCHAR* pszCursor = str;
	while ( T2Char16::IsSpace( *pszCursor ) ) pszCursor++;

	return pszCursor;
}

// NOTE: de Blob's version takes a TWCHAR (Barnyard's a TUINT8); the CRT toupper only handles the 8-bit range
// $deBlob: FUNCTION 006f1e70
// $Barnyard: FUNCTION 006bb430
TWCHAR T2String16::CharToUpperCase( TWCHAR a_uiChar )
{
	return TWCHAR( toupper( a_uiChar ) );
}

// $deBlob: FUNCTION 006f1ed0
TWCHAR T2String16::CharToLowerCase( TWCHAR a_uiChar )
{
	return TWCHAR( tolower( a_uiChar ) );
}

// $deBlob: FUNCTION 006f1de0
TWCHAR* T2String16::FindLastChar( TWCHAR* str, TWCHAR character )
{
	TWCHAR* pResult = TNULL;

	for ( ; *str != L'\0'; str++ )
	{
		if ( *str == character )
			pResult = str;
	}

	return pResult;
}

// $deBlob: FUNCTION 006f1e10
const TWCHAR* T2String16::FindLastChar( const TWCHAR* str, TWCHAR character )
{
	const TWCHAR* pResult = TNULL;

	for ( ; *str != L'\0'; str++ )
	{
		if ( *str == character )
			pResult = str;
	}

	return pResult;
}

// $deBlob: FUNCTION 006f1e40
TUINT32 T2String16::Hash( const TWCHAR* str )
{
	TUINT32 uiHash = 0;

	for ( ; *str != L'\0'; str++ )
		uiHash = uiHash * 5 + *str;

	return uiHash;
}

// $deBlob: FUNCTION 006f1f90
TWCHAR* T2String16::CopyFromChar( TWCHAR* a_UnicodeString, const TCHAR* a_CharString, TINT a_iLength )
{
	TASSERT( ( a_UnicodeString != TNULL ) && ( a_CharString != TNULL ) );

	TINT iLength = TINT( TStringManager::String8Length( a_CharString ) );
	TINT iCount  = ( a_iLength > iLength || a_iLength == -1 ) ? iLength : a_iLength;

	TINT i;
	for ( i = 0; i < iCount; i++ )
		a_UnicodeString[ i ] = TUINT8( a_CharString[ i ] );

	a_UnicodeString[ i ] = L'\0';
	return a_UnicodeString;
}

// $deBlob: FUNCTION 006f2010
TCHAR* T2String16::CopyToChar( TCHAR* a_CharString, const TWCHAR* a_UnicodeString, TINT a_iLength )
{
	TASSERT( ( a_UnicodeString != TNULL ) && ( a_CharString != TNULL ) );

	TINT iLength = TINT( Length( a_UnicodeString ) );
	TINT iCount  = ( a_iLength > iLength || a_iLength == -1 ) ? iLength : a_iLength;

	TINT i;
	for ( i = 0; i < iCount; i++ )
		a_CharString[ i ] = TCHAR( a_UnicodeString[ i ] );

	a_CharString[ i ] = '\0';
	return a_CharString;
}

// $deBlob: FUNCTION 006f1f30
// $Barnyard: FUNCTION 006bb450
TWCHAR* T2String16::CreateCopy( const TWCHAR* a_wszString, TSIZE a_uiSize )
{
	TSIZE uiSize = ( a_uiSize == -1 ) ? T2String16::Length( a_wszString ) : a_uiSize;

	// NOTE: de Blob uses TMalloc (Barnyard new[]), both are freed through TFree
	TWCHAR* wszBuffer = TSTATICCAST( TWCHAR, TMalloc( uiSize * sizeof( TWCHAR ) + sizeof( TWCHAR ) ) );
	memcpy( wszBuffer, a_wszString, uiSize * sizeof( TWCHAR ) );
	wszBuffer[ uiSize ] = L'\0';

	return wszBuffer;
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TStringManager.h"
#include "TString8.h"
#include "ToshiTools/T2DynamicStack.h"
#include <string>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006eb740
TCHAR* TStringManager::GetTempString8()
{
	return T2String8::ms_aScratchMem;
}

// $deBlob: FUNCTION 006eb8e0
// Splits a_szString at spaces, tabs and new lines and pushes every token on a_rOutTokens
void TStringManager::String8Tokenise( const TCHAR* a_szString, T2DynamicStack<TString8>& a_rOutTokens )
{
	TCONSTEXPR_VAR TINT MAX_TOKEN_SIZE = 256;

	TString8     strUnused;
	const TCHAR* pTokenStart = TNULL;

	while ( TTRUE )
	{
		TCHAR cChar = *a_szString;

		if ( cChar == '\0' || cChar == ' ' || cChar == '\n' || cChar == '\t' )
		{
			if ( pTokenStart != TNULL )
			{
				TINT len = TINT( a_szString - pTokenStart );
				TASSERT( len < MAX_TOKEN_SIZE - 1 );
				TASSERT( len > 0 );

				TCHAR szToken[ MAX_TOKEN_SIZE ];
				TUtil::MemCopy( szToken, pTokenStart, len );
				szToken[ len ] = '\0';

				a_rOutTokens.Push( TString8( szToken ) );
				pTokenStart = TNULL;
			}

			if ( *a_szString == '\0' )
				return;
		}
		else if ( pTokenStart == TNULL )
		{
			pTokenStart = a_szString;
		}

		a_szString++;
	}
}

// $deBlob: FUNCTION 006eba70
TWCHAR* TStringManager::String16Copy( TWCHAR* dst, const TWCHAR* src, TSIZE size )
{
	const TWCHAR* a_DestinationString = dst;
	const TWCHAR* a_SourceString      = src;
	TASSERT( ( a_DestinationString != TNULL ) && ( a_SourceString != TNULL ) );

	if ( size != -1 )
		return wcsncpy( dst, src, size );

	return wcscpy( dst, src );
}

// $deBlob: FUNCTION 006ebbd0
// $Barnyard: FUNCTION 006bb4b0
TWCHAR* TStringManager::StringCharToUnicode( TWCHAR* a_UnicodeString, const TCHAR* a_CharString, TUINT32 a_iLength )
{
	TASSERT( ( a_UnicodeString != TNULL ) && ( a_CharString != TNULL ) );

	TINT iCharStringLength = TINT( String8Length( a_CharString ) );
	TINT iLength           = TINT( a_iLength );

	if ( iLength > iCharStringLength || iLength == -1 )
		iLength = iCharStringLength;

	TINT i;
	for ( i = 0; i < iLength; i++ )
		a_UnicodeString[ i ] = TWCHAR( TUINT8( a_CharString[ i ] ) );

	a_UnicodeString[ i ] = L'\0';
	return a_UnicodeString;
}

// $deBlob: FUNCTION 006ebc50
// $Barnyard: FUNCTION 006bb500
TCHAR* TStringManager::StringUnicodeToChar( TCHAR* a_CharString, const TWCHAR* a_UnicodeString, TUINT32 a_iLength )
{
	TASSERT( ( a_UnicodeString != TNULL ) && ( a_CharString != TNULL ) );

	TINT iUnicodeStringLength = TINT( String16Length( a_UnicodeString ) );
	TINT iLength              = TINT( a_iLength );

	if ( iLength > iUnicodeStringLength || iLength == -1 )
		iLength = iUnicodeStringLength;

	TINT i;
	for ( i = 0; i < iLength; i++ )
		a_CharString[ i ] = TCHAR( a_UnicodeString[ i ] );

	a_CharString[ i ] = '\0';
	return a_CharString;
}

// $deBlob: FUNCTION 006eba40
TWCHAR* TStringManager::GetTempString16()
{
	return T2String16::ms_aScratchMem;
}

// $deBlob: FUNCTION 006eb750
// $Barnyard: FUNCTION 006bb070
TINT TStringManager::String8Compare( const TCHAR* str1, const TCHAR* str2, TSIZE size ) { return T2String8::Compare( str1, str2, size ); }

// $deBlob: FUNCTION 006eb760
TINT TStringManager::String8CompareNoCase( const TCHAR* str1, const TCHAR* str2, TSIZE size ) { return T2String8::CompareNoCase( str1, str2, size ); }

// $deBlob: FUNCTION 006eb770
TCHAR* TStringManager::String8Copy( TCHAR* dst, const TCHAR* src, TSIZE size ) { return T2String8::Copy( dst, src, size ); }

// $deBlob: FUNCTION 006eb780
TCHAR* TStringManager::String8CopySafe( TCHAR* dst, const TCHAR* src, TSIZE size ) { return T2String8::CopySafe( dst, src, size ); }

// $deBlob: FUNCTION 006eb790
TCHAR* TStringManager::String8Move( TCHAR* dst, const TCHAR* src, TSIZE size ) { return T2String8::Move( dst, src, size ); }

// $deBlob: FUNCTION 006eb7a0
TINT TStringManager::String8Length( const TCHAR* str ) { return T2String8::Length( str ); }

// $deBlob: FUNCTION 006eb7b0
const TCHAR* TStringManager::String8FindChar( const TCHAR* str, TCHAR character ) { return T2String8::FindChar( str, character ); }

// $deBlob: FUNCTION 006eb7c0
TINT TStringManager::String8Format( TCHAR* str, const TCHAR* format, ... )
{
	va_list args;

	va_start( args, format );
	TINT iResult = T2String8::FormatV( str, format, args );
	va_end( args );

	return iResult;
}

// $deBlob: FUNCTION 006eb7e0
// $Barnyard: FUNCTION 006bb1a0
TINT TStringManager::String8Format( TCHAR* str, TINT size, const TCHAR* format, ... )
{
	va_list args;

	va_start( args, format );
	TINT iResult = T2String8::FormatV( str, size, format, args );
	va_end( args );

	return iResult;
}

// $deBlob: FUNCTION 006eb800
TINT TStringManager::String8FormatV( TCHAR* str, TINT size, const TCHAR* format, va_list args ) { return T2String8::FormatV( str, size, format, args ); }

// $deBlob: FUNCTION 006eb810
const TCHAR* TStringManager::String8FindString( const TCHAR* str, const TCHAR* substr ) { return T2String8::FindString( str, substr ); }

// $deBlob: FUNCTION 006eb820
void TStringManager::String8IntToString( TINT value, TCHAR* dst, TINT radix ) { _itoa( value, dst, radix ); }

// $deBlob: FUNCTION 006eb840
TUINT32 TStringManager::String8Hash( const TCHAR* str ) { return T2String8::Hash( str ); }

// $deBlob: FUNCTION 006eb850
// $Barnyard: FUNCTION 006bb270
void TStringManager::String8ToLowerCase( TCHAR* str )
{
	T2String8::ToLowerCase( str );
}

// $deBlob: FUNCTION 006eb860
void TStringManager::String8ToUpperCase( TCHAR* str )
{
	T2String8::ToUpperCase( str );
}

// $deBlob: FUNCTION 006eb870
TCHAR TStringManager::String8CharToLowerCase( TCHAR a_cChar ) { return T2String8::CharToLowerCase( a_cChar ); }

// $deBlob: FUNCTION 006eb880
TCHAR TStringManager::String8CharToUpperCase( TCHAR a_cChar ) { return T2String8::CharToUpperCase( a_cChar ); }

// $deBlob: FUNCTION 006eb890
TINT TStringManager::String8ToInt( const TCHAR* str ) { return T2String8::StringToInt( str ); }

// $deBlob: FUNCTION 006eb8a0
TUINT TStringManager::String8ToUInt( const TCHAR* str ) { return T2String8::StringToUInt( str ); }

// $deBlob: FUNCTION 006eb8b0
// $Barnyard: FUNCTION 006bb2c0
TFLOAT TStringManager::String8ToFloat( const TCHAR* str ) { return T2String8::StringToFloat( str ); }

// $deBlob: FUNCTION 006eb8c0
TBOOL TStringManager::String8ToBool( const TCHAR* str ) { return T2String8::StringToBool( str ); }

// $deBlob: FUNCTION 006eb8d0
TBOOL TStringManager::String8IsNumeric( const TCHAR* str ) { return T2String8::IsNumeric( str ); }

// $deBlob: FUNCTION 006eba50
TINT TStringManager::String16Compare( const TWCHAR* str1, const TWCHAR* str2, TSIZE size ) { return T2String16::Compare( str1, str2, size ); }

// $deBlob: FUNCTION 006eba60
TINT TStringManager::String16CompareNoCase( const TWCHAR* str1, const TWCHAR* str2, TSIZE size ) { return T2String16::CompareNoCase( str1, str2, size ); }

// $deBlob: FUNCTION 006ebaf0
TWCHAR* TStringManager::String16CopySafe( TWCHAR* dst, const TWCHAR* src, TSIZE size ) { return T2String16::CopySafe( dst, src, size ); }

// $deBlob: FUNCTION 006ebb00
TWCHAR* TStringManager::String16Move( TWCHAR* dst, const TWCHAR* src, TSIZE size ) { return T2String16::Move( dst, src, size ); }

// $deBlob: FUNCTION 006ebb10
TINT TStringManager::String16Length( const TWCHAR* str ) { return T2String16::Length( str ); }

// $deBlob: FUNCTION 006ebb20
const TWCHAR* TStringManager::String16FindChar( const TWCHAR* str, TWCHAR character ) { return T2String16::FindChar( str, character ); }

// $deBlob: FUNCTION 006ebb30
TINT TStringManager::String16Format( TWCHAR* str, const TWCHAR* format, ... )
{
	va_list args;

	va_start( args, format );
	TINT iResult = T2String16::FormatV( str, format, args );
	va_end( args );

	return iResult;
}

// $deBlob: FUNCTION 006ebb50
// $Barnyard: FUNCTION 006bb5f0
TINT TStringManager::String16Format( TWCHAR* str, TINT size, const TWCHAR* format, ... )
{
	va_list args;

	va_start( args, format );
	TINT iResult = T2String16::FormatV( str, size, format, args );
	va_end( args );

	return iResult;
}

// $deBlob: FUNCTION 006ebb70
TINT TStringManager::String16FormatV( TWCHAR* str, TINT size, const TWCHAR* format, va_list args ) { return T2String16::FormatV( str, size, format, args ); }

// $deBlob: FUNCTION 006ebb80
const TWCHAR* TStringManager::String16FindString( const TWCHAR* str, TWCHAR* substr ) { return T2String16::FindString( str, substr ); }

// $deBlob: FUNCTION 006ebb90
TUINT32 TStringManager::String16Hash( const TWCHAR* str ) { return T2String16::Hash( str ); }

// $deBlob: FUNCTION 006ebba0
TWCHAR TStringManager::String16CharToUpperCase( TWCHAR a_cChar ) { return T2String16::CharToUpperCase( a_cChar ); }

// $deBlob: FUNCTION 006ebbb0
TWCHAR TStringManager::String16CharToLowerCase( TWCHAR a_cChar ) { return T2String16::CharToLowerCase( a_cChar ); }

// $deBlob: FUNCTION 006ebbc0
TWCHAR* TStringManager::String16CreateCopy( const TWCHAR* str, TSIZE size ) { return T2String16::CreateCopy( str, size ); }

TOSHI_NAMESPACE_END

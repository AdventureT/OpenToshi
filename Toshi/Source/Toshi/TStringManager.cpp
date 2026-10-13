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

// $deBlob: FUNCTION 006eb8e0
// Splits a_szString at spaces, tabs and new lines and pushes every token on a_rOutTokens
// NOTE: invented name (Globs of Doom has a wide TokeniseBuffer)
void TStringManager::String8Tokenise( const TCHAR* a_szString, T2DynamicStack<TString8>& a_rOutTokens )
{
	constexpr TINT MAX_TOKEN_SIZE = 256;

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
TCHAR* TStringManager::StringUnicodeToChar( TCHAR* a_CharString, const TWCHAR* const& a_UnicodeString, TUINT32 a_iLength )
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

TOSHI_NAMESPACE_END

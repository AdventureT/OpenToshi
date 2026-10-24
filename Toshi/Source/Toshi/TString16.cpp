#include "ToshiPCH.h"
#include "TString16.h"
#include "TString8.h"

#include "Toshi/TSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TString16::func_DefaultAllocatorCB TString16::sm_pDefaultAllocatorCB = &GetDefaultAllocatorCB;

// NOTE: inline in the original, an instance of it lives in the game code
// $Barnyard: FUNCTION 0042b530
TString16::TString16()
    : TString16( GetAllocator() )
{
}

// NOTE: retail's constructors store a_pAllocator as is (the callers pass GetAllocator() through the inlined default
// argument); a TNULL allocator falls back to TMalloc/TFree, the same heap as the default allocator
// $deBlob: FUNCTION 006f0180
// $Barnyard: FUNCTION 006cc820
TString16::TString16( T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator;
	AllocBuffer( 0, TTRUE );
}

#ifdef TOSHI_STRING_MOVE
TString16::TString16( TString16&& src ) TNOEXCEPT
{
	TString16::m_pAllocator = src.m_pAllocator;
	TString16::m_iExcessLen = src.m_iExcessLen;
	TString16::m_iStrLen    = src.m_iStrLen;
	TString16::m_pBuffer    = src.m_pBuffer;
	src.Reset();
}
#endif

// $deBlob: FUNCTION 006f1050
TString16& TString16::operator=( const TString16& str )
{
	Copy( str, -1 );
	return *this;
}

// $deBlob: FUNCTION 006f0170
T2Allocator* TString16::GetAllocator()
{
	return sm_pDefaultAllocatorCB();
}

// $deBlob: FUNCTION 006f0160
T2Allocator* TString16::GetDefaultAllocatorCB()
{
	return GetGlobalAllocator();
}

// $deBlob: FUNCTION 006f0330
TString16::TString16( const TString16& src, T2Allocator* allocator )
{
	m_pAllocator = allocator;
	Reset();
	Copy( src, -1 );
}

// $deBlob: FUNCTION 006f0220
TString16::TString16( TINT size, T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator;
	AllocBuffer( size );
}

// $deBlob: FUNCTION 006f0250
// $Barnyard: FUNCTION 006cc880
TString16::TString16( const TWCHAR* src, T2Allocator* allocator )
{
	m_pAllocator = allocator;
	Reset();
	*this        = src;
}

// $deBlob: FUNCTION 006f0390
// $Barnyard: FUNCTION 006cc8f0
TString16::TString16( const TString8& src, T2Allocator* allocator )
{
	m_pAllocator = allocator;
	Reset();
	*this        = src;
}

// $deBlob: FUNCTION 006f02c0
TString16::TString16( const TCHAR* src, T2Allocator* allocator )
{
	m_pAllocator = allocator;
	Reset();

	TINT iLength = !src ? 0 : TINT( strlen( src ) );
	AllocBuffer( iLength, TTRUE );

	if ( iLength != 0 )
		TStringManager::StringCharToUnicode( m_pBuffer, src, iLength );

	m_pBuffer[ iLength ] = L'\0';
}

// $deBlob: FUNCTION 006f0710
// $Barnyard: FUNCTION 006cc130
void TString16::Copy( const TString16& src, TINT size )
{
	// NOTE: retail doesn't check for copying a string into itself: the first character is cleared before wcsncpy, so the string ends up empty
	if ( size < 0 || size > src.Length() )
		size = src.Length();

	AllocBuffer( size, TTRUE );

	if ( size != 0 )
		wcsncpy( m_pBuffer, src.m_pBuffer, size );

	m_pBuffer[ size ] = L'\0';
}

// $deBlob: FUNCTION 006f07b0
// $Barnyard: FUNCTION 006cc0c0
void TString16::Copy( const TWCHAR* src, TINT size )
{
	// NOTE: unlike Barnyard, de Blob doesn't check for src == m_pBuffer
	TINT srcLen = !src ? 0 : TINT( wcslen( src ) );

	if ( size > srcLen || size == -1 )
		size = srcLen;

	AllocBuffer( size, TTRUE );

	if ( size != 0 )
		wcsncpy( m_pBuffer, src, size );

	m_pBuffer[ size ] = L'\0';
}

// $deBlob: FUNCTION 006f0760
void TString16::Copy( const TString8& src, TINT size )
{
	if ( size > src.Length() || size == -1 )
		size = src.Length();

	AllocBuffer( size, TTRUE );

	if ( size != 0 )
		TStringManager::StringCharToUnicode( m_pBuffer, (const TCHAR*)src, size );

	m_pBuffer[ size ] = L'\0';
}

// $deBlob: FUNCTION 006f0820
void TString16::Copy( const TCHAR* src, TINT size )
{
	TINT srcLen = !src ? 0 : TINT( strlen( src ) );

	if ( size > srcLen || size == -1 )
		size = srcLen;

	AllocBuffer( size, TTRUE );

	if ( size != 0 )
		TStringManager::StringCharToUnicode( m_pBuffer, src, size );

	m_pBuffer[ size ] = L'\0';
}

// $deBlob: FUNCTION 006f0e00
// $Barnyard: FUNCTION 006cca50
TINT TString16::Find( TWCHAR character, TINT pos ) const
{
	if ( pos > Length() || pos < 0 ) return -1;

	const TWCHAR* foundAt = wcschr( GetString() + pos, character );

	if ( foundAt == TNULL )
		return -1;

	return (TINT)( foundAt - GetString() );
}

// $deBlob: FUNCTION 006f0d90
// $Barnyard: FUNCTION 006cc9f0
TINT TString16::Find( const TWCHAR* substr, TINT pos ) const
{
	if ( pos > Length() || pos < 0 ) return -1;

	const TWCHAR* foundAt = wcsstr( GetString() + pos, substr );
	if ( foundAt == TNULL )
		return -1;

	return (TINT)( foundAt - GetString() );
}

// NOTE: de Blob's TString16 falls back to TMalloc/TFree when it has no allocator
// $deBlob: FUNCTION 006f1650
// $Barnyard: FUNCTION 006cbfe0
TBOOL TString16::AllocBuffer( TINT a_iLength, TBOOL freeMemory )
{
	TASSERT( a_iLength >= 0 );

	TBOOL hasChanged;

	if ( a_iLength == TMath::UInt32ToInt32( Length() ) )
	{
		hasChanged = TFALSE;
	}
	else
	{
		if ( a_iLength == 0 )
		{
			if ( freeMemory == TTRUE )
			{
				if ( m_pAllocator )
					m_pAllocator->Free( m_pBuffer );
				else
					TFree( m_pBuffer );
			}

			m_pBuffer    = NullWString;
			m_iExcessLen = 0;

			hasChanged = TTRUE;
		}
		else
		{
			TINT  iLength       = Length();
			TUINT newExcessLen = ( m_iExcessLen - a_iLength ) + iLength;

			if ( newExcessLen <= 0xFF )
			{
				m_iExcessLen = newExcessLen;
				hasChanged   = TFALSE;
			}
			else
			{
				if ( iLength > 0 && freeMemory == TTRUE )
				{
					if ( m_pAllocator != TNULL )
						m_pAllocator->Free( m_pBuffer );
					else
						TFree( m_pBuffer );
				}

				m_pBuffer    = TSTATICCAST( TWCHAR, m_pAllocator ? m_pAllocator->Malloc( ( a_iLength + 1 ) * sizeof( TWCHAR ) ) : TMalloc( ( a_iLength + 1 ) * sizeof( TWCHAR ) ) );
				m_iExcessLen = 0;
				TASSERT( m_pBuffer != TNULL );

				hasChanged = TTRUE;
			}
		}

		m_iStrLen = a_iLength;
	}

	if ( freeMemory == TTRUE ) m_pBuffer[ 0 ] = L'\0';
	return hasChanged;
}

// $deBlob: FUNCTION 006f0430
// $Barnyard: FUNCTION 006cc1a0
TString16& TString16::Format( const TWCHAR* a_pcFormat, ... )
{
	TWCHAR  buffer[ 0x400 ];
	va_list args;

	va_start( args, a_pcFormat );

	TStringManager::String16FormatV( buffer, TARRAYSIZE( buffer ), a_pcFormat, args );
	Copy( buffer, -1 );

	va_end( args );
	return *this;
}

// $deBlob: FUNCTION 006f1140
// $Barnyard: FUNCTION 006cc7d0
TString16& TString16::operator=( const TCHAR* a_szStr )
{
	TINT iLength = !a_szStr ? 0 : TINT( strlen( a_szStr ) );
	AllocBuffer( iLength, TTRUE );

	if ( iLength != 0 )
		TStringManager::StringCharToUnicode( m_pBuffer, a_szStr, iLength );

	m_pBuffer[ iLength ] = L'\0';
	return *this;
}

// $deBlob: FUNCTION 006f04c0
TString16& TString16::VFormat( const TWCHAR* a_pcFormat, va_list a_vargs )
{
	TWCHAR buffer[ 0x400 ];

	TStringManager::String16FormatV( buffer, TARRAYSIZE( buffer ), a_pcFormat, a_vargs );
	Copy( buffer, -1 );

	return *this;
}

void TString16::ForceSetData( TWCHAR* a_cString, TINT a_iLength )
{
	m_pBuffer = a_cString;

	if ( a_iLength < 0 )
	{
		m_iStrLen = TStringManager::String16Length( a_cString );
	}
	else
	{
		m_iStrLen = a_iLength;
	}

	m_iExcessLen = 0;
}

// $deBlob: FUNCTION 006f0e70
TINT TString16::FindReverse( TWCHAR a_findChar, TINT pos ) const
{
	if ( pos == -1 )
	{
		pos = Length();
	}
	else
	{
		if ( !IsIndexValid( pos ) ) return -1;
	}

	const TWCHAR* pBuffer = GetString();

	for ( ; pos >= 0; pos-- )
	{
		if ( pBuffer[ pos ] == a_findChar )
		{
			return pos;
		}
	}

	return -1;
}

// $deBlob: FUNCTION 006f0f00
void TString16::Truncate( TINT length )
{
	TWCHAR* oldBuffer   = m_pBuffer;
	TINT    iOldLength = Length();

	if ( length > iOldLength )
	{
		length = iOldLength;
	}

	TBOOL allocated = AllocBuffer( length, TFALSE );
	if ( allocated == TTRUE )
	{
		TStringManager::String16Copy( m_pBuffer, oldBuffer, length );
	}

	m_pBuffer[ length ] = 0;

	// NOTE: Barnyard's port checked the new length here
	if ( allocated == TTRUE && iOldLength > 0 )
	{
		if ( m_pAllocator != TNULL )
			m_pAllocator->Free( oldBuffer );
		else
			TFree( oldBuffer );
	}
}

// $deBlob: FUNCTION 006f03f0
// $Barnyard: FUNCTION 006ccab0
void TString16::FreeBuffer()
{
	if ( Length() > 0 )
	{
		if ( m_pAllocator != TNULL )
			m_pAllocator->Free( m_pBuffer );
		else
			TFree( m_pBuffer );

		m_pBuffer = TNULL;
	}

	Reset();
}

void TString16::FreeMemory( TWCHAR* a_pMem )
{
	if ( m_pAllocator )
		m_pAllocator->Free( a_pMem );
	else
		TFree( a_pMem );
}

// NOTE: no assert in de Blob
// $deBlob: FUNCTION 006f1550
// $Barnyard: FUNCTION 006cc950
const TWCHAR* TString16::GetString( TINT a_iIndex ) const
{
	if ( IsIndexValid( a_iIndex ) ) { return &m_pBuffer[ a_iIndex ]; }
	return TNULL;
}

// $deBlob: FUNCTION 006f15c0
const TString16& TString16::Print() const
{
	// NOTE: the printing has been compiled out of retail, only the conversion is left
	TASSERT( GetString() != TNULL );
	TStringManager::StringUnicodeToChar( TStringManager::GetTempString8(), GetString(), 1024 );
	return *this;
}

TWCHAR* TString16::GetStringUnsafe( TINT a_iIndex )
{
	TASSERT( a_iIndex >= 0 && a_iIndex <= m_iStrLen );
	if ( IsIndexValid( a_iIndex ) ) { return &m_pBuffer[ a_iIndex ]; }
	return TNULL;
}

// $deBlob: FUNCTION 006f0b00
// $Barnyard: FUNCTION 006cc320
TString16& TString16::Concat( const TWCHAR* str, TINT size )
{
	TINT len = (TINT)TStringManager::String16Length( str );

	if ( size > len || size == -1 )
	{
		size = len;
	}

	TINT    oldLength = m_iStrLen;
	TWCHAR* oldString = m_pBuffer;

	TBOOL allocated = AllocBuffer( oldLength + size, TFALSE );

	if ( allocated == TTRUE )
	{
		// since it has made a new buffer
		// it need to copy the old string
		// to the new buffer

		TStringManager::String16Copy( m_pBuffer, oldString, -1 );
	}

	TStringManager::String16Copy( m_pBuffer + oldLength, str, size );
	m_pBuffer[ oldLength + size ] = 0;

	if ( allocated == TTRUE && oldLength > 0 )
	{
		if ( m_pAllocator != TNULL )
			m_pAllocator->Free( oldString );
		else
			TFree( oldString );
	}

	return *this;
}

// $deBlob: FUNCTION 006f0a50
// $Barnyard: FUNCTION 006cc210
TString16& TString16::Concat( const TString16& str, TINT size )
{
	if ( size > str.Length() || size == -1 )
		size = str.Length();

	TINT    oldLength = m_iStrLen;
	TWCHAR* oldString = m_pBuffer;

	TBOOL allocated = AllocBuffer( oldLength + size, TFALSE );

	if ( allocated == TTRUE )
		TStringManager::String16Copy( m_pBuffer, oldString, -1 );

	TStringManager::String16Copy( m_pBuffer + oldLength, str.m_pBuffer, size );
	m_pBuffer[ oldLength + size ] = 0;

	if ( allocated == TTRUE && oldLength > 0 )
	{
		if ( m_pAllocator != TNULL )
			m_pAllocator->Free( oldString );
		else
			TFree( oldString );
	}

	return *this;
}

// $deBlob: FUNCTION 006f0be0
// $Barnyard: FUNCTION 006cc430
TString16& TString16::Insert( const TWCHAR* a_wszString, TINT a_iPos, TINT a_iCount )
{
	TINT iStrLength = (TINT)TStringManager::String16Length( a_wszString );

	if ( a_iCount > iStrLength || a_iCount == -1 )
		a_iCount = iStrLength;

	TINT    iOldLength = m_iStrLen;
	TWCHAR* pOldString = m_pBuffer;

	TBOOL bAllocated = AllocBuffer( iOldLength + a_iCount, TFALSE );

	if ( bAllocated )
	{
		// Copy the parts before and after the insertion point into the new buffer
		TStringManager::String16Copy( m_pBuffer, pOldString, a_iPos );
		TStringManager::String16Copy( m_pBuffer + a_iPos + a_iCount, pOldString + a_iPos, iOldLength - a_iPos );
	}
	else
	{
		// Shift the tail (and the terminator) to make room
		TStringManager::String16Move( m_pBuffer + a_iPos + a_iCount, m_pBuffer + a_iPos, iOldLength - a_iPos );
	}

	TStringManager::String16Copy( m_pBuffer + a_iPos, a_wszString, a_iCount );
	m_pBuffer[ m_iStrLen ] = 0;

	if ( bAllocated && iOldLength > 0 )
		FreeMemory( pOldString );

	return *this;
}

// $deBlob: FUNCTION 006f0ba0
TString16& TString16::Insert( const TString16& a_rcString, TINT a_iPos, TINT a_iCount )
{
	Insert( a_rcString.GetString(), a_iPos, a_iCount );
	return *this;
}

// $deBlob: FUNCTION 006f0cd0
// $Barnyard: FUNCTION 006cc5b0
TString16& TString16::Delete( TINT a_iPos, TINT a_iCount )
{
	TINT    iOldLength = m_iStrLen;
	TWCHAR* pOldString = m_pBuffer;

	TBOOL bAllocated = AllocBuffer( iOldLength - a_iCount, TFALSE );

	if ( bAllocated == TTRUE )
	{
		TStringManager::String16Copy( m_pBuffer, pOldString, a_iPos );
		TStringManager::String16Copy( m_pBuffer + a_iPos, pOldString + a_iPos + a_iCount, iOldLength - a_iPos - a_iCount );
	}
	else
	{
		TStringManager::String16Copy( m_pBuffer + a_iPos, m_pBuffer + a_iPos + a_iCount, iOldLength - a_iPos - a_iCount );
	}

	m_pBuffer[ m_iStrLen ] = 0;

	if ( bAllocated == TTRUE && iOldLength > 0 )
	{
		if ( m_pAllocator != TNULL )
			m_pAllocator->Free( pOldString );
		else
			TFree( pOldString );
	}

	return *this;
}

// $deBlob: FUNCTION 006f1090
// $Barnyard: FUNCTION 006cc740
TString16& TString16::operator=( const TString8& a_rcStr )
{
	TINT iLength = a_rcStr.Length();
	AllocBuffer( iLength, TTRUE );

	if ( iLength != 0 )
		TStringManager::StringCharToUnicode( m_pBuffer, (const TCHAR*)a_rcStr, iLength );

	m_pBuffer[ iLength ] = L'\0';
	return *this;
}

// $deBlob: FUNCTION 006f10e0
// $Barnyard: FUNCTION 006cc780
TString16& TString16::operator=( const TWCHAR* a_wszStr )
{
#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail doesn't check for self-assignment, which would clear the string
	if ( a_wszStr == m_pBuffer ) return *this;
#endif

	TINT iLength = !a_wszStr ? 0 : TINT( wcslen( a_wszStr ) );
	AllocBuffer( iLength, TTRUE );

	if ( iLength != 0 )
		wcsncpy( m_pBuffer, a_wszStr, iLength );

	m_pBuffer[ iLength ] = L'\0';
	return *this;
}

// $deBlob: FUNCTION 006f0880
// $Barnyard: FUNCTION 006cc980
TINT TString16::Compare( const TWCHAR* a_pcString, TINT a_iLength ) const
{
	TASSERT( a_pcString != TNULL );
	TASSERT( GetString() != TNULL );

	if ( a_iLength == -1 )
		return wcscmp( GetString(), a_pcString );

	return wcsncmp( GetString(), a_pcString, a_iLength );
}

TString16& TString16::Reserve( TINT size )
{
	if ( size > Length() + TINT( ExcessLength() ) )
	{
		TINT    iOldLength = m_iStrLen;
		TWCHAR* pOldString = m_pBuffer;
		TINT    iCopySize  = TMath::Min( iOldLength, size );

		TBOOL bAllocated = AllocBuffer( size, TFALSE );

		if ( bAllocated )
		{
			TStringManager::String16Copy( m_pBuffer, pOldString, iCopySize );
			m_pBuffer[ iCopySize ] = 0;
		}

		if ( bAllocated && iOldLength != 0 )
			FreeMemory( pOldString );
	}

	return *this;
}

// $deBlob: FUNCTION 006f0970
TINT TString16::CompareNoCase( const TWCHAR* a_pcString, TINT param_2 ) const
{
	TASSERT( a_pcString != TNULL );
	TASSERT( GetString() != TNULL );

	if ( param_2 == -1 )
	{
		return _wcsicmp( GetString(), a_pcString );
	}

	return _wcsnicmp( GetString(), a_pcString, param_2 );
}

// $deBlob: FUNCTION 006f0550
TString16 TString16::Mid( TINT a_iFirst, TINT a_iCount ) const
{
	if ( a_iFirst < 0 )
	{
		a_iFirst = 0;
	}
	else if ( Length() <= a_iFirst )
	{
		// Can't return string bigger that the original
		return TString16();
	}

	if ( a_iCount < 0 || Length() < a_iFirst + a_iCount )
	{
		a_iCount = Length() - a_iFirst;
	}

	TString16 strResult = TString16( a_iCount );
	wcsncpy( strResult.m_pBuffer, m_pBuffer + a_iFirst, a_iCount );
	strResult.m_pBuffer[ a_iCount ] = 0;

	return strResult;
}

TString16 TString16::Right( TINT a_iFrom ) const
{
	return Mid( a_iFrom, Length() - a_iFrom );
}

// NOTE: de Blob only fails on an upper case character (Barnyard's port required every character to be lower case)
// $deBlob: FUNCTION 006f0f80
TBOOL TString16::IsAllLowerCase() const
{
	for ( TINT i = 0; i < Length(); i++ )
	{
		if ( iswupper( m_pBuffer[ i ] ) ) return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f0fc0
TBOOL TString16::IsAllUpperCase() const
{
	for ( TINT i = 0; i < Length(); i++ )
	{
		if ( iswlower( m_pBuffer[ i ] ) ) return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f1000
TBOOL TString16::IsAllAlphaNumeric() const
{
	for ( TINT i = 0; i < Length(); i++ )
	{
		TWCHAR cChar = m_pBuffer[ i ];
		if ( !iswalnum( cChar ) && cChar != L'_' ) return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f11e0
TString16 TString16::operator+( const TString16& a_rcString ) const
{
	TString16 strResult( *this );
	strResult.Concat( a_rcString, -1 );
	return strResult;
}

// $deBlob: FUNCTION 006f12c0
TString16 TString16::operator+( const TWCHAR* a_pRHS ) const
{
	TString16 strResult( *this );
	TASSERT( a_pRHS );
	strResult.Concat( a_pRHS, -1 );
	return strResult;
}

// $deBlob: FUNCTION 006f1810
TString16 operator+( const TWCHAR* a_pLHS, const TString16& a_rcString )
{
	TString16 strResult( a_pLHS );
	TASSERT( a_pLHS );
	strResult.Concat( a_rcString, -1 );
	return strResult;
}

TOSHI_NAMESPACE_END

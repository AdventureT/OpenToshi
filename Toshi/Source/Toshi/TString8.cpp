#include "ToshiPCH.h"
#include "TString8.h"
#include "TString16.h"

#include "Toshi/TSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TString8::func_DefaultAllocatorCB TString8::sm_pDefaultAllocatorCB = &GetDefaultAllocatorCB;

// $deBlob: FUNCTION 006ec650
// $Barnyard: FUNCTION 006bd790
TString8::TString8( T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	AllocBuffer( 0 );
}

// $deBlob: FUNCTION 006ec8d0
// $Barnyard: FUNCTION 006bd910
TString8::TString8( const TString8& src, T2Allocator* allocator ) TNOEXCEPT
{
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	Reset();
	Copy( src, -1 );
}

// $deBlob: FUNCTION 006ec720
// $Barnyard: FUNCTION 006bd7f0
TString8::TString8( TINT size, T2Allocator* allocator )
{
	Reset();
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	AllocBuffer( size );
}

// $deBlob: FUNCTION 006ec960
TString8::TString8( const TString16& src, T2Allocator* allocator )
{
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	Reset();
	Copy( src );
}

// $deBlob: FUNCTION 006ec790
// $Barnyard: FUNCTION 006bd890
TString8::TString8( const TCHAR* src, T2Allocator* allocator )
{
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	Reset();
	Copy( src );
}

// $deBlob: FUNCTION 006ec830
// $Barnyard: FUNCTION 006bd8d0
TString8::TString8( const TWCHAR* src, T2Allocator* allocator )
{
	m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
	Reset();
	Copy( src );
}

#ifdef TOSHI_STRING_MOVE
TString8::TString8( TString8&& src ) TNOEXCEPT
{
	TString8::m_pAllocator = src.m_pAllocator;
	TString8::m_iExcessLen = src.m_iExcessLen;
	TString8::m_iStrLen    = src.m_iStrLen;
	TString8::m_pBuffer    = src.m_pBuffer;
	src.Reset();
}
#endif

// $deBlob: FUNCTION 005533a0
TString8 TString8::Right( TINT a_iCount ) const
{
	return Mid( Length() - a_iCount, a_iCount );
}

// $deBlob: FUNCTION 006ed930
// $Barnyard: FUNCTION 006bcfc0
TString8& TString8::MakeLower()
{
	if ( m_pBuffer ) _strlwr( m_pBuffer );
	return *this;
}

// $deBlob: FUNCTION 006edd70
// $Barnyard: FUNCTION 006bd6e0
TString8& TString8::operator+=( const TCHAR* cstr )
{
	Concat( cstr, -1 );
	return *this;
}

// $deBlob: FUNCTION 006edc60
// $Barnyard: FUNCTION 006bd6c0
TString8& TString8::operator+=( const TString8& str )
{
	Concat( str, -1 );
	return *this;
}

// $deBlob: FUNCTION 006edce0
TString8& TString8::operator+=( const TString16& str )
{
	Concat( str, -1 );
	return *this;
}

// $deBlob: FUNCTION 006eda60
TString8::operator const TCHAR*() const
{
	return m_pBuffer;
}

// $deBlob: FUNCTION 006edbb0
// $Barnyard: FUNCTION 006bd6a0
TString8& TString8::operator=( const TCHAR* cstr )
{
	Copy( cstr, -1 );
	return *this;
}

// $deBlob: FUNCTION 006edc00
TString8& TString8::operator=( const TWCHAR* wcstr )
{
	Copy( wcstr, -1 );
	return *this;
}

// $deBlob: FUNCTION 006edb30
// $Barnyard: FUNCTION 006bd680
TString8& TString8::operator=( const TString8& str )
{
	Copy( str, -1 );
	return *this;
}

// $deBlob: FUNCTION 006edb70
TString8& TString8::operator=( const TString16& str )
{
	Copy( str, -1 );
	return *this;
}

// $deBlob: FUNCTION 006ec610
T2Allocator* TString8::GetAllocator()
{
	TASSERT( TNULL != sm_pDefaultAllocatorCB );
	return sm_pDefaultAllocatorCB();
}

// $deBlob: FUNCTION 006ec600
T2Allocator* TString8::GetDefaultAllocatorCB()
{
	return GetGlobalAllocator();
}

// $deBlob: FUNCTION 006ece00
void TString8::Copy( const TString16& src, TINT size )
{
	TINT srcLen = src.Length();

	if ( size > srcLen || size == -1 ) { size = srcLen; }

	AllocBuffer( size, TTRUE );
	TStringManager::StringUnicodeToChar( m_pBuffer, src, size );
	m_pBuffer[ size ] = 0;
}

// $deBlob: FUNCTION 006ecdb0
// $Barnyard: FUNCTION 006bd300
void TString8::Copy( const TString8& src, TINT size )
{
	if ( this != &src )
	{
		if ( size > src.Length() || size == -1 )
			size = src.Length();

		AllocBuffer( size, TTRUE );
		TUtil::MemCopy( m_pBuffer, src.m_pBuffer, size );

		m_pBuffer[ size ] = 0;
	}
}

// $deBlob: FUNCTION 006ece50
// $Barnyard: FUNCTION 006bd180
void TString8::Copy( const TCHAR* src, TINT size )
{
	if ( m_pBuffer != src )
	{
		TINT srcLen = !src ? 0 : TINT( TStringManager::String8Length( src ) );

		if ( size > srcLen || size == -1 )
		{
			size = srcLen;
		}

		AllocBuffer( size, TTRUE );
		TUtil::MemCopy( m_pBuffer, src, size );

		m_pBuffer[ size ] = 0;
	}
}

// $deBlob: FUNCTION 006ecea0
// $Barnyard: FUNCTION 006bd240
void TString8::Copy( const TWCHAR* src, TINT size )
{
	TINT srcLen = !src ? 0 : TINT( TStringManager::String16Length( src ) );

	if ( size > srcLen || size == -1 ) { size = srcLen; }

	AllocBuffer( size, TTRUE );

	if ( src )
		TStringManager::StringUnicodeToChar( m_pBuffer, src, size );

	m_pBuffer[ size ] = 0;
}

// $deBlob: FUNCTION 006ed7a0
// $Barnyard: FUNCTION 006bcf30
TINT TString8::Find( TCHAR character, TINT pos ) const
{
	if ( pos > Length() || pos < 0 ) return -1;

	const TCHAR* foundAt = strchr( GetString() + pos, character );
	if ( foundAt != TNULL )
		return (TINT)( foundAt - GetString() );

	return -1;
}

// $deBlob: FUNCTION 006ed6d0
// $Barnyard: FUNCTION 006bcef0
TINT TString8::Find( const TCHAR* substr, TINT pos ) const
{
	if ( pos > Length() || pos < 0 ) return -1;

	const TCHAR* foundAt = strstr( GetString() + pos, substr );
	if ( foundAt == TNULL )
		return -1;

	return (TINT)( foundAt - GetString() );
}

// $deBlob: FUNCTION 006ee970
TBOOL TString8::AllocBuffer( TINT a_iLength, TBOOL a_bFreeMemory )
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
			if ( a_bFreeMemory == TTRUE ) m_pAllocator->Free( m_pBuffer );

			m_pBuffer    = NullString;
			m_iExcessLen = 0;

			hasChanged = TTRUE;
		}
		else
		{
			TINT   iLength        = Length();
			TUINT  newExcessLen   = ( m_iExcessLen - a_iLength ) + iLength;

			if ( newExcessLen <= 0xFF )
			{
				m_iExcessLen = newExcessLen;
				hasChanged   = TFALSE;
			}
			else
			{
				if ( iLength > 0 && a_bFreeMemory == TTRUE )
				{
					m_pAllocator->Free( m_pBuffer );
				}

				m_pBuffer    = (TCHAR*)m_pAllocator->Malloc( a_iLength + 1 );
				m_iExcessLen = 0;
				TASSERT( m_pBuffer != TNULL );

				hasChanged = TTRUE;
			}
		}

		m_iStrLen = a_iLength;
	}

	if ( a_bFreeMemory == TTRUE ) m_pBuffer[ 0 ] = '\0';
	return hasChanged;
}

// $deBlob: FUNCTION 006eca20
// $Barnyard: FUNCTION 006bd3c0
TString8& TString8::Format( const TCHAR* a_pcFormat, ... )
{
	va_list args;
	va_start( args, a_pcFormat );

	TCHAR buffer[ 1024 ];
	TStringManager::String8FormatV( buffer, sizeof( buffer ), a_pcFormat, args );
	Copy( buffer, -1 );

	return *this;
}

// $deBlob: FUNCTION 006ecaa0
TString8& TString8::VFormat( const TCHAR* a_pcFormat, va_list a_vargs )
{
	TCHAR buffer[ 0x400 ];
	TStringManager::String8FormatV( buffer, sizeof( buffer ), a_pcFormat, a_vargs );
	Copy( buffer, -1 );

	return *this;
}

TString8 TString8::VarArgs( const TCHAR* a_pcFormat, ... )
{
	TCHAR    buffer[ 0x400 ];
	TString8 buffer2;
	va_list  args;

	va_start( args, a_pcFormat );

	// TASSERT is only in T2String8
	TINT iResult = _vsnprintf( buffer, sizeof( buffer ), a_pcFormat, args );
	TASSERT( iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem" );
	buffer2.Copy( buffer, -1 );

	return buffer2;
}

// $deBlob: FUNCTION 006ee6c0
void TString8::ForceSetData( TCHAR* a_cString, TINT a_iLength )
{
	TASSERT( TNULL == m_pBuffer || NullString == m_pBuffer );
	TASSERT( 0 == m_iStrLen );
	TASSERT( 0 == m_iExcessLen );

	m_pBuffer = a_cString;

	if ( a_iLength < 0 )
	{
		m_iStrLen = TINT( TStringManager::String8Length( a_cString ) );
	}
	else
	{
		m_iStrLen = a_iLength;
	}

	m_iExcessLen = 0;
}

// $deBlob: FUNCTION 006ed880
// $Barnyard: FUNCTION 006bcf70
TINT TString8::FindReverse( TCHAR a_findChar, TINT pos ) const
{
	if ( pos == -1 )
	{
		pos = Length();
	}
	else
	{
		if ( pos > Length() || pos < 0 ) return -1;
	}

	const TCHAR* pBuffer = GetString();

	for ( ; pos >= 0; pos-- )
	{
		if ( pBuffer[ pos ] == a_findChar )
		{
			return pos;
		}
	}

	return -1;
}

// $deBlob: FUNCTION 006ed950
// $Barnyard: FUNCTION 006bd5d0
void TString8::Truncate( TINT length )
{
	if ( length > Length() )
	{
		length = Length();
	}

	TCHAR* oldBuffer  = m_pBuffer;
	TINT   iOldLength = Length();

	TBOOL allocated = AllocBuffer( length, TFALSE );
	if ( allocated == TTRUE )
	{
		TStringManager::String8Copy( m_pBuffer, oldBuffer, length );
	}

	m_pBuffer[ length ] = 0;

	if ( allocated == TTRUE && iOldLength > 0 )
	{
		m_pAllocator->Free( oldBuffer );
	}
}

// $deBlob: FUNCTION 006ec9f0
// $deBlob: FUNCTION 0075af40
void TString8::FreeBuffer()
{
	if ( Length() > 0 )
	{
		m_pAllocator->Free( m_pBuffer );
		m_pBuffer = TNULL;
	}

	Reset();
}

// NOTE: de Blob only asserts the index (Barnyard returned TNULL for an invalid one, see GetStringSafe)
// $deBlob: FUNCTION 006eda70
// $Barnyard: FUNCTION 006bd110
const TCHAR* TString8::GetString( TINT a_iIndex ) const
{
	TASSERT( ( a_iIndex >= 0 ) && ( a_iIndex <= (TINT)m_iStrLen ) );
	return &m_pBuffer[ a_iIndex ];
}

// $deBlob: FUNCTION 006edad0
TCHAR* TString8::GetStringUnsafe( TINT a_iIndex )
{
	TASSERT( ( a_iIndex >= 0 ) && ( a_iIndex <= (TINT)m_iStrLen ) );
	return &m_pBuffer[ a_iIndex ];
}

// $deBlob: FUNCTION 006ee5d0
const TCHAR* TString8::GetStringSafe( TINT a_iIndex ) const
{
	if ( IsIndexValid( a_iIndex ) )
		return &m_pBuffer[ a_iIndex ];

	return TNULL;
}

// $deBlob: FUNCTION 006ee600
TNOINLINE TBOOL TString8::IsIndexValid( TINT a_iIndex ) const
{
	return a_iIndex <= Length() && a_iIndex >= 0;
}

// $deBlob: FUNCTION 006ee7f0
TCHAR TString8::SetChar( TINT a_iIndex, TCHAR a_cChar )
{
	TASSERT( a_iIndex < m_iStrLen );
	m_pBuffer[ a_iIndex ] = a_cChar;
	return a_cChar;
}

// $deBlob: FUNCTION 006ee640
const TString8& TString8::Print() const
{
	// NOTE: the printing has been compiled out of retail
	TASSERT( GetString() != TNULL );
	return *this;
}

// $deBlob: FUNCTION 006ed360
// $Barnyard: FUNCTION 006bd410
TString8& TString8::Concat( const TString8& str, TINT size )
{
	if ( size > str.Length() || size == -1 )
		size = str.Length();

	TINT   oldLength = m_iStrLen;
	TCHAR* oldString = m_pBuffer;

	TBOOL allocated = AllocBuffer( oldLength + size, TFALSE );

	if ( allocated == TTRUE )
		TStringManager::String8Copy( m_pBuffer, oldString, -1 );

	TStringManager::String8Copy( m_pBuffer + oldLength, str.m_pBuffer, size );
	m_pBuffer[ oldLength + size ] = 0;

	if ( allocated == TTRUE && oldLength > 0 )
		m_pAllocator->Free( oldString );

	return *this;
}

// $deBlob: FUNCTION 006ed480
// $Barnyard: FUNCTION 006bd4f0
TString8& TString8::Concat( const TCHAR* str, TINT size )
{
	TINT len = TINT( TStringManager::String8Length( str ) );

	if ( ( len < size ) || ( size == -1 ) )
	{
		size = len;
	}

	TINT   oldLength = m_iStrLen;
	TCHAR* oldString = m_pBuffer;

	TBOOL allocated = AllocBuffer( m_iStrLen + size, TFALSE );

	if ( allocated )
	{
		// since it has made a new buffer
		// it needs to copy the old string
		// to the new buffer

		TStringManager::String8Copy( m_pBuffer, oldString, -1 );
	}

	TStringManager::String8Copy( m_pBuffer + oldLength, str, size );
	m_pBuffer[ m_iStrLen ] = 0;

	if ( allocated && oldLength != 0 )
	{
		m_pAllocator->Free( oldString );
	}

	return *this;
}

TString8& TString8::Reserve( TINT size )
{
	if ( size > Length() + TINT( ExcessLength() ) )
	{
		TINT   iOldLength = m_iStrLen;
		TCHAR* pOldString = m_pBuffer;
		TINT   iCopySize  = TMath::Min( iOldLength, size );

		// Reallocate buffer if necessary
		TBOOL bAllocated = AllocBuffer( size, TFALSE );

		// Update excess length
		TASSERT( iCopySize <= size );
		AllocBuffer( iCopySize, TFALSE );

		if ( bAllocated )
		{
			TStringManager::String8Copy( m_pBuffer, pOldString, iCopySize );
			m_pBuffer[ iCopySize ] = 0;
		}

		if ( bAllocated && iOldLength != 0 )
			m_pAllocator->Free( pOldString );
	}

	return *this;
}

// $deBlob: FUNCTION 006ecef0
// $Barnyard: FUNCTION 006bce60
TINT TString8::Compare( const TCHAR* a_pcString, TINT a_iLength ) const
{
	TASSERT( a_pcString != TNULL );
	TASSERT( GetString() != TNULL );

	if ( a_iLength != -1 )
		return strncmp( GetString(), a_pcString, a_iLength );

	return strcmp( GetString(), a_pcString );
}

// $deBlob: FUNCTION 006ed070
// $Barnyard: FUNCTION 006bcec0
TINT TString8::CompareNoCase( const TCHAR* a_pcString, TINT a_iLength ) const
{
	TASSERT( a_pcString != TNULL );
	TASSERT( GetString() != TNULL );

	if ( a_iLength == -1 )
		return _stricmp( GetString(), a_pcString );

	return _strnicmp( GetString(), a_pcString, a_iLength );
}

// $deBlob: FUNCTION 006ecb20
// $Barnyard: FUNCTION 006bd950
TString8 TString8::Mid( TINT a_iFirst, TINT a_iCount ) const
{
	if ( a_iFirst < 0 )
	{
		a_iFirst = 0;
	}
	else if ( Length() <= a_iFirst )
	{
		// Can't return string bigger that the original
		return TString8();
	}

	if ( a_iCount < 0 || Length() < a_iFirst + a_iCount )
	{
		a_iCount = Length() - a_iFirst;
	}

	TString8 strResult = TString8( a_iCount );
	TUtil::MemCopy( strResult.m_pBuffer, m_pBuffer + a_iFirst, a_iCount );
	strResult.m_pBuffer[ a_iCount ] = 0;

	return strResult;
}

// $deBlob: FUNCTION 006ed3f0
TString8& TString8::Concat( const TString16& str, TINT size )
{
	TINT len = str.Length();

	if ( ( len < size ) || ( size == -1 ) )
	{
		size = len;
	}

	TUINT32 oldLength = m_iStrLen;
	TCHAR*  oldString = m_pBuffer;

	TBOOL allocated = AllocBuffer( m_iStrLen + size, TFALSE );

	if ( allocated )
	{
		// since it has made a new buffer
		// it needs to copy the old string
		// to the new buffer

		TStringManager::String8Copy( m_pBuffer, oldString, -1 );
	}

	TStringManager::StringUnicodeToChar( m_pBuffer + oldLength, str.GetString(), size );
	m_pBuffer[ m_iStrLen ] = 0;

	// NOTE: Barnyard's port checked the new length here, de Blob checks the old one like the other Concats
	if ( allocated && oldLength > 0 )
	{
		m_pAllocator->Free( oldString );
	}

	return *this;
}

// $deBlob: FUNCTION 006ed9b0
// $Barnyard: FUNCTION 006bd0e0
TBOOL TString8::IsAllLowerCase() const
{
	for ( TINT i = 0; i < m_iStrLen; i++ )
	{
		TCHAR cChar = m_pBuffer[ i ];
		if ( cChar >= 'A' && cChar <= 'Z' ) return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006ed9e0
TBOOL TString8::IsAllUpperCase() const
{
	for ( TINT i = 0; i < m_iStrLen; i++ )
	{
		TCHAR cChar = m_pBuffer[ i ];
		if ( cChar >= 'a' && cChar <= 'z' ) return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006eda10
TBOOL TString8::IsAllAlphaNumeric() const
{
	for ( TINT i = 0; i < Length(); i++ )
	{
		TCHAR cChar = m_pBuffer[ i ];
		if ( !isalnum( cChar ) && cChar != '_' ) return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006ed570
TString8& TString8::Insert( const TCHAR* a_pcString, TINT a_iIndex, TINT a_iLength )
{
	TINT iLength = TINT( TStringManager::String8Length( a_pcString ) );

	if ( a_iLength > iLength || a_iLength == -1 )
		a_iLength = iLength;

	TINT   iOldLength = m_iStrLen;
	TCHAR* pOldString = m_pBuffer;
	TBOOL  bAllocated = AllocBuffer( iOldLength + a_iLength, TFALSE );

	if ( bAllocated )
	{
		TStringManager::String8Copy( m_pBuffer, pOldString, a_iIndex );
		TStringManager::String8Copy( m_pBuffer + a_iIndex + a_iLength, pOldString + a_iIndex, iOldLength - a_iIndex );
	}
	else
	{
		TStringManager::String8Move( m_pBuffer + a_iIndex + a_iLength, m_pBuffer + a_iIndex, iOldLength - a_iIndex );
	}

	TStringManager::String8Copy( m_pBuffer + a_iIndex, a_pcString, a_iLength );
	m_pBuffer[ iOldLength + a_iLength ] = '\0';

	if ( bAllocated && iOldLength > 0 )
		m_pAllocator->Free( pOldString );

	return *this;
}

// $deBlob: FUNCTION 006ed510
TString8& TString8::Insert( const TString8& a_rcString, TINT a_iIndex, TINT a_iLength )
{
	Insert( a_rcString.GetString(), a_iIndex, a_iLength );
	return *this;
}

// $deBlob: FUNCTION 006ed630
TString8& TString8::Delete( TINT a_iIndex, TINT a_iCount )
{
	TINT   iOldLength = m_iStrLen;
	TCHAR* pOldString = m_pBuffer;
	TBOOL  bAllocated = AllocBuffer( iOldLength - a_iCount, TFALSE );

	if ( bAllocated )
	{
		TStringManager::String8Copy( m_pBuffer, pOldString, a_iIndex );
		TStringManager::String8Copy( m_pBuffer + a_iIndex, pOldString + a_iIndex + a_iCount, iOldLength - a_iIndex - a_iCount );
	}
	else
	{
		// NOTE: overlapping forward copy, safe since the destination is before the source
		TStringManager::String8Copy( m_pBuffer + a_iIndex, m_pBuffer + a_iIndex + a_iCount, iOldLength - a_iIndex - a_iCount );
	}

	m_pBuffer[ iOldLength - a_iCount ] = '\0';

	if ( bAllocated && iOldLength > 0 )
		m_pAllocator->Free( pOldString );

	return *this;
}

// $deBlob: FUNCTION 006ed1d0
TBOOL TString8::IsEqual( const TString8& a_rcString ) const
{
	if ( Length() != a_rcString.Length() )
		return TFALSE;

	return strcmp( GetString(), a_rcString.GetString() ) == 0;
}

// $deBlob: FUNCTION 006ed2b0
TBOOL TString8::IsEqualNoCase( const TString8& a_rcString ) const
{
	if ( Length() != a_rcString.Length() )
		return TFALSE;

	return _stricmp( GetString(), a_rcString.GetString() ) == 0;
}

// $deBlob: FUNCTION 006ee860
TINT TString8::ToInt() const
{
	return TStringManager::String8ToInt( GetString() );
}

// $deBlob: FUNCTION 006ee8b0
TFLOAT TString8::ToFloat() const
{
	return TStringManager::String8ToFloat( GetString() );
}

// $deBlob: FUNCTION 006ee900
TBOOL TString8::ToBool() const
{
	return TStringManager::String8ToBool( GetString() );
}

// $deBlob: FUNCTION 006eddf0
TString8 TString8::operator+( const TString8& str ) const
{
	TString8 strResult( *this );
	strResult += str;

	return strResult;
}

// $deBlob: FUNCTION 006edf20
TString8 TString8::operator+( const TString16& str ) const
{
	TString8 strResult( *this );
	strResult += str;

	return strResult;
}

// $deBlob: FUNCTION 006ee250
TString8 TString8::operator+( const TWCHAR* a_pRHS ) const
{
	TString8 strResult( *this );
	TASSERT( a_pRHS );
	strResult += TString8( a_pRHS );

	return strResult;
}

TBOOL TString8::StartsWith( const TCHAR* a_szString, TINT a_iLength /*= -1 */ ) const
{
	TINT iStringLength = ( a_iLength != -1 ) ? a_iLength : T2String8::Length( a_szString );

	if ( Length() < iStringLength )
		return TFALSE;

	return Compare( a_szString, iStringLength ) == 0;
}

TBOOL TString8::EndsWith( const TCHAR* a_szString, TINT a_iLength /*= -1 */ ) const
{
	TINT iStringLength = ( a_iLength != -1 ) ? a_iLength : T2String8::Length( a_szString );

	if ( Length() < iStringLength )
		return TFALSE;

	return strncmp( GetString( Length() - iStringLength ), a_szString, iStringLength ) == 0;
}

TBOOL TString8::StartsWithNoCase( const TCHAR* a_szString, TINT a_iLength /*= -1 */ ) const
{
	TINT iStringLength = ( a_iLength != -1 ) ? a_iLength : T2String8::Length( a_szString );

	if ( Length() < iStringLength )
		return TFALSE;

	return CompareNoCase( a_szString, iStringLength ) == 0;
}

TBOOL TString8::EndsWithNoCase( const TCHAR* a_szString, TINT a_iLength /*= -1 */ ) const
{
	TINT iStringLength = ( a_iLength != -1 ) ? a_iLength : T2String8::Length( a_szString );

	if ( Length() < iStringLength )
		return TFALSE;

	return strnicmp( GetString( Length() - iStringLength ), a_szString, iStringLength ) == 0;
}

// $deBlob: FUNCTION 006ee050
// $Barnyard: FUNCTION 006bdab0
TString8 TString8::operator+( const TCHAR* cstr ) const
{
	TString8     strResult( *this );
	const TCHAR* a_pRHS = cstr;
	TASSERT( a_pRHS );
	strResult.Concat( TString8( cstr ) );

	return strResult;
}

// $deBlob: FUNCTION 006eeaf0
// $Barnyard: FUNCTION 006bdb80
TString8 operator+( const TCHAR* a_pcString, const TString8& a_rcString )
{
	TString8     strResult( a_pcString );
	const TCHAR* a_pLHS = a_pcString;
	TASSERT( a_pLHS );
	strResult.Concat( a_rcString );

	return strResult;
}

TOSHI_NAMESPACE_END

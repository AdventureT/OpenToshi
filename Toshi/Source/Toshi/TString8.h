/**
 * @file TString8.h
 * Char string implementation for the Toshi engine
 * 
 * This file provides string functionality including:
 * - Char string manipulation
 * - String memory management
 * - String formatting and parsing
 * - String comparison and searching
 */

#pragma once
#include "TString.h"
#include "T2Allocator.h"

TOSHI_NAMESPACE_START

class TString16;

class TString8
{
public:
	// $Barnyard: FUNCTION 004024c0
	// NOTE: inline in retail, it just calls TString8( TNULL )
	TString8()
	    : TString8( TSTATICCAST( T2Allocator, TNULL ) )
	{}
	TString8( T2Allocator* allocator );
#ifdef TOSHI_STRING_MOVE
	TString8( TString8&& src ) TNOEXCEPT;
#endif
	TString8( const TString8& src, T2Allocator* allocator = TNULL ) TNOEXCEPT;
	TString8( const TString16& src, T2Allocator* allocator = TNULL );
	TString8( const TCHAR* src, T2Allocator* allocator = TNULL );
	TString8( const TWCHAR* src, T2Allocator* allocator = TNULL );
	TString8( TINT size, T2Allocator* allocator = TNULL );
	// $deBlob: FUNCTION 006eeac0
	// $Barnyard: FUNCTION 006bd140
	~TString8() { FreeBuffer(); }

	void Copy( const TString8& src, TINT size = -1 );
	void Copy( const TString16& src, TINT size = -1 );
	void Copy( const TCHAR* src, TINT size = -1 );
	void Copy( const TWCHAR* src, TINT size = -1 );

	void FreeBuffer();

	// Returns TTRUE if allocated memory
	TBOOL AllocBuffer( TINT a_iLength, TBOOL a_bFreeMemory = TTRUE );

	TString8& Format( const TCHAR* a_pcFormat, ... );
	TString8& VFormat( const TCHAR* a_pcFormat, va_list a_vargs );

	static TString8 VarArgs( const TCHAR* a_pcFormat, ... );

	// $deBlob: FUNCTION 006ee7b0
	void UndoForceSetData() { Reset(); }
	void ForceSetData( TCHAR* a_pchString, TINT a_iLength );

	TINT FindReverse( TCHAR a_findChar, TINT pos = -1 ) const;

	void Truncate( TINT length );

	// Returns position of specified character
	TINT Find( TCHAR character, TINT pos = 0 ) const;

	// Returns position of specified substring
	TINT Find( const TCHAR* substr, TINT pos = 0 ) const;

	// Returns const string starting from specified a_iIndex
	const TCHAR* GetString( TINT a_iIndex ) const;

	// Returns string starting from specified a_iIndex. Use only if you are sure about what you do!
	TCHAR* GetStringUnsafe( TINT a_iIndex = 0 );

	// Returns TNULL if the index is not valid
	const TCHAR* GetStringSafe( TINT a_iIndex ) const;

	TCHAR SetChar( TINT a_iIndex, TCHAR a_cChar );

	const TString8& Print() const;

	TString8& Insert( const TCHAR* a_pcString, TINT a_iIndex, TINT a_iLength = -1 );
	TString8& Insert( const TString8& a_rcString, TINT a_iIndex, TINT a_iLength = -1 );
	TString8& Delete( TINT a_iIndex, TINT a_iCount );

	TBOOL IsEqual( const TString8& a_rcString ) const;
	TBOOL IsEqualNoCase( const TString8& a_rcString ) const;

	TINT   ToInt() const;
	TFLOAT ToFloat() const;
	TBOOL  ToBool() const;

	TString8& Concat( const TString8& str, TINT size = -1 );
	TString8& Concat( const TString16& src, TINT size = -1 );
	TString8& Concat( const TCHAR* src, TINT size = -1 );

	TString8& Reserve( TINT size );

	TINT Compare( const TCHAR* a_szString, TINT a_iLength = -1 ) const;
	TINT CompareNoCase( const TCHAR* a_szString, TINT a_iLength = -1 ) const;

	// Returns a substring contained in a [a_iFirst; a_iFirst + a_iCount] range
	TString8 Mid( TINT a_iFirst, TINT a_iCount ) const;

	// Returns the first a_iCount characters
	// $Barnyard: FUNCTION 0060ac20
	TString8 Left( TINT a_iCount ) const { return Mid( 0, a_iCount ); }

	// Returns the last a_iCount characters (de Blob; Barnyard's Right takes the first index instead)
	TString8 Right( TINT a_iCount ) const;

	// $deBlob: FUNCTION 006ed910
	TString8& MakeUpper()
	{
		if ( m_pBuffer ) _strupr( m_pBuffer );
		return *this;
	}

	TString8& MakeLower();

	TINT    Length() const { return m_iStrLen; }
	TUINT16 ExcessLength() const { return m_iExcessLen; }

	TBOOL StartsWith( const TCHAR* a_szString, TINT a_iLength = -1 ) const;
	TBOOL EndsWith( const TCHAR* a_szString, TINT a_iLength = -1 ) const;
	TBOOL StartsWithNoCase( const TCHAR* a_szString, TINT a_iLength = -1 ) const;
	TBOOL EndsWithNoCase( const TCHAR* a_szString, TINT a_iLength = -1 ) const;

	TBOOL IsAllLowerCase() const;
	TBOOL IsAllUpperCase() const;
	TBOOL IsAllAlphaNumeric() const;
	TBOOL IsIndexValid( TINT a_iIndex ) const;
	// $deBlob: FUNCTION 006edc50
	TBOOL IsEmpty() const { return m_iStrLen == 0; }
	// $deBlob: FUNCTION 006ee630
	TBOOL IsUnicode() const { return TFALSE; } // Who would have known?

public:
	TString8 operator+( const TCHAR* cstr ) const;
	TString8 operator+( const TWCHAR* a_pRHS ) const;
	TString8 operator+( const TString8& str ) const;
	TString8 operator+( const TString16& str ) const;

	TString8& operator+=( const TCHAR* cstr );

	TString8& operator+=( const TString8& str );

	TString8& operator+=( const TString16& str );

	TCHAR&       operator[]( TINT a_iIndex ) { return m_pBuffer[ a_iIndex ]; }
	const TCHAR& operator[]( TINT a_iIndex ) const { return *GetString( a_iIndex ); }

	// $deBlob: FUNCTION 005532e0
	const TCHAR* GetString() const
	{
		TASSERT( IsIndexValid( 0 ) );
		return m_pBuffer;
	}

	// NOTE: retail's conversion returns the buffer without the assert (3 bytes)
	operator const TCHAR*() const;
	operator const TBOOL() const { return !IsEmpty(); }

	TBOOL operator!() { return m_iStrLen == 0; }
	// NOTE: de Blob compares through Compare (Barnyard: strcmp, and its < / <= were inverted)
	// $deBlob: FUNCTION 006ee470
	// $Barnyard: FUNCTION 006bd040
	TBOOL operator==( const TCHAR* cstr ) const { return Compare( cstr, -1 ) ? TFALSE : TTRUE; }
	// $deBlob: FUNCTION 006ee450
	// $Barnyard: FUNCTION 006bcff0
	TBOOL operator==( const TString8& str ) const { return Compare( str.m_pBuffer, -1 ) ? TFALSE : TTRUE; }
	// $deBlob: FUNCTION 006ee4b0
	TBOOL operator!=( const TCHAR* cstr ) const { return Compare( cstr, -1 ) != 0; }
	// $deBlob: FUNCTION 006ee490
	TBOOL operator!=( const TString8& str ) const { return Compare( str.m_pBuffer, -1 ) != 0; }
	// $deBlob: FUNCTION 006ee4f0
	TBOOL operator<( const TCHAR* cstr ) const { return Compare( cstr, -1 ) < 0; }
	// $deBlob: FUNCTION 006ee4d0
	TBOOL operator<( const TString8& str ) const { return Compare( str.m_pBuffer, -1 ) < 0; }
	// $deBlob: FUNCTION 006ee530
	TBOOL operator>( const TCHAR* cstr ) const { return Compare( cstr, -1 ) > 0; }
	// $deBlob: FUNCTION 006ee510
	TBOOL operator>( const TString8& str ) const { return Compare( str.m_pBuffer, -1 ) > 0; }
	// $deBlob: FUNCTION 006ee570
	TBOOL operator<=( const TCHAR* cstr ) const { return Compare( cstr, -1 ) <= 0; }
	// $deBlob: FUNCTION 006ee550
	TBOOL operator<=( const TString8& str ) const { return Compare( str.m_pBuffer, -1 ) <= 0; }
	// $deBlob: FUNCTION 006ee5b0
	TBOOL operator>=( const TCHAR* cstr ) const { return Compare( cstr, -1 ) >= 0; }
	// $deBlob: FUNCTION 006ee590
	TBOOL operator>=( const TString8& str ) const { return Compare( str.m_pBuffer, -1 ) >= 0; }

	TString8& operator=( const TCHAR* cstr );

	TString8& operator=( const TWCHAR* wcstr );

	TString8& operator=( const TString8& str );

#ifdef TOSHI_STRING_MOVE
	TString8& operator=( TString8&& str )
	{
		FreeBuffer();
		m_pBuffer    = str.m_pBuffer;
		m_iExcessLen = str.m_iExcessLen;
		m_iStrLen    = str.m_iStrLen;
		m_pAllocator = str.m_pAllocator;

		str.Reset();
		return *this;
	}
#endif

	TString8& operator=( const TString16& str );

public:
	typedef T2Allocator* ( *func_DefaultAllocatorCB )();

	// Sets the callback giving the allocator of new strings (de Blob: AppStringAllocator)
	// NOTE: no assert in de Blob
	static void SetDefaultAllocatorCB( func_DefaultAllocatorCB a_pCallback )
	{
		sm_pDefaultAllocatorCB = a_pCallback;
	}

	static func_DefaultAllocatorCB GetDefaultAllocatorCallback() { return sm_pDefaultAllocatorCB; }

private:
	// $deBlob: FUNCTION 006ee950
	// $Barnyard: FUNCTION 006bd700
	void Reset()
	{
		m_pBuffer    = NullString;
		m_iStrLen    = 0;
		m_iExcessLen = 0;
	}

	T2Allocator* GetAllocator();

	static T2Allocator* GetDefaultAllocatorCB();

private:
	static func_DefaultAllocatorCB sm_pDefaultAllocatorCB;

private:
	TCHAR*       m_pBuffer;        // 0x0
	TINT32       m_iStrLen : 24;   // 0x4
	TUINT32      m_iExcessLen : 8; // 0x5
	T2Allocator* m_pAllocator;     // 0x8
};

// $deBlob: FUNCTION 006eec60
// $Barnyard: FUNCTION 006bd090
inline TBOOL operator==( const TCHAR* a_pcString, const TString8& a_rcString )
{
	return a_rcString.Compare( a_pcString, -1 ) == 0;
}

TString8 operator+( const TCHAR* a_pcString, const TString8& a_rcString );

TOSHI_NAMESPACE_END

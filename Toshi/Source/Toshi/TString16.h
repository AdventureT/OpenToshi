#pragma once
#include "TString.h"
#include "T2Allocator.h"

TOSHI_NAMESPACE_START

class TString8;

class TString16
{
public:
	TString16();
	TString16( T2Allocator* allocator );
#ifdef TOSHI_STRING_MOVE
	TString16( TString16&& src ) TNOEXCEPT;
#endif
	TString16( const TString16& src, T2Allocator* allocator = sm_pDefaultAllocatorCB() );
	TString16( const TWCHAR* src, T2Allocator* allocator = sm_pDefaultAllocatorCB() );
	TString16( const TString8& src, T2Allocator* allocator = sm_pDefaultAllocatorCB() );
	TString16( const TCHAR* src, T2Allocator* allocator = sm_pDefaultAllocatorCB() );
	TString16( TINT size, T2Allocator* allocator = sm_pDefaultAllocatorCB() );
	// $deBlob: FUNCTION 006f17d0
	~TString16() { FreeBuffer(); }

	void Copy( const TString16& src, TINT size = -1 );
	void Copy( const TWCHAR* src, TINT size = -1 );
	void Copy( const TString8& src, TINT size = -1 );
	void Copy( const TCHAR* src, TINT size = -1 );

	const TString16& Print() const;

	void FreeBuffer();

	// Returns TTRUE if allocated memory
	TBOOL AllocBuffer( TINT size, TBOOL freeMemory = TTRUE );

	TString16& Format( const TWCHAR* a_pcFormat, ... );
	TString16& VFormat( const TWCHAR* a_pcFormat, va_list a_vargs );

	void ForceSetData( TWCHAR* a_cString, TINT a_ilength );
	void UndoForceSetData() { Reset(); }

	TINT FindReverse( TWCHAR a_findChar, TINT pos = -1 ) const;

	void Truncate( TINT length );

	// Returns position of specified character
	TINT Find( TWCHAR character, TINT pos = 0 ) const;

	// Returns position of specified substring
	TINT Find( const TWCHAR* substr, TINT pos = 0 ) const;

	// Returns string starting from specified index
	const TWCHAR* GetString( TINT index ) const;
	const TWCHAR* GetString() const { return !IsIndexValid( 0 ) ? TNULL : m_pBuffer; }

	// Returns string starting from specified a_iIndex. Use only if you are sure about what you do!
	TWCHAR* GetStringUnsafe( TINT a_iIndex = 0 );

	TString16& Concat( const TString16& str, TINT size = -1 );
	TString16& Concat( const TWCHAR* src, TINT size = -1 );

	// Inserts a_iCount characters of a_wszString at a_iPos
	TString16& Insert( const TWCHAR* a_wszString, TINT a_iPos, TINT a_iCount = -1 );
	TString16& Insert( const TString16& a_rcString, TINT a_iPos, TINT a_iCount = -1 );

	// Removes a_iCount characters starting at a_iPos
	TString16& Delete( TINT a_iPos, TINT a_iCount );

	TString16& Reserve( TINT size );

	TINT Compare( const TWCHAR* a_wszString, TINT a_iLength = -1 ) const;
	TINT CompareNoCase( const TWCHAR* a_wszString, TINT a_iLength = -1 ) const;

	// Returns a substring contained in a [a_iFirst; a_iFirst + a_iCount] range
	TString16 Mid( TINT a_iFirst, TINT a_iCount ) const;

	// Returns a substring contained in a [a_iFrom; end of the string] range
	TString16 Right( TINT a_iFrom ) const;

	// $deBlob: FUNCTION 006f0ec0
	// $Barnyard: FUNCTION 006cc700
	TString16& MakeUpper()
	{
		if ( m_pBuffer ) _wcsupr( m_pBuffer );
		return *this;
	}

	// $deBlob: FUNCTION 006f0ee0
	// $Barnyard: FUNCTION 006cc720
	TString16& MakeLower()
	{
		if ( m_pBuffer ) _wcslwr( m_pBuffer );
		return *this;
	}

	TINT    Length() const { return m_iStrLen; }
	TUINT16 ExcessLength() const { return m_iExcessLen; }

	TBOOL IsAllLowerCase() const;
	TBOOL IsAllUpperCase() const;
	TBOOL IsAllAlphaNumeric() const;
	// $deBlob: FUNCTION 006f1580
	TBOOL IsIndexValid( TINT index ) const { return index <= Length() && index >= 0; }
	// $deBlob: FUNCTION 006f1190
	TBOOL IsEmpty() const { return m_iStrLen == 0; }
	// $deBlob: FUNCTION 006f15b0
	TBOOL IsUnicode() const { return TTRUE; }

public:
	TString16 operator+( const TWCHAR* a_pRHS ) const;
	TString16 operator+( const TString16& a_rcString ) const;

	// $deBlob: FUNCTION 006f11c0
	TString16& operator+=( TWCHAR const* a_wszStr )
	{
		Concat( a_wszStr, -1 );
		return *this;
	}

	// $deBlob: FUNCTION 006f11a0
	TString16& operator+=( const TString16& str )
	{
		Concat( str, -1 );
		return *this;
	}

	TWCHAR&       operator[]( TINT index ) { return m_pBuffer[ index ]; }
	const TWCHAR& operator[]( TINT index ) const { return *GetString( index ); }
	operator const TWCHAR*() const { return m_pBuffer; }

	TBOOL operator!() { return m_iStrLen == 0; }
	// $deBlob: FUNCTION 006f13f0
	TBOOL operator==( const TWCHAR* a_wszStr ) const { return Compare( a_wszStr, -1 ) == 0; }
	// $deBlob: FUNCTION 006f13d0
	TBOOL operator==( const TString16& str ) const { return Compare( str.m_pBuffer, -1 ) == 0; }
	// $deBlob: FUNCTION 006f1430
	TBOOL operator!=( const TWCHAR* a_wszStr ) const { return Compare( a_wszStr, -1 ) != 0; }
	// $deBlob: FUNCTION 006f1410
	TBOOL operator!=( const TString16& str ) const { return Compare( str.m_pBuffer, -1 ) != 0; }
	// NOTE: Barnyard's < and <= were inverted, de Blob's are the usual ones
	// $deBlob: FUNCTION 006f1470
	TBOOL operator<( const TWCHAR* a_wszStr ) const { return Compare( a_wszStr, -1 ) < 0; }
	// $deBlob: FUNCTION 006f1450
	TBOOL operator<( const TString16& str ) const { return Compare( str.m_pBuffer, -1 ) < 0; }
	// $deBlob: FUNCTION 006f14b0
	TBOOL operator>( const TWCHAR* a_wszStr ) const { return Compare( a_wszStr, -1 ) > 0; }
	// $deBlob: FUNCTION 006f1490
	TBOOL operator>( const TString16& str ) const { return Compare( str.m_pBuffer, -1 ) > 0; }
	// $deBlob: FUNCTION 006f14f0
	TBOOL operator<=( const TWCHAR* a_wszStr ) const { return Compare( a_wszStr, -1 ) <= 0; }
	// $deBlob: FUNCTION 006f14d0
	TBOOL operator<=( const TString16& str ) const { return Compare( str.m_pBuffer, -1 ) <= 0; }
	// $deBlob: FUNCTION 006f1530
	TBOOL operator>=( const TWCHAR* a_wszStr ) const { return Compare( a_wszStr, -1 ) >= 0; }
	// $deBlob: FUNCTION 006f1510
	TBOOL operator>=( const TString16& str ) const { return Compare( str.m_pBuffer, -1 ) >= 0; }

	TString16& operator=( const TWCHAR* a_wszStr );
	TString16& operator=( const TString8& a_rcStr );

	// Copies a C-string, widening each character
	TString16& operator=( const TCHAR* a_szStr );

	TString16& operator=( const TString16& str );

#ifdef TOSHI_STRING_MOVE
	TString16& operator=( TString16&& str )
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

public:
	typedef T2Allocator* ( *func_DefaultAllocatorCB )();

	// Sets the callback giving the allocator of new strings (de Blob: AppStringAllocator)
	// $deBlob: FUNCTION 00575040
	static void SetDefaultAllocatorCB( func_DefaultAllocatorCB a_pCallback )
	{
		TASSERT( TNULL != a_pCallback );
		sm_pDefaultAllocatorCB = a_pCallback;
	}

private:

	// $deBlob: FUNCTION 006f1630
	void Reset()
	{
		m_pBuffer    = NullWString;
		m_iStrLen    = 0;
		m_iExcessLen = 0;
	}

	T2Allocator* GetAllocator();

	void FreeMemory( TWCHAR* a_pMem );

	static T2Allocator* GetDefaultAllocatorCB();

private:
	static func_DefaultAllocatorCB sm_pDefaultAllocatorCB;

private:
	TWCHAR*      m_pBuffer;        // 0x0
	TINT32       m_iStrLen : 24;   // 0x4
	TUINT32      m_iExcessLen : 8; // 0x5
	T2Allocator* m_pAllocator;     // 0x8
};

TString16 operator+( const TWCHAR* a_pLHS, const TString16& a_rcString );

// NOTE: de Blob compares the right side with the left one here, so < and > are swapped (retail quirk, kept)
// $deBlob: FUNCTION 006f1930
inline TBOOL operator==( const TWCHAR* a_pcString, const TString16& a_rcString ) { return a_rcString.Compare( a_pcString, -1 ) == 0; }
// $deBlob: FUNCTION 006f1950
inline TBOOL operator!=( const TWCHAR* a_pcString, const TString16& a_rcString ) { return a_rcString.Compare( a_pcString, -1 ) != 0; }
// $deBlob: FUNCTION 006f1970
inline TBOOL operator<( const TWCHAR* a_pcString, const TString16& a_rcString ) { return a_rcString.Compare( a_pcString, -1 ) < 0; }
// $deBlob: FUNCTION 006f1990
inline TBOOL operator>( const TWCHAR* a_pcString, const TString16& a_rcString ) { return a_rcString.Compare( a_pcString, -1 ) > 0; }
// $deBlob: FUNCTION 006f19b0
inline TBOOL operator<=( const TWCHAR* a_pcString, const TString16& a_rcString ) { return a_rcString.Compare( a_pcString, -1 ) <= 0; }
// $deBlob: FUNCTION 006f19d0
inline TBOOL operator>=( const TWCHAR* a_pcString, const TString16& a_rcString ) { return a_rcString.Compare( a_pcString, -1 ) >= 0; }

TOSHI_NAMESPACE_END

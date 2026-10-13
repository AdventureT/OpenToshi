#pragma once
#include "T2String8.h"
#include "T2String16.h"

#include <cstdarg>

TOSHI_NAMESPACE_START

class TString8;
template <class T>
class T2DynamicStack;

class TStringManager
{
public:
	static void String8Tokenise( const TCHAR* a_szString, T2DynamicStack<TString8>& a_rOutTokens );

	// NOTE: retail returns its own scratch buffers (0x009b12d8 and 0x009b17c0)
	// $deBlob: FUNCTION 006eb740
	static TCHAR* GetTempString8() { return T2String8::ms_aScratchMem; }
	// $deBlob: FUNCTION 006eb750
	// $Barnyard: FUNCTION 006bb070
	static TINT String8Compare( const TCHAR* str1, const TCHAR* str2, TSIZE size = -1 ) { return T2String8::Compare( str1, str2, size ); }
	// $deBlob: FUNCTION 006eb760
	static TINT String8CompareNoCase( const TCHAR* str1, const TCHAR* str2, TSIZE size = -1 ) { return T2String8::CompareNoCase( str1, str2, size ); }
	// $deBlob: FUNCTION 006eb770
	static TCHAR* String8Copy( TCHAR* dst, const TCHAR* src, TSIZE size = -1 ) { return T2String8::Copy( dst, src, size ); }
	// $deBlob: FUNCTION 006eb780
	static TCHAR* String8CopySafe( TCHAR* dst, const TCHAR* src, TSIZE size = -1 ) { return T2String8::CopySafe( dst, src, size ); }
	// $deBlob: FUNCTION 006eb790
	static TCHAR* String8Move( TCHAR* dst, const TCHAR* src, TSIZE size = -1 ) { return T2String8::Move( dst, src, size ); }
	// $deBlob: FUNCTION 006eb7a0
	static TSIZE String8Length( const TCHAR* str ) { return T2String8::Length( str ); }
	// $deBlob: FUNCTION 006eb7b0
	static const TCHAR* String8FindChar( const TCHAR* str, TCHAR character ) { return T2String8::FindChar( str, character ); }
	// $deBlob: FUNCTION 006eb810
	static const TCHAR* String8FindString( const TCHAR* str, const TCHAR* substr ) { return T2String8::FindString( str, substr ); }
	// $deBlob: FUNCTION 006eb820
	static void String8IntToString( TINT value, TCHAR* dst, TINT radix ) { _itoa( value, dst, radix ); }
	// $deBlob: FUNCTION 006eb840
	static TUINT32 String8Hash( const TCHAR* str ) { return T2String8::Hash( str ); }
	// $deBlob: FUNCTION 006eb850
	// $Barnyard: FUNCTION 006bb270
	static void String8ToLowerCase( TCHAR* str ) { T2String8::ToLowerCase( str ); }
	// $deBlob: FUNCTION 006eb860
	static void String8ToUpperCase( TCHAR* str ) { T2String8::ToUpperCase( str ); }
	// $deBlob: FUNCTION 006eb870
	static TCHAR String8CharToLowerCase( TCHAR a_cChar ) { return T2String8::CharToLowerCase( a_cChar ); }
	// $deBlob: FUNCTION 006eb880
	static TCHAR String8CharToUpperCase( TCHAR a_cChar ) { return T2String8::CharToUpperCase( a_cChar ); }
	// $deBlob: FUNCTION 006eb890
	static TINT String8ToInt( const TCHAR* str ) { return T2String8::StringToInt( str ); }
	// $deBlob: FUNCTION 006eb8a0
	static TUINT String8ToUInt( const TCHAR* str ) { return T2String8::StringToUInt( str ); }
	// $deBlob: FUNCTION 006eb8b0
	// $Barnyard: FUNCTION 006bb2c0
	static TFLOAT String8ToFloat( const TCHAR* str ) { return T2String8::StringToFloat( str ); }
	// $deBlob: FUNCTION 006eb8c0
	static TBOOL String8ToBool( const TCHAR* str ) { return T2String8::StringToBool( str ); }
	// $deBlob: FUNCTION 006eb8d0
	static TBOOL String8IsNumeric( const TCHAR* str ) { return T2String8::IsNumeric( str ); }

	// $deBlob: FUNCTION 006eb7c0
	static TINT String8Format( TCHAR* str, const TCHAR* format, ... )
	{
		va_list args;

		va_start( args, format );
		TINT iResult = T2String8::FormatV( str, format, args );
		va_end( args );

		return iResult;
	}

	// $deBlob: FUNCTION 006eb7e0
	// $Barnyard: FUNCTION 006bb1a0
	static TINT String8Format( TCHAR* str, TINT size, const TCHAR* format, ... )
	{
		va_list args;

		va_start( args, format );
		TINT iResult = T2String8::FormatV( str, size, format, args );
		va_end( args );

		return iResult;
	}

	// $deBlob: FUNCTION 006eb800
	static TINT String8FormatV( TCHAR* str, TINT size, const TCHAR* format, va_list args ) { return T2String8::FormatV( str, size, format, args ); }

	// $deBlob: FUNCTION 006eba40
	static TWCHAR* GetTempString16() { return T2String16::ms_aScratchMem; }
	// $deBlob: FUNCTION 006eba50
	static TINT String16Compare( const TWCHAR* str1, const TWCHAR* str2, TSIZE size = -1 ) { return T2String16::Compare( str1, str2, size ); }
	// $deBlob: FUNCTION 006eba60
	static TINT String16CompareNoCase( const TWCHAR* str1, const TWCHAR* str2, TSIZE size = -1 ) { return T2String16::CompareNoCase( str1, str2, size ); }
	// $Barnyard: FUNCTION 006bb310
	static TWCHAR* String16Copy( TWCHAR* dst, const TWCHAR* src, TSIZE size = -1 );
	// $deBlob: FUNCTION 006ebaf0
	static TWCHAR* String16CopySafe( TWCHAR* dst, const TWCHAR* src, TSIZE size = -1 ) { return T2String16::CopySafe( dst, src, size ); }
	// $deBlob: FUNCTION 006ebb00
	static TWCHAR* String16Move( TWCHAR* dst, const TWCHAR* src, TSIZE size = -1 ) { return T2String16::Move( dst, src, size ); }
	// $deBlob: FUNCTION 006ebb10
	static TSIZE String16Length( const TWCHAR* str ) { return T2String16::Length( str ); }
	// $deBlob: FUNCTION 006ebb20
	static const TWCHAR* String16FindChar( const TWCHAR* str, TWCHAR character ) { return T2String16::FindChar( str, character ); }
	// $deBlob: FUNCTION 006ebb80
	static const TWCHAR* String16FindString( const TWCHAR* str, TWCHAR* substr ) { return T2String16::FindString( str, substr ); }
	// $deBlob: FUNCTION 006ebb90
	static TUINT32 String16Hash( const TWCHAR* str ) { return T2String16::Hash( str ); }
	// $deBlob: FUNCTION 006ebba0
	static TWCHAR String16CharToUpperCase( TWCHAR a_cChar ) { return T2String16::CharToUpperCase( a_cChar ); }
	// $deBlob: FUNCTION 006ebbb0
	static TWCHAR String16CharToLowerCase( TWCHAR a_cChar ) { return T2String16::CharToLowerCase( a_cChar ); }
	// $deBlob: FUNCTION 006ebbc0
	static TWCHAR* String16CreateCopy( const TWCHAR* str, TSIZE size = -1 ) { return T2String16::CreateCopy( str, size ); }

	// $deBlob: FUNCTION 006ebb30
	static TINT String16Format( TWCHAR* str, const TWCHAR* format, ... )
	{
		va_list args;

		va_start( args, format );
		TINT iResult = T2String16::FormatV( str, format, args );
		va_end( args );

		return iResult;
	}

	// $deBlob: FUNCTION 006ebb50
	// $Barnyard: FUNCTION 006bb5f0
	static TINT String16Format( TWCHAR* str, TINT size, const TWCHAR* format, ... )
	{
		va_list args;

		va_start( args, format );
		TINT iResult = T2String16::FormatV( str, size, format, args );
		va_end( args );

		return iResult;
	}

	// $deBlob: FUNCTION 006ebb70
	static TINT String16FormatV( TWCHAR* str, TINT size, const TWCHAR* format, va_list args ) { return T2String16::FormatV( str, size, format, args ); }

	static TWCHAR* StringCharToUnicode( TWCHAR* a_UnicodeString, const TCHAR* a_CharString, TUINT32 a_iLength );
	static TCHAR*  StringUnicodeToChar( TCHAR* a_CharString, const TWCHAR* const& a_UnicodeString, TUINT32 a_iLength );
};

TOSHI_NAMESPACE_END

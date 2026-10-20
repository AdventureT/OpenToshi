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

	// NOTE: retail returns its own scratch buffers
	static TCHAR* GetTempString8();
	static TINT String8Compare( const TCHAR* str1, const TCHAR* str2, TSIZE size = -1 );
	static TINT String8CompareNoCase( const TCHAR* str1, const TCHAR* str2, TSIZE size = -1 );
	static TCHAR* String8Copy( TCHAR* dst, const TCHAR* src, TSIZE size = -1 );
	static TCHAR* String8CopySafe( TCHAR* dst, const TCHAR* src, TSIZE size = -1 );
	static TCHAR* String8Move( TCHAR* dst, const TCHAR* src, TSIZE size = -1 );
	static TINT String8Length( const TCHAR* str );
	static const TCHAR* String8FindChar( const TCHAR* str, TCHAR character );
	static const TCHAR* String8FindString( const TCHAR* str, const TCHAR* substr );
	static void String8IntToString( TINT value, TCHAR* dst, TINT radix );
	static TUINT32 String8Hash( const TCHAR* str );
	static void String8ToLowerCase( TCHAR* str );
	static void String8ToUpperCase( TCHAR* str );
	static TCHAR String8CharToLowerCase( TCHAR a_cChar );
	static TCHAR String8CharToUpperCase( TCHAR a_cChar );
	static TINT String8ToInt( const TCHAR* str );
	static TUINT String8ToUInt( const TCHAR* str );
	static TFLOAT String8ToFloat( const TCHAR* str );
	static TBOOL String8ToBool( const TCHAR* str );
	static TBOOL String8IsNumeric( const TCHAR* str );

	static TINT String8Format( TCHAR* str, const TCHAR* format, ... );

	static TINT String8Format( TCHAR* str, TINT size, const TCHAR* format, ... );

	static TINT String8FormatV( TCHAR* str, TINT size, const TCHAR* format, va_list args );

	static TWCHAR* GetTempString16();
	static TINT String16Compare( const TWCHAR* str1, const TWCHAR* str2, TSIZE size = -1 );
	static TINT String16CompareNoCase( const TWCHAR* str1, const TWCHAR* str2, TSIZE size = -1 );
	// $Barnyard: FUNCTION 006bb310
	static TWCHAR* String16Copy( TWCHAR* dst, const TWCHAR* src, TSIZE size = -1 );
	static TWCHAR* String16CopySafe( TWCHAR* dst, const TWCHAR* src, TSIZE size = -1 );
	static TWCHAR* String16Move( TWCHAR* dst, const TWCHAR* src, TSIZE size = -1 );
	static TINT String16Length( const TWCHAR* str );
	static const TWCHAR* String16FindChar( const TWCHAR* str, TWCHAR character );
	static const TWCHAR* String16FindString( const TWCHAR* str, TWCHAR* substr );
	static TUINT32 String16Hash( const TWCHAR* str );
	static TWCHAR String16CharToUpperCase( TWCHAR a_cChar );
	static TWCHAR String16CharToLowerCase( TWCHAR a_cChar );
	static TWCHAR* String16CreateCopy( const TWCHAR* str, TSIZE size = -1 );

	static TINT String16Format( TWCHAR* str, const TWCHAR* format, ... );

	static TINT String16Format( TWCHAR* str, TINT size, const TWCHAR* format, ... );

	static TINT String16FormatV( TWCHAR* str, TINT size, const TWCHAR* format, va_list args );

	static TWCHAR* StringCharToUnicode( TWCHAR* a_UnicodeString, const TCHAR* a_CharString, TUINT32 a_iLength );
	static TCHAR*  StringUnicodeToChar( TCHAR* a_CharString, const TWCHAR* a_UnicodeString, TUINT32 a_iLength );
};

TOSHI_NAMESPACE_END

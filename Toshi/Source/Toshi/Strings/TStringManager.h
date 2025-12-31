#pragma once
#include "Toshi2/T2String8.h"
#include "Toshi2/T2String16.h"

TOSHI_NAMESPACE_START

class TStringManager
{
public:
	// $deBlob: FUNCTION 006eb740
	static TCHAR*       GetTempString8() { return T2String8::ms_aScratchMem; }
	// $deBlob: FUNCTION 006eb750
	static TINT         String8Compare(const TCHAR* str1, const TCHAR* str2, size_t size = -1) { return T2String8::Compare(str1, str2, size); }
	// $deBlob: FUNCTION 006eb760
	static TINT         String8CompareNoCase(const TCHAR* str1, const TCHAR* str2, size_t size = -1) { return T2String8::CompareNoCase(str1, str2, size); }
	// $deBlob: FUNCTION 006eb770
	static TCHAR*       String8Copy(TCHAR* dst, const TCHAR* src, size_t size = -1) { return T2String8::Copy(dst, src, size); }
	// $deBlob: FUNCTION 006eb780
	static TCHAR*       String8CopySafe(TCHAR* dst, const TCHAR* src, size_t size = -1) { return T2String8::CopySafe(dst, src, size); }
	static const TCHAR* String8FindChar(const TCHAR* str, TCHAR character) { return T2String8::FindChar(str, character); }
	static const TCHAR* String8FindString(const TCHAR* str, TCHAR* substr) { return T2String8::FindString(str, substr); }
	// $deBlob: FUNCTION 006eb7a0
	static size_t       String8Length(const TCHAR* str) { return T2String8::Length(str); }
	static void         String8ToLowerCase(TCHAR* str) { T2String8::ToLowerCase(str); }

	// $deBlob: FUNCTION 006eb7e0
	static void String8Format(TCHAR* str, TINT size, const TCHAR* format, ...)
	{
		va_list args;

		va_start(args, format);

		T2String8::FormatV(str, size, format, args);

		va_end(args);
	}

	static TWCHAR*       GetTempString16() { return T2String16::ms_aScratchMem; }
	// $deBlob: FUNCTION 006eba50
	static TINT          String16Compare(const TWCHAR* str1, const TWCHAR* str2, size_t size = -1) { return T2String16::Compare(str1, str2, size); }
	static TINT          String16CompareNoCase(const TWCHAR* str1, const TWCHAR* str2, size_t size = -1) { return T2String16::CompareNoCase(str1, str2, size); }
	// $deBlob: FUNCTION 006eba70
	static TWCHAR*       String16Copy(TWCHAR* dst, const TWCHAR* src, size_t size = -1) { return T2String16::Copy(dst, src, size); }
	static TWCHAR*       String16CopySafe(TWCHAR* dst, const TWCHAR* src, size_t size = -1) { return T2String16::CopySafe(dst, src, size); }
	static const TWCHAR* String16FindChar(const TWCHAR* str, TWCHAR character) { return T2String16::FindChar(str, character); }
	static const TWCHAR* String16FindString(const TWCHAR* str, TWCHAR* substr) { return T2String16::FindString(str, substr); }
	// $deBlob: FUNCTION 006ebb10
	static size_t        String16Length(const TWCHAR* str) { return T2String16::Length(str); }

	static TWCHAR* StringCharToUnicode(TWCHAR* a_UnicodeString, const TCHAR* a_CharString, TUINT32 a_iLength);
	static TCHAR*  StringUnicodeToChar(TCHAR* a_CharString, const TWCHAR* const& a_UnicodeString, TUINT32 a_iLength);
};

TOSHI_NAMESPACE_END

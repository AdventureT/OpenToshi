#pragma once

#undef TCHAR

using TFLOAT   = float;
using TBOOL    = bool;
using TSHORT   = short;
using TUSHORT  = unsigned short;
using TINT     = int;
using TUINT    = unsigned int;
using TINT32   = int32_t;
using TUINT32  = uint32_t;
using TINT16   = int16_t;
using TUINT16  = uint16_t;
using TINT8    = int8_t;
using TUINT8   = uint8_t;
using TUINTPTR = uintptr_t;
using TSIZE    = size_t;
using TCHAR    = char;
using TWCHAR   = wchar_t;

union TVersion
{
	TUINT32 Value;

	TVersion() = default;
	constexpr TVersion(TUINT32 a_uiVersion)
	    : Value(a_uiVersion) {}

	struct
	{
		TUINT16 Minor;
		TUINT16 Major;
	} Parts;

	void operator=(TUINT32 a_uiVersion) { Value = a_uiVersion; }

	operator TUINT32&() { return Value; }
};

#define TVERSION(VER_MAJOR, VER_MINOR) ((VER_MAJOR << 16U) | VER_MINOR)

#define TNULL  nullptr
#define TFALSE false
#define TTRUE  true

#define _BYTE  TUINT8
#define _WORD  TUINT16
#define _DWORD TUINT32
#define _QWORD uint64_t

#define LOWBYTE(x)   (*((_BYTE*)&(x)))  // low byte
#define LOWWORD(x)   (*((_WORD*)&(x)))  // low word
#define LOWDWORD(x)  (*((_DWORD*)&(x))) // low dword
#define HIGHBYTE(x)  (*((_BYTE*)&(x) + 1))
#define HIGHWORD(x)  (*((_WORD*)&(x) + 1))
#define HIGHDWORD(x) (*((_DWORD*)&(x) + 1))
#define BYTEn(x, n)  (*((_BYTE*)&(x) + n))
#define WORDn(x, n)  (*((_WORD*)&(x) + n))

#define BYTE1(x)  BYTEn(x, 1)
#define BYTE2(x)  BYTEn(x, 2)
#define BYTE3(x)  BYTEn(x, 3)
#define BYTE4(x)  BYTEn(x, 4)
#define BYTE5(x)  BYTEn(x, 5)
#define BYTE6(x)  BYTEn(x, 6)
#define BYTE7(x)  BYTEn(x, 7)
#define BYTE8(x)  BYTEn(x, 8)
#define BYTE9(x)  BYTEn(x, 9)
#define BYTE10(x) BYTEn(x, 10)
#define BYTE11(x) BYTEn(x, 11)
#define BYTE12(x) BYTEn(x, 12)
#define BYTE13(x) BYTEn(x, 13)
#define BYTE14(x) BYTEn(x, 14)
#define BYTE15(x) BYTEn(x, 15)

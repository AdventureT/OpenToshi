#pragma once
#include "Toshi/Xui/TXUIElement.h"
#include "Toshi/Core/Endianness.h"

TOSHI_NAMESPACE_START

class XURReader
{
public:
	XURReader(TUINT8*& buffer)
	    : m_Buffer(buffer), m_ReadFlags(0) {}

	__forceinline void SetPos(TUINT8* buffer) { m_Buffer = buffer; }

	__forceinline void SeekFromCur(TINT offset) { m_Buffer += offset; }

	__forceinline TUINT8* GetPos() { return m_Buffer; }

	__forceinline void SetReadFlags(TUINT32 readFlags) { m_ReadFlags = readFlags; }

	__forceinline TUINT32 GetReadFlags() { return m_ReadFlags; }

	template <TUINT32 numOfProps> __forceinline TUINT32 ReadPropsInfo()
	{
		if constexpr (numOfProps > 8)
		{
			m_ReadFlags = ReadUInt16();
		}
		else
		{
			m_ReadFlags = ReadUInt8();
		}

		return m_ReadFlags;
	}

	__forceinline TUINT32 ReadUInt32LE()
	{
		TUINT32 result = PARSEDWORD(m_Buffer);
		m_Buffer += sizeof(TUINT32);
		return result;
	}

	__forceinline TUINT16 ReadUInt16LE()
	{
		TUINT16 result = PARSEWORD(m_Buffer);
		m_Buffer += sizeof(TUINT16);
		return result;
	}

	__forceinline TUINT32 ReadUInt32()
	{
		TUINT32 result = PARSEDWORD_BIG(m_Buffer);
		m_Buffer += sizeof(TUINT32);
		return result;
	}

	__forceinline TUINT16 ReadUInt16()
	{
		TUINT16 result = PARSEWORD_BIG(m_Buffer);
		m_Buffer += sizeof(TUINT16);
		return result;
	}

	__forceinline TUINT8 ReadUInt8()
	{
		TUINT8 result = *m_Buffer;
		m_Buffer += 1;
		return result;
	}

	__forceinline TFLOAT ReadFloat()
	{
		TFLOAT result = PARSEFLOAT_BIG(m_Buffer);
		m_Buffer += sizeof(TFLOAT);
		return result;
	}

	__forceinline TUINT8 ReadUInt8From32()
	{
		TASSERT(PARSEDWORD_BIG(m_Buffer) < (1 << 8));

		TUINT8 result = *(m_Buffer + 3);
		m_Buffer += sizeof(TUINT32);
		return result;
	}

	__forceinline TUINT16 ReadUInt16From32()
	{
		TASSERT(PARSEDWORD_BIG(m_Buffer) < (1 << 16));
		TUINT16 result = PARSEWORD_BIG(m_Buffer + 2);

		m_Buffer += sizeof(TUINT32);
		return result;
	}

	__forceinline TINT32 ReadInt32() { return ReadUInt32(); }

	__forceinline TINT16 ReadInt16() { return ReadUInt16(); }

	__forceinline TUINT32 ReadPackedUInt32()
	{
		TUINT32 result;
		TUINT32 firstByte = ReadUInt8();

		if (firstByte != 0xFF)
		{
			if (firstByte < 0xF0)
			{
				result = firstByte;
			}
			else
			{
				TUINT32 secondByte = ReadUInt8();
				TUINT32 highPart   = (firstByte << 8) & 0x0F00;
				result             = highPart | secondByte;
			}
		}
		else
		{
			result = ReadUInt32();
		}

		return result;
	}

	__forceinline XUIEPTBool ReadEPTBool() { return ReadUInt8(); }

	__forceinline XUIEPTUInt8 ReadEPTUInt8() { return ReadUInt8(); }

	__forceinline XUIEPTInteger ReadEPTInteger() { return ReadInt32(); }

	__forceinline XUIEPTUnsigned ReadEPTUnsigned() { return ReadUInt32(); }

	__forceinline XUIEPTFloat ReadEPTFloat() { return ReadFloat(); }

	__forceinline XUIEPTString ReadEPTString() { return ReadUInt16(); }

	__forceinline XUIEPTShort32 ReadEPTShort32() { return ReadUInt16From32(); }

	__forceinline XUIEPTUShort32 ReadEPTUShort32() { return ReadUInt16From32(); }

	__forceinline XUIEPTUColor ReadEPTColor() { return ReadUInt32(); }

	__forceinline XUIEPTVector ReadEPTVector() { return ReadEPTUShort32(); }

	__forceinline XUIEPTQuaternion ReadEPTQuaternion() { return ReadEPTUShort32(); }

	__forceinline TBOOL ShouldReadThisProp(TINT propId) { return (m_ReadFlags & (1 << propId)); }

	template <TINT EPT, typename T> __forceinline TBOOL ReadProperty(TINT propId, T& outValue)
	{
		if (ShouldReadThisProp(propId))
		{
			if constexpr (EPT == XUI_EPT_BOOL)
			{
				outValue = ReadEPTBool();
			}
			else if constexpr (EPT == XUI_EPT_INTEGER)
			{
				outValue = ReadEPTInteger();
			}
			else if constexpr (EPT == XUI_EPT_UNSIGNED)
			{
				outValue = ReadEPTUnsigned();
			}
			else if constexpr (EPT == XUI_EPT_FLOAT)
			{
				outValue = ReadEPTFloat();
			}
			else if constexpr (EPT == XUI_EPT_STRING)
			{
				outValue = ReadEPTString();
			}
			else if constexpr (EPT == XUI_EPT_COLOR)
			{
				outValue = ReadEPTColor();
			}
			else if constexpr (EPT == XUI_EPT_VECTOR)
			{
				outValue = ReadEPTVector();
			}
			else if constexpr (EPT == XUI_EPT_QUATERNION)
			{
				outValue = ReadEPTQuaternion();
			}
			else if constexpr (EPT == XUI_EPT_OBJECT)
			{
				TASSERT(TFALSE && "Not implemented");
			}
			else if constexpr (EPT == XUI_EPT_CUSTOM)
			{
				outValue = ReadEPTUnsigned();
			}
			else if constexpr (EPT == XUI_EPT_SHORT32)
			{
				outValue = ReadEPTShort32();
			}
			else if constexpr (EPT == XUI_EPT_USHORT32)
			{
				outValue = ReadEPTUShort32();
			}

			return TTRUE;
		}

		return TFALSE;
	}

private:
	TUINT8*& m_Buffer;
	TUINT32  m_ReadFlags;
};

TOSHI_NAMESPACE_END

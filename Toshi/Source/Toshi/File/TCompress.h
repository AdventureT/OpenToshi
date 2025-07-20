#pragma once

TOSHI_NAMESPACE_START

enum TCOMPRESS_ERROR
{
	TCOMPRESS_ERROR_OK               = 0,
	TCOMPRESS_ERROR_WRONG_MAGIC      = -1,
	TCOMPRESS_ERROR_WRONG_VERSION    = -2,
	TCOMPRESS_ERROR_WRONG_SIZE       = -3,
	TCOMPRESS_ERROR_WRONG_HEADERSIZE = -4
};

enum BTECSizeFlag
{
	BTECSizeFlag_BigSize  = BITFIELD(6),
	BTECSizeFlag_NoOffset = BITFIELD(7),
	BTECSizeFlag_SizeMask = BTECSizeFlag_BigSize - 1,
};

enum BTECOffsetFlag
{
	BTECOffsetFlag_BigOffset  = BITFIELD(7),
	BTECOffsetFlag_OffsetMask = BTECOffsetFlag_BigOffset - 1,
};

class TCompress
{
public:
	struct Header
	{
		TUINT32  Magic;
		TVersion Version;
		TUINT32  CompressedSize;
		TUINT32  Size;
		TUINT32  XorValue;
	};

public:
	// Size of header that is common for any BTEC version
	static constexpr TUINT32 HEADER_SIZE_12 = sizeof(Header) - sizeof(Header::XorValue);

	// Size of header that is common for BTEC 1.3
	static constexpr TUINT32 HEADER_SIZE_13 = sizeof(Header);

	// Custom variable used to help the compressor/decompressor decide which byte order it should use
	inline static TBOOL ms_bIsBigEndian = TFALSE;
};

TOSHI_NAMESPACE_END

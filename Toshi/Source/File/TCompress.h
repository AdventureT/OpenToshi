#pragma once
#include "Toshi/TVersion.h"
#include "File/TFile.h"

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
	BTECSizeFlag_BigSize  = BITFLAG( 6 ),
	BTECSizeFlag_NoOffset = BITFLAG( 7 ),
	BTECSizeFlag_SizeMask = BTECSizeFlag_BigSize - 1,
};

enum BTECOffsetFlag
{
	BTECOffsetFlag_BigOffset  = BITFLAG( 7 ),
	BTECOffsetFlag_OffsetMask = BTECOffsetFlag_BigOffset - 1,
};

class TCompress
{
public:
	//-----------------------------------------------------------------------------
	// General Information
	//-----------------------------------------------------------------------------

	// NOTE: de Blob only knows BTEC 1.2 (16 bytes header, Barnyard's 1.3 adds a XOR value)
	struct Header
	{
		TUINT32  Magic;          // 0x0
		TVersion Version;        // 0x4
		TUINT32  CompressedSize; // 0x8
		TUINT32  size;           // 0xC
	};

	// Size of header that is common for any BTEC version
	static TCONSTEXPR_VAR TUINT32 HEADER_SIZE_12 = sizeof( Header );

	// Custom variable used to help the compressor/decompressor decide which byte order it should use
	// NOTE: not in de Blob (its decompressor never swaps), kept for the PTRB tool plugin
	static TBOOL ms_bIsBigEndian;

public:
	//-----------------------------------------------------------------------------
	// Compressor
	//-----------------------------------------------------------------------------

	static TCONSTEXPR_VAR TINT maxlength = 0x4000;
	static TINT           usemaxoffset;

public:
	static size_t Compress( TFile* file, TBYTE* data, TUINT32 size, TUINT32 unused, TBOOL isBigEndian );

private:
	static TINT WriteOffset( TUINT32 length, TINT offset, TBYTE*& data, TFile* file );
	static TINT Write( TUINT32 length, TBYTE*& data, TFile* file );

public:
	//-----------------------------------------------------------------------------
	// Decompressor
	//-----------------------------------------------------------------------------

	static TINT   Decompress( TFile* file, TCompress::Header* header, TBYTE* buffer, TUINT32 bufferSize );
	static TINT GetHeader( TFile* file, TCompress::Header& btecHeader );
	static TINT   GetCommand( TFile* file, TBOOL& hasOffset, TUINT32& size, TINT& offset );
};

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TCompress.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// Copies a_iSize bytes that were already unpacked a_iOffset bytes back
// NOTE: not referenced in retail (Decompress has its own copy of it)
// $deBlob: FUNCTION 0068abc0
static TINT CopyBack( TBYTE*& a_rpDst, TINT a_iSize, TINT a_iOffset )
{
	TBYTE* pSrc = a_rpDst - a_iOffset;

	for ( ; a_iSize > 0; a_iSize-- )
		*( a_rpDst++ ) = *( pSrc++ );

	return 0;
}

// Reads a_uiSize bytes of raw data
// NOTE: not referenced in retail (Decompress has its own copy of it)
// $deBlob: FUNCTION 0068abf0
static void ReadRaw( TFile* a_pFile, TBYTE*& a_rpDst, TSIZE a_uiSize )
{
	a_rpDst += a_pFile->Read( a_rpDst, a_uiSize );
}

// $deBlob: FUNCTION 0068aa10
// $Barnyard: FUNCTION 006dc4f0
TINT TCompress::Decompress( TFile* file, TCompress::Header* header, TBYTE* buffer, TUINT32 bufferSize )
{
	TPROFILER_SCOPE();

	if ( header->Magic != TFourCC( "BTEC" ) )
		return TCOMPRESS_ERROR_WRONG_MAGIC;
	if ( header->Version != TVERSION( 1, 2 ) )
		return TCOMPRESS_ERROR_WRONG_VERSION;
	if ( header->size > bufferSize )
		return TCOMPRESS_ERROR_WRONG_SIZE;

	TBYTE* pBufferPos     = buffer;
	TINT   compressedLeft = header->CompressedSize;

	while ( compressedLeft != 0 )
	{
		TUINT32 chunkSize;
		TBOOL   noOffset;
		TINT    offset;

		compressedLeft -= GetCommand( file, noOffset, chunkSize, offset );

		if ( !noOffset )
		{
			// The data is already unpacked so just copy it
			TBYTE* unpackedData = pBufferPos - offset;

			for ( TINT i = chunkSize; i > 0; i-- )
				*( pBufferPos++ ) = *( unpackedData++ );
		}
		else
		{
			// The data wasn't previously unpacked, read it from file
			TINT iRead = file->Read( pBufferPos, chunkSize );
			pBufferPos += iRead;
			compressedLeft -= iRead;
		}
	}

	TINT sizedecompressed = TINT( pBufferPos - buffer );
	TASSERT( sizedecompressed == header->size );

	return sizedecompressed;
}

// $deBlob: FUNCTION 0068a9b0
// $Barnyard: FUNCTION 006dc220
int8_t TCompress::GetHeader( TFile* file, TCompress::Header& btecHeader )
{
	TSIZE uiSavedPos = file->Tell();

	if ( file->Read( &btecHeader, TCompress::HEADER_SIZE_12 ) != TCompress::HEADER_SIZE_12 )
	{
		file->Seek( uiSavedPos, TSEEK_SET );
		return TCOMPRESS_ERROR_WRONG_HEADERSIZE;
	}

	if ( btecHeader.Magic != TFourCC( "BTEC" ) )
	{
		file->Seek( uiSavedPos, TSEEK_SET );
		return TCOMPRESS_ERROR_WRONG_MAGIC;
	}

	return TCOMPRESS_ERROR_OK;
}

// $deBlob: FUNCTION 0068aaf0
// $Barnyard: FUNCTION 006dc340
TINT TCompress::GetCommand( TFile* file, TBOOL& noOffset, TUINT32& size, TINT& offset )
{
	TBYTE aBytes[ 2 ];

	TINT read_count = TINT( file->Read( &aBytes[ 0 ], 1 ) );
	noOffset        = ( aBytes[ 0 ] & BTECSizeFlag_NoOffset ) != 0;

	if ( aBytes[ 0 ] & BTECSizeFlag_BigSize )
	{
		// now the size is 14 bits long
		read_count += TINT( file->Read( &aBytes[ 1 ], 1 ) );
		size = ( ( aBytes[ 0 ] & BTECSizeFlag_SizeMask ) << 8 ) | aBytes[ 1 ];
	}
	else
	{
		// by default size is 6 bits long
		size = aBytes[ 0 ] & BTECSizeFlag_SizeMask;
	}

	size += 1;

	if ( noOffset )
	{
		offset = 0;
		return read_count;
	}

	// by default offset is 7 bits long
	read_count += TINT( file->Read( &aBytes[ 0 ], 1 ) );

	if ( aBytes[ 0 ] & BTECOffsetFlag_BigOffset )
	{
		// now the offset is 15 bits long
		read_count += TINT( file->Read( &aBytes[ 1 ], 1 ) );
		offset = ( ( ( aBytes[ 0 ] & BTECOffsetFlag_OffsetMask ) << 8 ) | aBytes[ 1 ] ) + 1;
	}
	else
	{
		offset = ( aBytes[ 0 ] & BTECOffsetFlag_OffsetMask ) + 1;
	}

	return read_count;
}

TOSHI_NAMESPACE_END

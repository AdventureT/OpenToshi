/**
 * @file TTRB.h
 * TRB (Toshi Relocatable Binary) resource system for the Toshi engine
 * 
 * This file provides TRB resource functionality including:
 * - Resource file loading and management
 * - Resource data access and manipulation
 * - Resource compression and decompression
 * - Resource caching and optimization
 */

#pragma once
#ifndef __TOSHI_TTRB_H__
#  define __TOSHI_TTRB_H__
#endif

#include "Toshi/TVersion.h"

#include "File/TFile.h"
#include "File/TCompress.h"

#include "Math/TVector3.h"
#include "Math/TVector4.h"
#include "Math/TQuaternion.h"

#undef ERROR

TOSHI_NAMESPACE_START

class TFORM
{
public:
	TUINT32 Unk;
};

class TTSFI;
class TTSFO;

class TTSF
{
public:
	static TCONSTEXPR_VAR TUINT32 IDMAGICB = 0x42465354; // TFourCC( "TSFB" )
	static TCONSTEXPR_VAR TUINT32 IDMAGICL = 0x4c465354; // TFourCC( "TSFL" )

	struct Header
	{
		TUINT32 Magic;
		TUINT32 FileSize;
	};

	struct FileInfo
	{
		TUINT32 FileStartOffset; // offset to TRBF
		TUINT32 FileSize;        // just the size
	};

	struct Hunk
	{
		TUINT32 Name;
		TUINT32 Size;
	};

	// Called for every hunk (that isn't a FORM) by TTSFI::ProcessForm_Parse, returns TFALSE to stop
	using t_HunkCallback = TBOOL( __stdcall* )( TTSFI* a_pTTSFI, TUINT32 a_uiHunkName, void* a_pUserData );

public:
	TTSF()
	    : m_pFile( TNULL ) {}

	~TTSF() = default;

	// Allocate a reader/writer into *a_ppOut, return 1 if the allocation failed (else 0)
	static TUINT32 CreateTTSFI( TTSFI** a_ppOut );
	static TUINT32 CreateTTSFO( TTSFO** a_ppOut );

	// $deBlob: FUNCTION 006873f0
	Endianess GetEndianess() const { return m_Endianess; }

	// A 32 bit value in the file's byte order (swapped unless little endian)
	// $deBlob: FUNCTION 00689180
	// $deBlob: FUNCTION 006891d0
	// $deBlob: FUNCTION 00689220
	// $deBlob: FUNCTION 00689270
	TUINT32 ConvertEndian32( TUINT32 a_uiValue ) const
	{
		if ( m_Endianess != Endianess_Little )
			a_uiValue = SWAP32( a_uiValue );

		return a_uiValue;
	}

protected:
	Endianess m_Endianess; // 0x0
	TFile*    m_pFile;     // 0x4
};

// NOTE: de Blob: 0x124 bytes
class TTSFI : public TTSF
{
public:
	friend class TTRB;

public:
	TTSFI();
	~TTSFI();

	TUINT32 ReadAlignmentPad();

	TUINT32 Open( TFile* a_pFile );
	TUINT32 Open( const TCHAR* a_szFilePath );

	TUINT32 PushForm();
	TUINT32 PopForm();

	void ReadRaw( void* dst, TUINT32 size );

	template <class T> void Read( T* a_pDst ) { m_iReadPos += m_pFile->Read( a_pDst, sizeof( T ) ); }
	template <class T> void Read( T* a_pDst, TUINT a_uiCount ) { m_iReadPos += m_pFile->Read( a_pDst, a_uiCount * sizeof( T ) ); }

	TINT ReadHunk();
	TINT SkipHunk();
	TINT ReadFORM( TFORM* section );
	TINT ReadHunkData( void* dest );

	void Close( TBOOL free = TTRUE );

	void ReadCompressed( void* buffer, TUINT32 size );
	void CompressSection( TFile* file, TBYTE* data, TUINT32 unk2, TUINT32 unk3 ) { TCompress::Compress( file, data, unk2, unk3, m_Endianess ); }

	TSIZE Tell();
	void  Seek( TINT a_iOffset );

	// Walks every hunk of the current form (recursing into FORMs), calling a_fnCallback for each of them
	TBOOL ProcessForm_Parse( TFORM* a_pForm, t_HunkCallback a_fnCallback, void* a_pUserData );

	// Same as ProcessForm_Parse but checks that the file has just been opened
	TINT Parse( t_HunkCallback a_fnCallback, void* a_pUserData );

	// Opens a_szFilePath and walks its hunks
	static TUINT32 Read( const TCHAR* a_szFilePath, TUINT32 a_uiMagic, t_HunkCallback a_fnCallback, void* a_pUserData );

	TBOOL   ReadBool();
	TINT32  ReadInt32();
	TUINT32 ReadUInt32();
	TINT16  ReadInt16();
	TUINT16 ReadUInt16();
	TINT8   ReadInt8();
	TUINT8  ReadUInt8();
	TFLOAT  ReadFloat();

	TVector3&    ReadVector3( TVector3& a_rOutVec );
	TVector4&    ReadVector4( TVector4& a_rOutVec );
	TQuaternion& ReadQuaternion( TQuaternion& a_rOutQuat );

	void ReadBool( TBOOL* a_pOut );
	void ReadInt32( TINT32* a_pOut );
	void ReadUInt32( TUINT32* a_pOut );
	void ReadInt16( TINT16* a_pOut );
	void ReadUInt16( TUINT16* a_pOut );
	void ReadInt8( TINT8* a_pOut );
	void ReadUInt8( TUINT8* a_pOut );
	void ReadFloat( TFLOAT* a_pOut );

	TINT  ReadString( TCHAR* a_pszOut, TINT16 a_iMaxLength );
	void  ReadString( TString8& a_rOutString );
	void  ReadVector3( TVector3* a_pOut );
	void  ReadVector4( TVector4* a_pOut );
	void  ReadQuaternion( TQuaternion* a_pOut );
	TBOOL Tell( TUINT32* a_pOutPos );
	TBOOL Align( TINT a_iUnused = 0 );

	// $deBlob: FUNCTION 006873c0
	const Hunk& GetCurrentHunk() const { return m_CurrentHunk; }

	// $deBlob: FUNCTION 006873e0
	TUINT32* GetMagic() { return &m_Magic; }

private:
	TUINT32  m_FileInfoCount;  // 0x8
	FileInfo m_FileInfo[ 32 ]; // 0xC
	Header   m_Header;         // 0x10C
	TUINT32  m_Magic;          // 0x114
	Hunk     m_CurrentHunk;    // 0x118
	TUINT32  m_iReadPos;       // 0x120
};

// NOTE: de Blob: 0x8C bytes
class TTSFO : public TTSF
{
public:
	typedef TUINT8 ERROR;
	enum ERROR_ : ERROR
	{
		ERROR_OK,
		ERROR_UNKNOWN,
		ERROR_FILE
	};

	struct HunkMark
	{
		TUINT32 Name;
		TUINT32 Pos;
	};

public:
	TTSFO();
	~TTSFO();

	/**
		* Aligns current position to 4.
		*
		* @return number of written bytes
		*/
	TUINT32 WriteAlignmentPad();

	/**
		* Creates file for writing.
		*
		* @param filepath - path to the file
		* @param magic - magic value which is placed after the first hunk
		* @return TTRB::ERROR_OK if successful
		*/
	TTSFO::ERROR Create( const TCHAR* filepath, TUINT32 magic, Endianess endianess = Endianess_Little );
	TTSFO::ERROR Create( const TCHAR* filepath, const TCHAR* magic = "TRBF", Endianess endianess = Endianess_Little ) { return Create( filepath, TFourCC( magic ), endianess ); }

	/**
		* Closes the file.
		*
		*/
	void Close();

	/**
		* Begin a new form and saves it's info
		*
		* @param name - 4 bytes long value which will be written after the hunk
		* @return number of written bytes
		*/
	TSIZE BeginForm( TUINT32 name );
	TSIZE BeginForm( const TCHAR* name ) { return BeginForm( TFourCC( name ) ); }

	/**
		* Ends the current form if it exists.
		*
		* @return size of the form
		*/
	TSIZE EndForm();

	/**
		* Opens new hunk.
		*
		* @param hunkMark - pointer to TTSFO::HunkMark which has to be opened
		* @param hunkName - 4 bytes long name of the hunk
		* @return always TTRUE
		*/
	TBOOL OpenHunk( HunkMark* hunkMark, TUINT32 hunkName );
	TBOOL OpenHunk( HunkMark* hunkMark, const TCHAR* hunkName ) { return OpenHunk( hunkMark, TFourCC( hunkName ) ); }

	/**
		* Closes the hunk.
		*
		* @param hunkMark - pointer to TTSFO::HunkMark which has to be closed
		* @return always TTRUE
		*/
	TBOOL CloseHunk( HunkMark* hunkMark );

	/**
		* Writes a hunk of data.
		*
		* @param hunkName - magic value that is used to identificate hunk (use TMAKEFOUR to get it)
		* @param buffer - pointer to the buffer of data (can be TNULL)
		* @param bufferSize - size of the buffer (can be 0)
		* @return number of written bytes
		*/
	TUINT32 WriteHunk( TUINT32 hunkName, void* buffer = TNULL, TSIZE bufferSize = 0 );

	template <class T>
	TUINT32 Write( const T& value )
	{
		return m_pFile->Write( &value, sizeof( T ) );
	}

	// $deBlob: FUNCTION 00688ed0
	TUINT32 WriteRaw( const void* buffer, TSIZE size )
	{
		return m_pFile->Write( buffer, size );
	}

	// $deBlob: FUNCTION 00688ee0
	TUINT32 WriteCompressed( const void* buffer, TSIZE size, TUINT32 a_uiUnknown = 0 )
	{
		return TCompress::Compress( m_pFile, (TBYTE*)buffer, size, a_uiUnknown, m_Endianess != Endianess_Little );
	}

	// $deBlob: FUNCTION 00688f10
	void WriteBool( TBOOL value ) { m_pFile->Write( &value, 1 ); }
	// $deBlob: FUNCTION 00688f30
	void WriteInt32( TINT32 value ) { m_pFile->Write( &value, 4 ); }
	// $deBlob: FUNCTION 00688f50
	void WriteUInt32( TUINT32 value ) { m_pFile->Write( &value, 4 ); }
	// $deBlob: FUNCTION 00688f70
	void WriteInt16( TINT16 value ) { m_pFile->Write( &value, 2 ); }
	// $deBlob: FUNCTION 00688f90
	void WriteUInt16( TUINT16 value ) { m_pFile->Write( &value, 2 ); }
	// $deBlob: FUNCTION 00688fb0
	void WriteInt8( TINT8 value ) { m_pFile->Write( &value, 1 ); }
	// $deBlob: FUNCTION 00688fd0
	void WriteUInt8( TUINT8 value ) { m_pFile->Write( &value, 1 ); }
	// $deBlob: FUNCTION 00688ff0
	void WriteFloat( TFLOAT value ) { m_pFile->Write( &value, 4 ); }
	// The length (TUINT16) and the characters
	// $deBlob: FUNCTION 00689010
	TBOOL WriteString( const TCHAR* a_szString )
	{
		TINT    iLength  = T2String8::Length( a_szString );
		TUINT16 uiLength = TUINT16( TMath::Int32ToInt16( iLength ) );
		m_pFile->Write( &uiLength, 2 );
		m_pFile->Write( a_szString, iLength );
		return TTRUE;
	}
	void WriteVector3( TVector3& value ) { Write( value ); }
	void WriteVector4( TVector4& value ) { Write( value ); }
	void WriteQuaternion( TQuaternion& value ) { Write( value ); }

private:
	TINT    m_PositionCount;   // 0x08
	TUINT32 m_Positions[ 32 ]; // 0x0C
};

// NOTE: de Blob: 0x14 bytes, the TTSFI is a local of TTRB::Load (Barnyard keeps one in the TTRB)
class TTRB
{
public:
	friend class TAssetInit; // DestroySection

	// NOTE: int-sized in de Blob (Load returns it in eax, callers test eax)
	typedef TINT ERROR;
	enum ERROR_ : ERROR
	{
		ERROR_OK                   = 0,
		ERROR_NO_HEADER            = 1,
		ERROR_NOT_TRBF             = 2,
		ERROR_PARSE_ERROR          = 3,
		ERROR_WRONG_MAGIC          = 4,
		ERROR_FORM_MAGIC           = 5,
		ERROR_NO_FILE              = 6,
		ERROR_NOT_TRB              = 7,
		ERROR_NO_FILEINFO_ON_STACK = 8,
		ERROR_PARSE_FAILED         = 10, // TTSFI::Parse: the hunks couldn't be parsed
		ERROR_NOT_AT_START         = 11, // TTSFI::Parse: the file position isn't right behind the header
		ERROR_SAME_MAGIC           = 12, // TTSFI::Read: the file has the given magic
	};

	typedef TUINT8 AllocType;
	enum AllocType_ : AllocType
	{
		AllocType_Header,
		AllocType_Data,
		AllocType_Symbols,
	};

	using t_MemoryFuncAlloc   = void*(__stdcall*)( AllocType alloctype, TUINT32 size, TINT16 alignment, TUINT32 unk2, void* userData );
	using t_MemoryFuncDealloc = void( __stdcall* )( AllocType alloctype, void* ptr, TINT16 alignment, TUINT32 unk2, void* userData );

	struct SecInfo
	{
		TINT16  m_Unused;    // 0x0 (padding)
		TINT16  m_Alignment; // 0x2, 0 means 16
		TUINT32 m_Size;      // 0x4
		void*   m_Data;      // 0x8
		TUINT32 m_Unk2;      // 0xC
	};

	struct Header
	{
		TVersion m_ui32Version;
		TINT32   m_i32SectionCount;
	};

	struct RELCEntry
	{
		TINT16  HDRX1;  // HDRX of pointer
		TINT16  HDRX2;  // HDRX of data
		TUINT32 Offset; // Offset to pointer in HDRX1
	};

	struct TTRBSymbol
	{
		TINT16  HDRX; // NOTE: signed in retail (movsx)
		TUINT16 NameOffset;
		TUINT16 Padding;
		TINT16  NameHash;
		TUINT32 DataOffset;
	};

	struct SYMB
	{
		TINT32 m_i32SymbCount;
	};

public:
	TTRB();
	~TTRB();

	// Creates TFile and reads TRB from it
	ERROR Load( const TCHAR* a_szFilePath, TUINT32 a_uiUnknown = 0 );
	ERROR Load( TFile* a_pFile, TUINT32 a_uiUnknown = 0 );

	// Returns index of TTRBSymbol
	TINT GetSymbolIndex( const TCHAR* symbName );

	// Returns pointer to data if found and TNULL if not
	void* GetSymbolAddress( const TCHAR* symbName );

	// $deBlob: FUNCTION 00686d10
	void* GetSymbolAddress( TTRBSymbol& symb ) { return static_cast<TCHAR*>( GetSection( symb.HDRX ) ) + symb.DataOffset; }

	// Destroys TRB file and the content
	void Close();

	template <typename T>
	T* CastSymbol( const TCHAR* symbName ) { return TSTATICCAST( T, GetSymbolAddress( symbName ) ); }

	// $deBlob: FUNCTION 00686a30
	Header* GetHeader() const { return m_pHeader; }

	// $deBlob: FUNCTION 00686b20
	SecInfo* GetSectionInfoList() const { return reinterpret_cast<SecInfo*>( m_pHeader + 1 ); }

	// $deBlob: FUNCTION 00686b30
	SecInfo* GetSectionInfo( TINT index ) const { return GetSectionInfoList() + index; }

	TTRBSymbol*  GetSymbol( TINT index ) const;
	TTRBSymbol*  GetSymbol( const TCHAR* a_symbolName );
	TTRBSymbol*  GetSymbol( const void* a_pAddress );
	const TCHAR* GetSymbolName( TINT index ) const;
	const TCHAR* GetSymbolName( TTRBSymbol* symbol ) const;
	TINT32       GetNumSymbols() const { return m_SYMB->m_i32SymbCount; }

	// $deBlob: FUNCTION 00686b50
	SYMB* GetSymbolTable() const { return m_SYMB; }
	void  DeleteSymbolTable();

	void SetMemoryFunctions( t_MemoryFuncAlloc allocator, t_MemoryFuncDealloc deallocator, void* userdata );

	// Sets the memory functions new TTRBs start with (TNULL restores TMemalign/TFree)
	static void SetDefaultMemoryFunctions( t_MemoryFuncAlloc allocator, t_MemoryFuncDealloc deallocator, void* userdata );

public:
	// $deBlob: FUNCTION 00686ee0
	// $Barnyard: FUNCTION 006ba9b0
	static TUINT32 GetHeaderSize( TUINT32 count ) { return sizeof( SecInfo ) * count + sizeof( Header ); }

	// $deBlob: FUNCTION 00686ef0
	// $Barnyard: FUNCTION 006ba9c0
	static TUINT32 GetSymbolTableSize( TUINT32 count ) { return sizeof( TTRBSymbol ) * count + sizeof( SYMB ); }
	static TINT16  HashString( const TCHAR* str );

private:
	// Parses sections
	TBOOL ProcessForm( TTSFI& ttsf, TFORM* a_pForm );

	// Frees the data of a section
	void DestroySection( TINT index );

	// Returns pointer to section by index
	void* GetSection( TINT index ) { return GetSectionInfo( index )->m_Data; }

private:
	static t_MemoryFuncAlloc   s_cbDefAllocator;
	static t_MemoryFuncDealloc s_cbDefDeallocator;
	static void*               s_pDefAllocatorUserData;

private:
	Header*             m_pHeader;        // 0x00
	SYMB*               m_SYMB;           // 0x04
	t_MemoryFuncAlloc   m_MemAllocator;   // 0x08
	t_MemoryFuncDealloc m_MemDeallocator; // 0x0C
	void*               m_MemUserData;    // 0x10
};

TOSHI_NAMESPACE_END

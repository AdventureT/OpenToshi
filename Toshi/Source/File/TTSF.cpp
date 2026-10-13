#include "ToshiPCH.h"
#include "TTSF.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// Target of the alignment pads that are read/written
static TCHAR s_AlignBuffer[ 4 ] = { 0, 0, 0, 0 };

// $deBlob: FUNCTION 00687d80
// $Barnyard: FUNCTION 006d8a20
TTSFI::TTSFI()
{
	m_pFile         = TNULL;
	m_FileInfoCount = 0;
	m_iReadPos       = 0;
	TUtil::MemClear( m_FileInfo, sizeof( m_FileInfo ) );
	m_Magic            = 0;
	m_CurrentHunk.Name = 0;
	m_CurrentHunk.Size = 0;
}

// $deBlob: FUNCTION 00687dd0
TTSFI::~TTSFI()
{
	PopForm();

	if ( m_pFile )
		m_pFile->Destroy();

	m_FileInfoCount = 0;
	m_pFile         = TNULL;
}

// $deBlob: FUNCTION 00687e40
// $Barnyard: FUNCTION 006d8ca0
TUINT32 TTSFI::Open( TFile* a_pFile )
{
	TPROFILER_SCOPE();

	m_FileInfoCount = 0;
	m_pFile         = a_pFile;

	if ( m_pFile == TNULL )
	{
		return TTRB::ERROR_NO_FILE;
	}

	m_pFile->Read( &m_Header, sizeof( TTSFI::Header ) );
	m_iReadPos = 0;

	if ( m_Header.Magic == IDMAGICL )
	{
		m_Endianess = Endianess_Little;
	}
	else
	{
		if ( m_Header.Magic != IDMAGICB )
		{
			return TTRB::ERROR_NOT_TRB;
		}

		m_Endianess = Endianess_Big;
	}

	if ( m_Endianess != Endianess_Little )
		m_Header.FileSize = PARSEDWORD_BIG( m_Header.FileSize );

	m_iReadPos += m_pFile->Read( &m_Magic, 4 );

	if ( m_Endianess != Endianess_Little )
		m_Magic = PARSEDWORD_BIG( m_Magic );

	m_CurrentHunk.Name = m_Header.Magic;
	m_CurrentHunk.Size = m_Header.FileSize;
	PushForm();

	return TTRB::ERROR_OK;
}

// $deBlob: FUNCTION 00687e00
// $Barnyard: FUNCTION 006d8e70
TUINT32 TTSFI::Open( const TCHAR* a_szFilePath )
{
	TPROFILER_SCOPE();

	return Open( TFile::Create( a_szFilePath, TFILEMODE_READ ) );
}

// $deBlob: FUNCTION 00687f70
// $Barnyard: FUNCTION 006d8dc0
void TTSFI::Close( TBOOL free )
{
	PopForm();

	if ( m_pFile != TNULL && free )
	{
		TFile::Destroy( m_pFile );
	}

	m_pFile         = TNULL;
	m_FileInfoCount = 0;
}

// $deBlob: FUNCTION 00687fa0
// $Barnyard: FUNCTION 006d8a60
TUINT8 TTSFI::ReadHunk()
{
	m_pFile->Read( &m_CurrentHunk, sizeof( Hunk ) );

	if ( m_Endianess != Endianess_Little )
	{
		m_CurrentHunk.Name = PARSEDWORD_BIG( m_CurrentHunk.Name );
		m_CurrentHunk.Size = PARSEDWORD_BIG( m_CurrentHunk.Size );
	}

	m_iReadPos = 0;

	return TTRB::ERROR_OK;
}

// $deBlob: FUNCTION 00688040
// $Barnyard: FUNCTION 006d8e00
TUINT8 TTSFI::ReadHunkData( void* dst )
{
	if ( m_CurrentHunk.Name == TFourCC( "FORM" ) )
	{
		return TTRB::ERROR_FORM_MAGIC;
	}

	TASSERT( m_iReadPos == 0 );
	m_iReadPos += m_pFile->Read( dst, m_CurrentHunk.Size );
	ReadAlignmentPad();

	return TTRB::ERROR_OK;
}

// $deBlob: FUNCTION 006880e0
// $Barnyard: FUNCTION 006d8ba0
TUINT8 TTSFI::SkipHunk()
{
	TUINT32 alignedSize = TAlignNumUp( m_CurrentHunk.Size );
	m_pFile->Seek( alignedSize - m_iReadPos, TSEEK_CUR );
	m_iReadPos = alignedSize;

	return TTRB::ERROR_OK;
}

// $deBlob: FUNCTION 00688120
// $Barnyard: FUNCTION 006d8ae0
TUINT8 TTSFI::ReadFORM( TFORM* section )
{
	if ( m_CurrentHunk.Name != TFourCC( "FORM" ) )
	{
		return TTRB::ERROR_WRONG_MAGIC;
	}

	m_iReadPos += m_pFile->Read( section, sizeof( TFORM ) );

	return TTRB::ERROR_OK;
}

// $deBlob: FUNCTION 00688160
// $Barnyard: FUNCTION 006d8be0
TUINT32 TTSFI::PushForm()
{
	if ( m_CurrentHunk.Name != TFourCC( "FORM" ) &&
	     m_CurrentHunk.Name != TFourCC( "TSFL" ) &&
	     m_CurrentHunk.Name != TFourCC( "TSFB" ) )
	{
		return TTRB::ERROR_WRONG_MAGIC;
	}

	m_FileInfo[ m_FileInfoCount ].FileStartOffset = m_pFile->Tell() - 4;
	m_FileInfo[ m_FileInfoCount ].FileSize        = m_CurrentHunk.Size;
	m_FileInfoCount++;

	return TTRB::ERROR_OK;
}

// $deBlob: FUNCTION 006881b0
// $Barnyard: FUNCTION 006d8c30
TUINT32 TTSFI::PopForm()
{
	if ( TINT( m_FileInfoCount ) < 1 ) return TTRB::ERROR_NO_FILEINFO_ON_STACK;

	m_FileInfoCount--;
	auto& fileInfo = m_FileInfo[ m_FileInfoCount ];

	m_CurrentHunk.Size = fileInfo.FileSize;
	m_iReadPos          = m_pFile->Tell() - fileInfo.FileStartOffset;

	TUINT32 alignedPos = TAlignNumUp( m_CurrentHunk.Size );
	m_pFile->Seek( alignedPos - m_iReadPos, TSEEK_CUR );
	m_iReadPos = alignedPos;

	return TTRB::ERROR_OK;
}

// $deBlob: FUNCTION 00688220
TUINT32 TTSFI::ReadAlignmentPad()
{
	TUINT32 uiPos       = m_pFile->Tell();
	TUINT32 uiPadLength = 0;

	if ( uiPos & 3 )
	{
		uiPadLength = 4 - ( uiPos & 3 );
		m_iReadPos += m_pFile->Read( s_AlignBuffer, uiPadLength );
	}

	return uiPadLength;
}

// $deBlob: FUNCTION 00688250
// $Barnyard: FUNCTION 006d8b20
void TTSFI::ReadRaw( void* dst, TUINT32 size )
{
	m_iReadPos += m_pFile->Read( dst, size );
}

// $deBlob: FUNCTION 00688270
// $Barnyard: FUNCTION 006d8b40
void TTSFI::ReadCompressed( void* buffer, TUINT32 size )
{
	TCompress::Header header;

	if ( TCompress::GetHeader( m_pFile, header ) == TCOMPRESS_ERROR_OK )
	{
		TCompress::Decompress( m_pFile, &header, (TBYTE*)buffer, size );
		m_iReadPos += header.CompressedSize + sizeof( TCompress::Header );
	}
}

// $deBlob: FUNCTION 006882e0
TSIZE TTSFI::Tell()
{
	return m_pFile->Tell();
}

// $deBlob: FUNCTION 006882f0
void TTSFI::Seek( TINT a_iOffset )
{
	TSIZE uiOldPos = m_pFile->Tell();
	m_pFile->Seek( a_iOffset, TSEEK_SET );
	m_iReadPos += m_pFile->Tell() - uiOldPos;
}

// $deBlob: FUNCTION 00688340
TUINT32 TTSFI::Read( const TCHAR* a_szFilePath, TUINT32 a_uiMagic, t_HunkCallback a_fnCallback, void* a_pUserData )
{
	TTSFI   oTTSFI;
	TUINT32 uiResult = oTTSFI.Open( TFile::Create( a_szFilePath, TFILEMODE_READ ) );

	if ( uiResult == TTRB::ERROR_OK )
	{
		// NOTE: retail stops with error 12 when the file has the given magic and walks it otherwise
		if ( a_uiMagic == oTTSFI.m_Magic )
		{
			uiResult = 12;
		}
		else
		{
			oTTSFI.ProcessForm_Parse( TREINTERPRETCAST( TFORM*, &oTTSFI.m_Magic ), a_fnCallback, a_pUserData );
		}

		oTTSFI.Close();
	}

	return uiResult;
}

// $deBlob: FUNCTION 00688470
TUINT8 TTSFI::Parse( t_HunkCallback a_fnCallback, void* a_pUserData )
{
	if ( m_pFile->Tell() != 12 )
		return 11;

	return ProcessForm_Parse( TREINTERPRETCAST( TFORM*, &m_Magic ), a_fnCallback, a_pUserData ) ? 0 : 10;
}

// $deBlob: FUNCTION 006884c0
TBOOL TTSFI::ReadBool()
{
	TBOOL bValue;
	m_iReadPos += m_pFile->Read( &bValue, 1 );
	return bValue;
}

// $deBlob: FUNCTION 006884f0
TINT32 TTSFI::ReadInt32()
{
	TINT32 iValue;
	m_iReadPos += m_pFile->Read( &iValue, 4 );
	return iValue;
}

// $deBlob: FUNCTION 00688520
TUINT32 TTSFI::ReadUInt32()
{
	TUINT32 uiValue;
	m_iReadPos += m_pFile->Read( &uiValue, 4 );
	return uiValue;
}

// $deBlob: FUNCTION 00688550
TINT16 TTSFI::ReadInt16()
{
	TINT16 iValue;
	m_iReadPos += m_pFile->Read( &iValue, 2 );
	return iValue;
}

// $deBlob: FUNCTION 00688580
TUINT16 TTSFI::ReadUInt16()
{
	TUINT16 uiValue;
	m_iReadPos += m_pFile->Read( &uiValue, 2 );
	return uiValue;
}

// $deBlob: FUNCTION 006885b0
TINT8 TTSFI::ReadInt8()
{
	TINT8 iValue;
	m_iReadPos += m_pFile->Read( &iValue, 1 );
	return iValue;
}

// $deBlob: FUNCTION 006885e0
TUINT8 TTSFI::ReadUInt8()
{
	TUINT8 uiValue;
	m_iReadPos += m_pFile->Read( &uiValue, 1 );
	return uiValue;
}

// $deBlob: FUNCTION 00688610
TFLOAT TTSFI::ReadFloat()
{
	TFLOAT fValue;
	m_iReadPos += m_pFile->Read( &fValue, 4 );
	return fValue;
}

// $deBlob: FUNCTION 00688640
TVector3& TTSFI::ReadVector3( TVector3& a_rOutVec )
{
	m_iReadPos += m_pFile->Read( &a_rOutVec, sizeof( TVector3 ) );
	return a_rOutVec;
}

// $deBlob: FUNCTION 00688660
TVector4& TTSFI::ReadVector4( TVector4& a_rOutVec )
{
	m_iReadPos += m_pFile->Read( &a_rOutVec, sizeof( TVector4 ) );
	return a_rOutVec;
}

// $deBlob: FUNCTION 00688680
TQuaternion& TTSFI::ReadQuaternion( TQuaternion& a_rOutQuat )
{
	m_iReadPos += m_pFile->Read( &a_rOutQuat, sizeof( TQuaternion ) );
	return a_rOutQuat;
}

// $deBlob: FUNCTION 006886a0
void TTSFI::ReadBool( TBOOL* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, 1 );
}

// $deBlob: FUNCTION 006886c0
void TTSFI::ReadInt32( TINT32* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, 4 );
}

// $deBlob: FUNCTION 006886e0
void TTSFI::ReadUInt32( TUINT32* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, 4 );
}

// $deBlob: FUNCTION 00688700
void TTSFI::ReadInt16( TINT16* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, 2 );
}

// $deBlob: FUNCTION 00688720
void TTSFI::ReadUInt16( TUINT16* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, 2 );
}

// $deBlob: FUNCTION 00688740
void TTSFI::ReadInt8( TINT8* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, 1 );
}

// $deBlob: FUNCTION 00688760
void TTSFI::ReadUInt8( TUINT8* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, 1 );
}

// $deBlob: FUNCTION 00688780
void TTSFI::ReadFloat( TFLOAT* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, 4 );
}

// $deBlob: FUNCTION 006887a0
TINT TTSFI::ReadString( TCHAR* a_pszOut, TINT16 a_iMaxLength )
{
	TINT16 iLength;
	m_iReadPos += m_pFile->Read( &iLength, 2 );

	if ( iLength == 0 )
	{
		*a_pszOut = '\0';
		return 1;
	}

	iLength      = TMath::Min( iLength, a_iMaxLength );
	TINT iReaded = m_pFile->Read( a_pszOut, iLength );
	m_iReadPos += iReaded;
	a_pszOut[ iLength ] = '\0';

	return iReaded;
}

// $deBlob: FUNCTION 00688810
void TTSFI::ReadString( TString8& a_rOutString )
{
	TINT16 iLength;
	m_iReadPos += m_pFile->Read( &iLength, 2 );

	a_rOutString.AllocBuffer( iLength + 1 );
	TCHAR* pszBuffer = a_rOutString.GetStringUnsafe();

	m_iReadPos += m_pFile->Read( pszBuffer, iLength );
	pszBuffer[ iLength ] = '\0';
}

// $deBlob: FUNCTION 006888a0
void TTSFI::ReadVector3( TVector3* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, sizeof( TVector3 ) );
}

// $deBlob: FUNCTION 006888c0
void TTSFI::ReadVector4( TVector4* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, sizeof( TVector4 ) );
}

// $deBlob: FUNCTION 006888e0
void TTSFI::ReadQuaternion( TQuaternion* a_pOut )
{
	m_iReadPos += m_pFile->Read( a_pOut, sizeof( TQuaternion ) );
}

// $deBlob: FUNCTION 00688900
TBOOL TTSFI::Tell( TUINT32* a_pOutPos )
{
	*a_pOutPos = m_pFile->Tell();
	return TTRUE;
}

// $deBlob: FUNCTION 00688920
TBOOL TTSFI::Align()
{
	TUINT32 alignedSize = TAlignNumUp( m_CurrentHunk.Size );
	m_pFile->Seek( alignedSize - m_iReadPos, TSEEK_CUR );
	m_iReadPos = alignedSize;

	return TTRUE;
}

// $deBlob: FUNCTION 00688960
TBOOL TTSFI::ProcessForm_Parse( TFORM* a_pForm, t_HunkCallback a_fnCallback, void* a_pUserData )
{
	TINT iLeftSize = m_CurrentHunk.Size - 4;
	PushForm();

	while ( iLeftSize > 0 )
	{
		if ( ReadHunk() != TTRB::ERROR_OK )
			return TFALSE;

		iLeftSize -= TAlignNumUp( m_CurrentHunk.Size ) + sizeof( Hunk );

		if ( m_CurrentHunk.Name == TFourCC( "FORM" ) )
		{
			TFORM oForm;
			m_iReadPos += m_pFile->Read( &oForm, sizeof( oForm ) );

			if ( !ProcessForm_Parse( &oForm, a_fnCallback, a_pUserData ) )
				return TFALSE;
		}
		else
		{
			m_pFile->Tell();
			TBOOL bContinue = a_fnCallback( this, m_CurrentHunk.Name, a_pUserData );
			SkipHunk();

			if ( !bContinue )
				return TFALSE;
		}
	}

	PopForm();
	return TTRUE;
}

// $deBlob: FUNCTION 00688a90
TTSFO::TTSFO()
{
	m_pFile         = TNULL;
	m_PositionCount = 0;
	TUtil::MemClear( m_Positions, sizeof( m_Positions ) );
}

// $deBlob: FUNCTION 00688ac0
TTSFO::~TTSFO()
{
	Close();
}

TUINT32 TTSFO::WriteAlignmentPad()
{
	TUINT32 uiPos = m_pFile->Tell();

	if ( uiPos & 3 )
		return m_pFile->Write( s_AlignBuffer, 4 - ( uiPos & 3 ) );

	return 0;
}

// $deBlob: FUNCTION 00688ae0
TTSFO::ERROR TTSFO::Create( const TCHAR* filepath, TUINT32 magic, Endianess endianess )
{
	m_Endianess = endianess;
	m_pFile     = TFile::Create( filepath, TFILEMODE_CREATENEW | TFILEMODE_WRITE );

	if ( m_pFile == TNULL )
		return TTSFO::ERROR_FILE;

	TTSF::Hunk hunk{ ( m_Endianess == Endianess_Little ) ? IDMAGICL : IDMAGICB, 0 };

	if ( m_Endianess != Endianess_Little )
		magic = PARSEDWORD_BIG( magic );

	m_pFile->Write( &hunk, sizeof( hunk ) );
	m_pFile->Write( &magic, sizeof( magic ) );

	m_Positions[ m_PositionCount ] = 0;
	m_PositionCount += 1;

	return TTSFO::ERROR_OK;
}

// $deBlob: FUNCTION 00688bc0
void TTSFO::Close()
{
	if ( m_pFile != TNULL )
	{
		EndForm();
		TFile::Destroy( m_pFile );
		m_pFile = TNULL;
	}
}

// $deBlob: FUNCTION 00688bf0
TSIZE TTSFO::BeginForm( TUINT32 name )
{
	m_Positions[ m_PositionCount ] = m_pFile->Tell();

	TTSF::Hunk hunk{ TFourCC( "FORM" ), 0 };

	if ( m_Endianess != Endianess_Little )
	{
		hunk.Name = PARSEDWORD_BIG( hunk.Name );
		name      = PARSEDWORD_BIG( name );
	}

	TSIZE written1 = m_pFile->Write( &hunk, sizeof( hunk ) );
	TSIZE written2 = m_pFile->Write( &name, sizeof( name ) );
	m_PositionCount++;

	return written1 + written2;
}

// $deBlob: FUNCTION 00688c90
TSIZE TTSFO::EndForm()
{
	TUINT32 formSize = 0;

	// NOTE: retail decrements the count even when no form is open
	if ( m_PositionCount-- > 0 )
	{
		TSIZE uiOldPos = m_pFile->Tell();
		m_pFile->Seek( m_Positions[ m_PositionCount ] + 4, TSEEK_SET );
		formSize = ( uiOldPos - m_Positions[ m_PositionCount ] ) - 8;

		if ( m_Endianess != Endianess_Little )
			formSize = PARSEDWORD_BIG( formSize );

		m_pFile->Write( &formSize, sizeof( formSize ) );
		m_pFile->Seek( uiOldPos, TSEEK_SET );

		return formSize;
	}

	return 0;
}

// $deBlob: FUNCTION 00688e30
TBOOL TTSFO::OpenHunk( HunkMark* hunkMark, TUINT32 hunkName )
{
	hunkMark->Name = hunkName;
	hunkMark->Pos  = m_pFile->Tell();

	WriteHunk( hunkName, TNULL, 0 );
	return TTRUE;
}

// $deBlob: FUNCTION 00688e60
TBOOL TTSFO::CloseHunk( HunkMark* hunkMark )
{
	TSIZE uiOldPos = m_pFile->Tell();

	m_pFile->Seek( hunkMark->Pos, TSEEK_SET );
	WriteHunk( hunkMark->Name, TNULL, ( uiOldPos - hunkMark->Pos ) - sizeof( TTSF::Hunk ) );
	m_pFile->Seek( uiOldPos, TSEEK_SET );
	TTSFO::WriteAlignmentPad();

	return TTRUE;
}

// $deBlob: FUNCTION 00688d40
TUINT32 TTSFO::WriteHunk( TUINT32 hunkName, void* buffer, TSIZE bufferSize )
{
	TTSF::Hunk hunk{ hunkName, bufferSize };

	// Convert endianess
	if ( m_Endianess != Endianess_Little )
	{
		hunk.Name = PARSEDWORD_BIG( hunk.Name );
		hunk.Size = PARSEDWORD_BIG( hunk.Size );
	}

	// Write hunk
	TSIZE writtenSize = m_pFile->Write( &hunk, sizeof( hunk ) );

	// Write buffer
	if ( bufferSize != 0 && buffer != TNULL )
	{
		writtenSize += m_pFile->Write( buffer, bufferSize );
	}

	// Return num of written bytes
	return writtenSize + TTSFO::WriteAlignmentPad();
}

TOSHI_NAMESPACE_END

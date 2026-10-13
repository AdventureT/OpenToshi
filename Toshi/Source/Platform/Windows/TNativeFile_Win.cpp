#include "ToshiPCH.h"
#include "TNativeFile_Win.h"

#include <windows.h>

#undef CreateFile
#undef FindFirstFile

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

#pragma region TNativeFileSystem

// $deBlob: FUNCTION 006892c0
// $Barnyard: FUNCTION 006dd280
TNativeFileSystem::TNativeFileSystem( const TCHAR* name )
    : TFileSystem( name )
{
	m_NextFileHandle = INVALID_HANDLE_VALUE;
	TFileManager::GetSingletonSafe()->MountFileSystem( this );
}

// $deBlob: FUNCTION 006892f0
// $deBlob: FUNCTION 0068a180
// $Barnyard: FUNCTION 006be2c0
// $Barnyard: FUNCTION 006dd6b0
TNativeFileSystem::~TNativeFileSystem()
{
	UnmountFileSystem();
}

// $deBlob: FUNCTION 00689310
// $Barnyard: FUNCTION 006dd2b0
TFile* TNativeFileSystem::CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode )
{
	TNativeFile* nativeFile = new TNativeFile( this );

	if ( !nativeFile->Open( a_rcFileName, a_eFileMode ) )
	{
		delete nativeFile;
		return TNULL;
	}

	return nativeFile;
}

// $deBlob: FUNCTION 006893a0
// $Barnyard: FUNCTION 006dd6e0
void TNativeFileSystem::DestroyFile( TFile* file )
{
	if ( file != TNULL )
	{
		TSTATICCAST( TNativeFile, file )->Close();
		delete file;
	}
}

// $deBlob: FUNCTION 00689710
// $Barnyard: FUNCTION 006dccc0
TBOOL TNativeFileSystem::RemoveFile( const TString8& a_rcFileName )
{
	return DeleteFileA( a_rcFileName.GetString() ) != 0;
}

// $deBlob: FUNCTION 006893c0
// $Barnyard: FUNCTION 006dd330
TString8 TNativeFileSystem::MakeInternalPath( const TString8& a_rcPath )
{
	TString8 strInternalPath;

	// Check if it's already an absolute path
	if ( a_rcPath.Compare( "//", 2 ) == 0 ||
	     a_rcPath.Compare( "\\\\", 2 ) == 0 ||
	     ( a_rcPath.Length() >= 2 && a_rcPath[ 1 ] == ':' ) )
	{
		// Seems that the path is already an absolute path
		strInternalPath = a_rcPath;
	}
	else
	{
		// It's definitely not an absolute path so let's make one
		strInternalPath = m_Prefix;

		// Remove slash at the end of the prefix
		if ( strInternalPath.Length() > 0 )
		{
			TCHAR cLastChar = strInternalPath[ strInternalPath.Length() - 1 ];

			if ( cLastChar == '/' || cLastChar == '\\' )
				strInternalPath.Truncate( strInternalPath.Length() - 1 );
		}

		strInternalPath += TFileManager::GetSingleton()->MakeAbsolutePath( a_rcPath );
	}

	FixPathSlashes( strInternalPath );
	return strInternalPath;
}

// $deBlob: FUNCTION 006894d0
// $Barnyard: FUNCTION 006dcaf0
TBOOL TNativeFileSystem::GetFirstFile( const TString8& a_rcPath, TString8& a_rOutFileName, TFINDFILE a_ui8Flags )
{
	TString8 strPath( a_rcPath );

	// Okay... I know using TString8::Mid here is kinda original but that's how it works in the original
	const char* pchFilter =
	    ( a_rcPath.Mid( a_rcPath.Length() - 1, 1 ).Compare( "\\" ) != 0 &&
	      a_rcPath.Mid( a_rcPath.Length() - 1, 1 ).Compare( "/" ) != 0 ) ?
	    "\\*" :
	    "*";

	strPath += pchFilter;

	WIN32_FIND_DATAA oFindData;
	HANDLE           hFirstFile = FindFirstFileA( strPath.GetString(), &oFindData );
	m_NextFileHandle            = hFirstFile;

	if ( hFirstFile != INVALID_HANDLE_VALUE )
	{
		TFINDFILE ui8MaskedFlags;

		if ( oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			ui8MaskedFlags = a_ui8Flags & TFINDFILE_DIRECTORY;
		else
			ui8MaskedFlags = a_ui8Flags & TFINDFILE_FILE;

		if ( ui8MaskedFlags != 0 )
		{
			a_rOutFileName = oFindData.cFileName;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00689770
// $Barnyard: FUNCTION 006dcce0
TBOOL TNativeFileSystem::MakeDirectory( TString8 const& string )
{
	return CreateDirectoryA( string.GetString(), TNULL );
}

// $deBlob: FUNCTION 00689680
// $Barnyard: FUNCTION 006dcc30
TBOOL TNativeFileSystem::GetNextFile( TString8& a_rOutFileName, TFINDFILE a_ui8Flags )
{
	WIN32_FIND_DATAA findFileData;

	if ( m_NextFileHandle != INVALID_HANDLE_VALUE )
	{
		TBOOL bResult = FindNextFileA( m_NextFileHandle, &findFileData );
		if ( !bResult )
		{
			m_NextFileHandle = INVALID_HANDLE_VALUE;
		}
		else if ( findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if ( a_ui8Flags & TFINDFILE_DIRECTORY )
			{
				a_rOutFileName = findFileData.cFileName;
				return TTRUE;
			}
		}
		else if ( a_ui8Flags & TFINDFILE_FILE )
		{
			a_rOutFileName = findFileData.cFileName;
			return TTRUE;
		}
	}

	return TFALSE;
}

#pragma endregion

// $deBlob: FUNCTION 00689d60
TNativeFile::TNativeFile( TNativeFileSystem* pFS )
    : TFile( pFS )
{
	m_Handle           = INVALID_HANDLE_VALUE;
	m_Position         = -1;
	m_RBufferPosition  = -1;
	m_PrevBufferPos    = -1;
	m_LastBufferSize   = 0;
	m_pBuffer          = TNULL;
	m_WBuffer          = TNULL;
	m_iWriteBufferUsed = 0;
	m_bWriteBuffered   = TTRUE;
}

TNativeFile::TNativeFile( const TNativeFile& other )
    : TFile( other )
{
	m_Handle           = other.m_Handle;
	m_Position         = other.m_Position;
	m_RBufferPosition  = other.m_RBufferPosition;
	m_PrevBufferPos    = other.m_PrevBufferPos;
	m_LastBufferSize   = other.m_LastBufferSize;
	m_pBuffer          = other.m_pBuffer;
	m_WBuffer          = other.m_WBuffer;
	m_iWriteBufferUsed = other.m_iWriteBufferUsed;
	m_bWriteBuffered   = other.m_bWriteBuffered;
}

// $deBlob: FUNCTION 0068a150
// $Barnyard: FUNCTION 006dd450
TNativeFile::~TNativeFile()
{
}

// $deBlob: FUNCTION 00689ff0
// $Barnyard: FUNCTION 006dce20
TBOOL TNativeFile::LoadBuffer( TUINT32 bufferPos )
{
	DWORD lpNumberOfBytesRead;
	TASSERT( m_pBuffer != TNULL );

	if ( m_PrevBufferPos != bufferPos )
	{
		if ( m_RBufferPosition != bufferPos )
		{
			m_RBufferPosition = SetFilePointer( m_Handle, bufferPos, TNULL, FILE_BEGIN );
			if ( m_RBufferPosition != bufferPos ) return TFALSE;
		}

		if ( ReadFile( m_Handle, m_pBuffer, BUFFER_SIZE, &lpNumberOfBytesRead, TNULL ) == 0 )
		{
			return TFALSE;
		}

		m_RBufferPosition += lpNumberOfBytesRead;
		m_LastBufferSize = lpNumberOfBytesRead;
		m_PrevBufferPos  = bufferPos;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0068a090
// $Barnyard: FUNCTION 006dce90
TINT TNativeFile::FlushWriteBuffer()
{
#ifdef DEBLOB_COMMUNITY_PATCH

	// Slightly improves reading performance: retail calls WriteFile with 0 bytes on every read/seek/tell.
	if ( m_iWriteBufferUsed == 0 )
		return 0;

#endif // DEBLOB_COMMUNITY_PATCH

	if ( m_Position != m_RBufferPosition )
	{
		TASSERT( TFALSE == m_bWriteBuffered || m_iWriteBufferUsed == 0 );
		m_RBufferPosition = SetFilePointer( m_Handle, m_Position, NULL, FILE_BEGIN );

		if ( m_RBufferPosition == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
		{
			return 0;
		}

		m_Position = m_RBufferPosition;
	}

	DWORD lpNumberOfBytesWritten;
	if ( WriteFile( m_Handle, m_WBuffer, m_iWriteBufferUsed, &lpNumberOfBytesWritten, TNULL ) == 0 )
		return 0;

	m_RBufferPosition += lpNumberOfBytesWritten;
	m_Position         = m_RBufferPosition;
	m_iWriteBufferUsed = 0;
	return lpNumberOfBytesWritten;
}

// $deBlob: FUNCTION 00689f80
// $Barnyard: FUNCTION 006dd4e0
TINT TNativeFile::ReadUnbuffered( LPVOID dst, TUINT size )
{
	DWORD lpNumberOfBytesRead;
	FlushWriteBuffer();

	if ( m_Position != m_RBufferPosition )
	{
		m_RBufferPosition = SetFilePointer( m_Handle, m_Position, TNULL, FILE_BEGIN );

		if ( m_RBufferPosition == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
		{
			return 0;
		}

		m_Position = m_RBufferPosition;
	}

	if ( ReadFile( m_Handle, dst, size, &lpNumberOfBytesRead, TNULL ) == 0 )
	{
		return 0;
	}

	m_RBufferPosition += lpNumberOfBytesRead;
	m_Position = m_RBufferPosition;

	return lpNumberOfBytesRead;
}

// $deBlob: FUNCTION 006897d0
// $Barnyard: FUNCTION 006dd560
TSIZE TNativeFile::Read( void* a_pDst, TSIZE a_uiSize )
{
	FlushWriteBuffer();

	if ( TINT( a_uiSize ) < 1 ) return 0;

	if ( m_pBuffer != TNULL )
	{
		TINT   readedCount  = 0;
		TINT   startPos     = m_Position;
		TINT   curBufferPos = startPos & ~( BUFFER_SIZE - 1 );
		TINT   newBufferPos = ( startPos + a_uiSize ) & ~( BUFFER_SIZE - 1 );
		LPVOID curPosBuffer = a_pDst;

		if ( curBufferPos != newBufferPos )
		{
			if ( curBufferPos == m_PrevBufferPos )
			{
				TINT readCount = m_LastBufferSize - ( startPos - curBufferPos );

				if ( readCount > 0 )
				{
					TUtil::MemCopy( a_pDst, m_pBuffer + startPos - curBufferPos, readCount );

					curPosBuffer = (TCHAR*)a_pDst + readCount;
					m_Position += readCount;
					readedCount = readCount;
				}
			}

			TINT toReadCount = newBufferPos - m_Position;
			curBufferPos     = newBufferPos;

			if ( toReadCount > 0 )
			{
				TINT readed  = ReadUnbuffered( curPosBuffer, toReadCount );
				curPosBuffer = (TCHAR*)curPosBuffer + readed;
				readedCount += readed;

				if ( readed != toReadCount )
				{
					// end of file?
					return readedCount;
				}
			}
		}

		if ( readedCount != a_uiSize && LoadBuffer( curBufferPos ) )
		{
			TINT iSizeLeft      = a_uiSize - readedCount;
			TINT bufferLeftSize = m_Position - curBufferPos;
			TINT readCount      = TMath::Min( m_LastBufferSize - bufferLeftSize, iSizeLeft );

			if ( readCount > 0 )
			{
				TUtil::MemCopy( curPosBuffer, m_pBuffer + bufferLeftSize, readCount );
				m_Position += readCount;
				readedCount += readCount;
			}
		}

		return readedCount;
	}

	return ReadUnbuffered( a_pDst, a_uiSize );
}

// $deBlob: FUNCTION 006898f0
// $Barnyard: FUNCTION 006dcf00
TSIZE TNativeFile::Write( const void* buffer, TSIZE size )
{
	if ( m_RBufferPosition != m_Position )
	{
		TASSERT( TFALSE == m_bWriteBuffered || m_iWriteBufferUsed == 0 );
		m_RBufferPosition = SetFilePointer( m_Handle, m_Position, NULL, FILE_BEGIN );

		if ( m_RBufferPosition == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
		{
			return 0;
		}

		m_Position = m_RBufferPosition;
	}

	if ( m_bWriteBuffered == TFALSE )
	{
		DWORD written;
		BOOL  bRes = WriteFile( m_Handle, buffer, size, &written, NULL );

		if ( bRes == FALSE )
		{
			return 0;
		}

		m_RBufferPosition += written;
		m_Position = m_RBufferPosition;
		return written;
	}

	// Flush write buffer if data doesn't fit it
	if ( TINT( m_iWriteBufferUsed + size ) >= TINT( BUFFER_SIZE ) )
	{
		FlushWriteBuffer();
	}

	if ( TINT( size ) < TINT( BUFFER_SIZE ) )
	{
		// Data fits the write buffer so append it to it
		TUtil::MemCopy( m_WBuffer + m_iWriteBufferUsed, buffer, size );
		m_iWriteBufferUsed += size;
		return size;
	}
	else
	{
		// Data doesn't fit the write buffer at all so write it right now
		DWORD written;
		BOOL  bRes = WriteFile( m_Handle, buffer, size, &written, NULL );

		if ( bRes != FALSE )
		{
			m_RBufferPosition += written;
			m_Position = m_RBufferPosition;
			return written;
		}
	}

	return 0;
}

// $deBlob: FUNCTION 00689aa0
// $Barnyard: FUNCTION 006dd190
TSIZE TNativeFile::Tell()
{
	FlushWriteBuffer();
	return m_Position;
}

// $deBlob: FUNCTION 00689a20
// $Barnyard: FUNCTION 006dd000
TBOOL TNativeFile::Seek( TINT a_iOffset, TSEEK a_eOrigin )
{
	FlushWriteBuffer();

	if ( a_eOrigin == TSEEK_SET )
	{
		m_Position = ( a_iOffset < 0 ) ? 0 : a_iOffset;
	}
	else if ( a_eOrigin == TSEEK_CUR )
	{
		m_Position += a_iOffset;
		if ( m_Position < 0 ) m_Position = 0;
	}
	else if ( a_eOrigin == TSEEK_END )
	{
		m_RBufferPosition = SetFilePointer( m_Handle, a_iOffset, TNULL, FILE_END );

		if ( m_RBufferPosition == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
		{
			return TFALSE;
		}

		m_Position = m_RBufferPosition;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00689b40
// $Barnyard: FUNCTION 006dd0c0
TCHAR TNativeFile::GetCChar()
{
	FlushWriteBuffer();

	if ( m_pBuffer != TNULL )
	{
		TINT curBufferPos = m_Position & ~( BUFFER_SIZE - 1 );
		if ( ( curBufferPos == m_PrevBufferPos ) && ( m_Position - curBufferPos <= m_LastBufferSize - 1 ) )
		{
			TCHAR c = m_pBuffer[ m_Position - curBufferPos ];
			m_Position += sizeof( c );
			return c;
		}
	}

	TCHAR result;
	if ( Read( &result, sizeof( result ) ) != sizeof( result ) )
		return -1;

	return result;
}

// $deBlob: FUNCTION 00689bb0
// $Barnyard: FUNCTION 006dd120
TWCHAR TNativeFile::GetWChar()
{
	FlushWriteBuffer();

	if ( m_pBuffer != TNULL )
	{
		TINT curBufferPos = m_Position & ~( BUFFER_SIZE - 1 );
		if ( ( curBufferPos == m_PrevBufferPos ) && ( m_Position - curBufferPos <= m_LastBufferSize - 2 ) )
		{
			TWCHAR c = *TREINTERPRETCAST( TWCHAR*, &m_pBuffer[ m_Position - curBufferPos ] );
			m_Position += sizeof( c );
			return c;
		}
	}

	TWCHAR result;
	if ( Read( &result, sizeof( result ) ) != sizeof( result ) )
		return L'\xFFFF';

	return result;
}

// $deBlob: FUNCTION 00689c30
// $Barnyard: FUNCTION 006dc670
TINT TNativeFile::PutCChar( TCHAR a_cCharacter )
{
	if ( Write( &a_cCharacter, 1 ) != 1 )
	{
		return -1;
	}

	// NOTE: de Blob sign-extends the character (Barnyard masks it with 0xFF)
	return a_cCharacter;
}

// $deBlob: FUNCTION 00689c60
// $Barnyard: FUNCTION 006dc690
TINT TNativeFile::PutWChar( TWCHAR a_wcCharacter )
{
	if ( Write( &a_wcCharacter, 2 ) != 2 )
	{
		return -1;
	}

	return a_wcCharacter & 0xFFFF;
}

// $deBlob: FUNCTION 00689c90
// $Barnyard: FUNCTION 006dd1b0
TINT TNativeFile::VCPrintf( const TCHAR* a_szFormat, va_list a_vargs )
{
	TCHAR str[ 512 ];

	TINT iResult = T2String8::FormatV( str, TARRAYSIZE( str ), a_szFormat, a_vargs );

	if ( iResult < 0 )
		iResult = TARRAYSIZE( str );

	return Write( str, iResult );
}

// $deBlob: FUNCTION 00689cf0
// $Barnyard: FUNCTION 006dd200
TINT TNativeFile::VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs )
{
	TWCHAR str[ 512 ];

	TINT iResult = T2String16::FormatV( str, TARRAYSIZE( str ), a_wszFormat, a_vargs );

	if ( iResult < 0 )
		iResult = TARRAYSIZE( str );

	TINT iWritten = TINT( Write( str, iResult * sizeof( TWCHAR ) ) );

	if ( iWritten >= 0 )
		iWritten /= 2;

	return iWritten;
}

// $deBlob: FUNCTION 00689ac0
// $Barnyard: FUNCTION 006dd080
TSIZE TNativeFile::GetSize()
{
	FlushWriteBuffer();
	m_RBufferPosition = SetFilePointer( m_Handle, 0, TNULL, FILE_END );

	if ( m_RBufferPosition == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR )
	{
		return 0;
	}

	return m_RBufferPosition;
}

// $deBlob: FUNCTION 00689b00
// $Barnyard: FUNCTION 006dd250
TUINT64 TNativeFile::GetDate()
{
	TUINT64 uiLastWriteTime = 0;

	if ( m_Handle != INVALID_HANDLE_VALUE )
	{
		FILETIME oCreationTime;
		FILETIME oLastAccessTime;
		GetFileTime( m_Handle, &oCreationTime, &oLastAccessTime, (LPFILETIME)&uiLastWriteTime );
	}

	return uiLastWriteTime;
}

// $deBlob: FUNCTION 00689dd0
// $Barnyard: FUNCTION 006dcd00
TBOOL TNativeFile::Open( const TString8& a_FileName, TFILEMODE a_Mode )
{
	// NOTE: de Blob resolves the internal (native) path here, through the file system (prefix + working directory)
	TString8 strInternalPath = GetFileSystem()->MakeInternalPath( a_FileName );

	DWORD dwCreationDisposition;
	DWORD dwDesiredAccess = 0;
	DWORD dwShareMode     = 0;

	if ( a_Mode & TFILEMODE_READ ) dwDesiredAccess = GENERIC_READ;
	if ( a_Mode & TFILEMODE_WRITE ) dwDesiredAccess |= GENERIC_WRITE;
	if ( a_Mode & TFILEMODE_READWRITE ) dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;

	if ( a_Mode & TFILEMODE_CREATENEW )
	{
		dwShareMode           = FILE_SHARE_READ;
		dwCreationDisposition = CREATE_ALWAYS;

		if ( dwDesiredAccess == 0 )
			dwDesiredAccess = GENERIC_WRITE;
	}
	else
	{
		dwCreationDisposition = OPEN_EXISTING;
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail opens every file exclusively (dwShareMode 0), so loading a data file silently fails
	// while anything else has it open (another instance, a virus scanner...). Allow other readers for read-only opens.
	if ( !( dwDesiredAccess & GENERIC_WRITE ) )
		dwShareMode |= FILE_SHARE_READ;
#endif // DEBLOB_COMMUNITY_PATCH

	m_Handle = CreateFileA( strInternalPath.GetString(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, NULL, NULL );

	if ( m_Handle == INVALID_HANDLE_VALUE )
		return TFALSE;

	m_Position        = 0;
	m_RBufferPosition = 0;
	m_PrevBufferPos   = -1;
	m_LastBufferSize  = 0;

	if ( a_Mode & TFILEMODE_NOBUFFER )
	{
		m_bWriteBuffered = TFALSE;
	}
	else
	{
		m_pBuffer        = (TCHAR*)TMalloc( BUFFER_SIZE );
		m_WBuffer        = (TCHAR*)TMalloc( BUFFER_SIZE );
		m_bWriteBuffered = TTRUE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00689f20
// $Barnyard: FUNCTION 006dd480
void TNativeFile::Close()
{
	FlushWriteBuffer();
	CloseHandle( m_Handle );

	m_Handle          = INVALID_HANDLE_VALUE;
	m_Position        = -1;
	m_RBufferPosition = -1;
	m_PrevBufferPos   = -1;
	m_LastBufferSize  = 0;

	if ( m_pBuffer != TNULL )
	{
		TFree( m_pBuffer );
		m_pBuffer = TNULL;
	}

	if ( m_WBuffer != TNULL )
	{
		TFree( m_WBuffer );
		m_WBuffer = TNULL;
	}
}

// $deBlob: FUNCTION 00689ab0
void TNativeFile::FlushBuffers()
{
	FlushWriteBuffer();
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TLogFile.h"
#include "File/TFile.h"

#include <time.h>

#ifdef TOSHI_SKU_WINDOWS
#  include <windows.h>
#  include <dbghelp.h>
#  include <shlwapi.h>
#  pragma comment( lib, "dbghelp.lib" )
#  pragma comment( lib, "shlwapi.lib" )
#endif // TOSHI_SKU_WINDOWS

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

const TINT TLogFile::cLevelMax;

static const TCHAR* s_szDefUser = "DefUser";
static const TCHAR* s_szDefSys  = "DefSys";

const TCHAR* const TLogFile::cTypeStrings[ cTypeCount ] = { "Info", "Warning", "Error" };

#ifdef TOSHI_SKU_WINDOWS

//-----------------------------------------------------------------------------
// Path helpers used by TLogFile::Create to make sure the folder of the log exists.
//-----------------------------------------------------------------------------
static TCHAR s_szCanonicalPath[ MAX_PATH ];

// Returns the canonical form of a_szPath (relative to a_szBase when a_szPath has no drive), TNULL on failure
// NOTE: also used by TIniFile::Open (File/TIniFile.cpp)
// $deBlob: FUNCTION 00828730
TCHAR* CanonicalizePath( const TCHAR* a_szPath, const TCHAR* a_szBase )
{
	TCHAR szBase[ MAX_PATH ] = { 0 };
	TCHAR szCombined[ MAX_PATH ];

	if ( a_szPath == TNULL )
		return TNULL;

	TINT iLength = 0;
	for ( ; a_szPath[ iLength ] != '\0'; iLength++ )
		s_szCanonicalPath[ iLength ] = ( a_szPath[ iLength ] == '/' ) ? '\\' : a_szPath[ iLength ];

	s_szCanonicalPath[ iLength ] = '\0';

	TCHAR szDrive[ 4 ];
	TCHAR szExt[ 255 ];
	_splitpath( a_szPath, szDrive, TNULL, TNULL, szExt );

	// No extension, it's a folder
	if ( szExt[ 0 ] == '\0' )
		PathAddBackslashA( s_szCanonicalPath );

	if ( szDrive[ 0 ] == '\0' && a_szBase != TNULL )
	{
		iLength = 0;
		for ( ; a_szBase[ iLength ] != '\0'; iLength++ )
			szBase[ iLength ] = ( a_szBase[ iLength ] == '/' ) ? '\\' : a_szBase[ iLength ];

		szBase[ iLength ] = '\0';
		PathAddBackslashA( szBase );

		T2String8::Copy( szCombined, szBase, -1 );
		if ( s_szCanonicalPath[ 0 ] == '\\' )
			strcat( szCombined, s_szCanonicalPath + 1 );
		else
			strcat( szCombined, s_szCanonicalPath );

		if ( !PathCanonicalizeA( s_szCanonicalPath, szCombined ) )
			return TNULL;
	}

	return s_szCanonicalPath;
}

// Returns the folder part of a_szFullPath
// $deBlob: FUNCTION 008289a0
static TNOINLINE TCHAR* GetPathDirectory( const TCHAR* a_szFullPath )
{
	TCHAR szDrive[ 4 ];
	TCHAR szDir[ 256 ];
	TCHAR szPath[ MAX_PATH ];

	_splitpath( a_szFullPath, szDrive, szDir, TNULL, TNULL );
	_makepath( szPath, szDrive, szDir, TNULL, TNULL );

	return CanonicalizePath( szPath, TNULL );
}

// Creates every missing folder of a_szDirectory
// $deBlob: FUNCTION 00828a40
static TNOINLINE TBOOL MakeSureDirectoryExists( const TCHAR* a_szDirectory )
{
	TINT iLength = T2String8::Length( a_szDirectory );
	if ( iLength == 0 ) return TFALSE;

	TCHAR* pszBuffer = new TCHAR[ iLength + 2 ];
	if ( pszBuffer == TNULL ) return TFALSE;

	T2String8::Copy( pszBuffer, a_szDirectory, -1 );
	TINT iBufferLength = T2String8::Length( pszBuffer );

	for ( TINT i = 0; i < iBufferLength; i++ )
	{
		if ( pszBuffer[ i ] == '/' )
			pszBuffer[ i ] = '\\';
	}

	if ( pszBuffer[ iLength - 1 ] != '\\' )
	{
		pszBuffer[ iLength ]     = '\\';
		pszBuffer[ iLength + 1 ] = '\0';
	}

	TBOOL bResult = MakeSureDirectoryPathExists( pszBuffer ) == 1;

	if ( !bResult )
	{
		LPSTR pszMessage;
		FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, TNULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPSTR)&pszMessage, 0, TNULL );
		LocalFree( pszMessage );
	}

	delete[] pszBuffer;
	return bResult;
}

#endif // TOSHI_SKU_WINDOWS

// $deBlob: FUNCTION 00687740
TLogFile::TLogFile()
{
	m_pFile             = TNULL;
	m_iTotalLogCount    = 0;
	m_bIsSimpleMode     = TFALSE;
	m_bAllowIndentation = TFALSE;
	m_curLevel          = 0;
	*m_LevelString      = 0;
	m_typeCounts[ 0 ]   = 0;
	m_typeCounts[ 1 ]   = 0;
	m_typeCounts[ 2 ]   = 0;
}

// $deBlob: FUNCTION 00687780
// $deBlob: FUNCTION 0065f340
TLogFile::~TLogFile()
{
	TASSERT( TNULL == m_pFile );
	Close();
}

// $deBlob: FUNCTION 006877c0
TLogFile::Error TLogFile::Create( const TCHAR* fileName, const TCHAR* str2, TBOOL writeExisting )
{
	TASSERT( TNULL == m_pFile );

	if ( m_pFile != TNULL )
	{
		return Error::AlreadyCreated;
	}

#ifdef TOSHI_SKU_WINDOWS
	MakeSureDirectoryExists( GetPathDirectory( fileName ) );
#endif // TOSHI_SKU_WINDOWS

	TFILEMODE fileMode = TFILEMODE_NOBUFFER | TFILEMODE_WRITE | TFILEMODE_CREATENEW;

	if ( writeExisting )
		fileMode = TFILEMODE_NOBUFFER | TFILEMODE_WRITE;

	m_pFile = TFile::Create( fileName, fileMode );

	if ( m_pFile == TNULL )
	{
		return Error::OpenFile;
	}

	if ( writeExisting )
	{
		m_pFile->Seek( 0, TSEEK_END );
	}

	m_iTotalLogCount = 0;
	*m_LevelString   = 0;
	m_curLevel       = 0;

	__time64_t tNow;
	_time64( &tNow );

	m_pFile->CPrintf( "Log created [%s]:[%s]: %s\n", fileName, str2, TUtil::GetTime() );

	// NOTE: like retail, prints the link time stamp of the exe (PE header); retail reads it from the fixed image base 0x400000
	// as a 64-bit time (the 32-bit stamp and the following PointerToSymbolTable, 0 in a linked exe)
	const TUINT8* pImageBase = TREINTERPRETCAST( const TUINT8*, GetModuleHandleA( TNULL ) );
	const TUINT8* pNtHeader  = pImageBase + *TREINTERPRETCAST( const TINT32*, pImageBase + 0x3c );

	m_pFile->CPrintf( "Compilation: %s", _ctime64( TREINTERPRETCAST( const __time64_t*, pNtHeader + 8 ) ) );

	return Error::OK;
}

// $deBlob: FUNCTION 006878d0
// $Barnyard: FUNCTION 006d9080
void TLogFile::Close()
{
	m_curLevel = 0;
	RecalcLevel();

	for ( TINT i = 0; i < cTypeCount; i++ )
	{
		Print( "%s count = %d\n", cTypeStrings[ i ], m_typeCounts[ i ] );
	}

	if ( m_pFile != TNULL )
	{
		m_pFile->Destroy();
		m_pFile = TNULL;
	}
}

// $deBlob: FUNCTION 00687930
void TLogFile::Flush()
{
	if ( m_pFile )
		m_pFile->FlushBuffers();
}

// $deBlob: FUNCTION 00687b30
void TLogFile::Down()
{
	m_curLevel++;
	RecalcLevel();
}

// $deBlob: FUNCTION 00687b40
void TLogFile::Up()
{
	m_curLevel--;
	RecalcLevel();
}

// $deBlob: FUNCTION 00687b50
void TLogFile::Indent()
{
	m_curLevel++;
	RecalcLevel();
}

// $deBlob: FUNCTION 00687b60
void TLogFile::UnIndent()
{
	m_curLevel--;
	RecalcLevel();
}

// $deBlob: FUNCTION 00687ba0
// $Barnyard: FUNCTION 006d9020
void TLogFile::RecalcLevel()
{
	m_curLevel = TMath::Min<TINT>( m_curLevel, cLevelMax );
	m_curLevel = TMath::Max<TINT>( m_curLevel, 0 );

	for ( TINT i = 0; i < m_curLevel; i++ )
	{
		m_LevelString[ i ] = 9;
	}

	m_LevelString[ m_curLevel ] = 0;
}

// $deBlob: FUNCTION 00687960
// $Barnyard: FUNCTION 006d8ec0
void TLogFile::Print( const TCHAR* format, ... )
{
	if ( m_pFile != TNULL )
	{
		va_list args;
		va_start( args, format );

		TCHAR str[ 1024 ];
		T2String8::FormatV( str, sizeof( str ), format, args );

		va_end( args );

		if ( m_bAllowIndentation == TTRUE )
		{
			m_pFile->CPrintf( "%s%s", m_LevelString, str );
		}
		else
		{
			m_pFile->CPrintf( str );
		}
	}
}

// $deBlob: FUNCTION 006879e0
// $Barnyard: FUNCTION 006d8f30
void TLogFile::Log( TUtil::LogType type, const TCHAR* str1, const TCHAR* str2, const TCHAR* format, ... )
{
	if ( m_pFile != TNULL )
	{
		TCHAR strTime[ 128 ];
		strTime[ 0 ] = '\0';
		_strtime( strTime );

		if ( m_bIsSimpleMode == TTRUE )
		{
			if ( m_bAllowIndentation == TTRUE )
			{
				m_pFile->CPrintf(
				    "%d [%s]: %s",
				    m_iTotalLogCount,
#ifdef DEBLOB_COMMUNITY_PATCH
				    TUtil::LogTypeToString( type ),
#else
				    cTypeStrings[ type ],
#endif
				    m_LevelString
				);
			}
		}
		else
		{
			m_pFile->CPrintf(
			    "%d [%s] [%s]: %s: %s: %s",
			    m_iTotalLogCount,
#ifdef DEBLOB_COMMUNITY_PATCH
			    TUtil::LogTypeToString( type ),
#else
			    cTypeStrings[ type ],
#endif
			    strTime,
			    str1,
			    str2 != TNULL ? str2 : "",
			    m_bAllowIndentation == TTRUE ? m_LevelString : ""
			);
		}

		va_list args;
		va_start( args, format );
		m_pFile->VCPrintf( format, args );
		va_end( args );

		m_pFile->CPrintf( "\n" );

		// NOTE: Barnyard's fourth log type (Critical) has no counter in de Blob
#ifdef DEBLOB_COMMUNITY_PATCH
		if ( type < cTypeCount ) m_typeCounts[ type ]++;
#else
		m_typeCounts[ type ]++;
#endif
		m_iTotalLogCount++;
	}
}

// $deBlob: FUNCTION 00687ad0
void TLogFile::LogSimple( TUtil::LogType type, const TCHAR* format, ... )
{
	va_list args;
	va_start( args, format );

	TCHAR str[ 1024 ];
	T2String8::FormatV( str, sizeof( str ), format, args );

	va_end( args );

	// NOTE: retail passes the formatted text as the format of Log
	Log( type, s_szDefUser, s_szDefSys, str );
}

TOSHI_NAMESPACE_END

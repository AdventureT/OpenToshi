#include "pch.h"
#include "ADebugLog.h"

#include <Toshi/TStringManager.h>
#include <Toshi/T2String8.h>
#include <Toshi/TUtil.h>
#include <File/TFile.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The formatted text of Write
static TCHAR s_szFmtStr[ ADebugLog::DEBUGLOG_FMTSTR_BUFLEN ];

// $deBlob: FUNCTION 005b1de0
ADebugLog::ADebugLog( const TCHAR* a_szBaseName, const TCHAR* a_szSecondaryName, TINT a_iBufSize )
{
	m_iBufUsedSize = 0;

	// NOTE: retail compares the size as a float
	if ( TFLOAT( a_iBufSize ) < 0.0f )
		a_iBufSize = 0x4000;

	m_iBufSize = a_iBufSize;
	TASSERT( m_iBufSize > DEBUGLOG_FMTSTR_BUFLEN );

	TINT iBaseNameLen = TStringManager::String8Length( a_szBaseName );
	TASSERT( iBaseNameLen <= 8, "Max filename len (8) exceeded." );

	m_szBaseName = new TCHAR[ iBaseNameLen + 1 ];
	TStringManager::String8Copy( m_szBaseName, a_szBaseName, -1 );

	m_szSecondaryName = new TCHAR[ TStringManager::String8Length( a_szSecondaryName ) + 1 ];
	TStringManager::String8Copy( m_szSecondaryName, a_szSecondaryName, -1 );

	m_pBuffer = TSTATICCAST( TCHAR, TMemoryHeap_Memalign( TMemory_GetGlobalHeap(), 4, m_iBufSize ) );
}

// $deBlob: FUNCTION 005b1ed0
// $deBlob: FUNCTION 00605730
ADebugLog::~ADebugLog()
{
	Flush();
	TFree( m_szSecondaryName );
	TFree( m_szBaseName );
	TFree( m_pBuffer );
}

// $deBlob: FUNCTION 005b1f00
void ADebugLog::Write( const TCHAR* a_szFormat, ... )
{
	va_list args;
	va_start( args, a_szFormat );
	T2String8::FormatV( s_szFmtStr, DEBUGLOG_FMTSTR_BUFLEN, a_szFormat, args );
	va_end( args );

	s_szFmtStr[ DEBUGLOG_FMTSTR_BUFLEN - 1 ] = '\0';
	TINT iFmtStrSize                         = TStringManager::String8Length( s_szFmtStr );

	if ( m_iBufSize - m_iBufUsedSize < iFmtStrSize )
		Flush();

	TUtil::MemCopy( m_pBuffer + m_iBufUsedSize, s_szFmtStr, iFmtStrSize );
	m_iBufUsedSize += iFmtStrSize;

	if ( m_iBufUsedSize >= m_iBufSize )
		Flush();
}

// $deBlob: FUNCTION 005b1f70
void ADebugLog::Flush()
{
	if ( m_iBufUsedSize <= 0 )
		return;

	TCHAR szFileName[ 256 ];
	TStringManager::String8Format( szFileName, 255, "%s.%s", m_szBaseName, m_szSecondaryName );

	TINT iSize     = m_iBufUsedSize;
	m_iBufUsedSize = 0;

	// NOTE: retail recreates (empties) the file on every flush, so it only ever holds the last buffer
	TFile* pFile = TFile::Create( szFileName, TFILEMODE_WRITE | TFILEMODE_CREATENEW | TFILEMODE_NOBUFFER );
	TASSERT( pFile != TNULL );

	if ( pFile != TNULL )
	{
		pFile->Write( m_pBuffer, iSize );
		TFile::Destroy( pFile );
	}
}

// $deBlob: FUNCTION 005b2080
void ADebugLog::CreateLogFile()
{
	TCHAR szFileName[ 256 ];
	TStringManager::String8Format( szFileName, 255, "%s.%s", m_szBaseName, m_szSecondaryName );

	TFile* pFile = TFile::Create( szFileName, TFILEMODE_WRITE | TFILEMODE_CREATENEW | TFILEMODE_NOBUFFER );
	TASSERT( pFile != TNULL );

	if ( pFile != TNULL )
		TFile::Destroy( pFile );
}

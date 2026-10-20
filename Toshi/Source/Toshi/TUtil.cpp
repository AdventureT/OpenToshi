#include "ToshiPCH.h"
#include "TUtil.h"
#include "Toshi/TSystem.h"
#include "Toshi/TError.h"
#include "Toshi/TRegion.h"
#include "Toshi/T2String.h"
#include "File/TLogFile.h"
#include "Toshi/TFreeList.h"
#include "File/TFile.h"

#include <time.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

const TCHAR* const TUtil::kTypeStrings[ LogType_NUMOF ] = {
	"Info",
	"Warning",
	"Error",
	"Critical"
};
TPString8Pool** TUtil::ms_poStringPool;
TUINT32         TUtil::s_aiCRC32LUT[ CRC_TABSIZE ] = {};

// $deBlob: FUNCTION 0065d2d0
void TUtil::Destroy()
{
	// NOTE: retail inlines LogUninitialise here
	GetSingleton()->LogUninitialise();

	if ( TUtil* pUtil = GetSingleton() )
		delete pUtil;
}

// $deBlob: FUNCTION 0065d370
// $Barnyard: FUNCTION 006bff30
TBOOL TUtil::ToshiCreate( TINT a_iArgc, TCHAR** a_ppArgv, const TMemoryParams* a_pMemoryParams )
{
	TRegion_LockRegion();
	TMemory_Init( a_pMemoryParams );
	TSystemManager::Create();
	TFileManager::Create();
	Create();
	TError::CreateSingleton( 6144, 256 );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: de Blob has no TPString8, the imported engine code still uses its pool
	CreateTPStringPool();
#endif

	return TTRUE;
}

// $deBlob: FUNCTION 0065d3e0
// $Barnyard: FUNCTION 006bffb0
void TUtil::ToshiDestroy()
{
	ToshiDestroySubsystems();
	ToshiDestroyMemory();
}

// $deBlob: FUNCTION 0065d420
void TUtil::ToshiDestroySubsystems()
{
	if ( TError* pError = TError::GetSingleton() )
		delete pError;

	TFreeList::DeinitialiseAll();
	Destroy();
	TFileManager::Destroy();
	TSystemManager::Destroy();

#ifdef DEBLOB_COMMUNITY_PATCH
	DestroyTPStringPool();
#endif
}

// $deBlob: FUNCTION 0065d450
void TUtil::ToshiDestroyMemory()
{
	TMemory_Shutdown();
}

// $deBlob: FUNCTION 0065d460
void TUtil::ToshiUpdate()
{
	TSystemManager::GetSingleton()->Update();
}

// $deBlob: FUNCTION 0065d470
// $Barnyard: FUNCTION 006bfcc0
// NOTE: de Blob's MemCopy is a memmove
void* TUtil::MemCopy( void* dst, const void* src, TSIZE size )
{
	return std::memmove( dst, src, size );
}

// $deBlob: FUNCTION 0065d490
// $Barnyard: FUNCTION 006bfcf0
void* TUtil::MemMove( void* dst, const void* src, TSIZE size )
{
	return std::memmove( dst, src, size );
}

// $deBlob: FUNCTION 0065d4b0
// $Barnyard: FUNCTION 006bfd10
void TUtil::MemSet( void* ptr, TINT value, TSIZE size )
{
	std::memset( ptr, value, size );
}

// $deBlob: FUNCTION 0065d4d0
// $Barnyard: FUNCTION 006bfd50
void TUtil::MemClear( void* ptr, TSIZE size )
{
	std::memset( ptr, 0, size );
}

// $deBlob: FUNCTION 0065d4f0
TINT TUtil::MemCompare( const void* ptr1, const void* ptr2, TSIZE size )
{
	return std::memcmp( ptr1, ptr2, size );
}

// $deBlob: FUNCTION 0065d560
void TUtil::LogConsole( const TCHAR* a_szFormat, ... )
{
	if ( GetCurrentLogFile() )
	{
		TCHAR szBuffer[ 1024 ];

		va_list args;
		va_start( args, a_szFormat );
		T2String8::FormatV( szBuffer, sizeof( szBuffer ), a_szFormat, args );
		va_end( args );

		OutputDebugStringA( szBuffer );

		// NOTE: like retail, the formatted message is used as a format string
		printf( szBuffer );
	}
}

// $deBlob: FUNCTION 0065d5d0
void TUtil::LogSet( TLogFile* a_pLogFile )
{
	Log( "Changing log file." );

	if ( TUtil::GetSingleton()->m_pCurrentLogFile )
		TUtil::GetSingleton()->m_pCurrentLogFile->Close();

	if ( !a_pLogFile )
		a_pLogFile = TUtil::GetSingleton()->m_pDefaultLogFile;

	TUtil::GetSingleton()->m_pCurrentLogFile = a_pLogFile;
}

// $deBlob: FUNCTION 0065d610
// $Barnyard: FUNCTION 006bfe50
void TUtil::Log( const TCHAR* a_szFormat, ... )
{
	if ( IsSingletonCreated() && GetCurrentLogFile() )
	{
		TCHAR szBuffer[ 2048 ];

		va_list args;
		va_start( args, a_szFormat );
		T2String8::FormatV( szBuffer, sizeof( szBuffer ), a_szFormat, args );
		va_end( args );

		// NOTE: like retail, the formatted message is passed to TLogFile::Log as a format string, so a '%' in it is expanded again
		GetCurrentLogFile()->Log( LogType_Info, "Toshi", "Kernel", szBuffer );
		GetSingleton()->m_LogEmitter.Throw( LogEvent( GetCurrentLogFile(), LogType_Info, szBuffer ) );
	}
}

// $deBlob: FUNCTION 0065d6b0
void TUtil::Log( LogType a_eLogType, const TCHAR* a_szFormat, ... )
{
	if ( GetCurrentLogFile() )
	{
		TCHAR szBuffer[ 2048 ];

		va_list args;
		va_start( args, a_szFormat );
		T2String8::FormatV( szBuffer, sizeof( szBuffer ), a_szFormat, args );
		va_end( args );

		GetCurrentLogFile()->Log( a_eLogType, "Toshi", "Kernel", szBuffer );
		GetSingleton()->m_LogEmitter.Throw( LogEvent( GetCurrentLogFile(), a_eLogType, szBuffer ) );
	}
}

// $deBlob: FUNCTION 0065d750
void TUtil::LogDown()
{
	if ( IsSingletonCreated() && GetCurrentLogFile() )
		GetCurrentLogFile()->Down();
}

// $deBlob: FUNCTION 0065d770
void TUtil::LogUp()
{
	if ( IsSingletonCreated() && GetCurrentLogFile() )
		GetCurrentLogFile()->Up();
}

// $deBlob: FUNCTION 0065d790
// NOTE: same body as LogDown in retail
void TUtil::LogIndent()
{
	if ( IsSingletonCreated() && GetCurrentLogFile() )
		GetCurrentLogFile()->Down();
}

// $deBlob: FUNCTION 0065d7b0
// NOTE: same body as LogUp in retail
void TUtil::LogUnIndent()
{
	if ( IsSingletonCreated() && GetCurrentLogFile() )
		GetCurrentLogFile()->Up();
}

// $deBlob: FUNCTION 0065d7e0
TUINT32 TUtil::BCDToInteger( TUINT32 a_uiBCD )
{
	TUINT32 uiResult     = 0;
	TUINT32 uiMultiplier = 1;

	while ( a_uiBCD != 0 )
	{
		uiResult += uiMultiplier * ( a_uiBCD & 0xF );
		a_uiBCD >>= 4;
		uiMultiplier *= 10;
	}

	return uiResult;
}

// $deBlob: FUNCTION 0065d810
// $Barnyard: FUNCTION 006bfe10
// Source: https://lentz.com.au/blog/tag/crc-table-generator
TUINT32 TUtil::CRC32( void* buffer, TUINT32 len )
{
	TBYTE*  bytes = (TBYTE*)buffer;
	TUINT32 crc   = 0;

	while ( len-- )
		crc = crc32upd( s_aiCRC32LUT, crc, *bytes++ );

	return CRC32POST( crc );
}

// $deBlob: FUNCTION 0065d850
TUtil::TUtil()
    : m_LogEmitter( this )
{
	m_pDefaultLogFile = TNULL;
	m_pCurrentLogFile = TNULL;
}

// $deBlob: FUNCTION 0065d900
TBOOL TUtil::LogInitialise()
{
	TLogFile* pLogFile = new TLogFile;
	m_pDefaultLogFile  = pLogFile;
	m_pCurrentLogFile  = pLogFile;

	TrimLog( "*.log", 9 );

	__time64_t iTime;
	_time64( &iTime );
	tm* pTime = _localtime64( &iTime );

	TCHAR szFileName[ 256 ];
	sprintf(
	    szFileName,
	    "deblob_%d%02d%02d_%02d_%02d_%02d.log",
	    pTime->tm_year + 1900,
	    pTime->tm_mon + 1,
	    pTime->tm_mday,
	    pTime->tm_hour,
	    pTime->tm_min,
	    pTime->tm_sec
	);

	if ( GetCurrentLogFile()->Create( szFileName, "Toshi 2.0", TFALSE ) != TLogFile::Error::OK )
		return TFALSE;

	GetCurrentLogFile()->AllowIndentation( TTRUE );
	GetCurrentLogFile()->SetSimpleMode( TTRUE );

	return TTRUE;
}

// $deBlob: FUNCTION 0065d9f0
// $Barnyard: FUNCTION 006bff50
void TUtil::LogUninitialise()
{
	TUtil* pUtil = TUtil::GetSingleton();

	if ( pUtil->m_pCurrentLogFile )
		pUtil->m_pCurrentLogFile->Close();

	if ( m_pDefaultLogFile )
	{
		delete m_pDefaultLogFile;
		m_pDefaultLogFile = TNULL;
	}

	m_pCurrentLogFile = TNULL;
}

// $Barnyard: FUNCTION 006bfee0
void TUtil::Create()
{
	new TUtil;
	GetSingleton()->LogInitialise();
	CRCInitialise();
}

// $deBlob: FUNCTION 0065da30
// $Barnyard: FUNCTION 006bfd80
// Source: https://lentz.com.au/blog/tag/crc-table-generator
void TUtil::CRCInitialise()
{
	TUINT32 crc;

	for ( TINT i = 0; i < CRC_TABSIZE; i++ )
	{
		crc = i;
		for ( TINT j = 8; j > 0; j-- )
		{
			if ( crc & 1 )
				crc = ( crc >> 1 ) ^ CRC32POLY;
			else
				crc >>= 1;
		}
		s_aiCRC32LUT[ i ] = crc;
	}
}

// $Barnyard: FUNCTION 006bb560
void TUtil::CreateTPStringPool()
{
	TASSERT( ms_poStringPool == TNULL );
	ms_poStringPool  = new TPString8Pool*;
	*ms_poStringPool = TNULL;
}

// $Barnyard: FUNCTION 006bb620
void TUtil::DestroyTPStringPool()
{
	if ( ms_poStringPool )
	{
		delete ms_poStringPool;
		ms_poStringPool = TNULL;
	}
}

TOSHI_NAMESPACE_END

#include "pch.h"
#include "SteamManager.h"
#include "AApplication.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const AppId_t SteamManager::APP_ID = 532320;
SteamManager* SteamManager::m_instance;

// $deBlob: FUNCTION 00619f50
void SteamManager::Create()
{
	if ( m_instance == TNULL )
	{
		m_instance = new SteamManager();
	}
	else
	{
		TASSERT( TFALSE, "Already initialized" );
	}
}

// $deBlob: FUNCTION 00619fb0
void SteamManager::Unload()
{
	if ( m_instance != TNULL )
	{
		delete m_instance;
	}

	m_instance = TNULL;
}

// $deBlob: FUNCTION 0061a020
TBOOL SteamManager::InitializeSteam()
{
	if ( !SteamAPI_Init() )
	{
		TUtil::Log( "Steam initialization: SteamAPI_Init() failed" );
		return TFALSE;
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: no steam_api.dll next to the executable (see SteamAPI.cpp): the game runs offline
	if ( !SteamAPI_IsAvailable() )
	{
		TUtil::Log( "Steam initialization: steam_api.dll not available, running without Steam" );
		return TTRUE;
	}
#endif

	TUtil::Log( "Steam Initialized (BuildID = %d)", SteamApps()->GetAppBuildId() );

	if ( SteamAPI_RestartAppIfNecessary( APP_ID ) )
	{
		TUtil::Log( "Steam Initialization: Restarting app" );
		return TFALSE;
	}

	return RequestStats();
}

// $deBlob: FUNCTION 0061a0c0
const TCHAR* SteamManager::GetSteamLanguage()
{
	if ( !SteamApps() ) return TNULL;

	return SteamApps()->GetCurrentGameLanguage();
}

#if defined( DEBLOB_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )

// The folder of -devsavedir <dir> (the folder may be quoted; unquoted it ends at the next " -switch")
static TCHAR s_szDevSaveDir[ MAX_PATH ];
static TBOOL s_bDevSaveDirParsed = TFALSE;

const TCHAR* SteamManager::GetDevSaveDir()
{
	if ( !s_bDevSaveDirParsed )
	{
		s_bDevSaveDirParsed = TTRUE;
		s_szDevSaveDir[ 0 ] = '\0';

		const TCHAR* pszSwitch = T2String8::FindString( GetCommandLineA(), "-devsavedir" );

		if ( pszSwitch )
		{
			const TCHAR* pchCursor = pszSwitch + 11;
			while ( *pchCursor == ' ' || *pchCursor == '\t' ) pchCursor++;

			TINT iLength = 0;

			if ( *pchCursor == '"' )
			{
				pchCursor++;

				while ( *pchCursor != '\0' && *pchCursor != '"' && iLength < MAX_PATH - 1 )
					s_szDevSaveDir[ iLength++ ] = *( pchCursor++ );
			}
			else
			{
				while ( *pchCursor != '\0' && !( pchCursor[ 0 ] == ' ' && pchCursor[ 1 ] == '-' ) && iLength < MAX_PATH - 1 )
					s_szDevSaveDir[ iLength++ ] = *( pchCursor++ );
			}

			while ( iLength > 0 && ( s_szDevSaveDir[ iLength - 1 ] == ' ' || s_szDevSaveDir[ iLength - 1 ] == '\t' || s_szDevSaveDir[ iLength - 1 ] == '\\' || s_szDevSaveDir[ iLength - 1 ] == '/' ) )
				iLength--;

			s_szDevSaveDir[ iLength ] = '\0';
		}
	}

	return s_szDevSaveDir[ 0 ] != '\0' ? s_szDevSaveDir : TNULL;
}

// FileOperation on <-devsavedir>\<file> (native files) with the results of the Steam Cloud
static TINT DevSaveDir_FileOperation( const TCHAR* a_szDir, SteamManager::FileOp a_eOperation, const TCHAR* a_szFileName, void* a_pData, TINT a_iSize )
{
	TCHAR szPath[ MAX_PATH ];
	T2String8::Format( szPath, sizeof( szPath ), "%s\\%s", a_szDir, a_szFileName );

	TINT iResult = SteamManager::FileResult_Failed;

	if ( a_eOperation == SteamManager::FileOp_Write )
	{
		HANDLE hFile = CreateFileA( szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			DWORD uiWritten = 0;
			if ( WriteFile( hFile, a_pData, DWORD( a_iSize ), &uiWritten, NULL ) && uiWritten == DWORD( a_iSize ) )
				iResult = SteamManager::FileResult_OK;

			CloseHandle( hFile );
		}
	}
	else if ( a_eOperation == SteamManager::FileOp_Read )
	{
		HANDLE hFile = CreateFileA( szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			// Like ISteamRemoteStorage::FileRead: OK if anything was read
			DWORD uiRead = 0;
			if ( ReadFile( hFile, a_pData, DWORD( a_iSize ), &uiRead, NULL ) && uiRead != 0 )
				iResult = SteamManager::FileResult_OK;

			CloseHandle( hFile );
		}
		else
		{
			iResult = SteamManager::FileResult_NotFound;
		}
	}

	TINFO( "[devsavedir] %s %s (%d bytes): %d\n", a_eOperation == SteamManager::FileOp_Write ? "write" : "read", szPath, a_iSize, iResult );
	return iResult;
}

#endif // DEBLOB_COMMUNITY_PATCH && !TOSHI_FINAL

// $deBlob: FUNCTION 0061a120
TBOOL SteamManager::FileOperation( FileOp a_eOperation, const TCHAR* a_szFileName, void* a_pData, TINT a_iSize, t_FileCallback a_fnCallback )
{
#if defined( DEBLOB_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
	if ( const TCHAR* szDevSaveDir = GetDevSaveDir() )
	{
		TINT iDevResult = DevSaveDir_FileOperation( szDevSaveDir, a_eOperation, a_szFileName, a_pData, a_iSize );

		if ( a_fnCallback != TNULL )
			a_fnCallback( iDevResult );

		return iDevResult == FileResult_OK;
	}
#endif

	if ( !SteamAPI_GetHSteamPipe() || !SteamRemoteStorage() )
	{
#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail calls the callback without checking it and SaveLoad passes none: without Steam, loading or
		// saving crashes
		if ( a_fnCallback != TNULL )
			a_fnCallback( FileResult_Failed );
#else
		a_fnCallback( FileResult_Failed );
#endif
		return TFALSE;
	}

	TINT iResult = FileResult_Failed;

	if ( a_eOperation == FileOp_Write )
	{
		if ( SteamRemoteStorage()->FileWrite( a_szFileName, a_pData, a_iSize ) )
			iResult = FileResult_OK;
	}
	else if ( a_eOperation == FileOp_Read )
	{
		if ( SteamRemoteStorage()->FileExists( a_szFileName ) )
		{
			if ( SteamRemoteStorage()->FileRead( a_szFileName, a_pData, a_iSize ) != 0 )
				iResult = FileResult_OK;
		}
		else
		{
			iResult = FileResult_NotFound;
		}
	}

	if ( a_fnCallback != TNULL )
		a_fnCallback( iResult );

	return iResult == FileResult_OK;
}

// $deBlob: FUNCTION 0061a220
TBOOL SteamManager::SetAchievement( const TCHAR* a_szName )
{
	if ( !m_bStatsValid ) return TFALSE;

	// NOTE: like retail, no check of the interface: m_bStatsValid is only set once Steam sent the stats
	SteamUserStats()->SetAchievement( a_szName );
	return SteamUserStats()->StoreStats();
}

// $deBlob: FUNCTION 0061a2a0
TBOOL SteamManager::ClearAchievement( const TCHAR* a_szName )
{
	if ( !m_bStatsValid ) return TFALSE;

	SteamUserStats()->ClearAchievement( a_szName );
	return SteamUserStats()->StoreStats();
}

// $deBlob: FUNCTION 0061a320
TBOOL SteamManager::RequestStats()
{
	if ( !SteamUserStats() || !SteamUser() ) return TFALSE;

	if ( !SteamUser()->BLoggedOn() ) return TFALSE;

	return SteamUserStats()->RequestCurrentStats();
}

// $deBlob: FUNCTION 0061a3d0
SteamManager::SteamManager()
    : m_CallbackUserStatsReceived( this, &SteamManager::OnUserStatsReceived )
    , m_CallbackUserStatsStored( this, &SteamManager::OnUserStatsStored )
    , m_CallbackAchievementStored( this, &SteamManager::OnAchievementStored )
{
	// NOTE: the callback members register in their constructors (SteamAPI_RegisterCallback)
	m_bStatsValid = TFALSE;
}

// $deBlob: FUNCTION 0061a4a0
// $deBlob: FUNCTION 0061a830
SteamManager::~SteamManager()
{
	SteamAPI_Shutdown();
}

// $deBlob: FUNCTION 0061a4f0
void SteamManager::OnUserStatsReceived( UserStatsReceived_t* a_pCallback )
{
	if ( a_pCallback->m_nGameID == APP_ID )
	{
		m_bStatsValid = TTRUE;

		if ( a_pCallback->m_eResult == 1 ) // k_EResultOK
		{
			OutputDebugStringA( "Received stats and achievements from Steam\n" );
		}
		else
		{
			TCHAR szBuffer[ 128 ];
			_snprintf( szBuffer, sizeof( szBuffer ), "RequestStats - failed, %d\n", a_pCallback->m_eResult );
			OutputDebugStringA( szBuffer );
		}
	}
}

// $deBlob: FUNCTION 0061a560
void SteamManager::OnUserStatsStored( UserStatsStored_t* a_pCallback )
{
	if ( a_pCallback->m_nGameID == APP_ID )
	{
		if ( a_pCallback->m_eResult == 1 ) // k_EResultOK
		{
			OutputDebugStringA( "Stored stats for Steam\n" );
		}
		else
		{
			TCHAR szBuffer[ 128 ];
			_snprintf( szBuffer, sizeof( szBuffer ), "StatsStored - failed, %d\n", a_pCallback->m_eResult );
			OutputDebugStringA( szBuffer );
		}
	}
}

// $deBlob: FUNCTION 0061a5d0
void SteamManager::OnAchievementStored( UserAchievementStored_t* a_pCallback )
{
	if ( a_pCallback->m_nGameID == APP_ID )
	{
		OutputDebugStringA( "Stored Achievement for Steam\n" );
	}
}

// $deBlob: FUNCTION 00632b50
TINT SteamManager::GetLanguageIdFromCode( const TWCHAR* a_wszCode )
{
	if ( wcsncmp( a_wszCode, L"en", 2 ) == 0 ) return 0;
	if ( wcsncmp( a_wszCode, L"fr", 2 ) == 0 ) return 7;
	if ( wcsncmp( a_wszCode, L"de", 2 ) == 0 ) return 3;
	if ( wcsncmp( a_wszCode, L"es", 2 ) == 0 ) return 5;
	if ( wcsncmp( a_wszCode, L"it", 2 ) == 0 ) return 4;
	if ( wcsncmp( a_wszCode, L"ja", 2 ) == 0 ) return 12;
	if ( wcsncmp( a_wszCode, L"ko", 2 ) == 0 ) return 13;
	if ( wcsncmp( a_wszCode, L"da", 2 ) == 0 ) return 8;
	if ( wcsncmp( a_wszCode, L"nl", 2 ) == 0 ) return 2;
	if ( wcsncmp( a_wszCode, L"fi", 2 ) == 0 ) return 11;
	if ( wcsncmp( a_wszCode, L"nb", 2 ) == 0 || wcsncmp( a_wszCode, L"nn", 2 ) == 0 ) return 9;
	if ( wcsncmp( a_wszCode, L"sv", 2 ) == 0 ) return 10;
	if ( wcsncmp( a_wszCode, L"pt", 2 ) == 0 ) return 14;
	if ( wcsncmp( a_wszCode, L"zh", 2 ) == 0 ) return 15;

	return 0;
}

// An older mapping of three Steam languages (english 0, french 1, spanish 2, others 0)
// $deBlob: FUNCTION 00632d10
TINT SteamManager::GetLanguageIdFromName( const TCHAR* a_szLanguage )
{
	if ( a_szLanguage == TNULL || _stricmp( a_szLanguage, "english" ) == 0 ) return 0;
	if ( _stricmp( a_szLanguage, "french" ) == 0 ) return 1;

	return _stricmp( a_szLanguage, "spanish" ) != 0 ? 0 : 2;
}

// $deBlob: FUNCTION 00632d70
TINT SteamManager::GetLanguageId()
{
	const TCHAR* szForcedLocale = AApplication::sm_ForcedLocale;

	if ( szForcedLocale != TNULL )
	{
		TWCHAR wszCode[ 85 ] = { 0 };

		if ( mbstowcs( wszCode, szForcedLocale, 84 ) != 0 )
			return GetLanguageIdFromCode( wszCode );
	}

	GetInstance();
	const TCHAR* szLanguage = GetSteamLanguage();

	if ( szLanguage == TNULL ) return 0;

	TUtil::Log( "SteamLanguage = '%s'", szLanguage );

	if ( _stricmp( szLanguage, "english" ) == 0 ) return 0;
	if ( _stricmp( szLanguage, "french" ) == 0 ) return 7;
	if ( _stricmp( szLanguage, "spanish" ) == 0 ) return 5;
	if ( _stricmp( szLanguage, "german" ) == 0 ) return 3;
	if ( _stricmp( szLanguage, "italian" ) == 0 ) return 4;
	if ( _stricmp( szLanguage, "koreana" ) == 0 ) return 13;
	if ( _stricmp( szLanguage, "japanese" ) == 0 ) return 12;
	if ( _stricmp( szLanguage, "danish" ) == 0 ) return 8;
	if ( _stricmp( szLanguage, "dutch" ) == 0 ) return 2;
	if ( _stricmp( szLanguage, "finnish" ) == 0 ) return 11;
	if ( _stricmp( szLanguage, "norwegian" ) == 0 ) return 9;
	if ( _stricmp( szLanguage, "swedish" ) == 0 ) return 10;
	if ( _stricmp( szLanguage, "brazilian" ) == 0 ) return 14;

	return _stricmp( szLanguage, "schinese" ) != 0 ? 0 : 15;
}

// $deBlob: FUNCTION 00574df0
TBOOL SteamManager::IsJapaneseLanguage()
{
	if ( AApplication::sm_ForcedLocale != TNULL )
		return GetLanguageId() == 12;

	GetInstance();
	const TCHAR* szLanguage = GetSteamLanguage();

	if ( szLanguage != TNULL )
		return _stricmp( szLanguage, "japanese" ) == 0;

	return TFALSE;
}

// $deBlob: FUNCTION 00574e70
TBOOL SteamManager::IsKoreanLanguage()
{
	if ( AApplication::sm_ForcedLocale != TNULL )
		return GetLanguageId() == 13;

	GetInstance();
	const TCHAR* szLanguage = GetSteamLanguage();

	if ( szLanguage != TNULL )
		return _stricmp( szLanguage, "koreana" ) == 0;

	return TFALSE;
}

// $deBlob: FUNCTION 00574ef0
TBOOL SteamManager::IsChineseLanguage()
{
	if ( AApplication::sm_ForcedLocale != TNULL )
		return GetLanguageId() == 15;

	GetInstance();
	const TCHAR* szLanguage = GetSteamLanguage();

	if ( szLanguage != TNULL )
		return _stricmp( szLanguage, "schinese" ) == 0;

	return TFALSE;
}

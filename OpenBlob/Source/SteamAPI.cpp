#include "pch.h"
#include "SteamAPI.h"

#ifdef TRENDERINTERFACE_DX11
#  include <Platform/DX11/TMSWindow_DX11.h>
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// NOTE: retail imports these from steam_api.dll (steam_api.lib). Without the Steamworks SDK we load the retail
// steam_api.dll at runtime, only from the folder of the executable (LOAD_LIBRARY_SEARCH_APPLICATION_DIR): never from
// the "-gamedir" folder, so test runs (whose executable folder has no steam_api.dll) never talk to the user's Steam
// client (SteamAPI_RestartAppIfNecessary would launch the retail game through Steam). Copy steam_api.dll (and
// steam_appid.txt) next to the executable to run with Steam.
// Without the DLL every call fails like an unreachable Steam client; with DEBLOB_COMMUNITY_PATCH, SteamAPI_Init
// succeeds instead (the game runs offline: no language from Steam, no stats/achievements).
// "-background" runs (DEBLOB_COMMUNITY_PATCH, unattended tests: mytest.sh copies the retail steam_api.dll next to the
// test exe) never load steam_api.dll either, so they never use the user's Steam client.

typedef bool( __cdecl* t_SteamAPI_Init )();
typedef void( __cdecl* t_SteamAPI_Shutdown )();
typedef bool( __cdecl* t_SteamAPI_RestartAppIfNecessary )( AppId_t );
typedef void( __cdecl* t_SteamAPI_RunCallbacks )();
typedef void( __cdecl* t_SteamAPI_RegisterCallback )( CCallbackBase*, int );
typedef void( __cdecl* t_SteamAPI_UnregisterCallback )( CCallbackBase* );
typedef void( __cdecl* t_SteamAPI_SetMiniDumpComment )( const char* );
typedef void( __cdecl* t_SteamAPI_WriteMiniDump )( TUINT32, void*, TUINT32 );
typedef HSteamUser( __cdecl* t_SteamAPI_GetHSteamUser )();
typedef HSteamPipe( __cdecl* t_SteamAPI_GetHSteamPipe )();
typedef void*( __cdecl* t_SteamInternal_CreateInterface )( const char* );

struct SteamAPIImports
{
	TBOOL                            bLoaded;
	HMODULE                          hModule;
	t_SteamAPI_Init                  pfnInit;
	t_SteamAPI_Shutdown              pfnShutdown;
	t_SteamAPI_RestartAppIfNecessary pfnRestartAppIfNecessary;
	t_SteamAPI_RunCallbacks          pfnRunCallbacks;
	t_SteamAPI_RegisterCallback      pfnRegisterCallback;
	t_SteamAPI_UnregisterCallback    pfnUnregisterCallback;
	t_SteamAPI_SetMiniDumpComment    pfnSetMiniDumpComment;
	t_SteamAPI_WriteMiniDump         pfnWriteMiniDump;
	t_SteamAPI_GetHSteamUser         pfnGetHSteamUser;
	t_SteamAPI_GetHSteamPipe         pfnGetHSteamPipe;
	t_SteamInternal_CreateInterface  pfnCreateInterface;
};

static SteamAPIImports s_oSteamAPI;

// The interfaces de Blob uses (CSteamAPIContext of steam_api.h)
struct SteamAPIContext
{
	ISteamClient*        pSteamClient;
	ISteamUser*          pSteamUser;
	ISteamUserStats*     pSteamUserStats;
	ISteamRemoteStorage* pSteamRemoteStorage;
	ISteamApps*          pSteamApps;
};

static SteamAPIContext s_oSteamContext;

static void SteamAPI_Load()
{
	if ( s_oSteamAPI.bLoaded ) return;
	s_oSteamAPI.bLoaded = TTRUE;

#if defined( DEBLOB_COMMUNITY_PATCH ) && defined( TRENDERINTERFACE_DX11 )
	if ( Toshi::TMSWindow::IsBackgroundMode() )
	{
		TWARN( "-background: steam_api.dll isn't loaded, Steam isn't available\n" );
		return;
	}
#endif

	s_oSteamAPI.hModule = LoadLibraryExA( "steam_api.dll", NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR );

	if ( s_oSteamAPI.hModule == NULL )
	{
		TWARN( "steam_api.dll not found next to the executable, Steam isn't available\n" );
		return;
	}

	HMODULE hModule = s_oSteamAPI.hModule;

	s_oSteamAPI.pfnInit                  = (t_SteamAPI_Init)GetProcAddress( hModule, "SteamAPI_Init" );
	s_oSteamAPI.pfnShutdown              = (t_SteamAPI_Shutdown)GetProcAddress( hModule, "SteamAPI_Shutdown" );
	s_oSteamAPI.pfnRestartAppIfNecessary = (t_SteamAPI_RestartAppIfNecessary)GetProcAddress( hModule, "SteamAPI_RestartAppIfNecessary" );
	s_oSteamAPI.pfnRunCallbacks          = (t_SteamAPI_RunCallbacks)GetProcAddress( hModule, "SteamAPI_RunCallbacks" );
	s_oSteamAPI.pfnRegisterCallback      = (t_SteamAPI_RegisterCallback)GetProcAddress( hModule, "SteamAPI_RegisterCallback" );
	s_oSteamAPI.pfnUnregisterCallback    = (t_SteamAPI_UnregisterCallback)GetProcAddress( hModule, "SteamAPI_UnregisterCallback" );
	s_oSteamAPI.pfnSetMiniDumpComment    = (t_SteamAPI_SetMiniDumpComment)GetProcAddress( hModule, "SteamAPI_SetMiniDumpComment" );
	s_oSteamAPI.pfnWriteMiniDump         = (t_SteamAPI_WriteMiniDump)GetProcAddress( hModule, "SteamAPI_WriteMiniDump" );
	s_oSteamAPI.pfnGetHSteamUser         = (t_SteamAPI_GetHSteamUser)GetProcAddress( hModule, "SteamAPI_GetHSteamUser" );
	s_oSteamAPI.pfnGetHSteamPipe         = (t_SteamAPI_GetHSteamPipe)GetProcAddress( hModule, "SteamAPI_GetHSteamPipe" );
	s_oSteamAPI.pfnCreateInterface       = (t_SteamInternal_CreateInterface)GetProcAddress( hModule, "SteamInternal_CreateInterface" );

	if ( !s_oSteamAPI.pfnInit || !s_oSteamAPI.pfnShutdown || !s_oSteamAPI.pfnRestartAppIfNecessary ||
	     !s_oSteamAPI.pfnRunCallbacks || !s_oSteamAPI.pfnRegisterCallback || !s_oSteamAPI.pfnUnregisterCallback ||
	     !s_oSteamAPI.pfnSetMiniDumpComment || !s_oSteamAPI.pfnWriteMiniDump || !s_oSteamAPI.pfnGetHSteamUser ||
	     !s_oSteamAPI.pfnGetHSteamPipe || !s_oSteamAPI.pfnCreateInterface )
	{
		TWARN( "steam_api.dll misses exports, Steam isn't available\n" );
		FreeLibrary( hModule );
		s_oSteamAPI.hModule = NULL;
	}
}

#ifdef DEBLOB_COMMUNITY_PATCH
TBOOL SteamAPI_IsAvailable()
{
	SteamAPI_Load();
	return s_oSteamAPI.hModule != NULL;
}
#endif

bool SteamAPI_Init()
{
	SteamAPI_Load();

	if ( s_oSteamAPI.hModule == NULL )
	{
#ifdef DEBLOB_COMMUNITY_PATCH
		// Runs without Steam
		return true;
#else
		return false;
#endif
	}

	return s_oSteamAPI.pfnInit();
}

void SteamAPI_Shutdown()
{
	if ( s_oSteamAPI.hModule ) s_oSteamAPI.pfnShutdown();
	s_oSteamContext = SteamAPIContext();
}

bool SteamAPI_RestartAppIfNecessary( AppId_t a_unOwnAppID )
{
	SteamAPI_Load();
	return s_oSteamAPI.hModule ? s_oSteamAPI.pfnRestartAppIfNecessary( a_unOwnAppID ) : false;
}

void SteamAPI_RunCallbacks()
{
	if ( s_oSteamAPI.hModule ) s_oSteamAPI.pfnRunCallbacks();
}

void SteamAPI_RegisterCallback( CCallbackBase* a_pCallback, int a_iCallback )
{
	SteamAPI_Load();
	if ( s_oSteamAPI.hModule ) s_oSteamAPI.pfnRegisterCallback( a_pCallback, a_iCallback );
}

void SteamAPI_UnregisterCallback( CCallbackBase* a_pCallback )
{
	if ( s_oSteamAPI.hModule ) s_oSteamAPI.pfnUnregisterCallback( a_pCallback );
}

void SteamAPI_SetMiniDumpComment( const char* a_pchMsg )
{
	if ( s_oSteamAPI.hModule ) s_oSteamAPI.pfnSetMiniDumpComment( a_pchMsg );
}

void SteamAPI_WriteMiniDump( TUINT32 a_uStructuredExceptionCode, void* a_pvExceptionInfo, TUINT32 a_uBuildID )
{
	if ( s_oSteamAPI.hModule ) s_oSteamAPI.pfnWriteMiniDump( a_uStructuredExceptionCode, a_pvExceptionInfo, a_uBuildID );
}

HSteamUser SteamAPI_GetHSteamUser()
{
	return s_oSteamAPI.hModule ? s_oSteamAPI.pfnGetHSteamUser() : 0;
}

HSteamPipe SteamAPI_GetHSteamPipe()
{
	return s_oSteamAPI.hModule ? s_oSteamAPI.pfnGetHSteamPipe() : 0;
}

void* SteamInternal_CreateInterface( const char* a_szVersion )
{
	return s_oSteamAPI.hModule ? s_oSteamAPI.pfnCreateInterface( a_szVersion ) : TNULL;
}

static TBOOL SteamAPIContext_Init()
{
	HSteamUser hSteamUser = SteamAPI_GetHSteamUser();
	HSteamPipe hSteamPipe = SteamAPI_GetHSteamPipe();
	if ( !hSteamPipe ) return TFALSE;

	s_oSteamContext.pSteamClient = TSTATICCAST( ISteamClient, SteamInternal_CreateInterface( "SteamClient017" ) );
	if ( !s_oSteamContext.pSteamClient ) return TFALSE;

	s_oSteamContext.pSteamUser = s_oSteamContext.pSteamClient->GetISteamUser( hSteamUser, hSteamPipe, "SteamUser019" );
	if ( !s_oSteamContext.pSteamUser ) return TFALSE;

	s_oSteamContext.pSteamUserStats = s_oSteamContext.pSteamClient->GetISteamUserStats( hSteamUser, hSteamPipe, "STEAMUSERSTATS_INTERFACE_VERSION011" );
	if ( !s_oSteamContext.pSteamUserStats ) return TFALSE;

	s_oSteamContext.pSteamApps = s_oSteamContext.pSteamClient->GetISteamApps( hSteamUser, hSteamPipe, "STEAMAPPS_INTERFACE_VERSION008" );
	if ( !s_oSteamContext.pSteamApps ) return TFALSE;

	s_oSteamContext.pSteamRemoteStorage = s_oSteamContext.pSteamClient->GetISteamRemoteStorage( hSteamUser, hSteamPipe, "STEAMREMOTESTORAGE_INTERFACE_VERSION013" );
	if ( !s_oSteamContext.pSteamRemoteStorage ) return TFALSE;

	return TTRUE;
}

ISteamUser* SteamUser()
{
	if ( !SteamAPI_GetHSteamPipe() ) return TNULL;
	if ( !s_oSteamContext.pSteamUser ) SteamAPIContext_Init();
	return s_oSteamContext.pSteamUser;
}

ISteamUserStats* SteamUserStats()
{
	if ( !SteamAPI_GetHSteamPipe() ) return TNULL;
	if ( !s_oSteamContext.pSteamUserStats ) SteamAPIContext_Init();
	return s_oSteamContext.pSteamUserStats;
}

ISteamApps* SteamApps()
{
	if ( !SteamAPI_GetHSteamPipe() ) return TNULL;
	if ( !s_oSteamContext.pSteamApps ) SteamAPIContext_Init();
	return s_oSteamContext.pSteamApps;
}

ISteamRemoteStorage* SteamRemoteStorage()
{
	if ( !SteamAPI_GetHSteamPipe() ) return TNULL;
	if ( !s_oSteamContext.pSteamRemoteStorage ) SteamAPIContext_Init();
	return s_oSteamContext.pSteamRemoteStorage;
}

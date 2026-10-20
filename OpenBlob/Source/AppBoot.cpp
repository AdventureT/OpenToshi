#include "pch.h"
#include "Cameras/ACameraManager.h"
#include "AppBoot.h"
#include "SteamManager.h"
#include "AAssetStreaming.h"
#include "Memory/AMemory.h"
#include "Render/ARenderer.h"
#include "Input/AInputManager2.h"
#include "Input/ABlobInputMap.h"
#include "GUI/AGUIManager.h"
#include "GUI/AFontManager.h"
#include "Locale/ALocaleManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AAppInitState.h"
#include "GameInterface/AXUIState.h"
#include "GUI/A2GUIRenderer_DX11.h"
#include "Movie/AXUIBinkMovie_DX11.h"
#include "Sound/AXUIFMODExAudio.h"

#include <Toshi/TUtil.h>
#include <Sound/TSound.h>
#include <File/TFile.h>
#include <Toshi/TSystem.h>
#include <Toshi/T2String8.h>
#include <Toshi/TString16.h>
#include <Toshi/TRegion.h>
#include <GUI/T2GUI.h>
#include <GUI/T2GUIFontManager.h>
#include <XUI/TXUI.h>
#include <XUI/TXUICanvas.h>
#include <XUI/TXUIText.h>
#include <Render/TRender.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static const TCHAR* s_szBuildVersion = "0.28";

// OSVERSIONINFOEXW of RtlGetVersion
typedef LONG( WINAPI* t_RtlGetVersion )( OSVERSIONINFOEXW* );

// The allocator of the strings: the pool of the frequent allocations
// $deBlob: FUNCTION 00574d90
static T2Allocator* AppStringAllocator()
{
	return AMemory::GetAllocator( AMemory::POOL_FrequentAllocations );
}

// Initialises the classes that aren't
// $deBlob: FUNCTION 00574da0
static TBOOL InitialiseStaticCallback( TClass* a_pClass, void* a_pUserData )
{
	if ( !a_pClass->IsInitialized() )
		a_pClass->Initialize();

	return TTRUE;
}

// The movie of the XUI video elements (TXUI::CreateMovie)
// $deBlob: FUNCTION 00574dc0
static TXUIMovie* CreateXUIMovie()
{
	return new ( TXUI::ms_pXUIMemoryBlock ) AXUIBinkMovie();
}

// The game's string overrides as T2Locale's hook (T2Locale's strings aren't const)
static T2LocalisedString GetOverwrittenLocaleString( TINT a_iStringId, T2Locale::Lang a_iLangId )
{
	return const_cast<T2LocalisedString>( ALocaleManager::GetOverwrittenString( a_iStringId, a_iLangId ) );
}

// $deBlob: FUNCTION 005747d0
TBOOL AApplication::OnCreate( TINT argc, TCHAR** argv )
{
	ParseCommandLineParams( argc, argv );
	TDebug_FinalPrintf( "Starting Blob...\n" );

	AMemory::CreatePools();
	AMemory::SetupTRBAllocators();
	InitialiseFreeLists();

	TString8::SetDefaultAllocatorCB( AppStringAllocator );
	TString16::SetDefaultAllocatorCB( AppStringAllocator );

	m_pRenderer = new ARenderer();

	if ( !m_pRenderer->CreateInterface() )
		return TFALSE;

	T2GUI::Open( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) );

	TGetClass( TObject ).RecurseTree( TNULL, TNULL, InitialiseStaticCallback, TNULL );

	new AInputManager2();

	new TSound();

	m_pSoundMempool = TMemoryHeap_Memalign( TMemory_GetGlobalHeap(), 32, 0x8000000 );

	TBOOL bResult = TSound::GetSingleton()->Create( m_pSoundMempool, 0x8000000, -1, -1, TSound::SpeakerType_Driver );
	TASSERT( TTRUE == bResult );

#ifdef DEBLOB_COMMUNITY_PATCH
	// -gamedir: FMOD reads through TFileManager so the sound banks fall back to the game folder
	if ( TFileManager::GetSingleton()->FindFileSystem( "gamedir" ) != TNULL )
		TSound::GetSingleton()->SetTFileSystem( 0x10000 );
	else
#endif
		TSound::GetSingleton()->GetSystem()->setFileSystem( TNULL, TNULL, TNULL, TNULL, TNULL, TNULL, 0x10000 );

	ALocaleManager::Create();

	// NOTE: retail's T2Locale::GetString calls ALocaleManager::FixStringIdPlatform and GetOverwrittenString directly
	T2Locale::SetGameHooks( ALocaleManager::FixStringIdPlatform, GetOverwrittenLocaleString );

	TXUI::ms_pXUIMemoryBlock    = AMemory::GetPool( AMemory::POOL_XUI );
	TXUI::ms_pXUITRBMemoryBlock = AMemory::GetPool( AMemory::POOL_XUI );
	TMemoryHeap* pOldTRBHeap    = AMemory::SetTRBHeap( AMemory::GetPool( AMemory::POOL_XUI ) );

	TXUI* pXUI = new TXUI();

	pXUI->SetCreateMovieCallback( CreateXUIMovie );

	// NOTE: retail's TXUIScene reads the bindings of ABlobInputMap_Win.cpp directly
	ABlobInputMap::InstallXUIBindings();

	TXUIText::SetClipExtended( TTRUE );

	// NOTE: retail's T2GUIFont::SetRegion calls SteamManager directly
	T2GUIFont::ms_fnIsJapaneseLanguage = SteamManager::IsJapaneseLanguage;
	T2GUIFont::ms_fnIsChineseLanguage  = SteamManager::IsChineseLanguage;
	T2GUIFontManager::Open();

	const TCHAR* szDefaultFont;

	if ( SteamManager::IsJapaneseLanguage() )
		szDefaultFont = "data/font/JP/Arial_Unicode_MS_14.font.trb";
	else if ( SteamManager::IsKoreanLanguage() )
		szDefaultFont = "data/font/KO/Arial_Unicode_MS_14.font.trb";
	else if ( SteamManager::IsChineseLanguage() )
		szDefaultFont = "data/font/ZH/Arial_Unicode_MS_14.font.trb";
	else
		szDefaultFont = "data/font/Arial_Unicode_MS_14.font.trb";

	pXUI->SetDefaultFont( szDefaultFont, "Arial_Unicode_MS_14" );
	AXUIState::SetSkin1( "commonskin.trb", "commonskin.xur" );
	AXUIState::SetSkin2( "frontendskin.trb", "frontendskin.xur" );

	const TCHAR* szFont;

	if ( SteamManager::IsJapaneseLanguage() )
		szFont = "data/font/JP/blob.font.trb";
	else if ( SteamManager::IsKoreanLanguage() )
		szFont = "data/font/KO/blob.font.trb";
	else if ( SteamManager::IsChineseLanguage() )
		szFont = "data/font/ZH/blob.font.trb";
	else
		szFont = "data/font/blob.font.trb";

	AXUIState::InitFont( szFont );
	AXUIState::SetCommonSkin( "common.trb" );
	AXUIState::SetSkinWithFont( "frontend.trb", TFALSE );

	pXUI->SetAudio( new AXUIFMODExAudio() );

	pXUI->SetCanvasDimensions( 1920.0f, 1080.0f );
	AMemory::SetTRBHeap( pOldTRBHeap );

	// The canvas (1920x1080) is scaled to the display
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();
	TFLOAT                  fScaleX        = TFLOAT( ceil( ( TFLOAT( TINT( pDisplayParams->uiWidth ) ) / 1920.0f ) * 256.0f ) );
	TFLOAT                  fScaleY        = TFLOAT( ceil( ( TFLOAT( TINT( pDisplayParams->uiHeight ) ) / 1080.0f ) * 256.0f ) );
	pXUI->GetCanvas()->SetScale( fScaleX * ( 1.0f / 256.0f ), fScaleY * ( 1.0f / 256.0f ) );

	new AAssetStreaming();

	if ( !m_pRenderer->Create() )
		return TFALSE;

	const TCHAR* szGUIFonts;

	if ( SteamManager::IsJapaneseLanguage() )
		szGUIFonts = "data/gui/JP/fonts.trb";
	else if ( SteamManager::IsKoreanLanguage() )
		szGUIFonts = "data/gui/KO/fonts.trb";
	else if ( SteamManager::IsChineseLanguage() )
		szGUIFonts = "data/gui/ZH/fonts.trb";
	else
		szGUIFonts = "data/gui/fonts.trb";

	T2GUIFontManager::LoadFontTRB( szGUIFonts );
	AFontManager::Create();
	new AGUIManager();

	T2GUI::GetSingleton()->SetRenderer( new A2GUIRenderer() );

	AGameState::SetupLoadIcon();
	g_oTheApp.SetRenderWorld( TTRUE );

	m_pGameStateController = new AGameStateController();
	m_pGameStateController->Create();
	m_bInitialised = TTRUE;

	new ACameraManager();

	AGameStateController::GetSingleton()->PushState( new AAppInitState() );

	return TApplication::OnCreate( argc, argv );
}

// $deBlob: FUNCTION 00576760
static LONG WINAPI TopLevelExceptionFilter( EXCEPTION_POINTERS* a_pExceptionInfo )
{
	printf( "Blob_UnhandledExceptionFilter\n" );

	DWORD uiExceptionCode = a_pExceptionInfo->ExceptionRecord->ExceptionCode;
	SteamAPI_SetMiniDumpComment( "Deblob07.exe\n" );
	SteamAPI_WriteMiniDump( uiExceptionCode, a_pExceptionInfo, 0 );

	return EXCEPTION_EXECUTE_HANDLER;
}

// The name of the Windows version
struct OSName
{
	const TCHAR* szName;
	DWORD        dwMajorVersion;
	DWORD        dwMinorVersion;
	TBOOL        bWorkstation;
};

static const OSName s_aOSNames[] = {
	{ "Windows 10", 10, 0, TTRUE },
	{ "Windows Server 2016", 10, 0, TFALSE },
	{ "Windows 8.1", 6, 3, TTRUE },
	{ "Windows Server 2012 R2", 6, 3, TFALSE },
	{ "Windows 8", 6, 2, TTRUE },
	{ "Windows Server 2012", 6, 2, TFALSE },
	{ "Windows 7", 6, 1, TTRUE },
	{ "Windows Server 2008 R2", 6, 1, TFALSE },
	{ "Windows Server 2008", 6, 0, TFALSE },
	{ "Windows Vista", 6, 0, TTRUE },
};

// Fills a_pVersionInfo (0x11c bytes) with RtlGetVersion: TRUE if it succeeded
// $deBlob: FUNCTION 005767a0
BOOL GetOSVersion( OSVERSIONINFOEXW* a_pVersionInfo )
{
	memset( a_pVersionInfo, 0, sizeof( *a_pVersionInfo ) );
	a_pVersionInfo->dwOSVersionInfoSize = sizeof( *a_pVersionInfo );

	HMODULE         hNtDll           = GetModuleHandleW( L"ntdll.dll" );
	t_RtlGetVersion pfnRtlGetVersion = (t_RtlGetVersion)GetProcAddress( hNtDll, "RtlGetVersion" );

	if ( pfnRtlGetVersion == NULL )
		return FALSE;

	return pfnRtlGetVersion( a_pVersionInfo ) == 0;
}

// $deBlob: FUNCTION 005767f0
static const TCHAR* GetOSName()
{
	OSVERSIONINFOEXW oVersionInfo;
	memset( &oVersionInfo, 0, sizeof( oVersionInfo ) );
	oVersionInfo.dwOSVersionInfoSize = sizeof( oVersionInfo );

	HMODULE         hNtDll           = GetModuleHandleW( L"ntdll.dll" );
	t_RtlGetVersion pfnRtlGetVersion = (t_RtlGetVersion)GetProcAddress( hNtDll, "RtlGetVersion" );
	if ( pfnRtlGetVersion != NULL ) pfnRtlGetVersion( &oVersionInfo );

	for ( TINT i = 0; i < TINT( TARRAYSIZE( s_aOSNames ) ); i++ )
	{
		if ( oVersionInfo.dwMajorVersion == s_aOSNames[ i ].dwMajorVersion &&
		     oVersionInfo.dwMinorVersion == s_aOSNames[ i ].dwMinorVersion &&
		     ( oVersionInfo.wProductType == VER_NT_WORKSTATION ) == s_aOSNames[ i ].bWorkstation )
			return s_aOSNames[ i ].szName;
	}

	return "unknown";
}

#ifdef DEBLOB_COMMUNITY_PATCH
static TBOOL HasSwitch( const TCHAR* a_szSwitch )
{
	return T2String8::FindString( GetCommandLineA(), a_szSwitch ) != TNULL;
}
#endif

// $deBlob: FUNCTION 005768c0
#ifdef TOSHI_FINAL
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, INT cmdShow )
{
	TINT    argc = __argc;
	TCHAR** argv = __argv;
#else  // TOSHI_FINAL
int __cdecl main( int argc, TCHAR** argv ) // __cdecl: the build uses /Gz
{
#endif // !TOSHI_FINAL
	SetProcessDPIAware();
	TRegion_SetRegion( TREGION_UK );

	TMemoryParams oMemoryParams;
	oMemoryParams.flags  = 0;
	oMemoryParams.uiSize = 640 * 1024 * 1024;
	TUtil::ToshiCreate( argc, argv, &oMemoryParams );

	TUtil::Log( "Build Version %s", s_szBuildVersion );

	OSVERSIONINFOEXW oVersionInfo;
	TUtil::MemClear( &oVersionInfo, sizeof( oVersionInfo ) );
	oVersionInfo.dwOSVersionInfoSize = sizeof( oVersionInfo );

	HMODULE         hNtDll           = GetModuleHandleW( L"ntdll.dll" );
	t_RtlGetVersion pfnRtlGetVersion = (t_RtlGetVersion)GetProcAddress( hNtDll, "RtlGetVersion" );
	if ( pfnRtlGetVersion != NULL ) pfnRtlGetVersion( &oVersionInfo );

	TCHAR szCommandLine[ 4096 ];
	TINT  iCommandLineLength = 0;

	for ( TINT i = 0; i < argc; i++ )
		iCommandLineLength += sprintf( &szCommandLine[ iCommandLineLength ], "%s ", argv[ i ] );

	TUtil::Log( "Command Line: %s", szCommandLine );
	TUtil::Log( "OS Name: %s", GetOSName() );

	// NOTE: retail logs the (wide) service pack string with %s: only its first character
	TUtil::Log( "OS Version: %d.%d Build:%d %s", oVersionInfo.dwMajorVersion, oVersionInfo.dwMinorVersion, oVersionInfo.dwBuildNumber, oVersionInfo.szCSDVersion );

	// Single instance
	HANDLE hMutex = TNULL;

#ifdef DEBLOB_COMMUNITY_PATCH
	// Development: "-multiinstance" allows several (test) runs at once
	if ( !HasSwitch( "-multiinstance" ) )
#endif
	{
		hMutex = CreateMutexA( NULL, TRUE, "BLOB07" );

		if ( GetLastError() == ERROR_ALREADY_EXISTS )
		{
			MessageBoxA( NULL, "Game is already running", "de Blob", MB_OK );
			return 0;
		}
	}

	if ( !IsDebuggerPresent() )
		SetUnhandledExceptionFilter( TopLevelExceptionFilter );

	SteamManager::Create();

	if ( !SteamManager::GetInstance()->InitializeSteam() )
	{
		MessageBoxA( NULL, "Failed to init steam", "de Blob", MB_OK );
		return -1;
	}

	SteamManager::GetInstance();
	const TCHAR* szLanguage = SteamManager::GetSteamLanguage();
	printf( "Application language = %s\n", szLanguage ? szLanguage : "none" );

	if ( g_oTheApp.Create( "Blob07 - (c) Blue Tongue Software", argc, argv ) == TTRUE )
		g_oTheApp.Execute();

	g_oTheApp.Destroy();
	SteamManager::Unload();

	if ( hMutex ) ReleaseMutex( hMutex );

	// NOTE: like retail, nothing is destroyed: TUtil::ToshiDestroy is never called and ExitProcess skips the static
	// destructors
	ExitProcess( 0 );
}

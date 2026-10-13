#include "pch.h"
#include <Toshi/TUtil.h>
#include <Toshi/TSystem.h>
#include <Toshi/T2String8.h>
#include <Toshi/TRegion.h>

#ifdef TRENDERINTERFACE_DX11
#  include <Platform/DX11/TRender_DX11.h>
#endif

#include "AppBoot.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// NOTE: placeholder boot of the rebased engine. The retail boot (main 005768c0, AApplication::OnCreate 005747d0,
// ARenderer::CreateTRender 005f0780...) gets reconstructed in phase 4, see Docs/NewBranchPlan.md. For now it creates
// the DX11 render interface and its window and clears the screen every frame.

AApplication g_oTheApp;

TOSHI_NAMESPACE_USING

static TBOOL HasSwitch( const TCHAR* a_szSwitch )
{
	return T2String8::FindString( GetCommandLineA(), a_szSwitch ) != TNULL;
}

TBOOL AApplication::OnCreate( TINT argc, TCHAR** argv )
{
	TINFO( "Starting de Blob...\n" );

	if ( !TApplication::OnCreate( argc, argv ) )
		return TFALSE;

#ifdef TRENDERINTERFACE_DX11
	// Placeholder of ARenderer::CreateTRender 005f0780: windowed ("-windowed", "-sw") 1280x720, exclusive fullscreen
	// ("-exclusivefullscreen") or (default) a borderless window of the desktop's size. The width and height switches
	// and setup.ini (AApplication's command line parser 0056b6a0) aren't ported yet.
	TRenderDX11* pRender = new TRenderDX11();
	pRender->SetAutoCreateSystemResources( TFALSE );

	if ( !pRender->Create() )
		return TFALSE;

	TBOOL bWindowed   = HasSwitch( "-windowed" ) || HasSwitch( "-sw" );
	TBOOL bFullscreen = HasSwitch( "-exclusivefullscreen" );

	if ( bFullscreen ) bWindowed = TFALSE;

#  ifdef DEBLOB_COMMUNITY_PATCH
	// "-background" runs (see TMSWindow::IsBackgroundMode) are always windowed
	if ( TMSWindow::IsBackgroundMode() )
	{
		bWindowed   = TTRUE;
		bFullscreen = TFALSE;
	}
#  endif

	TRender::DISPLAYPARAMS oDisplayParams;
	oDisplayParams.uiWidth                   = TRenderDX11::MIN_WIDTH;
	oDisplayParams.uiHeight                  = TRenderDX11::MIN_HEIGHT;
	oDisplayParams.uiColourDepth             = 32;
	oDisplayParams.eDepthStencilFormat       = 3;
	oDisplayParams.bWindowed                 = bWindowed;
	oDisplayParams.bFullscreen               = bFullscreen;
	oDisplayParams.uiMultisampleQualityLevel = 1;

	if ( !pRender->CreateDisplay( oDisplayParams ) )
		return TFALSE;
#endif

	return TTRUE;
}

TBOOL AApplication::OnUpdate( TFLOAT a_fDeltaTime )
{
#ifdef TRENDERINTERFACE_DX11
	TRenderDX11* pRender = TRenderDX11::Interface();

	pRender->Update( a_fDeltaTime );

	// The window got WM_QUIT
	if ( pRender->GetMSWindow()->IsDestroyed() )
		return TFALSE;

	pRender->BeginScene();

	// Placeholder: a visible colour, so screenshots show that the frame went through
	const FLOAT aColour[ 4 ] = { 0.10f, 0.45f, 0.85f, 1.0f };
	pRender->GetDeviceContext()->ClearRenderTargetView( pRender->GetMainRenderTargetView(), aColour );

	pRender->EndScene();
#endif

	return TApplication::OnUpdate( a_fDeltaTime );
}

TBOOL AApplication::OnDestroy()
{
#ifdef TRENDERINTERFACE_DX11
	if ( TRenderDX11* pRender = TRenderDX11::Interface() )
	{
		pRender->Destroy();
		delete pRender;
	}
#endif

	return TApplication::OnDestroy();
}

// $deBlob: FUNCTION 005768c0
#ifdef TOSHI_FINAL
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, INT cmdShow )
{
	TINT    argc = __argc;
	TCHAR** argv = __argv;
#else  // TOSHI_FINAL
int main( int argc, TCHAR** argv )
{
#endif // !TOSHI_FINAL
	SetProcessDPIAware();
	TRegion_SetRegion( TREGION_UK );

	// NOTE: TUtil::ToshiCreate initialises the memory too, so nothing may allocate before main()
	TMemoryParams oMemoryParams;
	oMemoryParams.flags  = 0;
	oMemoryParams.uiSize = 640 * 1024 * 1024;
	TUtil::ToshiCreate( argc, argv, &oMemoryParams );

	TIMPLEMENT();

#if 0 // GHIDRA 005768c0: int __cdecl main(int argc, const char **argv, const char **envp)
  sub_65D610("Build Version %s", off_97B740[0]);
  memset(v15, 0, sizeof(v15));
  v15[0] = 284;
  ModuleHandleW = GetModuleHandleW(L"ntdll.dll");
  ProcAddress = GetProcAddress(ModuleHandleW, "RtlGetVersion");
  if ( ProcAddress != nullptr )
    ((void (__stdcall *)(_DWORD *))ProcAddress)(v15);
  v5 = 0;
  for ( i = 0; i < argc; v5 += v7 )
    v7 = sprintf(&ArgList[v5], "%s ", *(const char **)(v14 + 4 * i++));
  sub_65D610("Command Line: %s", (char)ArgList);
  v8 = sub_5767F0();
  sub_65D610("OS Name: %s", v8);
  sub_65D610("OS Version: %d.%d Build:%d %s", v15[1]);
#endif

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

#if 0 // GHIDRA 005768c0: (continued)
    if ( !IsDebuggerPresent() )
      SetUnhandledExceptionFilter(TopLevelExceptionFilter);
    sub_619F50();
    (... Steam: the SteamManager instance has to exist and init, else "Failed to init steam" ...)
    if ( (unsigned __int8)sub_61A020(dword_9A1EE0) != 0 )
    {
      v11 = sub_61A0C0(dword_9A1EE0);
      v12 = "none";
      if ( v11 != 0 )
        v12 = (const char *)v11;
      printf("Application language = %s\n", v12);
      if ( (unsigned __int8)sub_6EF1A0("Blob07 - (c) Blue Tongue Software", argc, v14) == 1 )
        sub_6EF100(&dword_990938);
      sub_6EF210(&dword_990938);
      sub_619FB0();
      ReleaseMutex(MutexA);
      ExitProcess(0);
    }
    MessageBoxA(nullptr, "Failed to init steam", "de Blob", 0);
    return -1;
#endif

	// NOTE: placeholder, retail creates the application as "Blob07 - (c) Blue Tongue Software"
	if ( g_oTheApp.Create( "de Blob", argc, argv ) )
		g_oTheApp.Execute();

	g_oTheApp.Destroy();

	if ( hMutex ) ReleaseMutex( hMutex );

	// NOTE: like retail, nothing is destroyed: TUtil::ToshiDestroy is never called and ExitProcess skips the static
	// destructors
	ExitProcess( 0 );
}

#include "ToshiPCH.h"
#include "TMSWindow_DX11.h"
#include "TRender_DX11.h"
#include "Toshi/TApplication.h"
#include "Toshi/TSystem.h"
#include "Toshi/T2String8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: de Blob's TMSWindow is a TObject (vftable ??_7TMSWindow@Toshi@@6B@)
TDEFINE_CLASS( TMSWindow );

// $deBlob: FUNCTION 006b09f0
TMSWindow::TMSWindow()
{
	m_hWnd            = NULL;
	m_pRender         = TNULL;
	m_bIsWindowed     = TTRUE;
	m_bIsDestroyed    = TFALSE;
	m_bIsFocused      = TFALSE;
	m_bCursorHidden   = TFALSE;
	m_iMouseX         = 0;
	m_iMouseY         = 0;
	m_bIsPopup        = TTRUE;
	m_bCloseRequested = TFALSE;
	// NOTE: retail doesn't initialise m_bTrackingMouse (0x20)
	m_bTrackingMouse = TFALSE;

	Destroy();

#ifdef DEBLOB_COMMUNITY_PATCH
	// Background test runs don't touch the user's sticky keys setting (several runs at once would mix it up)
	if ( IsBackgroundMode() ) return;
#endif

	// Turns the sticky keys off while the game runs (restored when it loses the focus)
	ms_StickyKeys.cbSize = sizeof( STICKYKEYS );
	SystemParametersInfoA( SPI_GETSTICKYKEYS, sizeof( STICKYKEYS ), &ms_StickyKeys, 0 );

	STICKYKEYS oNewStickyKeys = { ms_StickyKeys.cbSize, 0 };
	SystemParametersInfoA( SPI_SETSTICKYKEYS, ms_StickyKeys.cbSize, &oNewStickyKeys, 0 );
}

// $deBlob: FUNCTION 006b0a80
TMSWindow::~TMSWindow()
{
}

#ifdef DEBLOB_COMMUNITY_PATCH

TBOOL TMSWindow::IsBackgroundMode()
{
	static const TBOOL s_bBackground = T2String8::FindString( GetCommandLineA(), "-background" ) != TNULL;
	return s_bBackground;
}

TBOOL TMSWindow::IsNoPauseMode()
{
	static const TBOOL s_bNoPause = T2String8::FindString( GetCommandLineA(), "-nopause" ) != TNULL;
	return s_bNoPause;
}

#endif // DEBLOB_COMMUNITY_PATCH

// $deBlob: FUNCTION 006b0a90
TBOOL TMSWindow::Create( TRenderDX11* a_pRender, LPCSTR a_szTitle )
{
	m_hModule = GetModuleHandleA( NULL );
	Destroy();

	m_pRender = a_pRender;

	// NOTE: the window class is named after the render interface's class ("TRenderDX11")
	const TCHAR* szClassName = TGetClass( TRenderDX11 ).GetName();

	WNDCLASSA oWndClass     = {};
	oWndClass.style         = CS_VREDRAW | CS_HREDRAW;
	oWndClass.lpfnWndProc   = WndProc;
	oWndClass.cbClsExtra    = 0;
	oWndClass.cbWndExtra    = 4;
	oWndClass.hInstance     = m_hModule;
	// NOTE: icon resource 103 of the exe (retail first loads IDI_APPLICATION and overwrites it)
	oWndClass.hIcon         = LoadIconA( GetModuleHandleA( NULL ), MAKEINTRESOURCEA( 103 ) );
	oWndClass.hCursor       = LoadCursorA( NULL, IDC_ARROW );
	oWndClass.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	oWndClass.lpszMenuName  = NULL;
	oWndClass.lpszClassName = szClassName;
	RegisterClassA( &oWndClass );

	DWORD uiStyle   = m_bIsPopup ? s_uiPopupStyles : 0;
	DWORD uiExStyle = 0;

#ifdef DEBLOB_COMMUNITY_PATCH
	// Not in the taskbar/alt-tab and never activated by clicks
	if ( IsBackgroundMode() ) uiExStyle = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
#endif

	m_hWnd = CreateWindowExA( uiExStyle, szClassName, a_szTitle, uiStyle, 100, 100, 0, 0, NULL, NULL, m_hModule, this );

	if ( m_hWnd == NULL )
		return TFALSE;

	EnableWindow( m_hWnd, TRUE );

#ifdef DEBLOB_COMMUNITY_PATCH
	if ( IsBackgroundMode() )
	{
		// Move it out of the desktop before it's shown, and don't take the focus
		SetWindowPos( m_hWnd, HWND_BOTTOM, GetSystemMetrics( SM_XVIRTUALSCREEN ) - 4096, GetSystemMetrics( SM_YVIRTUALSCREEN ) - 4096, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
		ShowWindow( m_hWnd, SW_SHOWNOACTIVATE );
		return TTRUE;
	}
#endif

	ShowWindow( m_hWnd, SW_SHOW );
	SetForegroundWindow( m_hWnd );

	if ( GetForegroundWindow() != m_hWnd )
	{
		TUtil::Log( "Not foreground window, Pausing Systems!\n" );

#ifdef DEBLOB_COMMUNITY_PATCH
		if ( !IsNoPauseMode() )
#endif
			TSystemManager::GetSingleton()->Pause( TTRUE );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006b0c00
void TMSWindow::Destroy()
{
	if ( m_hWnd != NULL )
	{
		if ( ms_bIsFocused )
		{
			SetThreadExecutionState( ES_CONTINUOUS );
			ShowCursor( TRUE );
			TSystemManager::GetSingleton()->Pause( TTRUE );
			ms_bIsFocused = TFALSE;
			SystemParametersInfoA( SPI_SETSTICKYKEYS, ms_StickyKeys.cbSize, &ms_StickyKeys, 0 );
		}

		DestroyWindow( m_hWnd );
		m_hWnd         = NULL;
		m_bIsDestroyed = TTRUE;
	}

	if ( m_pRender != TNULL )
	{
		UnregisterClassA( TGetClass( TRenderDX11 ).GetName(), m_hModule );
		m_pRender = TNULL;
	}
}

// $deBlob: FUNCTION 006b0ca0
void TMSWindow::SetPosition( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight )
{
	RECT oRect;
	oRect.left   = 0;
	oRect.top    = 0;
	oRect.right  = a_iWidth;
	oRect.bottom = a_iHeight;

	AdjustWindowRect( &oRect, m_bIsPopup ? s_uiPopupStylesAdjust : 0, FALSE );

#ifdef DEBLOB_COMMUNITY_PATCH
	if ( IsBackgroundMode() )
	{
		// Above and to the left of the whole virtual desktop (all monitors), so it never covers anything
		const TINT iWidth  = oRect.right - oRect.left;
		const TINT iHeight = oRect.bottom - oRect.top;
		const TINT iX      = GetSystemMetrics( SM_XVIRTUALSCREEN ) - iWidth - 256;
		const TINT iY      = GetSystemMetrics( SM_YVIRTUALSCREEN ) - iHeight - 256;
		SetWindowPos( m_hWnd, HWND_BOTTOM, iX, iY, iWidth, iHeight, SWP_NOACTIVATE );
		return;
	}
#endif

	SetWindowPos( m_hWnd, HWND_TOP, a_iX, a_iY, oRect.right - oRect.left, oRect.bottom - oRect.top, 0 );
}

// $deBlob: FUNCTION 006b0d20
void TMSWindow::SetWindowed()
{
	TASSERT( GetHWND() != TNULL );
	m_bIsWindowed = TTRUE;
}

// $deBlob: FUNCTION 006b0d60
void TMSWindow::SetFullscreen()
{
	TASSERT( GetHWND() != TNULL );
	m_bIsWindowed = TFALSE;
}

// $deBlob: FUNCTION 006b0da0
void TMSWindow::Update()
{
	MSG oMsg;

	if ( !m_bIsDestroyed && PeekMessageA( &oMsg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		while ( GetMessageA( &oMsg, NULL, 0, 0 ) )
		{
			TranslateMessage( &oMsg );
			DispatchMessageA( &oMsg );

			if ( !PeekMessageA( &oMsg, NULL, 0, 0, PM_NOREMOVE ) )
				return;
		}

		// WM_QUIT
		m_bIsDestroyed = TTRUE;
	}
}

// $deBlob: FUNCTION 006b0e20
void TMSWindow::SetFocused( TBOOL a_bFocused )
{
#ifdef DEBLOB_COMMUNITY_PATCH
	if ( IsBackgroundMode() ) return;
#endif

	if ( a_bFocused != ms_bIsFocused )
	{
		SetThreadExecutionState( a_bFocused ? ( ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED ) : ES_CONTINUOUS );
		ShowCursor( !a_bFocused );
		TSystemManager::GetSingleton()->Pause( !a_bFocused );
		ms_bIsFocused = a_bFocused;

		if ( a_bFocused )
		{
			SystemParametersInfoA( SPI_GETSTICKYKEYS, ms_StickyKeys.cbSize, &ms_StickyKeys, 0 );
			STICKYKEYS oNewStickyKeys = { ms_StickyKeys.cbSize, 0 };
			SystemParametersInfoA( SPI_SETSTICKYKEYS, ms_StickyKeys.cbSize, &oNewStickyKeys, 0 );
		}
		else
		{
			SystemParametersInfoA( SPI_SETSTICKYKEYS, ms_StickyKeys.cbSize, &ms_StickyKeys, 0 );
		}
	}
}

// $deBlob: FUNCTION 006b0ef0
LRESULT CALLBACK TMSWindow::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// TODO: retail null-checks TSystemManager::ms_pSingleton (now a TSingleton like in retail)
	TMSWindow*   pWindow = TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) );
	TRenderDX11* pRender = TRenderDX11::Interface();

	// Windowed in the size of the desktop (neither "-windowed" nor exclusive fullscreen): the cursor is kept in the window
	TBOOL bLockCursor = pWindow != TNULL && !pRender->GetCurrentDisplayParams()->bFullscreen && !pRender->GetCurrentDisplayParams()->bWindowed;
	TBOOL bFullscreen = pWindow != TNULL && pRender->GetCurrentDisplayParams()->bFullscreen && !pRender->GetCurrentDisplayParams()->bWindowed;

#ifdef DEBLOB_COMMUNITY_PATCH
	// Background test runs never clip the user's cursor
	if ( IsBackgroundMode() ) bLockCursor = bFullscreen = TFALSE;
#endif

	RECT oRect;

	if ( uMsg > WM_ACTIVATEAPP )
	{
		if ( uMsg < WM_LBUTTONDOWN )
		{
			if ( uMsg == WM_MOUSEMOVE )
			{
				TMSWindow* pMouseWindow = TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) );

				if ( pMouseWindow->m_bIsWindowed )
				{
					if ( !pMouseWindow->m_bCursorHidden )
					{
						ShowCursor( FALSE );
						pMouseWindow->m_bCursorHidden = TTRUE;

						if ( !pMouseWindow->m_bTrackingMouse )
						{
							TRACKMOUSEEVENT oTrackMouseEvent;
							oTrackMouseEvent.cbSize    = sizeof( TRACKMOUSEEVENT );
							oTrackMouseEvent.dwFlags   = TME_LEAVE;
							oTrackMouseEvent.hwndTrack = hWnd;

							if ( TrackMouseEvent( &oTrackMouseEvent ) )
								pMouseWindow->m_bTrackingMouse = TTRUE;
						}
					}

					pMouseWindow->m_iMouseX = lParam & 0xFFFF;
					pMouseWindow->m_iMouseY = TUINT32( lParam ) >> 16;
				}

				return 1;
			}

			if ( uMsg == WM_SETCURSOR )
			{
				if ( LOWORD( lParam ) == HTCLIENT )
				{
					SetCursor( NULL );
					return 1;
				}

				return DefWindowProcA( hWnd, uMsg, wParam, lParam );
			}

			if ( uMsg == WM_SYSCOMMAND )
			{
				if ( wParam == SC_CLOSE )
				{
					TMSWindow* pCloseWindow = TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) );

					if ( pCloseWindow != TNULL && pCloseWindow->m_bIsPopup )
					{
						pCloseWindow->m_bCloseRequested = TTRUE;

						TApplicationExitEvent oEvent;
						oEvent.m_bLockedCursor = bLockCursor;
						TGlobalEmitter<TApplicationExitEvent>::Throw( oEvent );
					}

					return 0;
				}

				if ( wParam == SC_TASKLIST || wParam == SC_MINIMIZE )
				{
					TMSWindow* pMinWindow = TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) );

					if ( pMinWindow->m_bIsWindowed )
						return ShowWindow( hWnd, SW_MINIMIZE );

					return 0;
				}
			}
		}
		else
		{
			if ( uMsg == WM_DEVICECHANGE )
			{
				if ( wParam == DBT_DEVNODES_CHANGED )
				{
					// TInputDXInterface::LostDevice( TInputInterface::s_Instance )
					TTODO( "TInputDXInterface::LostDevice 00680030" );
				}

				return 1;
			}

			if ( uMsg == WM_MOUSELEAVE )
			{
				TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) )->m_bCursorHidden  = TFALSE;
				TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) )->m_bTrackingMouse = TFALSE;
				ShowCursor( TRUE );
				return 1;
			}
		}

		return DefWindowProcA( hWnd, uMsg, wParam, lParam );
	}

	if ( uMsg == WM_ACTIVATEAPP )
	{
		if ( bFullscreen || bLockCursor )
		{
			GetWindowRect( hWnd, &oRect );
			ClipCursor( &oRect );
		}

		if ( TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) )->m_bIsDestroyed )
			return 0;

#ifdef DEBLOB_COMMUNITY_PATCH
		// Background test runs keep running while another application is active
		if ( IsBackgroundMode() || IsNoPauseMode() ) return 0;
#endif

		// NOTE: retail has separate branches for windowed and fullscreen that do the same
		TSystemManager::GetSingleton()->Pause( wParam != TRUE );
		return 0;
	}

	switch ( uMsg )
	{
		case WM_CREATE:
		{
			SetWindowLongA( hWnd, GWL_USERDATA, *TREINTERPRETCAST( LONG*, lParam ) );

			DEV_BROADCAST_DEVICEINTERFACE_A oNotificationFilter = {};
			oNotificationFilter.dbcc_size                       = sizeof( DEV_BROADCAST_DEVICEINTERFACE_A );
			oNotificationFilter.dbcc_devicetype                 = DBT_DEVTYP_DEVICEINTERFACE;
			oNotificationFilter.dbcc_classguid                  = WceusbshGUID;

			ms_hDeviceNotify = RegisterDeviceNotificationA( hWnd, &oNotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE );

			if ( ms_hDeviceNotify == NULL )
				ExitProcess( 1 );

			return 0;
		}

		case WM_SIZE:
			if ( bFullscreen || bLockCursor )
			{
				GetWindowRect( hWnd, &oRect );
				ClipCursor( &oRect );
			}

			break;

		case WM_ACTIVATE:
#ifdef DEBLOB_COMMUNITY_PATCH
			// Background test runs are never focused: keep the user's sticky keys, cursor and the game running
			if ( IsBackgroundMode() ) break;
#endif

			if ( wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE )
			{
				if ( !ms_bIsFocused )
				{
					SetThreadExecutionState( ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED );
					ShowCursor( FALSE );
					TSystemManager::GetSingleton()->Pause( TFALSE );
					ms_bIsFocused = TTRUE;

					SystemParametersInfoA( SPI_GETSTICKYKEYS, ms_StickyKeys.cbSize, &ms_StickyKeys, 0 );
					STICKYKEYS oNewStickyKeys = { ms_StickyKeys.cbSize, 0 };
					SystemParametersInfoA( SPI_SETSTICKYKEYS, ms_StickyKeys.cbSize, &oNewStickyKeys, 0 );
				}

				if ( pRender != TNULL && pRender->GetSwapChain() != TNULL && ms_bIsFullscreen )
				{
					TUtil::Log( "###### Restore fullscreen" );
					ms_bIsFullscreen = FALSE;
					pRender->GetSwapChain()->SetFullscreenState( TRUE, NULL );
				}
			}
			else
			{
				if ( pRender != TNULL && pRender->GetSwapChain() != TNULL )
				{
					pRender->GetSwapChain()->GetFullscreenState( &ms_bIsFullscreen, NULL );

					if ( pRender->GetCurrentDisplayParams()->bFullscreen && ms_bIsFullscreen )
					{
						TUtil::Log( "#####  Disable fullscreen" );
						pRender->GetSwapChain()->SetFullscreenState( FALSE, NULL );
					}
				}

				if ( ms_bIsFocused )
				{
					SetThreadExecutionState( ES_CONTINUOUS );
					ShowCursor( TRUE );

#ifdef DEBLOB_COMMUNITY_PATCH
					if ( !IsNoPauseMode() )
#endif
						TSystemManager::GetSingleton()->Pause( TTRUE );

					ms_bIsFocused = TFALSE;
					SystemParametersInfoA( SPI_SETSTICKYKEYS, ms_StickyKeys.cbSize, &ms_StickyKeys, 0 );
				}
			}

			break;

		case WM_SETFOCUS:
			TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) )->m_bIsFocused = TTRUE;
			ShowCursor( FALSE );
			TSystemManager::GetSingleton()->Pause( TFALSE );
			return 1;

		case WM_KILLFOCUS:
			TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) )->m_bIsFocused = TFALSE;
			ShowCursor( TRUE );

#ifdef DEBLOB_COMMUNITY_PATCH
			if ( IsBackgroundMode() || IsNoPauseMode() ) return 1;
#endif

			TSystemManager::GetSingleton()->Pause( TTRUE );

			if ( bLockCursor )
			{
				TUtil::Log( "Minimize window\n" );
				ShowWindow( hWnd, SW_MINIMIZE );
			}

			return 1;
	}

	return DefWindowProcA( hWnd, uMsg, wParam, lParam );
}

TOSHI_NAMESPACE_END

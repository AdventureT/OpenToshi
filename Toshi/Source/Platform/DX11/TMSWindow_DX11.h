#pragma once
#include "Toshi/TObject.h"

#include <dbt.h>
#include <windowsx.h>

TOSHI_NAMESPACE_START

class TRenderDX11;
class TRender;

//-----------------------------------------------------------------------------
// The window of TRenderDX11 (embedded at TRenderDX11+0x6B0), size 0x28.
//-----------------------------------------------------------------------------
class TMSWindow : public TObject
{
public:
	TDECLARE_CLASS( TMSWindow, TObject );

public:
	// Styles of the popup window (the window class style is set by TRenderDX11::CreateDisplay)
	static TCONSTEXPR_VAR DWORD s_uiPopupStyles       = 0x80ca0000; // WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
	static TCONSTEXPR_VAR DWORD s_uiPopupStylesAdjust = 0x80c80000; // WS_POPUP | WS_CAPTION | WS_SYSMENU

public:
	TMSWindow();
	~TMSWindow();

	TBOOL Create( TRenderDX11* a_pRender, LPCSTR a_szTitle );
	void  Destroy();
	void  Update();

	void SetPosition( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight );
	void SetWindowed();
	void SetFullscreen();

	static void SetFocused( TBOOL a_bFocused );

	TBOOL IsDestroyed() const { return m_bIsDestroyed; }
	TBOOL IsWindowed() const { return m_bIsWindowed; }
	TBOOL IsPopup() const { return m_bIsPopup; }
	TBOOL IsCursorHidden() const { return m_bCursorHidden; }
	HWND  GetHWND() const { return m_hWnd; }

	// $deBlob: FUNCTION 0067fbb0
	void GetMousePosition( TINT& a_rX, TINT& a_rY ) const
	{
		a_rX = m_iMouseX;
		a_rY = m_iMouseY;
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	// Development: "-background" keeps the window of unattended test runs out of the user's way.
	// The window is placed outside of the desktop, never activated or focused and not shown in the taskbar,
	// it doesn't touch the user's mouse cursor or sticky keys, and the game keeps running (no pause) while it
	// isn't the active application. Background runs are always windowed.
	static TBOOL IsBackgroundMode();

	// Development: "-nopause" keeps a visible game running while it isn't the active application (watched test runs)
	static TBOOL IsNoPauseMode();
#endif

private:
	static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
	// GUID_DEVINTERFACE_USB_DEVICE (WCEUSBSH), registered for WM_DEVICECHANGE in WM_CREATE
	static const GUID WceusbshGUID;

	static TBOOL      ms_bIsFocused;
	static STICKYKEYS ms_StickyKeys;    // the user's setting, restored when the game loses the focus
	static BOOL       ms_bIsFullscreen; // swap chain fullscreen state while deactivated
	static HDEVNOTIFY ms_hDeviceNotify;

private:
	HWND         m_hWnd;            // 0x04
	TRenderDX11* m_pRender;         // 0x08
	TBOOL        m_bIsWindowed;     // 0x0C
	TBOOL        m_bIsDestroyed;    // 0x0D
	TBOOL        m_bIsFocused;      // 0x0E
	HMODULE      m_hModule;         // 0x10
	TBOOL        m_bCursorHidden;   // 0x14 hidden while the mouse is over the (windowed) window
	TINT         m_iMouseX;         // 0x18
	TINT         m_iMouseY;         // 0x1C
	TBOOL        m_bTrackingMouse;  // 0x20 TrackMouseEvent( TME_LEAVE ) requested
	TBOOL        m_bIsPopup;        // 0x21
	TBOOL        m_bCloseRequested; // 0x22 SC_CLOSE
};

TOSHI_NAMESPACE_END

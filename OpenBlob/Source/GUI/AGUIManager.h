#pragma once
#include "AStack.h"

#include <Toshi/TSingleton.h>
#include <GUI/T2GUIElement.h>

class AGUICursor;
class ASafeZone;

//-----------------------------------------------------------------------------
// Purpose: the game's GUI: the safe area element the game's T2GUI elements
// are added to, the safe zone frame, the stack of T2GUI visibility masks (T2GUIElement::s_uiGlobalVisMask) and the
// mouse cursor of the menus (shown while the mouse is used, hidden after 3 seconds or when a key/button is pressed).
//-----------------------------------------------------------------------------
class AGUIManager : public Toshi::TSingleton<AGUIManager>
{
public:
	AGUIManager();
	~AGUIManager();

	// Resets the cursor
	void Reinitialize();

	void Tick( TFLOAT a_fDeltaTime );

	void CreateSafeArea();

	// $deBlob: FUNCTION 005d6800
	Toshi::T2GUIElement* GetSafeArea() const { return m_pSafeArea; }
	// $deBlob: FUNCTION 005d6810
	ASafeZone* GetSafeZone() const { return m_pSafeZone; }

	// The visibility masks
	TUINT32 GetGUIMask();
	void    SetGUIMask( TUINT32 a_uiMask );
	void    RemoveCurrentGUIMask();

	// Shows/activates the cursor if a_iCount > 0
	void SetCursorVisibleCount( TINT a_iCount );
	void SetCursorActiveCount( TINT a_iCount );

	// Enables the cursor (the menus that use it)
	void SetCursorVisible( TBOOL a_bVisible, TBOOL a_bUnused );
	void SetCursorActive( TBOOL a_bActive );
	void SetCursorEnabled( TBOOL a_bEnabled );

	// The visibility mask of the cursor: 4 while it's visible
	TUINT32 GetCursorMask();

	// Shows/hides the safe zone frame (a debug toggle)
	static TBOOL ToggleSafeZone( void* a_pUnused );

	AGUICursor* GetCursor() const { return m_pCursor; }

private:
	AGUICursor*          m_pCursor;        // 0x00
	Toshi::T2GUIElement* m_pSafeArea;      // 0x04 in T2GUI's root
	AStack<TUINT32, 10>  m_GUIMasks;       // 0x08
	ASafeZone*           m_pSafeZone;      // 0x34 in the safe area
	TFLOAT               m_fCursorTime;    // 0x38 since the mouse was last used
	TFLOAT               m_fHideDelay;     // 0x3C -1: none
	TINT                 m_iLastMouseX;    // 0x40
	TINT                 m_iLastMouseY;    // 0x44
	TBOOL                m_bCursorEnabled; // 0x48
	TBOOL                m_bCursorShown;   // 0x49
};

TSTATICASSERT( sizeof( AGUIManager ) == 0x4C );

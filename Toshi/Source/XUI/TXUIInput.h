#pragma once
#include "Toshi/TEvent.h"
#include "Input/TInputInterface.h"

TOSHI_NAMESPACE_START

class TXUIElement;

//-----------------------------------------------------------------------------
// Turns the input events of TInputInterface into XUI button presses (0x28 bytes, a TGenericListener connected to
// TInputInterface's event emitter).
//-----------------------------------------------------------------------------
class TXUIInput : public TGenericListener
{
public:
	static TCONSTEXPR_VAR TINT MAXCONTROLLERS = 4;

public:
	TXUIInput();
	~TXUIInput();

	TBOOL Tick( TFLOAT a_fDeltaTime );

	// NOTE: retail takes the controller (RET 4) but doesn't use it
	void PlayClickAnimation( TINT a_iController );

	// Inlined into TXUI (TransitionTo, TransitionBack, SetFocussedScene, Reset...)
	void ResetRepeat() { m_pRepeatElement = TNULL; }

	TXUIElement* GetRepeatElement() const { return m_pRepeatElement; }

	// Inlined into TXUI::SetDefaultController
	void  SetEnabled( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }
	TBOOL IsEnabled() const { return m_bEnabled; }

	// The game's cursor (AGUICursor) sets these
	void SetCursorVisible( TBOOL a_bValue ) { m_bCursorVisible = a_bValue; }
	// $deBlob: FUNCTION 005d7670
	void SetClickElement( TXUIElement* a_pElement ) { m_pClickElement = a_pElement; }
	void SetRepeatElement( TXUIElement* a_pElement ) { m_pRepeatElement = a_pElement; }

	TINT GetMouseX() const { return m_iMouseX; }
	TINT GetMouseY() const { return m_iMouseY; }

	static TBOOL __stdcall OnInputEvent( void* a_pCaller, void* a_pOwner, void* a_pData );

private:
	TBOOL ProcessInputEvent( TInputInterface* a_pInputInterface, TInputInterface::InputEvent* a_pEvent );

private:
	TXUIElement* m_pRepeatElement; // 0x14 the control under the mouse cursor (ProcessInputEvent); a click presses it
	TBOOL        m_bCursorVisible; // 0x18 TTRUE; set by AGUICursor
	TBOOL        m_bEnabled;       // 0x19
	TXUIElement* m_pClickElement;  // 0x1C plays "Click"/"EndClick"
	TINT         m_iMouseX;        // 0x20 the mouse position (passed to HandleAnalog)
	TINT         m_iMouseY;        // 0x24
};

TOSHI_NAMESPACE_END

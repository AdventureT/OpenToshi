#pragma once
#include "XUI/ABlobMenu.h"
#include "MemoryCard/EnSaveData.h"

namespace Toshi
{
class TInputDevice;
}

//-----------------------------------------------------------------------------
// Purpose: the title screen (splash.xur): shows "press start"
// over the title movie and waits for a key/button; then creates the profile of the first save slot if it doesn't
// exist yet and goes to the main menu (mainmenu.xur). Back asks to quit the game.
//-----------------------------------------------------------------------------
class ABlobSplash : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobSplash, ABlobMenu );

	// The save slot of the profile
	static TINT s_iSaveSlot;

	// The nunchuk popup was shown
	static TBOOL s_bNunchukPopupShown;

public:
	ABlobSplash();
	~ABlobSplash();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;     // +0x114
	virtual void TransBackTo() override; // +0x11C
	virtual void EndTransTo() override;  // +0x124

	//-----------------------------------------------------------------------------
	// ABlobMenu
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0063b0e0
	virtual void OnAttractModeUpdate() override { ABlobMenu::OnAttractModeUpdate(); } // +0x134
	virtual void OnConfirmationClosed( TBOOL a_bYes ) override;                       // +0x138

	// SaveLoad's callback at the end of the save of the new profile
	void OnProfileSaved( TINT a_iResult );

private:
	// Creates the profile of s_iSaveSlot and saves it
	void CreateProfile();

	static TBOOL IsNunchukConnected( Toshi::TInputDevice* a_pDevice );

private:
	TBOOL                    m_bStartPressed;   // 0x140
	TBOOL                    m_bReady;          // 0x141 "Show" is over, the input is taken
	Toshi::TXUIElement*      m_pElement1;       // 0x144 the element with the UID 1
	Toshi::TXUIElement*      m_pElement2;       // 0x148 the element with the UID 2 (plays "Incorrect")
	TFLOAT                   m_fStartTimer;     // 0x14C
	TUINT32                  m_uiUnk150;        // 0x150
	TFLOAT                   m_fUnk154;         // 0x154
	TFLOAT                   m_fIncorrectTimer; // 0x158
	TBOOL                    m_bShown;          // 0x15C
	TBOOL                    m_bUnk15D;         // 0x15D
	TBOOL                    m_bSaving;         // 0x15E
	TBOOL                    m_bProfileCreated; // 0x15F
	EnSaveData::SaveGameSlot m_oSlotBackup;     // 0x160 the slot before the new profile
	TBOOL                    m_bPopup;          // 0x478 the nunchuk popup is shown
	Toshi::TXUIScene*        m_pPopup;          // 0x47C
	Toshi::TXUIElement*      m_pLabel;          // 0x480 XuiLabel1
	TWCHAR                   m_wszLabel[ 256 ]; // 0x484
};

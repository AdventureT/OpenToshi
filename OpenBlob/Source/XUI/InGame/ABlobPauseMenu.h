#pragma once
#include "XUI/ABlobMenu.h"

class ABlobConfirmationMenu;

//-----------------------------------------------------------------------------
// Purpose: the pause menu of a level (pausemenu.xur, mppausemenu.xur for
// multiplayer): Resume, Options, Quit, Challenge and Reset, each of the last three opens a ABlobConfirmationMenu.
// The pause menu is a child of ABlobInGame, which holds the game while it is shown.
//-----------------------------------------------------------------------------
class ABlobPauseMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobPauseMenu, ABlobMenu );

	// What the confirmation popup answers (m_iConfirmAction)
	enum CONFIRM : TINT
	{
		CONFIRM_None      = -1,
		CONFIRM_Quit      = 0,
		CONFIRM_Reset     = 1,
		CONFIRM_Challenge = 2,
	};

public:
	ABlobPauseMenu();
	~ABlobPauseMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                        // +0x04
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override;                 // +0xF8

	//-----------------------------------------------------------------------------
	// ABlobMenu
	//-----------------------------------------------------------------------------
	virtual void OnConfirmationClosed( TBOOL a_bYes ) override; // +0x138

	// Pauses the game and shows the menu
	void Pause();
	// Hides the menu and restores the game after it closed
	void Unpause();
	// Leaves the pause menu: the game goes on
	void Resume();

	// Saves the default controller and the controller state, restores them after the pause
	void SaveDefaultController();
	void RestoreDefaultController();

private:
	// Creates the confirmation popup and parents it to this menu's scene
	ABlobConfirmationMenu* CreateConfirmation();
	// Shows m_pConfirmation over this menu
	void ShowConfirmation();
	// Whether the Challenge option can be started
	TBOOL IsChallengeAvailable() const;

	Toshi::TXUIControl*    m_pResume;          // 0x140 "Resume"
	Toshi::TXUIControl*    m_pOptions;         // 0x144 "Options"
	Toshi::TXUIControl*    m_pQuit;            // 0x148 "Quit"
	Toshi::TXUIControl*    m_pChallenge;       // 0x14C "Challenge" (not in multiplayer)
	Toshi::TXUIControl*    m_pReset;           // 0x150 "Reset"
	TUINT32                m_uiCursorMask;     // 0x154 the cursor mask of AGUIManager while paused
	TBOOL                  m_bFadeEnabled;     // 0x158 the fade state before the pause
	TFLOAT                 m_fFade;            // 0x15C the fade before the pause
	TINT                   m_iConfirmAction;   // 0x160 CONFIRM
	TBOOL                  m_bSoundStateSaved; // 0x164 the sound state saved on pause
	ABlobConfirmationMenu* m_pConfirmation;    // 0x168 the "Confirmation" popup (confirmation.xur)
	void*                  m_pSavedController; // 0x16C the saved default controller state (56 bytes)
	TINT                   m_iController;      // 0x170 the controller the menu was opened with
};

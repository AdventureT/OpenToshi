#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the main menu (mainmenu.xur): Story, Free Paint,
// Blob Party, Options, Extras and Quit; the "New" labels of the modes; back goes to the title screen.
//-----------------------------------------------------------------------------
class ABlobMainMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobMainMenu, ABlobMenu );

public:
	ABlobMainMenu();
	~ABlobMainMenu();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;    // +0x04
	virtual void SetVisible( TBOOL a_bVisible ) override; // +0x48

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;        // +0x114
	virtual void TransBackTo() override;    // +0x11C
	virtual void EndTransTo() override;     // +0x124
	virtual void EndTransBackTo() override; // +0x12C

	//-----------------------------------------------------------------------------
	// ABlobMenu
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 00636920
	virtual void OnAttractModeUpdate() override { ABlobMenu::OnAttractModeUpdate(); } // +0x134
	virtual void OnConfirmationClosed( TBOOL a_bYes ) override;                       // +0x138
	virtual void EndAttractMode() override;                                           // +0x140

private:
	void ToggleDebugButton();
	void ShowCursor();
	void PlayMenuMovie();

private:
	Toshi::TXUIControl* m_pStory;        // 0x140 the control with the UID 6
	Toshi::TXUIControl* m_pFreePaint;    // 0x144 UID 9
	Toshi::TXUIControl* m_pBlobParty;    // 0x148 UID 8
	Toshi::TXUIControl* m_pExtras;       // 0x14C UID 5
	Toshi::TXUIControl* m_pQuit;         // 0x150 UID 14
	Toshi::TXUIControl* m_pFreePaintNew; // 0x154 UID 11
	Toshi::TXUIControl* m_pExtrasNew;    // 0x158 UID 10
	Toshi::TXUIControl* m_pBlobPartyNew; // 0x15C UID 12
	Toshi::TXUIControl* m_pDebug;        // 0x160 UID 3, hidden
	Toshi::TXUIControl* m_pOptions;      // 0x164 UID 7
	TBOOL               m_bBeatScene;    // 0x168
	TBOOL               m_bFadingIn;     // 0x169
};

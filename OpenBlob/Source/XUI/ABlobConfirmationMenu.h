#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the yes/no popup (confirmation.xur): a title and a text, the answer goes to the menu that opened it
// (ABlobMenu::OnConfirmationClosed). It can fade to black before answering yes (m_bFadeOnYes).
//-----------------------------------------------------------------------------
class ABlobConfirmationMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobConfirmationMenu, ABlobMenu );

public:
	ABlobConfirmationMenu();
	~ABlobConfirmationMenu();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	// $deBlob: FUNCTION 00630410
	void SetParentMenu( ABlobMenu* a_pMenu ) { m_pParentMenu = a_pMenu; }
	void SetFadeOnYes( TBOOL a_bFade ) { m_bFadeOnYes = a_bFade; }
	void SetCloseOnYes( TBOOL a_bClose ) { m_bCloseOnYes = a_bClose; }

	void SetTitle( const TWCHAR* a_wszTitle );
	void SetText( const TWCHAR* a_wszText );

	// Only the "no" button, labelled "OK" (a message)
	void SetMessageOnly();

	// Hides the UID 1 element (m_pElement1), the pause menu does it for every popup it opens
	void HideElement1() { m_pElement1->SetVisible( TFALSE ); }

private:
	Toshi::TXUIControl* m_pYes;        // 0x140 UID 5
	Toshi::TXUIControl* m_pNo;         // 0x144 UID 4
	ABlobMenu*          m_pParentMenu; // 0x148 gets the answer
	Toshi::TXUIControl* m_pTitle;      // 0x14C UID 6
	Toshi::TXUIControl* m_pText;       // 0x150 UID 7
	TBOOL               m_bFadeOnYes;  // 0x154 yes fades to black first, then answers
	TBOOL               m_bCloseOnYes; // 0x155 yes transitions back from the popup (TTRUE by default)
	TBOOL               m_bFading;     // 0x156
	TXUIElement*        m_pElement1;   // 0x158 UID 1 (not used)
};

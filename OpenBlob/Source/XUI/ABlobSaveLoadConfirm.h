#pragma once
#include <XUI/TXUIScene.h>

class ABlobMenu;

//-----------------------------------------------------------------------------
// Purpose: the save/load confirmation popup (saveloadconfirm*.xur, the "Confirm" control): button 0
// answers yes, button 1 no (only when the popup has a "Cancel" control), the answer goes to the menu that opened it
// (ABlobMenu::OnConfirmationClosed).
//-----------------------------------------------------------------------------
class ABlobSaveLoadConfirm : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobSaveLoadConfirm, Toshi::TXUIScene );

public:
	ABlobSaveLoadConfirm();
	~ABlobSaveLoadConfirm();

	void SetMessageText( const TWCHAR* a_wszText );

	// $deBlob: FUNCTION 0063b640
	void SetParentMenu( ABlobMenu* a_pMenu ) { m_pParentMenu = a_pMenu; }

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

private:
	ABlobMenu*        m_pParentMenu;  // 0x130 gets the answer
	TBOOL             m_bHasCancel;   // 0x134 the popup has a "Cancel" control
	Toshi::TXUIScene* m_pParentScene; // 0x138 not initialised by the ctor
};

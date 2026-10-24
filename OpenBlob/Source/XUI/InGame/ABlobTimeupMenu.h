#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the "time up" menu of a timed level: shown when the
// level timer runs out, the player retries or leaves.
//-----------------------------------------------------------------------------
class ABlobTimeupMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobTimeupMenu, ABlobMenu );

public:
	ABlobTimeupMenu();
	~ABlobTimeupMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                        // +0x04
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// ABlobMenu
	//-----------------------------------------------------------------------------
	virtual void OnConfirmationClosed( TBOOL a_bYes ) override; // +0x138

private:
	// 0x140 the retry button (control id 5)
	Toshi::TXUIControl* m_pRetryButton;
	// 0x144 the quit button (control id 4), opens the confirmation popup
	Toshi::TXUIControl* m_pQuitButton;
	// 0x148 the element (id 2) shown or hidden by OnCreated, from the game state
	Toshi::TXUIElement* m_pElement2;
	// 0x14C the element (id 3), set to the same visibility as m_pElement2 by OnCreated
	Toshi::TXUIElement* m_pElement3;
	// 0x150 the confirmation popup is open
	TUINT8 m_bConfirming;
	// 0x151
	TUINT8 m_aUnk151[ 0x154 - 0x151 ];
};

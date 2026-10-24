#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the menu after a Blob Party match's results: play again (fades out and restarts the level) or quit
// (confirmation popup, then the level ends).
//-----------------------------------------------------------------------------
class ABlobMultiplayerGameOver : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobMultiplayerGameOver, ABlobMenu );

public:
	ABlobMultiplayerGameOver();
	~ABlobMultiplayerGameOver();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                 // +0x04
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// ABlobMenu
	//-----------------------------------------------------------------------------
	virtual void OnConfirmationClosed( TBOOL a_bYes ) override; // +0x138

private:
	Toshi::TXUIControl* m_pRestart;    // 0x140 control 6
	Toshi::TXUIControl* m_pQuit;       // 0x144 control 4
	TBOOL               m_bRestarting; // 0x148 the fade out before the restart
};

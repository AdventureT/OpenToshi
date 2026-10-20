#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the game over menu of a sprint level: restart
// (control 5) or quit (control 6), both fade to black first; restarting restarts the level and closes the menus,
// quitting ends the level.
//-----------------------------------------------------------------------------
class ABlobSprintGameOver : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobSprintGameOver, ABlobMenu );

public:
	ABlobSprintGameOver();
	~ABlobSprintGameOver();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

private:
	Toshi::TXUIControl* m_pRestart; // 0x140 control 5
	Toshi::TXUIControl* m_pQuit;    // 0x144 control 6
	TBOOL               m_bFading;  // 0x148
	TBOOL               m_bRestart; // 0x149
};

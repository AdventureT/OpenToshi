#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the number of players menu:
// 2, 3 or 4 players (controls 4, 3, 2), only the counts the connected controllers allow are enabled; leaving it back
// sets one player again.
//-----------------------------------------------------------------------------
class ABlobNumberPlayersMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobNumberPlayersMenu, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_CHOICES = 3;

public:
	ABlobNumberPlayersMenu();
	~ABlobNumberPlayersMenu();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0
	virtual TBOOL OnNotifySetFocus( Toshi::TXUIElement* a_pSource ) override;                  // +0xB4

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;       // +0x114
	virtual void TransBackTo() override;   // +0x11C
	virtual void TransBackFrom() override; // +0x120

private:
	// Shows the menu cursor
	static void ShowCursor();

	// a_iValue modulo a_iCount, positive
	static TINT Wrap( TINT a_iValue, TINT a_iCount );

private:
	Toshi::TXUIControl* m_apChoices[ NUM_CHOICES ]; // 0x140 2, 3 and 4 players (controls 4, 3, 2)
	TINT                m_iNumControllers;          // 0x14C the connected controllers when it was created
};

#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the multiplayer colour selection: each player in turn picks one of 7 colours (a 4 + 3 grid), its
// button is replaced by a "selected" marker
// with the player's label; once every player has a colour "OK" shows and leads to the level selection.
//-----------------------------------------------------------------------------
class ABlobColourSelectMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobColourSelectMenu, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_COLOURS = 7;

public:
	ABlobColourSelectMenu();
	~ABlobColourSelectMenu();

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
	virtual void TransTo() override;        // +0x114
	virtual void TransFrom() override;      // +0x118
	virtual void TransBackTo() override;    // +0x11C
	virtual void TransBackFrom() override;  // +0x120
	virtual void EndTransTo() override;     // +0x124
	virtual void EndTransBackTo() override; // +0x12C

private:
	// The first player chooses again
	void RestartChoice();

	// The "CHOOSE COLOR P<n>" title of the current player
	void UpdateTitle( TWCHAR* a_wszBuffer );

	// a_iValue modulo a_iCount, positive
	static TINT Wrap( TINT a_iValue, TINT a_iCount );

private:
	Toshi::TXUIControl* m_pOK;                       // 0x140 control 21
	TUINT8              m_aUnk144[ 0x200 ];          // 0x144
	Toshi::TXUIControl* m_apButtons[ NUM_COLOURS ];  // 0x344
	Toshi::TXUIControl* m_apSelected[ NUM_COLOURS ]; // 0x360 the markers of the chosen colours
	TUINT32             m_uiUnk37C;                  // 0x37C
	TINT                m_iUnk380;                   // 0x380
	TINT                m_iPlayer;                   // 0x384 the player choosing
};

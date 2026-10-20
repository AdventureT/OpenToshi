#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the multiplayer match type menu: "MatchType1".."MatchType4" set AGameStatus's multiplayer game type;
// from the front end the level selection
// follows, in a level ("InGame") the level restarts and the menus close back to the HUD.
//-----------------------------------------------------------------------------
class ABlobMatchTypeMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobMatchTypeMenu, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_MATCH_TYPES = 4;

public:
	ABlobMatchTypeMenu();
	~ABlobMatchTypeMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;          // +0x114
	virtual void TransBackTo() override;      // +0x11C
	virtual void EndTransBackFrom() override; // +0x130

private:
	Toshi::TXUIControl* m_apMatchTypes[ NUM_MATCH_TYPES ]; // 0x140 "MatchType1".."MatchType4"
	TBOOL               m_bInGame;                         // 0x150 chosen in a level
};

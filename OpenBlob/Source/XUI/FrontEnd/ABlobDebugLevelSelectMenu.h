#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the debug level selection menu:
// pressing control 2 loads the chosen level (AXUIState's next level), a multiplayer level ("MP_") first asks the
// number of players (debugnumberplayers.xur), the music tester goes to NextState_Unk3.
//-----------------------------------------------------------------------------
class ABlobDebugLevelSelectMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobDebugLevelSelectMenu, ABlobMenu );

public:
	ABlobDebugLevelSelectMenu();
	~ABlobDebugLevelSelectMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

private:
	Toshi::TXUIControl* m_pSelect; // 0x140 control 2
};

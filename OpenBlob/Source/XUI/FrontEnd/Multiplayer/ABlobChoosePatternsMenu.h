#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the multiplayer pattern choice menu (no members nor virtuals of its own).
//-----------------------------------------------------------------------------
class ABlobChoosePatternsMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobChoosePatternsMenu, ABlobMenu );

public:
	ABlobChoosePatternsMenu();
	~ABlobChoosePatternsMenu();
};

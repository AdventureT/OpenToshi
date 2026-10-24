#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the multiplayer match options menu (no members nor virtuals of its own).
//-----------------------------------------------------------------------------
class ABlobMatchOptionsMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobMatchOptionsMenu, ABlobMenu );

public:
	ABlobMatchOptionsMenu();
	~ABlobMatchOptionsMenu();
};

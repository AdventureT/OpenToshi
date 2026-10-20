#pragma once
#include <XUI/TXUICommonList.h>

//-----------------------------------------------------------------------------
// Purpose: the list of the debug menu (a TXUICommonList): its items pick the debug level selection's mode
// (ABlobDebugLevelSelect::ms_eMode) or unlock things in the current
// profile (then saved), "Music Tester" is remembered.
//-----------------------------------------------------------------------------
class ABlobDebugMenuList : public Toshi::TXUICommonList
{
public:
	TDECLARE_CLASS( ABlobDebugMenuList, Toshi::TXUICommonList );

	// The "Music Tester" item was pressed last
	static TBOOL s_bMusicTester;

public:
	ABlobDebugMenuList();
	~ABlobDebugMenuList();

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	virtual void OnItemPressed( Toshi::TXUIListItem* a_pItem ) override; // +0x158
};

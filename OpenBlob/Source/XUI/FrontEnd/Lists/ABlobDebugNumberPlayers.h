#pragma once
#include <XUI/TXUICommonList.h>

//-----------------------------------------------------------------------------
// Purpose: the number of players list of the debug menu: item i sets i + 1 players.
//-----------------------------------------------------------------------------
class ABlobDebugNumberPlayers : public Toshi::TXUICommonList
{
public:
	TDECLARE_CLASS( ABlobDebugNumberPlayers, Toshi::TXUICommonList );

public:
	ABlobDebugNumberPlayers();
	~ABlobDebugNumberPlayers();

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	virtual void OnItemPressed( Toshi::TXUIListItem* a_pItem ) override; // +0x158
};

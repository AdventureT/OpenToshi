#pragma once
#include <XUI/TXUICommonList.h>

//-----------------------------------------------------------------------------
// Purpose: the multiplayer game type list of the debug menu: item i sets game type i, gives the players their
// default colours and loads
// the level.
//-----------------------------------------------------------------------------
class ABlobDebugMPMode : public Toshi::TXUICommonList
{
public:
	TDECLARE_CLASS( ABlobDebugMPMode, Toshi::TXUICommonList );

public:
	ABlobDebugMPMode();
	~ABlobDebugMPMode();

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	virtual void OnItemPressed( Toshi::TXUIListItem* a_pItem ) override; // +0x158
};

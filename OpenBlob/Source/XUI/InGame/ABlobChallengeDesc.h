#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the description of the current challenge: puts the challenge's text in its text element (UID 4).
//-----------------------------------------------------------------------------
class ABlobChallengeDesc : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobChallengeDesc, ABlobMenu );

public:
	ABlobChallengeDesc();
	~ABlobChallengeDesc();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94
};

TSTATICASSERT( sizeof( ABlobChallengeDesc ) == 0x140 );

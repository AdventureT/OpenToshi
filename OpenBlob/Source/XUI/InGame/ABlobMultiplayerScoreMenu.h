#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the results of a Blob Party match (multiplayerscoremenu.xur,
// created by AInGameState once the match is won): the players in their rank order
// (AGameStatus::m_aoRanks) with their name, colour and points, and the medals of the first three.
//-----------------------------------------------------------------------------
class ABlobMultiplayerScoreMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobMultiplayerScoreMenu, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_ROWS   = 4;
	static TCONSTEXPR_VAR TINT NUM_MEDALS = 3;

public:
	ABlobMultiplayerScoreMenu();
	~ABlobMultiplayerScoreMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

private:
	TWCHAR              m_awszScores[ NUM_ROWS ][ 16 ]; // 0x140 "%d"
	Toshi::TXUIElement* m_apMedals[ NUM_MEDALS ];       // 0x2C0 elements 0x17-0x19
};

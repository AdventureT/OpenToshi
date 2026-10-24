#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the multiplayer results board (scene "LeaderBoardHud"). It has three rows (the controls 9-11
// show the place, 12-14 the player's name, the elements 15-17 the score); each row is filled from AGameStatus' ranks.
// It registers itself in AGameStatus (GetLeaderBoardHud) while it exists.
//-----------------------------------------------------------------------------
class ABlobLeaderBoardHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobLeaderBoardHud, Toshi::TXUIScene );

	static TCONSTEXPR_VAR TINT NUM_ROWS = 3;

public:
	ABlobLeaderBoardHud();
	~ABlobLeaderBoardHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	// Fills the rows from the ranks of AGameStatus
	void UpdateRanks();

	// Clears the rows to the first place's names and "0" scores
	void ResetRanks();

private:
	Toshi::TXUIControl* m_apName[ NUM_ROWS ];          // 0x130 controls 12-14: the player's name
	Toshi::TXUIControl* m_apPlace[ NUM_ROWS ];         // 0x13C controls 9-11: the place ("1st", "2nd"...)
	Toshi::TXUIControl* m_apScore[ NUM_ROWS ];         // 0x148 elements 15-17: the score (SetText)
	TWCHAR              m_awszScore[ NUM_ROWS ][ 32 ]; // 0x154 the score texts ("%d")
};

#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the level progress screen: the progress of the
// player through the level's goals and the medals (awards) of the level.
//-----------------------------------------------------------------------------
class ABlobLevelProgress : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobLevelProgress, ABlobMenu );

	static TCONSTEXPR_VAR TINT MAXAWARDS = 8; // "Award1".."Award8" controls, their awarded flags are m_abAwarded

public:
	ABlobLevelProgress();
	~ABlobLevelProgress();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                       // +0x94
	virtual TBOOL OnNotifySetFocus( Toshi::TXUIElement* a_pSource ) override; // +0xB4

private:
	// 0x140 the "%d/%d" text of the counter control 0x12
	TWCHAR m_wszCounter140[ 0x10 ];
	// 0x160 the percentage text "%d%%" of the level's progress (field 0x170 of AGameStatus)
	TWCHAR m_wszPercent[ 0x10 ];
	// 0x180 the "%d/%d" text of the medal trees
	TWCHAR m_wszTrees[ 0x10 ];
	// 0x1A0 the "%d/%d" text of the raydian medal
	TWCHAR m_wszRaydian[ 0x10 ];
	// 0x1C0
	TUINT8 m_aUnk1C0[ 0x20 ];
	// 0x1E0 the "%d/%d" text of the transform medal
	TWCHAR m_wszTransform[ 0x10 ];
	// 0x200 the "%d/%d" text of the bill medal
	TWCHAR m_wszBill[ 0x10 ];
	// 0x220 the "%d/%d" text of the landmarks medal
	TWCHAR m_wszLandmarks[ 0x10 ];
	// 0x240
	TUINT8 m_aUnk240[ 0x20 ];
	// 0x260 the "%d/%d" text of the styles medal
	TWCHAR m_wszStyles[ 0x10 ];
	// 0x280 the time text "%d:%d" (minutes:seconds, the level time)
	TWCHAR m_wszTime[ 0x10 ];
	// 0x2A0 the "%d" text of the first player's points
	TWCHAR m_wszNumber2A0[ 0x10 ];
	// 0x2C0 the "%d" text of the gates left
	TWCHAR m_wszGatesLeft[ 0x10 ];
	// 0x2E0 the award controls "Award1".."Award8"
	Toshi::TXUIControl* m_apAwards[ MAXAWARDS ];
	// 0x300 the awarded flag of each award (the award's sound plays on focus when set): 0 Medal paint, 1 trees,
	// 2 raydian, 3 bill, 4 landmarks, 5 transform, 6 challenge, 7 styles
	TUINT8 m_abAwarded[ MAXAWARDS ];
};

TSTATICASSERT( sizeof( ABlobLevelProgress ) == 0x308 );

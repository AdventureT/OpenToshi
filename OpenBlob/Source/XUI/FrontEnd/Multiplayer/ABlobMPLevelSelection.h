#pragma once
#include "XUI/ABlobMenu.h"

class ABlobCarouselList;

//-----------------------------------------------------------------------------
// Purpose: the multiplayer level selection:
// a carousel of the multiplayer levels (unlocked in EnSaveData::SlotData::m_iUnlockedBlobParty), with the level's name, its game
// type's description and a "New" label; selecting one loads the level from the frontend (AXUIState) or from a level
// (AInGameState).
//-----------------------------------------------------------------------------
class ABlobMPLevelSelection : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobMPLevelSelection, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_LEVELS = 9; // AGameStatus::MultiplayerLevel_NUMOF

	// The levels, their names, the descriptions of their game types and their game
	// types
	static const TCHAR* sm_aszLevels[ NUM_LEVELS ];
	static const TINT   sm_aiLevelNames[ NUM_LEVELS ];
	static const TINT   sm_aiLevelDescriptions[ NUM_LEVELS ];
	static const TINT   sm_aiLevelGameTypes[ NUM_LEVELS ];

public:
	ABlobMPLevelSelection();
	~ABlobMPLevelSelection();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;          // +0x114
	virtual void TransBackFrom() override;    // +0x120
	virtual void EndTransBackFrom() override; // +0x130

private:
	// Loads the level a_iLevelIndex
	void StartLevel( TINT a_iLevelIndex );

	// Shows the name, description and "New" label of a level
	void UpdateLevelInfo( TINT a_iLevelIndex );

private:
	Toshi::TXUIControl* m_pName;        // 0x140 control 3
	Toshi::TXUIControl* m_pDescription; // 0x144 control 2
	Toshi::TXUIControl* m_pLeft;        // 0x148 control 6
	Toshi::TXUIControl* m_pRight;       // 0x14C control 5
	ABlobCarouselList*  m_pCarousel;    // 0x150 control 7
	Toshi::TXUIControl* m_pSelect;      // 0x154 control 8
	Toshi::TXUIControl* m_pNew;         // 0x158 control 10
};

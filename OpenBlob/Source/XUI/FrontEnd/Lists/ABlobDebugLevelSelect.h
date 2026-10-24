#pragma once
#include <XUI/TXUIList.h>
#include <Toshi/TString16.h>

//-----------------------------------------------------------------------------
// Purpose: the list of the debug level selection: its items are the worlds, the sprint challenges of a kind, the
// multiplayer levels or all the
// levels of ALevelInformation, depending on the mode the debug menu (ABlobDebugMenuList) picked; pressing one makes the
// current AXUIState load that level.
//-----------------------------------------------------------------------------
class ABlobDebugLevelSelect : public Toshi::TXUIList
{
public:
	TDECLARE_CLASS( ABlobDebugLevelSelect, Toshi::TXUIList );

	enum MODE : TINT
	{
		MODE_Worlds         = 0, // the ten worlds
		MODE_Sprints        = 1, // twelve sprint challenges
		MODE_SprintsA       = 2, // four sprint challenges
		MODE_Multiplayer    = 3, // the multiplayer levels
		MODE_SprintsB       = 4, // four other sprint challenges
		MODE_AllLevels      = 5, // the levels of ALevelInformation with m_uiUnk28 set
		MODE_AllLevelsIndex = 6, // the same, the level's index is passed on
		MODE_AllLevels2     = 7, // the same as MODE_AllLevels
	};

	// The mode (set by ABlobDebugMenuList)
	static TINT ms_eMode;

public:
	ABlobDebugLevelSelect();
	~ABlobDebugLevelSelect();

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	virtual const TWCHAR* GetListItemText( TINT a_iItemIndex ) override;          // +0x124
	virtual void          OnItemPressed( Toshi::TXUIListItem* a_pItem ) override; // +0x158

private:
	Toshi::TString16* m_pItemTexts; // 0xD8 m_uiNumItems strings
};                                  // 0xDC

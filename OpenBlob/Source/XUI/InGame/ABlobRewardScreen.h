#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the reward screen after a level: the rewards and the
// unlocks the level gave, each with its bling and sparkles.
//-----------------------------------------------------------------------------
class ABlobRewardScreen : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobRewardScreen, ABlobMenu );

	static TCONSTEXPR_VAR TINT MAXBLINGS   = 3; // "Bling1".."Bling3" controls (ids 10..12)
	static TCONSTEXPR_VAR TINT MAXSPARKLES = 5; // per bling: the base visual and "Sparkle1".."Sparkle4"

public:
	ABlobRewardScreen();
	~ABlobRewardScreen();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                              // +0x04
	virtual TBOOL SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam ) override; // +0x78
	virtual void  OnCreated() override;                                              // +0x94
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override;       // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void EndTransTo() override; // +0x124

	static TCONSTEXPR_VAR TINT MAXREWARDS = 10; // "m_iNumRewards < MAXREWARDS"

	// The kinds of rewards
	enum RewardType : TINT
	{
		RewardType_World,     // 0 a world (its name)
		RewardType_WorldMode, // 1 a mode of a world
		RewardType_Challenge, // 2 a sprint challenge (two per world)
		RewardType_Movie,     // 3 a movie
		RewardType_Video,     // 4 a video of the extras
		RewardType_Gallery,   // 5 a gallery item
		RewardType_BlobParty, // 6 a Blob party item
	};

	// The display of a reward: the bling's visual animation of its type
	enum RewardDisplay : TINT
	{
		RewardDisplay_Normal,
		RewardDisplay_Mode,
		RewardDisplay_Movie,
		RewardDisplay_BlobParty,
	};

	// Adds a reward to the list shown by the screen
	void AddReward( TINT a_eType, TINT a_iIndex );

	TINT GetNumRewards() const { return m_iNumRewards; }

private:
	// Shows the next rewards (up to one per bling) and plays the "Display" animation
	void ShowNextRewards();

private:
	struct Reward
	{
		TINT m_eType;    // 0x0 RewardType
		TINT m_eDisplay; // 0x4 RewardDisplay
		TINT m_iIndex;   // 0x8 the world, challenge, movie... of the reward
	};

	// 0x140 the rewards of the level (AddReward)
	Reward m_aRewards[ MAXREWARDS ];
	// 0x1B8 the bling controls "Bling1".."Bling3"
	Toshi::TXUIControl* m_apBlings[ MAXBLINGS ];
	// 0x1C4 the "ItemType" text of each bling's visual (the reward's type)
	Toshi::TXUIControl* m_apItemTypes[ MAXBLINGS ];
	// 0x1D0 the "Description" text of each bling's visual (the reward's name)
	Toshi::TXUIControl* m_apDescriptions[ MAXBLINGS ];
	// 0x1DC the "Bling" element of each bling's visual, the parent of the sparkles
	Toshi::TXUIElement* m_apBlingElements[ MAXBLINGS ];
	// 0x1E8 the sparkles of each bling: [0] "Sparkle", [1..4] "Sparkle1".."Sparkle4"
	Toshi::TXUIControl* m_apSparkles[ MAXBLINGS ][ MAXSPARKLES ];
	// 0x224 the description text of each bling (L"%ls" of the reward's name)
	TWCHAR m_awszDescriptions[ MAXBLINGS ][ 0x100 ];
	// 0x824 the number of rewards of the level (EndTransTo asserts it is > 0)
	TINT m_iNumRewards;
	// 0x828 the index of the reward shown (HandleButton compares it with m_iNumRewards)
	TINT m_iCurrentReward;
	// 0x82C the group (id 9) that plays the "Display" animation, the screen waits while it plays
	Toshi::TXUIElement* m_pElement9;
	// 0x830
	TUINT8 m_aUnk830[ 4 ];
	// 0x834 the continue button was pressed, Tick waits for the fade
	TUINT8 m_bLeaving;
	// 0x835 Tick's branch once faded: set, the level ends; else the level restarts and the screen
	// goes back
	TUINT8 m_bExit;
	// 0x836
	TUINT8 m_aUnk836[ 0x838 - 0x836 ];
};

TSTATICASSERT( sizeof( ABlobRewardScreen ) == 0x838 );

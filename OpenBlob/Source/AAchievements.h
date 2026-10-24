#pragma once
#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Purpose: the Steam achievements: checks the current profile's progress and unlocks the achievements it earned through
// SteamManager (ISteamUserStats). Remembers which ones it unlocked this session; nothing is saved (Steam keeps the
// achievements).
//-----------------------------------------------------------------------------
class AAchievements
    : public Toshi::TSingleton<AAchievements>
{
public:
	enum ACHIEVEMENT : TINT
	{
		ACHIEVEMENT_LastResortSavior, // the level's bit 0 (completed) of each world
		ACHIEVEMENT_UptownSavior,
		ACHIEVEMENT_DowntownSavior,
		ACHIEVEMENT_TheChromaDamSavior,
		ACHIEVEMENT_GuggentrazIslandSavior,
		ACHIEVEMENT_TheDocklandsSavior,
		ACHIEVEMENT_HangingGardensSavior,
		ACHIEVEMENT_ChromaCityFunParkSavior,
		ACHIEVEMENT_MinistryOfInkSavior,
		ACHIEVEMENT_LakeRaydaSavior,
		ACHIEVEMENT_SPGold,            // the gold medal of every world
		ACHIEVEMENT_SPAllGold,         // SPGold and the gold medal of every sprint level
		ACHIEVEMENT_100PercentPainted, // the award 0x80 (Paint100) in every world
		ACHIEVEMENT_ChallengeKing,     // 0x100
		ACHIEVEMENT_SpeedKing,         // 0x200
		ACHIEVEMENT_PatternMaster,     // 0x400
		ACHIEVEMENT_NatureBoy,         // 0x800
		ACHIEVEMENT_Landmarker,        // 0x1000
		ACHIEVEMENT_BillboardBuster,   // 0x2000
		ACHIEVEMENT_UrbanLandscaper,   // 0x4000
		ACHIEVEMENT_RadianRescue,      // 0x8000
		ACHIEVEMENT_Blimp,             // 0x10000
		ACHIEVEMENT_Platinum,          // all the others
		ACHIEVEMENT_NUMOF,

		// CheckAchievements: every achievement
		ACHIEVEMENT_ALL = ACHIEVEMENT_NUMOF
	};

	static TCONSTEXPR_VAR TINT NUM_WORLDS        = 10;
	static TCONSTEXPR_VAR TINT NUM_SPRINT_LEVELS = 20;

public:
	AAchievements();

	// The scalar deleting destructor; the destructor itself is TSingleton's (see TSingleton.h)
	// $deBlob: FUNCTION 0056c190
	~AAchievements() = default;

	// Forgets the achievements unlocked this session (a profile was loaded)
	void Reset();

	// Called each frame by AApplication::OnUpdate; does nothing
	void Update( TFLOAT a_fDeltaTime );

	// Checks the achievement a_eAchievement (ACHIEVEMENT_ALL: every one) against the current profile and unlocks it if
	// earned; Platinum is always checked
	void CheckAchievements( TINT a_eAchievement );

	// The front end menu shown (ABlobMainMenu: 0 main menu, 1 story, 2 extras); never read
	void SetMenuState( TINT a_iState );

	// Locks every achievement on Steam (unused); TFALSE once one fails
	static TBOOL LockAllAchievements();

	// Unlocks a_eAchievement on Steam if it wasn't unlocked this session
	void Unlock( TINT a_eAchievement );

	// Unlocks a_eAchievement on Steam (unused)
	static TBOOL UnlockSteamAchievement( TINT a_eAchievement );

private:
	// The Steam API names of the achievements
	static const TCHAR* ms_aszAchievementIDs[ ACHIEVEMENT_NUMOF ];

private:
	TBOOL m_abUnlocked[ ACHIEVEMENT_NUMOF ]; // 0x00 unlocked this session
	TINT  m_iMenuState;                      // 0x18 -1 by default
};

#include "pch.h"
#include "AAchievements.h"
#include "SteamManager.h"
#include "GameInterface/APlayerProfileManager.h"
#include "MemoryCard/EnSaveData.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AAchievements ) == 0x1C );

const TCHAR* AAchievements::ms_aszAchievementIDs[ ACHIEVEMENT_NUMOF ] = {
	"AchievementID_LastResortSavior",
	"AchievementID_UptownSavior",
	"AchievementID_DowntownSavior",
	"AchievementID_TheChromaDamSavior",
	"AchievementID_GuggentrazIslandSavior",
	"AchievementID_TheDocklandsSavior",
	"AchievementID_HangingGardensSavior",
	"AchievementID_ChromaCityFunParkSavior",
	"AchievementID_MinistryOfInkSavior",
	"AchievementID_LakeRaydaSavior",
	"AchievementID_SPGold",
	"AchievementID_SPAllGold",
	"AchievementID_100PercentPainted",
	"AchievementID_ChallengeKing",
	"AchievementID_SpeedKing",
	"AchievementID_PatternMaster",
	"AchievementID_NatureBoy",
	"AchievementID_Landmarker",
	"AchievementID_BillboardBuster",
	"AchievementID_UrbanLandscaper",
	"AchievementID_RadianRescue",
	"AchievementID_Blimp",
	"AchievementID_Platinum",
};

// $deBlob: FUNCTION 005e7680
AAchievements::AAchievements()
{
	Reset();
}

// $deBlob: FUNCTION 005e7730
void AAchievements::Reset()
{
	for ( TINT i = 0; i < ACHIEVEMENT_NUMOF; i++ )
		m_abUnlocked[ i ] = TFALSE;

	m_iMenuState = -1;
}

// $deBlob: FUNCTION 005e7750
void AAchievements::Update( TFLOAT a_fDeltaTime )
{
}

// $deBlob: FUNCTION 005e7760
void AAchievements::CheckAchievements( TINT a_eAchievement )
{
	APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
	if ( pProfile == TNULL || !pProfile->m_bUsed ) return;

	TBOOL bAll = a_eAchievement == ACHIEVEMENT_ALL;

	// The worlds' savior achievements: the level was completed
	for ( TINT i = ACHIEVEMENT_LastResortSavior; i <= ACHIEVEMENT_LakeRaydaSavior; i++ )
	{
		if ( !m_abUnlocked[ i ] && ( a_eAchievement == i || bAll ) && pProfile->GetData()->IsLevelFlagSet( 0, i ) )
			Unlock( i );
	}

	// The gold medal of every world
	if ( !m_abUnlocked[ ACHIEVEMENT_SPGold ] && ( a_eAchievement == ACHIEVEMENT_SPGold || bAll ) )
	{
		TBOOL bGold = TTRUE;

		for ( TINT i = 0; i < NUM_WORLDS && bGold; i++ )
			bGold &= pProfile->HasGoldMedal( i );

		if ( bGold )
			Unlock( ACHIEVEMENT_SPGold );
	}

	// SPGold and the gold medal of every sprint level
	if ( !m_abUnlocked[ ACHIEVEMENT_SPAllGold ] && ( a_eAchievement == ACHIEVEMENT_SPAllGold || bAll ) )
	{
		TBOOL bGold = m_abUnlocked[ ACHIEVEMENT_SPGold ];

		for ( TINT i = 0; i < NUM_SPRINT_LEVELS && bGold; i++ )
			bGold &= pProfile->HasSprintGoldMedal( i );

		if ( bGold )
			Unlock( ACHIEVEMENT_SPAllGold );
	}

	// The awards (0x80 Paint100 ... 0x10000) in every world
	for ( TINT i = ACHIEVEMENT_100PercentPainted; i <= ACHIEVEMENT_Blimp; i++ )
	{
		if ( !m_abUnlocked[ i ] && ( a_eAchievement == i || bAll ) )
		{
			TUINT32 uiAward = 0x80 << ( i - ACHIEVEMENT_100PercentPainted );
			TBOOL   bAward  = TTRUE;

			for ( TINT j = 0; j < NUM_WORLDS && bAward; j++ )
				bAward &= pProfile->GetData()->HasMedalFlags( uiAward, j );

			if ( bAward )
				Unlock( i );
		}
	}

	// Platinum: every other achievement (checked whatever a_eAchievement is)
	TBOOL bPlatinum = TTRUE;

	for ( TINT i = 0; i < ACHIEVEMENT_NUMOF; i++ )
	{
		if ( i != ACHIEVEMENT_Platinum )
			bPlatinum &= m_abUnlocked[ i ];

		if ( !bPlatinum ) return;
	}

	if ( bPlatinum )
		Unlock( ACHIEVEMENT_Platinum );
}

// $deBlob: FUNCTION 005e80e0
void AAchievements::SetMenuState( TINT a_iState )
{
	if ( m_iMenuState != a_iState )
		m_iMenuState = a_iState;
}

// $deBlob: FUNCTION 005e8100
TBOOL AAchievements::LockAllAchievements()
{
	TBOOL bResult = TTRUE;

	for ( TINT i = 0; i < ACHIEVEMENT_NUMOF; i++ )
		bResult = bResult && SteamManager::GetInstance()->ClearAchievement( ms_aszAchievementIDs[ i ] );

	return bResult;
}

// $deBlob: FUNCTION 005e8170
void AAchievements::Unlock( TINT a_eAchievement )
{
	if ( !m_abUnlocked[ a_eAchievement ] )
	{
		if ( SteamManager::GetInstance()->SetAchievement( ms_aszAchievementIDs[ a_eAchievement ] ) )
			m_abUnlocked[ a_eAchievement ] = TTRUE;
	}
}

// $deBlob: FUNCTION 005e81e0
TBOOL AAchievements::UnlockSteamAchievement( TINT a_eAchievement )
{
	return SteamManager::GetInstance()->SetAchievement( ms_aszAchievementIDs[ a_eAchievement ] );
}

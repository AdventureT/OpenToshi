#include "pch.h"
#include "APlayerProfileManager.h"
#include "AApplication.h"
#include "MemoryCard/EnSaveData.h"
#include "GameController/AGameStatus.h"
#include "Sound/AMusicManager.h"
#include "AAchievements.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: no CreateObject
// $deBlob: FUNCTION 005d16f0
// $deBlob: FUNCTION 005d1700
// $deBlob: FUNCTION 005d1710
// $deBlob: FUNCTION 005d1720
// $deBlob: FUNCTION 005d1730
// $deBlob: FUNCTION 0053c6f0
// $deBlob: FUNCTION 00858500
TDEFINE_CLASS_NORUNTIME( APlayerProfile );

// The name of a profile without a slot
static const TWCHAR s_wszNoProfileName[] = L"";

// $deBlob: FUNCTION 005d1740
APlayerProfile::APlayerProfile()
{
	m_bUsed = TTRUE;
	m_pData = TNULL;
}

// $deBlob: FUNCTION 005d1770
// $deBlob: FUNCTION 005d2390
APlayerProfile::~APlayerProfile()
{
}

// $deBlob: FUNCTION 005d1800
void APlayerProfile::SetName( const TWCHAR* a_wszName )
{
	TStringManager::String16Copy( m_pData->m_wszName, a_wszName, 12 );
}

// $deBlob: FUNCTION 005d1820
void APlayerProfile::RecordLevelResult( TINT a_iWorld, TINT a_eLevelMode, TUINT32 a_uiMedals, TFLOAT a_fTime, TUINT32 a_uiPoints, ABlobRewardScreen* a_pRewardScreen )
{
	if ( TUINT( a_iWorld ) > 9 )
		return;

	if ( a_eLevelMode == AGameStatus::Mode_Story )
		m_pData->AddMedals( a_iWorld, a_uiMedals, a_pRewardScreen );

	// The best time of the medal won
	if ( a_uiMedals & 1 )
		m_pData->RecordBest( a_iWorld, TUINT8( a_eLevelMode ), a_fTime, a_uiPoints, 0 );
	else if ( a_uiMedals & 2 )
		m_pData->RecordBest( a_iWorld, TUINT8( a_eLevelMode ), a_fTime, a_uiPoints, 1 );
	else if ( a_uiMedals & 4 )
		m_pData->RecordBest( a_iWorld, TUINT8( a_eLevelMode ), a_fTime, a_uiPoints, 2 );

	// Any medal unlocks the next world, silver its first challenge, gold its second one
	TBOOL bAnyMedal    = TFALSE;
	TBOOL bSilverMedal = TFALSE;
	TBOOL bGoldMedal   = TFALSE;

	if ( a_uiMedals & 1 )
	{
		bAnyMedal = TTRUE;
	}
	else if ( a_uiMedals & 2 )
	{
		bSilverMedal = TTRUE;
		bAnyMedal    = TTRUE;
	}
	else if ( a_uiMedals & 4 )
	{
		bGoldMedal   = TTRUE;
		bSilverMedal = TTRUE;
		bAnyMedal    = TTRUE;
	}

	if ( AGameStatus::GetSingleton() != TNULL && AGameStatus::GetSingleton()->GetLevelData().m_bGoalCheat )
		bAnyMedal = TTRUE;

	// The Blob party item each even world unlocks (-1: none)
	static TCONSTEXPR_VAR TINT s_aiBlobPartyItems[ 9 ] = { 2, -1, 5, -1, 6, -1, 7, -1, 8 };

	if ( a_iWorld < 9 )
	{
		if ( bAnyMedal )
		{
			m_pData->UnlockWorld( a_iWorld + 1, a_pRewardScreen );
			m_pData->UnlockWorldMode( a_iWorld, 1, a_pRewardScreen );
			m_pData->UnlockMovie( a_iWorld + 1, a_pRewardScreen );

			if ( s_aiBlobPartyItems[ a_iWorld ] != -1 )
				m_pData->UnlockBlobParty( s_aiBlobPartyItems[ a_iWorld ], a_pRewardScreen );
		}
	}
	else
	{
		// The last world unlocks the last two movies
		if ( bAnyMedal )
		{
			m_pData->UnlockMovie( 10, a_pRewardScreen );
			m_pData->UnlockMovie( 11, a_pRewardScreen );
		}
	}

	if ( bSilverMedal )
		m_pData->UnlockChallenge( a_iWorld, a_iWorld * 2, a_pRewardScreen );

	if ( bGoldMedal )
		m_pData->UnlockChallenge( a_iWorld, a_iWorld * 2 + 1, a_pRewardScreen );

	AAchievements::GetSingleton()->CheckAchievements( AAchievements::ACHIEVEMENT_ALL );
}

// $deBlob: FUNCTION 005d1c30
void APlayerProfile::RecordSprintResult( TINT a_iSlot, TUINT32 a_uiMedals, TFLOAT a_fTime, TUINT32 a_uiCount, TUINT32 a_uiTotal, ABlobRewardScreen* a_pRewardScreen )
{
	m_pData->AddSprintMedals( a_iSlot, a_uiMedals );

	TBOOL bMedal;

	if ( a_uiMedals & 1 )
	{
		bMedal = TTRUE;
		m_pData->RecordSprintMedalTime( a_iSlot, a_fTime, 0 );
	}
	else if ( a_uiMedals & 2 )
	{
		bMedal = TTRUE;
		m_pData->RecordSprintMedalTime( a_iSlot, a_fTime, 1 );
	}
	else if ( a_uiMedals & 4 )
	{
		bMedal = TTRUE;
		m_pData->RecordSprintMedalTime( a_iSlot, a_fTime, 2 );
	}
	else
	{
		// NOTE: retail never sets its flag without a medal: the compiler reads the stack byte that holds the high byte
		// of a_pRewardScreen (a heap pointer), so a sprint finished without a medal unlocks the same things as one with
		// a medal (the flag is also set for any reward screen at 0x01000000 or above)
		bMedal = ( TUINTPTR( a_pRewardScreen ) >> 24 ) != 0;
	}

	m_pData->RecordSprintBest( a_iSlot, a_uiCount, a_uiTotal );

	// Each world has two sprint levels: the first unlocks the world's mode 2 and its gallery item, the second its mode 3
	// and its video
	if ( a_iSlot >= 0 && a_iSlot < AGameStatus::SprintLevel_NUMOF && bMedal )
	{
		TINT iWorld = a_iSlot / 2;

		if ( ( a_iSlot & 1 ) == 0 )
		{
			m_pData->UnlockWorldMode( iWorld, 2, a_pRewardScreen );
			m_pData->UnlockGalleryItem( iWorld, a_pRewardScreen );
		}
		else
		{
			m_pData->UnlockWorldMode( iWorld, 3, a_pRewardScreen );
			m_pData->UnlockVideo( iWorld, a_pRewardScreen );
		}
	}

	AAchievements::GetSingleton()->CheckAchievements( AAchievements::ACHIEVEMENT_ALL );
}

// $deBlob: FUNCTION 005d20b0
void APlayerProfile::UnlockAllTracks()
{
	m_pData->m_iUnlockedTracks |= 0xD;
}

// $deBlob: FUNCTION 005d20c0
TBOOL APlayerProfile::IsTrackLocked( TINT a_iTrackIndex )
{
	// NOTE: retail's MUSIC_SP_FUNK_1 is 1 here (the assert fails for 0)
	TASSERT( ( a_iTrackIndex >= AMusicManager::MUSIC_SP_FUNK_1 + 1 ) && ( a_iTrackIndex < AMusicManager::MUSIC_NUM_SP_TRACKS ) );

	// NOTE: retail tests the index itself instead of its bit (UnlockTrack sets 1 << a_iTrackIndex), not referenced
	return ( a_iTrackIndex & m_pData->m_iUnlockedTracks ) == 0;
}

// $deBlob: FUNCTION 005d2120
void APlayerProfile::UnlockTrack( TINT a_iTrackIndex )
{
	// NOTE: retail's MUSIC_SP_FUNK_1 is 1 here (the assert fails for 0)
	TASSERT( ( a_iTrackIndex >= AMusicManager::MUSIC_SP_FUNK_1 + 1 ) && ( a_iTrackIndex < AMusicManager::MUSIC_NUM_SP_TRACKS ) );
	m_pData->m_iUnlockedTracks |= 1 << a_iTrackIndex;
}

// $deBlob: FUNCTION 005d2240
void APlayerProfile::UnlockAllWorlds()
{
	m_pData->m_iUnlockedWorlds |= 0x3FF;
}

// $deBlob: FUNCTION 005d2250
void APlayerProfile::UnlockWorldAndModes( TINT a_iWorld )
{
	m_pData->m_iUnlockedWorlds |= 1 << a_iWorld;
	m_pData->m_auiUnlockedModes[ a_iWorld ] |= 0xE;
	m_pData->m_iUnlockedChallenges |= ( 1 << ( a_iWorld * 2 ) ) | ( 1 << ( a_iWorld * 2 + 1 ) );
}

// $deBlob: FUNCTION 005d2290
void APlayerProfile::UnlockAllVideos()
{
	m_pData->m_iUnlockedVideos |= 0x3FF;
	EnSaveData::SlotData* pData = m_pData;
	pData->m_uiNewFlags |= EnSaveData::NEWFLAG_Movies;
	pData->m_uiNewFlagsUnseen |= EnSaveData::NEWFLAG_Movies;
}

// $deBlob: FUNCTION 005d22b0
void APlayerProfile::UnlockAllMovies()
{
	m_pData->m_iUnlockedMovies |= 0xFFF;
	EnSaveData::SlotData* pData = m_pData;
	pData->m_uiNewFlags |= EnSaveData::NEWFLAG_Cutscenes;
	pData->m_uiNewFlagsUnseen |= EnSaveData::NEWFLAG_Cutscenes;
}

// $deBlob: FUNCTION 005d22d0
void APlayerProfile::UnlockAllGalleryItems()
{
	m_pData->m_uiUnlockedGalleryItems |= 0x3FF;
	EnSaveData::SlotData* pData = m_pData;
	pData->m_uiNewFlags |= EnSaveData::NEWFLAG_Gallery;
	pData->m_uiNewFlagsUnseen |= EnSaveData::NEWFLAG_Gallery;
}

// $deBlob: FUNCTION 005d2300
void APlayerProfile::UnlockLastGalleryItem()
{
	m_pData->m_uiUnlockedGalleryItems |= 0x400;
	EnSaveData::SlotData* pData = m_pData;
	pData->m_uiNewFlags |= 8;
	pData->m_uiNewFlagsUnseen |= 8;
}

// $deBlob: FUNCTION 005d2330
void APlayerProfile::UnlockAllBlobPartyItems()
{
	m_pData->m_iUnlockedBlobParty |= 0x1FF;
}

// $deBlob: FUNCTION 005d2460
// $deBlob: FUNCTION 005d2470
// $deBlob: FUNCTION 005d24e0
// $deBlob: FUNCTION 005d2540
// $deBlob: FUNCTION 005d2550
// $deBlob: FUNCTION 0053c730
// $deBlob: FUNCTION 00858510
TDEFINE_CLASS( APlayerProfileManager );

// $deBlob: FUNCTION 005d2560
APlayerProfileManager::APlayerProfileManager()
{
	m_iCurrentProfile = -1;

	EnSaveData* pSaveData = TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData );

	for ( TINT i = 0; i < NUMPROFILES; i++ )
	{
		TASSERT( i >= 0 && i < EnSaveData::SAVEGAME_SLOT_COUNT );
		m_aProfiles[ i ].m_pData = &pSaveData->GetSaveData()->m_aSaveGameSlots[ i ].m_oData;
	}
}

// $deBlob: FUNCTION 005d2680
// $deBlob: FUNCTION 005d2940
APlayerProfileManager::~APlayerProfileManager()
{
}

// $deBlob: FUNCTION 005d27c0
void APlayerProfileManager::SetCurrentProfile( TINT a_iIndex )
{
	m_iCurrentProfile = a_iIndex;

	if ( a_iIndex != -1 )
		g_oTheApp.SetCurrentSaveSlot( a_iIndex );
}

// $deBlob: FUNCTION 005d27e0
TBOOL APlayerProfileManager::IsProfileUsed( TINT a_iIndex )
{
	TASSERT( a_iIndex >= 0 && a_iIndex < NUMPROFILES );
	return m_aProfiles[ a_iIndex ].m_bUsed;
}

// $deBlob: FUNCTION 005d2830
void APlayerProfileManager::SetProfileUnused( TINT a_iIndex )
{
	TASSERT( a_iIndex >= 0 && a_iIndex < NUMPROFILES );
	m_aProfiles[ a_iIndex ].m_bUsed = TFALSE;
}

// $deBlob: FUNCTION 005d2760
void APlayerProfileManager::SetCurrentProfileName( const TWCHAR* a_wszName )
{
	if ( m_iCurrentProfile >= 0 && m_iCurrentProfile < NUMPROFILES )
	{
		m_aProfiles[ m_iCurrentProfile ].SetName( a_wszName );
		m_aProfiles[ m_iCurrentProfile ].m_bUsed = TTRUE;
	}
}

// $deBlob: FUNCTION 005d27a0
const TWCHAR* APlayerProfileManager::GetCurrentProfileName()
{
	if ( m_iCurrentProfile >= 0 && m_iCurrentProfile < NUMPROFILES )
		return m_aProfiles[ m_iCurrentProfile ].m_pData->m_wszName;

	return s_wszNoProfileName;
}

// $deBlob: FUNCTION 005d2880
APlayerProfile* APlayerProfileManager::GetCurrentProfile()
{
	if ( m_iCurrentProfile >= 0 && m_iCurrentProfile < NUMPROFILES )
		return &m_aProfiles[ m_iCurrentProfile ];

	return TNULL;
}

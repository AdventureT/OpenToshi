#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include "MemoryCard/EnSaveData.h"

//-----------------------------------------------------------------------------
// Purpose: a player profile: its name lives in its slot of the application's save data.
//-----------------------------------------------------------------------------
class APlayerProfile
    : public Toshi::TObject
    , public Toshi::T2DList<APlayerProfile>::Node
{
public:
	TDECLARE_CLASS( APlayerProfile, Toshi::TObject );

public:
	APlayerProfile();
	~APlayerProfile();

	// $deBlob: FUNCTION 005d2430
	virtual TBOOL Unknown0C() { return TFALSE; } // +0x0C

	TNOINLINE void SetName( const TWCHAR* a_wszName );

	// $deBlob: FUNCTION 005e82d0
	EnSaveData::SlotData* GetData() const { return m_pData; }

	// Every music track is unlocked (m_iUnlockedTracks & 0xD)
	// $deBlob: FUNCTION 005d2090
	TBOOL AreAllTracksUnlocked() const { return ( m_pData->m_iUnlockedTracks & 0xD ) != 0; }

	// The medals of a world (m_auiMedals: 1 bronze, 2 silver, 4 gold) and of a sprint level (m_auiSprintMedals), at least
	// the given medal
	// $deBlob: FUNCTION 005d2180
	TBOOL HasGoldMedal( TINT a_iWorld ) const { return ( m_pData->m_auiMedals[ a_iWorld ] >> 2 ) & 1; }
	// $deBlob: FUNCTION 005d21a0
	TBOOL HasSilverMedal( TINT a_iWorld ) const { return ( m_pData->m_auiMedals[ a_iWorld ] & 6 ) != 0; }
	// $deBlob: FUNCTION 005d21c0
	TBOOL HasBronzeMedal( TINT a_iWorld ) const { return ( m_pData->m_auiMedals[ a_iWorld ] & 7 ) != 0; }
	// $deBlob: FUNCTION 005d21e0
	TBOOL HasSprintGoldMedal( TINT a_iSlot ) const { return ( m_pData->m_auiSprintMedals[ a_iSlot ] >> 2 ) & 1; }
	// $deBlob: FUNCTION 005d2200
	TBOOL HasSprintSilverMedal( TINT a_iSlot ) const { return ( m_pData->m_auiSprintMedals[ a_iSlot ] & 6 ) != 0; }
	// $deBlob: FUNCTION 005d2220
	TBOOL HasSprintBronzeMedal( TINT a_iSlot ) const { return ( m_pData->m_auiSprintMedals[ a_iSlot ] & 7 ) != 0; }

	// Records the results of the sprint level a_iSlot (ABlobSprintSummaryMenu): its medals, the medal's time, the
	// level's counter (AChallengeManager 0x258/0x25C) and the unlocks the medal gives
	void RecordSprintResult( TINT a_iSlot, TUINT32 a_uiMedals, TFLOAT a_fTime, TUINT32 a_uiCount, TUINT32 a_uiTotal, class ABlobRewardScreen* a_pRewardScreen );

	// The unlocks of the cheats (ACheatManager) and of the debug menu (ABlobDebugMenuList)
	void  UnlockAllTracks();
	TBOOL IsTrackLocked( TINT a_iTrackIndex );
	void  UnlockTrack( TINT a_iTrackIndex );
	void  UnlockAllWorlds();
	void  UnlockWorldAndModes( TINT a_iWorld );
	void  UnlockAllVideos();
	void  UnlockAllMovies();
	void  UnlockAllGalleryItems();
	void  UnlockLastGalleryItem();
	void  UnlockAllBlobPartyItems();

	// Records the results of a level of world a_iWorld (ABlobScoreMenu): the medals (a_uiMedals, the level's PlayerData
	// 0x3C), the best time and score, and the unlocks the medals give (the next world, its mode, movies, Blob party
	// items, the world's challenges), each added to a_pRewardScreen
	void RecordLevelResult( TINT a_iWorld, TINT a_eLevelMode, TUINT32 a_uiMedals, TFLOAT a_fTime, TUINT32 a_uiPoints, class ABlobRewardScreen* a_pRewardScreen );

public:
	TBOOL                 m_bUsed; // 0x0C TTRUE by default, cleared for the unused save slots
	EnSaveData::SlotData* m_pData; // 0x10 the data of the save slot (starts with the name)
};

//-----------------------------------------------------------------------------
// Purpose: the profiles of the save slots
//-----------------------------------------------------------------------------
class APlayerProfileManager
    : public Toshi::TObject
    , public Toshi::TSingleton<APlayerProfileManager>
{
public:
	TDECLARE_CLASS( APlayerProfileManager, Toshi::TObject );

	static TCONSTEXPR_VAR TINT NUMPROFILES = 4; // EnSaveData::SAVEGAME_SLOT_COUNT

public:
	APlayerProfileManager();
	~APlayerProfileManager();

	void            SetCurrentProfile( TINT a_iIndex );
	TBOOL           IsProfileUsed( TINT a_iIndex );
	void            SetProfileUnused( TINT a_iIndex );
	void            SetCurrentProfileName( const TWCHAR* a_wszName );
	const TWCHAR*   GetCurrentProfileName();
	APlayerProfile* GetCurrentProfile();

	// $deBlob: FUNCTION 005d2740
	APlayerProfile* GetProfile( TINT a_iIndex ) { return &m_aProfiles[ a_iIndex ]; }

	TINT GetCurrentProfileIndex() const { return m_iCurrentProfile; }

private:
	APlayerProfile m_aProfiles[ NUMPROFILES ]; // 0x04
	TINT           m_iCurrentProfile;          // 0x54 -1: none
};

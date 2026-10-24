#pragma once

//-----------------------------------------------------------------------------
// Purpose: the game's save data in memory: a buffer of SAVE_SIZE
// bytes with a header ("BB", size, CRC), the slots of the profiles and the options (SaveExtra).
// The buffer is what SaveLoad reads and writes as is (SaveData.dat, SaveLoad_Win.cpp).
//-----------------------------------------------------------------------------
class EnSaveData
{
public:
	static TCONSTEXPR_VAR TINT    SAVEGAME_SLOT_COUNT = 4;
	static TCONSTEXPR_VAR TUINT32 SAVE_SIZE           = 0xC88;

	// The medals and awards of a level (SlotData::m_auiMedals, the sprint medals of SlotData::m_auiSprintMedals)
	enum AWARD : TUINT32
	{
		AWARD_Bronze    = BITFLAG( 0 ),
		AWARD_Silver    = BITFLAG( 1 ),
		AWARD_Gold      = BITFLAG( 2 ),
		AWARD_Paint25   = BITFLAG( 4 ),
		AWARD_Paint50   = BITFLAG( 5 ),
		AWARD_Paint75   = BITFLAG( 6 ),
		AWARD_Paint100  = BITFLAG( 7 ),
		AWARD_Challenge = BITFLAG( 8 ),
		AWARD_Time      = BITFLAG( 9 ),
		AWARD_Styles    = BITFLAG( 10 ),
		AWARD_Trees     = BITFLAG( 11 ),
		AWARD_Landmarks = BITFLAG( 12 ),
		AWARD_Bill      = BITFLAG( 13 ),
		AWARD_Transform = BITFLAG( 14 ),
		AWARD_Raydian   = BITFLAG( 15 ),
		AWARD_Vista     = BITFLAG( 16 ),
	};

	// The "New" labels of the menus (SlotData::m_uiNewFlags, m_uiNewFlagsUnseen)
	enum NEWFLAG : TUINT32
	{
		NEWFLAG_Cutscenes      = BITFLAG( 0 ),
		NEWFLAG_Movies         = BITFLAG( 1 ),
		NEWFLAG_Gallery        = BITFLAG( 2 ),
		NEWFLAG_Extras         = BITFLAG( 4 ),
		NEWFLAG_FreePaint      = BITFLAG( 5 ),
		NEWFLAG_World          = BITFLAG( 6 ),  // << world: a world of Story
		NEWFLAG_FreePaintWorld = BITFLAG( 16 ), // << world: a world of Free Paint
	};

	// The "New" labels of SlotData::m_uiNewFlagsExtended: bits 0-8 are the multiplayer levels
	enum NEWFLAGEXT : TUINT32
	{
		NEWFLAGEXT_BlobParty = BITFLAG( 9 ),
	};

	struct Header
	{
		TCHAR   m_szID[ 4 ]; // 0x0 "BB"
		TUINT32 m_uiSize;    // 0x4 SAVE_SIZE
		TUINT32 m_uiCRC;     // 0x8
	};

	struct MedalTimes
	{
		TFLOAT m_fBronzeTime; // 0x0 the time of the medals (GetMedalTime)
		TFLOAT m_fSilverTime; // 0x4
		TFLOAT m_fGoldTime;   // 0x8
	};

	// The data of a profile (0x314 bytes)
	struct SlotData
	{
		SlotData();

		// The bit a_uiBit of m_auiLevelFlags[ a_iLevel ] (bit 0: the level has stats)
		// $deBlob: FUNCTION 005e3520
		TBOOL IsLevelFlagSet( TINT a_uiBit, TINT a_iLevel ) const { return ( m_auiLevelFlags[ a_iLevel ] & ( 1 << a_uiBit ) ) != 0; }

		// Any of the bits a_uiMask of the medals and awards of world a_iWorld (m_auiMedals)
		// $deBlob: FUNCTION 005e3500
		TBOOL HasMedalFlags( TUINT32 a_uiMask, TINT a_iWorld ) const { return ( a_uiMask & m_auiMedals[ a_iWorld ] ) != 0; }

		// The time of the best medal of a world (m_auiMedals) and of a sprint level (m_auiSprintMedals)
		TFLOAT GetMedalTime( TINT a_iWorld ) const;
		TFLOAT GetSprintMedalTime( TINT a_iSlot ) const;

		// The results of a level (APlayerProfile::RecordLevelResult): the medals and awards of a world (m_auiMedals, the
		// best medal and the best award are kept), the mode played, the best score and the best time of the medal
		void AddMedals( TINT a_iWorld, TUINT32 a_uiMedals, class ABlobRewardScreen* a_pRewardScreen );
		void RecordBest( TINT a_iWorld, TUINT8 a_uiMode, TFLOAT a_fTime, TUINT32 a_uiPoints, TINT a_iMedal );

		// The results of a sprint level (APlayerProfile::RecordSprintResult): its medals (m_auiSprintMedals, only the best one is
		// kept), the best time of a medal (m_aSprintMedalTimes) and the best count of its challenges (m_auiSprintBestCount, of m_auiSprintTotal)
		void AddSprintMedals( TINT a_iSlot, TUINT32 a_uiMedals );
		void RecordSprintMedalTime( TINT a_iSlot, TFLOAT a_fTime, TINT a_iMedal );
		void RecordSprintBest( TINT a_iLevel, TUINT32 a_uiCount, TUINT32 a_uiTotal );

		// The number of awards the worlds have (m_uiNumAwards)
		TINT CountAwards();

		// The unlocks: each one not unlocked yet is marked "New" in the menus and added to the reward screen
		void UnlockWorld( TINT a_iWorld, class ABlobRewardScreen* a_pRewardScreen );
		void UnlockWorldMode( TINT a_iWorld, TINT a_iMode, class ABlobRewardScreen* a_pRewardScreen );
		void UnlockChallenge( TINT a_iWorld, TINT a_iChallenge, class ABlobRewardScreen* a_pRewardScreen );
		void UnlockMovie( TINT a_iMovie, class ABlobRewardScreen* a_pRewardScreen );
		void UnlockBlobParty( TINT a_iItem, class ABlobRewardScreen* a_pRewardScreen );
		void UnlockVideo( TINT a_iVideo, class ABlobRewardScreen* a_pRewardScreen );
		void UnlockGalleryItem( TINT a_iItem, class ABlobRewardScreen* a_pRewardScreen );

		void Reset();

		// NOTE: always TTRUE on PC (EnSaveData::Validate checks every used slot with it)
		// $deBlob: FUNCTION 005e3690
		TBOOL Validate() const { return TTRUE; }

		TWCHAR     m_wszName[ 12 ];            // 0x000
		TINT       m_iUnlockedWorlds;          // 0x018 1 by default (bits)
		TINT       m_iUnlockedChallenges;      // 0x01C
		TINT       m_iUnlockedBlobParty;       // 0x020 27 by default
		TUINT8     m_auiUnlockedModes[ 10 ];   // 0x024 1 by default (bits: the unlocked modes of the world)
		TUINT8     m_auiLevelFlags[ 10 ];      // 0x02E bits (IsLevelFlagSet)
		TUINT32    m_auiMedals[ 10 ];          // 0x038
		TUINT32    m_auiSprintMedals[ 20 ];    // 0x060
		MedalTimes m_aMedalTimes[ 10 ];        // 0x0B0
		TUINT32    m_auiBestPoints[ 10 ];      // 0x128
		TUINT32    m_auiSprintBestCount[ 20 ]; // 0x150
		TUINT32    m_auiSprintTotal[ 20 ];     // 0x1A0
		MedalTimes m_aSprintMedalTimes[ 20 ];  // 0x1F0
		TINT       m_iUnlockedVideos;          // 0x2E0
		TINT       m_iUnlockedMovies;          // 0x2E4 1 by default
		TUINT16    m_uiUnlockedGalleryItems;   // 0x2E8
		TINT       m_iUnlockedTracks;          // 0x2EC
		TINT       m_iUnk2F0;                  // 0x2F0
		TUINT32    m_uiNumAwards;              // 0x2F4
		TUINT32    m_uiNewFlags;               // 0x2F8 the "New" labels of the menus (ABlobMenu)
		TUINT32    m_uiNewFlagsUnseen;         // 0x2FC
		TUINT32    m_uiNewFlagsExtended;       // 0x300
		TUINT32    m_uiNewFlagsExtendedUnseen; // 0x304
		TUINT32    m_auiUnk308[ 3 ];           // 0x308

		// Marks the full screen help topic a_iTopic as seen (m_auiUnk308[ 2 ])
		// $deBlob: FUNCTION 00645a60
		void SetHelpTopicSeen( TINT a_iTopic )
		{
			TASSERT( a_iTopic >= 0 );
			m_auiUnk308[ 2 ] |= 1 << a_iTopic;
		}

		TBOOL GetNewFlag( TUINT32 a_uiNewFlag ) const { return ( m_uiNewFlags & a_uiNewFlag ) != 0; }

		// $deBlob: FUNCTION 006308c0
		void  ClearNewFlag( TUINT32 a_uiNewFlag ) { m_uiNewFlags &= ~a_uiNewFlag; }
		TBOOL GetNewFlag_extended( TUINT32 a_uiNewFlag ) const { return ( m_uiNewFlagsExtended & a_uiNewFlag ) != 0; }
	};

	// A profile (0x318 bytes)
	struct SaveGameSlot
	{
		SaveGameSlot() { m_bUsedSlot = TFALSE; }

		TBOOL    m_bUsedSlot; // 0x000
		SlotData m_oData;     // 0x004
	};

	// The options (0x1C bytes)
	struct SaveExtra
	{
		enum FLAG : TUINT32
		{
			FLAG_InvertCamera = BITFLAG( 1 ), // ABlobFollowCamera
			FLAG_HelpEnabled  = BITFLAG( 4 ), // the help messages
		};

		TINT    m_iUnk00;       // 0x00 2 by default
		TINT    m_iSFXVolume;   // 0x04 8 by default, 0-10 (AOptions)
		TINT    m_iMusicVolume; // 0x08 8 by default, 0-10 (AOptions)
		TFLOAT  m_fCameraSpeed; // 0x0C 0.5 by default, clamped to [0.5, 1.0] (ABlobFollowCamera)
		TUINT32 m_uiFlags;      // 0x10 FLAG
		TINT    m_iUnk14;       // 0x14
		TINT    m_iUnk18;       // 0x18

		// The defaults (the camera speed clamped unless s_iUnk0099698c is set)
		SaveExtra();
		void Reset();

		// Clamps the camera speed; always TTRUE
		TBOOL Validate();

		// Called after a save was loaded; NOTE: empty on PC
		// $deBlob: FUNCTION 005e3820
		void Sync() {}

		// Flags of two SUBTITLES_OPTIONS (their bits), passed to ASubtitleManager::PlaySubtitle by AMovieState
		// $deBlob: FUNCTION 005e3850
		static TUINT32 ConvertSubtitleOptionsToFlag( TINT a_eOption1, TINT a_eOption2 )
		{
			return ( 1 << a_eOption1 ) | ( 1 << a_eOption2 );
		}
	};

	struct SaveData
	{
		Header       m_oHeader;                               // 0x000
		SaveGameSlot m_aSaveGameSlots[ SAVEGAME_SLOT_COUNT ]; // 0x00C
		SaveExtra    m_oExtra;                                // 0xC6C

		void ResetExtra();
	};

public:
	EnSaveData();
	~EnSaveData();

	// Copies the data of another save
	void Copy( EnSaveData* a_pSaveData );

	// The default options
	void ResetExtra();

	// Clears the buffer: the header, no profiles, the default options and the CRC
	void Reset();

	// Writes the header ("BB", SAVE_SIZE, CRC 0)
	void SetHeader();

	// Computes the CRC of the buffer (the CRC field counted as 0) and stores it in the header
	void SetCRC();

	// The CRC of the buffer with the CRC field counted as 0
	TUINT32 CalculateCRC32();

	// TTRUE if the header and the CRC are right and the profiles are valid; clamps the camera speed
	TBOOL Validate();

	// Reads a whole file into a new buffer (TMemalign 0x20)
	static void GetRawDataFromFile( const TCHAR* a_szFileName, TUINT8** a_ppData, TUINT32* a_pSize );

	// $deBlob: FUNCTION 005e3d90
	TUINT8* GetBuffer() const { return m_pBuffer; }
	// $deBlob: FUNCTION 005e3da0
	TUINT32 GetBufferSize() const { return m_uiBufferSize; }

	SaveData* GetSaveData() const { return m_pSaveData; }

	// The profile a_iSlot
	// $deBlob: FUNCTION 005d2980
	SaveGameSlot& GetSlot( TINT a_iSlot ) const
	{
		TASSERT( a_iSlot >= 0 && a_iSlot < SAVEGAME_SLOT_COUNT );
		return m_pSaveData->m_aSaveGameSlots[ a_iSlot ];
	}

private:
	TUINT32   m_uiBufferSize; // 0x0
	TUINT32   m_uiSize;       // 0x4
	TUINT8*   m_pBuffer;      // 0x8
	SaveData* m_pSaveData;    // 0xC the buffer
};

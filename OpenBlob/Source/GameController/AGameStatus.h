#pragma once
#include "ALevelInformation.h"
#include "GameController/APointDataFunction.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <ToshiTools/T2DynamicVector.h>
#include <File/TTRB.h>
#include "Terrain2/TerrainTypes.h"

namespace Toshi
{
class TXUIControl;
class TXUIElement;
class TVector4;
class TMatrix44;
class TTexture;
} // namespace Toshi

class ABaseEntity;
class AIOEntity;
class ABuildingInstance;
class ATreeInstance;
class AModelInstance2;
class ACamera;
class ABlobScoreFloatHud;
class ABlobScoreHud;
class APropDoor;

//-----------------------------------------------------------------------------
// Purpose: the state of the game: the current level, the selected world and mode of the level selection, the
// multiplayer game type and the per-level data (the players' scores, buildings painted, challenges...).
// NOTE: only the parts the frontend, the level selection and the level start use are ported yet
//-----------------------------------------------------------------------------
class AGameStatus : public Toshi::TSingleton<AGameStatus>
{
	// Writes the terrain instance counts (0x170-0x17C) and the counters it resets each frame
	friend class ATerrain2Interface;

public:
	// The modes of the level selection
	enum LevelMode
	{
		Mode_Story,
		Mode_FreePaint,
	};

	// The multiplayer game types
	enum GameType
	{
		GAMETYPE_MP_NONE  = -1,
		GAMETYPE_MP_COUNT = 4,
	};

	// The single player levels
	enum SPLevel
	{
		SPLevel_Invalid = -1,
		SPLevel_LastResort,
		SPLevel_NUMOF = 10,
	};

	// The sprint levels (two per world)
	enum SprintLevel
	{
		SprintLevel_NUMOF = 20,
	};

	// The data of a player in the level (0x4C bytes)
	struct PlayerData
	{
		PlayerData();

		// Clears the data
		void Reset();

		// The per frame update of a player (LevelData::Update): its combo timer, the medals and awards it reaches and
		// the environment's colour blend (m_auiUnk3C[ 0 ] the awards, [ 1 ] the blend as a float)
		void Update( TFLOAT a_fDeltaTime );

		// The chain (combo) of the player
		// $deBlob: FUNCTION 005c0be0
		void StartChain( TINT a_eMode )
		{
			TASSERT( 0 != a_eMode );
			m_iComboState = a_eMode;
			m_uiComboTime = *TSTATICCAST( TUINT32, AGameStatus::GetBlobScoringProperties() );
			m_iComboCount = 1;
		}

		// $deBlob: FUNCTION 005c0790
		void IncrementChain()
		{
			TASSERT( 0 != m_iComboState );
			m_iComboCount++;
		}

		union
		{
			TUINT32 m_auiUnk00[ 19 ];
			struct
			{
				TINT    m_iPoints;           // 0x00 the score
				TUINT32 m_uiUnk04;           // 0x04
				TINT    m_iBuildingsPainted; // 0x08
				TUINT32 m_auiProgress[ 9 ];  // 0x0C
				TUINT32 m_uiComboTime;       // 0x30 BlobScoringProperties+0x00 when a combo goes on
				TINT    m_iComboCount;       // 0x34 the multiplier index
				TINT    m_iComboState;       // 0x38 1/4: a combo runs
				TUINT32 m_auiUnk3C[ 4 ];     // 0x3C
			};
		};
	};

	// The data of the current level (0x814 bytes)
	struct LevelData
	{
		LevelData();
		// NOTE: the compiler's destruction of the lists (called by AGameStatus's ctor unwind and dtor)
		// $deBlob: FUNCTION 005c0450
		~LevelData() = default;

		// Resets the data for the level about to be loaded
		void Init( ALevelInformation::LevelProperties* a_pLevelProperties );

		// Clears the instance and challenge counts (0x140-0x16C) and m_uiUnk780 (the level unload, ResetLevelReferences);
		void ClearCounts();

		// The combo of a player (PlayerData 0x34-0x38: its count and state, the scoring time at 0x30); the states are
		// 1 (painted in a colour), 4 (inked) and 0 (none)
		TINT    GetComboState( TINT a_iPlayer ) const;
		void    IncrementComboCount( TINT a_iPlayer );
		TUINT32 StartCombo( TINT a_iPlayer, TINT a_iState );
		void    EndCombo( TINT a_iPlayer );

		// The per frame update of the level: the glint, the players' combos, the level timer and its warnings (called
		// with this = m_oLevelData); the level completion calls it with 0.0
		void Update( TFLOAT a_fDeltaTime );

		// Counts a building painted by a_iPlayer and scores it
		void OnBuildingPainted( ABuildingInstance* a_pBuilding, TINT a_iPlayer, const Toshi::TVector4* a_pPosition, TINT a_iBonus, TBOOL a_bNoPopup );

		// The time is over (m_fTimeLeft <= 0)
		TBOOL IsTimeUp() const;
		// The level has a target time and m_fLevelTime is within it
		TBOOL IsWithinTargetTime() const;

		// Uncounts a building (inked, or taken by another player)
		void OnBuildingUnpainted( ABuildingInstance* a_pBuilding, TINT a_iPlayer, TBOOL a_bScore, const Toshi::TVector4* a_pPosition, TINT a_iBonus );

		// Adds a score (and its bonus) to a player, the music reacts
		void AddScore( TINT a_iPlayer, TINT a_iScore, TINT a_iBonus );

		// Scores a kill of a_iPlayer at a_rPosition (an enemy, a jetbike): the score float pops up with the combo
		// multiplier (single player only), and an inked combo (state 2) restarts its timer
		void AddKillScore( TINT a_iPlayer, const Toshi::TVector4& a_rPosition, TINT a_iScore );

		// Adds points to a player's score (clamped at 0)
		void AddPoints( TINT a_iPlayer, TINT a_iPoints );

		// The civilians freed by a player (PlayerData 0x14, clamped to the level's, m_auiLevelCounts[ 6 ]); the civilian
		// challenge takes them back
		void AddCivsFreed( TINT a_iPlayer, TINT a_iCount );

		void   IncrementUnk28( TINT a_iPlayer );                  // PlayerData 0x28
		void   SetUnk2C( TINT a_iPlayer, TUINT32 a_uiValue );     // PlayerData 0x2C
		void   SetUnk04( TINT a_iPlayer, TUINT32 a_uiValue );     // PlayerData 0x04
		TFLOAT GetLandmarksCompletedPercentage( TINT a_iPlayer ); // PlayerData 0x1C of the level's landmarks (m_auiLevelCounts[ 2 ])

		// PlayerData 0x44 (a pattern collected, APickupPattern) and 0x48
		void IncrementUnk44( TINT a_iPlayer );

		// The score float HUD of a player (rotates through them in single player)
		ABlobScoreFloatHud* GetScoreFloatHud( TINT a_iPlayer );

		// $deBlob: FUNCTION 005bbe10
		void SetScoreFloatHud( TINT a_iPlayer, ABlobScoreFloatHud* a_pHud )
		{
			TASSERT( a_iPlayer >= 0 && a_iPlayer < 4 );
			m_apScoreFloatHud[ a_iPlayer ] = a_pHud;
		}

		// $deBlob: FUNCTION 005bb8a0
		TINT GetPoints( TINT a_iPlayer ) const
		{
			TASSERT( a_iPlayer >= 0 && a_iPlayer < 4 );
			return m_aPlayerData[ a_iPlayer ].m_iPoints;
		}

		// The percentage of the buildings and landmarks a player painted
		TFLOAT GetPaintPercentage( TINT a_iPlayer ) const;

		// The help collectables (APropTriggerHelp of HelpType 4) found (AGameStatus+0x50) and in the level
		// (AGameStatus+0x184, counted by APropTriggerHelp::ProcessParams)
		TINT GetNumHelpCollected() const { return TINT( m_aPlayerData[ 0 ].m_auiProgress[ 1 ] ); }
		void AddHelpCollected() { m_aPlayerData[ 0 ].m_auiProgress[ 1 ]++; }

		// A bonus pickup of the level was created (APickupBonus::ProcessParams)
		// $deBlob: FUNCTION 007b04b0
		void AddBonusPickup() { m_auiUnk160[ 1 ]++; }

		// a_iPlayer collected a bonus pickup (APickupBonus::OnCollected)
		void AddBonusCollected( TINT a_iPlayer );

		// The trees painted by a player (AGameStatus+0x4C, ATreeInstance)
		void AddTreesPainted( TINT a_iPlayer, TINT a_iCount ) { m_aPlayerData[ a_iPlayer ].m_auiProgress[ 0 ] += a_iCount; }

		// A civilian of a_iPlayer is released (ACiv)
		void AddCivReleased( TINT a_iPlayer );

		// A civilian freed by a_iPlayer: the count, the score (combo state 3) and the HUD text
		void OnCivFreed( TINT a_iPlayer, const Toshi::TVector4* a_pPosition, TINT a_iScore );

		// A landmark completed (a_iCount) by a_iPlayer
		void AddLandmarksCompleted( TINT a_iPlayer, TINT a_iCount );
		TINT GetNumHelpCollectables() const { return TINT( m_auiLevelCounts[ 5 ] ); }
		void AddHelpCollectable() { m_auiLevelCounts[ 5 ]++; }

		// The civilians of the level that can be liberated (AGameStatus+0x188, counted by ACiv::ProcessParams and by the
		// paintable blocks, ABuildingManager::SetupPaintableBlocks)
		void AddCivilian() { m_auiLevelCounts[ 6 ]++; }
		void AddCivilians( TINT a_iNumCivilians ) { m_auiLevelCounts[ 6 ] += a_iNumCivilians; }
		void RemoveCivilian() { m_auiLevelCounts[ 6 ]--; } // ALandmarkInstance::UncountCivs

		// A grass bomb of the level (AGameStatus+0x180, counted by APropGrassBomb::Create)
		void AddGrassBomb() { m_auiLevelCounts[ 4 ]++; }

		// A grass bomb was collected by a_iPlayer (PlayerData 0x24, APropGrassBomb::HandleEntityIO)
		void AddGrassBombCollected( TINT a_iPlayer );

		// The blimp of the level was painted (ACar::SetBlimpColoured): the achievement 0x10000 of the first player is
		// unlocked and the HUD shows it
		void OnBlimpPainted();

		// Sets m_pMessageControl and its frame times
		void SetMessageControl( Toshi::TXUIControl* a_pControl );

		// Shows a_wszText: on the score float HUD of a_iPlayer at a_pPosition (a_iType 2, 3: a time pickup), else on
		// m_pMessageControl with an animation (4 "TimeBonus", 5 "TimeRunningOut", 6 hidden, 7 "PatternCollected" with the
		// pattern count, 8 "ChallengeCompleted", 9 "AchievementUnlocked", 10 "Graydian"); 1: nothing
		void ShowMessage( const TWCHAR* a_wszText, TINT a_iType, const Toshi::TVector4* a_pPosition, TINT a_iPlayer );

		// Sets the goal of the goal HUD (m_iGoal, -1: none, hides it) and m_uiUnk774
		void SetGoal( TINT a_iGoal, TUINT32 a_uiUnk774 );

		// The combo multiplier of the a_iCount-th combo (BlobScoringProperties 0x14-0x2C)
		static TINT GetComboMultiplier( TINT a_iCount );

		TBOOL                               m_bMultiplayer;            // 0x000 the level's name starts with "mp_"
		TFLOAT                              m_fTimeLimit;              // 0x004 in seconds, -1: none (the level's, 15 minutes by default)
		TFLOAT                              m_fTimeLeft;               // 0x008
		TBOOL                               m_bUnk00C;                 // 0x00C
		PlayerData                          m_aPlayerData[ 4 ];        // 0x010 APlayerManager::MAX_PLAYERS
		TUINT32                             m_auiLevelCounts[ 7 ];     // 0x140 [0]: m_iNumBuildings
		TUINT32                             m_uiUnk15C;                // 0x15C
		TUINT32                             m_auiUnk160[ 2 ];          // 0x160
		TINT                                m_iNumChallenges;          // 0x168 without the null and sub challenges
		TINT                                m_iNumCompletedChallenges; // 0x16C
		TWCHAR                              m_wszUnk170[ 128 ];        // 0x170
		TUINT8                              m_auiUnk270[ 0x100 ];      // 0x270
		TWCHAR                              m_wszMessage[ 0x100 ];     // 0x370 the text of ShowMessage on m_pMessageControl
		TUINT8                              m_auiUnk570[ 0x204 ];      // 0x570
		TUINT32                             m_uiUnk774;                // 0x774
		TINT                                m_iGoal;                   // 0x778 -1
		TBOOL                               m_bGoalCheat;              // 0x77C
		TUINT32                             m_uiUnk780;                // 0x780
		Toshi::T2GenericDList               m_oList784;                // 0x784
		Toshi::T2GenericDList               m_oList78C;                // 0x78C
		Toshi::T2GenericDList               m_oList794;                // 0x794
		ABlobScoreFloatHud*                 m_apScoreFloatHud[ 4 ];    // 0x79C per player (ABlobHud::OnCreated)
		TINT                                m_iLastScoreFloatHud;      // 0x7AC -1, the last used score float HUD
		Toshi::TXUIElement*                 m_pGoalHud;                // 0x7B0 ABlobHud's element 0x19
		ABlobScoreHud*                      m_pScoreHud;               // 0x7B4 (ABlobHud::OnCreated)
		Toshi::TXUIControl*                 m_pMessageControl;         // 0x7B8 its visual plays "EndNormal"
		TFLOAT                              m_fEndGraydianFrame;       // 0x7BC m_pMessageControl's "EndGraydian" frame
		TFLOAT                              m_fRaydianFreeFrame;       // 0x7C0 m_pMessageControl's "RaydianFree" frame
		ALevelInformation::LevelProperties* m_pLevelProps;             // 0x7C4 (assert "rLevelData.m_pLevelProps->m_sprintLevel")
		TUINT8                              m_auiUnk7C8[ 0x40 ];       // 0x7C8
		void*                               m_pMedalsHud;              // 0x808
		TFLOAT                              m_fLevelTime;              // 0x80C
		TFLOAT                              m_fTargetTime;             // 0x810 of the SPLevel, -1: none
	};

	// A rank of the multiplayer results (0x10 bytes, assert "m_aoRanks[0].m_iPlayer>=0")
	struct Rank
	{
		TINT   m_iRank;    // 0x0 1: first (equal scores share a rank)
		TINT   m_iPlayer;  // 0x4 -1: none
		TINT   m_iScore;   // 0x8 the player's points
		TFLOAT m_fPercent; // 0xC of all the players' points
	};

	// A checkpoint of the level (8 bytes): the IO entities whose input 0x39 has the parameter 1 register themselves
	// (IO action 0x3A), the actions 0x38/0x39 activate and deactivate them
	struct CheckPoint
	{
		AIOEntity* m_pEntity;    // 0x0
		TBOOL      m_bActivated; // 0x4
	};

public:
	AGameStatus();
	~AGameStatus();

	// Adds a bonus score (critters, civs...) to a player
	void AddBonusScore( TINT a_iPlayer, TINT a_iScore, const Toshi::TVector4* a_pPosition );

	// Whether a_pBuilding is one of the six m_apNextBuildingPaintable slots
	TBOOL IsNextPaintableBuilding( const ABuildingInstance* a_pBuilding ) const;

	// The level restart (AInGameState): clears the level's state, the multiplayer modes set up their buildings and
	// scores again
	void ResetLevel();

	// Removes the projections of the six m_apNextBuildingPaintable buildings and clears the slots (multiplayer)
	void ClearNextPaintableProjections();

	// Empties m_vecCheckPoints (the level unload, AInGameState::OnRemoval)
	// $deBlob: FUNCTION 005bc980
	void ClearCheckPoints() { m_vecCheckPoints.Clear(); }

	// Forgets the level's point data, gates, grass bomb, counts and the m_apNextBuildingPaintable buildings (the level unload,
	// AInGameState::OnRemoval; not after the last level's completion)
	void ResetLevelReferences();

	// A building's paint challenge projection gets removed (game type 2)
	void OnBuildingPaintChanged( ABuildingInstance* a_pBuilding );

	// Sets the paint challenge projection of a building
	static void __stdcall SetBuildingProjection( ABuildingInstance* a_pBuilding, TINT a_iProjUDataNum, TBOOL a_bUnk );

	// Removes the projection of a building, returns it (-1: none)
	static TINT __stdcall RemoveBuildingProjection( ABuildingInstance* a_pBuilding );

	// The free projections are a stack (AStack, m_iNumFreeProjections is its m_iStackIndex)
	// A projection goes back to the free ones
	void FreeProjection( TINT a_iProjUDataNum );
	// A free projection, -1: none
	TINT AllocateProjection();
	// No free projection left
	void ClearProjections();

	// Every projection is free again (m_aiFreeProjections = 0..89, the boss level, ABoss::Tick)
	void ResetFreeProjections();

	// The glint of a building's projection (m_iGlintIndex) fades (paint challenge)
	static void __stdcall UpdateGlint( ABuildingInstance* a_pInstance, TFLOAT a_fDeltaTime );
	// The glint of a building's projection follows the camera (paint challenge)
	static void __stdcall UpdateGlintCamera( ABuildingInstance* a_pInstance, ACamera* a_pCamera, TINT a_iColour, TBOOL a_bUnk );

	// Game type 2: updates the glints of the m_apNextBuildingPaintable buildings for the camera of the viewport being
	// rendered, in the start colour of its player's blob (ARenderer::MainScene)
	void UpdateNextBuildingGlintCameras();
	static void __stdcall UpdateProjectionGlint( TINT a_iProjUDataNum, TFLOAT a_fDeltaTime );
	static void __stdcall UpdateProjectionCamera( TINT a_iProjUDataNum, AModelInstance2* a_pInstance, ACamera* a_pCamera, TINT a_iColour, TBOOL a_bAlpha, TBOOL a_bTree );

	// The same for the trees (ATreeInstance, the projection is on the model shown)
	static void __stdcall SetTreeProjection( TerrainTypes::CellInstance* a_pInstance, TINT a_iProjUDataNum, TBOOL a_bUnk );
	static void __stdcall UpdateTreeGlint( ATreeInstance* a_pInstance, TFLOAT a_fDeltaTime );
	static void __stdcall UpdateTreeGlintCamera( ATreeInstance* a_pInstance, ACamera* a_pCamera, TINT a_iColour, TBOOL a_bUnk );

	// The projection a_iProjUDataNum of TWorldShaderHAL on an instance (its meshes and, for a building, the ones of
	// its civilian spawners) from the height of the bounds of the model a_iModel (a T2ResourcePtr ID, 0: the
	// instance's)
	static void __stdcall SetInstanceProjection( TerrainTypes::CellInstance* a_pInstance, TINT a_iProjUDataNum, TBOOL a_bUnk, TINT a_iModel );

	// Takes the projection off an instance
	static void __stdcall RemoveInstanceProjection( TerrainTypes::CellInstance* a_pInstance );

	// The projection texture of a building's paint (always TNULL)
	// $deBlob: FUNCTION 005bf080
	static Toshi::TTexture* __stdcall GetProjectionTexture( ABuildingInstance* a_pBuilding ) { return TNULL; }

	// Multiplayer: a_iKillerPlayerNumber inked a_iPlayerToBeKilled and scores for it (more when the inked one leads)
	void OnPlayerKilled( TINT a_iKillerPlayerNumber, TINT a_iPlayerToBeKilled );

	// The blob of a_iPlayer died (ABlobEntity::OnOutOfPoints): the death's score (0xB0 of BlobScoringProperties) in
	// the multiplayer game types but 1
	void OnPlayerDied( TINT a_iPlayer );

	// $deBlob: FUNCTION 00591830
	LevelData& GetLevelData() { return m_oLevelData; }

	// The level start: the blob attached to the level start gate's animation (ABlobFollowCamera::Update reads it)
	TBOOL  IsLevelStart() const { return m_bLevelStart; }
	void   SetLevelStart( TBOOL a_bLevelStart, TBOOL a_bRespawn );
	TBOOL  GetLevelStartCamera( Toshi::TMatrix44& a_rOutMatrix, TFLOAT& a_rfWeight );
	TBOOL  GetLevelStartBlobPosition( Toshi::TVector4& a_rOutPosition );
	TFLOAT GetLevelStartAnimTime();

	APropDoor* GetLevelStartGate() const { return m_pLevelStartGate; }
	// $deBlob: FUNCTION 00789790
	void SetLevelStartGate( APropDoor* a_pGate )
	{
		TASSERT( m_pLevelStartGate == TNULL );
		m_pLevelStartGate = a_pGate;
	}
	// $deBlob: FUNCTION 006564c0
	APropDoor* GetSprintGate() const { return m_pSprintGate; }

	// Sends MESSAGE_OPEN to the sprint gate (the sprint's last challenge is done)
	void OnSprintChallengesCompleted();
	// $deBlob: FUNCTION 00789780
	void SetSprintGate( APropDoor* a_pGate ) { m_pSprintGate = a_pGate; }

	// The level the game state loads next (ALoadLevelState), a_iLevel >= -1
	TBOOL SetCurrentLevel( TINT a_iLevel );

	// The checkpoint the level starts at (m_iCurrentCheckpoint), -1: none
	TBOOL       SetCurrentCheckpoint( TINT a_iCheckpoint );
	TBOOL       StartAtCheckpoint();
	CheckPoint* FindCheckPoint( AIOEntity* a_pEntity );
	// The a_iIndex-th checkpoint, TNULL if there are fewer
	CheckPoint* GetCheckPoint( TINT a_iIndex );
	static void UpdateCurrentCheckPoint();

	// The IO callbacks of the checkpoints (AIOEntity::InitialiseStatic: the actions 0x38, 0x39 and 0x3A)
	static void __stdcall OnCheckPointActivate( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );
	static void __stdcall OnCheckPointDeactivate( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );
	static void __stdcall OnCheckPointRegister( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );

	// Resets the level data for the level about to be loaded
	void InitLevelData( ALevelInformation::LevelProperties* a_pLevelProperties );

	// The kind of the single player levels' slots (two per world: the challenges of the level selection)
	static TBOOL IsPaintSlot( TINT a_iSlot );
	static TBOOL IsCombatSlot( TINT a_iSlot );
	static TBOOL IsRaceSlot( TINT a_iSlot );
	static TBOOL IsLandmarkSlot( TINT a_iSlot );

	// The ALevelInformation name of a single player level
	static const TCHAR* GetSPLevelName( TINT a_eLevel );

	// The ALevelInformation name of a sprint level
	static const TCHAR* GetSprintLevelName( TINT a_eSprintLevel );

	// The single player or sprint level name a_szName (an extension is ignored) is, TNULL if none
	static const TCHAR* FindLevelName( const TCHAR* a_szName );

	// The per frame counters of the drawn instances near the camera (painted ones weighted, all paintable ones),
	// written by RenderLightmappedInstance
	static void AddVisiblePaintedInstances( TINT a_iCount ) { s_uiNumVisiblePaintedInstances += a_iCount; }
	static void AddVisiblePaintableInstance() { s_uiNumVisiblePaintableInstances++; }

	// The scoring properties of Blob
	static void* GetBlobScoringProperties() { return s_pBlobScoringProperties; }

	// $deBlob: FUNCTION 00591820
	TINT GetCurrentLevel() const { return m_iCurrentLevel; }
	TINT GetMultiPlayerGameType() const { return m_iMultiPlayerGameType; }
	void SetMultiPlayerGameType( TINT a_iType );

	APointDataFunction& GetPointData() { return m_oPointData; }
	TBOOL               IsMultiplayer() const { return m_oLevelData.m_bMultiplayer; }
	TFLOAT              GetTimeLeft() const { return m_oLevelData.m_fTimeLeft; }
	void                SetTimeLeft( TFLOAT a_fTimeLeft ) { m_oLevelData.m_fTimeLeft = a_fTimeLeft; }

	ALevelInformation::LevelProperties* GetLevelProperties() const { return m_oLevelData.m_pLevelProps; }

	// $deBlob: FUNCTION 005c0950
	TINT GetSPLevel() const { return m_SPLevel; }
	void SetSPLevel( TINT a_iLevel ) { m_SPLevel = a_iLevel; }

	// $deBlob: FUNCTION 00635e30
	TINT GetSelectedWorld() const { return m_iSelectedWorld; }
	void SetSelectedWorld( TINT a_iWorld ) { m_iSelectedWorld = a_iWorld; }

	// $deBlob: FUNCTION 005c0960
	TINT GetSelectedSlot() const { return m_iSelectedSlot; }
	void SetSelectedSlot( TINT a_iSlot ) { m_iSelectedSlot = a_iSlot; }

	// The picture ABlobGalleries opens (gallery.xur)
	// $deBlob: FUNCTION 006318f0
	void SetSelectedGallery( TINT a_iGallery ) { m_uiSelectedGallery = a_iGallery; }
	// $deBlob: FUNCTION 00631e40
	TINT GetSelectedGallery() const { return TINT( m_uiSelectedGallery ); }

	// The level is completed (the blob went through the exit door, APropTriggerLevelComplete)
	void OnLevelComplete();

	// The last life was lost (ABlobLifeLostHud): the level ends (complete, not won) and the input is disabled
	void OnGameOver();

	// The per frame update of the level (AInGameState::OnUpdate): the level data, the multiplayer game type, Blob loses
	// when the time limit runs out
	void Update( TFLOAT a_fDeltaTime );

	// The wings over the blob that is "it" (game type 1), hidden
	void SetItMarker( Toshi::TXUIElement* a_pElement );

	// Game type 1: a_iPlayer is "it", the wings in its colour go to its HUD panel
	void SetItPlayer( TINT a_iPlayer );

	// Game type 1: the wings follow the blob that is "it" while they animate, then hide
	void UpdateItMarker( TFLOAT a_fDeltaTime );

	// Game type 1: a_pBlob is "it" (painted by a Leechbot or a paint pool): the colour's message, the other blobs
	// become ink blobs, the crown goes to it
	void SetItBlob( ABaseEntity* a_pBlob );

	// Game type 1: a_pInker inked a_pInked and is "it" (as SetItBlob)
	void OnBlobInked( ABaseEntity* a_pInked, ABaseEntity* a_pInker );

	// Game type 1: no blob is painted any more, the blobs are ink bots
	void OnAllBlobsUnpainted();

	// Multiplayer game type 1: the "it" blob, then the timed end (UpdateTimedMatch)
	void UpdateItMatch( TFLOAT a_fDeltaTime );

	// Multiplayer game type 2: six glinting buildings near the blobs' centre wait to be painted (painted ones are
	// replaced); the match ends when the time runs out or no building is left
	void UpdateGlintMatch( TFLOAT a_fDeltaTime );

	// The message of a colour (ABlobColourComponent's index 1-7), TNULL for the others
	static const TWCHAR* GetColourMessage( TUINT32 a_uiColour );

	// Multiplayer: sorts the players by points into m_aoRanks (equal points share a rank), their share of all the
	// points, the crown on the leaders' blobs, m_iLeader (the leader) and the leaderboard HUD
	void UpdateRanks();

	// Multiplayer game types 0 and 3: the match ends when the time runs out, the leader wins
	void UpdateTimedMatch();

	// The awards of a level (16 bytes each): two locale strings (the title, the description), the flag in the level's
	// and the profile's medals (PlayerData 0x3C, SlotData m_auiMedals) and the name of its "Medal<name>On/Off" visuals
	struct Award
	{
		TINT          m_iString;
		TINT          m_iDescription;
		TUINT32       m_uiFlag;
		const TWCHAR* m_wszName;
	};

	static const Award s_aAwards[ 13 ];
	static TINT        s_iNumAwards;

	// The score summary of the results screen (24 bytes)
	struct ScoreSummary
	{
		TINT m_iPoints;       // 0x00 the player's points
		TINT m_iTimeBonus;    // 0x04 the time left (seconds) * BlobScoringProperties+0x4C
		TINT m_iRescueBonus;  // 0x08 the current player's BlobData+0x58 * BlobScoringProperties+0x64
		TINT m_iRankBonus;    // 0x0C BlobScoringProperties+0x54..0x60 by the level's rank flag (0x10..0x80), 0: none
		TINT m_iTotal;        // 0x10
		TINT m_iRankStringID; // 0x14 the locale string of the rank (0x43..0x46, 0x47: none)
	};

	// Fills a_pSummary from the level data (single player)
	static void CalculateScoreSummary( ScoreSummary* a_pSummary );

	static ScoreSummary s_oScoreSummary;

	// The level is complete and was won (0x844 and 0x845), the results screen follows
	// $deBlob: FUNCTION 005c8880
	TBOOL IsLevelWon() const { return m_bLevelComplete == TTRUE && m_bUnk845 == TTRUE; }

	// m_iLeader once the level is won, else -1 (ABlobEntity's game over state 13 hands it to ABlobHud::ShowWinner)
	// $deBlob: FUNCTION 00751ae0
	TINT GetLevelWonLeader() const { return ( m_bLevelComplete == TTRUE && m_bUnk845 == TTRUE ) ? m_iLeader : -1; }

	TINT GetLevelMode() const { return m_eLevelMode; }

	// A rank of the multiplayer results (m_aoRanks[ a_iIndex ], 0..3)
	// $deBlob: FUNCTION 006491f0
	const Rank& GetRankResult( TINT a_iIndex ) const { return m_aoRanks[ a_iIndex ]; }

	// The leaderboard HUD (0xA3C: set by its OnCreated, cleared by its dtor)
	class ABlobLeaderBoardHud* GetLeaderBoardHud() const { return *(class ABlobLeaderBoardHud**)&m_uiLeaderBoardHud; }
	// $deBlob: FUNCTION 006568e0
	void SetLeaderBoardHud( class ABlobLeaderBoardHud* a_pHud ) { *(class ABlobLeaderBoardHud**)&m_uiLeaderBoardHud = a_pHud; }

	// The rank of a player in the multiplayer results (m_aoRanks[].m_iRank), -1: none
	// $deBlob: FUNCTION 005bd9c0
	TINT GetRank( TINT a_iPlayer ) const
	{
		for ( TINT i = 0; i < 4; i++ )
		{
			if ( m_aoRanks[ i ].m_iPlayer == a_iPlayer )
				return m_aoRanks[ i ].m_iRank;
		}

		return -1;
	}
	// The level is complete (0x844)
	TBOOL IsLevelComplete() const { return m_bLevelComplete; }
	void  SetLevelMode( TINT a_eLevelMode ) { m_eLevelMode = a_eLevelMode; }

	// 0x00C (set when the boss grass bomb sequence ends, APropGrassBomb::Tick)
	void  SetUnk00C( TBOOL a_bUnk ) { m_bUnk00C = a_bUnk; }
	TBOOL GetUnk00C() const { return m_bUnk00C; }

	// 0xA40: the grass bomb that completes the boss level (APropGrassBomb), TNULL if none
	void                  SetBossGrassBomb( class APropGrassBomb* a_pBomb ) { m_uiBossGrassBomb = TUINT32( a_pBomb ); }
	class APropGrassBomb* GetBossGrassBomb() const { return TREINTERPRETCAST( class APropGrassBomb*, m_uiBossGrassBomb ); }

private:
	static Toshi::TTRB s_ScorePropertiesTRB;     // Data/Scoring/ScoreProperties.trb
	static void*       s_pBlobScoringProperties; // its BlobScoringProperties
	static TUINT32     s_uiNumVisiblePaintedInstances;
	static TUINT32     s_uiNumVisiblePaintableInstances;
	static TFLOAT      s_fGlintLerp; // set by LevelData::Update

private:
	TBOOL                              m_bUnk000;                      // 0x000
	Toshi::TXUIElement*                m_pItMarker;                    // 0x004 the wings over the blob that is "it" (game type 1, the split screen HUDs' element)
	TUINT32                            m_uiUnk008;                     // 0x008
	TBOOL                              m_bUnk00C;                      // 0x00C
	TINT                               m_iCurrentLevel;                // 0x010 the level index (ALevelInformation)
	TINT                               m_iCurrentCheckpoint;           // 0x014 -1: none
	TINT                               m_iMultiPlayerGameType;         // 0x018 -1: single player
	Toshi::T2DynamicVector<CheckPoint> m_vecCheckPoints;               // 0x01C at most 64
	LevelData                          m_oLevelData;                   // 0x030
	TBOOL                              m_bLevelComplete;               // 0x844 set by OnLevelComplete
	TBOOL                              m_bUnk845;                      // 0x845
	TINT                               m_iLeader;                      // 0x848 -1
	Rank                               m_aoRanks[ 4 ];                 // 0x84C
	TINT                               m_iNumFreeProjections;          // 0x88C -1, the count of m_aiFreeProjections
	TINT                               m_aiFreeProjections[ 90 ];      // 0x890 the freed projections
	TUINT32                            m_uiUnk9F8;                     // 0x9F8
	TUINT32                            m_uiUnk9FC;                     // 0x9FC
	ABuildingInstance*                 m_apNextBuildingPaintable[ 6 ]; // 0xA00 game type 2: the glinting buildings to paint next
	APointDataFunction                 m_oPointData;                   // 0xA18
	TINT                               m_SPLevel;                      // 0xA1C SPLevel, -1 none
	TINT                               m_iSelectedWorld;               // 0xA20 the world of the level selection, -1 none
	TINT                               m_iSelectedSlot;                // 0xA24 the challenge slot (world * 2 + 0/1), -1 none
	TINT                               m_eLevelMode;                   // 0xA28 LevelMode
	TUINT32                            m_uiSelectedGallery;            // 0xA2C the selected gallery picture (ABlobGalleries)
	APropDoor*                         m_pLevelStartGate;              // 0xA30 the DoorType 1 APropDoor
	APropDoor*                         m_pSprintGate;                  // 0xA34 the first DoorType 2 APropDoor
	TBOOL                              m_bLevelStart;                  // 0xA38
	TBOOL                              m_bLevelStartRespawn;           // 0xA39
	TBOOL                              m_bPlaceFirstBuildings;         // 0xA3A
	TUINT32                            m_uiLeaderBoardHud;             // 0xA3C
	TUINT32                            m_uiBossGrassBomb;              // 0xA40
};

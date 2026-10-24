#pragma once
#include <Toshi/TSingleton.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextBox.h>
#include <Math/TPlane.h>
#include "ARegionManager.h"
#include "Cameras/ASSCameraFunction.h"
#include "Cameras/ANedCameraFunction.h"

class ABaseEntity;
class ACamera;
class ABlobFollowCamera;
class AScriptedCamera;
class AKeyFrameCamera;
class ABlobVistaHud;

//-----------------------------------------------------------------------------
// Purpose: the players of the level: the blob, camera and controller of each player, the split screen, the start
// positions...
// NOTE: only what the frontend and the level start use is ported yet
//-----------------------------------------------------------------------------
class APlayerManager : public Toshi::TSingleton<APlayerManager>
{
public:
	static TCONSTEXPR_VAR TINT MAX_PLAYERS         = 4;
	static TCONSTEXPR_VAR TINT MAX_START_POSITIONS = 16; // the multiplayer start points

	// The level's player effects; empty in retail.
	// $deBlob: FUNCTION 005cee20
	static void LoadEffects() {}
	// $deBlob: FUNCTION 005cee30
	static void ClearEffects() {}

	// The data of a player's blob (0xB8 bytes)
	struct BlobData
	{
		BlobData();

		// Resets the data
		void Reset();

		TBOOL IsKeyFrameCameraActive() const;

		// The bonus pickups of a_iSet a_iType collected (m_auiBonusCollected[ a_iSet ], a bit per type)
		// $deBlob: FUNCTION 007b04f0
		void SetBonusCollected( TINT a_iSet, TINT a_iType, TBOOL a_bCollected )
		{
			if ( a_bCollected )
				m_auiBonusCollected[ a_iSet ] |= ( 1 << a_iType );
			else
				m_auiBonusCollected[ a_iSet ] &= ~( 1 << a_iType );
		}

		// Starts a power-up (APickupPowerUp::OnCollected): m_iPowerUpType its type, m_fPowerUpTime and the timer m_fPowerUpTimer its time
		// $deBlob: FUNCTION 007afb60
		void SetPowerUp( TINT8 a_iType, TFLOAT a_fTime, TFLOAT a_fTimer )
		{
			m_fPowerUpTime  = a_fTime;
			m_iPowerUpType  = a_iType;
			m_fPowerUpTimer = a_fTimer;
		}

		// $deBlob: FUNCTION 007b0860
		void SetStencilCollected( TINT a_iType, TBOOL a_bCollected )
		{
			if ( a_bCollected )
				m_uiStencilsCollected |= ( 1 << a_iType );
			else
				m_uiStencilsCollected &= ~( 1 << a_iType );
		}

		// retail compares the set's collected bits with 4
		TBOOL IsBonusSetComplete( TINT a_iSet ) const;

		// Sets the lives (m_uiLives) to the level mode's start lives (BlobEntityCommonProperties) and updates the
		// life HUD
		void ResetLives();

		// Takes a life (not below 0) and updates the life HUD, animated unless a_bNoAnim
		void LoseLife( TBOOL a_bNoAnim );

		// Gives a life (at most 10) and updates the life HUD (animated)
		void AddLife();

		// The key frame camera plays (or is about to) the SS camera a_iCamera
		TBOOL IsSSCameraPlaying( TINT a_iCamera ) const;

		// The next pattern the player has after a_iPattern, -1: none
		TINT GetNextPattern( TINT a_iPattern );

		void UpdatePowerUpTimer( TFLOAT a_fDeltaTime );
		TINT GetNumBonusesCollected() const;

		// Sprint levels: gives the player the first MAX_SPRINT_PATTERNS loaded patterns (m_iPattern: the first one if
		// none)
		void AddSprintPatterns();

		TBOOL                             m_bKeyFrameCameraActive;  // 0x00
		TUINT8                            m_auiBonusCollected[ 8 ]; // 0x01
		TUINT8                            m_auiUnk09[ 2 ];          // 0x09 a TUINT16 (unaligned); [ 0 ]: the respawn camera plays (no fades), [ 1 ]: it hands over to the follow camera
		TUINT8                            m_uiPad0B;                // 0x0B
		ABaseEntity*                      m_pBlob;                  // 0x0C ABlobEntity
		ABlobFollowCamera*                m_pBlobCamera;            // 0x10
		AKeyFrameCamera*                  m_pKeyFrameCamera;        // 0x14
		AScriptedCamera*                  m_pScriptedCamera;        // 0x18 the scripted camera in use
		AScriptedCamera*                  m_pRespawnCamera;         // 0x1C "Blob_Respawn_Camera.trb"
		AScriptedCamera*                  m_pBossCamera;            // 0x20 "Bossmech_camera.trb" (spaceship level)
		TUINT32                           m_uiLifeHud;              // 0x24
		ANedCameraFunction::SSCamTrigger* m_pSSCamTrigger;          // 0x28 the last SSCamTrigger entry of the camera regions that started an SS camera (UpdateCameraRegions)
		TFLOAT                            m_fPowerUpTimer;          // 0x2C a timer: m_iPowerUpType is reset once it runs out
		TFLOAT                            m_fPowerUpTime;           // 0x30 the power-up's time (APickupPowerUp)
		TFLOAT                            m_fSSCamTriggerTime;      // 0x34 10 s after m_pSSCamTrigger was found, then m_pSSCamTrigger is cleared
		TUINT32                           m_uiUnk38;                // 0x38
		TINT                              m_iCameraState;           // 0x3C 5, the camera state (UpdateCameraStates): 0 fade out, 1 switch, 2 respawn, 3 fade in, 4 help, 5 playing
		TINT                              m_iNextCameraMode;        // 0x40 -1, the camera mode to switch to (UpdateCameraStates)
		TINT                              m_eCameraMode;            // 0x44 -1, 1 once the player is added; GetPlayerCamera: 4 key frame, 2 scripted, else follow
		TINT                              m_iColour;                // 0x48 the start colour of the blob (multiplayer: the engagement menu's choice)
		TINT8                             m_iPowerUpType;           // 0x4C -1
		TINT8                             m_iUnk4D;                 // 0x4D -1
		TINT8                             m_iController;            // 0x4E AddPlayer
		TINT                              m_iPattern;               // 0x50 -1, the current pattern (GetPatternTexture)
		TUINT32                           m_uiLives;                // 0x54
		TUINT32                           m_uiUnk58;                // 0x58
		TINT                              m_iQueuedAnimState;       // 0x5C -1
		TBOOL                             m_bUnk60;                 // 0x60
		TUINT32                           m_uiUnk64;                // 0x64
		TBOOL                             m_bUnk68;                 // 0x68
		TBOOL                             m_bUnk69;                 // 0x69
		TUINT32                           m_uiPatternMask;          // 0x6C the patterns the player has (bit per pattern index)
		TUINT16                           m_uiStencilsCollected;    // 0x70 the stencils collected (bit per APickupStencil type)
		TUINT8                            m_auiUnk72[ 0x40 ];       // 0x72
		TINT16                            m_iHelpTopic;             // 0xB2 -1
		TINT16                            m_iHelpString;            // 0xB4 -1
	};

	// The vista score of the single player (m_soTempVistaScore, filled by CalculateVistaScore)
	struct VistaScore
	{
		TINT   m_iPlayer;                 // 0x00
		TINT   m_iNumColoursUsed;         // 0x04 the colours of m_aiColourCounts (not 0 and 4) with a count
		TINT   m_iTotalPatternsUsed;      // 0x08 the patterns of m_aiPatternCounts with a count
		TFLOAT m_fLPBPercent;             // 0x0C landmarks, public spaces and billboards painted (iLPBTotal)
		TFLOAT m_fBTPercent;              // 0x10 buildings and trees painted (iBTTotal)
		TFLOAT m_fColourSpread;           // 0x14 how evenly the 7 colours are used (%)
		TFLOAT m_fScore;                  // 0x18 the score of m_iRank
		TINT   m_iMessage;                // 0x1C locale string of the hint for the next rank, -1: none
		TINT   m_iRank;                   // 0x20 locale string of the rank
		TINT   m_iMaxColour;              // 0x24 the most used colour (m_aiColourCounts index)
		TINT   m_iMinColour;              // 0x28 the least used colour (m_aiColourCounts index)
		TINT   m_iNumLandmarks;           // 0x2C
		TINT   m_iNumLandmarksPainted;    // 0x30
		TINT   m_iNumPublicSpaces;        // 0x34 the grass instances
		TINT   m_iNumPublicSpacesPainted; // 0x38
		TINT   m_iNumBillBoards;          // 0x3C the landmarks that are billboards
		TINT   m_iNumBillBoardsPainted;   // 0x40
		TINT   m_iNumTrees;               // 0x44
		TINT   m_iNumTreesPainted;        // 0x48
		TINT   m_iNumBuildings;           // 0x4C
		TINT   m_iNumBuildingsPainted;    // 0x50
		TINT   m_aiColourCounts[ 9 ];     // 0x54 the trees and buildings of each colour
		TINT   m_aiPatternCounts[ 17 ];   // 0x78 the buildings of each pattern (APickupTypes::PT_PATTERN_COUNT_TOTAL)
		TWCHAR m_wszMessage[ 256 ];       // 0xBC the text of m_iMessage (CreateVistaScoreTexts)
	};

	// The state of the vista sequence (m_eVistaState, UpdateVista)
	enum VistaState : TINT
	{
		VistaState_Start     = 0, // ABlobVistasMenu starts a vista
		VistaState_Intro     = 3, // the vista HUD plays "EndToStatic"
		VistaState_Streaming = 4, // waits for the terrain of the camera
		VistaState_Score     = 6, // the score is calculated and shown
		VistaState_Shown     = 7, // the score is shown (ABlobVistaHud goes back on button 0)
		VistaState_End       = 8,
	};

	// A line of the vista score texts (0x268 bytes)
	struct VistaScoreText
	{
		// $deBlob: FUNCTION 005d12f0
		VistaScoreText() {}

		// NOTE: explicitly defaulted to carry the marker (m_aVistaScoreTexts' eh vector destructor in APlayerManager's
		// ctor/dtor: jumps to ~T2GUITextBox)
		// $deBlob: FUNCTION 005d1340
		~VistaScoreText() = default;

		Toshi::T2GUITextBox m_oTextBox;       // 0x000
		TWCHAR              m_wszText[ 256 ]; // 0x068
	};

	static TCONSTEXPR_VAR TINT NUM_VISTA_SCORE_TEXTS = 11;

public:
	APlayerManager();
	~APlayerManager();

	TINT GetNumPlayers() const;

	// NOTE: retail's GetNumPlayers is a real function (005cee10) that most of its ~100 callers call; a few small
	// functions have it expanded inline (the compiler's inlining budget), this is the accessor for those
	TINT GetNumPlayersInline() const { return m_iNumPlayers; }

	// $deBlob: FUNCTION 005b9df0
	void SetNumPlayers( TINT a_iDesiredPlayers )
	{
		TASSERT( a_iDesiredPlayers > 0 && a_iDesiredPlayers <= MAX_PLAYERS );
		m_iNumPlayers = a_iDesiredPlayers;
	}

	// The colour of a player's blob (BlobData::m_iColour, ABlobColourComponent's colour index)
	// $deBlob: FUNCTION 005b9d80
	void SetPlayerColour( TINT a_iPlayer, TINT a_iColour )
	{
		TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );
		m_aBlobData[ a_iPlayer ].m_iColour = a_iColour;
	}

	// $deBlob: FUNCTION 005d75c0
	TINT GetPlayerColour( TINT a_iPlayer ) const
	{
		TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );
		return m_aBlobData[ a_iPlayer ].m_iColour;
	}

	void Update( TFLOAT a_fDeltaTime );
	void Render( TFLOAT a_fDeltaTime );
	void CreatePlayers();

	// Update's work for one player
	void UpdatePlayer( TFLOAT a_fDeltaTime, TINT a_iPlayer );
	// Creates the HUD scene for the number of players and focuses it (as CreatePlayers does)
	void CreateHUD();
	// Removes the player of a_pBlob (RemovePlayer( 0xFF ) when it's no player's)
	void RemovePlayerOf( ABaseEntity* a_pBlob );
	// NOTE: a_bUnused isn't read by retail (CreatePlayers passes 0, RestartPlayers 1 in single player)
	void AssignPlayers( TBOOL a_bUnused );

	// Resets the players for the level restart (AInGameState): their data, blobs and cameras, puts them at the start
	// points again
	void RestartPlayers();

	// Gives the blobs their start colour: the sprint level's colour in single player, m_uiUnk48 of each player in
	// multiplayer (game type 1: colourless with 20 paint points)
	void SetupColours();

	// $deBlob: FUNCTION 005cf100
	// Resets the vista state and forgets the vista camera
	void ResetUnk2E8()
	{
		m_eVistaState = VistaState_Start;
		m_pVistaCam   = TNULL;
	}
	ABaseEntity* AddPlayer( TUINT8 a_uiPlayer, TINT8 a_iController, const Toshi::TVector4* a_pPosition );
	void         RemovePlayer( TUINT8 a_uiPlayer );
	// Removes the blobs and cameras of all the players (last player first)
	void RemovePlayers();
	// Removes all the players and clears 0x2F0/0x2F4 (AInGameState::OnRemoval)
	void DestroyPlayers();

	// Sets up the viewports of the players (AXYZViewportManager) and makes their cameras current
	void SetupViewports();

	// BlobData::AddSprintPatterns for every player
	void AddSprintPatterns();

	// Starts (or queues) the SS cameras of the camera regions (ANedCameraFunction SSCamTrigger entries) a_iPlayer's blob is in,
	// a_bUnk4 includes the entries with flag 4; plays the first queued SS camera once the follow camera is free
	void UpdateCameraRegions( TFLOAT a_fDeltaTime, TINT a_iPlayer, TBOOL a_bUnk4 );

	// The camera player a_uiPlayer looks through
	ACamera* GetPlayerCamera( TUINT8 a_uiPlayer );

	// The camera of the player whose blob is a_pBlob (player 0xFF if none)
	ACamera* GetBlobCamera( ABaseEntity* a_pBlob );

	// Plays the SS camera a_iCamera (ASSCameraFunction) through the key frame camera of player a_uiPlayer (the fly-by
	// of a completed block, ABuildingManager::OnBlockBuildingPainted)
	void PlaySSCamera( TUINT8 a_uiPlayer, TINT a_iCamera );

	// The camera states of the players: fades between the follow (1), scripted (2) and key frame (4) cameras of
	// BlobData::m_iNextCameraMode
	void UpdateCameraStates( TFLOAT a_fDeltaTime );

	// A key frame camera moved on to the SS camera a_pNext (a_pPrev unused): its help is kept in the players'
	// m_iHelpTopic/m_iHelpString (shown after the camera)
	void OnSSCameraChanged( const ASSCameraFunction::Camera* a_pPrev, const ASSCameraFunction::Camera* a_pNext );

	// The follow camera of the player whose blob is a_pBlob, TNULL if none
	ABlobFollowCamera* GetBlobFollowCamera( ABaseEntity* a_pBlob );

	// Blob of player a_iPlayer respawns at a_rPosition (a_pRotation: its rotation, turned around): the respawn camera
	// plays "respawn_loop" there without fades and hands over to the follow camera (a_bHandOver) with the blend out
	// fractions a_fBlendOutStart/a_fBlendOutEnd (ABlobEntity::Respawn)
	void StartRespawnCamera( TINT8 a_iPlayer, const Toshi::TVector4& a_rPosition, const Toshi::TVector4* a_pRotation, TBOOL a_bHandOver, TFLOAT a_fBlendOutStart, TFLOAT a_fBlendOutEnd );

	// Back to the follow camera's fade out state
	void ResetCameraState( TINT8 a_iPlayer );

	// The boss camera of player a_iPlayer at a_rcTransform plays "Intro" (a_iAnim 0) or "Death" (1)
	void StartBossCamera( TINT8 a_iPlayer, const Toshi::TMatrix44& a_rcTransform, TINT8 a_iAnim );

	// The blob's animation state (queued or current) is 0xF
	TBOOL IsBlobInState15( ABaseEntity* a_pBlob );
	// The follow camera's value at 0x14C (read as a float) of the player of a_pBlob when its camera mode is 1, else 0
	TFLOAT GetFollowCameraUnk14C( ABaseEntity* a_pBlob );

	// Multiplayer game types 0-2: the blobs take random start points (of the four types)
	void AssignMultiplayerPlayers();

	// The start points of the game type: AssignPlayers or AssignMultiplayerPlayers (not called by retail)
	void AssignStartPositions( TBOOL a_bUnk );

	// Multiplayer game type 3 (four players): the colours of the teams (players 0 and 1, 2 and 3), their mixes
	void GetTeamColours( TUINT32& a_riColor1, TUINT32& a_riColor2 );

	// The Z-jump of another player's blob locks on a_pBlob (ABlobMultiPlayerHud's threat)
	TBOOL IsBlobLockedOn( ABaseEntity* a_pBlob );

	// The respawn camera's animation ended: plays its "respawn_end" part
	TBOOL UpdateRespawnCamera( TINT8 a_iPlayer, TFLOAT a_fDeltaTime );

	BlobData* GetBlobData( TUINT8 a_uiPlayer );

	// Finds the textures of the current region's patterns and the barcode one
	void LoadPatternTextures();
	void UnloadPatternTextures();

	// The pattern texture of a pattern index (PT_PATTERN_COUNT_TOTAL - 1: the barcode one)
	ARegionManager::PatternTexture* GetPatternTexture( TINT a_iIndex );

	// The pattern of a player and moves to its next one, -1: none or the barcode
	TINT CyclePattern( TINT a_iPlayer );

	// The blob of player a_iPlayer
	// $deBlob: FUNCTION 005cd400
	ABaseEntity* GetPlayerBlob( TUINT8 a_iPlayer )
	{
		TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );
		return m_aBlobData[ a_iPlayer ].m_pBlob;
	}


	// Makes a_iPlayer the current player (m_uiCurrentPlayer) and returns its blob
	ABaseEntity* SetCurrentPlayer( TUINT8 a_iPlayer );

	// The cameras of a player, TNULL for a bad index
	ABlobFollowCamera* GetFollowCamera( TUINT8 a_iPlayer );

	void             SetCurrentPlayerBlob( ABaseEntity* a_pBlob );
	ABaseEntity*     GetActiveBlob( TINT a_iIndex ) const;
	AKeyFrameCamera* GetKeyFrameCamera( TUINT8 a_iPlayer );
	AScriptedCamera* GetScriptedCamera( TUINT8 a_iPlayer );

	// The cameras of the player whose blob is a_pBlob
	AKeyFrameCamera* GetBlobKeyFrameCamera( ABaseEntity* a_pBlob );
	AScriptedCamera* GetBlobScriptedCamera( ABaseEntity* a_pBlob );

	// BlobData::m_eCameraMode of a player with a blob, -1 without
	TINT GetCameraMode( TUINT8 a_iPlayer );

	// BlobData::m_uiUnk64 is set (TFALSE for a negative id)
	TBOOL IsUnk64( TINT a_iPlayer );

	// The m_iUnk3E of the fixed camera the follow camera of a_iPlayer is on (camera mode 1), 0 if none
	TINT GetFixedCamUnk3E( TINT a_iPlayer );

	// The vista score is shown
	TBOOL IsVistaScoreShown() const;

	// Clears a player's m_bUnk60/m_bUnk68/0x69, detaches its blob from the level start, removes the vista score texts
	// and forgets the vista HUD unless the level is complete
	void ResetPlayerUnk( TUINT a_uiPlayer );

	// The player whose blob is nearest to a_rvPosition (0 with one player)
	TINT8 GetClosestPlayer( const Toshi::TVector4& a_rvPosition );

	// The data of the current player (the low byte of m_uiCurrentPlayer), TNULL if it isn't valid
	// $deBlob: FUNCTION 005ce6c0
	BlobData* GetCurrentBlobData()
	{
		TUINT8 uiPlayer = TUINT8( m_uiCurrentPlayer & 0xFF );
		TASSERT( uiPlayer >= 0 && uiPlayer < MAX_PLAYERS );

		if ( uiPlayer < MAX_PLAYERS )
			return &m_aBlobData[ uiPlayer ];

		return TNULL;
	}

	// The blob of the current player (the low byte of m_uiCurrentPlayer), TNULL if none
	// $deBlob: FUNCTION 005cd460
	ABaseEntity* GetCurrentPlayerBlob() const
	{
		// NOTE: retail reads the member again after the assert
		if ( TINT8( m_uiCurrentPlayer & 0xFF ) >= 0 )
		{
			TASSERT( TINT8( m_uiCurrentPlayer & 0xFF ) >= 0 && TINT8( m_uiCurrentPlayer & 0xFF ) < MAX_PLAYERS );
			return m_aBlobData[ TINT8( m_uiCurrentPlayer & 0xFF ) ].m_pBlob;
		}

		return TNULL;
	}

	// The copy of the vista camera (m_oVistaCam, ABlobFollowCamera's category 0x40) while a vista is shown, else TNULL;
	// $deBlob: FUNCTION 005cfb80
	ANedCameraFunction::FixedCam* GetVistaCamera() { return ( m_pVistaCam != TNULL ) ? &m_oVistaCam : TNULL; }

	// BlobData::m_bUnk60 of a player, TFALSE for a_iPlayer < 0
	// $deBlob: FUNCTION 005cfa30
	TBOOL IsPlayerUnk60( TINT a_iPlayer ) const
	{
		if ( a_iPlayer >= 0 )
		{
			TASSERT( a_iPlayer < MAX_PLAYERS );
			return m_aBlobData[ a_iPlayer ].m_bUnk60;
		}

		return TFALSE;
	}

	// The blob of the player whose follow or key frame camera is a_pCamera, TNULL if none
	// $deBlob: FUNCTION 005cd4c0
	ABaseEntity* GetBlobOfCamera( const ACamera* a_pCamera ) const
	{
		for ( TINT i = MAX_PLAYERS; i != 0; )
		{
			i--;

			if ( m_aBlobData[ i ].m_pBlob != TNULL && ( TSTATICCAST( const void, m_aBlobData[ i ].m_pBlobCamera ) == a_pCamera || TSTATICCAST( const void, m_aBlobData[ i ].m_pKeyFrameCamera ) == a_pCamera ) )
				return m_aBlobData[ i ].m_pBlob;
		}

		return TNULL;
	}

	// The controller of the player whose blob is a_pBlob, -1 if none
	// $deBlob: FUNCTION 005ce750
	TINT8 GetBlobController( ABaseEntity* a_pBlob ) const
	{
		if ( a_pBlob != TNULL )
		{
			TUINT8 i = MAX_PLAYERS;

			do
			{
				i--;

				if ( a_pBlob == m_aBlobData[ TINT8( i ) ].m_pBlob )
					return GetController( i );
			} while ( i != 0 );
		}

		return -1;
	}

	// The controller of a player, -1 if none
	// $deBlob: FUNCTION 005ce720
	TINT8 GetController( TUINT8 a_uiPlayer ) const
	{
		if ( a_uiPlayer < MAX_PLAYERS )
			return m_aBlobData[ a_uiPlayer ].m_iController;

		return -1;
	}

	// The player whose blob is a_pBlob, -1 if none
	TINT8 GetPlayerIndex( ABaseEntity* a_pBlob ) const;
	// GetPlayerIndex as retail inlines it (GetBlobDataOf)
	TINT8 GetPlayerIndexInline( ABaseEntity* a_pBlob ) const
	{
		if ( a_pBlob == TNULL )
			return -1;

		for ( TINT8 i = MAX_PLAYERS; i != 0; )
		{
			i--;

			if ( a_pBlob == m_aBlobData[ i ].m_pBlob )
				return i;
		}

		return -1;
	}

	// The data of the player whose blob is a_pBlob
	// $deBlob: FUNCTION 005ce640
	BlobData* GetBlobDataOf( ABaseEntity* a_pBlob )
	{
		TUINT8 uiPlayer = TUINT8( GetPlayerIndexInline( a_pBlob ) );
		TASSERT( uiPlayer >= 0 && uiPlayer < MAX_PLAYERS );
		return ( uiPlayer < MAX_PLAYERS ) ? &m_aBlobData[ uiPlayer ] : TNULL;
	}

	// No player's blob has a paint colour (ink doesn't count)
	TBOOL IsNoBlobPainted() const;
	// The last player's blob that has a paint colour (ink doesn't count), TNULL if none
	ABaseEntity* GetPaintedBlob() const;

	TUINT8       GetNthPlayer( TINT a_iIndex );
	ABaseEntity* GetNthBlob( TINT a_iIndex );

	void StartQueuedAnimState( TINT a_iPlayer );

	// Queues the SS camera a_iCamera (MAX_SSCAM_QUEUE) for player a_uiPlayer (only 0)
	void QueueSSCamera( TUINT8 a_uiPlayer, TINT a_iCamera );

	// The SS camera a_iCamera is queued for player a_uiPlayer (only 0)
	TBOOL IsSSCameraQueued( TUINT8 a_uiPlayer, TINT a_iCamera ) const;

	// The count of the scripted sequences that hold the SS cameras back (m_iSSCamQueueDelayCount)
	void IncrementSSCameraDelay() { m_uiSSCameraDelay++; }
	// $deBlob: FUNCTION 00788430
	void DecrementSSCameraDelay()
	{
		m_uiSSCameraDelay--;
		TASSERT( TINT( m_uiSSCameraDelay ) >= 0 );
	}

	// Vistas (single player): ABlobVistasMenu sets m_iVista and VistaState_Start, ABlobVistaHud::Tick runs
	// UpdateVista.

	// Starts the vista a_iVista (ABlobVistasMenu)
	void StartVista( TINT a_iVista )
	{
		m_iVista      = a_iVista;
		m_eVistaState = VistaState_Start;
	}

	// Runs the vista sequence: shows the vista camera and its HUD, waits for the terrain, then calculates and shows the
	// score; TFALSE once it's over (and always in multiplayer)
	TBOOL UpdateVista( TFLOAT a_fDeltaTime );

	// $deBlob: FUNCTION 005c88a0
	TINT GetVistaState() const { return m_eVistaState; }

	// $deBlob: FUNCTION 0065c0e0
	void           SetVistaHud( ABlobVistaHud* a_pHud ) { m_pVistaHud = a_pHud; }
	ABlobVistaHud* GetVistaHud() const { return m_pVistaHud; }

	// Fills the vista score (m_soTempVistaScore) of a_iPlayer with what can be seen from a_pCamera: its instances, or
	// the instances in its frustum if it lists none
	static void CalculateVistaScore( TINT a_iPlayer, const ANedCameraFunction::FixedCam* a_pCamera );

	// The vista score of a_iPlayer (the last CalculateVistaScore's player)
	static VistaScore* GetVistaScore( TINT a_iPlayer );

	// The vista score read directly (ABlobVistaHud::OnCreated, no player check)
	static VistaScore& GetTempVistaScore() { return m_soTempVistaScore; }

	// a_bShow: fills the score texts and shows the vista HUD; else removes the texts (and forgets the HUD unless the
	// level is complete)
	void ShowVistaScore( TBOOL a_bShow );

private:
	// The planes of the frustum of a camera at a_rMatrix (FOV a_fFOV, near/far distances)
	static void CreateVistaFrustum( Toshi::TPlane* a_pOutPlanes, const Toshi::TMatrix44& a_rMatrix, TFLOAT a_fFOV, TFLOAT a_fNear, TFLOAT a_fFar );

	// TBoxTree render function of CalculateVistaScore: counts a cell instance (TerrainTypes::CellInstance) into a
	// VistaScore
	static void CountVistaInstance( void* a_pInstance, void* a_pVistaScore );

	// Creates the (hidden) score texts of m_soTempVistaScore and its message
	void CreateVistaScoreTexts();

	// Removes the score texts and their rectangle from the GUI
	void RemoveVistaScoreTexts();

	static VistaScore m_soTempVistaScore;

private:
	BlobData                            m_aBlobData[ MAX_PLAYERS ];                  // 0x000
	TINT                                m_iNumPlayers;                               // 0x2E0
	TUINT16                             m_uiCurrentPlayer;                           // 0x2E4 0xFF
	TINT                                m_eVistaState;                               // 0x2E8 VistaState
	TINT                                m_iVista;                                    // 0x2EC the vista camera shown next (ABlobVistasMenu)
	TUINT32                             m_uiNumQueuedSSCameras;                      // 0x2F0
	TUINT32                             m_uiSSCameraDelay;                           // 0x2F4
	TUINT8                              m_auiQueuedSSCameras[ 0x28 ];                // 0x2F8
	const ANedCameraFunction::FixedCam* m_pVistaCam;                                 // 0x320 the vista camera shown (FindVistaCam)
	ANedCameraFunction::FixedCam        m_oVistaCam;                                 // 0x324 its copy used by ABlobFollowCamera
	Toshi::T2GUIRectangle               m_oRectangle;                                // 0x384
	VistaScoreText                      m_aVistaScoreTexts[ NUM_VISTA_SCORE_TEXTS ]; // 0x3E4 the lines of m_oRectangle
	ARegionManager::PatternTexture      m_oBarcodePattern;                           // 0x1E5C "pattern_barcode.tga", index 16
	ABlobVistaHud*                      m_pVistaHud;                                 // 0x1E70 set by ABlobVistaHud while it exists
};

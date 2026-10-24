#pragma once
#include "AGameState.h"
#include "AInGameStateHelper.h"

class ATRBLoaderJob;

namespace Toshi
{
class TTRB;
}

//-----------------------------------------------------------------------------
// Purpose: the state of a level being played (400 bytes), pushed by ALoadLevelState once the level is loaded: creates
// the players, updates the game systems, ends the level.
//-----------------------------------------------------------------------------
class ABlobHud;

class AInGameState : public AGameState
{
public:
	TDECLARE_CLASS( AInGameState, AGameState );

public:
	AInGameState();
	~AInGameState();

	virtual void         HandleMenuRequestToCancel() override;                            // +0x0C
	virtual TBOOL        WasControllerRemoved() override;                                 // +0x10
	virtual void         OnFadeFinished( TINT a_iViewport, TBOOL a_bFromBlack ) override; // +0x14
	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override;                        // +0x18
	virtual void         OnInsertion() override;                                          // +0x1C
	virtual void         OnRemoval() override;                                            // +0x20
	virtual void         OnActivate() override;                                           // +0x2C
	virtual void         OnDeactivate() override;                                         // +0x30
	virtual TBOOL        CheckForControllerRemoval() override;                            // +0x38
	virtual TBOOL        PauseOnControllerReinserted() override;                          // +0x3C

	// The pause menu is open (ACameraManager only updates the follow cameras then)
	TBOOL IsPaused() const { return m_bPaused; }
	// $deBlob: FUNCTION 005c8a90
	void SetPaused( TBOOL a_bPaused ) { m_bPaused = a_bPaused; }
	// $deBlob: FUNCTION 00645a50
	TBOOL IsWaitingToStart() const { return m_bWaitingToStart; }
	// $deBlob: FUNCTION 006444c0
	void* GetControllerPopup() const { return m_pControllerPopup; }

	// The level is over (set by the pause menu's Quit, ends the level in OnUpdate)
	void SetLevelEnded( TBOOL a_bEnded ) { m_bLevelEnded = a_bEnded; }

	// $deBlob: FUNCTION 00654650
	void SetMultiplayerHud( ABlobHud* a_pHud ) { m_pMultiplayerHud = a_pHud; }

	// The results screen faded out: the level ends and the game returns to the level selection (ABlobScoreMenu sets
	// 0x2D and 0x2E)
	void EndLevelFromResults()
	{
		m_bLevelEnded = TTRUE;
		m_bUnk2E      = TTRUE;
	}

	// Restarts the level in place (the Retry of the time-up, score and reward menus, the pause menu's Reset): the
	// level's systems, entities, players and HUD are reset, the scenes above the HUD are closed
	void RestartLevel();

	// The level loaded next from the level (ABlobMPLevelSelection): its name, title and index, loaded once
	// m_bLoadNextLevel is set
	void SetNextLevel( const TCHAR* a_szLevel, TINT a_iTitleId, TINT a_iLevel );
	void SetLoadNextLevel( TBOOL a_bLoad ) { m_bLoadNextLevel = a_bLoad; }

	// The level start is over: the music plays again
	void OnLevelStartFinished();

	// Unpauses the sounds and the music (at once in a sprint level, else after a fade)
	void UpdateMusicPause();

	// The level resumes after a fade in: the multiplayer level timer is refreshed, the sounds unpaused (UpdateMusicPause);
	void OnResume();

	// Requests a TRB to stream: OnUpdate starts it once the HUD or the in-game scene is shown and the game paused
	// for the full screen help (APropTriggerHelp, ABlobHelp)
	void RequestStreamedTRB( Toshi::TTRB* a_pTRB, const TCHAR* a_szFileName, void* a_pUserData );

private:
	TINT UpdateReady( TFLOAT a_fDeltaTime );

	// Streams the TRB a_pTRB from a_szFileName (AAssetStreaming); OnStreamedJobFinished gets a_pUserData
	// NOTE: retail is a member that doesn't read this
	static void StartStreamedJob( Toshi::TTRB* a_pTRB, const TCHAR* a_szFileName, void* a_pUserData );
	static void OnStreamedJobFinished( ATRBLoaderJob* a_pJob, void* a_pUserData );

	// ATerrain2Interface's streaming position: the player's (or the camera's)
	static void __stdcall GetTerrainStreamingPosition( Toshi::TVector4* a_pOutPosition );

private:
	TBOOL              m_bLevelEndAcknowledged;  // 0x2C
	TBOOL              m_bLevelEnded;            // 0x2D
	TBOOL              m_bUnk2E;                 // 0x2E
	TINT               m_iReadyState;            // 0x30 0, 1: UpdateReady, 2: removed
	AInGameStateHelper m_oHelper;                // 0x34
	TBOOL              m_bWaitingToStart;        // 0x60 the fade in waits for the level (0.2 s)
	TFLOAT             m_fWaitTime;              // 0x64
	TFLOAT             m_fEndTimer;              // 0x68 -1.0f
	TBOOL              m_bPaused;                // 0x6C
	TBOOL              m_bUnk6D;                 // 0x6D
	ABlobHud*          m_pMultiplayerHud;        // 0x70 ABlobHud_2P/3P/4P (their OnCreated), its match start follows the fade in
	TUINT32            m_uiNextLevel;            // 0x74 the level AGameControllerHelper::LoadLevel loads next
	TUINT32            m_uiNextLevelName;        // 0x78
	TINT               m_iNextLevelIndex;        // 0x7C -1
	TBOOL              m_bLoadNextLevel;         // 0x80
	TBOOL              m_bWaitForFade;           // 0x81
	Toshi::TTRB*       m_pTRB;                   // 0x84 the TRB to stream (the full screen help's)
	TCHAR              m_szTRBFileName[ 0x100 ]; // 0x88
	void*              m_pTRBUserData;           // 0x188 the help trigger (APropTriggerHelp), TNULL from the help menu
	void*              m_pControllerPopup;       // 0x18C ABlobControllerPopup "ControllerPopup"
};

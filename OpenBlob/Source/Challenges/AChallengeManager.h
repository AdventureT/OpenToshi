#pragma once
#include "AChallenge.h"
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

class APaintChallenge;
class ABlobChallengeScene;
class AZJumpChallenge;
class ACombatChallenge;
class ARaceChallenge;
class ALandmarkChallenge;
class ACivilianChallenge;
class ACarChallenge;
class ANullChallenge;
class AUberChallenge;
class AFXCompositionInstance;

//-----------------------------------------------------------------------------
// Purpose: the challenges of the level (created by AAppInitState): builds them from the level's "Challenges" symbol
// (LoadChallenges), starts the one Blob accepts at its marker, runs it and ends it (completed, failed or quit).
//-----------------------------------------------------------------------------
class AChallengeManager
    : public Toshi::TSingleton<AChallengeManager>
{
public:
	friend class AChallenge;
	friend class AChallenge::CollisionHandler;
	friend class ABlobChallengeScene;

	// The level's "Challenges" symbol: the data of each type (0x48 bytes)
	struct ChallengesData
	{
		TUINT32 m_uiNumPaint;    // 0x00
		void*   m_pPaint;        // 0x04 0x40 bytes each
		TUINT32 m_uiNumZJump;    // 0x08
		void*   m_pZJump;        // 0x0C 0x40 bytes each
		TUINT32 m_uiNumCombat;   // 0x10
		void*   m_pCombat;       // 0x14 0x40 bytes each
		TUINT32 m_uiNumRace;     // 0x18
		void*   m_pRace;         // 0x1C 0x48 bytes each
		TUINT32 m_uiNumLandmark; // 0x20
		void*   m_pLandmark;     // 0x24 0x40 bytes each
		TUINT32 m_uiNumCivilian; // 0x28
		void*   m_pCivilian;     // 0x2C 0x40 bytes each
		TUINT32 m_uiNumCar;      // 0x30
		void*   m_pCar;          // 0x34 0x40 bytes each
		TUINT32 m_uiNumNull;     // 0x38
		void*   m_pNull;         // 0x3C 0x3C bytes each
		TUINT32 m_uiNumUber;     // 0x40
		void*   m_pUber;         // 0x44 0x3C bytes each
	};

	// What Blob looked like when the current challenge started (0x1C bytes)
	struct RestartInfo
	{
		Toshi::TVector4 m_vBlobPosition; // 0x00
		AChallenge*     m_pChallenge;    // 0x10
		TUINT32         m_uiBlobColour;  // 0x14
		TINT            m_iBlobPoints;   // 0x18
	};

public:
	AChallengeManager();
	~AChallengeManager();

	// Builds the challenges of the level
	void LoadChallenges();

	// Destroys the challenges
	void Destroy();

	// Resets the challenges (and their cameras) to their state at the level's start
	void Reset();

	// Disables the cameras of every challenge in the TRB
	void DisableAllChallengeCameras();

	// Once a frame
	void Update( TFLOAT a_fDeltaTime );

	// Starts m_pNearbyChallenge
	void StartChallenge();

	// Restarts the challenge kept in m_oRestartInfo
	void RestartChallenge();

	// A challenge was completed (a_bShowHelp: show the completed help once the HUD is back)
	void OnChallengeCompleted( AChallenge* a_pChallenge, TBOOL a_bShowHelp );

	// OnChallengeCompleted( m_pCurrentChallenge, TFALSE)
	void CompleteCurrentChallenge();

	// A challenge's time ran out
	void OnChallengeTimeOut( AChallenge* a_pChallenge );

	// Fails the current challenge
	void FailCurrentChallenge();

	// Quits the current challenge (a_bForce: even if it may not be quit)
	void QuitChallenge( TBOOL a_bForce );

	// Ends a_pChallenge: completed (a_bFailed false) or failed (a_bQuiet: no HUD/music change)
	void EndChallenge( AChallenge* a_pChallenge, TBOOL a_bFailed, TBOOL a_bQuiet );

	// Opens challenge.xur for m_pNearbyChallenge
	void ShowChallengeScreen();

	// Hides the prompt of m_pNearbyChallenge
	void HideNearbyPrompt();

	// The challenge with a_iGUID, TNULL if there's none
	AChallenge* FindChallengeByGUID( TINT a_iGUID ) const;

	// The AZJumpChallenge that uses the waypoint set a_iWPSetID, TNULL if there's none
	AZJumpChallenge* FindZJumpChallenge( TINT a_iWPSetID ) const;

	// Resets the challenges (Reset( TFALSE )) and their availability
	void ResetChallenges();

	// The counter shown in the HUD (a_iCount of a_iTotal)
	void SetCounter( TINT a_iCount, TINT a_iTotal );

	// The position of the challenge nearest to a_rvPos on XZ
	TFLOAT GetNearestChallengePosition( const Toshi::TVector4& a_rvPos, Toshi::TVector4& a_rvOutPos ) const;

	// The available challenge nearest to Blob (a_bVisible: only those in the camera's frustum)
	static AChallenge* FindNearestChallengeToBlob( TBOOL a_bVisible );

	// The available challenge nearest to a_rvPos on XZ
	AChallenge* FindNearestAvailableChallenge( const Toshi::TVector4& a_rvPos ) const;

	// The available challenge nearest to a_rvPos on XZ, in the camera's frustum
	AChallenge* FindNearestVisibleChallenge( const Toshi::TVector4& a_rvPos ) const;

	// The index a_iID of the current APaintChallenge (through its uber challenges) maps to, -1 if none
	TINT GetPaintChallengeIndex( TINT a_iID ) const;

	// Whether m_pNearbyChallenge may be started now
	TBOOL CanStartNearbyChallenge() const;

	// Starts the available challenge nearest to Blob
	void StartNearestChallenge();

	// Resets the entities that are always ticked (at a challenge's start)
	static void ResetEntities();

	// Shows/hides the markers of the challenges
	void SetMarkersVisible( TBOOL a_bVisible );

	// Disables the cameras of one challenge's data
	static void DisableChallengeCameras( AChallenge::Data* a_pData );

	// The powerup effect played by ARaceChallenge
	static void CreateEffect();
	static void DestroyEffect();
	static void TriggerEffect( const Toshi::TMatrix44& a_rTransform );

	// The events forwarded to the current challenge
	void OnPaintEvent( void* a_pBuilding );
	void OnPaintEvent2( void* a_pTree );
	void OnZJumpEvent( void* a_pSet );
	void OnCombatEvent( void* a_pEntityID );
	void OnCivilianEvent( void* a_pEntityID );
	void OnCarEvent( void* a_pEntityID );
	void OnLandmarkEvent( void* a_pLandmark );
	void OnLandmarkEvent2( void* a_pLandmarkInstance );

	// Blob died during the challenge
	void OnBlobDied();

	void SetRestartInfo( const RestartInfo& a_rInfo );

	// $deBlob: FUNCTION 0059c390
	TBOOL IsEnding() const { return m_bEnding; }

	// Sends MESSAGE_SKIP to the entities of the current challenge (AKeyFrameCamera, when its camera ends)
	void SendSkipToCurrentChallenge();

	// $deBlob: FUNCTION 00599950
	void SetLastScore( TINT a_iScore ) { m_iLastScore = a_iScore; }
	// $deBlob: FUNCTION 005c08d0
	TINT GetLastScore() const { return m_iLastScore; }

	// $deBlob: FUNCTION 00599990
	void SetNearbyChallenge( AChallenge* a_pChallenge ) { m_pNearbyChallenge = a_pChallenge; }

	// The challenge played with its camera (the blob camera's regions with flag 0x200)
	TBOOL IsWaitingForPrompt() const { return m_bWaitingForPrompt; }
	// The start prompt of a challenge is open (APropTriggerHelp::CanObjectTrigger)
	TBOOL        IsPromptShown() const { return m_bPromptShown; }
	AChallenge*  GetNearbyChallenge() const { return m_pNearbyChallenge; }
	AChallenge*  GetCurrentChallenge() const { return m_pCurrentChallenge; }
	RestartInfo& GetRestartInfo() { return m_oRestartInfo; }

	ChallengesData* GetChallengesData() const { return m_pChallengesData; }
	TINT            GetTotalNumChallenges() const { return m_iTotalNumChallenges; }
	// $deBlob: FUNCTION 005c09b0
	TINT GetTotalCompletedChallenges() const { return m_iTotalCompletedChallenges; }
	// $deBlob: FUNCTION 005c09c0
	TINT GetNumSubChallenges() const { return m_iNumSubChallenges; }
	// $deBlob: FUNCTION 0064ccc0
	TINT GetCounter() const { return m_iCounter; }
	TINT GetCounterTotal() const { return m_iCounterTotal; }

	// The HUD element of the challenge (ABlobHud 0x13C)
	// $deBlob: FUNCTION 00653e60
	void  SetHudElement( void* a_pElement ) { m_pHudElement = a_pElement; }
	void* GetHudElement() const { return m_pHudElement; }

private:
	static AFXCompositionInstance* s_pEffect;

private:
	ChallengesData*      m_pChallengesData;           // 0x000
	AChallenge**         m_ppChallenges;              // 0x004 all of them (m_iTotalNumChallenges)
	APaintChallenge*     m_pPaintChallenges;          // 0x008
	AZJumpChallenge*     m_pZJumpChallenges;          // 0x00C
	ACombatChallenge*    m_pCombatChallenges;         // 0x010
	ARaceChallenge*      m_pRaceChallenges;           // 0x014
	ALandmarkChallenge*  m_pLandmarkChallenges;       // 0x018
	ACivilianChallenge*  m_pCivilianChallenges;       // 0x01C
	ACarChallenge*       m_pCarChallenges;            // 0x020
	ANullChallenge*      m_pNullChallenges;           // 0x024
	AUberChallenge*      m_pUberChallenges;           // 0x028
	AChallenge*          m_pNearbyChallenge;          // 0x02C the one whose marker Blob touches
	AChallenge*          m_pCurrentChallenge;         // 0x030
	ABlobChallengeScene* m_pChallengeScene;           // 0x034 the open start prompt
	void*                m_pHudElement;               // 0x038
	TUINT32              m_uiUnk3C;                   // 0x03C
	TBOOL                m_bWaitingForPrompt;         // 0x040 the start prompt is open
	TUINT8               m_aUnk41[ 0x101 ];           // 0x041
	TWCHAR               m_wszCounter[ 0x80 ];        // 0x142
	TINT                 m_iTotalNumChallenges;       // 0x244
	TINT                 m_iTotalCompletedChallenges; // 0x248
	TINT                 m_iNumSubChallenges;         // 0x24C the AUberChallenge sub challenges
	TBOOL                m_bPromptShown;              // 0x250 the start prompt is open (Blob may leave the marker)
	TINT                 m_iLastScore;                // 0x254
	TINT                 m_iCounter;                  // 0x258
	TINT                 m_iCounterTotal;             // 0x25C
	TUINT8               m_aUnk260[ 0x240 ];          // 0x260
	TFLOAT               m_fContextHelpTimer;         // 0x4A0
	TBOOL                m_bEnding;                   // 0x4A4 the challenge is over, challengeend.xur is pending
	TBOOL                m_bFailed;                   // 0x4A5
	TUINT32              m_uiLastNumLinked;           // 0x4A8 m_uiNumLinkedChallenges of the last ended challenge
	TBOOL                m_bShowChallengeScreen;      // 0x4AC
	TBOOL                m_bShowCompletedHelp;        // 0x4AD
	AChallenge*          m_completedChallenge;        // 0x4B0
	RestartInfo          m_oRestartInfo;              // 0x4B4
};

TSTATICASSERT( sizeof( AChallengeManager ) == 0x4D0 );

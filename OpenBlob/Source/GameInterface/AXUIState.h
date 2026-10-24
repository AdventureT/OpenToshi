#pragma once
#include "AGameState.h"
#include "Input/AInputHelper.h"

#include <File/TTRB.h>
#include <Sound/TSound.h>

class ATRBLoaderJob;
class ABlobControllerPopup;

namespace Toshi
{
class T2GUIElement;
class TXUIScene;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: the game state of a XUI menu: creates the scene of a XUI resource (now, or once the streamed TRB is loaded),
// transitions to it and gives it the input. Its static helpers load the XUI skins, the XUI font and the localised XUI
// TRBs ("data/xui/<locale>/<file>") with AMemory::TemporaryTRBAllocator.
//-----------------------------------------------------------------------------
class AXUIState : public AGameState
{
public:
	TDECLARE_CLASS( AXUIState, AGameState );

	static TCONSTEXPR_VAR TINT BUFFSIZE = 128;

	// What the state does once it's left (SetNextState)
	enum NextState
	{
		NextState_None,
		NextState_SaveLoadStart, // TXUI::Reset, pushes ASaveLoadStartAppState (the language menu)
		NextState_LoadLevel,     // fades out, then loads the level m_iNextLevel (AGameControllerHelper)
		NextState_Unk3,
		NextState_FrontEnd, // back to the frontend (AGameControllerHelper)
	};

	// What the state is created with (0x30)
	struct Assets
	{
		Assets();

		TUINT32              m_uiUnk00;          // 0x00
		TUINT32              m_uiUnk04;          // 0x04
		TUINT32              m_uiUnk08;          // 0x08
		const TCHAR*         m_szSoundFile;      // 0x0C streamed (FMOD) while the state is on
		const TCHAR*         m_sResourceName;    // 0x10 the .xur
		const TWCHAR*        m_wszSceneName;     // 0x14 TNULL: the first scene
		TFLOAT               m_fSoundVolume;     // 0x18
		TBOOL                m_bCacheScene;      // 0x1C
		TBOOL                m_bActivatesCameras; // 0x1D
		Toshi::T2GUIElement* m_pTargetParent;     // 0x20 the scene is added to it
		TBOOL                m_bDelayed;          // 0x24 the scene is created once the streamed TRB is loaded
		TINT                 m_iTransitionIndex;  // 0x28 -1: the scene's own
		TBOOL                m_bUnk2C;            // 0x2C
		TBOOL                m_bBackFromLevel;    // 0x2D
	};

public:
	// The sound of the state and its paused flag (AMusicManager::PauseAll/ResumeAll)
	FMOD::Channel* GetChannel() const { return m_pChannel; }
	TBOOL*         GetChannelPausedFlag() { return TREINTERPRETCAST( TBOOL*, &m_uiUnk34 ); }

public:
	AXUIState( const Assets& a_rAssets );
	~AXUIState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x18
	virtual void         OnInsertion() override;                   // +0x1C
	virtual void         OnRemoval() override;                     // +0x20
	virtual void         OnActivate() override;                    // +0x2C
	virtual void         OnDeactivate() override;                  // +0x30

	//-----------------------------------------------------------------------------
	// Static helpers
	//-----------------------------------------------------------------------------

	// TXUI::SetSkin1/SetSkin2 with the localised path of the TRB
	static void SetSkin1( const TCHAR* a_szTRBFileName, const TCHAR* a_szXURName );
	static void SetSkin2( const TCHAR* a_szTRBFileName, const TCHAR* a_szXURName );

	// Loads the XUI font TRB (a "tfnt" symbol) in the global heap
	static TBOOL InitFont( const TCHAR* a_szTRBFileName );

	// Loads the localised TRB of the XUI resources the menus share
	static TBOOL SetCommonSkin( const TCHAR* a_szTRBFileName );

	// Loads the localised TRB of the menus' XUI resources, on the asset streaming thread if a_bStreamed
	static TBOOL SetSkinWithFont( const TCHAR* a_szTRBFileName, TBOOL a_bStreamed );

	// Unloads the streamed TRB of the delayed scenes
	static void UnloadStreamedTRB();

	// Loads the localised TRB of the in-game XUI resources (ingame.trb)
	static TBOOL SetInGameSkin( const TCHAR* a_szTRBFileName );

	// Unload the skin TRB (SetSkinWithFont) and the in-game TRB, TXUI::Reset
	static void UnloadSkin();
	static void UnloadInGameSkin();
	// Unload the common TRB (SetCommonSkin) and the font TRB (AApplication::OnDestroy)
	static void UnloadCommonSkin();
	static void UnloadFontSkin();
	static void ResetXUI();

	// Streams the localised frontendcarousel.trb (the delayed scenes: splash.xur, levelselection.xur...)
	static void LoadFrontendCarousel();

	// Leaves the state with the given NextState (fading out first for NextState_LoadLevel)
	void SetNextState( TINT a_eNextState );

	// $deBlob: FUNCTION 0062fd90
	const TCHAR* GetNextLevel() const { return m_szNextLevel; }
	// $deBlob: FUNCTION 0062fda0
	TINT GetLevelIndex() const { return m_iLevelIndex; }

	// The level of NextState_LoadLevel (the level selection)
	void SetNextLevel( const TCHAR* a_szLevel, const TWCHAR* a_wszName, TINT a_iLevelIndex )
	{
		m_szNextLevel      = a_szLevel;
		m_wszNextLevelName = a_wszName;
		m_iLevelIndex      = a_iLevelIndex;
	}

	// The same with the name as a locale string (-1: none, L""), like AInGameState::SetNextLevel
	void SetNextLevel( const TCHAR* a_szLevel, TINT a_iNameId, TINT a_iLevelIndex );

private:
	// Writes "data/xui/<locale>/<a_szFileName>" to s_AssetLoadQueue
	static void MakeLocalisedPath( const TCHAR* a_szFileName );

	static void SkinWithFontLoaded( ATRBLoaderJob* a_pJob, void* a_pUserData );
	static void FrontendCarouselLoaded( ATRBLoaderJob* a_pJob, void* a_pUserData );

	// The end of a fade out (NextState_LoadLevel)
	UpdateResult GoToNextState();


private:
	static TCHAR        s_AssetLoadQueue[ BUFFSIZE ];
	static Toshi::TTRB  s_FontTRB;
	static TBOOL        s_bFontLoaded;
	static Toshi::TTRB  s_CommonTRB;
	static TBOOL        s_bCommonLoaded;
	static Toshi::TTRB  s_SkinTRB;
	static TBOOL        s_bSkinLoaded;
	static TMemoryHeap* s_pSkinOldTRBHeap;
	static Toshi::TTRB  s_StreamedTRB; // the TRB of the delayed scenes
	static TBOOL        m_bLoading;    // the streamed TRB is being loaded (a static)
	static TBOOL        s_bStreamedTRBLoaded;
	static Toshi::TTRB  s_InGameTRB;
	static TBOOL        s_bInGameLoaded;

private:
	friend class ACheatManager;

	FMOD::Sound*          m_pSound;                  // 0x2C
	FMOD::Channel*        m_pChannel;                // 0x30
	TUINT32               m_uiUnk34;                 // 0x34
	TUINT32               m_uiUnk38;                 // 0x38
	TINT                  m_eNextState;              // 0x3C NextState
	AInputHelper          m_InputHelper;             // 0x40 the cancel button of the menu context
	Toshi::TXUIScene*     m_pPreviousScene;          // 0x64 the active scene when the state got deactivated
	TBOOL                 m_bActivatesCameras;       // 0x68
	const TCHAR*          m_szNextLevel;             // 0x6C NextState_LoadLevel: the level's name (ALevelInformation)
	const TWCHAR*         m_wszNextLevelName;        // 0x70 NextState_LoadLevel: shown on the loading screen
	TINT                  m_iLevelIndex;             // 0x74
	Toshi::TXUIScene*     m_pScene;                  // 0x78
	ABlobControllerPopup* m_pControllerPopup;        // 0x7C
	TBOOL                 m_bFadeToNextState;        // 0x80 NextState_LoadLevel fades out first
	TBOOL                 m_bFadedOut;               // 0x81
	TBOOL                 m_bTransitioned;           // 0x82 the state has transitioned to its scene
	TWCHAR                m_wszSceneName[ 256 ];     // 0x84 of the delayed scene
	TCHAR                 m_szResourceName[ 256 ];   // 0x284 of the delayed scene
	Toshi::T2GUIElement*  m_pTargetParent;           // 0x384 of the delayed scene
	TBOOL                 m_bDelayedCacheScene;      // 0x388
	TINT                  m_iDelayedTransitionIndex; // 0x38C
	TBOOL                 m_bPendingTransition;      // 0x390 the delayed scene is created, transition to it
	TBOOL                 m_bBackFromLevel;          // 0x391 the delayed scene is ABlobMainMenu or ABlobLevelSelection, back from a level
};

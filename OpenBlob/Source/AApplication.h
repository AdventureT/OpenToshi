#pragma once
#include <Toshi/TApplication.h>

class ARenderer;
class AGameStateController;

TOSHI_NAMESPACE_START
class TSystemManager;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: the game application (OnCreate is in AppBoot.cpp).
//-----------------------------------------------------------------------------
class AApplication : public Toshi::TApplication
{
public:
	static const TCHAR* const scm_sDefaultLevelPropertiesFileName;

public:
	AApplication();
	~AApplication();

	//-----------------------------------------------------------------------------
	// Toshi::TApplication
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate( TINT argc, TCHAR** argv ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual TBOOL OnDestroy() OVERRIDE;

	// Reads -width/-height (-w/-h), -exclusivefullscreen, -windowed (-sw), -locale and setup.ini
	void ParseCommandLineParams( TINT argc, TCHAR** argv );

	void SetRenderWorld( TBOOL a_bRenderWorld );
	// Pauses the game (m_bGamePaused), the material animations and the music
	void SetGamePaused( TBOOL a_bPaused );
	// $deBlob: FUNCTION 005c1cc0
	void* GetSaveData() const { return m_pSaveData; }
	TBOOL IsGamePaused() const { return m_bGamePaused; }
	TBOOL ShouldRenderWorld() const { return m_bRenderWorld; }

	// $deBlob: FUNCTION 005c1cb0
	TBOOL ShouldLoadModelViewState() const { return m_bLoadModelViewState; }

	void SetCurrentSaveSlot( TINT a_iCurrentSaveSlot );

	// Creates the save data objects
	void CreateSaveData();

	const TCHAR* GetLevelPropertiesFile() const;

	// Nothing in de Blob
	void InitialiseFreeLists();

private:
	// Listener of TSystemManager's pause emitter: m_bInitialised = !*a_pPause (never connected)
	static TBOOL __stdcall OnPauseEvent( AApplication* a_pApp, Toshi::TSystemManager* a_pSystemManager, TBOOL* a_pPause );

	void UpdateSound( TFLOAT a_fDeltaTime );

public:
	// "-locale <code>": overrides the language of Steam
	static const TCHAR* sm_ForcedLocale;

public:
	void*                 m_pSoundMempool;             // 0x2C
	TUINT32               m_uiUnk30;                   // 0x30
	ARenderer*            m_pRenderer;                 // 0x34
	AGameStateController* m_pGameStateController;      // 0x38
	void*                 m_pSaveData;                 // 0x3C
	void*                 m_pSaveData2;                // 0x40 EnSaveData::SlotData, a default profile (CreateSaveData): AAutosaveManager::Save copies it over the current slot
	TINT                  m_iCurrentSaveSlot;          // 0x44
	TBOOL                 m_bRenderWorld;              // 0x48
	TBOOL                 m_bGamePaused;               // 0x49
	TBOOL                 m_bInitialised;              // 0x4A set at the end of OnCreate
	TFLOAT                m_fMaxDeltaTime;             // 0x4C
	TFLOAT                m_fTotalTime;                // 0x50 seconds of the scheduler
	TBOOL                 m_bUnk54;                    // 0x54
	TINT                  m_iWidth;                    // 0x58 -1: not set
	TINT                  m_iHeight;                   // 0x5C -1: not set
	TBOOL                 m_bWindowed;                 // 0x60
	TBOOL                 m_bExclusiveFullscreen;      // 0x61
	TBOOL                 m_bUnk62;                    // 0x62
	TBOOL                 m_bLoadModelViewState;       // 0x63
	TBOOL                 m_bFXView;                   // 0x64
	TBOOL                 m_bSkipFrontend;             // 0x65
	const TCHAR*          m_szLevelPropertiesFileName; // 0x68
};

extern AApplication g_oTheApp;

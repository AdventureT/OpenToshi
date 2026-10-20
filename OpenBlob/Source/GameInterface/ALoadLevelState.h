#pragma once
#include "AGameState.h"
#include "ALevelInformation.h"

#include <File/TTRB.h>

class ABlobControllerPopup;
class AEntityLoader;

//-----------------------------------------------------------------------------
// Purpose: loads a level: shows the loading screen, streams the level's data blocks (ACurrentLevelInformation), loads
// the music and the sounds, then the terrain, the physics, the entities... Step by step (m_State), and pushes the
// in-game state once done.
//-----------------------------------------------------------------------------
class ALoadLevelState : public AGameState
{
public:
	TDECLARE_CLASS( ALoadLevelState, AGameState );

	enum STATE
	{
		STATE_WaitForJobs,   // 0x00 the frontend's streaming jobs
		STATE_LoadingScreen, // 0x01 creates the loading screen
		STATE_LevelData,     // 0x02 streams the data blocks
		STATE_Entities,      // 0x03
		STATE_Music,         // 0x04 loads the music
		STATE_RegionSounds,
		STATE_RegionSoundsLoading,
		STATE_ChallengeVoices,
		STATE_FXParticles,
		STATE_EntityProperties,
		STATE_Terrain,
		STATE_EnvParticles,
		STATE_AINetwork,
		STATE_FXCompositions,
		STATE_TerrainInstances,
		STATE_TerrainStreaming,
		STATE_EnvironmentFX,
		STATE_Unk11,
		STATE_LoadEntities, // 0x12 AEntityLoader
		STATE_WaitForOK,    // 0x13 the loading screen shows "Continue"
		STATE_Done,         // 0x14
	};

public:
	// NOTE: retail's default constructor leaves the members uninitialised
	// $deBlob: FUNCTION 005ca5b0
	ALoadLevelState() {}
	ALoadLevelState( AGameState* a_pNextState, ALevelInformation::LevelProperties* a_pLevelProperties, const TWCHAR* a_wszLevelName );
	~ALoadLevelState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x18
	virtual void         OnInsertion() override;                   // +0x1C
	virtual void         OnRemoval() override;                     // +0x20

	static ALoadLevelState* GetSingleton() { return s_pInstance; }

	// Loads the shader assets of the level once the assets are streamed in: the jelly solids, the water texture
	static void LoadGUI();

	TINT GetLoadState() const { return m_State; }

	// The player pressed "Continue" on the loading screen (ABlobLoadingScreen::HandleButton)
	// $deBlob: FUNCTION 006484d0
	void Continue()
	{
		TASSERT( m_State == STATE_WaitForOK );
		SetState( STATE_Done );
	}

private:
	void SetState( TINT a_eState );

	// Frees the entity loader, updates the entities' LOD and loads the pattern textures
	void OnEntitiesLoaded();

	// Initialises TPhysicsManager with the bounds of the terrain's first region
	void InitialisePhysics();

private:
	static ALoadLevelState* s_pInstance;

private:
	const TWCHAR*                       m_wszLevelName;      // 0x2C shown on the loading screen
	AGameState*                         m_pNextState;        // 0x30 pushed once done (AInGameState)
	TINT                                m_State;             // 0x34 STATE
	AEntityLoader*                      m_pEntityLoader;     // 0x38
	ALevelInformation::LevelProperties* m_levelProperties;   // 0x3C
	TBOOL                               m_bFadeIn;           // 0x40 fades from black once the fade out is done
	ABlobControllerPopup*               m_pControllerPopup;  // 0x44
	Toshi::TTRB                         m_oLoadingScreenTRB; // 0x48
};

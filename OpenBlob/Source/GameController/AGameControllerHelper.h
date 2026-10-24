#pragma once

class AIOEntity;

//-----------------------------------------------------------------------------
// Purpose: switches between the frontend and the levels: clears the game state stack and the XUI, then pushes the level
// loading state (ALoadLevelState).
//-----------------------------------------------------------------------------
class AGameControllerHelper
{
public:
	// The IO entity of an ID, TNULL if there is none
	static AIOEntity* FindIOEntity( TINT a_iID );
	// The IO entity "CheckPoint1" of the level, TNULL if none
	static AIOEntity* GetCheckPointStartEntity();

	// Loads the level named a_szLevelName (ALevelInformation), a_wszDisplayName is shown on the loading screen
	static void LoadLevel( const TCHAR* a_szLevelName, const TWCHAR* a_wszDisplayName );

	// Loads the level a_iLevelIndex (ALevelInformation)
	static void LoadLevel( TINT a_iLevelIndex, const TWCHAR* a_wszDisplayName );
	static void LoadFirstLevel();
	// Leaves the level for the frontend: the game state stack is reset and the main menu (a_bLevelSelect FALSE) or the
	// level selection is pushed (AXUIState, delayed).
	static void ReturnToFrontEnd( TBOOL a_bLevelSelect );

	// Leaves the level for the boss end sequence (ABossEndSeqState): the score summary is calculated, the game state
	// stack is reset and the XUI and the skins are unloaded.
	static void StartBossEndSequence();
};

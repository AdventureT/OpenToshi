#pragma once
#include <File/TTRB.h>
#include <Toshi/THashTable.h>

//-----------------------------------------------------------------------------
// Purpose: the levels of the game: the TRB named by the application's level properties file (data/<file>.trb, symbol
// "LevelsMain") and a hash table from level name to level index.
// Not a TSingleton: retail asserts on its own sm_singleton.
//-----------------------------------------------------------------------------
class ALevelInformation : public Toshi::TTRB
{
public:
	// The properties of a level (100 bytes)
	struct LevelProperties
	{
		const TCHAR* m_szName;            // 0x00 the hash table key (AGameControllerHelper::LoadLevel)
		const TCHAR* m_directory;         // 0x04 the level's folder in data/
		const TCHAR* m_szUnk08;           // 0x08 a level name (ABlobDebugLevelSelect matches it against the world and sprint levels)
		TUINT32      m_uiUnk0C;           // 0x0C
		const TCHAR* m_szMovie;           // 0x10 the movie played before the level, "" none
		const TCHAR* m_szEnvFX;           // 0x14 data/envfx/<name>.trb, "default" or "": the data block's
		const TCHAR* m_szMusic;           // 0x18 AMusicManager::LoadMusicData, "": "default"
		TFLOAT       m_afTimeLimits[ 3 ]; // 0x1C in minutes, -1: none (AGameStatus: level mode -1/0, 2, 3)
		TUINT32      m_uiUnk28;           // 0x28
		TINT         m_iTerrainStreaming; // 0x2C (ALoadLevelState: == 1)
		TINT         m_sprintLevel;       // 0x30 != 0: a sprint level (loadingsprint.xur)
		TUINT32      m_auiMedalLimits[ 9 ]; // 0x34
		TINT         m_iMaxScore;           // 0x58 AMusicManager::SetMaxScore
		TUINT32      m_auiGoals[ 2 ];       // 0x5C
	};

	TSTATICASSERT( sizeof( LevelProperties ) == 100 );

	struct LevelHeader
	{
		TINT             m_levelCount;  // 0x0
		LevelProperties* m_pLevelProps; // 0x4
	};

public:
	ALevelInformation();
	~ALevelInformation();

	// Loads the TRB and builds the hash table
	void Create( const TCHAR* a_szFileName );

	// The index of the level named a_szName, -1 if there's none
	TINT GetLevelIndex( const TCHAR* a_szName );

	// The index of the level whose properties are at a_pProperties, -1 if there's none
	TINT GetLevelIndex( const LevelProperties* a_pProperties );

	LevelProperties* GetLevelProperties( TINT a_iLevelIndex );

	// The level named a_szName (compared up to the extension of the level's name), TNULL if there is none
	LevelProperties* FindLevelProperties( const TCHAR* a_szName );

	// $deBlob: FUNCTION 0056de20
	LevelHeader* GetLevelHeader() const { return m_pData; }

	static ALevelInformation* GetSingleton() { return sm_singleton; }

private:
	void MakeStringToIDHashTable();

	static TBOOL LevelNameCompare( void* a_pMem1, void* a_pMem2, TINT a_iSize );

private:
	static ALevelInformation* sm_singleton;

private:
	LevelHeader*      m_pData;      // 0x14
	Toshi::THashTable m_oHashTable; // 0x18
};

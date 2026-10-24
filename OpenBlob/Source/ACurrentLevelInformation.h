#pragma once
#include "ALevelInformation.h"

class ATRBLoaderJob;

//-----------------------------------------------------------------------------
// Purpose: the data blocks of the level being loaded: the TRBs of the level's folder (data/<directory>/...) and the
// common ones (data/...), streamed by AAssetStreaming.
// Not a TSingleton: retail asserts on its own sm_singleton.
//-----------------------------------------------------------------------------
class ACurrentLevelInformation
{
public:
	enum DataBlock
	{
		DataBlock_PatternSettings,
		DataBlock_CameraShakeCommon,
		DataBlock_FXParticles,
		DataBlock_BlobProperties,
		DataBlock_SoundBreakPoints,
		DataBlock_EnvFXDefault,
		DataBlock_EnvFXDefaultMP,
		DataBlock_RegionRuntimeData,
		DataBlock_NavMarkers,
		DataBlock_ZJumpData,
		DataBlock_Waypoints,
		DataBlock_NEDCamera,
		DataBlock_SSCamera,
		DataBlock_CameraShake,
		DataBlock_PointData,
		DataBlock_SoundEmitter,
		DataBlock_PaintableBlockData,
		DataBlock_Network,
		DataBlock_Entities,
		DataBlock_Terrain,
		DataBlock_ChallengeData,
		DataBlock_VOMapping,
		DataBlock_FXData,
		DataBlock_MPData,
		DataBlockCount
	};

	struct DataBlockFile
	{
		const TCHAR* m_szFileName; // 0x0
		TBOOL        m_bCommon;    // 0x4 in data/, not in the level's folder
	};

public:
	ACurrentLevelInformation( ALevelInformation::LevelProperties* a_pLevelProperties );
	~ACurrentLevelInformation();

	// Streams every data block (DataBlock_MPData only with a_bMultiplayer)
	void Create( TBOOL a_bMultiplayer );

	// Closes the data blocks
	void Destroy();

	// TTRUE while a data block is still streaming (releases the finished jobs)
	TBOOL IsStreaming();

	// Releases the finished jobs (asserting nothing streams any more) and returns the data block
	Toshi::TTRB* Stream( TINT a_type );

	static const TCHAR* GetDataBlockFileName( TINT a_type );

	void GetDataBlockPath( TINT a_type, TCHAR* a_szPath, TINT a_iSize );

	// $deBlob: FUNCTION 00576100
	ALevelInformation::LevelProperties* GetLevelProperties() const { return m_pLevelProperties; }

	static ACurrentLevelInformation* GetSingleton() { return sm_singleton; }

private:
	static const DataBlockFile sm_aDataBlockFiles[ DataBlockCount ];

	static ACurrentLevelInformation* sm_singleton;

private:
	ALevelInformation::LevelProperties* m_pLevelProperties;                // 0x00
	Toshi::TTRB*                        m_datablocks[ DataBlockCount ];    // 0x04
	ATRBLoaderJob*                      m_streamingJobs[ DataBlockCount ]; // 0x64
};

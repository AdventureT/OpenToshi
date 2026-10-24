#pragma once
#include <File/TTRB.h>

class ATRBLoaderJob;

namespace Toshi
{
class T2Material;
class TTexture;
class TWorldMaterial;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: loads the asset packs of the level's regions (8 bytes): the region's TRBs (data/<level>/<asset>.trb), Blob's
// character pack and the FX packs, streamed by AAssetStreaming and initialised (TAssetInit: models, world meshes,
// materials...) when they arrive. Created by ALoadLevelState from the level's RegionRuntimeData data block.
//-----------------------------------------------------------------------------
class ARegionManager
{
public:
	static TCONSTEXPR_VAR TINT MaxAssets = 32;

	// A pattern texture of a region (0x14 bytes, the painters' patterns)
	struct PatternTexture
	{
		const TCHAR*     m_szTexture;  // 0x00
		Toshi::TTexture* m_pTex;       // 0x04 set by APlayerManager::LoadPatternTextures
		TINT             m_eBlendType; // 0x08 TWorldMaterialHAL::MULTIPASSBLENDTYPE, toggled 1 <-> 2 on each load
		TUINT32          m_uiUnk0C;    // 0x0C
		TINT             m_iIndex;     // 0x10 APickupTypes pattern index
	};

	// A region of RegionRuntimeData (0x1C bytes)
	struct Region
	{
		const TCHAR*    m_szName;              // 0x00
		TINT            m_iNumAssets;          // 0x04
		const TCHAR**   m_ppAssets;            // 0x08 TRB names in data/<level>/, without the extension
		TINT            m_iNumSoundGroups;     // 0x0C
		TCHAR**         m_ppSoundGroups;       // 0x10 ASoundLoadHelper::LoadSounds
		TINT            m_iNumPatternTextures; // 0x14
		PatternTexture* m_pPatternTextures;    // 0x18
	};

	// The level's RegionRuntimeData symbol
	struct RegionRuntimeData
	{
		Region* m_pRegions;    // 0x00
		TINT    m_iNumRegions; // 0x04
	};

	enum STATE : TINT
	{
		STATE_None,
		STATE_Loading,
		STATE_Loaded,
	};

	// The loading state of a region (0x8C bytes)
	struct RegionState
	{
		void AddAsset( Toshi::TTRB* a_pTRB );

		STATE        m_eState;                // 0x00
		Toshi::TTRB* m_apAssets[ MaxAssets ]; // 0x04
		TINT         m_iAssetCount;           // 0x84
		TINT         m_iQueuedAssetCount;     // 0x88
	};

public:
	ARegionManager( RegionRuntimeData* a_pRuntimeData );
	~ARegionManager();

	// Queues the assets of the region (by index/name) if it has none yet
	void LoadRegion( TINT a_iRegion );
	void LoadRegion( const TCHAR* a_szRegion );

	// Starts loading the region's sound groups (ASoundLoadHelper)
	void LoadRegionSounds( TINT a_iRegion );
	void LoadRegionSounds( const TCHAR* a_szRegion );

	// Deinitialises and frees the assets of a loaded region / of every loaded region
	void UnloadRegion( const TCHAR* a_szRegion );
	void UnloadAllRegions();

	// TTRUE while a region's assets are streaming
	TBOOL IsLoading();

	// TTRUE while ASoundLoadHelper loads (its state isn't 5)
	TBOOL IsLoadingSounds();

	// Updates the sound loading
	void UpdateLoading();
	void UpdateSoundLoading();

	RegionState* GetRegionState( const TCHAR* a_szRegion );
	Region*      GetRegion( const TCHAR* a_szRegion );

	// The pattern textures of a region
	// $deBlob: FUNCTION 005d1400
	TINT GetNumPatternTextures( TINT a_region ) const
	{
		TASSERT( a_region >= 0 && a_region < m_pRuntimeData->m_iNumRegions );
		return m_pRuntimeData->m_pRegions[ a_region ].m_iNumPatternTextures;
	}

	// $deBlob: FUNCTION 005d1470
	PatternTexture* GetPatternTexture( TINT a_region, TINT a_index ) const
	{
		TASSERT( a_index >= 0 && a_index < GetNumPatternTextures( a_region ) );
		return &m_pRuntimeData->m_pRegions[ a_region ].m_pPatternTextures[ a_index ];
	}

	static ARegionManager* GetSingleton() { return sm_instance; }

private:
	static TBOOL          QueueRegionAssets( Region* a_pRegion, RegionState* a_pRegionState );
	static TBOOL          QueueCharacterAssets( Region* a_pRegion, RegionState* a_pRegionState );
	static TBOOL          QueueFXAssets( Region* a_pRegion, RegionState* a_pRegionState );
	static void           LoadSoundGroups( Region* a_pRegion );
	static ATRBLoaderJob* CreateTRBJob( const TCHAR* a_szFileName );
	static void           OnAssetLoaded( ATRBLoaderJob* a_pJob, void* a_pRegionState );
	static void           DeinitAsset( Toshi::TTRB* a_pTRB );

	static void __stdcall OnMaterialLoaded( Toshi::T2Material* a_pMaterial, void* a_pUserData );
	static void __stdcall OnMaterialUnloaded( Toshi::T2Material* a_pMaterial, void* a_pUserData );
	static void __stdcall OnWorldMaterialCreated( Toshi::TWorldMaterial* a_pMaterial, void* a_pT2Material );
	static void __stdcall OnWorldMaterialDestroyed( Toshi::TWorldMaterial* a_pMaterial );

private:
	static ARegionManager* sm_instance;

private:
	RegionState*       m_pRegionStates; // 0x00 one per region
	RegionRuntimeData* m_pRuntimeData;  // 0x04
};                                      // 0x08

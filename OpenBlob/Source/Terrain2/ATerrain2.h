#pragma once
#include "TerrainTypes.h"
#include "ACellBucket.h"

#include <File/TTRB.h>
#include <Render/TWorldVIS.h>
#include <Toshi/T2DList.h>
#include <Toshi/T2SList.h>

namespace Toshi
{
class TWorldMaterial;
} // namespace Toshi

class ATRBLoaderJob;

//-----------------------------------------------------------------------------
// Purpose: the level's terrain: the "Terrain_Main" header of the terrain data block, its regions and the cells that are
// streamed in (each into an ACellBucket) through the load and unload queues.
//-----------------------------------------------------------------------------
class ATerrain2
{
public:
	static TCONSTEXPR_VAR TINT MAX_MATERIALS      = 3000;
	static TCONSTEXPR_VAR TINT MAX_CELL_CALLBACKS = 5;

	using RegionInfo = TerrainTypes::RegionInfo;
	using Region     = TerrainTypes::Region;
	using Header     = TerrainTypes::Header;

	// A cell waiting in the load or the unload queue (0x18 bytes)
	struct CellLoadQueueObject : public Toshi::T2DList<CellLoadQueueObject>::Node
	{
		enum Flag : TUINT32
		{
			Flag_Priority = 1 << 0, // at the front of the queue
		};

		TINT         m_regionIndex; // 0x08
		TINT         m_cellIndex;   // 0x0C
		TUINT32      m_uiFlags;     // 0x10
		ACellBucket* m_bucket;      // 0x14 the bucket the cell is unloaded from (unload queue)
	};

	// The materials ACellBucket gives to the cell instances (0xC bytes)
	struct MaterialEntry : public Toshi::T2GenericSList::Node
	{
		Toshi::TWorldMaterial* m_pMaterial; // 0x04
		TUINT32                m_uiFlags;   // 0x08 1: can be shared by the instances of the cell, 2: light mapped
	};

	// Called with the bucket of a cell once it's loaded / before it's unloaded
	using t_CellCallback = void ( * )( ACellBucket* a_pBucket );

	// Returns the bucket to unload to make room for a queued cell (TNULL: none)
	using t_FindBucketToReuse = ACellBucket* (*)( TBOOL a_bPriority );

public:
	ATerrain2();
	~ATerrain2();

	void Load( Toshi::TTRB* a_pTRB, TBOOL a_bStreaming, TINT a_iNumCells, TINT a_iNumQueueObjects );

	// Sets m_MP_TRB_Header from the level's multiplayer data
	void LoadMPData( Toshi::TTRB* a_pTRB );

	// Processes one object of the unload queue and one of the load queue
	void UpdateQueues();

	// Queues a_iCell for loading (a_bLoad) or unloading, at the front of the queue if a_bPriority
	CellLoadQueueObject* QueueCell( TINT a_iRegion, TINT a_iCell, TBOOL a_bLoad, TBOOL a_bPriority );
	void                 RequestCell( TINT a_iRegion, TINT a_iCell, TBOOL a_bLoad, TBOOL a_bPriority );

	// Starts loading a_iCell into a free bucket
	ACellBucket* LoadCell( TINT a_iRegion, TINT a_iCell );

	// The cell named a_szName (case-sensitive) and its region and index, TNULL if there's none
	TerrainTypes::Cell* FindCell( const TCHAR* a_szName, TINT* a_pRegion, TINT* a_pCell );
	TerrainTypes::Cell* FindCell( const TCHAR* a_szName );
	// Loads the cell named a_szName unless it has flag 0x10000; returns the bucket, or the cell when it isn't loaded
	void* LoadCell( const TCHAR* a_szName );

	// Cancels the loads, empties the load and unload queues and unloads every active cell (the unloaded callbacks
	// are called for the loaded cells if a_bCallCallbacks)
	void UnloadAll( TBOOL a_bCallCallbacks );

	// Unloads every cell without the callbacks and forgets the region's header
	void Release();

	// TRUE while a cell is loading or queued for unloading, or the physics have bodies to add
	TBOOL IsStreaming();

	// The number of active cells that are loading or queued for loading (the debug HUD)
	TINT GetNumLoadingCells();

	// The cells next to a_iCell (TerrainTypes::CellVis::NONE: none), TNULL for CellVis::NONE
	TINT* GetCellVis( TINT a_iRegion, TINT a_iCell );

	TerrainTypes::Cell* GetCell( TINT a_iRegion, TINT a_iCell );
	TINT                GetNumCells( TINT a_iRegion );

	TINT                        GetNumInstances() const { return m_Header->instancecount; }
	TerrainTypes::CellInstance* GetInstances() const { return m_Header->instances; }

	// The multiplayer data of the instances ("MP_Data_Header")
	struct MPInstanceData
	{
		TINT m_iUnk0;         // 0x0
		TINT m_iBuildingRank; // 0x4 the order the game type 2 has the buildings painted in, 0: none
	};

	struct MPHeader
	{
		MPInstanceData* m_pInstances;  // 0x0 one per instance of m_Header
		TINT            instancecount; // 0x4
	};

	// The multiplayer data of a_pInstance
	MPInstanceData* GetMPInstanceData( const TerrainTypes::CellInstance* a_pInstance ) const;

	// $deBlob: FUNCTION 0057b340
	TerrainTypes::CellInstance* GetCellInstance( TINT a_instance ) const
	{
		TASSERT( a_instance < m_Header->instancecount );
		return &m_Header->instances[ a_instance ];
	}

	// The cell at a_pPos (CellVis::NONE: none), a_ppCell (optional) receives it
	TINT GetCellIndex( TINT a_iRegion, const Toshi::TVector4* a_pPos, TerrainTypes::Cell** a_ppCell );

	// TRUE if a_pPos is in a_pCell (in region 0)
	TBOOL IsPosInsideCell( TerrainTypes::Cell* a_pCell, const Toshi::TVector4* a_pPos );

	// Removes the first lower priority load queue object of a cell that isn't next to a_iCell
	void CancelQueuedLoads( TINT a_iRegion, TINT a_iCell, TINT* a_pNeighbours );

	// Takes a material entry of the free list (its UV modes and user data reset)
	MaterialEntry* AllocMaterial();

	// Gives a material entry back to the free list
	void ReleaseMaterial( MaterialEntry* a_pEntry );

	TINT AddCellLoadedCallback( t_CellCallback a_fnCallback );
	TINT AddCellUnloadedCallback( t_CellCallback a_fnCallback );

	// $deBlob: FUNCTION 0061f620
	void RemoveCellLoadedCallback( t_CellCallback a_fnCallback )
	{
		for ( TINT i = MAX_CELL_CALLBACKS - 1; i > 0; i-- )
		{
			if ( m_afnCellLoaded[ i ] == a_fnCallback )
			{
				m_afnCellLoaded[ i ] = TNULL;
				return;
			}
		}
	}

	// $deBlob: FUNCTION 0061f650
	void RemoveCellUnloadedCallback( t_CellCallback a_fnCallback )
	{
		for ( TINT i = MAX_CELL_CALLBACKS - 1; i > 0; i-- )
		{
			if ( m_afnCellUnloaded[ i ] == a_fnCallback )
			{
				m_afnCellUnloaded[ i ] = TNULL;
				return;
			}
		}
	}

	void SetFindBucketToReuseCallback( t_FindBucketToReuse a_fnCallback ) { m_fnFindBucketToReuse = a_fnCallback; }

	t_CellCallback GetCellLoadedCallback( TINT a_iSlot ) const;
	t_CellCallback GetCellUnloadedCallback( TINT a_iSlot ) const;
	// Calls the callbacks a_maxCallbacks - 1 down to 1 with a_pBucket. NOTE: like the slots above, slot 0 is skipped
	static void CallCellCallbacks( t_CellCallback* a_pCallbacks, TINT a_maxCallbacks, ACellBucket* a_pBucket );

	// $deBlob: FUNCTION 0056a740
	Region* GetRegion( TINT a_region ) const
	{
		TASSERT( a_region >= 0 && a_region < m_Header->regionCount );
		return &m_Header->regions[ a_region ];
	}

	// $deBlob: FUNCTION 006253a0
	const Toshi::T2DList<ACellBucket>& GetActiveCells() const { return m_activeCells; }

	// The index of a_pCell in a_region
	// $deBlob: FUNCTION 006253b0
	TINT GetCellIndex( TINT a_region, const TerrainTypes::Cell* a_pCell ) const
	{
		TASSERT( a_region >= 0 && a_region < m_Header->regionCount );
		return TINT( a_pCell - m_Header->regions[ a_region ].cells );
	}

	static ATerrain2* GetInstance() { return sm_Instance; }

private:
	TerrainTypes::Cell* StartLoad( TINT a_iRegion, TINT a_iCell, ACellBucket* a_pBucket );
	void                UnloadQueued( CellLoadQueueObject* a_pObject );
	ACellBucket*        FindActiveBucket( TerrainTypes::Cell* a_pCell );

	static void* __stdcall CellTRBAlloc( Toshi::TTRB::AllocType a_eType, TUINT32 a_uiSize, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData );
	static void __stdcall CellTRBFree( Toshi::TTRB::AllocType a_eType, void* a_pMem, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData );

	static void OnCellTRBLoaded( ATRBLoaderJob* a_pJob, void* a_pBucket );

	static void __stdcall OnRenderCellMesh( Toshi::TTMDWin::TRBWorldCellMesh* a_pCellMesh, Toshi::TWorldVis::RenderData* a_pRenderData, void* a_pCell );

private:
	static ATerrain2* sm_Instance;

	// Draws the bounding spheres of the cell meshes (no writer in retail)
	static TBOOL ms_bDebugDrawCellMeshes;
	// Renders the cell meshes
	static TBOOL ms_bRenderCellMeshes;

private:
	TBOOL                               m_bStreaming;                            // 0x00
	t_CellCallback                      m_afnCellLoaded[ MAX_CELL_CALLBACKS ];   // 0x04 (slot 0 is never used)
	t_CellCallback                      m_afnCellUnloaded[ MAX_CELL_CALLBACKS ]; // 0x18 (slot 0 is never used)
	t_FindBucketToReuse                 m_fnFindBucketToReuse;                   // 0x2C
	Header*                             m_Header;                                // 0x30
	void*                               m_MP_TRB_Header;                         // 0x34
	Toshi::T2DList<ACellBucket>         m_activeCells;                           // 0x38
	Toshi::T2DList<ACellBucket>         m_activeCellFreeList;                    // 0x40
	ACellBucket*                        m_pBuckets;                              // 0x48
	Toshi::T2DList<CellLoadQueueObject> m_loadQueue;                             // 0x4C
	Toshi::T2DList<CellLoadQueueObject> m_unloadQueue;                           // 0x54
	Toshi::T2DList<CellLoadQueueObject> m_queueFreeList;                         // 0x5C
	CellLoadQueueObject*                m_pQueueObjects;                         // 0x64
	MaterialEntry*                      m_pMaterials;                            // 0x68 MAX_MATERIALS of them
	Toshi::T2GenericSList               m_oFreeList;                             // 0x6C the free materials
};

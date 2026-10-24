#pragma once
#include "TerrainTypes.h"

#include <Math/TMatrix44.h>
#include <Math/TPlane.h>
#include <Math/TVector4.h>
#include <Render/TTransformObject.h>
#include <Render/TWorldVIS.h>
#include <Render/TBoxTree.h>
#include <Toshi/T2ResourceHelper.h>

class ACellBucket;

//-----------------------------------------------------------------------------
// Purpose: streams the cells of ATerrain2 around a position (the player's, see AInGameState) and renders them, and
// creates the instances placed in the cells (300 bytes, created by ALoadLevelState).
//-----------------------------------------------------------------------------
class ATerrain2Interface
{
	friend class AJellyShader_DX11; // RenderInstance
	friend class AEnvironmentFX;    // sets ms_afInstanceLODDistancesSq

public:
	static TFLOAT GetInstanceLODDistanceSq( TINT a_iIndex ) { return ms_afInstanceLODDistancesSq[ a_iIndex ]; }

	// Is the LOD 0 bounding sphere of the instance's model in the frustum of the current camera (APaintChallenge's
	// tree targets)
	static TBOOL __stdcall IsInstanceInView( TerrainTypes::CellInstance* a_pInstance );

	static TCONSTEXPR_VAR TINT MAX_RENDERING_CELLS = 96;

	enum Flag : TUINT16
	{
		Flag_CreatingInstances = 1 << 0,
	};

	// Writes the position the cells are streamed around
	using t_GetPosition = void( __stdcall* )( Toshi::TVector4* a_pOutPosition );

	// An entry of the world vis pool (0x8C bytes)
	struct WorldVisEntry
	{
		// $deBlob: FUNCTION 00624e80
		WorldVisEntry()
		{
		}

		// $deBlob: FUNCTION 00624f70
		// $deBlob: FUNCTION 00625230
		~WorldVisEntry()
		{
		}

		Toshi::T2ModelPtr m_pModel;    // 0x00 the model of the world mesh (its resource ID is the key of the pool)
		Toshi::TWorldVis  m_oWorldVis; // 0x04
	};

	// A visible cell, sorted far to near
	struct RenderNode
	{
		RenderNode*         m_pNext;       // 0x00
		TerrainTypes::Cell* m_pCell;       // 0x04
		TFLOAT              m_fDistanceSq; // 0x08 to the camera
	};                                     // 0x0C

	// The low LOD model of the cells that have one (instead of a TModelInstance)
	struct CellLowLOD
	{
		// $deBlob: FUNCTION 00624de0
		CellLowLOD()
		    : m_szModel( TNULL )
		{
		}

		// $deBlob: FUNCTION 00624ee0
		// $deBlob: FUNCTION 00625090
		~CellLowLOD()
		{
		}

		Toshi::TWorldVis  m_oWorldVis; // 0x00
		const TCHAR*      m_szModel;   // 0x88 the cell's m_szLowLODModel (the pool's key)
		Toshi::T2ModelPtr m_pModel;    // 0x8C
	}; // 0x90

	// The TRB record of an animated instance (flag 0x20, CellInstance::m_pAnimRecord). Its first word is the model's
	// ".txs" name until CreateInstanceAnim replaces it with the TModelInstance
	struct InstanceAnimRecord
	{
		union
		{
			const TCHAR*           m_szModel;        // 0x00 the name of the model (without .txs)
			Toshi::TModelInstance* m_pModelInstance; // 0x00 after CreateInstanceAnim
		};
		union
		{
			const TCHAR* m_szBone;  // 0x04
			TINT         m_iBoneID; // 0x04 after CreateInstanceAnim
		};
		union
		{
			const TCHAR* m_szSequence;  // 0x08
			TINT         m_iSequenceID; // 0x08 after CreateInstanceAnim
		};
		TUINT32 m_uiFlags;    // 0x0C (1: random start time, 2: m_fStartTime)
		TFLOAT  m_fStartTime; // 0x10 the start time as a fraction of the sequence
		TINT    m_iSpeed;     // 0x14 the bits of the TAnimation speed
	};

	// An animated instance (0x0C bytes)
	struct InstanceAnim
	{
		// $deBlob: FUNCTION 00623d60
		InstanceAnim()
		{
			m_pBreakpoint    = TNULL;
			m_pModelInstance = TNULL;
			m_pInstance      = TNULL;
		}

		Toshi::TModelInstance*      m_pModelInstance; // 0x00
		TerrainTypes::CellInstance* m_pInstance;      // 0x04
		void*                       m_pBreakpoint;         // 0x08 deleted through its virtual destructor

		~InstanceAnim();
	};

public:
	ATerrain2Interface( TINT a_iRegion );
	~ATerrain2Interface();

	// Loads the cell at a_pPosition and its neighbours (streamed terrain)
	void Init( const Toshi::TVector4* a_pPosition, const Toshi::TVector4* a_pDirection );

	// Loads every cell (the terrain isn't streamed)
	void InitAllCells();

	// Counts the instances of the cells and starts creating them (UpdateInstanceCreation)
	void StartInstanceCreation();

	// Creates the instances of a few cells (in up to 0.95 frames if a_bTimed), FALSE once they're all created
	TBOOL UpdateInstanceCreation( TBOOL a_bTimed );

	// The level restart (AInGameState): unloads the cells (and requests them all again when not streaming), resets the
	// visibility of the instances, their moved bodies and their type's state
	void ResetAllCells();

	void Update( TFLOAT a_fDeltaTime );

	// Culls the cells of the region against the camera frustum and renders the visible ones
	void Render();

	// TRUE while ATerrain2 streams
	TBOOL IsStreaming();

	// TRUE once the cell at the streaming position is loaded and its instances have their physics
	TBOOL IsCurrentCellReady();

	// IsCurrentCellReady and none of the visible neighbour cells is still loading (AKeyFrameCamera only plays on then);
	TBOOL IsCurrentCellAreaReady();

	// FALSE if a_rPosition is in a cell (or, with a_bCheckNeighbours, a neighbour cell containing it) that isn't
	// CellRuntimeFlag_Ready
	TBOOL IsPositionInReadyCell( const Toshi::TVector4& a_rPosition, TBOOL a_bCheckNeighbours );

	// $deBlob: FUNCTION 005c8aa0
	void SetPositionCallback( t_GetPosition a_fnCallback ) { m_fnGetPosition = a_fnCallback; }

	static ATerrain2Interface* GetInstance() { return sm_Instance; }

	// Adds a_pInstance to (a_bAdd) or removes it from the list of the extra instances rendered with its cell (at most
	// 8 of them), the flag 0x20000000 of the instance says it is in the list (ACollisionInstanceMover
	// moves a hit instance in and out)
	void SetInstanceInCellList( TerrainTypes::CellInstance* a_pInstance, TBOOL a_bAdd );

	// The world vis of a flag 0x40 instance (TNULL: none)
	// $deBlob: FUNCTION 00554cb0
	Toshi::TWorldVis* GetWorldVis( TerrainTypes::CellInstance* a_instance )
	{
		if ( a_instance->worldVisIndex == 0xFF )
			return TNULL;

		TASSERT( a_instance->worldVisIndex >= 0 && a_instance->worldVisIndex < m_worldVisPoolSize );
		return &m_pWorldVisPool[ a_instance->worldVisIndex ].m_oWorldVis;
	}
	TINT             GetCurrentRegion() const { return m_currentRegion; }
	Toshi::TBoxTree* GetPerCellInstancesVisTrees() const { return m_pPerCellInstancesVisTree; }

private:
	void CreateInstances();

	// Sets up an animated instance (flag 0x20): replaces the name in its TRB record with the model instance and adds
	// the animation to the instance's skeleton
	void CreateInstanceAnim( TerrainTypes::CellInstance* a_pInstance );

	// Adds the collision (type 0x40) of the instances of the 4 cells nearest to the player and removes the one of
	// the cells that aren't any more (single player only)
	void UpdateNearCellCollision();

	// Adds/removes the type 0x40 collision of the instances of a_pCell
	static void AddCellInstanceCollision( TerrainTypes::Cell* a_pCell );
	static void RemoveCellInstanceCollision( TerrainTypes::Cell* a_pCell );

	// The ACollisionInstance2 of a cell instance (owner type 3), TNULL: the model has no collision
	static class ACollisionInstance2* CreateInstanceCollision( TerrainTypes::CellInstance* a_pInstance, Toshi::TModelInstance* a_pModelInstance );
	// Allocates the low LOD pool of the region and gives the cells with a low LOD their pool entry (or a TModelInstance);
	// NOTE: shared by Init and InitAllCells
	void InitCellLowLODs();
	// Allocates the world vis pool of the region and gives each flag 0x40 instance the entry of its model (new entries
	// for the models that have none yet)
	void InitWorldVisPool();
	void InitCellInstances( TerrainTypes::Cell* a_pCell );
	void CreatePerCellInstancesVisTree();

	// The teardown of the destructor: unloads the cells, then destroys the animated instances, the cell instances and
	// the low LOD cells (the destructor calls them in this order)
	void UnloadAllCells();
	void DestroyInstanceAnims();
	void DestroyCellInstances();
	void DestroyCellLowLODs();

	void DestroyInstances();
	void NotifyCellLoaded( ACellBucket* a_pBucket );
	void CreateVisTrees();

public:
	// Also called by ABlobEntity::PrepareRespawn
	void UpdateStreaming();

private:
	void UpdateCellsReady();

	// Updates the animated instances (flag 0x20) of the visible cells
	void UpdateInstanceAnims( TFLOAT a_fDeltaTime );

	void AddRenderNode( TerrainTypes::Cell* a_pCell, TFLOAT a_fDistanceSq );
	void RenderCell( TINT a_iCell, TerrainTypes::Cell* a_pCell, Toshi::TMatrix44* a_pUnused, const Toshi::TMatrix44& a_rProjection, const Toshi::TPlane* a_pPlanes );

	void OnCellLoaded( ACellBucket* a_pBucket );
	void OnCellUnloaded( ACellBucket* a_pBucket );

	// OnCellLoaded's work for one instance, and the collision parts of OnCellLoaded/OnCellUnloaded
	void        AddInstanceToWorld( ACellBucket* a_pBucket, TerrainTypes::CellInstance* a_pInstance );
	static void AddInstanceCollision( TerrainTypes::Cell* a_pCell, TerrainTypes::CellInstance* a_pInstance, TUINT32 a_uiMask );
	static void RemoveInstanceCollision( TerrainTypes::Cell* a_pCell, TerrainTypes::CellInstance* a_pInstance, TUINT32 a_uiMask );

	static void RenderCellModel( TerrainTypes::Cell* a_pCell, Toshi::TMatrix44* a_pUnused, const Toshi::TMatrix44& a_rProjection );
	// Draws a low LOD cell: its pool entry's world vis (flag CellRuntimeFlag_LowLODPool) or its TModelInstance
	void RenderCellLowLOD( TerrainTypes::Cell* a_pCell, Toshi::TMatrix44* a_pUnused, const Toshi::TMatrix44& a_rProjection );

	// The world vis callback of a low LOD cell (user data: the cell)
	static void __stdcall OnRenderCellLowLODMesh( Toshi::TTMDWin::TRBWorldCellMesh* a_pCellMesh, Toshi::TWorldVis::RenderData* a_pRenderData, void* a_pCell );

	static ACellBucket* FindBucketToReuse( TBOOL a_bPriority );

	// Renders an instance of a cell (the render function of the cell box trees); a_pUserData: the projection
	static void RenderInstance( void* a_pInstance, void* a_pUserData );
	static void OnCellLoadedCallback( ACellBucket* a_pBucket );
	static void OnCellUnloadedCallback( ACellBucket* a_pBucket );

private:
	static ATerrain2Interface* sm_Instance;

	// The squared distances of the instance LODs
	static TFLOAT ms_afInstanceLODDistancesSq[ 4 ];

	// Removes the queued loads of the cells that aren't next to the new cell (no writer in retail)
	static TBOOL ms_bCancelFarLoads;

	// Cleared while the box trees of the cell instances are rendered
	static TBOOL ms_bCullInstances;

	// Draws the world meshes of the low LOD cells (their callback); never written in retail
	static TBOOL ms_bRenderLowLODMeshes;

	// The shade coefficients of the instances (never written in retail: 0)
	static TFLOAT ms_fInstanceShadeCoeffHigh;
	static TFLOAT ms_fInstanceShadeCoeffFade;
	static TFLOAT ms_fInstanceShadeCoeffNear;
	static TFLOAT ms_fInstanceShadeCoeffLow;

private:
	TINT                    m_currentRegion;            // 0x000
	TINT                    m_iCurrentCell;             // 0x004 CellVis::NONE: none
	Toshi::TVector4         m_vPosition;                // 0x008 the streaming position
	Toshi::TVector4         m_vStartPosition;           // 0x018
	t_GetPosition           m_fnGetPosition;            // 0x028
	TBOOL                   m_bStreaming;               // 0x02C
	InstanceAnim*           m_pInstAnims;               // 0x030
	TINT                    m_numInstAnims;             // 0x034
	Toshi::TBoxTree*        m_pPerCellInstancesVisTree; // 0x038 one per cell of the region (GetPerCellInstancesVisTrees)
	TUINT32*                m_apInstanceFades[ 4 ];     // 0x03C two words per instance of the level
	TFLOAT                  m_fTime;                    // 0x04C
	TUINT16                 m_uiFlags;                  // 0x050
	TUINT16                 m_uiCreateCell;             // 0x052
	TUINT16                 m_uiCreateInstance;         // 0x054
	void*                   m_pAnimSheet;               // 0x058 the "Main" symbol of the level's TRB (GetAnimSheet)
	CellLowLOD*             m_pCellLowLODPool;          // 0x05C
	TINT16                  m_numCellLowLODPool;        // 0x060
	Toshi::TTransformObject m_oTransform;               // 0x064
	TUINT32                 m_worldVisPoolSize;         // 0x108
	WorldVisEntry*          m_pWorldVisPool;            // 0x10C
	void**                  m_ppCellInstanceLists;      // 0x110 per cell (0x20 bytes each)
	TINT*                   m_piNumCellInstances;       // 0x114 per cell
	RenderNode*             m_pRenderNodes;             // 0x118 MAX_RENDERING_CELLS of them
	RenderNode*             m_renderingCellsRoot;       // 0x11C
	TINT                    m_usedRenderNodes;          // 0x120
	TINT*                   m_piNearCells;                 // 0x124 4 of them
	TBOOL                   m_bSinglePlayer;            // 0x128
	TBOOL                   m_bUseBlobPosition;                  // 0x129
};

#pragma once
#include <Math/TMatrix44.h>
#include <Math/TVector4.h>
#include <Toshi/T2DList.h>
#include <Platform/DX11/TWorldShader_DX11.h>

namespace Toshi
{
class TMaterial;
class TModelInstance;
class TTexture;
class TWorldVis;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: the data of the "Terrain_Main" symbol of a level's terrain TRB (regions and their cells) and the runtime
// flags of the cells.
//-----------------------------------------------------------------------------
namespace TerrainTypes
{

enum CellFlag : TUINT32
{
	CellFlag_Unk1      = 1 << 0,
	CellFlag_HasModel  = 1 << 1, // the cell's TRB has a world model (CellData)
	CellFlag_HasLowLOD = 1 << 2, // a low LOD model of the level's models is shown instead
	CellFlag_Collision = 1 << 3, // the cell's model has its collision

	CellRuntimeFlag_Loaded       = 1 << 16,
	CellRuntimeFlag_Loading      = 1 << 17,
	CellRuntimeFlag_CancelLoad   = 1 << 18,
	CellRuntimeFlag_Visible      = 1 << 19, // in the frustum this frame (ATerrain2Interface::Render)
	CellRuntimeFlag_Unloading    = 1 << 20,
	CellRuntimeFlag_QueuedUnload = 1 << 21,
	CellRuntimeFlag_QueuedLoad   = 1 << 22,
	CellRuntimeFlag_LowLODPool   = 1 << 23, // m_pLowLOD is an entry of ATerrain2Interface's low LOD pool
	CellRuntimeFlag_Hidden       = 1 << 24,
	CellRuntimeFlag_Ready        = 1 << 26,
};

// The type of a cell instance (CellInstance::m_uiFlags & 7)
enum InstanceType : TUINT32
{
	InstanceType_Model       = 0,
	InstanceType_Building    = 1, // ABuildingInstance
	InstanceType_Landmark    = 2, // ALandmarkInstance
	InstanceType_Tree        = 3, // ATreeInstance
	InstanceType_Grass       = 4, // AGrassInstance
	InstanceType_TextureSwap = 5, // ATextureSwapInstance
	InstanceType_ToggleVis   = 6, // AToggleVisInstance
	InstanceType_ZJump       = 7,
};

// The flags of a cell instance (CellInstance::m_uiFlags, the type is in the low 3 bits).
enum InstanceFlags : TUINT32
{
	InstanceFlags_ScaleOnVisToggle = 1 << 12, // 0x1000 the scale follows the toggle's curve
	InstanceFlags_GlowOnVisToggle  = 1 << 13, // 0x2000 the glow follows the toggle
	InstanceFlags_TransitioningIn  = 1 << 27, // 0x8000000 the toggle is running
	InstanceFlags_Visible          = 1 << 23, // 0x800000 (ATerrain2Interface::RenderInstance)
	InstanceFlags_TransitioningOut = 1 << 28, // 0x10000000 the toggle is leaving
};

struct CellData
{
	const TCHAR*           m_szTRB;            // 0x00 the file of the cell
	Toshi::TModelInstance* m_pModelInstance;   // 0x04 of the cell's world model
	void*                  m_pCollision;       // 0x08 ACollisionInstance2 (CellFlag_Collision)
	void*                  m_pCollisionObject; // 0x0C ACollisionObject (CellFlag_Collision)
	Toshi::TWorldVis*      m_pWorldVis;        // 0x10 of the world model
};

struct Cell;

// The meshes of a flag 0x40 instance (a symbol of the cell's TRB): each is drawn as an instance of the cell model's
// world mesh it names
struct InstanceMeshes
{
	TUINT32                                m_uiNumMeshes; // 0x00
	Toshi::TWorldMeshInstanceData::Source* m_pMeshes;     // 0x04 (0x1C bytes each)
	Toshi::TVector3                        m_vCentre;     // 0x08 of the bounding box of the meshes
	Toshi::TVector3                        m_vExtents;    // 0x14
};

// The data of a grass instance (TerrainTypes::InstanceType_Grass, CellInstance::m_pTypeData)
struct GrassData
{
	TUINT32       m_uiUnk00;        // 0x00
	const TCHAR** m_ppTextures;     // 0x04
	TUINT8        m_aUnk08[ 0x10 ]; // 0x08
	TINT16        numtextures;      // 0x18
};

// A texture swapped for another one (instance flag 0x2000000, CellInstance::m_pTypeData of a type 5 instance, a
// list through m_pNext)
struct TextureOverride
{
	const TCHAR*      m_szName;    // 0x00 the name of the texture to replace
	Toshi::TTexture*  m_pTexture;  // 0x04 the new texture (its name until ATextureSwapInstance::OnCreated loads it)
	Toshi::TMaterial* m_pSource;   // 0x08 the material of the model's mesh the texture is found in (OnCreated)
	Toshi::TMaterial* m_pMaterial; // 0x0C the copy of m_pSource with the new texture (OnCreated)
	void*             m_pClones;   // 0x10 the copies of the materials of the instance's meshes (OnPaint)
	TextureOverride*  m_pNext;     // 0x14
};
TSTATICASSERT( sizeof( TextureOverride ) == 0x18 );

struct CellInstance
{
	struct ZJump
	{
		TINT16 m_iSet;  // 0x30
		TINT16 m_iNode; // 0x32
	};

	Toshi::TMatrix44*      m_pMatrix;         // 0x00
	TUINT32                m_uiFlags;         // 0x04 (& 7: the type)
	const TCHAR*           m_szModel;         // 0x08 the streamed model (without .trb)
	const TCHAR*           m_szLowLOD;        // 0x0C the low LOD model
	Toshi::TVector4**      m_ppCentre;        // 0x10 the centre of the instance (through two pointers)
	void*                  m_pModelInstance2; // 0x14 AModelInstance2 of m_szModel
	Toshi::TModelInstance* m_pLowLODInstance; // 0x18
	TUINT32                m_uiUnk1C;         // 0x1C
	const TCHAR*           symbol;            // 0x20 a symbol of the cell's TRB (flag 0x40)
	InstanceMeshes*        m_pSymbolData;     // 0x24
	void*                  m_pUnk28;          // 0x28
	Cell*                  m_pCell;           // 0x2C set when the instance is created
	union
	{
		void*   m_pTypeData; // 0x30 (type 2)
		TUINT32 m_uiColour;  // 0x30 (type 1 without a streamed model: the ARGB colour)
		ZJump   zjump;       // 0x30 (type 7)
	};
	void*  m_pLinks;             // 0x34 type 2: ALandmarkInstance::Links
	TUINT8 worldVisIndex;        // 0x38 into ATerrain2Interface's world vis pool (0xFF: none)
	TUINT8 m_uiTeam;             // 0x39
	TINT16 m_iAmbientScaleIndex; // 0x3A into ALightingData's instance ambient scales
	void*  m_pAnimRecord;        // 0x3C the animation record of a flag 0x20 instance (ATerrain2Interface::InstanceAnimRecord,
	                             // its first word becomes the TModelInstance* in CreateInstanceAnim)
};

struct Cell
{
	const TCHAR*           m_szName;         // 0x00
	TUINT32                m_uiUnk04;        // 0x04
	Toshi::TVector4*       m_pCentre;        // 0x08 of the bounding box
	Toshi::TVector4*       m_pExtents;       // 0x0C of the bounding box
	CellData*              m_pData;          // 0x10
	const TCHAR*           m_szLowLODModel;  // 0x14 the name of the low LOD model (CellFlag_HasLowLOD)
	void*                  m_pLowLOD;        // 0x18 TModelInstance, or an ATerrain2Interface low LOD pool entry
	Toshi::TMatrix44*      m_pMatrix;        // 0x1C
	TUINT32                m_uiFlags;        // 0x20 CellFlag
	TUINT32                m_uiNumInstances; // 0x24
	CellInstance*          m_pInstances;     // 0x28
	void**                 m_ppUnk2C;        // 0x2C
	TUINT32                m_uiNumUnk2C;     // 0x30
	void*                  m_pUnk34;         // 0x34 a box tree of ATerrain2Interface
	Toshi::T2GenericDList* m_pUnk38;         // 0x38 created by ATerrain2Interface::InitCellInstances
}; // 0x3C

// The (up to 8) cells next to a cell, -1 for none
struct CellVis
{
	enum
	{
		NONE = -1
	};

	TINT m_aNeighbours[ 8 ];
}; // 0x20

// The bounds of a region (the first cell's corner and the size in cells, 150 units each), followed by the index of
// the cell of each square (0xff: none)
struct RegionInfo
{
	TFLOAT m_fX;            // 0x00
	TFLOAT m_fZ;            // 0x04
	TUINT8 m_uiSizeX;       // 0x08
	TUINT8 m_uiSizeZ;       // 0x09
	TUINT8 m_auiCells[ 1 ]; // 0x0A m_uiSizeX * m_uiSizeZ of them
};

// An enemy pool of a region (8 bytes)
struct EnemyPool
{
	TINT m_eType;  // 0x00 AEnemyManager::EnemyType
	TINT m_iCount; // 0x04
};

struct Region
{
	Cell*       cells;             // 0x00
	CellVis*    cellVis;           // 0x04
	TINT        cellCount;         // 0x08
	RegionInfo* m_pInfo;           // 0x0C
	TINT16      m_iNumCellLowLODs; // 0x10 the size of ATerrain2Interface's low LOD pool
	TUINT32     numInstAnimModel;  // 0x14 the size of ATerrain2Interface's animated instances
	EnemyPool*  m_pEnemyPools;     // 0x18 the enemy pools of the region (AEnemyManager::InitEnemyPool)
	TINT        m_iNumEnemyPools;  // 0x1C
	TUINT32     m_uiNumWorldVis;   // 0x20 the size of ATerrain2Interface's world vis pool
};                                 // 0x24

struct Header
{
	Region*       regions;       // 0x00
	TINT          regionCount;   // 0x04
	CellInstance* instances;     // 0x08 every instance of the level
	TINT          instancecount; // 0x0C
};

} // namespace TerrainTypes

#pragma once
#include "TerrainTypes.h"

#include <Core/TMemory.h>
#include <File/TTRB.h>
#include <Toshi/T2DList.h>
#include <Toshi/T2SList.h>

class AMaterialInfoPool;
class AUVAnimationPool;

namespace Toshi
{
class T2Material;
class TWorldMaterial;
class TWorldMaterialHAL;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: the memory a streamed terrain cell is loaded into: its own 10 MB heap, the cell's TRB (its world model) and
// the material info / UV animation pools of the cell's materials.
// ATerrain2 owns a fixed number of them (m_activeCells / m_activeCellFreeList).
//-----------------------------------------------------------------------------
class ACellBucket : public Toshi::T2DList<ACellBucket>::Node
{
public:
	enum Flag : TUINT16
	{
		Flag_AssetsInitialised = 1 << 0,
	};

public:
	ACellBucket();
	~ACellBucket();

	// Forgets the cell and gives its used materials back to ATerrain2
	void Reset();

	void Set( TerrainTypes::Cell* a_pCell, TINT a_iRegion, TINT a_iCell );

	// Initialises the loaded TRB: the cell's world model instance, its collision and world visibility
	void OnLoaded();

	// Creates the physics bodies of the cell model's collision (CellData::m_pCollision) at a_rMatrix
	void CreateCollision( const Toshi::TMatrix44& a_rMatrix );

	// Gives the used materials of the cell's instances back to ATerrain2
	void ReleaseMaterials();

	// Creates (or shares) the world materials of the meshes of a flag 0x40 instance (ATerrain2Interface::OnCellLoaded);
	void CreateInstanceMaterials( TerrainTypes::CellInstance* a_pInstance );

	// Destroys what OnLoaded created and closes the TRB
	void Unload();

	// ATerrain2_Win.cpp: creates the heap of the bucket ("terrain<a_iIndex>")
	void AllocateMemory( TINT a_iIndex );

	TerrainTypes::Cell* GetCell() const { return m_pCell; }
	TINT                GetRegionIndex() const { return m_iRegion; }
	TINT                GetCellIndex() const { return m_iCell; }
	TMemoryHeap*        GetHeap() const { return m_heap; }
	Toshi::TTRB*        GetTRB() const { return m_pTRB; }
	void                SetTRB( Toshi::TTRB* a_pTRB ) { m_pTRB = a_pTRB; }

private:
	static void __stdcall OnMaterialLoaded( Toshi::T2Material* a_pMaterial, void* a_pBucket );
	static void __stdcall OnMaterialUnloaded( Toshi::T2Material* a_pMaterial, void* a_pBucket );
	static void __stdcall OnWorldMaterialCreated( Toshi::TWorldMaterial* a_pMaterial, void* a_pT2Material );
	static void __stdcall OnWorldMaterialDestroyed( Toshi::TWorldMaterial* a_pMaterial );

private:
	// The bucket whose TRB is being initialised or deinitialised
	static ACellBucket* ms_pCurrentBucket;

private:
	TerrainTypes::Cell*   m_pCell;            // 0x08
	TINT                  m_iRegion;          // 0x0C
	TINT                  m_iCell;            // 0x10
	TMemoryHeap*          m_heap;             // 0x14
	void*                 m_pUnk18;           // 0x18 freed by the destructor (no writer found)
	Toshi::TTRB*          m_pTRB;             // 0x1C
	AMaterialInfoPool*    m_pMatInfoPool;     // 0x20
	AUVAnimationPool*     m_pUVAnimPool;      // 0x24
	TCHAR                 m_szHeapName[ 16 ]; // 0x28
	TUINT16               m_uiFlags;          // 0x38
	Toshi::T2GenericSList m_oUsedList;        // 0x3C the ATerrain2::MaterialEntry of the cell's instances
};

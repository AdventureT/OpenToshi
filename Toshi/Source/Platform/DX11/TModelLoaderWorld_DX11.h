#pragma once
#include "Render/TModel.h"
#include "Render/TTMDWin.h"
#include "Shaders/WorldShader/TWorldShader.h"

TOSHI_NAMESPACE_START

class TMaterial;

//-----------------------------------------------------------------------------
// Purpose: creates the world meshes of a "twld" TRB asset, used by TAssetInit::InitWorld.
//-----------------------------------------------------------------------------
class TModelLoaderWorld_DX11
{
public:
	static TCONSTEXPR_VAR TINT LOCALMATS_COUNT = 1024;

	struct CellRef
	{
		TTMDWin::TRBWorldCell* pCell; // 0x00
		TINT                   iUnk;  // 0x04 always 0
	};

public:
	static void LoadWorldMeshTRB( TTMDWin::TRBWorldData* a_pWorldData, TModelLOD* a_pModelLOD );
	static void LoadTreeIntersect( void* a_pNode, CellRef* a_pCell, TModelLOD* a_pModelLOD );

	// NOTE: TWorldShader's static (0x009b26c0), kept here for TAssetInit_DX11
	static TMemoryHeap* GetWorldMemHeap() { return TWorldShader::GetWorldMemHeap(); }
	static void         SetWorldMemHeap( TMemoryHeap* a_pHeap ) { TWorldShader::SetWorldMemHeap( a_pHeap ); }

	// The created material named a_szName (case-insensitive), TNULL if none
	static TWorldMaterial* FindCreatedMaterial( const TCHAR* a_szName );

private:
	static TWorldMaterial* sm_createdMaterials[ LOCALMATS_COUNT ];
	static TINT            sm_createdMaterialsCount;
	static TINT            s_iMeshLoadCount;
	static const TClass*   s_pWorldMaterialClass; // looked up and never used
};

TOSHI_NAMESPACE_END

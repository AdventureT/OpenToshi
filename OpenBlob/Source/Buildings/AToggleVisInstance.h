#pragma once
#include "Terrain2/TerrainTypes.h"

#include <Math/TVector4.h>

class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: a cell instance that toggles its visibility when Blob paints or inks the level (Source\Buildings\
// AToggleVisInstance.cpp, cell instance type 6, TerrainTypes::InstanceType_ToggleVis). Its instances are plain
// AModelInstance2 (AModel2::CreateInstance's default case), so the class only holds the callbacks of the type in
// ATerrain2Interface's per type tables. The scale and glow transitions (TerrainTypes::InstanceFlags_*) run from the
// tick. NOTE: the file name and the TerrainTypes::InstanceType_ToggleVis / InstanceFlags_ScaleOnVisToggle,
// GlowOnVisToggle
//-----------------------------------------------------------------------------
class AToggleVisInstance
{
public:
	// The callbacks of TerrainTypes::InstanceType_ToggleVis in ATerrain2Interface's per type tables
	static void  OnCreated( TerrainTypes::CellInstance* a_pInstance );
	static void  OnBlobTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const Toshi::TVector4* a_pPosition );
	static void  OnAddedToWorld( TerrainTypes::CellInstance* a_pInstance );
	static TBOOL OnTick( TerrainTypes::CellInstance* a_pInstance, TFLOAT a_fDeltaTime, TFLOAT a_fTime );
	static void  OnReset( TerrainTypes::CellInstance* a_pInstance );
	static void  OnPaint( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob );

	// Ends the glow of the materials of the instance (the glow materials go back to the cell's own materials)
	static void ResetGlowMaterials( TerrainTypes::CellInstance* a_pInstance );
};

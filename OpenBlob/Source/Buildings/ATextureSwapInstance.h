#pragma once
#include "Terrain2/TerrainTypes.h"
#include "Shaders/AShaderEffects.h"

class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: a cell instance whose meshes take other textures when Blob paints it (Source\Buildings\
// ATextureSwapInstance.cpp, cell instance type 5, TerrainTypes::InstanceType_TextureSwap). Its instances are plain
// AModelInstance2 (AModel2::CreateInstance's default case), so the class only holds the callbacks of the type in
// ATerrain2Interface's per type tables. The texture overrides are TerrainTypes::TextureOverride
// (CellInstance::m_pTypeData).
//-----------------------------------------------------------------------------
class ATextureSwapInstance
{
public:
	// The copy of a material of the instance's meshes with the textures swapped (TextureOverride::m_pClones), made by
	// AShaderEffects::AddClone; the copies of one override are chained by m_pNextCopy (+0x20)
	using Clone = AShaderEffects::Clone;

public:
	// The callbacks of TerrainTypes::InstanceType_TextureSwap in ATerrain2Interface's per type tables
	static void OnCreated( TerrainTypes::CellInstance* a_pInstance );
	static void OnBlobTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const Toshi::TVector4* a_pPosition );
	static void OnPaint( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob );
	static void OnRemovedFromWorld( TerrainTypes::CellInstance* a_pInstance );
	static void OnReset( TerrainTypes::CellInstance* a_pInstance );

	// Frees the copies of the materials of every override (the reset and the removal of the instance)
	static void ClearClones( TerrainTypes::CellInstance* a_pInstance );

	// The release callback of a copy (AShaderEffects::Update): unlinks it from its override's list. The override is a
	// TerrainTypes::TextureOverride.
	static void __stdcall UnlinkClone( Clone* a_pClone, void* a_pOverride );
};

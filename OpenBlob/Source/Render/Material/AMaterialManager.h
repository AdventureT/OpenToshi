#pragma once
#include "Render/Material/AMaterialInfo.h"
#include "Render/UVAnimation/AUVAnimation.h"

#include <Toshi/TSingleton.h>
#include <File/TTRB.h>

namespace TerrainTypes
{
struct InstanceMeshes;
}

//-----------------------------------------------------------------------------
// Purpose: the UV animations of the materials: an AMaterialInfo per animated meta material (T2Material with a UV
// function type 1..3 on a pass), with an AUVAnimation per animated pass. ARenderer advances m_fTime/m_iFrame.
//-----------------------------------------------------------------------------
class AMaterialManager
    : public Toshi::TSingleton<AMaterialManager>
{
public:
	// T2Material::GetPassUVFunctionType
	enum AUVFunctionType
	{
		AUVFunctionType_None   = 0,
		AUVFunctionType_Offset = 1,
		AUVFunctionType_Unk2   = 2,
		AUVFunctionType_Unk3   = 3,
		AUVFunctionType_Unk4   = 4, // no animation
		AUVFunctionType_NumOf,
	};

public:
	AMaterialManager();
	~AMaterialManager();

	// Sets the UV offsets of a_pMaterial's passes from its AMaterialInfo (a_bShiftPass: a TWorldMaterialHAL's
	// passes are offset by one, the last one skipped)
	void UpdateUVs( Toshi::TMaterial* a_pMaterial, TBOOL a_bShiftPass, TBOOL a_bUnk );

	// UpdateUVs for the materials of a_pModel's LOD 0
	void UpdateUVs( Toshi::TModel* a_pModel );

	// UpdateUVs for the materials of the meshes of a flag 0x40 cell instance (the pass after the light map shifted)
	void UpdateUVs( TerrainTypes::InstanceMeshes* a_pMeshes );

	static const TCHAR* GetBlendModeString( TINT a_eBlendMode );

	// TTRUE if a pass of a_pMaterial has an animated UV function type
	static TBOOL HasUVAnimation( Toshi::T2Material* a_pMaterial );

	// CreateMaterialUVAnimation for every meta material ("tmat" symbol) of a_pTRB
	void CreateMaterialUVAnimations( Toshi::TTRB* a_pTRB, AMaterialInfoPool* a_pPool, AUVAnimationPool* a_pUVPool );

	// DestroyMaterialUVAnimation for every meta material of a_pTRB
	void DestroyMaterialUVAnimations( Toshi::TTRB* a_pTRB, AMaterialInfoPool* a_pPool );

	// Finds or creates the AMaterialInfo of a_pMaterial; TNULL if it isn't animated. The pools default to the common ones
	AMaterialInfo* CreateMaterialUVAnimation( Toshi::T2Material* a_pMaterial, AMaterialInfoPool* a_pPool, AUVAnimationPool* a_pUVPool );

	void DestroyMaterialUVAnimation( Toshi::T2Material* a_pMaterial, AMaterialInfoPool* a_pPool );

	// Sets the UV animation of the materials of a_pModel's LOD 0 from their meta materials
	void SetupModelUVAnimations( Toshi::TModel* a_pModel, AMaterialInfoPool* a_pPool );

	AMaterialInfo* FindMaterialUVAnimation( Toshi::T2Material* a_pMaterial, AMaterialInfoPool* a_pPool );
	AMaterialInfo* FindMaterialUVAnimation( const TCHAR* a_szMaterialName, AMaterialInfoPool* a_pPool );

	// Stops (a_bPaused) or resumes the UV animations
	void SetPaused( TBOOL a_bPaused ) { m_bPaused = a_bPaused; }

private:
	friend class ARenderer; // MainScene advances the clock

	TBOOL             m_bPaused;           // 0x00 no UV updates
	TFLOAT            m_fTime;             // 0x04
	TINT              m_iFrame;            // 0x08
	AMaterialInfoPool m_oMaterialInfoPool; // 0x0C "Common Mat Info Pool"
	AUVAnimationPool  m_oUVAnimPool;       // 0x28 "Common UV Anim Pool"
};                                         // 0x44

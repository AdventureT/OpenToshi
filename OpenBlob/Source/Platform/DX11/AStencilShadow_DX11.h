#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Math/TVector4.h>

namespace Toshi
{
class TModelInstance;
class TMaterial;
class TPhysicsPhantomContainer;
} // namespace Toshi

class ACamera;
class ASkinMaterial;

//-----------------------------------------------------------------------------
// Purpose: the stencil shadows under the players' blobs. Each blob has an instance of blob_shad(_mp).txs, put on the
// ground under the blob (two rays: the decal world and the gameplay world) and animated like the blob's character
// model; it's rendered twice with the BLENDMODE_STENCIL skin materials of its two passes (ASkinShader's stencil order
// tables).
// Created by ALoadLevelState::LoadGUI, updated by AInGameState after the physics, rendered by ARenderer::MainScene.
//-----------------------------------------------------------------------------
class AStencilShadow
    : public Toshi::TSingleton<AStencilShadow>
{
public:
	static TCONSTEXPR_VAR TINT MAX_SHADOWS   = 4;
	static TCONSTEXPR_VAR TINT MAX_MESHES    = 4;
	static TCONSTEXPR_VAR TINT NUM_PASSES    = 2;
	static TCONSTEXPR_VAR TINT MAX_MATERIALS = MAX_MESHES * NUM_PASSES;
	static TCONSTEXPR_VAR TINT MAX_SEQUENCES = 256;
	static TCONSTEXPR_VAR TINT MAX_ANIMS     = 4;

	// A stencil material (0xC bytes)
	struct MatAlloc
	    : public Toshi::T2DList<MatAlloc>::Node
	{
		Toshi::TMaterial* m_pMat; // 0x08
	};

public:
	AStencilShadow();
	~AStencilShadow();

	// Puts the shadows on the ground under the blobs and copies the blobs' animations
	void Update( TFLOAT a_fDeltaTime );

	// Renders the shadows of the visible blobs (both passes); the arguments aren't used
	void Render( TFLOAT a_fDeltaTime, ACamera* a_pCamera );

	// The position of shadow a_iIndex
	void GetShadowPosition( TINT a_iIndex, Toshi::TVector4& a_rOutPosition );

private:
	// Gives the meshes of a_pInstance's model the materials of a_iPass
	void SetPassMaterials( Toshi::TModelInstance* a_pInstance, TINT a_iPass );

public:
	// The shade drawn into the stencil (ASkinShader's BLENDMODE_STENCIL pass colour), set per blob by Render
	static TUINT8 ms_uiStencilColour;

private:
	// The passes that are rendered
	static TBOOL ms_bRenderPass0;
	static TBOOL ms_bRenderPass1;

private:
	Toshi::TModelInstance*           m_apInstances[ MAX_SHADOWS ];              // 0x000 blob_shad(_mp).txs
	Toshi::TPhysicsPhantomContainer* m_apGroundPhantoms[ MAX_SHADOWS ];         // 0x010 the gameplay world (0x4cf000)
	Toshi::TPhysicsPhantomContainer* m_apDecalPhantoms[ MAX_SHADOWS ];          // 0x020 the decal world (0x8000)
	TBOOL                            m_abHidden[ MAX_SHADOWS ];                 // 0x030 no ground under the blob, or in water/ink
	MatAlloc                         m_oMatAlloc[ MAX_MATERIALS ];              // 0x034
	Toshi::T2DList<MatAlloc>         m_oMatFreeList;                            // 0x094
	Toshi::TMaterial*                m_apOriginalMaterials[ MAX_MESHES ];       // 0x09C the shadow model's own
	ASkinMaterial*                   m_apMaterials[ NUM_PASSES ][ MAX_MESHES ]; // 0x0AC
	TINT                             m_aiShadowSequences[ MAX_SEQUENCES ];      // 0x0CC the shadow's sequence of each of the blob's
	TINT                             m_aiBlobSequences[ MAX_SEQUENCES ];        // 0x4CC the blob's sequence of each of the shadow's
};

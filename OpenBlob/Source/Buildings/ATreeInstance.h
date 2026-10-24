#pragma once
#include "Animation/AModelInstance2.h"
#include "Terrain2/TerrainTypes.h"

#include <Math/TVector4.h>

namespace Toshi
{
class TMaterial;
class TParticleEffect;
} // namespace Toshi

class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: a tree of the terrain (cell instance type 3, TerrainTypes::InstanceType_Tree): two models, the dead (grey)
// tree and the tree, swapped by the "tree_down"/"tree_up" animations when Blob paints it (the tree takes his colour,
// ATreeInstance::Paint) or inks it (back to the dead tree). Created by AModel2::CreateInstance (type 3), its terrain
// callbacks are in ATerrain2Interface's per type tables.
//-----------------------------------------------------------------------------
class ATreeInstance
    : public AModelInstance2
{
public:
	static TCONSTEXPR_VAR TINT NUM_EMERGE_EFFECTS = 15;

	// A key of the scale curves of the transform (0x0C bytes)
	struct ScaleKey
	{
		TFLOAT m_fTime;   // 0x00
		TFLOAT m_fScaleX; // 0x04 x and z
		TFLOAT m_fScaleY; // 0x08
	};

public:
	ATreeInstance( AModel2* a_pModel, TBOOL a_bCreateSoundBreakpoints );
	~ATreeInstance();

	// The callbacks of TerrainTypes::InstanceType_Tree in ATerrain2Interface's per type tables
	static void  OnCreated( TerrainTypes::CellInstance* a_pInstance );
	static void  OnBlobTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const Toshi::TVector4* a_pPosition );
	static TBOOL OnTick( TerrainTypes::CellInstance* a_pInstance, TFLOAT a_fDeltaTime, TFLOAT a_fTime );
	static void  OnReset( TerrainTypes::CellInstance* a_pInstance );
	static void  OnPaintRandom( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob );

	// Inks a painted tree: back to the dead tree, the score of the tree is taken back
	static void Ink( TerrainTypes::CellInstance* a_pInstance );

	// The leaves of the tree emerging, one per colour (ABuildingManager::Init)
	static void CreateEffects();
	static void DestroyEffects();

	// Sets the scale of a_pModel from the curve a_pKeys at a_fTime (also used by the cell instances of type 6)
	static void __stdcall SetScaleFromCurve( AModelInstance2* a_pModel, TFLOAT a_fTime, const ScaleKey* a_pKeys, TINT a_iNumKeys );

	// The model's render callback: the glint material of the paint challenge around the render of the models
	static void __stdcall RenderCallback( Toshi::TModelInstance* a_pModelInstance, void* a_pUserData );

	// Paints the tree in a_iColour (a_pBlob painted it, the score popup at a_pPosition)
	void Paint( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const Toshi::TVector4* a_pPosition, TINT a_iColour );

	// The paint challenge's glint materials of the models (copies of their materials with a projection pass)
	void CreateGlintMaterials();

	// $deBlob: FUNCTION 00582f00
	AModelInstance2* GetTreeModel() const { return m_pTreeModel; }
	// $deBlob: FUNCTION 00582ee0
	AModelInstance2* GetDeadTreeModel() const { return m_pDeadTreeModel; }
	// The model shown (the tree once transformed)
	AModelInstance2* GetCurrentModel() const { return m_bTransformed ? m_pTreeModel : m_pDeadTreeModel; }

	TBOOL IsTransformed() const { return m_bTransformed; }
	// $deBlob: FUNCTION 00582f30
	TBOOL IsTransforming() const { return m_bTransforming; }
	// Painted, or being painted (the transform swaps the state)
	TBOOL IsPainted() const { return m_bTransformed != m_bTransforming; }
	// $deBlob: FUNCTION 0059f010
	TINT GetColour() const { return m_iColour; }

private:
	AModelInstance2* CreateModel( const TCHAR* a_szName, TerrainTypes::CellInstance* a_pInstance );

	// Updates the transform, TRUE once it's over
	TBOOL UpdateTransform( TFLOAT a_fDeltaTime );

	// The colour of the tree on the tree model
	void ApplyColour();

public:
	static Toshi::TParticleEffect* sm_pTreeEmergeEffects[ NUM_EMERGE_EFFECTS ]; // [ colour - 1 ]

public:
	TINT16            m_iGlintIndex;           // 0x74 the paint challenge projection (TWorldShaderHAL::ProjTexture), -1: none
	AModelInstance2*  m_pDeadTreeModel;        // 0x78 CellInstance::m_pTypeData[ 0 ]
	AModelInstance2*  m_pTreeModel;            // 0x7C CellInstance::m_pTypeData[ 1 ]
	TINT              m_iTreeUpId;             // 0x80 "tree_up" of m_pTreeModel
	TINT              m_iTreeDownId;           // 0x84 "tree_down" of m_pDeadTreeModel
	TINT              m_iColour;               // 0x88 ABlobColourComponent::COLOUR
	TBOOL             m_bTransformed;          // 0x8C painted
	TBOOL             m_bTransforming;         // 0x8D
	Toshi::TMaterial* m_apGlintMaterials[ 2 ]; // 0x90 of m_pTreeModel's mesh 1 and m_pDeadTreeModel's mesh 0
};

TSTATICASSERT( sizeof( ATreeInstance ) == 0x98 );

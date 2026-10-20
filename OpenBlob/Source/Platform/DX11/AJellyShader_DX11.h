#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Math/TMatrix44.h>
#include <Render/TOrderTable.h>
#include <Platform/DX11/TWorldShader_DX11.h>

namespace Toshi
{
class TModelInstance;
class TMaterial;
struct TModelLOD;
} // namespace Toshi

namespace TerrainTypes
{
struct CellInstance;
struct InstanceMeshes;
} // namespace TerrainTypes

class AModelInstance2;
class ACamera;

// The size of a building's pattern (the "PatternSettings" symbol of the level's TRB, 0x10 bytes)
struct PatternSetting
{
	TUINT32      m_uiHash; // 0x00 of m_szName
	const TCHAR* m_szName; // 0x04 the model
	TFLOAT       m_fSizeX; // 0x08
	TFLOAT       m_fSizeY; // 0x0C
};

struct PatternSettings
{
	TUINT32         m_uiNumSettings; // 0x00
	PatternSetting* m_pSettings;     // 0x04
};

// The "PatternSettings" symbol of the level's first data block
PatternSettings* GetPatternSettings();

// The setting of a_szModel (the first one if none)
const PatternSetting* FindPatternSetting( const PatternSettings* a_pSettings, const TCHAR* a_szModel );

//-----------------------------------------------------------------------------
// Purpose: the "jelly" paint effect of the buildings (AJellyShader_DX11.h, created by AAppInitState): when a building
// is painted one of the four building configs takes it, an animated "solid" (a drip, a fast drip, a multiplayer drip or
// a cone model) grows over it while its world materials are swapped for jelly ones that project the new colour (the
// projected texture data of TWorldShaderHAL), and once the animation is over the building gets the new colour as its
// local ambient colour and its materials back.
//-----------------------------------------------------------------------------
class AJellyShader_DX11
    : public Toshi::TSingleton<AJellyShader_DX11>
{
public:
	static TCONSTEXPR_VAR TINT NUM_BUILDING_CONFIGS = 4;
	static TCONSTEXPR_VAR TINT NUM_SOLIDS           = 5;
	static TCONSTEXPR_VAR TINT MAX_ORIGINAL_MATS    = 16;
	static TCONSTEXPR_VAR TINT NUM_PASSES           = 5;
	static TCONSTEXPR_VAR TINT MAX_MATERIALS        = 448;
	static TCONSTEXPR_VAR TINT NUM_ORDER_TABLES     = 5;
	static TCONSTEXPR_VAR TINT NUM_ORDER_TABLES2    = 3;
	static TCONSTEXPR_VAR TINT NUM_SHADER_PASSES    = 7; // the passes of the jelly materials (GetOrderTable's NUM_PASSES)
	static TCONSTEXPR_VAR TINT NUM_SOLID_PASSES     = 2;
	static TCONSTEXPR_VAR TINT NUM_SPLAT_TEXTURES   = 4;
	static TCONSTEXPR_VAR TINT BLENDMODE_JELLY      = 18;

	// The solids (and paint modes): SetPaintColour passes 0 (single player), 2 (multiplayer) or 4 (flag 0x100000 of
	// the cell instance); 1 and 3 come from the other callers
	enum Solid
	{
		Solid_Drip,
		Solid_Unk1,
		Solid_MPDrip,
		Solid_Cone,
		Solid_FastDrip,
	};

	// A jelly material swapped in for a mesh (0x10 bytes)
	struct MatAlloc
	    : public Toshi::T2DList<MatAlloc>::Node
	{
		Toshi::TMaterial* m_pMat;         // 0x08
		Toshi::TMaterial* m_pOriginalMat; // 0x0C
	};

	// A building being painted (0xF4 bytes)
	struct BuildingConfig
	{
		// NOTE: explicitly defaulted to carry the markers (m_oBuildingConfigs' eh vector constructor/destructor in
		// AJellyShader_DX11's ctor/dtor: they only construct/destroy m_aMaterials)
		// $deBlob: FUNCTION 0055d8c0
		BuildingConfig() = default;
		// $deBlob: FUNCTION 0055d9e0
		~BuildingConfig() = default;

		Toshi::TModelInstance*   m_apSolids[ NUM_SOLIDS ];             // 0x00
		Toshi::TMaterial*        m_aOriginalMats[ MAX_ORIGINAL_MATS ]; // 0x14
		Toshi::T2DList<MatAlloc> m_aMaterials[ NUM_PASSES ];           // 0x54
		AModelInstance2*         m_pInstance;                          // 0x7C
		TUINT32                  m_uiOldColour;                        // 0x80
		TUINT32                  m_uiNewColour;                        // 0x84
		TUINT32                  m_uiUnk88;                            // 0x88
		TBOOL                    m_bIsReset;                           // 0x8C
		TINT                     m_eSolid;                             // 0x90 Solid
		Toshi::TMatrix44         m_mProjection;                        // 0x94
		TINT                     m_iCurSplatTex;                       // 0xD4 the frame of the splat animation
		TBOOL                    m_bSplatTextures;                     // 0xD8 the splat textures are projected (else m_apProjTextures)
		TINT                     m_iPattern;                           // 0xDC the pattern of the painter (APlayerManager), -1: none
		TFLOAT                   m_fSplatTime;                         // 0xE0
		TINT16                   m_uiProjTexDataIndex;                 // 0xE4
		TINT16                   m_uiTexScaleDataIndex;                // 0xE6
		TUINT8                   m_aUnkE8[ 4 ];                        // 0xE8
		TFLOAT                   m_fSpeed;                             // 0xEC
		TUINT8                   m_uiUnkF0;                            // 0xF0 the cameras that rendered it (multiplayer)
		TBOOL                    m_bUnkF1;                             // 0xF1 cleared each frame (ARenderer::Update)
		TUINT8                   m_aUnkF2[ 2 ];                        // 0xF2

		// Shows/hides the building: the cell instance's visible flag (flag 0x10 or a grass instance), else the model
		// instance's FLAG_Visible
		void SetVisible( TBOOL a_bVisible );
	};

public:
	AJellyShader_DX11();
	~AJellyShader_DX11();

	// Paints a_pInstance from a_uiOldColour to a_uiNewColour (ABuildingInstance::SetPaintColour)
	void StartPaint( AModelInstance2* a_pInstance, TUINT32 a_uiOldColour, TUINT32 a_uiNewColour, TINT a_eSolid, TINT a_iPlayer, TFLOAT a_fSpeed );

	// Clears the per frame flag of the configs (ARenderer::Update)
	void Update();

	// Finishes the configs whose animation is over and resets the counters (the end of a level)
	void Reset();

	// Is a_pInstance being painted?
	TBOOL IsPainting( AModelInstance2* a_pInstance ) const;

	// Finishes every config at once
	void FinishAll();

	// Finishes the config of the cell instance a_pInstance (its cell is unloaded)
	void OnInstanceRemoved( TerrainTypes::CellInstance* a_pInstance );

	// $deBlob: FUNCTION 00578400
	TINT16 AllocTexScale()
	{
		TASSERT( m_iCurTexScale < Toshi::TWorldShaderHAL::MAX_TEX_SCALES );
		return TINT16( m_iCurTexScale++ );
	}

	// $deBlob: FUNCTION 0055dde0
	Toshi::TOrderTable* GetOrderTable( TINT a_iPass )
	{
		TASSERT( 0 <= a_iPass && a_iPass < NUM_SHADER_PASSES );
		TASSERT( 0 <= s_ciIndices[ a_iPass ] && s_ciIndices[ a_iPass ] < scm_NumJellyOrderTables );
		return &m_aOrderTables[ s_ciIndices[ a_iPass ] ];
	}

	// $deBlob: FUNCTION 0055e0b0
	Toshi::TOrderTable* GetWindowOrderTable( TINT a_iPass )
	{
		TASSERT( 0 <= a_iPass && a_iPass < NUM_SHADER_PASSES );
		TASSERT( 0 <= s_ciWindowIndices[ a_iPass ] && s_ciWindowIndices[ a_iPass ] < scm_NumWindowOrderTables );
		return &m_aOrderTables2[ s_ciWindowIndices[ a_iPass ] ];
	}

	// Creates the jelly materials and loads the solids (ALoadLevelState::LoadGUI)
	TBOOL LoadSolids();

	// Destroys what LoadSolids created (AInGameState::OnRemoval)
	TBOOL UnloadSolids();

	// Swaps the materials of the buildings being painted for jelly ones (ARenderer::MainScene)
	void PrepareConfigs( TFLOAT a_fDeltaTime );

	// Renders the buildings being painted and their solids (ARenderer::MainScene)
	void RenderConfigs( TFLOAT a_fDeltaTime, ACamera* a_pCamera );

private:
	// Gives the building of a_rConfig its materials back (a_bRestoreTextures: the projected texture stages too)
	void ResetConfig( BuildingConfig& a_rConfig, TBOOL a_bRestoreTextures, TBOOL a_bKeepMaterials );

	// The building of a_rConfig gets its new colour and its materials back, the solid stops
	void FinishConfig( BuildingConfig& a_rConfig, TBOOL a_bRestoreTextures, TBOOL a_bKeepMaterials );

	// TWorldMaterialHAL callbacks: blend mode 18 makes a material a jelly one (its order table by pass), PreRender
	// sets the render state of each jelly pass
	static TBOOL __stdcall SetBlendModeCallback( Toshi::TWorldMaterialHAL* a_pMaterial, Toshi::TWorldMaterial::BLENDMODE a_eBlendMode );
	static TBOOL __stdcall PreRenderCallback( Toshi::TWorldMaterialHAL* a_pMaterial );
	static TBOOL __stdcall InstanceCallback( Toshi::TWorldMaterialHAL* a_pMaterial );

	// Copies the textures and pass settings of a_pSource to a_pDest (a jelly material of the building)
	static void __stdcall CopyMaterial( Toshi::TMaterial* a_pSource, Toshi::TMaterial* a_pDest );

	// The meshes of the solid's LOD a_pLOD get the jelly skin materials of pass a_iPass of a_pMaterials
	static void __stdcall SetSolidMaterials( Toshi::TModelLOD* a_pLOD, TINT a_iPass, Toshi::T2DList<MatAlloc>* a_pMaterials );

	// The meshes of a building get the jelly materials of pass a_iPass of a_pMaterials
	static void __stdcall SetInstanceMaterials( TerrainTypes::InstanceMeshes* a_pMeshes, TINT a_iPass, Toshi::T2DList<MatAlloc>* a_pMaterials );

	// Renders a building (a_szMaterialFilter: only its meshes of this material)
	static void __stdcall RenderBuilding( AModelInstance2* a_pInstance, const TCHAR* a_szMaterialFilter );

	// TRUE if the LOD 0 bounding sphere of a_pModelInstance is outside the frustum of a_pCamera,
	// a_pOutDistanceSq receives its squared distance to the camera
	static TBOOL __stdcall IsSolidCulled( Toshi::TModelInstance* a_pModelInstance, ACamera* a_pCamera, TFLOAT* a_pOutDistanceSq );

	// The order tables of the jelly / window passes
	static TCONSTEXPR_VAR TINT scm_NumJellyOrderTables  = NUM_ORDER_TABLES;
	static TCONSTEXPR_VAR TINT scm_NumWindowOrderTables = NUM_ORDER_TABLES2;
	static const TINT          s_ciIndices[ NUM_SHADER_PASSES ];
	static const TINT          s_ciWindowIndices[ NUM_SHADER_PASSES ];

private:
	BuildingConfig           m_oBuildingConfigs[ NUM_BUILDING_CONFIGS ];          // 0x0000
	TINT                     m_iCurConfig;                                        // 0x03D0 the config the next paint takes
	MatAlloc                 m_oMatAlloc[ MAX_MATERIALS ];                        // 0x03D4
	Toshi::T2DList<MatAlloc> m_oMatFreeList;                                      // 0x1FD4
	Toshi::T2DList<MatAlloc> m_aSolidMaterials[ NUM_SOLIDS ][ NUM_SOLID_PASSES ]; // 0x1FDC the jelly skin materials of the solids' meshes
	Toshi::TMaterial*        m_apSolidMeshMaterials[ NUM_SOLIDS ];                // 0x202C the own materials of the solids' meshes
	Toshi::TTexture*         m_apProjTextures[ NUM_SPLAT_TEXTURES ];              // 0x2040 (never set)
	Toshi::TTexture*         m_apSplatTextures[ NUM_SPLAT_TEXTURES ];             // 0x2050 "splatlong_frame1-4.tga"
	Toshi::TOrderTable       m_aOrderTables[ NUM_ORDER_TABLES ];                  // 0x2060
	Toshi::TOrderTable       m_aOrderTables2[ NUM_ORDER_TABLES2 ];                // 0x2128
	TINT                     m_iCurTexScale;                                      // 0x21A0
	TINT                     m_iNumCones;                                         // 0x21A4 the configs using Solid_Cone
};                                                                                // 0x21A8

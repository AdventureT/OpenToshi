#pragma once
#include "Shaders/WorldShader/TWorldShader.h"
#include "TShaderCache_DX11.h"
#include "Math/TVector2.h"
#include "Math/TVector4.h"
#include "Math/TMatrix44.h"

#include <d3d11.h>

class ACellBucket;
class AMaterialInfo;
class AJellyShader_DX11;

TOSHI_NAMESPACE_START

class TOrderTable;

// Unpacks an ARGB colour into ( r, g, b, a ) in [0;1] (TWorldShader_DX11.cpp, also used by the game's instance lighting)
void UnpackColour( TUINT32 a_uiColour, TVector4& a_rOutColour );

// The render state of the world shader (TWorldShader_DX11.cpp) the jelly shader's PreRender callback sets
extern TBOOL    s_bWaterFrameReset2;  // the reflection texture is made once per frame (ResetWaterFrame sets it; ABlobEntity's shockwave clears it)
extern TVector4 s_vAlphaTest;         // x: 1.0 to test, y: the reference (PS c0)
extern TVector4 s_vFogStartEnd;       // x: start, y: end, z: 1.0 when fogged (VS c36)
extern TINT     s_iLighting;          // index into s_auiLightingFlags
extern TINT     s_iIndirect;          // index into s_auiIndirectFlags
extern TINT     s_aiTexCoordGen[ 5 ]; // the TexCoordGen function of each pass
class TTexture;
class TWorldMeshHAL;
class TWorldMaterialHAL;

//-----------------------------------------------------------------------------
// Purpose: a mesh of a world TRB ("twld" cell meshes), the data TWorldMeshHAL makes its buffers of.
// Only the members the engine reads; m_pMesh is set by TModelLoaderWorld_DX11.
//-----------------------------------------------------------------------------
struct TTRBWorldMesh
{
	struct Vertex
	{
		TVector3 Position; // 0x00
		TVector3 Normal;   // 0x0C
		TVector3 Colour;   // 0x18
		TVector2 UV0;      // 0x24
		TVector2 UV1;      // 0x2C
	}; // 0x34

	// The bits of m_uiFlags: an instanced mesh is queued once per frame in the opaque order table and once in the instance order tables
	enum Flags : TUINT32
	{
		Flags_QueuedOpaque   = 2,
		Flags_QueuedInstance = 4,
	};

	// Draws a_uiCount vertices of a_pVertices, or a_uiCount indices of a_pIndices from the mesh's vertex buffer, or
	// else the whole mesh
	void RenderMesh( const void* a_pVertices, const void* a_pIndices, TUINT16 a_uiCount );

	TWorldMeshHAL* m_pMesh;          // 0x00
	TUINT32        m_uiIndexCount;   // 0x04
	TUINT32        m_uiVertexCount;  // 0x08
	TUINT32        m_uiUnk0C;        // 0x0C
	const TCHAR*   m_szMaterialName; // 0x10
	Vertex*        m_pVertices;      // 0x14
	TUINT16*       m_pIndices;       // 0x18
	TUINT32        m_uiFlags;        // 0x1C Flags
	TUINT32        m_uiUnk20;        // 0x20
}; // 0x24 (TWorldShaderHAL::Render copies it)

//-----------------------------------------------------------------------------
// Purpose: one instance of a world mesh added by TWorldMeshHAL::AddInstance (a pool of TWorldShaderHAL)
//-----------------------------------------------------------------------------
struct TWorldMeshInstanceData
{
	// What the instance is drawn with (the game's)
	struct Source
	{
		const void*        m_pVertices; // 0x00 drawn instead of the mesh when set (m_uiCount of them)
		const TCHAR*       m_szName;    // 0x04 the material name of the mesh (the TTRBWorldMesh's pointer once matched)
		const TCHAR*       m_szLightmap; // 0x08 the light map texture (TNULL: none)
		const void*        m_pIndices;  // 0x0C else drawn with the mesh's vertices when set
		TUINT16            m_uiCount;   // 0x10
		TUINT16            m_uiUnk12;   // 0x12
		TUINT32            m_uiMeshId;  // 0x14 the TTRBWorldMesh's m_uiUnk20
		TWorldMaterialHAL* m_pMaterial; // 0x18
	};

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_OPT_RENDERLIST_1 = BITFLAG( 2 ),
		FLAGS_OPT_RENDERLIST_2 = BITFLAG( 3 ),
	};

	TMatrix44               m_oModelView; // 0x00 the render context's model view matrix
	struct Source*          m_pSource;    // 0x40
	TVector4                m_vAmbient;   // 0x44 the render context's ambient colour
	TWorldMeshInstanceData* m_prev;       // 0x54
	TWorldMeshInstanceData* m_next;       // 0x58
	TUINT32                 m_uiFlags;    // 0x5C
}; // 0x60

//-----------------------------------------------------------------------------
// Purpose: DX11 part of TWorldMaterial: the per pass UV offsets/modes/streams and the multi pass blend types the world
// pixel shader permutation is built from (PreRender).
//-----------------------------------------------------------------------------
class TWorldMaterialHAL : public TWorldMaterial
{
public:
	TDECLARE_CLASS( TWorldMaterialHAL, TWorldMaterial );

public:
	static TCONSTEXPR_VAR TINT s_iMaxPasses = 5;

	enum UData
	{
		UData_0,
		UData_1,
		UData_2,
		UData_NUMOF
	};

	// How the passes are combined: the pixel shader pass functions (s_aszPassFunctions, the HLSL's Pass* functions)
	// in this order (the T2Material passes' Unk08)
	using MULTIPASSBLENDTYPE = TINT32;
	enum MULTIPASSBLENDTYPE_ : MULTIPASSBLENDTYPE
	{
		MULTIPASSBLENDTYPE_OVERRIDE          = 0,
		MULTIPASSBLENDTYPE_MODULATE          = 1,
		MULTIPASSBLENDTYPE_ADD               = 2,
		MULTIPASSBLENDTYPE_SUBTRACTIVE       = 3,
		MULTIPASSBLENDTYPE_BLEND             = 4,
		MULTIPASSBLENDTYPE_ADDHALF           = 5,
		MULTIPASSBLENDTYPE_SHADOW            = 6,
		MULTIPASSBLENDTYPE_PROJECTEDTEX      = 7,
		MULTIPASSBLENDTYPE_WINDOWS           = 8,
		MULTIPASSBLENDTYPE_WINDOWS2          = 9,
		MULTIPASSBLENDTYPE_ADDWRITEALPHA     = 10,
		MULTIPASSBLENDTYPE_WRITEALPHA        = 11,
		MULTIPASSBLENDTYPE_ADDALPHA          = 12,
		MULTIPASSBLENDTYPE_SUBTRACTIVEALPHA  = 13,
		MULTIPASSBLENDTYPE_MODULATEALPHA     = 14,
		MULTIPASSBLENDTYPE_TEXALPHABLEND     = 15,
		MULTIPASSBLENDTYPE_OVERRIDEZEROALPHA = 16,
		MULTIPASSBLENDTYPE_OVERRIDECHAN2     = 17,
		MULTIPASSBLENDTYPE_LERPCOLOUR        = 18,
		MULTIPASSBLENDTYPE_LERPCOLOUR2       = 19,
		MULTIPASSBLENDTYPE_WINDOWSPROJ       = 20,
		MULTIPASSBLENDTYPE_BLENDWRITEALPHA   = 21,
		MULTIPASSBLENDTYPE_LERPCONST         = 22,
		MULTIPASSBLENDTYPE_LERPTEX           = 23,
		MULTIPASSBLENDTYPE_LERPTEXLM1        = 24,
		MULTIPASSBLENDTYPE_LERPTEXLM2        = 25,
		MULTIPASSBLENDTYPE_ADDALPHALERP      = 26,
		MULTIPASSBLENDTYPE_NUMOF             = 27,
	};

	// The texture coordinates of a pass (picks the vertex shader's TexCoordGen function)
	using UVMODE = TUINT8;
	enum UVMODE_ : UVMODE
	{
		UVMODE_STREAM    = 0, // UV0/UV1 (m_iUVStream)
		UVMODE_OFFSET    = 1, // the stream translated by the pass's UV offset
		UVMODE_ENVMAP    = 2, // from the normals (texMtx7)
		UVMODE_PROJECTED = 4, // from the positions (texMtx6, the UData_0 projected texture), sets TMaterial flag 0x200
		UVMODE_TEXSCALE  = 5, // the stream scaled by the UData_1 texture scale
	};

	friend class TWorldShaderHAL;
	friend class TWorldMeshHAL;
	friend class ::ACellBucket;   // reads the UV streams of the light mapped terrain materials
	friend class ::AMaterialInfo; // swaps the passes of the light mapped terrain materials
	friend class ::AJellyShader_DX11; // copies the jelly materials, its callbacks set the blend mode

	// Called instead of the own logic when set and it returns TTRUE (set by AJellyShader)
	using t_SetBlendModeCallback = TBOOL( __stdcall* )( TWorldMaterialHAL* a_pMaterial, BLENDMODE a_eBlendMode );
	using t_RenderCallback       = TBOOL( __stdcall* )( TWorldMaterialHAL* a_pMaterial );

public:
	TWorldMaterialHAL();
	~TWorldMaterialHAL();

	//-----------------------------------------------------------------------------
	// TMaterial
	//-----------------------------------------------------------------------------
	virtual void OnDestroy() override;  // +0x0C
	virtual void PreRender() override;  // +0x14
	virtual void PostRender() override; // +0x18

	//-----------------------------------------------------------------------------
	// TWorldMaterial
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( BLENDMODE a_eBlendMode ) override;       // +0x1C
	virtual void  SetBlendMode( BLENDMODE a_eBlendMode ) override; // +0x20

	void SetBlendMode( BLENDMODE a_eBlendMode, TUINT8 a_uiPass );

	// Registers the material in a_pOrderTable (deregistering it from the old one) when a_bRegister
	void SetOrderTable( TOrderTable* a_pOrderTable, TINT a_iUnused, TBOOL a_bRegister );
	TOrderTable* GetOrderTable() const { return m_pOrderTable; }

	// NOTE: the mode is passed as an int in retail (compared as a dword), stored as a byte
	void SetUVMode( TINT a_iPass, TINT a_eMode );
	void SetUVOffset( TFLOAT a_fUOffset, TFLOAT a_fVOffset, TINT a_Pass );

	// $deBlob: FUNCTION 00705c30
	void GetMultiPassBlendTypes12( MULTIPASSBLENDTYPE& a_reType1, MULTIPASSBLENDTYPE& a_reType2 ) const
	{
		a_reType1 = m_aMultiPassBlendTypes[ 1 ];
		a_reType2 = m_aMultiPassBlendTypes[ 2 ];
	}

	// $deBlob: FUNCTION 00705e10
	void SetMultiPassBlendTypes12( MULTIPASSBLENDTYPE a_eType1, MULTIPASSBLENDTYPE a_eType2 )
	{
		m_aMultiPassBlendTypes[ 1 ] = a_eType1;
		m_aMultiPassBlendTypes[ 2 ] = a_eType2;
	}

	TINT16 GetUserDataIndex( UData a_eData ) const { return m_iUserDataIndices[ a_eData ]; }
	void   SetUserDataIndex( UData a_eData, TINT16 a_iIndex ) { m_iUserDataIndices[ a_eData ] = a_iIndex; }

	MULTIPASSBLENDTYPE GetMultiPassBlendType( TINT a_iPass ) const { return m_aMultiPassBlendTypes[ a_iPass ]; }
	void               SetMultiPassBlendType( TINT a_iPass, MULTIPASSBLENDTYPE a_eType ) { m_aMultiPassBlendTypes[ a_iPass ] = a_eType; }
	// $deBlob: FUNCTION 0055e040
	UVMODE GetUVMode( TINT a_iPass ) const
	{
		TASSERT( a_iPass < s_iMaxPasses );
		return m_aUVModes[ a_iPass ];
	}

	TUINT8 GetUVStream( TINT a_iPass ) const { return m_iUVStream[ a_iPass ]; }
	void   SetUVStream( TINT a_iPass, TUINT8 a_uiStream ) { m_iUVStream[ a_iPass ] = a_uiStream; }
	TUINT8 GetAlpha() const { return m_uiAlpha; }
	void   SetAlpha( TUINT8 a_uiAlpha ) { m_uiAlpha = a_uiAlpha; }
	// The UV offsets without SetUVOffset's range checks (APropHazard::Tick scrolls the hot plate's noise pass)
	TFLOAT GetUOffset( TINT a_iPass ) const { return m_afUOffset[ a_iPass ]; }
	void   SetUOffset( TINT a_iPass, TFLOAT a_fOffset ) { m_afUOffset[ a_iPass ] = a_fOffset; }
	void   SetVOffset( TINT a_iPass, TFLOAT a_fOffset ) { m_afVOffset[ a_iPass ] = a_fOffset; }
	TUINT8 GetPass() const { return m_uiPass; }

public:
	static t_SetBlendModeCallback s_fnSetBlendModeCallback;
	static t_RenderCallback       s_fnPreRenderCallback;
	static t_RenderCallback       s_fnInstanceCallback; // (no caller in the engine)
	static t_RenderCallback       s_fnPostRenderCallback;

	// PS c2/c7 of MULTIPASSBLENDTYPE_LERPCONST: the glow colour and the fade of a completed landmark's paint pass
	// (ALandmarkInstance::UpdateImp/AddPaintPass)
	static TVector4 s_vMultiPassColour;
	static TVector4 s_vLerpConstColour;

private:
	TFLOAT             m_afUOffset[ s_iMaxPasses ];               // 0x64
	TFLOAT             m_afVOffset[ s_iMaxPasses ];               // 0x78
	TOrderTable*       m_pOrderTable;                             // 0x8C
	MULTIPASSBLENDTYPE m_aMultiPassBlendTypes[ s_iMaxPasses ];    // 0x90
	TINT16             m_iUserDataIndices[ UData_NUMOF ];         // 0xA4 indices into TWorldShaderHAL's tables
	TUINT8             m_uiAlpha;                                 // 0xAA
	TUINT8             m_iUVStream[ s_iMaxPasses ];               // 0xAB
	TUINT8             m_uiPass;                                  // 0xB0
	UVMODE             m_aUVModes[ s_iMaxPasses ];                // 0xB1
	TUINT8             m_aPadB6[ 2 ];                             // 0xB6
}; // 0xB8

//-----------------------------------------------------------------------------
// Purpose: DX11 part of TWorldMesh: the vertex/index buffers of a TTRBWorldMesh and its instance list.
//-----------------------------------------------------------------------------
class TWorldMeshHAL : public TWorldMesh
{
public:
	TDECLARE_CLASS( TWorldMeshHAL, TWorldMesh );

public:
	friend class TWorldShaderHAL;

	TWorldMeshHAL();
	~TWorldMeshHAL();

	//-----------------------------------------------------------------------------
	// TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() override;    // +0x18
	virtual void  OnDestroy() override; // +0x1C

	//-----------------------------------------------------------------------------
	// TWorldMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL RenderWithCallback( t_RenderCallback a_fnCallback, void* a_pUserData ) override; // +0x20

	// Creates the immutable index and vertex buffers of a_pTRBMesh
	TBOOL Create( TTRBWorldMesh* a_pTRBMesh );

	// Adds the render packets of the mesh (a_iOrderTableIndex -1: the material's order table)
	TBOOL RenderWithCallback( t_RenderCallback a_fnCallback, void* a_pUserData, TINT16 a_iOrderTableIndex );

	// Adds an instance (the current model view matrix) to the mesh's instance list
	void AddInstance( TWorldMeshInstanceData::Source* a_pSource, TUINT32 a_uiFlags );

	TTRBWorldMesh* GetTRBMesh() const { return m_pTRBMesh; }
	ID3D11Buffer*  GetIndexBuffer() const { return m_pIndexBuffer; }
	ID3D11Buffer*  GetVertexBuffer() const { return m_pVertexBuffer; }

private:
	TTRBWorldMesh*          m_pTRBMesh;      // 0x14
	TWorldMeshInstanceData* m_prev;          // 0x18
	TWorldMeshInstanceData* m_next;          // 0x1C
	TWorldMeshInstanceData* m_current;       // 0x20
	ID3D11Buffer*           m_pIndexBuffer;  // 0x24
	ID3D11Buffer*           m_pVertexBuffer; // 0x28
}; // 0x2C

//-----------------------------------------------------------------------------
// Purpose: the world shader (DX11). Owns the 12 order tables of the world, the shader permutations (a TShaderCache_DX11
// of Data\Shaders\World\, compiled from the HLSL below), the instance pool and the tables the game fills (projected
// textures, texture scales, colours, transitions, crossfades). Created by ARenderer::Create followed by Create.
//-----------------------------------------------------------------------------
class TWorldShaderHAL : public TWorldShader
{
public:
	// The instance order tables are used (TWorldMeshHAL::RenderWithCallback, RenderLightmappedInstance, AJellyShader)
	static TBOOL s_bInstancing;

public:
	TDECLARE_CLASS( TWorldShaderHAL, TWorldShader );

public:
	static TCONSTEXPR_VAR TINT NUM_ORDERTABLES         = 12;
	static TCONSTEXPR_VAR TINT MAX_PROJ_TEXTURES_ALLOC = 91;
	static TCONSTEXPR_VAR TINT MAX_TEX_SCALES          = 5000;
	static TCONSTEXPR_VAR TINT MAX_COLOURS             = 16;
	static TCONSTEXPR_VAR TINT MAX_TRANSITIONS         = 4;
	static TCONSTEXPR_VAR TINT MAX_CROSSFADES          = 16;

	enum OTABLE
	{
		OTABLE_ORDERING_OPT_OPAQUE = 11,
	};

	struct ProjTexture
	{
		TMatrix44 m_oMatrix1;    // 0x00 texMtx6 = m_oMatrix1 * m_oMatrix2
		TMatrix44 m_oMatrix2;    // 0x40
		TUINT32   m_uiColour;    // 0x80 ARGB
		TFLOAT    m_fMin;        // 0x84 the bottom of the projection (AGameStatus::SetInstanceProjection)
		TFLOAT    m_fMax;        // 0x88 its top
		TFLOAT    m_fCurrent;    // 0x8C the glint (AGameStatus::UpdateProjectionGlint)
		TFLOAT    m_fHeight;     // 0x90 of the instance's bounds
	}; // 0x94

	struct TexScale
	{
		TFLOAT m_fScaleU; // 0x00
		TFLOAT m_fScaleV; // 0x04
	}; // 0x08

	struct Colour
	{
		TUINT32 m_uiColour1; // 0x00 ARGB
		TUINT32 m_uiColour2; // 0x04 ARGB
	}; // 0x08

	// A water/ink transition of an APropWater (its materials' UData_0 is the slot)
	struct TransitionRef
	{
		TUINT32            m_uiOwner;    // 0x00 the APropWater's trigger caller, -1 free
		TFLOAT             m_fProgress;  // 0x04
		TWorldMaterialHAL* m_pWaterMat;  // 0x08 "water_trans_mat" (drawn over the ink, blend mode 10)
		TWorldMaterialHAL* m_pInkMat;    // 0x0C "ink_trans_mat" (blend mode 8)
		TINT               m_iNumUsers;  // 0x10 the APropWaters that use the slot
	}; // 0x14

	// Called by Render before the own logic, which is skipped when it returns TTRUE (set by AJellyShader)
	using t_RenderCallback = TBOOL( __stdcall* )( TRenderPacket* a_pPacket );

	friend TWorldMeshHAL;
	friend TWorldMaterialHAL;

public:
	TWorldShaderHAL();
	~TWorldShaderHAL();

	//-----------------------------------------------------------------------------
	// TShader
	//-----------------------------------------------------------------------------
	virtual void  OnDestroy() override;                         // +0x0C
	// $deBlob: FUNCTION 00701830
	virtual void  Flush() override {}                           // +0x10
	virtual void  StartFlush() override;                        // +0x14
	virtual void  EndFlush() override;                          // +0x18
	virtual TBOOL Create() override;                            // +0x1C
	virtual void  Render( TRenderPacket* a_pRenderPacket ) override; // +0x34

	//-----------------------------------------------------------------------------
	// TWorldShader
	//-----------------------------------------------------------------------------
	virtual TWorldMaterial* CreateMaterial( const TCHAR* a_szName ) override; // +0x58
	virtual TWorldMesh*     CreateMesh( const TCHAR* a_szName ) override;     // +0x5C

	// Renders a TRB world mesh with a_pMaterial (builds the shader permutation, sets the constants)
	void Render( TTRBWorldMesh* a_pMesh, TWorldMaterialHAL* a_pMaterial, const TMatrix44* a_pModelView, TBOOL a_bFirst, TTRBWorldMesh* a_pOrigMesh, TWorldMaterialHAL* a_pInstanceMaterial, const void* a_pVertices, const void* a_pIndices, TUINT16 a_uiCount );

	void InitWaterTexture();
	void DeinitWaterTexture();
	// Resolves the main render target into the reflection texture (created on the first call)
	void MakeReflectTexture();
	void BindReflectTexture( UINT a_uiSlot );
	void BindNoiseTexture( UINT a_uiSlot );
	// Scrolls the two water UV offsets (called by ARenderer each frame)
	void UpdateWater( TFLOAT a_fDeltaTime );
	void SetMemWater( void* a_pMem );

	static void ResetWaterFrame();
	static void SetMaxInstances( TUINT a_uiMaxInstances );

	TOrderTable*       GetOrderTable( TINT a_iIndex ) { return &m_aOrderTables[ a_iIndex ]; }
	// $deBlob: FUNCTION 006bba10
	TINT               GetNumOrderTables() const { return m_iNumOrderTables; }
	TWorldMaterialHAL* GetInstanceRenderMat( TINT a_iIndex ) const { return m_pInstanceRenderMats[ a_iIndex ]; }
	// $deBlob: FUNCTION 00705d80
	TTexture*          GetWaterTransitionTex() const { return m_pWaterTransitionTex; }
	TShaderCache_DX11& GetShaderCache() { return m_oShaderCache; }

	// $deBlob: FUNCTION 0055df60
	ProjTexture& GetProjTexture( TINT a_iIndex )
	{
		TASSERT( a_iIndex < MAX_PROJ_TEXTURES_ALLOC );
		return m_aProjTextures[ a_iIndex ];
	}

	// $deBlob: FUNCTION 00705cc0
	TexScale* GetTexScale( TINT a_iIndex )
	{
		TASSERT( a_iIndex < MAX_TEX_SCALES );
		return &m_aTexScales[ a_iIndex ];
	}

	// $deBlob: FUNCTION 0055e3a0
	void SetTexScale( TINT a_iIndex, TFLOAT a_fScaleU, TFLOAT a_fScaleV )
	{
		TASSERT( a_iIndex < MAX_TEX_SCALES );
		m_aTexScales[ a_iIndex ].m_fScaleU = a_fScaleU;
		m_aTexScales[ a_iIndex ].m_fScaleV = a_fScaleV;
	}

	// $deBlob: FUNCTION 005582e0
	void SetColour( TINT a_iIndex, TUINT32 a_uiColour1, TUINT32 a_uiColour2 )
	{
		TASSERT( a_iIndex < MAX_COLOURS );
		m_aColours[ a_iIndex ].m_uiColour1 = a_uiColour1;
		m_aColours[ a_iIndex ].m_uiColour2 = a_uiColour2;
	}

	// $deBlob: FUNCTION 00705ac0
	Colour* GetColour( TINT a_iIndex )
	{
		TASSERT( a_iIndex < MAX_COLOURS );
		return &m_aColours[ a_iIndex ];
	}

	// $deBlob: FUNCTION 00705d70
	TWorldMaterialHAL* GetNoMatWater() const { return m_pNoMatWater; }
	// $deBlob: FUNCTION 00705d20
	// $deBlob: FUNCTION 007a1790
	TransitionRef& GetTransitionRef( TINT a_iIndex )
	{
		TASSERT( a_iIndex < MAX_TRANSITIONS );
		return m_aTransitions[ a_iIndex ];
	}

	// $deBlob: FUNCTION 00705c60
	TUINT32 GetCrossfade( TINT a_iIndex )
	{
		TASSERT( a_iIndex < MAX_CROSSFADES );
		return m_auiCrossfades[ a_iIndex ];
	}

	// NOTE: retail returns a_iIndex or a_uiColour depending on a once-set flag, no caller reads it
	// $deBlob: FUNCTION 00600a20
	void SetCrossfade( TINT a_iIndex, TUINT32 a_uiColour )
	{
		TASSERT( a_iIndex < MAX_CROSSFADES );
		m_auiCrossfades[ a_iIndex ] = a_uiColour;
	}

private:
	// TShaderCache_DX11 callbacks: compile the world VS/PS permutation of an id, create the VS's input layout
	static ID3DBlob* __stdcall          CompileVertexShader( const VertexShaderId& a_rId );
	static ID3D11InputLayout* __stdcall CreateInputLayout( const VertexShaderId& a_rId, ID3DBlob* a_pShaderBlob );
	static ID3DBlob* __stdcall          CompilePixelShader( const PixelShaderId& a_rId );

public:
	static t_RenderCallback s_fnRenderCallback;
	static TUINT            s_uiMaxInstances;
	static TUINT            s_iMaxOfCurNumInstances;

	// The lighting colour scales of the LERPCOLOUR2/WINDOWS passes (PS c5, c6), set by AEnvironmentFX
	static TVector4 s_vLightingScale5;
	static TVector4 s_vLightingScale6;

private:
	TUINT32                 m_uiUnk20;                                    // 0x20
	TOrderTable             m_aOrderTables[ NUM_ORDERTABLES ];            // 0x24
	void*                   m_pUnk204;                                    // 0x204 cleared by Create
	TWorldMaterialHAL*      m_pNoMat;                                     // 0x208 "no_mat"
	TWorldMaterialHAL*      m_pNoMatWater;                                // 0x20C "no_mat_water"
	TWorldMaterialHAL*      m_pNoMatHeat;                                 // 0x210 "no_mat_heat"
	TINT                    m_iNumOrderTables;                            // 0x214
	TVector2                m_vWaterUVOffset1;                            // 0x218
	TVector2                m_vWaterUVOffset2;                            // 0x220
	TTexture*               m_pWaterNoiseTex;                             // 0x228 "water_noise.tga"
	TTexture*               m_pWaterTransitionTex;                        // 0x22C "watertransition.tga"
	void*                   m_pMemWater;                                  // 0x230
	ID3D11Texture2D*        m_pReflectTexture;                            // 0x234
	ID3D11ShaderResourceView* m_pReflectSRV;                              // 0x238
	ProjTexture             m_aProjTextures[ MAX_PROJ_TEXTURES_ALLOC ];   // 0x23C
	TexScale                m_aTexScales[ MAX_TEX_SCALES ];               // 0x36D8
	Colour                  m_aColours[ MAX_COLOURS ];                    // 0xD318
	TransitionRef           m_aTransitions[ MAX_TRANSITIONS ];            // 0xD398
	TUINT32                 m_auiCrossfades[ MAX_CROSSFADES ];            // 0xD3E8 ARGB colours
	TUINT                   m_uiMaxInstances;                             // 0xD428
	TUINT                   m_uiNumInstances;                             // 0xD42C
	TWorldMeshInstanceData* m_pInstanceData;                              // 0xD430
	TWorldMaterialHAL*      m_pInstanceRenderMats[ NUM_ORDERTABLES ];     // 0xD434 "inst_mat_%d", one per order table
	TShaderCache_DX11       m_oShaderCache;                               // 0xD464
}; // 0xD480

TOSHI_NAMESPACE_END

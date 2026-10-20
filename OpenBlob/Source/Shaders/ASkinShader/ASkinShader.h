#pragma once
#include <Render/TShader.h>
#include <Render/TMesh.h>
#include <Render/TMaterial.h>
#include <Math/TVector3.h>
#include <Math/TVector4.h>
#include <Toshi/TSingleton.h>

#include <d3d11.h>

//-----------------------------------------------------------------------------
// Purpose: the game's skin shader (ASkinShader.h): characters and props with skeletons. The DX11 half is ASkinShaderHAL
// (ASkinShader_DX11.cpp). Its methods are the same as the engine's TSkinShader, which de Blob doesn't use.
//-----------------------------------------------------------------------------
class ASkinShader
    : public Toshi::TShader
    , public Toshi::TSingleton<ASkinShader>
{
public:
	TDECLARE_CLASS( ASkinShader, Toshi::TShader );

public:
	ASkinShader();
	~ASkinShader();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual const TCHAR* GetShaderPrefix() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 005672c0
	virtual void EnableRenderEnvMap( TBOOL a_bEnable ) {} // +0x38
	// $deBlob: FUNCTION 005676f0
	virtual TBOOL IsHighEndSkinning() { return TTRUE; } // +0x3C
	// $deBlob: FUNCTION 005672b0
	virtual void EnableHighEndSkinning( TBOOL a_bEnable ) {} // +0x40
	// $deBlob: FUNCTION 005676c0
	virtual TBOOL IsCapableHighEndSkinning() { return TTRUE; } // +0x44
	// $deBlob: FUNCTION 00567700
	virtual TBOOL IsLightScattering() { return TFALSE; } // +0x48
	// $deBlob: FUNCTION 00567800
	virtual void SetLightScattering( TBOOL a_bEnable ) {} // +0x4C
	// $deBlob: FUNCTION 005676b0
	virtual TBOOL IsAlphaBlendMaterial() { return TFALSE; } // +0x50
	// $deBlob: FUNCTION 00567760
	virtual void SetAlphaBlendMaterial( TBOOL a_bEnable ) {} // +0x54

	virtual Toshi::TMaterial* CreateMaterial( const TCHAR* a_szName )                                         = 0; // +0x58
	virtual Toshi::TMesh*     CreateMesh( const TCHAR* a_szName )                                             = 0; // +0x5C
	virtual TINT              AddLight( const Toshi::TVector3& a_rPosition, TFLOAT a_fRadius )                = 0; // +0x60
	virtual void              SetLight( TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fRadius ) = 0; // +0x64
	virtual void              RemoveLight( TINT a_iIndex )                                                    = 0; // +0x68

	// $deBlob: FUNCTION 00567220
	virtual void EnableClouds( TBOOL a_bEnable ) {} // +0x6C
	// $deBlob: FUNCTION 005676e0
	virtual TBOOL IsEnabledClouds() { return TFALSE; } // +0x70
	// $deBlob: FUNCTION 005676d0
	virtual TBOOL IsCapablePointLights() { return TFALSE; } // +0x74
	// $deBlob: FUNCTION 00567820
	virtual void SetPointLights( TBOOL a_bEnable ) {} // +0x78
	// $deBlob: FUNCTION 00567720
	virtual TBOOL IsPointLightsEnabled() { return TFALSE; } // +0x7C

public:
	// Render state shared by the DX11 half while a packet is drawn
	static Toshi::TVector4       s_fogColor;
	static Toshi::TVector4       s_FogStartEnd; // x start, y end, z > 0: fog on
	static TINT                  s_NumTevStages;
	static Toshi::TVector4       s_AlphaTest; // x enabled, y reference
	static Toshi::TVector4       s_ChanAmbColor0;
	static Toshi::TVector4       s_ChanMatColor0;
	static const Toshi::TVector4 s_Light7Color;
	static const Toshi::TVector4 s_Light7Pos;

protected:
	TUINT32 m_uiUnk0C; // 0x0C
	TUINT32 m_uiUnk10; // 0x10 0x80
	TFLOAT  m_fUnk14;  // 0x14 1.0f
	TUINT32 m_uiUnk18; // 0x18
};                     // 0x1C

//-----------------------------------------------------------------------------
// Purpose: the material of a skin mesh: a blend mode (and the pass of the jelly/stencil blend modes)
//-----------------------------------------------------------------------------
class AJellyShader_DX11;

class ASkinMaterial : public Toshi::TMaterial
{
	friend class AJellyShader_DX11;

public:
	TDECLARE_CLASS( ASkinMaterial, Toshi::TMaterial );

public:
	using BLENDMODE = TINT;
	enum BLENDMODE_ : BLENDMODE
	{
		BLENDMODE_DEFAULT = 0,
		BLENDMODE_STENCIL = 16, // drawn into the stencil order tables (2 passes)
		BLENDMODE_JELLY   = 18, // drawn into the jelly order tables (6 passes)
		BLENDMODE_NUMOF   = 26,
	};

	// Material flags of the skin materials (TMaterial::m_Flags)
	enum SKINFLAGS : Toshi::TMaterial::Flags
	{
		SKINFLAG_BLENDMODE   = BITFLAG( 3 ),  // has a blend mode
		SKINFLAG_ALPHATABLE1 = BITFLAG( 6 ),  // alpha blended in the second alpha order table
		SKINFLAG_ALPHATABLE2 = BITFLAG( 7 ),  // alpha blended in the third alpha order table
		SKINFLAG_ENVMAP      = BITFLAG( 11 ), // a pass uses the environment map texture coordinates
		SKINFLAG_DEPTHWRITE  = BITFLAG( 12 ),
		SKINFLAG_UNK13       = BITFLAG( 13 ),
		SKINFLAG_RIMLIGHT    = BITFLAG( 14 ), // AMaterialInfo::APPMATFLAG_UV_RIMLIGHT
	};

public:
	ASkinMaterial();
	~ASkinMaterial();

	//-----------------------------------------------------------------------------
	// Toshi::TMaterial
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 005671f0
	virtual TBOOL Create() OVERRIDE { return Toshi::TMaterial::Create(); } // +0x10

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Create( BLENDMODE a_eBlendMode );                     // +0x1C
	virtual void SetBlendMode( BLENDMODE a_eBlendMode, TINT a_iPass ); // +0x20
	// $deBlob: FUNCTION 00601330
	virtual void                EnableEnvMap( TBOOL a_bEnable, TINT a_iPass ) {}                      // +0x24
	virtual void                SetOrderTable( Toshi::TOrderTable* a_pOrderTable, TBOOL a_bUnk ) = 0; // +0x28
	virtual Toshi::TOrderTable* GetOrderTable()                                                  = 0; // +0x2C

	void SetAlphaOrderTable();

	BLENDMODE GetBlendMode() const { return m_eBlendMode; }
	TINT      GetPass() const { return m_iPass; }

protected:
	BLENDMODE m_eBlendMode; // 0x54
	TBOOL     m_bUnk58;     // 0x58
	TINT      m_iPass;      // 0x5C
};                          // 0x60

//-----------------------------------------------------------------------------
// Purpose: a part of a skin mesh drawn with up to MAXBONES bones
//-----------------------------------------------------------------------------
struct ASkinSubMesh
{
	static TCONSTEXPR_VAR TINT MAXBONES = 30;

	ASkinSubMesh();
	~ASkinSubMesh();

	TBOOL CreateIndexBuffer( TUINT32 a_uiUnused, TINT a_iNumIndices, const void* a_pIndices );

	TUINT16       GetNumIndices() const { return uiNumIndices; }
	ID3D11Buffer* GetIndexBuffer() const { return pIndexBuffer; }
	TUINT32       GetNumBoneInfluences() const { return uiNumBones; }
	TINT          GetRemappedMatrix( TUINT32 a_uiBone ) const { return aBoneRemap[ a_uiBone ]; }

	TUINT16       uiNumIndices;           // 0x00
	TUINT16       uiNumVertices;          // 0x02
	ID3D11Buffer* pIndexBuffer;           // 0x04
	TFLOAT        afUnk08[ 6 ];           // 0x08 copied from the TRB sub mesh
	TUINT32       uiNumBones;             // 0x20
	TINT          aBoneRemap[ MAXBONES ]; // 0x24
};                                        // 0x9C

//-----------------------------------------------------------------------------
// Purpose: a skin mesh: a vertex buffer shared by its sub meshes
//-----------------------------------------------------------------------------
class ASkinMesh : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( ASkinMesh, Toshi::TMesh );

	static TCONSTEXPR_VAR TUINT32 VERTEX_SIZE = 0x2C;

public:
	ASkinMesh();
	~ASkinMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() OVERRIDE;
	virtual void  Invalidate() OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL         Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT32 a_uiNumSubMeshes ); // +0x20
	virtual TBOOL         Lock( void* a_pLockBuffer );                                                    // +0x24
	virtual void          Unlock( TUINT32 a_uiNumVertices );                                              // +0x28
	virtual ID3D11Buffer* GetVertexPool();                                                                // +0x2C

	using Toshi::TMesh::Create;

	TUINT16 GetNumSubMeshes() const { return m_uiNumSubMeshes; }
	State   GetState() const { return m_State; }
	// $deBlob: FUNCTION 00567590
	ASkinSubMesh* GetSubMesh( TUINT32 a_uiSubMesh )
	{
		TASSERT( a_uiSubMesh < m_uiNumSubMeshes );
		return &m_pSubMeshes[ a_uiSubMesh ];
	}

	Toshi::TMaterial* GetMaterial2() const { return m_pMaterial2; }
	void              SetMaterial2( Toshi::TMaterial* a_pMaterial ) { m_pMaterial2 = a_pMaterial; }

protected:
	TBOOL CreatePools();
	// Releases the vertex buffer and the sub meshes (OnDestroy without the materials); always TFALSE
	TBOOL DestroyPools();

protected:
	TUINT32           m_uiFlags;        // 0x10
	TUINT16           m_uiMaxVertices;  // 0x14
	TUINT16           m_uiNumSubMeshes; // 0x16
	ID3D11Buffer*     m_pVertexBuffer;  // 0x18
	ASkinSubMesh*     m_pSubMeshes;     // 0x1C
	TUINT32           m_uiUnk20;        // 0x20
	Toshi::TMaterial* m_pMaterial2;     // 0x24 a second material, destroyed with the mesh
};                                      // 0x28

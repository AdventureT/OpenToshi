#pragma once
#include "Shaders/ASkinShader/ASkinShader.h"

#include <Render/TOrderTable.h>
#include <Render/TRenderPacket.h>
#include <Math/TMatrix44.h>
#include <Platform/DX11/TShaderCache_DX11.h>

//-----------------------------------------------------------------------------
// Purpose: DX11 half of the skin material: up to MAX_PASSES texture passes (a TEV-like stage each, from the
// GameCube/Wii versions), the order table it's registered in
//-----------------------------------------------------------------------------
class AJellyShader_DX11;

class ASkinMaterialHAL : public ASkinMaterial
{
	friend class ::AJellyShader_DX11; // resets the pass of the solids' materials (debug)

public:
	TDECLARE_CLASS( ASkinMaterialHAL, ASkinMaterial );

	static TCONSTEXPR_VAR TINT MAX_PASSES = 4;

	// How a pass computes its texture coordinates
	using PASSMODE = TINT;
	enum PASSMODE_ : PASSMODE
	{
		PASSMODE_DEFAULT   = 0,
		PASSMODE_UVOFFSET  = 1, // scrolled by m_afUOffset/m_afVOffset
		PASSMODE_ENVMAP    = 2, // SKINFLAG_ENVMAP
		PASSMODE_PROJECTED = 3, // one of ASkinShaderHAL's projected textures (m_iUserDataIndex)
		PASSMODE_RIMLIGHT  = 4, // SKINFLAG_RIMLIGHT
	};

public:
	ASkinMaterialHAL();
	~ASkinMaterialHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMaterial
	//-----------------------------------------------------------------------------
	virtual void OnDestroy() OVERRIDE;
	virtual void PreRender() OVERRIDE;
	virtual void PostRender() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ASkinMaterial
	//-----------------------------------------------------------------------------
	virtual void                Create( BLENDMODE a_eBlendMode ) OVERRIDE;
	virtual void                SetBlendMode( BLENDMODE a_eBlendMode, TINT a_iPass ) OVERRIDE;
	virtual void                EnableEnvMap( TBOOL a_bEnable, TINT a_iPass ) OVERRIDE;
	virtual void                SetOrderTable( Toshi::TOrderTable* a_pOrderTable, TBOOL a_bUnk ) OVERRIDE;
	virtual Toshi::TOrderTable* GetOrderTable() OVERRIDE;

	using ASkinMaterial::Create;

	void SetPassMode( TINT a_iPass, PASSMODE a_eMode );
	void SetUVOffset( TFLOAT a_fUOffset, TFLOAT a_fVOffset, TINT a_iPass );

	// $deBlob: FUNCTION 00567630
	PASSMODE GetPassMode( TINT a_iPass ) const
	{
		TASSERT( a_iPass < MAX_PASSES );
		return m_aPassModes[ a_iPass ];
	}

	TINT  GetTevMode( TINT a_iPass ) const { return m_aTevModes[ a_iPass ]; }
	void  SetTevMode( TINT a_iPass, TINT a_iMode ) { m_aTevModes[ a_iPass ] = a_iMode; }
	TINT8 GetUserDataIndex() const { return m_iUserDataIndex; }
	void  SetUserDataIndex( TINT8 a_iIndex ) { m_iUserDataIndex = a_iIndex; }

	TINT16 GetFakeOpaqueFadeID() const { return m_iFakeOpaqueFadeID; }
	void   SetFakeOpaqueFadeID( TINT16 a_iID ) { m_iFakeOpaqueFadeID = a_iID; }

	TUINT8 GetAlphaRef() const { return m_uiAlphaRef; }

	// Sets the UV offset of a pass without SetUVOffset's range asserts (ABlobAnimationComponent::Update)
	void SetUVOffsetNoCheck( TFLOAT a_fUOffset, TFLOAT a_fVOffset, TINT a_iPass )
	{
		m_afUOffset[ a_iPass ] = a_fUOffset;
		m_afVOffset[ a_iPass ] = a_fVOffset;
	}

	static ASkinMaterialHAL* Cast( Toshi::TMaterial* a_pMaterial )
	{
		TASSERT( a_pMaterial == TNULL || a_pMaterial->IsA( &TGetClass( ASkinMaterialHAL ) ) );
		return TSTATICCAST( ASkinMaterialHAL, a_pMaterial );
	}

private:
	TFLOAT              m_afUOffset[ MAX_PASSES ];  // 0x60
	TFLOAT              m_afVOffset[ MAX_PASSES ];  // 0x70
	Toshi::TOrderTable* m_pOrderTable;              // 0x80
	TINT                m_aTevModes[ MAX_PASSES ];  // 0x84 how each stage combines its texture (0x11, 0x13...)
	TINT                m_iNumTevStages;            // 0x94
	TUINT8              m_uiAlphaRef;               // 0x98 alpha test reference (/255)
	TINT8               m_iUserDataIndex;           // 0x99 the projected texture of PASSMODE_PROJECTED
	TINT16              m_iFakeOpaqueFadeID;        // 0x9A slot in ASkinShaderHAL's fake opaque fades, -1: none
	PASSMODE            m_aPassModes[ MAX_PASSES ]; // 0x9C
};                                                  // 0xAC

//-----------------------------------------------------------------------------
// Purpose: DX11 half of the skin mesh: Render adds a render packet to its material
//-----------------------------------------------------------------------------
class ASkinMeshHAL : public ASkinMesh
{
public:
	TDECLARE_CLASS( ASkinMeshHAL, ASkinMesh );

public:
	ASkinMeshHAL();
	~ASkinMeshHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;

private:
	TUINT32 m_uiUnk28; // 0x28
};                     // 0x2C

//-----------------------------------------------------------------------------
// Purpose: DX11 half of the skin shader: the order tables, compiles its vertex/pixel shader permutations from the HLSL
// in the exe through a TShaderCache_DX11 (Data\Shaders\Skin\), draws the packets with up to ASkinSubMesh::MAXBONES bone
// matrices per sub mesh.
//-----------------------------------------------------------------------------
class ASkinShaderHAL : public ASkinShader
{
public:
	TDECLARE_CLASS( ASkinShaderHAL, ASkinShader );

	static TCONSTEXPR_VAR TINT ORDER_TABLE_COUNT         = 3;
	static TCONSTEXPR_VAR TINT ALPHA_ORDER_TABLE_COUNT   = 3;
	static TCONSTEXPR_VAR TINT JELLY_ORDER_TABLE_COUNT   = 6;
	static TCONSTEXPR_VAR TINT STENCIL_ORDER_TABLE_COUNT = 2;
	static TCONSTEXPR_VAR TINT MAX_FAKE_OPAQUE_FADES     = 150;
	static TCONSTEXPR_VAR TINT MAX_PROJ_TEXTURES         = 4;

	// The tev register scales of Render (PS c5, c6), set by AEnvironmentFX
	static Toshi::TVector4 s_vTevReg0Scale;
	// The heap of the skin meshes and materials (TNULL: the default one); set by AMemory::SetAssetHeaps
	static TMemoryHeap*    ms_pMemHeap;
	static Toshi::TVector4 s_vTevReg1Scale;
	// PS c2/c7 of tev mode 0x16: the glow colour and the fade of a completed landmark's accessory paint pass
	// (ALandmarkInstance::UpdateImp/AddPaintPass)
	static Toshi::TVector4 s_vTevMode16Colour;
	static Toshi::TVector4 s_vTevMode16Const;
	static Toshi::TVector4 s_vTevMode1AColour; // tev mode 0x1a (Blob's paint flash), w set by ABlobAnimationComponent

	// A material that draws an opaque mesh while it fades out (alpha blended, depth written first)
	struct FakeOpaqueFade
	{
		ASkinMaterialHAL* pMaterial;       // 0x00
		ASkinMaterialHAL* pSourceMaterial; // 0x04 the mesh's own material while it fades
	};

	// A texture projected onto the skin meshes (PASSMODE_PROJECTED)
	struct ProjectedTexture
	{
		Toshi::TMatrix44 oMatrix;        // 0x00
		TUINT32          uiColour;       // 0x40 ARGB
		TUINT32          bHasFrustum;    // 0x44 0: the default 40 degree projection
		TFLOAT           fUnk48;         // 0x48
		TFLOAT           fUnk4C;         // 0x4C
		TFLOAT           fUnk50;         // 0x50
		TFLOAT           fUnk54;         // 0x54
		TUINT8           aUnk58[ 0x0C ]; // 0x58
	};                                   // 0x64

public:
	ASkinShaderHAL();
	~ASkinShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void  OnDestroy() OVERRIDE;
	virtual void  Flush() OVERRIDE;
	virtual void  StartFlush() OVERRIDE;
	virtual void  EndFlush() OVERRIDE;
	virtual TBOOL Create() OVERRIDE;
	virtual void  Render( Toshi::TRenderPacket* a_pRenderPacket ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// ASkinShader
	//-----------------------------------------------------------------------------
	virtual Toshi::TMaterial* CreateMaterial( const TCHAR* a_szName ) OVERRIDE;
	virtual Toshi::TMesh*     CreateMesh( const TCHAR* a_szName ) OVERRIDE;
	// $deBlob: FUNCTION 005671d0
	virtual TINT AddLight( const Toshi::TVector3& a_rPosition, TFLOAT a_fRadius ) OVERRIDE { return -1; }
	// $deBlob: FUNCTION 005677d0
	virtual void SetLight( TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fRadius ) OVERRIDE {}
	// $deBlob: FUNCTION 00567750
	virtual void RemoveLight( TINT a_iIndex ) OVERRIDE {}

	Toshi::TOrderTable* GetAlphaOrderTable( TINT a_iIndex );
	Toshi::TOrderTable* GetOrderTableAt( TINT a_iIndex ) { return &m_aOrderTables[ a_iIndex ]; }
	Toshi::TOrderTable* GetOrderTableByIndex( TINT a_iIndex );
	void                ResetFakeOpaqueFades();

	// $deBlob: FUNCTION 00567500
	Toshi::TOrderTable* GetOpaqueOrderTable() { return &m_OrderTable; }
	// $deBlob: FUNCTION 00567690
	Toshi::TOrderTable* GetUnkOrderTable() { return &m_UnkOrderTable; }

	// $deBlob: FUNCTION 00567420
	Toshi::TOrderTable* GetJellyOrderTable( TINT a_iPass ) { return &m_aJellyOrderTables[ a_iPass ]; }
	// $deBlob: FUNCTION 00567570
	Toshi::TOrderTable* GetStencilOrderTable( TINT a_iPass ) { return &m_aStencilOrderTables[ a_iPass ]; }

	// $deBlob: FUNCTION 00567300
	FakeOpaqueFade& GetFakeOpaqueFade( TINT a_iID, TBOOL a_bSecond )
	{
		TASSERT( -1 < a_iID && a_iID < MAX_FAKE_OPAQUE_FADES );
		return a_bSecond ? m_aFakeOpaqueFades2[ a_iID ] : m_aFakeOpaqueFades[ a_iID ];
	}

	// $deBlob: FUNCTION 005675e0
	ProjectedTexture& GetProjectedTexture( TINT a_iIndex )
	{
		TASSERT( a_iIndex < MAX_PROJ_TEXTURES );
		return m_aProjectedTextures[ a_iIndex ];
	}

	TINT16 GetNumFakeOpaqueFades() const { return m_iNumFakeOpaqueFades; }
	void   SetNumFakeOpaqueFades( TINT16 a_iNum ) { m_iNumFakeOpaqueFades = a_iNum; }

private:
	static ID3DBlob* __stdcall CompileVertexShader( const Toshi::VertexShaderId& a_rId );
	static ID3D11InputLayout* __stdcall CreateInputLayout( const Toshi::VertexShaderId& a_rId, ID3DBlob* a_pBlob );
	static ID3DBlob* __stdcall CompilePixelShader( const Toshi::PixelShaderId& a_rId );

private:
	Toshi::TOrderTable       m_OrderTable;                                       // 0x01C opaque
	Toshi::TOrderTable       m_aOrderTables[ ORDER_TABLE_COUNT ];                // 0x044
	Toshi::TOrderTable       m_aAlphaOrderTables[ ALPHA_ORDER_TABLE_COUNT ];     // 0x0BC
	Toshi::TOrderTable       m_aJellyOrderTables[ JELLY_ORDER_TABLE_COUNT ];     // 0x134
	Toshi::TOrderTable       m_aStencilOrderTables[ STENCIL_ORDER_TABLE_COUNT ]; // 0x224
	Toshi::TOrderTable       m_UnkOrderTable;                                    // 0x274
	FakeOpaqueFade           m_aFakeOpaqueFades[ MAX_FAKE_OPAQUE_FADES ];        // 0x29C
	FakeOpaqueFade           m_aFakeOpaqueFades2[ MAX_FAKE_OPAQUE_FADES ];       // 0x74C
	TINT16                   m_iNumFakeOpaqueFades;                              // 0xBFC
	ProjectedTexture         m_aProjectedTextures[ MAX_PROJ_TEXTURES ];          // 0xC00
	Toshi::TVector4          m_LightPos;                                         // 0xD90 the sun, in view space
	Toshi::TShaderCache_DX11 m_ShaderCache;                                      // 0xDA0
};                                                                               // 0xDBC

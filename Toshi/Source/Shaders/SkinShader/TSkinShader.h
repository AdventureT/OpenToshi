#pragma once
#include "Render/TShader.h"
#include "Render/TMesh.h"
#include "Render/TMaterial.h"
#include "Math/TVector3.h"
#include "Toshi/TSingleton.h"

TOSHI_NAMESPACE_START

class TResource;

//-----------------------------------------------------------------------------
// Purpose: the engine's skin material Note: de Blob's skin meshes are made by the game's ASkinShader (with its own
// ASkinMaterial); nothing creates a TSkinShader, so these classes are only reached through TModelHAL's default skin
// loader (LoadSkinLOD)
//-----------------------------------------------------------------------------
class TSkinMaterial : public TMaterial
{
public:
	TDECLARE_CLASS( TSkinMaterial, TMaterial );

public:
	using BLENDMODE = TINT;

public:
	TSkinMaterial();
	~TSkinMaterial();

	//-----------------------------------------------------------------------------
	// TMaterial
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0070b910
	virtual TBOOL Create() OVERRIDE { return TMaterial::Create(); } // +0x10

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Create( BLENDMODE a_eBlendMode );                       // +0x1C
	virtual void SetBlendMode( BLENDMODE a_eBlendMode, TINT a_iPass );   // +0x20
	// $deBlob: FUNCTION 0070b950
	virtual void EnableEnvMap( TBOOL a_bEnable, TINT a_iPass ) {}        // +0x24

	BLENDMODE GetBlendMode() const { return m_eBlendMode; }

protected:
	BLENDMODE m_eBlendMode; // 0x54
	TBOOL     m_bUnk58;     // 0x58
}; // 0x5C

//-----------------------------------------------------------------------------
// Purpose: a part of a TSkinMesh drawn with up to MAXBONES bones (0x9C bytes)
//-----------------------------------------------------------------------------
struct TSkinSubMesh
{
	static TCONSTEXPR_VAR TINT MAXBONES = 30;

	TSkinSubMesh();
	~TSkinSubMesh();

	TUINT16    uiUnk0;                    // 0x00
	TUINT16    uiNumVertices;             // 0x02
	TResource* pIndexPool;                // 0x04
	TFLOAT     afUnk08[ 6 ];              // 0x08 copied from the TRB sub mesh (+0x1C..+0x30)
	TUINT32    uiNumBones;                // 0x20
	TINT       aBoneRemap[ MAXBONES ];    // 0x24
}; // 0x9C

//-----------------------------------------------------------------------------
// Purpose: the engine's skin mesh: a vertex pool shared by sub meshes, each with its index pool and bone remap
//-----------------------------------------------------------------------------
class TSkinMesh : public TMesh
{
public:
	TDECLARE_CLASS( TSkinMesh, TMesh );

public:
	TSkinMesh();
	~TSkinMesh();

	//-----------------------------------------------------------------------------
	// TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() OVERRIDE;
	virtual void  Invalidate() OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL      Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT32 a_uiNumSubMeshes ); // +0x20
	virtual TBOOL      Lock( void* a_pLockBuffer );                                                    // +0x24
	virtual void       Unlock( TUINT32 a_uiNumVertices );                                              // +0x28
	virtual TResource* GetVertexPool();                                                                // +0x2C

	using TMesh::Create;

	TUINT16       GetNumSubMeshes() const { return m_uiNumSubMeshes; }
	// $deBlob: FUNCTION 006b6210
	TSkinSubMesh* GetSubMesh( TUINT32 a_uiSubMesh )
	{
		TASSERT( a_uiSubMesh < m_uiNumSubMeshes );
		return &m_pSubMeshes[ a_uiSubMesh ];
	}

	TMaterial* GetMaterial2() const { return m_pMaterial2; }
	void       SetMaterial2( TMaterial* a_pMaterial ) { m_pMaterial2 = a_pMaterial; }

private:
	TBOOL CreatePools();
	void  DestroyPools();

private:
	TUINT32       m_uiFlags;        // 0x10
	TUINT16       m_uiMaxVertices;  // 0x14
	TUINT16       m_uiNumSubMeshes; // 0x16
	TResource*    m_pVertexPool;    // 0x18
	TSkinSubMesh* m_pSubMeshes;     // 0x1C
	TUINT32       m_uiUnk20;        // 0x20
	TMaterial*    m_pMaterial2;     // 0x24 a second material, destroyed with the model
}; // 0x28

//-----------------------------------------------------------------------------
// Purpose: the engine's skin shader, abstract (no platform half in de Blob)
//-----------------------------------------------------------------------------
class TSkinShader
    : public TShader
    , public TSingleton<TSkinShader>
{
public:
	TDECLARE_CLASS( TSkinShader, TShader );

public:
	TSkinShader();
	~TSkinShader();

	//-----------------------------------------------------------------------------
	// TShader
	//-----------------------------------------------------------------------------
	virtual const TCHAR* GetShaderPrefix() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods (the same as the game's ASkinShader)
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0070b970
	virtual void EnableRenderEnvMap( TBOOL a_bEnable ) {} // +0x38
	// $deBlob: FUNCTION 0070b9d0
	virtual TBOOL IsHighEndSkinning() { return TTRUE; } // +0x3C
	// $deBlob: FUNCTION 0070b960
	virtual void EnableHighEndSkinning( TBOOL a_bEnable ) {} // +0x40
	// $deBlob: FUNCTION 0070b9a0
	virtual TBOOL IsCapableHighEndSkinning() { return TTRUE; } // +0x44
	// $deBlob: FUNCTION 0070b9e0
	virtual TBOOL IsLightScattering() { return TFALSE; } // +0x48
	// $deBlob: FUNCTION 0070ba10
	virtual void SetLightScattering( TBOOL a_bEnable ) {} // +0x4C
	// $deBlob: FUNCTION 0070b990
	virtual TBOOL IsAlphaBlendMaterial() { return TFALSE; } // +0x50
	// $deBlob: FUNCTION 0070ba00
	virtual void SetAlphaBlendMaterial( TBOOL a_bEnable ) {} // +0x54

	virtual TMaterial* CreateMaterial( const TCHAR* a_szName )                                    = 0; // +0x58
	virtual TMesh*     CreateMesh( const TCHAR* a_szName )                                        = 0; // +0x5C
	virtual TINT       AddLight( const TVector3& a_rPosition, TFLOAT a_fRadius )                  = 0; // +0x60
	virtual void       SetLight( TINT a_iIndex, const TVector3& a_rPosition, TFLOAT a_fRadius )   = 0; // +0x64
	virtual void       RemoveLight( TINT a_iIndex )                                               = 0; // +0x68

	// $deBlob: FUNCTION 0070b940
	virtual void EnableClouds( TBOOL a_bEnable ) {} // +0x6C
	// $deBlob: FUNCTION 0070b9c0
	virtual TBOOL IsEnabledClouds() { return TFALSE; } // +0x70
	// $deBlob: FUNCTION 0070b9b0
	virtual TBOOL IsCapablePointLights() { return TFALSE; } // +0x74
	// $deBlob: FUNCTION 0070ba20
	virtual void SetPointLights( TBOOL a_bEnable ) {} // +0x78
	// $deBlob: FUNCTION 0070b9f0
	virtual TBOOL IsPointLightsEnabled() { return TFALSE; } // +0x7C

protected:
	TUINT32 m_uiUnk0C; // 0x0C
	TUINT32 m_uiUnk10; // 0x10 0x80
	TFLOAT  m_fUnk14;  // 0x14 1.0f
	TUINT32 m_uiUnk18; // 0x18
}; // 0x1C

TOSHI_NAMESPACE_END

#pragma once
#include "Render/TShader.h"
#include "Render/TMesh.h"
#include "Render/TRenderPacket.h"
#include "Toshi/TSingleton.h"

TOSHI_NAMESPACE_START

class TRenderPacket;
class TWorldShader;

//-----------------------------------------------------------------------------
// Purpose: a material of the world meshes. Created by TWorldShader::CreateMaterial with the name of the TRB world
// mesh's material (the name isn't copied), reference counted by the TWorldMeshes that use it and kept in
// TWorldMaterialManager while it's used.
//-----------------------------------------------------------------------------
class TWorldMaterial : public TMaterial
{
public:
	TDECLARE_CLASS( TWorldMaterial, TMaterial );

public:
	friend class TWorldMaterialManager;
	friend class TWorldMesh;

	// The pass blend modes of the T2Materials (T2Material::GetPassBlendMode), used by the DX11 layer
	using BLENDMODE = TUINT32;
	enum BLENDMODE_ : BLENDMODE
	{
		BLENDMODE_OPAQUE          = 0, // alpha tested
		BLENDMODE_ZPASS           = 6, // depth only
		BLENDMODE_WATER           = 8,
		BLENDMODE_REFLECTIVEWATER = 9,
		BLENDMODE_SHOCKWAVE       = 10,
		BLENDMODE_HEAT            = 22,
	};

	// The number of texture passes
	static TCONSTEXPR_VAR TINT TEXTURETYPE_NUMOF = 5;
	// The bump (noise) pass
	static TCONSTEXPR_VAR TINT TEXTURETYPE_BUMP = 1;

public:
	TWorldMaterial();
	~TWorldMaterial();

	//-----------------------------------------------------------------------------
	// TMaterial
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 00705a70
	virtual TBOOL Create() override { return TMaterial::Create(); } // +0x10

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( BLENDMODE a_eBlendMode );       // +0x1C
	virtual void  SetBlendMode( BLENDMODE a_eBlendMode ); // +0x20

	BLENDMODE    GetBlendMode() const { return m_eBlendMode; }
	// $deBlob: FUNCTION 00554ee0
	const TCHAR* GetMaterialName() const { return m_szMaterialName; }
	// $deBlob: FUNCTION 0055e4b0
	void         SetMaterialName( const TCHAR* a_szName ) { m_szMaterialName = a_szName; }

	// $deBlob: FUNCTION 00707260
	TINT GetRefCount() const { return m_iRefCount; }

	void IncRefCount() { m_iRefCount++; }

	void DecRefCount()
	{
		m_iRefCount--;
		TASSERT( m_iRefCount >= 0 );
	}

	// The T2Material the loader made this material from (TMaterial's 0x50)
	void* GetMetaMaterial() const { return m_pMetaMaterial; }
	void  SetMetaMaterial( void* a_pMetaMaterial ) { m_pMetaMaterial = a_pMetaMaterial; }

protected:
	TINT            m_iRefCount;      // 0x54
	TWorldMaterial* m_pNextMaterial;  // 0x58 TWorldMaterialManager's list
	const TCHAR*    m_szMaterialName; // 0x5C not a copy
	BLENDMODE       m_eBlendMode;     // 0x60
}; // 0x64

//-----------------------------------------------------------------------------
// Purpose: the TWorldMaterials in use (a list linked through m_pNextMaterial from the last added one).
//-----------------------------------------------------------------------------
class TWorldMaterialManager : public TSingleton<TWorldMaterialManager>
{
public:
	friend TWorldShader;

public:
	// $deBlob: FUNCTION 007068c0
	TWorldMaterialManager()
	    : m_pFirstMaterial( TNULL ) {}

	// $deBlob: FUNCTION 00707160
	~TWorldMaterialManager() = default;

	// TNULL if there's none of that name (case-insensitive)
	TWorldMaterial* FindMaterial( const TCHAR* a_szName );
	void            AddMaterial( TWorldMaterial* a_pMat );
	// Unlinks and deletes the material (destroying it if it's created), asserts that nothing uses it anymore
	void            RemoveMaterial( TWorldMaterial* a_pMat );

private:
	TWorldMaterial* m_pFirstMaterial; // 0x00
};

//-----------------------------------------------------------------------------
// Purpose: a mesh of the world (the TRB world meshes of a cell) rendered by TWorldShader.
//-----------------------------------------------------------------------------
class TWorldMesh : public TMesh
{
public:
	TDECLARE_CLASS( TWorldMesh, TMesh );

public:
	// Called before (a_bBefore TTRUE) and after a world mesh render packet is rendered
	using t_RenderCallback = TRenderPacket::t_RenderCallback;

public:
	TWorldMesh();
	~TWorldMesh();

	//-----------------------------------------------------------------------------
	// TMesh
	//-----------------------------------------------------------------------------
	virtual void OnDestroy() override; // +0x1C

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL RenderWithCallback( t_RenderCallback a_fnCallback, void* a_pUserData ); // +0x20

	// Sets the material, adds it to TWorldMaterialManager when it isn't used yet (a_bManaged) and releases the old
	// one (removing it when it isn't used anymore and a_bManaged)
	void SetMaterial( TWorldMaterial* a_pMaterial, TBOOL a_bManaged );

	// $deBlob: FUNCTION 00705b50
	// $deBlob: FUNCTION 00705bb0
	TWorldMaterial* GetMaterial() const
	{
		TASSERT( ( m_pMaterial == TNULL ) || m_pMaterial->IsA( &TGetClass( TWorldMaterial ) ) );
		return TSTATICCAST( TWorldMaterial, m_pMaterial );
	}

	// The owner shader with the TObject.h cast assert (defined after TWorldShader)
	TWorldShader* GetOwnerWorldShader() const;

protected:
	void* m_pUnk10; // 0x10 cleared by the constructor
}; // 0x14

//-----------------------------------------------------------------------------
// Purpose: the world shader, the platform independent part (the DX11 one is TWorldShaderHAL).
//-----------------------------------------------------------------------------
class TWorldShader
    : public TShader
    , public TSingleton<TWorldShader>
{
public:
	TDECLARE_CLASS( TWorldShader, TShader );

public:
	// Called by TModelLoaderWorld_DX11 with the new material and the T2Material it was made of
	using t_MaterialCreatedCallback   = void( __stdcall* )( TWorldMaterial* a_pMaterial, void* a_pT2Material );
	// Called by TWorldMaterialHAL::OnDestroy
	using t_MaterialDestroyedCallback = void( __stdcall* )( TWorldMaterial* a_pMaterial );

	// Called by TModelLoaderWorld_DX11 instead of CreateMesh/CreateMaterial when set (retail never sets them)
	using t_CreateMeshCallback     = TWorldMesh*( __stdcall* )( const TCHAR* a_szName );
	using t_CreateMaterialCallback = TWorldMaterial*( __stdcall* )( const TCHAR* a_szName );

public:
	TWorldShader();
	~TWorldShader();

	//-----------------------------------------------------------------------------
	// TShader
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 00705c50
	virtual const TCHAR* GetShaderPrefix() override { return "wo_"; } // +0x28

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 00705ab0
	virtual void  EnableRenderEnvMap( TBOOL a_bEnable ) {} // +0x38
	// $deBlob: FUNCTION 00705db0
	virtual TBOOL IsHighEndSkinning() { return TTRUE; } // +0x3C
	// $deBlob: FUNCTION 00705aa0
	virtual void  EnableHighEndSkinning( TBOOL a_bEnable ) {} // +0x40
	// $deBlob: FUNCTION 00705da0
	virtual TBOOL IsCapableHighEndSkinning() { return TTRUE; } // +0x44
	// $deBlob: FUNCTION 00705dc0
	virtual TBOOL IsLightScattering() { return TFALSE; } // +0x48
	// $deBlob: FUNCTION 00705e00
	virtual void  SetLightScattering( TBOOL a_bEnable ) {} // +0x4C
	// $deBlob: FUNCTION 00705d90
	virtual TBOOL IsAlphaBlendMaterial() { return TFALSE; } // +0x50
	// $deBlob: FUNCTION 00705df0
	virtual void  SetAlphaBlendMaterial( TBOOL a_bEnable ) {} // +0x54

	virtual TWorldMaterial* CreateMaterial( const TCHAR* a_szName ) = 0; // +0x58
	virtual TWorldMesh*     CreateMesh( const TCHAR* a_szName )     = 0; // +0x5C

	// $deBlob: FUNCTION 006bba00
	t_MaterialCreatedCallback   GetMaterialCreatedCallback() const { return m_fnMaterialCreated; }
	// $deBlob: FUNCTION 00576150
	void                        SetMaterialCreatedCallback( t_MaterialCreatedCallback a_fnCallback ) { m_fnMaterialCreated = a_fnCallback; }
	t_MaterialDestroyedCallback GetMaterialDestroyedCallback() const { return m_fnMaterialDestroyed; }
	// $deBlob: FUNCTION 00576160
	void                        SetMaterialDestroyedCallback( t_MaterialDestroyedCallback a_fnCallback ) { m_fnMaterialDestroyed = a_fnCallback; }

	static TMemoryHeap* GetWorldMemHeap() { return s_pWorldMemHeap; }
	static void         SetWorldMemHeap( TMemoryHeap* a_pHeap ) { s_pWorldMemHeap = a_pHeap; }

public:
	static t_CreateMaterialCallback s_fnCreateMaterial;
	static t_CreateMeshCallback     s_fnCreateMesh;
	static TMemoryHeap*             s_pWorldMemHeap; // set by the game

protected:
	TUINT32                     m_uiUnk0C;             // 0x0C not set by the constructor (retail's members start at 0x10)
	TFLOAT                      m_fUnk10;              // 0x10 0.5f
	void*                       m_pUnk14;              // 0x14
	t_MaterialCreatedCallback   m_fnMaterialCreated;   // 0x18
	t_MaterialDestroyedCallback m_fnMaterialDestroyed; // 0x1C
}; // 0x20

// $deBlob: FUNCTION 006bba30
// $deBlob: FUNCTION 006bba90
// $deBlob: FUNCTION 006bbaf0
inline TWorldShader* TWorldMesh::GetOwnerWorldShader() const
{
	TASSERT( ( m_pOwnerShader == TNULL ) || m_pOwnerShader->IsA( &TGetClass( TWorldShader ) ) );
	return TSTATICCAST( TWorldShader, m_pOwnerShader );
}

TOSHI_NAMESPACE_END

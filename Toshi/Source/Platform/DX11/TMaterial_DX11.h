#pragma once
#include "Toshi/T2ResourceManager.h"

TOSHI_NAMESPACE_START

class TTexture;

//-----------------------------------------------------------------------------
// Purpose: a material asset ("tmat" symbol of a material TRB): its textures and passes, registered as a T2Resource and
// in TMaterialManager by name.
//-----------------------------------------------------------------------------
class T2Material : public T2Resource
{
public:
	friend class TMaterialManager;

	enum FLAGS_ : TUINT32
	{
		FLAGS_INMATERIALMANAGER = BITFLAG( 0 ),
	};

	struct Texture
	{
		// No caller in de Blob
		void Init( T2Material* a_pMaterial );

		TTexture*    m_pTexture; // 0x00 resolved by InitData
		const TCHAR* m_pName;    // 0x04
	}; // 0x08

	struct Pass
	{
		TUINT32  m_uiUnk00;     // 0x00
		TUINT32  m_uiBlendMode; // 0x04
		TUINT32  m_uiTevMode;   // 0x08
		Texture* m_pTexture;    // 0x0C
		TUINT32  m_uiUVFunctionType; // 0x10
		TUINT32  m_uiUFunction;      // 0x14
		TUINT32  m_uiVFunction;      // 0x18
		TFLOAT   m_fUParam1;         // 0x1C
		TFLOAT   m_fUParam2;         // 0x20
		TFLOAT   m_fVParam1;         // 0x24
		TFLOAT   m_fVParam2;         // 0x28
	}; // 0x2C

	using t_TRBLoadCallback   = void( __stdcall* )( T2Material* a_pMaterial, void* a_pUserData );
	using t_TRBUnloadCallback = void( __stdcall* )( T2Material* a_pMaterial, void* a_pUserData );

public:
	// Resolves the textures, registers the resource and adds the material to TMaterialManager
	void Init();
	void Deinit();

	// Resolves the textures by name (TTextureManager)
	void InitData();
	void InitResource();

	TTexture* GetTexture( TUINT a_iId ) const;

	const TCHAR* GetPassTextureName( TINT a_iPass ) const;
	Pass*        GetPassInfo( TINT a_iPass ) const;
	Texture*     GetPassTexture( TINT a_iPass ) const;
	TUINT32      GetPassBlendMode( TINT a_iPass ) const;
	TUINT32      GetPassTevMode( TINT a_iPass ) const;
	TUINT32      GetPassUVFunctionType( TINT a_iPass ) const;
	TUINT32      GetPassUFunction( TINT a_iPass ) const;
	TUINT32      GetPassVFunction( TINT a_iPass ) const;
	TFLOAT       GetPassUParam1( TINT a_iPass ) const;
	TFLOAT       GetPassUParam2( TINT a_iPass ) const;
	TFLOAT       GetPassVParam1( TINT a_iPass ) const;
	TFLOAT       GetPassVParam2( TINT a_iPass ) const;

	const TCHAR* GetName() const { return m_szName; }
	TINT         GetNumTextures() const { return m_iNumTex; }
	// $deBlob: FUNCTION 005f8eb0
	TINT         GetNumPasses() const { return m_iNumPasses; }
	// $deBlob: FUNCTION 005f99a0
	TUINT32 GetOrderTableIndex() const { return m_uiOrderTableIndex; } // the world order table index (TModelLoaderWorld_DX11)
	// $deBlob: FUNCTION 005f8ec0
	TUINT32      GetUnk1C() const { return m_uiUnk1C; } // the cull mode of the skin materials (AModelLoader::GetMaterial)
	// $deBlob: FUNCTION 005f9930
	TUINT32      GetFlags() const { return m_uiFlags; }

	TBOOL HasFlag( TUINT32 a_uiFlag ) const { return ( m_uiFlags & a_uiFlag ) == a_uiFlag; }

	static void* __stdcall ResourceCallback( void* a_pCustomData, TTRB* a_pTRB, TBOOL a_bCreated );

public:
	static t_TRBLoadCallback   sm_pTRBLoadCallback;
	static void*               sm_pTRBLoadCallbackData;
	static t_TRBUnloadCallback sm_pTRBUnloadCallback;
	static void*               sm_pTRBUnloadCallbackData;

private:
	TUINT32      m_uiUnk04;    // 0x04
	TUINT32      m_uiFlags;    // 0x08
	const TCHAR* m_szName;     // 0x0C
	TINT         m_iNumTex;    // 0x10
	Texture*     m_aTextures;  // 0x14
	TUINT32      m_uiOrderTableIndex; // 0x18
	TUINT32      m_uiUnk1C;    // 0x1C
	Pass*        m_aPasses;    // 0x20
	TINT         m_iNumPasses; // 0x24
	TUINT32      m_uiUnk28;    // 0x28
	T2Material*  m_pNext;      // 0x2C
	T2Material*  m_pPrev;      // 0x30
}; // 0x34

//-----------------------------------------------------------------------------
// Purpose: the T2Materials by name (a list linked through m_pNext from the last added one)
// de Blob: 4 bytes, created by TRenderDX11's constructor
//-----------------------------------------------------------------------------
class TMaterialManager
{
public:
	TMaterialManager();
	~TMaterialManager();

	void AddMaterial( T2Material* a_pMat );
	void RemoveMaterial( T2Material* a_pMat );

	// TNULL if there's none of that name
	T2Material* FindMaterial( const TCHAR* a_pName ) const;

	static TMaterialManager* GetSingleton() { return s_pSingleton; }

private:
	static TMaterialManager* s_pSingleton;

private:
	T2Material* m_pHeadMaterial; // 0x00
};

TOSHI_NAMESPACE_END

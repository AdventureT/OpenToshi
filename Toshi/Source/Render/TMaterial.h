#pragma once
#include "TTexture.h"

TOSHI_NAMESPACE_START

class TRenderPacket;
class TRegMaterial;
class TShader;
class TMesh;

class TMaterial : public TObject
{
public:
	TDECLARE_CLASS( TMaterial, TObject );

public:
	static TCONSTEXPR_VAR TUINT MAXTEXTURES   = 6;
	static TCONSTEXPR_VAR TUINT NAMESIZELIMIT = 31;

	typedef TUINT32 Flags;
	enum Flags_ : Flags
	{
		FLAGS_NONE             = 0,
		FLAGS_NO_CULL          = BITFLAG( 0 ),
		FLAGS_BLENDING         = BITFLAG( 1 ),
		FLAGS_UNK2             = BITFLAG( 2 ),
		FLAGS_EXCLUSIVE        = FLAGS_NO_CULL | FLAGS_BLENDING | FLAGS_UNK2, // SetFlags clears these when setting one
		FLAGS_GLOW             = BITFLAG( 3 ),
		FLAGS_CREATED          = BITFLAG( 4 ),
		FLAGS_INSTANCE         = BITFLAG( 8 ), // an instance material: TWorldShader sets it up with the material of its mesh
		FLAGS_PROJECTEDTEXTURE = BITFLAG( 9 ), // TWorldShader binds its projected texture (texMtx6)
		FLAGS_NO_DEPTH_TEST    = BITFLAG( 27 ),
		FLAGS_ALPHAUPDATE      = BITFLAG( 28 ),
		FLAGS_ALPHATEXTURE     = BITFLAG( 31 ),
	};

	friend TRegMaterial;

public:
	TMaterial();
	~TMaterial();

protected:
	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void  OnDestroy();  // +0x0C
	virtual TBOOL Create();     // +0x10
	virtual void  PreRender();  // +0x14
	virtual void  PostRender(); // +0x18

public:
	void Destroy();

	TShader* GetShader() const;
	void     SetShader( TShader* a_pShader );

	TTexture* GetTexture( TUINT32 a_iStage = 0 ) const;
	void      SetTexture( TUINT32 a_iStage, TTexture* a_pTexture );

	TUINT32 GetTextureNum() const;
	void    SetTextureNum( TUINT32 a_iNumTextures );
	TUINT32 GetNumTextures() const { return m_iNumTex; }

	const TCHAR* GetName() const { return m_szName; }
	void         SetName( const TCHAR* a_szName );

	Flags GetFlags() const { return m_Flags; }
	void  SetFlags( Flags a_eFlag, TBOOL a_bEnable );

	TRegMaterial* GetRegMaterial() const;
	void          SetRegMaterial( TRegMaterial* a_pRegMaterial );

	TBOOL IsCreated() const;
	TBOOL IsBlending() const;

	void* GetUVAnimation() const { return m_pUVAnimation; }
	void  SetUVAnimation( void* a_pUVAnimation ) { m_pUVAnimation = a_pUVAnimation; }
	// $deBlob: FUNCTION 00576110
	void* GetMetaMaterial() const { return m_pMetaMaterial; }
	void  SetMetaMaterial( void* a_pMetaMaterial ) { m_pMetaMaterial = a_pMetaMaterial; }

	TRenderPacket* AddRenderPacket( TMesh* a_pMesh );

protected:
	Flags         m_Flags;                       // 0x04
	TShader*      m_pShader;                     // 0x08
	TRegMaterial* m_pRegMaterial;                // 0x0C
	TUINT32       m_iNumTex;                     // 0x10
	TTexture*     m_pTextures[ MAXTEXTURES ];    // 0x14
	TCHAR         m_szName[ NAMESIZELIMIT + 1 ]; // 0x2C
	void*         m_pUVAnimation;                // 0x4C cleared by OnDestroy (old name)
	void*         m_pMetaMaterial;               // 0x50 (old name)
}; // 0x54

TOSHI_NAMESPACE_END

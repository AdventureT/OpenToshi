#pragma once
#include "Render/TMaterial.h"
#include "Render/TTexture.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// The material of the GUI elements (0x68 bytes): a texture and how it's sampled. Created by T2GUI::CreateMaterial.
// NOTE: de Blob's layout; Barnyard's (0x64) has one texture address and no sampler id.
//-----------------------------------------------------------------------------
class T2GUIMaterial : public TMaterial
{
public:
	TDECLARE_CLASS( T2GUIMaterial, TMaterial );

	friend class T2GUIRenderer;

	using BLENDSTATE = TUINT;
	enum BLENDSTATE_ : BLENDSTATE
	{
		BLENDSTATE_OPAQUE      = 0,
		BLENDSTATE_ALPHA       = 1,
		BLENDSTATE_ADDITIVE    = 2,
		BLENDSTATE_SUBTRACTIVE = 3,
	};

	// The TRenderDX11 sampler state of the texture (TRenderDX11::SAMPLER_): 1 linear clamp, 3 linear wrap
	using TEXTUREADDRESS = TUINT;
	enum TEXTUREADDRESS_ : TEXTUREADDRESS
	{
		TEXTUREADDRESS_CLAMP = 1,
		TEXTUREADDRESS_WRAP  = 3,
	};

public:
	// NOTE: T2GUI (de Blob's material owner) calls these
	using TMaterial::Create;
	using TMaterial::OnDestroy;

public:
	T2GUIMaterial() TNOEXCEPT;
	~T2GUIMaterial();

	// $deBlob: FUNCTION 005d5fb0
	void SetBlendState( BLENDSTATE a_eBlendState ) { m_eBlendState = a_eBlendState; }
	BLENDSTATE GetBlendState() const { return m_eBlendState; }

	// The address of U, also the sampler state TXUIRenderer binds
	void           SetTextureAddress( TEXTUREADDRESS a_eTextureAddress ) { m_eTextureAddress = a_eTextureAddress; }
	TEXTUREADDRESS GetTextureAddress() const { return m_eTextureAddress; }
	TINT           GetSamplerId() const { return TINT( m_eTextureAddress ); }

	void           SetTextureAddressV( TEXTUREADDRESS a_eTextureAddress ) { m_eTextureAddressV = a_eTextureAddress; }
	TEXTUREADDRESS GetTextureAddressV() const { return m_eTextureAddressV; }

	void SetUnk64( TUINT32 a_uiValue );

	// $deBlob: FUNCTION 00550dc0
	TTexture* GetTextureResource() const { return m_pTextureResource; }
	// $deBlob: FUNCTION 00551040
	void      SetTextureResource( TTexture* a_pTexture ) { m_pTextureResource = a_pTexture; }

protected:
	TTexture*      m_pTextureResource; // 0x54
	BLENDSTATE     m_eBlendState;      // 0x58
	TEXTUREADDRESS m_eTextureAddress;  // 0x5C U (the sampler id)
	TEXTUREADDRESS m_eTextureAddressV; // 0x60 V
	TUINT32        m_uiUnk64;          // 0x64
};

TOSHI_NAMESPACE_END

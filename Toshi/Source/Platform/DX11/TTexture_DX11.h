#pragma once
#include "Toshi/T2ResourceManager.h"

#include <d3d11.h>

TOSHI_NAMESPACE_START

class TTRB;

//-----------------------------------------------------------------------------
// de Blob's TTexture is not a TResource (no TTextureFactory): a plain
// 0x30-byte class linked into TTextureManager's list.
//-----------------------------------------------------------------------------
class TTexture
{
public:
	friend class TTextureManager;

	enum Flags1 : TUINT8
	{
		Flags1_Alpha = BITFLAG( 0 ), // the texture has an alpha channel
	};

	// 0x18 bytes
	struct Info
	{
		UINT                      Width;       // 0x00
		UINT                      Height;      // 0x04
		UINT                      MipMapCount; // 0x08
		TUINT16                   Width2;      // 0x0C not set in TTexture_DX11.cpp
		TUINT16                   Height2;     // 0x0E
		DXGI_FORMAT               Format;      // 0x10
		ID3D11ShaderResourceView* SRView;      // 0x14

		Info()
		{
			Width       = 0;
			Height      = 0;
			MipMapCount = 0;
			Width2      = 0;
			Height2     = 0;
			Format      = DXGI_FORMAT_UNKNOWN;
			SRView      = TNULL;
		}
	};

public:
	// $deBlob: FUNCTION 006a4420
	TTexture()
	{
		m_TexName      = TNULL;
		m_TexInfo      = TNULL;
		m_DataSize     = 0;
		m_TexData      = TNULL;
		m_pPrevTexture = TNULL;
		m_pNextTexture = TNULL;
		m_Flags1       = 0;
		m_eAddressU    = D3D11_TEXTURE_ADDRESS_WRAP;
		m_eAddressV    = D3D11_TEXTURE_ADDRESS_WRAP;
		m_eFilter      = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		m_SamplerId    = 3;
	}

	// Creates the shader resource view from the DDS data (or Data\TextureOverride\<name>) and adds the texture to
	// the manager
	void Init();

	// Releases the shader resource view and removes the texture from the manager
	void Deinit();

	// Registers the texture in T2ResourceManager (by name)
	void CreateResource();

	// R8G8B8A8 or BC3 (DXT5)
	TBOOL IsMultiBitAlpha();

	// Binds the texture and its sampler state to the pixel shader slot a_uiSlot
	void Bind( UINT a_uiSlot );

	void SetWrap( D3D11_TEXTURE_ADDRESS_MODE a_eAddressU, D3D11_TEXTURE_ADDRESS_MODE a_eAddressV );
	D3D11_TEXTURE_ADDRESS_MODE GetWrapS() const { return m_eAddressU; }
	D3D11_TEXTURE_ADDRESS_MODE GetWrapT() const { return m_eAddressV; }
	void SetFilter( D3D11_FILTER a_eFilter );

	// Picks the TRenderDX11 sampler state (SAMPLER_) of the addressing and filter
	void SelectSamplerId();

	static void* __stdcall ResourceCallback( void* a_pCustomData, TTRB* a_pTRB, TBOOL a_bCreated );

	const TCHAR*              GetName() const { return m_TexName; }
	void                      SetName( const TCHAR* a_szName ) { m_TexName = a_szName; }
	TUINT8                    GetFlags1() const { return m_Flags1; }
	Info*                     GetInfo() const { return m_TexInfo; }
	ID3D11ShaderResourceView* GetHandle() const { return m_TexInfo->SRView; }
	UINT                      GetWidth() const { return m_TexInfo->Width; }
	UINT                      GetHeight() const { return m_TexInfo->Height; }
	TINT                      GetSamplerId() const { return m_SamplerId; }

	void SetData( TUINT8* a_pData, TUINT32 a_uiDataSize )
	{
		m_TexData  = a_pData;
		m_DataSize = a_uiDataSize;
	}

	TTexture* GetPrev() const { return m_pPrevTexture; }
	TTexture* GetNext() const { return m_pNextTexture; }

	void SetAlphaEnabled( TBOOL a_bEnabled )
	{
		if ( a_bEnabled )
			m_Flags1 |= Flags1_Alpha;
		else
			m_Flags1 &= ~Flags1_Alpha;
	}

private:
	T2Resource                 m_Resource;     // 0x00
	const TCHAR*               m_TexName;      // 0x04
	Info*                      m_TexInfo;      // 0x08
	TUINT32                    m_DataSize;     // 0x0C
	TUINT8*                    m_TexData;      // 0x10 DDS file data (in the TTRB)
	TTexture*                  m_pPrevTexture; // 0x14
	TTexture*                  m_pNextTexture; // 0x18
	TUINT8                     m_Flags1;       // 0x1C
	D3D11_TEXTURE_ADDRESS_MODE m_eAddressU;    // 0x20
	D3D11_TEXTURE_ADDRESS_MODE m_eAddressV;    // 0x24
	D3D11_FILTER               m_eFilter;      // 0x28
	TINT                       m_SamplerId;    // 0x2C
};

//-----------------------------------------------------------------------------
// The textures are a list linked through m_pPrevTexture from the last added one; the white and invalid textures aren't in it.
//-----------------------------------------------------------------------------
class TTextureManager
{
public:
	friend class TTexture;

public:
	TTextureManager();
	~TTextureManager();

	// Creates a texture of a_pInfo (named "runtime") and adds it
	TTexture* CreateTexture( TTexture::Info* a_pInfo );

	// Removes and deletes a texture made by CreateTexture (its Info and view are the caller's)
	void DestroyTexture( TTexture* a_pTexture );

	// Returns the invalid texture if there's no texture of that name
	TTexture* FindTexture( const TCHAR* a_szName );

	void AddTexture( TTexture* a_pTexture );
	void RemoveTexture( TTexture* a_pTexture );

	TTexture* GetLastTexture() const { return m_pLastTexture; }
	TTexture* GetInvalidTexture() const { return m_pInvalidTexture; }
	TTexture* GetWhiteTexture() const { return m_pWhiteTexture; }

	static TTextureManager* GetSingleton() { return s_pSingleton; }

private:
	static TTextureManager* s_pSingleton;

private:
	TTexture* m_pLastTexture;    // 0x00
	TTexture* m_pInvalidTexture; // 0x04
	TTexture* m_pWhiteTexture;   // 0x08
};

TOSHI_NAMESPACE_END

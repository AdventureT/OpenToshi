#include "ToshiPCH.h"
#include "TTexture_DX11.h"
#include "TRender_DX11.h"
#include "XTK/DDSTextureLoader.h"
#include "XTK/WICTextureLoader.h"
#include "File/TFile.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TTextureManager* TTextureManager::s_pSingleton = TNULL;

static const TCHAR* g_InvalidTexName = "invalid";
static const TCHAR* g_WhiteTexName   = "white";

// Returns the file name of a_szPath (with its extension if a_bExtension), in a static buffer
// NOTE: retail's lives outside TTexture_DX11.cpp (TTexture::Init is its only caller)
// $deBlob: FUNCTION 00828930
static const TCHAR* GetFileNameFromPath( const TCHAR* a_szPath, TBOOL a_bExtension )
{
	static TCHAR s_szFileName[ 256 ];

	TCHAR szExt[ 256 ];
	TCHAR szFileName[ 256 ];
	_splitpath( a_szPath, TNULL, TNULL, szFileName, szExt );

	if ( a_bExtension == TTRUE )
		_makepath( s_szFileName, TNULL, TNULL, szFileName, szExt );
	else
		_makepath( s_szFileName, TNULL, TNULL, szFileName, TNULL );

	return s_szFileName;
}

// Retail falls back to DirectX::CreateWICTextureFromMemory (DirectXTK library code) when the data isn't a
// DDS file (maxsize 0)
static HRESULT CreateWICTextureFromMemory( ID3D11Device* a_pDevice, const TUINT8* a_pData, TSIZE a_uiDataSize, ID3D11Resource** a_ppTexture, ID3D11ShaderResourceView** a_ppTextureView )
{
	return DirectX::CreateWICTextureFromMemory( a_pDevice, a_pData, a_uiDataSize, a_ppTexture, a_ppTextureView, 0 );
}

// $deBlob: FUNCTION 00693620
void TTexture::Init()
{
	TASSERT( 0 < m_DataSize );

	TRenderDX11* pRender   = TRenderDX11::Interface();
	HRESULT      hResult   = -1;
	ID3D11Resource* pResource = TNULL;

	TString8 strPath = TString8( "Data\\TextureOverride\\" ) + GetFileNameFromPath( m_TexName, TTRUE );
	TFile*   pFile   = TFile::Create( strPath, TFILEMODE_READ );

	if ( pFile != TNULL )
	{
		if ( strPath.Find( ".tga", 0 ) <= 0 )
		{
			// A DDS (or WIC) file
			TSIZE   uiFileSize = pFile->GetSize();
			TUINT8* pFileData  = TSTATICCAST( TUINT8, TMemalign( 16, uiFileSize ) );
			pFile->Read( pFileData, uiFileSize );
			pFile->Destroy();

			hResult = DirectX::CreateDDSTextureFromMemory( pRender->GetDevice(), pFileData, uiFileSize, &pResource, &m_TexInfo->SRView, 0, TNULL );

			if ( FAILED( hResult ) )
				hResult = CreateWICTextureFromMemory( pRender->GetDevice(), pFileData, uiFileSize, &pResource, &m_TexInfo->SRView );

			TFree( pFileData );
		}
		else
		{
			pFile->Destroy();

			TINT      iWidth;
			TINT      iHeight;
			TINT      iNumComponents;
			stbi_uc* pImage = stbi_load( strPath.GetString( 0 ), &iWidth, &iHeight, &iNumComponents, 4 );

			if ( pImage != TNULL )
			{
				m_TexInfo->SRView = pRender->CreateTexture( iWidth, iHeight, DXGI_FORMAT_R8G8B8A8_UNORM, pImage, 0, D3D11_USAGE_IMMUTABLE, 0, 1 );

				if ( m_TexInfo->SRView != TNULL )
					hResult = S_OK;

				stbi_image_free( pImage );
			}
		}
	}

	if ( FAILED( hResult ) )
	{
		// The texture of the TTRB
		hResult = DirectX::CreateDDSTextureFromMemory( pRender->GetDevice(), m_TexData, m_DataSize, &pResource, &m_TexInfo->SRView, 0, TNULL );

		if ( FAILED( hResult ) )
			hResult = CreateWICTextureFromMemory( pRender->GetDevice(), m_TexData, m_DataSize, &pResource, &m_TexInfo->SRView );
	}

	if ( pResource != TNULL )
		pResource->Release();

	if ( SUCCEEDED( hResult ) )
		TTextureManager::GetSingleton()->AddTexture( this );
	else
		TASSERT( TFALSE );

	SelectSamplerId();
}

// $deBlob: FUNCTION 006938e0
void TTexture::Deinit()
{
	if ( m_TexInfo->SRView != TNULL )
	{
		m_TexInfo->SRView->Release();
		m_TexInfo->SRView = TNULL;
	}

	TTextureManager::GetSingleton()->RemoveTexture( this );
}

// $deBlob: FUNCTION 00693940
void TTexture::CreateResource()
{
	m_Resource.CreateResource( m_TexName, this, ResourceCallback, this );
}

// $deBlob: FUNCTION 00693950
TBOOL TTexture::IsMultiBitAlpha()
{
	return m_TexInfo->Format == DXGI_FORMAT_R8G8B8A8_UNORM || m_TexInfo->Format == DXGI_FORMAT_BC3_UNORM;
}

// $deBlob: FUNCTION 00693970
void TTexture::Bind( UINT a_uiSlot )
{
	TRenderDX11* pRender = TRenderDX11::Interface();
	pRender->GetDeviceContext()->PSSetShaderResources( a_uiSlot, 1, &m_TexInfo->SRView );
	pRender->SetSamplerState( a_uiSlot, m_SamplerId, TRUE );
}

// $deBlob: FUNCTION 006939b0
void TTexture::SetWrap( D3D11_TEXTURE_ADDRESS_MODE a_eAddressU, D3D11_TEXTURE_ADDRESS_MODE a_eAddressV )
{
	m_eAddressU = a_eAddressU;
	m_eAddressV = a_eAddressV;
	SelectSamplerId();
}

// $deBlob: FUNCTION 006939d0
void TTexture::SetFilter( D3D11_FILTER a_eFilter )
{
	m_eFilter = a_eFilter;
	SelectSamplerId();
}

// $deBlob: FUNCTION 006939f0
void* __stdcall TTexture::ResourceCallback( void* a_pCustomData, TTRB* a_pTRB, TBOOL a_bCreated )
{
	return a_bCreated ? a_pCustomData : TNULL;
}

// $deBlob: FUNCTION 00693a00
void TTexture::SelectSamplerId()
{
	if ( m_eAddressU == D3D11_TEXTURE_ADDRESS_CLAMP && m_eAddressV == D3D11_TEXTURE_ADDRESS_CLAMP )
	{
		switch ( m_eFilter )
		{
			case D3D11_FILTER_MIN_MAG_MIP_POINT:
				m_SamplerId = TRenderDX11::SAMPLER_POINT_CLAMP;
				break;
			case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
			case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
				m_SamplerId = TRenderDX11::SAMPLER_LINEAR_CLAMP;
				break;
			case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
				m_SamplerId = TRenderDX11::SAMPLER_BILINEAR_CLAMP;
				break;
			default:
				TASSERT( TFALSE );
				break;
		}
	}
	else if ( m_eAddressU == D3D11_TEXTURE_ADDRESS_WRAP && m_eAddressV == D3D11_TEXTURE_ADDRESS_WRAP )
	{
		if ( m_eFilter == D3D11_FILTER_MIN_MAG_MIP_POINT )
			m_SamplerId = TRenderDX11::SAMPLER_POINT_WRAP;
		else if ( m_eFilter == D3D11_FILTER_MIN_MAG_MIP_LINEAR )
			m_SamplerId = TRenderDX11::SAMPLER_LINEAR_WRAP;
		else if ( m_eFilter == D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT )
			m_SamplerId = TRenderDX11::SAMPLER_BILINEAR_WRAP;
		else
			TASSERT( TFALSE );
	}
	else if ( m_eAddressU == D3D11_TEXTURE_ADDRESS_CLAMP && m_eAddressV == D3D11_TEXTURE_ADDRESS_WRAP )
	{
		if ( m_eFilter == D3D11_FILTER_MIN_MAG_MIP_POINT )
			m_SamplerId = TRenderDX11::SAMPLER_POINT_WRAP_CLAMPV;
		else if ( m_eFilter == D3D11_FILTER_MIN_MAG_MIP_LINEAR )
			m_SamplerId = TRenderDX11::SAMPLER_LINEAR_WRAP_CLAMPV;
		else if ( m_eFilter == D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT )
			m_SamplerId = TRenderDX11::SAMPLER_BILINEAR_WRAP_CLAMPV;
		else
			TASSERT( TFALSE );
	}
	else
	{
		TASSERT( TFALSE );

		// NOTE: retail resets to clamp/clamp linear but picks the wrap/wrap linear sampler
		m_eAddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_eAddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_eFilter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		m_SamplerId = TRenderDX11::SAMPLER_LINEAR_WRAP;
	}
}

// $deBlob: FUNCTION 00693b90
TTextureManager::TTextureManager()
{
	m_pLastTexture = TNULL;

	TASSERT( TNULL == s_pSingleton );
	s_pSingleton = this;

	TRenderDX11* pRender = TRenderDX11::Interface();

	// White texture: 8x8 R8G8B8A8, 0xFFFFFFFF
	TCONSTEXPR_VAR TUINT32 WHITE_SIZE   = 8;
	TUINT32*          pWhitePixels = TSTATICCAST( TUINT32, TMemalign( 16, WHITE_SIZE * WHITE_SIZE * 4 ) );

	for ( TUINT32 i = 0; i < WHITE_SIZE * WHITE_SIZE; i++ )
		pWhitePixels[ i ] = 0xFFFFFFFF;

	TTexture::Info* pWhiteInfo = new TTexture::Info;
	pWhiteInfo->SRView         = pRender->CreateTexture( WHITE_SIZE, WHITE_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, pWhitePixels, 0, D3D11_USAGE_IMMUTABLE, 0, 1 );
	pWhiteInfo->Width          = WHITE_SIZE;
	pWhiteInfo->Height         = WHITE_SIZE;
	pWhiteInfo->MipMapCount    = 1;
	pWhiteInfo->Format         = DXGI_FORMAT_R8G8B8A8_UNORM;

	m_pWhiteTexture                 = CreateTexture( pWhiteInfo );
	m_pWhiteTexture->m_TexName      = g_WhiteTexName;
	m_pWhiteTexture->m_pPrevTexture = TNULL;
	m_pWhiteTexture->m_pNextTexture = TNULL;
	TFree( pWhitePixels );

	// Invalid texture: 128x128 R8G8B8A8, 16x16 checkers of random colours and black
	TCONSTEXPR_VAR TUINT32 INVALID_SIZE   = 128;
	TCONSTEXPR_VAR TUINT32 INVALID_BYTES  = INVALID_SIZE * INVALID_SIZE * 4;
	TUINT8*           pInvalidPixels = TSTATICCAST( TUINT8, TMemalign( 16, INVALID_BYTES ) );

	for ( TUINT32 i = 0; i < INVALID_BYTES; i += 4 )
	{
		if ( ( TUINT8( i ) ^ TUINT8( i >> 7 ) ) & 0x40 )
		{
			pInvalidPixels[ i + 0 ] = rand() % 255;
			pInvalidPixels[ i + 1 ] = rand() % 255;
			pInvalidPixels[ i + 2 ] = rand() % 255;
			pInvalidPixels[ i + 3 ] = rand() % 255;
		}
		else
		{
			*TREINTERPRETCAST( TUINT32*, &pInvalidPixels[ i ] ) = 0xFF000000;
		}
	}

	TTexture::Info* pInvalidInfo = new TTexture::Info;
	pInvalidInfo->SRView         = pRender->CreateTexture( INVALID_SIZE, INVALID_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, pInvalidPixels, 0, D3D11_USAGE_IMMUTABLE, 0, 1 );
	pInvalidInfo->Width          = INVALID_SIZE;
	pInvalidInfo->Height         = INVALID_SIZE;
	pInvalidInfo->MipMapCount    = 1;
	pInvalidInfo->Format         = DXGI_FORMAT_R8G8B8A8_UNORM;

	m_pInvalidTexture                 = CreateTexture( pInvalidInfo );
	m_pInvalidTexture->m_TexName      = g_InvalidTexName;
	m_pInvalidTexture->m_pPrevTexture = TNULL;
	m_pInvalidTexture->m_pNextTexture = TNULL;
	TFree( pInvalidPixels );
}

// $deBlob: FUNCTION 00693ee0
// $deBlob: FUNCTION 006aa950
TTextureManager::~TTextureManager()
{
	TASSERT( TNULL != s_pSingleton );
	s_pSingleton = TNULL;

	TTexture* aTextures[ 2 ] = { m_pInvalidTexture, m_pWhiteTexture };

	for ( TUINT i = 0; i < 2; i++ )
	{
		TTexture* pTexture = aTextures[ i ];
		pTexture->m_TexInfo->SRView->Release();
		RemoveTexture( pTexture );
		delete pTexture->m_TexInfo;

		T2ResourceManager::GetSingleton()->DestroyResource( pTexture->m_Resource );

		delete pTexture;
	}
}

// $deBlob: FUNCTION 00693fc0
TTexture* TTextureManager::CreateTexture( TTexture::Info* a_pInfo )
{
	TTexture* pTexture  = new TTexture;
	pTexture->m_TexName = "runtime";
	pTexture->m_TexInfo = a_pInfo;
	pTexture->m_TexData = TNULL;
	AddTexture( pTexture );

	return pTexture;
}

// $deBlob: FUNCTION 00694020
void TTextureManager::DestroyTexture( TTexture* a_pTexture )
{
	TASSERT( TNULL == a_pTexture->m_TexData );
	RemoveTexture( a_pTexture );

	T2ResourceManager::GetSingleton()->DestroyResource( a_pTexture->m_Resource );

	delete a_pTexture;
}

// $deBlob: FUNCTION 006940b0
TTexture* TTextureManager::FindTexture( const TCHAR* a_szName )
{
	TASSERT( T2String8::IsLowerCase( a_szName ) );

	for ( TTexture* pTexture = m_pLastTexture; pTexture != TNULL; pTexture = pTexture->m_pPrevTexture )
	{
		if ( T2String8::CompareNoCase( pTexture->m_TexName, a_szName ) == 0 )
			return pTexture;
	}

	return m_pInvalidTexture;
}

// $deBlob: FUNCTION 00694130
void TTextureManager::AddTexture( TTexture* a_pTexture )
{
	a_pTexture->m_pPrevTexture = m_pLastTexture;
	a_pTexture->m_pNextTexture = TNULL;

	if ( m_pLastTexture != TNULL )
		m_pLastTexture->m_pNextTexture = a_pTexture;

	m_pLastTexture = a_pTexture;
}

// $deBlob: FUNCTION 00694160
void TTextureManager::RemoveTexture( TTexture* a_pTexture )
{
	if ( a_pTexture->m_pPrevTexture != TNULL )
		a_pTexture->m_pPrevTexture->m_pNextTexture = a_pTexture->m_pNextTexture;

	if ( a_pTexture->m_pNextTexture != TNULL )
		a_pTexture->m_pNextTexture->m_pPrevTexture = a_pTexture->m_pPrevTexture;

	if ( a_pTexture == m_pLastTexture )
		m_pLastTexture = m_pLastTexture->m_pPrevTexture;

	a_pTexture->m_pPrevTexture = TNULL;
	a_pTexture->m_pNextTexture = TNULL;
}

TOSHI_NAMESPACE_END

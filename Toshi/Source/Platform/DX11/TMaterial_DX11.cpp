#include "ToshiPCH.h"
#include "TMaterial_DX11.h"
#include "TTexture_DX11.h"
#include "TRender_DX11.h"
#include "Toshi/TError.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2Material::t_TRBLoadCallback   T2Material::sm_pTRBLoadCallback       = TNULL;
void*                           T2Material::sm_pTRBLoadCallbackData   = TNULL;
T2Material::t_TRBUnloadCallback T2Material::sm_pTRBUnloadCallback     = TNULL;
void*                           T2Material::sm_pTRBUnloadCallbackData = TNULL;
TMaterialManager*               TMaterialManager::s_pSingleton        = TNULL;

// $deBlob: FUNCTION 006b1cb0
void T2Material::Texture::Init( T2Material* a_pMaterial )
{
	if ( m_pName )
	{
		TTextureManager* pTexManager = TTextureManager::GetSingleton();
		m_pTexture                   = pTexManager->FindTexture( m_pName );

		if ( m_pTexture == pTexManager->GetInvalidTexture() )
		{
			m_pTexture = TNULL;
			TError::GetSingleton()->AddError( "Material '%s' Couldnt find texture '%s'", a_pMaterial->m_szName, m_pName );
		}
	}
}

// $deBlob: FUNCTION 006b1d00
void T2Material::Init()
{
	InitData();
	CreateResource( m_szName, this, ResourceCallback, this );
	TASSERT( !HasFlag( FLAGS_INMATERIALMANAGER ) );

	TMaterialManager* pMatManager = TMaterialManager::GetSingleton();

	if ( pMatManager->FindMaterial( m_szName ) == TNULL )
	{
		m_uiFlags |= FLAGS_INMATERIALMANAGER;
		pMatManager->AddMaterial( this );
	}

	if ( sm_pTRBLoadCallback )
		sm_pTRBLoadCallback( this, sm_pTRBLoadCallbackData );
}

// $deBlob: FUNCTION 006b1da0
void T2Material::InitResource()
{
	InitData();
}

// $deBlob: FUNCTION 006b1db0
void T2Material::Deinit()
{
	if ( sm_pTRBUnloadCallback )
		sm_pTRBUnloadCallback( this, sm_pTRBUnloadCallbackData );

	if ( HasFlag( FLAGS_INMATERIALMANAGER ) )
	{
		m_uiFlags &= ~FLAGS_INMATERIALMANAGER;
		TMaterialManager::GetSingleton()->RemoveMaterial( this );
	}

	for ( TINT i = 0; i < m_iNumTex; i++ )
		m_aTextures[ i ].m_pTexture = TNULL;

	for ( TINT i = 0; i < m_iNumPasses; i++ )
		m_aPasses[ i ].m_pTexture->m_pTexture = TNULL;

	DestroyResource();
}

// $deBlob: FUNCTION 005f99c0
TTexture* T2Material::GetTexture( TUINT a_iId ) const
{
	TASSERT( a_iId < TUINT( m_iNumTex ) );
	return m_aTextures[ a_iId ].m_pTexture;
}

// $deBlob: FUNCTION 006b1e20
const TCHAR* T2Material::GetPassTextureName( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_pTexture->m_pName;
}

// $deBlob: FUNCTION 006b1e80
T2Material::Pass* T2Material::GetPassInfo( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return &m_aPasses[ a_iPass ];
}

// $deBlob: FUNCTION 006b1ed0
T2Material::Texture* T2Material::GetPassTexture( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_pTexture;
}

// $deBlob: FUNCTION 006b1f20
TUINT32 T2Material::GetPassBlendMode( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_uiBlendMode;
}

// $deBlob: FUNCTION 005f9950
TUINT32 T2Material::GetPassTevMode( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_uiTevMode;
}

// $deBlob: FUNCTION 006b1f70
TUINT32 T2Material::GetPassUVFunctionType( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_uiUVFunctionType;
}

// $deBlob: FUNCTION 006b1fc0
TUINT32 T2Material::GetPassUFunction( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_uiUFunction;
}

// $deBlob: FUNCTION 006b2010
TUINT32 T2Material::GetPassVFunction( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_uiVFunction;
}

// $deBlob: FUNCTION 006b2060
TFLOAT T2Material::GetPassUParam1( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_fUParam1;
}

// $deBlob: FUNCTION 006b20b0
TFLOAT T2Material::GetPassUParam2( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_fUParam2;
}

// $deBlob: FUNCTION 006b2100
TFLOAT T2Material::GetPassVParam1( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_fVParam1;
}

// $deBlob: FUNCTION 006b2150
TFLOAT T2Material::GetPassVParam2( TINT a_iPass ) const
{
	TASSERT( a_iPass < m_iNumPasses );
	return m_aPasses[ a_iPass ].m_fVParam2;
}

// $deBlob: FUNCTION 006b21a0
void T2Material::InitData()
{
	TTextureManager* pTexManager = TTextureManager::GetSingleton();

	for ( TINT i = 0; i < m_iNumTex; i++ )
		m_aTextures[ i ].m_pTexture = pTexManager->FindTexture( m_aTextures[ i ].m_pName );

	for ( TINT i = 0; i < m_iNumPasses; i++ )
	{
		Texture*     pPassTexture = GetPassTexture( i );
		const TCHAR* szName       = GetPassTextureName( i );

		pPassTexture->m_pTexture = pTexManager->FindTexture( szName );

		if ( pPassTexture->m_pTexture == pTexManager->GetInvalidTexture() )
			TError::GetSingleton()->AddError( "Material '%s' Couldn't find texture '%s'", m_szName, szName );
	}
}

// $deBlob: FUNCTION 006b22d0
void* __stdcall T2Material::ResourceCallback( void* a_pCustomData, TTRB* a_pTRB, TBOOL a_bCreated )
{
	if ( !a_bCreated )
		return TNULL;

	TRenderDX11::Interface()->WaitForEndOfRender();
	T2Material* pMaterial = TSTATICCAST( T2Material, a_pTRB->GetSymbolAddress( "tmat" ) );

	if ( pMaterial == TNULL )
		return a_pCustomData;

	pMaterial->InitData();
	return pMaterial;
}

// $deBlob: FUNCTION 006b2320
TMaterialManager::TMaterialManager()
{
	m_pHeadMaterial = TNULL;
	TASSERT( TNULL == s_pSingleton );
	s_pSingleton = this;
}

// $deBlob: FUNCTION 006b2370
// $deBlob: FUNCTION 006aa890
TMaterialManager::~TMaterialManager()
{
	TASSERT( TNULL != s_pSingleton );
	s_pSingleton = TNULL;
}

// $deBlob: FUNCTION 006b2450
void TMaterialManager::AddMaterial( T2Material* a_pMat )
{
	a_pMat->m_pNext = m_pHeadMaterial;
	a_pMat->m_pPrev = TNULL;

	if ( m_pHeadMaterial )
		m_pHeadMaterial->m_pPrev = a_pMat;

	m_pHeadMaterial = a_pMat;
}

// $deBlob: FUNCTION 006b2480
void TMaterialManager::RemoveMaterial( T2Material* a_pMat )
{
	TASSERT( TNULL != a_pMat );

	if ( a_pMat->m_pNext )
		a_pMat->m_pNext->m_pPrev = a_pMat->m_pPrev;

	if ( a_pMat->m_pPrev )
		a_pMat->m_pPrev->m_pNext = a_pMat->m_pNext;

	if ( a_pMat == m_pHeadMaterial )
		m_pHeadMaterial = m_pHeadMaterial->m_pNext;

	a_pMat->m_pPrev = TNULL;
	a_pMat->m_pNext = TNULL;
}

// $deBlob: FUNCTION 006b23c0
T2Material* TMaterialManager::FindMaterial( const TCHAR* a_pName ) const
{
	TASSERT( T2String8::IsLowerCase( a_pName ) );

	for ( T2Material* pMaterial = m_pHeadMaterial; pMaterial != TNULL; pMaterial = pMaterial->m_pNext )
	{
		if ( T2String8::CompareNoCase( pMaterial->m_szName, a_pName ) == 0 )
			return pMaterial;
	}

	return TNULL;
}

TOSHI_NAMESPACE_END

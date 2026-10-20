#include "ToshiPCH.h"
#include "TParticleMaterial.h"
#include "Render/TMaterial.h"
#include "Shaders/SysShader/TSysShader.h"
#ifdef TRENDERINTERFACE_DX11
#  include "Platform/DX11/TTexture_DX11.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0071bcb0
void TParticleMaterial::Create()
{
	TASSERT( m_uiVer == s_LatestVer );

	TCHAR szTexture[ 128 ];
	TStringManager::String8CopySafe( szTexture, m_szTexture, sizeof( szTexture ) );
	TStringManager::String8ToLowerCase( szTexture );

	m_pTexture = TTextureManager::GetSingleton()->FindTexture( szTexture );

	if ( m_pTexture == TNULL )
		m_pTexture = TTextureManager::GetSingleton()->GetInvalidTexture();

	TSysMaterial* pMaterial = TSysShader::GetSingleton()->CreateMaterial( m_szName );
	m_pMaterial             = pMaterial;
	pMaterial->SetTexture( 0, m_pTexture ); // inlined
	pMaterial->SetTextureNum( 1 );          // inlined

	switch ( m_iBlendMode )
	{
		case BLENDMODE_NORMAL:
			pMaterial->Create( TSysMaterial::BLENDMODE_1 );
			break;
		case BLENDMODE_ADDITIVE:
			pMaterial->Create( TSysMaterial::BLENDMODE_ADDITIVE );
			break;
		case BLENDMODE_2:
			pMaterial->Create( TSysMaterial::BLENDMODE_4 );
			break;
	}

	pMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );

	if ( m_uiFlags & FLAGS_ALPHATEXTURE )
		pMaterial->SetAlphaTexture();

	if ( m_uiFlags & FLAGS_NODEPTHTEST )
		pMaterial->SetFlags( TMaterial::FLAGS_NO_DEPTH_TEST, TTRUE );

	if ( m_uiFlags & FLAGS_4 )
		pMaterial->SetFlags( TMaterial::FLAGS_ALPHAUPDATE, TTRUE );
	else if ( m_uiFlags & FLAGS_8 )
		pMaterial->SetFlags( 0x40000000, TTRUE );
	else if ( m_uiFlags & FLAGS_10 )
		pMaterial->SetFlags( 0x20000000, TTRUE );
}

// $deBlob: FUNCTION 0071bdf0
void TParticleMaterial::Destroy()
{
	TMaterial* pMaterial = m_pMaterial;
	m_pTexture           = TNULL;

	if ( pMaterial != TNULL )
		pMaterial->Destroy(); // OnDestroy and delete
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TMaterial.h"
#include "TOrderTable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006922b0
// $deBlob: FUNCTION 006922c0
// $deBlob: FUNCTION 006922d0
// $deBlob: FUNCTION 006922e0
// $deBlob: FUNCTION 006922f0
// $deBlob: FUNCTION 0053eb60
// $deBlob: FUNCTION 00858ec0
TDEFINE_CLASS_NORUNTIME( TMaterial );

// $deBlob: FUNCTION 00692350
// $Barnyard: FUNCTION 006ccc60
TMaterial::TMaterial()
{
	m_Flags        = FLAGS_NONE;
	m_pShader      = TNULL;
	m_iNumTex      = 0;
	m_pUVAnimation = TNULL;
	TUtil::MemClear( m_pTextures, sizeof( m_pTextures ) );
	m_pRegMaterial  = TNULL;
	m_szName[ 0 ]   = '\0';
	m_pMetaMaterial = TNULL;
}

// $deBlob: FUNCTION 00692470
// $deBlob: FUNCTION 00692540
// $Barnyard: FUNCTION 006ccca0
// $Barnyard: FUNCTION 006ccdc0
TMaterial::~TMaterial()
{
	TUtil::MemClear( m_pTextures, sizeof( m_pTextures ) );
}

// $deBlob: FUNCTION 00692300
// $Barnyard: FUNCTION 006cccc0
void TMaterial::OnDestroy()
{
	// NOTE: Barnyard has no assert here
	TASSERT( TTRUE == IsCreated() );
	m_Flags &= ~FLAGS_CREATED;
	m_pUVAnimation = TNULL;
}

// $deBlob: FUNCTION 006923c0
// $Barnyard: FUNCTION 006cccb0
TBOOL TMaterial::Create()
{
	TASSERT( TFALSE == IsCreated() );
	m_Flags |= FLAGS_CREATED;
	return TTRUE;
}

// $deBlob: FUNCTION 0054f580
void TMaterial::PreRender()
{
}

// $deBlob: FUNCTION 0054f570
void TMaterial::PostRender()
{
}

void TMaterial::Destroy()
{
	OnDestroy();
	delete this;
}

// $deBlob: FUNCTION 00567830
// $deBlob: FUNCTION 00567880
// $deBlob: FUNCTION 005696f0
// $deBlob: FUNCTION 005fbba0
// $deBlob: FUNCTION 00705e30
// $deBlob: FUNCTION 0070a830
void TMaterial::SetShader( TShader* pShader )
{
	TASSERT( TNULL == m_pShader );
	m_pShader = pShader;
}

TShader* TMaterial::GetShader() const
{
	return m_pShader;
}

// $deBlob: FUNCTION 0054f5a0
void TMaterial::SetTextureNum( TUINT32 a_iNumTextures )
{
	TASSERT( a_iNumTextures <= MAXTEXTURES );
	m_iNumTex = a_iNumTextures;
}

// $deBlob: FUNCTION 0055e410
void TMaterial::SetTexture( TUINT32 a_iStage, TTexture* a_pTexture )
{
	TASSERT( a_iStage < MAXTEXTURES );
	m_pTextures[ a_iStage ] = a_pTexture;
}

TUINT32 TMaterial::GetTextureNum() const
{
	return m_iNumTex;
}

// $deBlob: FUNCTION 0055dfc0
TTexture* TMaterial::GetTexture( TUINT32 a_iStage /*= 0*/ ) const
{
	TASSERT( a_iStage < MAXTEXTURES );
	return m_pTextures[ a_iStage ];
}

// $deBlob: FUNCTION 00692440
// $Barnyard: FUNCTION 006cccd0
void TMaterial::SetName( const TCHAR* a_szName )
{
	// NOTE: Barnyard also logs names that are too long
	if ( a_szName == TNULL )
		m_szName[ 0 ] = '\0';
	else
		T2String8::Copy( m_szName, a_szName, NAMESIZELIMIT );
}

// $deBlob: FUNCTION 00692410
// $Barnyard: FUNCTION 006ccd10
void TMaterial::SetFlags( Flags a_eFlag, TBOOL a_bEnable )
{
	if ( a_bEnable )
	{
		// Only one of the exclusive flags is set at a time (Barnyard just sets the bits)
		if ( a_eFlag & FLAGS_EXCLUSIVE )
			m_Flags &= ~FLAGS_EXCLUSIVE;

		m_Flags |= a_eFlag;
	}
	else
	{
		m_Flags &= ~a_eFlag;
	}
}

TBOOL TMaterial::IsCreated() const
{
	return m_Flags & FLAGS_CREATED;
}

TRenderPacket* TMaterial::AddRenderPacket( TMesh* a_pMesh )
{
	TVALIDPTR( a_pMesh );
	TVALIDPTR( m_pRegMaterial );
	return m_pRegMaterial->AddRenderPacket( a_pMesh );
}

void TMaterial::SetRegMaterial( TRegMaterial* pRegMaterial )
{
	m_pRegMaterial = pRegMaterial;
}

TRegMaterial* TMaterial::GetRegMaterial() const
{
	return m_pRegMaterial;
}

TBOOL TMaterial::IsBlending() const
{
	return m_Flags & FLAGS_BLENDING;
}

TOSHI_NAMESPACE_END

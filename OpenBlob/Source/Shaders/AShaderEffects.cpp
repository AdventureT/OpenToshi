#include "pch.h"
#include "AShaderEffects.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005fffa0
AShaderEffects::AShaderEffects()
{
	m_paClones = new Clone[ MAX_CLONES ];
	for ( TINT i = 0; i < MAX_CLONES; i++ )
	{
		m_paClones[ i ].m_iIndex = TINT16( i );
		m_oFreeClones.PushBack( &m_paClones[ i ] );
	}
}

// $deBlob: FUNCTION 006000b0
// $deBlob: FUNCTION 006008e0
AShaderEffects::~AShaderEffects()
{
	m_oActiveClones.Clear();
	m_oFreeClones.Clear();
	delete[] m_paClones;
	m_paClones = TNULL;
}

// $deBlob: FUNCTION 006001a0
void AShaderEffects::Update( TFLOAT a_fDelta )
{
	// NOTE: retail's step is 2.0f * dt
	TFLOAT fStep = 2.0f * a_fDelta;

	TWorldShaderHAL* pWorldShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );

	Toshi::T2DList<Clone>::Iterator it = m_oActiveClones.Begin();
	while ( it != m_oActiveClones.End() )
	{
		// the next copy is read before this one can be released
		Clone* pClone = it.Get();
		it++;

		pClone->m_fTime -= fStep;
		if ( pClone->m_fTime >= 0.0f )
		{
			// the time as a colour: the byte in all four channels
			TUINT8  uiByte   = TUINT8( TINT( pClone->m_fTime * 255.0f ) );
			TUINT32 uiColour = TUINT32( uiByte ) * 0x01010101;
			pWorldShader->SetCrossfade( pClone->m_iIndex, uiColour );
		}
		else
		{
			if ( pClone->m_pCallback != TNULL )
				pClone->m_pCallback( pClone, pClone->m_pOwner );

			ReleaseClone( pClone, 1 );
		}
	}
}

// $deBlob: FUNCTION 006002e0
void AShaderEffects::ReleaseAll()
{
	while ( !m_oActiveClones.IsEmpty() )
		ReleaseClone( m_oActiveClones.Tail().Get(), 0 );
}

// $deBlob: FUNCTION 00600340
AShaderEffects::Clone* AShaderEffects::AddClone( TWorldMaterialHAL* a_pMaterial, Toshi::TTexture* a_pTexture, TINT16 a_iStage, TBOOL a_bFlag, t_CloneCallback a_pCallback, void* a_pOwner )
{
	if ( m_oFreeClones.IsEmpty() )
		return TNULL;

	// the pass the texture goes to (the material's texture count)
	TUINT32 uiPass = a_pMaterial->GetNumTextures();

	TASSERT( a_pMaterial->IsA( &TGetClass( TWorldMaterialHAL ) ) );

	if ( a_bFlag )
	{
		if ( uiPass != 2 || TINT( a_pMaterial->GetMultiPassBlendType( 0 ) ) != 19 )
			return TNULL;
		if ( TINT( a_pMaterial->GetMultiPassBlendType( 1 ) ) != 1 )
			return TNULL;
	}
	else
	{
		if ( uiPass != 1 )
			return TNULL;
		if ( TINT( a_pMaterial->GetMultiPassBlendType( 0 ) ) != 0 )
			return TNULL;
	}

	Clone* pClone = m_oFreeClones.Tail().Get();
	pClone->Remove();

	pClone->m_iStage    = a_iStage;
	pClone->m_pCallback = a_pCallback;
	pClone->m_pMaterial = a_pMaterial;
	pClone->m_fTime     = 1.0f;
	pClone->m_bFlag     = TUINT8( a_bFlag );
	pClone->m_pOwner    = a_pOwner;
	pClone->m_pNextCopy = TNULL;

	if ( a_bFlag )
	{
		a_pMaterial->SetMultiPassBlendType( uiPass - 1, TWorldMaterialHAL::MULTIPASSBLENDTYPE( 24 ) );
		a_pMaterial->SetMultiPassBlendType( uiPass, TWorldMaterialHAL::MULTIPASSBLENDTYPE( 25 ) );
	}
	else
	{
		a_pMaterial->SetMultiPassBlendType( uiPass, TWorldMaterialHAL::MULTIPASSBLENDTYPE( 23 ) );
	}

	a_pMaterial->SetTexture( uiPass, a_pTexture );
	a_pMaterial->SetUVStream( uiPass, 0 );
	a_pMaterial->SetUVMode( uiPass, TWorldMaterialHAL::UVMODE_STREAM );
	a_pMaterial->SetTextureNum( uiPass + 1 );
	a_pMaterial->SetUserDataIndex( TWorldMaterialHAL::UData_2, pClone->m_iIndex );

	TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() )->SetCrossfade( pClone->m_iIndex, 0xFFFFFFFF );

	m_oActiveClones.PushBack( pClone );
	return pClone;
}

// $deBlob: FUNCTION 006004e0
void AShaderEffects::ReleaseClone( Clone* a_pClone, TINT a_iUnused )
{
	if ( a_pClone->Next() == a_pClone )
		return;

	TWorldMaterialHAL* pMaterial = a_pClone->m_pMaterial;

	// the last texture of the material moves to the copy's stage, the last stage is cleared
	TUINT32 uiLast = pMaterial->GetNumTextures() - 1;
	TASSERT( uiLast < TMaterial::MAXTEXTURES );

	Toshi::TTexture* pTexture = pMaterial->GetTexture( uiLast );

	TUINT32 uiStage = TUINT32( TINT( a_pClone->m_iStage ) );
	TASSERT( uiStage < TMaterial::MAXTEXTURES );
	pMaterial->SetTexture( uiStage, pTexture );

	TASSERT( uiLast < TMaterial::MAXTEXTURES );
	pMaterial->SetTexture( uiLast, TNULL );

	if ( a_pClone->m_bFlag )
		pMaterial->SetMultiPassBlendType( uiStage, TWorldMaterialHAL::MULTIPASSBLENDTYPE( 1 ) );

	TASSERT( uiLast <= TMaterial::MAXTEXTURES );
	pMaterial->SetTextureNum( uiLast );

	pMaterial->SetUserDataIndex( TWorldMaterialHAL::UData_2, -1 );

	a_pClone->m_pNextCopy = TNULL;
	a_pClone->Remove();
	m_oFreeClones.PushBack( a_pClone );
}

#include "ToshiPCH.h"
#include "T2GUI.h"
#include "T2GUITextureSectionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TTRB*                                  T2GUITextureSectionManager::g_trb      = TNULL;
T2GUITextureSectionManager::TRBHeader* T2GUITextureSectionManager::g_defblock = TNULL;
T2GUITextureSection*                   T2GUITextureSectionManager::g_sections = TNULL;

// $deBlob: FUNCTION 006f5b70
// $Barnyard: FUNCTION 006bfb20
TBOOL T2GUITextureSectionManager::Open( const TCHAR* a_szFileName )
{
	TASSERT( g_trb == TNULL );
	g_trb = new TTRB();

	if ( g_trb->Load( a_szFileName, 0 ) != TTRB::ERROR_OK )
	{
		TASSERT( 0 && "T2GUITextureSectionManager::Open: Failed to load trb" );
		return TFALSE;
	}

	g_defblock = TSTATICCAST( TRBHeader, g_trb->GetSymbolAddress( "texturesections" ) );
	g_sections = g_defblock->m_pSections;
	CreateMaterials();

	return TTRUE;
}

// $deBlob: FUNCTION 006f5c30
// $Barnyard: FUNCTION 006bfc40
void T2GUITextureSectionManager::Close()
{
	if ( g_trb )
	{
		DestroyMaterials();
		g_sections = TNULL;
		g_defblock = TNULL;
		g_trb->Close();
		delete g_trb;
		g_trb = TNULL;
	}
}

// $deBlob: FUNCTION 006f5c90
// $Barnyard: FUNCTION 006bf890
T2GUITextureSection* T2GUITextureSectionManager::GetTextureSection( const TCHAR* a_szSectionName )
{
	if ( g_sections == TNULL )
		return TNULL;

	for ( TINT i = 0; i < TINT( g_defblock->m_uiNumSections ); i++ )
	{
		T2GUITextureSection* pSection = &g_sections[ i ];

		if ( TStringManager::String8Compare( a_szSectionName, pSection->GetName(), -1 ) == 0 )
			return pSection;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006f5ce0
// $Barnyard: FUNCTION 006bf8e0
void T2GUITextureSectionManager::CreateMaterials()
{
	for ( TINT i = 0; i < TINT( g_defblock->m_uiNumSections ); i++ )
	{
		T2GUITextureSection* pSection = &g_sections[ i ];

		if ( pSection->GetMaterial() == TNULL )
		{
			if ( TTexture* pTexture = T2GUI::GetTexture( pSection->GetTextureFile() ) )
			{
				T2GUIMaterial* pMaterial = T2GUI::CreateMaterial( pTexture );
				pSection->SetMaterial( pMaterial );

				TUINT uiWidth  = T2GUI::GetWidth( pMaterial );
				TUINT uiHeight = T2GUI::GetHeight( pSection->GetMaterial() );

				pSection->CalculateUV( TFLOAT( TINT( uiWidth ) ), TFLOAT( TINT( uiHeight ) ) );
			}
		}
	}
}

// $deBlob: FUNCTION 006f5e10
// $Barnyard: FUNCTION 006bf9c0
void T2GUITextureSectionManager::DestroyMaterials()
{
	for ( TINT i = 0; i < TINT( g_defblock->m_uiNumSections ); i++ )
	{
		T2GUITextureSection* pSection = &g_sections[ i ];

		if ( pSection->GetMaterial() != TNULL )
		{
			T2GUI::DestroyMaterial( pSection->GetMaterial() );
			pSection->SetMaterial( TNULL );
		}
	}
}

// $deBlob: FUNCTION 006f5e60
// $Barnyard: FUNCTION 006bfa10
void T2GUITextureSectionManager::UpdateMaterials()
{
	if ( g_sections != TNULL )
	{
		for ( TINT i = 0; i < TINT( g_defblock->m_uiNumSections ); i++ )
		{
			T2GUITextureSection* pSection = &g_sections[ i ];
			TTexture*            pTexture = T2GUI::GetTexture( pSection->GetTextureFile() );

			if ( pSection->GetMaterial() != TNULL )
			{
				if ( pTexture == TNULL )
				{
					T2GUI::DestroyMaterial( pSection->GetMaterial() );
					pSection->SetMaterial( TNULL );
				}
			}
			else if ( pTexture != TNULL )
			{
				T2GUIMaterial* pMaterial = T2GUI::CreateMaterial( pTexture );
				pSection->SetMaterial( pMaterial );

				TUINT uiWidth  = T2GUI::GetWidth( pMaterial );
				TUINT uiHeight = T2GUI::GetHeight( pSection->GetMaterial() );

				pSection->CalculateUV( TFLOAT( TINT( uiWidth ) ), TFLOAT( TINT( uiHeight ) ) );
			}
		}
	}
}

TOSHI_NAMESPACE_END

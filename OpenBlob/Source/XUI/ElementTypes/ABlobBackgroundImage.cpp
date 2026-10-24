#include "pch.h"
#include "ABlobBackgroundImage.h"
#include "Memory/AMemory.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIMaterial.h>
#include <GUI/T2GUIRenderer.h>
#include <Render/TAssetInit.h>
#include <Toshi/TStringManager.h>
#include <Toshi/T2String8.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIRenderer.h>

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0062b470
// $deBlob: FUNCTION 0062b480
// $deBlob: FUNCTION 0062b520
// $deBlob: FUNCTION 0062b5b0
// $deBlob: FUNCTION 0062b5c0
// $deBlob: FUNCTION 0053d190
// $deBlob: FUNCTION 008587e0
TDEFINE_CLASS( ABlobBackgroundImage );

// $deBlob: FUNCTION 0062bb00
ABlobBackgroundImage::ABlobBackgroundImage()
{
	m_bLoaded = TFALSE;
}

// $deBlob: FUNCTION 0062b5d0
// $deBlob: FUNCTION 0062bb73
// $deBlob: FUNCTION 0062bb80
ABlobBackgroundImage::~ABlobBackgroundImage()
{
	Unload();
}

// $deBlob: FUNCTION 0062b640
void ABlobBackgroundImage::Load( const TCHAR* a_szName )
{
	Unload();
	LoadMatLib( a_szName );

	TCHAR* szTextureName = TStringManager::GetTempString8();
	TINT   aiWidths[ NUM_COLUMNS ];
	TINT   aiHeights[ NUM_ROWS ];

	for ( TINT i = 0; i < NUM_TILES; i++ )
	{
		T2String8::Format( szTextureName, "%s_%d.tga", a_szName, i + 1 );

		m_aTiles[ i ].m_pMaterial = T2GUI::CreateMaterial( szTextureName );
		m_aTiles[ i ].m_pMaterial->SetBlendState( T2GUIMaterial::BLENDSTATE_ALPHA );

		TTexture* pTexture = m_aTiles[ i ].m_pMaterial->GetTextureResource();
		pTexture->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

		aiWidths[ TUINT( i ) % NUM_COLUMNS ]        = pTexture->GetWidth();
		aiHeights[ ( TUINT( i ) / NUM_COLUMNS ) % 2 ] = pTexture->GetHeight();
	}

	TFLOAT fTotalWidth  = TFLOAT( aiWidths[ 0 ] ) + 0.0f + TFLOAT( aiWidths[ 1 ] ) + TFLOAT( aiWidths[ 2 ] );
	TFLOAT fTotalHeight = TFLOAT( aiHeights[ 0 ] ) + 0.0f + TFLOAT( aiHeights[ 1 ] );
	TFLOAT fWidth       = GetWidth();
	TFLOAT fHeight      = GetHeight();

	Tile*  pTile = m_aTiles;
	TFLOAT fY    = 0.0f;

	for ( TINT iRow = 0; iRow < NUM_ROWS; iRow++ )
	{
		// NOTE: retail rounds with fistp (to nearest even)
		TFLOAT fTileHeight = TFLOAT( lrintf( ( TFLOAT( aiHeights[ iRow ] ) / fTotalHeight ) * fHeight ) );
		TFLOAT fX          = 0.0f;

		for ( TINT iColumn = 0; iColumn < NUM_COLUMNS; iColumn++ )
		{
			TFLOAT fTileWidth = TFLOAT( lrintf( ( TFLOAT( aiWidths[ iColumn ] ) / fTotalWidth ) * fWidth ) );

			pTile->m_fHeight = fTileHeight;
			pTile->m_fWidth  = fTileWidth;
			pTile->m_fY      = fY - fHeight * 0.5f;
			pTile->m_fX      = fX - fWidth * 0.5f;
			fX += fTileWidth;
			pTile++;
		}

		fY += fTileHeight;
	}

	m_bLoaded = TTRUE;
}

// $deBlob: FUNCTION 0062b860
void ABlobBackgroundImage::Unload()
{
	if ( m_bLoaded )
	{
		for ( TINT i = 0; i < NUM_TILES; i++ )
		{
			T2GUI::DestroyMaterial( m_aTiles[ i ].m_pMaterial );
			m_aTiles[ i ].m_pMaterial = TNULL;
		}

		TAssetInit::DeinitAssets( m_oTRB );
		m_oTRB.Close();
		m_bLoaded = TFALSE;
	}
}

// $deBlob: FUNCTION 0062b8c0
void ABlobBackgroundImage::Render()
{
	if ( m_bLoaded )
	{
		T2GUIRenderer* pRenderer = TXUI::GetSingleton()->GetRenderer();
		pRenderer->SetColour( m_uiColour );

		// The bottom row's textures are sampled a bit short of their bottom edge
		static TVector2 s_vUV2( 1.0f, 1.0f );

		for ( TINT i = 0; i < NUM_TILES; i++ )
		{
			s_vUV2.y = 0.875f;

			if ( i <= 2 )
				s_vUV2.y = 1.0f;

			Tile& rTile = m_aTiles[ i ];
			pRenderer->SetMaterial( rTile.m_pMaterial );

			TVector2 vMax( rTile.m_fWidth + rTile.m_fX, rTile.m_fHeight + rTile.m_fY );
			pRenderer->RenderRectangle( *TREINTERPRETCAST( TVector2*, &rTile.m_fX ), vMax, TVector2::VEC_ZERO, s_vUV2 );
		}
	}

	TXUIControl::Render();
}

// $deBlob: FUNCTION 0062b9a0
void ABlobBackgroundImage::LoadMatLib( const TCHAR* a_szName )
{
	m_oTRB.SetMemoryFunctions( AMemory::TemporaryTRBAllocator, AMemory::TemporaryTRBDeallocator, TNULL );

	TString8 strName( a_szName );

	TCHAR szPath[ 256 ];
	TStringManager::String8Format( szPath, sizeof( szPath ), "data/matlibs/%s.trb", strName.GetString() );

	TTRB::ERROR eRet = m_oTRB.Load( szPath );
	TASSERT( TTRB::ERROR_OK == eRet );

	TAssetInit::InitAssets( m_oTRB, TTRUE, TFALSE );
}

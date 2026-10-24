#include "pch.h"
#include "A2GUIBackground.h"
#include "Memory/AMemory.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIPolygon.h>
#include <GUI/T2GUIMaterial.h>
#include <Render/TAssetInit.h>
#include <File/TTRB.h>
#include <Toshi/TString8.h>

#ifdef TRENDERINTERFACE_DX11
#  include <Platform/DX11/TTexture_DX11.h>
#  include <Platform/DX11/TPrimShader_DX11.h>
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TFLOAT A2GUIBackground::s_fOffsetX = 0.0f;
TFLOAT A2GUIBackground::s_fOffsetY = 0.0f;

// $deBlob: FUNCTION 005d5810
A2GUIBackground::A2GUIBackground()
{
	m_pTRB = TNULL;
	TUtil::MemClear( m_apPoly, sizeof( m_apPoly ) );
}

// $deBlob: FUNCTION 005d5870
// $deBlob: FUNCTION 005d5f10
A2GUIBackground::~A2GUIBackground()
{
	Destroy();
}

// $deBlob: FUNCTION 005d58d0
void A2GUIBackground::Create( const TCHAR* a_szFileName, TINT a_iUnused )
{
	Destroy();
	TStringManager::String8Copy( m_szFileName, a_szFileName, MAX_FILENAME_LENGTH );
	LoadMaterialLibrary( a_szFileName );
	SetupBackground();
}

// $deBlob: FUNCTION 005d5910
void A2GUIBackground::Destroy()
{
	for ( TINT i = 0; i < MAX_TILES; i++ )
	{
		if ( m_apPoly[ i ] != TNULL )
		{
			T2GUI::DestroyMaterial( m_apPoly[ i ]->GetMaterial() );
			m_apPoly[ i ]->RemoveSelf();

			if ( m_apPoly[ i ] != TNULL )
				delete m_apPoly[ i ];

			m_apPoly[ i ] = TNULL;
		}
	}

	if ( m_pTRB != TNULL )
	{
		TAssetInit::DeinitAssets( *m_pTRB );

		if ( m_pTRB != TNULL )
		{
			m_pTRB->Close();
			TFree( m_pTRB );
		}

		m_pTRB = TNULL;
	}

	TPrimShader::GetSingleton()->SetImageAndUnlock( TNULL );
}

// $deBlob: FUNCTION 005d59b0
void A2GUIBackground::LoadMaterialLibrary( const TCHAR* a_szFileName )
{
	// NOTE: retail also creates an unused TString8 "16" here
	TString8 strFileName( a_szFileName );

	TCHAR szMatlibPath[ 256 ];
	TStringManager::String8Format( szMatlibPath, 256, "data/matlibs/%s.trb", strFileName.GetString() );

	if ( m_pTRB == TNULL )
		m_pTRB = new TTRB();

	TTRB::ERROR eRet = m_pTRB->Load( szMatlibPath );
	TASSERT( TTRB::ERROR_OK == eRet );

	TAssetInit::InitAssets( *m_pTRB, TTRUE, TFALSE );
}

// $deBlob: FUNCTION 005d5b30
void A2GUIBackground::SetupBackground()
{
	SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );

	TINT     iTileY = 0;
	TString8 strTextureName;

	TFLOAT fWidth, fHeight;
	T2GUI::GetSingleton()->GetRootElement()->GetDimensions( fWidth, fHeight );
	SetDimensions( fWidth, fHeight );

	// The sizes of the textures
	TCHAR*  szTextureName = TStringManager::GetTempString8();
	TUINT32 auiWidths[ 3 ];
	TUINT32 auiHeights[ 2 ];

	for ( TINT i = 0; i < MAX_TILES; i++ )
	{
		TStringManager::String8Format( szTextureName, "%s_%d.tga", m_szFileName, i + 1 );
		T2GUIMaterial* pMaterial = T2GUI::CreateMaterial( szTextureName );

		auiWidths[ i % 3 ]        = pMaterial->GetTextureResource()->GetWidth();
		auiHeights[ ( i / 3 ) & 1 ] = pMaterial->GetTextureResource()->GetHeight();

		T2GUI::DestroyMaterial( pMaterial );
	}

	TFLOAT fTotalWidth  = TFLOAT( auiWidths[ 0 ] ) + 0.0f + TFLOAT( auiWidths[ 1 ] ) + TFLOAT( auiWidths[ 2 ] );
	TFLOAT fTotalHeight = TFLOAT( auiHeights[ 0 ] ) + 0.0f + TFLOAT( auiHeights[ 1 ] );
	TINT   iTile        = 0;

	for ( TINT iRow = 0; iRow < 2; iRow++ )
	{
		TINT iTileHeight = TINT( ( TFLOAT( TINT( auiHeights[ iRow ] ) ) / fTotalHeight ) * fHeight );
		TINT iTileX      = 0;

		for ( TINT iColumn = 0; iColumn < 3; iColumn++ )
		{
			TASSERT( iTile < MAX_TILES );

			TINT iTileWidth = TINT( ( TFLOAT( TINT( auiWidths[ iColumn ] ) ) / fTotalWidth ) * fWidth );
			strTextureName.Format( "%s_%d.tga", m_szFileName, ++iTile );

			T2GUIPolygon*& rpTile = m_apPoly[ iTile - 1 ];
			TASSERT( TNULL == rpTile );

			rpTile = new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) T2GUIPolygon();
			rpTile->CreateBuffers( 4 );
			rpTile->SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_MIDDLECENTER );

			T2GUIMaterial* pMaterial = T2GUI::CreateMaterial( strTextureName.GetString() );
			pMaterial->SetBlendState( T2GUIMaterial::BLENDSTATE_ALPHA );
			rpTile->SetMaterial( pMaterial );
			pMaterial->GetTextureResource()->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

			TVector2* pUVs = rpTile->GetUVs();
			pUVs[ 0 ].Set( 0.0f, 0.0f );
			pUVs[ 1 ].Set( 1.0f, 0.0f );
			pUVs[ 2 ].Set( 0.0f, 1.0f );
			pUVs[ 3 ].Set( 1.0f, 1.0f );

			TFLOAT fX1 = TFLOAT( iTileX ) - s_fOffsetX;
			TFLOAT fY1 = TFLOAT( TINT( fHeight ) - iTileY ) - s_fOffsetY;
			iTileX += iTileWidth;
			TFLOAT fX2 = TFLOAT( iTileX );
			TFLOAT fY2 = TFLOAT( TINT( fHeight ) - iTileHeight - iTileY );

			TVector2* pVertices = rpTile->GetVertices();
			pVertices[ 0 ].Set( fX1, fY1 );
			pVertices[ 1 ].Set( fX2, fY1 );
			pVertices[ 2 ].Set( fX1, fY2 );
			pVertices[ 3 ].Set( fX2, fY2 );

			AddChildTail( rpTile );
		}

		iTileY += iTileHeight;
	}
}

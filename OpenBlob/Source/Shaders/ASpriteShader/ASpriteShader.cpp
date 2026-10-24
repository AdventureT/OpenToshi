#include "pch.h"
#include "ASpriteShader.h"
#include "ASpriteShaderHAL.h"
#include "ASpriteMesh.h"
#include "ASpriteMaterial.h"

#include <Render/TShader.h>
#include <Render/TRender.h>
#include <Render/TRenderContext.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00601350
// $deBlob: FUNCTION 00601360
// $deBlob: FUNCTION 00601370
// $deBlob: FUNCTION 00601380
// $deBlob: FUNCTION 00601390
// $deBlob: FUNCTION 0053cf10
// $deBlob: FUNCTION 00858740
TDEFINE_CLASS_NORUNTIME( ASpriteShader );

// NOTE: retail's RenderLineImpl reads two globals nothing else uses: a state initialised to -1 and a flag that is 1. While
// the state isn't 1 the line is generated; a cleared flag would let one more line through and then stop them all.
static TINT  s_iRenderLineState = -1;
static TBOOL s_bRenderLines     = TTRUE;

// $deBlob: FUNCTION 006021b0
ASpriteShader::ASpriteShader()
{
	m_bTransformChanged = TFALSE;
	m_itCurrentMesh     = TNULL;
	m_iNumIndices       = 0;
	m_iNumVertices      = 0;

	// NOTE: retail clears bits 0, 4 and 5 of the flag byte at 0x0C, sets bits 1, 2, 3 and 6 and keeps bit 7
	m_uiFlags = ( m_uiFlags & 0xCE ) | 0x4E;

	m_pLineMaterial          = TNULL;
	m_pFillMaterial          = TNULL;
	m_uiMeshFlags            = 100;
	m_uiMaxVertices          = 0x1800;
	m_uiMaxIndices           = 0x2400;
	m_pInitialMeshAllocation = TNULL;

	m_aiRenderState[ 0 ] = 4;
	m_aiRenderState[ 1 ] = 0x3F800000; // 1.0f
	m_aiRenderState[ 2 ] = 4;
	m_aiRenderState[ 3 ] = 5;
	m_aiRenderState[ 4 ] = 0;
	m_aiRenderState[ 5 ] = 4;
	m_pMaterial          = TNULL;
	m_uiColour           = 0xFFFFFFFF;

	m_mTransform = TMatrix44::IDENTITY;
}

// $deBlob: FUNCTION 00602380
// $deBlob: FUNCTION 006030f0
ASpriteShader::~ASpriteShader()
{
	m_pLineMaterial->OnDestroy();
	delete m_pLineMaterial;

	m_pFillMaterial->OnDestroy();
	delete m_pFillMaterial;

	// NOTE: the meshes of the pool aren't destroyed (and the ones AddMesh created are leaked), as in retail
	TFree( m_pInitialMeshAllocation );
}

// $deBlob: FUNCTION 006014e0
void ASpriteShader::OnDestroy()
{
	TShader::OnDestroy();
}

// $deBlob: FUNCTION 006013a0
TBOOL ASpriteShader::Create()
{
	TBOOL bCreated = TShader::Create();
	if ( bCreated )
	{
		TASSERT( TNULL == m_pInitialMeshAllocation );

		// The pool of the 256 meshes, each one is added to the mesh list
		TUINT32 uiStride         = 0;
		m_pInitialMeshAllocation = CreateMeshPool( 256, &uiStride );

		for ( TINT i = 0; i < 256; i++ )
		{
			ASpriteMesh* pMesh = TREINTERPRETCAST( ASpriteMesh*, TREINTERPRETCAST( TUINT8*, m_pInitialMeshAllocation ) + i * uiStride );
			pMesh->Create( m_uiMeshFlags | 0x4000, m_uiMaxVertices, m_uiMaxIndices );

			m_lMeshes.InsertTail( new ASpriteMeshNode( pMesh ) );
		}
	}

	return bCreated;
}

// $deBlob: FUNCTION 006014f0
TUINT8 ASpriteShader::SetTransformEnabled( TINT a_iEnable )
{
	TUINT8 uiToggle = ( TUINT8( a_iEnable << 5 ) ^ m_uiFlags ) & 0x20;
	m_uiFlags ^= uiToggle;
	return uiToggle;
}

// $deBlob: FUNCTION 00601510
void ASpriteShader::SetMaterial( ASpriteMaterial* a_pMaterial )
{
	if ( m_pMaterial != a_pMaterial )
		m_pMaterial = a_pMaterial;
}

// $deBlob: FUNCTION 005696e0
void ASpriteShader::SetColour( TUINT32 a_uiColour )
{
	m_uiColour = a_uiColour;
}

// $deBlob: FUNCTION 00601530
void ASpriteShader::SetTransform( const Toshi::TMatrix44& a_rMatrix )
{
	m_bTransformChanged = TTRUE;
	m_mTransform        = a_rMatrix;
}

// $deBlob: FUNCTION 006017d0
void ASpriteShader::RenderTriangle( const Toshi::TVector2& a_rPos1, const Toshi::TVector2& a_rUV1, const Toshi::TVector2& a_rPos2, const Toshi::TVector2& a_rUV2, const Toshi::TVector2& a_rPos3, const Toshi::TVector2& a_rUV3, TFLOAT a_fZ )
{
	RenderTriangle( a_rPos1.x, a_rPos1.y, a_rPos2.x, a_rPos2.y, a_rPos3.x, a_rPos3.y, a_fZ, a_rUV1.x, a_rUV1.y, a_rUV2.x, a_rUV2.y, a_rUV3.x, a_rUV3.y );
}

// $deBlob: FUNCTION 00601570
void ASpriteShader::RenderTriangle( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fX3, TFLOAT a_fY3, TFLOAT a_fZ, TFLOAT a_fU1, TFLOAT a_fV1, TFLOAT a_fU2, TFLOAT a_fV2, TFLOAT a_fU3, TFLOAT a_fV3 )
{
	TUINT32 uiColour = m_uiColour;

	ASpriteMesh* pMesh = AllocateMesh( 3, 5 );
	if ( pMesh == TNULL )
		return;

	TINT iStartIndex = m_iNumIndices;
	TINT iVertex     = m_iNumVertices;

	// The mesh already has indices: two degenerate indices join the triangle to the strip
	TINT iNumIndices = ( pMesh->m_uiStartIndex != TUINT32( iStartIndex ) ) * 2 + 3;

	TASSERT( 0 < m_oLockBuffer.oVertices.uiNumStreams );
	ASpriteVertex* pVertices = TREINTERPRETCAST( ASpriteVertex*, m_oLockBuffer.oVertices.apStreams[ 0 ] ) + iVertex;
	pVertices[ 0 ].fX        = a_fX1;
	pVertices[ 0 ].fY        = a_fY1;
	pVertices[ 0 ].fZ        = a_fZ;
	pVertices[ 0 ].uiColour  = uiColour;
	pVertices[ 0 ].fU        = a_fU1;
	pVertices[ 0 ].fV        = a_fV1;
	pVertices[ 1 ].fX        = a_fX2;
	pVertices[ 1 ].fY        = a_fY2;
	pVertices[ 1 ].fZ        = a_fZ;
	pVertices[ 1 ].uiColour  = uiColour;
	pVertices[ 1 ].fU        = a_fU2;
	pVertices[ 1 ].fV        = a_fV2;
	pVertices[ 2 ].fX        = a_fX3;
	pVertices[ 2 ].fY        = a_fY3;
	pVertices[ 2 ].fZ        = a_fZ;
	pVertices[ 2 ].uiColour  = uiColour;
	pVertices[ 2 ].fU        = a_fU3;
	pVertices[ 2 ].fV        = a_fV3;

	TASSERT( m_oLockBuffer.oIndices.pBuffer != TNULL );
	TUINT16* pIndices = m_oLockBuffer.oIndices.pBuffer + iStartIndex;

	if ( iNumIndices == 5 )
	{
		pIndices[ 0 ] = TMath::Int32ToUInt16( iVertex - 1 );
		pIndices[ 1 ] = TMath::Int32ToUInt16( iVertex );
		pIndices += 2;
	}

	pIndices[ 0 ] = TMath::Int32ToUInt16( iVertex );
	pIndices[ 1 ] = TMath::Int32ToUInt16( iVertex + 1 );
	pIndices[ 2 ] = TMath::Int32ToUInt16( iVertex + 2 );

	m_iNumVertices += 3;
	m_iNumIndices += iNumIndices;
}

// $deBlob: FUNCTION 00601b40
void ASpriteShader::RenderRectangle( const Toshi::TVector2& a_rPos1, const Toshi::TVector2& a_rUV1, const Toshi::TVector2& a_rPos2, const Toshi::TVector2& a_rUV2, TFLOAT a_fZ )
{
	RenderRectangle( a_rPos1.x, a_rPos1.y, a_rPos2.x, a_rPos2.y, a_fZ, a_rUV1.x, a_rUV1.y, a_rUV2.x, a_rUV2.y );
}

// $deBlob: FUNCTION 00601880
void ASpriteShader::RenderRectangle( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fZ, TFLOAT a_fU1, TFLOAT a_fV1, TFLOAT a_fU2, TFLOAT a_fV2 )
{
	TUINT32 uiColour = m_uiColour;

	ASpriteMesh* pMesh = AllocateMesh( 4, 6 );
	if ( pMesh == TNULL )
		return;

	TINT iStartIndex = m_iNumIndices;
	TINT iVertex     = m_iNumVertices;

	// The mesh already has indices: two degenerate indices join the rectangle to the strip
	TINT iNumIndices = ( pMesh->m_uiStartIndex != TUINT32( iStartIndex ) ) * 2 + 4;

	TASSERT( 0 < m_oLockBuffer.oVertices.uiNumStreams );
	ASpriteVertex* pVertices = TREINTERPRETCAST( ASpriteVertex*, m_oLockBuffer.oVertices.apStreams[ 0 ] ) + iVertex;
	pVertices[ 0 ].fX        = a_fX1;
	pVertices[ 0 ].fY        = a_fY1;
	pVertices[ 0 ].fZ        = a_fZ;
	pVertices[ 0 ].uiColour  = uiColour;
	pVertices[ 0 ].fU        = a_fU1;
	pVertices[ 0 ].fV        = a_fV1;
	pVertices[ 1 ].fX        = a_fX1;
	pVertices[ 1 ].fY        = a_fY2;
	pVertices[ 1 ].fZ        = a_fZ;
	pVertices[ 1 ].uiColour  = uiColour;
	pVertices[ 1 ].fU        = a_fU1;
	pVertices[ 1 ].fV        = a_fV2;
	pVertices[ 2 ].fX        = a_fX2;
	pVertices[ 2 ].fY        = a_fY1;
	pVertices[ 2 ].fZ        = a_fZ;
	pVertices[ 2 ].uiColour  = uiColour;
	pVertices[ 2 ].fU        = a_fU2;
	pVertices[ 2 ].fV        = a_fV1;
	pVertices[ 3 ].fX        = a_fX2;
	pVertices[ 3 ].fY        = a_fY2;
	pVertices[ 3 ].fZ        = a_fZ;
	pVertices[ 3 ].uiColour  = uiColour;
	pVertices[ 3 ].fU        = a_fU2;
	pVertices[ 3 ].fV        = a_fV2;

	TASSERT( m_oLockBuffer.oIndices.pBuffer != TNULL );
	TUINT16* pIndices = m_oLockBuffer.oIndices.pBuffer + iStartIndex;

	if ( iNumIndices == 6 )
	{
		pIndices[ 0 ] = TMath::Int32ToUInt16( iVertex - 1 );
		pIndices[ 1 ] = TMath::Int32ToUInt16( iVertex );
		pIndices += 2;
	}

	pIndices[ 0 ] = TMath::Int32ToUInt16( iVertex );
	pIndices[ 1 ] = TMath::Int32ToUInt16( iVertex + 1 );
	pIndices[ 2 ] = TMath::Int32ToUInt16( iVertex + 2 );
	pIndices[ 3 ] = TMath::Int32ToUInt16( iVertex + 3 );

	m_iNumVertices += 4;
	m_iNumIndices += iNumIndices;
}

// $deBlob: FUNCTION 00601bc0
void ASpriteShader::RenderTriStrip( Toshi::TVector2* a_pPositions, Toshi::TVector2* a_pUVs, TINT a_iNumVertices )
{
	TUINT32 uiColour = m_uiColour;

	ASpriteMesh* pMesh = AllocateMesh( a_iNumVertices, a_iNumVertices + 2 );
	if ( pMesh == TNULL )
		return;

	TINT  iStartIndex = m_iNumIndices;
	TBOOL bJoin       = pMesh->m_uiStartIndex != TUINT32( iStartIndex );

	// The mesh already has indices: two degenerate indices join this strip to it
	TINT iNumIndices = bJoin ? a_iNumVertices + 2 : a_iNumVertices;
	TINT iVertex     = m_iNumVertices;

	TASSERT( 0 < m_oLockBuffer.oVertices.uiNumStreams );
	ASpriteVertex* pVertices = TREINTERPRETCAST( ASpriteVertex*, m_oLockBuffer.oVertices.apStreams[ 0 ] ) + iVertex;

	for ( TINT i = 0; i < a_iNumVertices; i++ )
	{
		pVertices[ i ].fX       = a_pPositions[ i ].x;
		pVertices[ i ].fY       = a_pPositions[ i ].y;
		pVertices[ i ].fZ       = 0.0f;
		pVertices[ i ].uiColour = uiColour;
		pVertices[ i ].fU       = a_pUVs[ i ].x;
		pVertices[ i ].fV       = a_pUVs[ i ].y;
	}

	TASSERT( m_oLockBuffer.oIndices.pBuffer != TNULL );
	TUINT16* pIndices = m_oLockBuffer.oIndices.pBuffer + iStartIndex;

	if ( bJoin )
	{
		pIndices[ 0 ] = TMath::Int32ToUInt16( iVertex - 1 );
		pIndices[ 1 ] = TMath::Int32ToUInt16( iVertex );
		pIndices += 2;
	}

	for ( TINT i = 0; i < a_iNumVertices; i++ )
		*( pIndices++ ) = TMath::Int32ToUInt16( iVertex++ );

	m_iNumVertices += a_iNumVertices;
	m_iNumIndices += iNumIndices;
}

// $deBlob: FUNCTION 00601e70
void ASpriteShader::RenderLine( const Toshi::TVector2& a_rFrom, const Toshi::TVector2& a_rTo, TFLOAT a_fZ )
{
	ASpriteMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( m_pLineMaterial );
	RenderLineImpl( a_rFrom.x, a_rFrom.y, a_rTo.x, a_rTo.y, a_fZ );
	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 00601e00
void ASpriteShader::RenderLine( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fZ )
{
	ASpriteMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( m_pLineMaterial );
	RenderLineImpl( a_fX1, a_fY1, a_fX2, a_fY2, a_fZ );
	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 00601ee0
void ASpriteShader::RenderOutlineRectangle( const Toshi::TVector2& a_rMin, const Toshi::TVector2& a_rMax, TFLOAT a_fZ )
{
	ASpriteMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( m_pLineMaterial );

	TFLOAT fMinY = a_rMin.y;
	TFLOAT fMinX = a_rMin.x;
	TFLOAT fMaxY = a_rMax.y;
	TFLOAT fMaxX = a_rMax.x;
	RenderLineImpl( fMinX, fMinY, fMaxX, fMinY, a_fZ );
	RenderLineImpl( fMaxX, fMinY, fMaxX, fMaxY, a_fZ );
	RenderLineImpl( fMaxX, fMaxY, fMinX, fMaxY, a_fZ );
	RenderLineImpl( fMinX, fMaxY, fMinX, fMinY, a_fZ );

	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 00602020
void ASpriteShader::RenderFilledRectangle( const Toshi::TVector2& a_rMin, const Toshi::TVector2& a_rMax, TFLOAT a_fZ )
{
	ASpriteMaterial* pOldMaterial = m_pMaterial;
	SetMaterial( m_pFillMaterial );
	RenderRectangle( a_rMin, TVector2::VEC_ZERO, a_rMax, TVector2::VEC_ZERO, a_fZ );
	SetMaterial( pOldMaterial );
}

// $deBlob: FUNCTION 00602070
void ASpriteShader::BeginMeshGeneration()
{
	m_itCurrentMesh = m_lMeshes.End();
	m_iNumIndices   = 0;
	m_iNumVertices  = 0;
}

// $deBlob: FUNCTION 006020b0
void ASpriteShader::EndMeshGeneration()
{
	if ( !( m_itCurrentMesh == m_lMeshes.End() ) )
	{
		ASpriteMesh* pMesh = m_itCurrentMesh->GetMesh();

		TUINT16 uiNumIndices  = TMath::Int32ToUInt16( m_iNumIndices );
		TUINT16 uiNumVertices = TMath::Int32ToUInt16( m_iNumVertices );
		pMesh->Unlock( uiNumVertices, uiNumIndices );

		if ( pMesh->m_uiNumIndices != 0 )
		{
			pMesh->Validate();
			pMesh->Render();
		}
	}
}

// $deBlob: FUNCTION 006024a0
void ASpriteShader::AdvanceMeshList()
{
	// Closes and renders the current mesh
	if ( !( m_itCurrentMesh == m_lMeshes.End() ) )
	{
		ASpriteMesh* pMesh = m_itCurrentMesh->GetMesh();

		TUINT16 uiNumIndices  = TMath::Int32ToUInt16( m_iNumIndices );
		TUINT16 uiNumVertices = TMath::Int32ToUInt16( m_iNumVertices );
		pMesh->Unlock( uiNumVertices, uiNumIndices );

		if ( pMesh->m_uiNumIndices != 0 )
		{
			pMesh->Validate();
			pMesh->Render();
		}
	}

	++m_itCurrentMesh;

	// Out of meshes: a new one is added at the end of the list
	if ( m_itCurrentMesh == m_lMeshes.End() )
	{
		AddMesh();
		m_itCurrentMesh = m_lMeshes.RBegin();
	}

	ASpriteMesh* pMesh = m_itCurrentMesh->GetMesh();
	pMesh->SetMaterial( m_pMaterial );

	TBOOL res = pMesh->Lock( &m_oLockBuffer );
	TASSERT( res );

	TRender::GetSingleton()->GetCurrentContext()->SetModelViewMatrix( m_mTransform );
}

// $deBlob: FUNCTION 006026d0
ASpriteMesh* ASpriteShader::AllocateMesh( TINT a_iNumVertices, TINT a_iNumIndices )
{
	if ( a_iNumVertices + m_iNumVertices >= m_uiMaxVertices || a_iNumIndices + m_iNumIndices >= m_uiMaxIndices )
		return TNULL;

	if ( m_itCurrentMesh == m_lMeshes.End() )
	{
		AdvanceMeshList();
	}
	else if ( m_bTransformChanged == TTRUE )
	{
		AdvanceMeshList();
		m_bTransformChanged = TFALSE;
	}
	else if ( m_pMaterial != GetMesh()->GetMaterial() )
	{
		AdvanceMeshList();
	}

	return m_itCurrentMesh->GetMesh();
}

// $deBlob: FUNCTION 006027a0
ASpriteMesh* ASpriteShader::GetMesh()
{
	return m_itCurrentMesh->GetMesh();
}

// $deBlob: FUNCTION 006027f0
void ASpriteShader::AddMesh()
{
	ASpriteMesh* pMesh = CreateMesh( 0 );
	pMesh->Create( m_uiMeshFlags, m_uiMaxVertices, m_uiMaxIndices );

	m_lMeshes.InsertTail( new ASpriteMeshNode( pMesh ) );
}

// $deBlob: FUNCTION 00602890
void ASpriteShader::RenderLineImpl( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fZ )
{
	TUINT32 uiColour = m_uiColour;

	if ( s_iRenderLineState == 1 )
		return;

	if ( !s_bRenderLines )
		s_iRenderLineState = 1;

	if ( AllocateMesh( 2, 2 ) == TNULL )
		return;

	TINT iStartIndex = m_iNumIndices;
	TINT iVertex     = m_iNumVertices;

	TASSERT( 0 < m_oLockBuffer.oVertices.uiNumStreams );
	ASpriteVertex* pVertices = TREINTERPRETCAST( ASpriteVertex*, m_oLockBuffer.oVertices.apStreams[ 0 ] ) + iVertex;
	pVertices[ 0 ].fX        = a_fX1;
	pVertices[ 0 ].fY        = a_fY1;
	pVertices[ 0 ].fZ        = a_fZ;
	pVertices[ 0 ].uiColour  = uiColour;
	pVertices[ 0 ].fU        = 0.0f;
	pVertices[ 0 ].fV        = 0.0f;
	pVertices[ 1 ].fX        = a_fX2;
	pVertices[ 1 ].fY        = a_fY2;
	pVertices[ 1 ].fZ        = a_fZ;
	pVertices[ 1 ].uiColour  = uiColour;
	pVertices[ 1 ].fU        = 0.0f;
	pVertices[ 1 ].fV        = 0.0f;

	TASSERT( m_oLockBuffer.oIndices.pBuffer != TNULL );
	TUINT16* pIndices           = m_oLockBuffer.oIndices.pBuffer;
	pIndices[ iStartIndex ]     = TMath::Int32ToUInt16( iVertex );
	pIndices[ iStartIndex + 1 ] = TMath::Int32ToUInt16( iVertex + 1 );

	m_iNumVertices += 2;
	m_iNumIndices += 2;
}

// $deBlob: FUNCTION 005696a0
const TCHAR* ASpriteShader::GetShaderPrefix()
{
	return "";
}

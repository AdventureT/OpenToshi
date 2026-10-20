#include "ToshiPCH.h"
#include "TPhysicsCustomMeshShape.h"
#include "TPhysicsStandardMeshShape.h"
#include "TPhysicsMeshContainer.h"

#include <Physics/Collide/Shape/Convex/Triangle/hkpTriangleShape.h>
#include <Physics/Collide/Util/hkpTriangleUtil.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0072d7d0
TPhysicsCustomMeshShape::TPhysicsCustomMeshShape( TPhysicsMeshContainer* a_pContainer )
    : hkpMeshShape( 0.05f, 12 )
    , m_pContainer( a_pContainer )
{
	// NOTE: Havok 5.5's hkpMeshShape ctor clears its bounding box
	m_vAabbHalfExtents.setZero4();
	m_vAabbCenter.setZero4();
}

// $deBlob: FUNCTION 0072c540
void TPhysicsCustomMeshShape::getAabbImpl( const hkTransform& a_rLocalToWorld, hkReal a_fTolerance, hkAabb& a_rOut ) const
{
	TPhysicsStandardMeshShape::GetContainerAabb( m_pContainer, a_rLocalToWorld, a_fTolerance, a_rOut );
}

// $deBlob: FUNCTION 0072c860
void TPhysicsCustomMeshShape::addSubpart( const Subpart& a_rPart )
{
	hkpMeshShape::addSubpart( a_rPart );
	TPhysicsStandardMeshShape::AddContainerAabb( m_pContainer, m_scaling, m_radius, m_vAabbHalfExtents, m_vAabbCenter );
}

// $deBlob: FUNCTION 0072bc30
hkpShapeKey TPhysicsCustomMeshShape::getFirstKey() const
{
	ShapeBuffer oBuffer;

	for ( TUINT uiKey = 0; uiKey < TUINT( m_pContainer->GetNumTriangles() ); uiKey++ )
	{
		const hkpTriangleShape* pTriangle = TSTATICCAST( const hkpTriangleShape, getChildShape( uiKey, oBuffer ) );

		if ( !hkpTriangleUtil::isDegenerate( pTriangle->getVertex( 0 ), pTriangle->getVertex( 1 ), pTriangle->getVertex( 2 ), 1e-7f ) )
			return uiKey;
	}

	return HK_INVALID_SHAPE_KEY;
}

// $deBlob: FUNCTION 0072bd60
hkpShapeKey TPhysicsCustomMeshShape::getNextKey( hkpShapeKey a_uiOldKey ) const
{
	ShapeBuffer oBuffer;

	for ( TUINT uiKey = a_uiOldKey + 1; uiKey < TUINT( m_pContainer->GetNumTriangles() ); uiKey++ )
	{
		const hkpTriangleShape* pTriangle = TSTATICCAST( const hkpTriangleShape, getChildShape( uiKey, oBuffer ) );

		if ( !hkpTriangleUtil::isDegenerate( pTriangle->getVertex( 0 ), pTriangle->getVertex( 1 ), pTriangle->getVertex( 2 ), 1e-7f ) )
			return uiKey;
	}

	return HK_INVALID_SHAPE_KEY;
}

// $deBlob: FUNCTION 0072ccd0
hkUint32 TPhysicsCustomMeshShape::getCollisionFilterInfo( hkpShapeKey a_uiKey ) const
{
	return TPhysicsStandardMeshShape::GetFilterInfo( this, a_uiKey );
}

// One vertex of a custom mesh: a TINT16 vector scaled by 1 / ( 1 << a_iShift ), w 0
// $deBlob: FUNCTION 005abfc0
// $deBlob: FUNCTION 005ac1b0
static void DecompressVertex( hkVector4& a_rOutVertex, const TINT16* a_pVertex, TINT8 a_iShift )
{
	TFLOAT fScale = 1.0f / TFLOAT( 1 << a_iShift );
	a_rOutVertex.set( TFLOAT( a_pVertex[ 0 ] ) * fScale, TFLOAT( a_pVertex[ 1 ] ) * fScale, TFLOAT( a_pVertex[ 2 ] ) * fScale, fScale * 0.0f );
}

// The vertices of a custom mesh's triangle: three TINT16 vectors scaled by 1 / ( 1 << a_iShift ), w 0
// $deBlob: FUNCTION 00730480
// $deBlob: FUNCTION 005ac040
// $deBlob: FUNCTION 005ac230
static void DecompressTriangle( hkVector4* a_pOutVertices, const TINT16* a_pVertex0, const TINT16* a_pVertex1, const TINT16* a_pVertex2, TINT8 a_iShift )
{
	TFLOAT fScale = 1.0f / TFLOAT( 1 << a_iShift );

	a_pOutVertices[ 0 ].set( TFLOAT( a_pVertex0[ 0 ] ) * fScale, TFLOAT( a_pVertex0[ 1 ] ) * fScale, TFLOAT( a_pVertex0[ 2 ] ) * fScale, fScale * 0.0f );
	a_pOutVertices[ 1 ].set( TFLOAT( a_pVertex1[ 0 ] ) * fScale, TFLOAT( a_pVertex1[ 1 ] ) * fScale, TFLOAT( a_pVertex1[ 2 ] ) * fScale, fScale * 0.0f );
	a_pOutVertices[ 2 ].set( TFLOAT( a_pVertex2[ 0 ] ) * fScale, TFLOAT( a_pVertex2[ 1 ] ) * fScale, TFLOAT( a_pVertex2[ 2 ] ) * fScale, fScale * 0.0f );
}

// The triangle a_uiKey of the container, built in a_rBuffer
// $deBlob: FUNCTION 0072be70
const hkpShape* TPhysicsCustomMeshShape::getChildShape( hkpShapeKey a_uiKey, ShapeBuffer& a_rBuffer ) const
{
	// The byte offset of the triangle's first index: strips skip two indices (and 3 bytes, retail's formula) per strip
	TINT iIndexStride = m_pContainer->GetIndexStride();
	TINT iOffset;

	if ( m_pContainer->GetPrimitiveType() == TPhysicsPrimitiveType_Strips )
	{
		void* pStripIDs = m_pContainer->GetStripInfo();
		TINT  iStrip    = 0;

		if ( pStripIDs != TNULL )
		{
			if ( TUINT( m_pContainer->GetNumStrips() ) <= 0x80 )
				iStrip = TSTATICCAST( TUINT8, pStripIDs )[ a_uiKey ] & 0x7F;
			else
				iStrip = TSTATICCAST( TUINT16, pStripIDs )[ a_uiKey ] & 0x7FFF;
		}

		iOffset = a_uiKey * iIndexStride + iStrip * ( iIndexStride * 2 + 3 );
	}
	else
	{
		iOffset = a_uiKey * iIndexStride * 3;
	}

	TUINT8* pIndices   = TSTATICCAST( TUINT8, m_pContainer->GetIndices() );
	TINT    iIndexSize = m_pContainer->GetIndexSize();
	TUINT   uiIndex0;
	TUINT   uiIndex1;
	TUINT   uiIndex2;

	if ( m_pContainer->GetIndexSize() == 1 )
	{
		uiIndex0 = pIndices[ iOffset ];
		uiIndex1 = pIndices[ iOffset + iIndexStride ];
		uiIndex2 = pIndices[ iOffset + iIndexStride * 2 ];
	}
	else if ( m_pContainer->GetIndexSize() == 2 )
	{
		uiIndex0 = *TREINTERPRETCAST( TUINT16*, pIndices + iOffset );
		uiIndex1 = *TREINTERPRETCAST( TUINT16*, pIndices + iOffset + iIndexStride );
		uiIndex2 = *TREINTERPRETCAST( TUINT16*, pIndices + iOffset + iIndexStride * 2 );
	}
	else
	{
		// NOTE: the 32-bit indices step by the index size, not the stride (retail)
		uiIndex0 = *TREINTERPRETCAST( TUINT32*, pIndices + iOffset );
		uiIndex1 = *TREINTERPRETCAST( TUINT32*, pIndices + iIndexSize + iOffset );
		uiIndex2 = *TREINTERPRETCAST( TUINT32*, pIndices + iIndexSize * 2 + iOffset );
	}

	TUINT8* pVertices     = TREINTERPRETCAST( TUINT8*, m_pContainer->GetVertices() );
	TINT    iVertexStride = m_pContainer->GetVertexStride();

	hkVector4 aVertices[ 3 ];

	if ( m_pContainer->IsCustomMesh() == TTRUE )
	{
		// The custom meshes have compressed vertices
		DecompressTriangle(
		    aVertices,
		    TREINTERPRETCAST( const TINT16*, pVertices + iVertexStride * uiIndex0 ),
		    TREINTERPRETCAST( const TINT16*, pVertices + iVertexStride * uiIndex1 ),
		    TREINTERPRETCAST( const TINT16*, pVertices + iVertexStride * uiIndex2 ),
		    TINT8( m_pContainer->GetVertexShift() )
		);
	}
	else
	{
		const TFLOAT* pVertex0 = TREINTERPRETCAST( const TFLOAT*, pVertices + iVertexStride * uiIndex0 );
		const TFLOAT* pVertex1 = TREINTERPRETCAST( const TFLOAT*, pVertices + iVertexStride * uiIndex1 );
		const TFLOAT* pVertex2 = TREINTERPRETCAST( const TFLOAT*, pVertices + iVertexStride * uiIndex2 );

		aVertices[ 0 ].set( pVertex0[ 0 ], pVertex0[ 1 ], pVertex0[ 2 ], 0.0f );
		aVertices[ 1 ].set( pVertex1[ 0 ], pVertex1[ 1 ], pVertex1[ 2 ], 0.0f );
		aVertices[ 2 ].set( pVertex2[ 0 ], pVertex2[ 1 ], pVertex2[ 2 ], 0.0f );
	}

	return new ( &a_rBuffer ) hkpTriangleShape( aVertices[ 0 ], aVertices[ 1 ], aVertices[ 2 ], m_radius );
}

TOSHI_NAMESPACE_END

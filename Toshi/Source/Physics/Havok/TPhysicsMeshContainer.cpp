#include "ToshiPCH.h"
#include "TPhysicsMeshContainer.h"
#include "TPhysicsManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00734680
TPhysicsMeshDesc::TPhysicsMeshDesc()
{
	m_vAABBExtents    = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_vAABBCentre     = TVector4( 0.0f, 0.0f, 0.0f, 1.0f ); // TVector4::X0Y0Z0W1
	m_numMaterials    = 0;
	m_numTriangles    = 0;
	m_moppCode        = TNULL;
	m_fBoundingRadius = 1.0f;
	m_vertices        = TNULL;
	m_vertexStride    = 16;
	m_vertexCount     = 0;
	m_uiUnk2C         = 16;
	m_indices         = TNULL;
	m_indexStride     = 2;
	m_indexCount      = 0;
	m_indexSize       = 2;
	m_numStrips       = 0;
	m_pStripInfo      = TNULL;
	m_materialIndices = TNULL;
	m_primitiveType   = TPhysicsPrimitiveType_List;
	m_bCustomMesh     = TFALSE;
	m_uiVertexShift   = 0xff;
}

// $deBlob: FUNCTION 00734440
TPhysicsMeshInfo::TPhysicsMeshInfo()
{
	m_vAABBExtents  = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_vAABBCentre.x = m_vAABBCentre.y = m_vAABBCentre.z = 0.0f;
	m_vAABBCentre.w = 1.0f;
	m_uiNumVertices    = 0;
	m_uiNumIndices     = 0;
	m_pStripInfo                                        = TNULL;
	m_pMaterialIndices = TNULL;
	m_fBoundingRadius  = 1.0f;
	m_pIndices         = TNULL;
	m_pVertices        = TNULL;
	m_pMoppCode        = TNULL;
	m_iNumTriangles    = 0;
	m_uiIndexSize      = 4;
	m_uiIndexStride    = 4;
	m_uiUnk30          = 12;
	m_uiVertexStride   = 12;
	m_uiNumStrips      = 1;
	m_uiNumMaterials   = 0;
	m_uiPrimitiveType  = TPhysicsPrimitiveType_List;
	m_bCustomMesh      = TFALSE;
	m_uiVertexShift                                     = 0xff;
	m_uiFlags          = 0;
}

// $deBlob: FUNCTION 007344c0
void TPhysicsMeshInfo::Set( const TPhysicsMeshDesc& a_meshDesc )
{
	TASSERT( a_meshDesc.m_vertices );
	TASSERT( a_meshDesc.m_vertexCount );
	TASSERT( a_meshDesc.m_indices );
	TASSERT( a_meshDesc.m_indexCount );
	TASSERT( a_meshDesc.m_indexSize == sizeof( TUINT8 ) || a_meshDesc.m_indexSize == sizeof( TUINT16 ) || a_meshDesc.m_indexSize == sizeof( TUINT32 ) );

	m_pIndices         = a_meshDesc.m_indices;
	m_pVertices        = a_meshDesc.m_vertices;
	m_uiIndexSize      = a_meshDesc.m_indexSize;
	m_uiIndexStride    = a_meshDesc.m_indexStride;
	m_uiUnk30          = a_meshDesc.m_uiUnk2C;
	m_uiVertexStride   = a_meshDesc.m_vertexStride;
	m_uiNumStrips      = a_meshDesc.m_numStrips;
	m_pStripInfo       = a_meshDesc.m_pStripInfo;
	m_iNumTriangles    = a_meshDesc.m_numTriangles;
	m_uiNumVertices    = a_meshDesc.m_vertexCount;
	m_uiNumIndices     = a_meshDesc.m_indexCount;
	m_pMaterialIndices = a_meshDesc.m_materialIndices;
	m_uiNumMaterials   = a_meshDesc.m_numMaterials;
	m_vAABBExtents     = a_meshDesc.m_vAABBExtents;
	m_vAABBCentre      = a_meshDesc.m_vAABBCentre;
	m_fBoundingRadius  = a_meshDesc.m_fBoundingRadius;
	m_uiPrimitiveType  = a_meshDesc.m_primitiveType;
	m_bCustomMesh      = a_meshDesc.m_bCustomMesh;
	m_uiVertexShift    = a_meshDesc.m_uiVertexShift;

	TUINT8 uiFlags = m_uiFlags;
	m_uiFlags      = uiFlags | Flags_ExternalData;
	m_pMoppCode    = a_meshDesc.m_moppCode;

	if ( m_pMoppCode )
		m_uiFlags = uiFlags | Flags_ExternalData | Flags_ExternalMopp;
}

// $deBlob: FUNCTION 00734000
TPhysicsMeshContainer* TPhysicsMeshContainer::Create()
{
	return new ( TPhysicsManager::GetHeap() ) TPhysicsMeshContainer();
}

// $deBlob: FUNCTION 00734040
void TPhysicsMeshContainer::Destroy( TPhysicsMeshContainer* a_pContainer )
{
	if ( a_pContainer )
		delete a_pContainer;
}

// $deBlob: FUNCTION 00733f80
// $deBlob: FUNCTION 00734700
TPhysicsMeshContainer::~TPhysicsMeshContainer()
{
	if ( m_meshInfo )
	{
		if ( !( m_meshInfo->m_uiFlags & TPhysicsMeshInfo::Flags_ExternalData ) )
		{
			if ( m_meshInfo->m_pIndices ) TFree( m_meshInfo->m_pIndices );
			if ( m_meshInfo->m_pVertices ) TFree( m_meshInfo->m_pVertices );
			if ( m_meshInfo->m_pStripInfo ) TFree( m_meshInfo->m_pStripInfo );
		}

		if ( !( m_meshInfo->m_uiFlags & TPhysicsMeshInfo::Flags_ExternalMopp ) && m_meshInfo->m_pMoppCode )
			TFree( m_meshInfo->m_pMoppCode );
	}

	TFree( m_meshInfo );
	TFree( m_pMaterials );
}

// $deBlob: FUNCTION 00734060
TPhysicsMeshMaterial* TPhysicsMeshContainer::GetMaterial( TINT a_iIndex )
{
	if ( a_iIndex >= 0 && TUINT( a_iIndex ) < TUINT( GetNumMaterials() ) )
		return &m_pMaterials[ a_iIndex ];

	return TNULL;
}

// $deBlob: FUNCTION 007340e0
void TPhysicsMeshContainer::CreateMesh32( TFLOAT* a_pVertices, TUINT16 a_uiVertexStride, TUINT16 a_uiVertexCount, TUINT32* a_pIndices, TUINT a_indexCount, TModelCollisionMopp* a_pMoppCode )
{
	TASSERT( a_indexCount % 3 == 0 );

	TPhysicsMeshDesc oDesc;
	oDesc.m_vertices        = a_pVertices;
	oDesc.m_vertexStride    = a_uiVertexStride;
	oDesc.m_vertexCount     = a_uiVertexCount;
	oDesc.m_uiUnk2C         = 12;
	oDesc.m_indices         = a_pIndices;
	oDesc.m_vAABBExtents    = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	oDesc.m_indexStride     = 4;
	oDesc.m_moppCode        = a_pMoppCode;
	oDesc.m_numMaterials    = 0;
	oDesc.m_numTriangles    = a_indexCount / 3;
	oDesc.m_vAABBCentre     = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	oDesc.m_indexCount      = TUINT16( a_indexCount );
	oDesc.m_indexSize       = sizeof( TUINT32 );
	oDesc.m_numStrips       = 1;
	oDesc.m_pStripInfo      = TNULL;
	oDesc.m_materialIndices = TNULL;
	oDesc.m_primitiveType   = TPhysicsPrimitiveType_List;
	oDesc.m_bCustomMesh     = TFALSE;
	oDesc.m_uiVertexShift   = 0xff;
	oDesc.m_fBoundingRadius = 1.0f;

	SetMeshDesc( oDesc );
}

// $deBlob: FUNCTION 007341e0
void TPhysicsMeshContainer::CreateMesh16( TFLOAT* a_pVertices, TUINT16 a_uiVertexStride, TUINT16 a_uiVertexCount, TUINT16* a_pIndices, TUINT a_indexCount, TModelCollisionMopp* a_pMoppCode )
{
	TPhysicsMeshDesc oDesc;
	oDesc.m_vertices        = a_pVertices;
	oDesc.m_vertexStride    = a_uiVertexStride;
	oDesc.m_vertexCount     = a_uiVertexCount;
	oDesc.m_uiUnk2C         = 12;
	oDesc.m_indices         = a_pIndices;
	oDesc.m_indexStride     = 2;
	oDesc.m_numMaterials    = 0;
	oDesc.m_vAABBExtents    = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	oDesc.m_moppCode        = a_pMoppCode;
	oDesc.m_indexCount      = TUINT16( a_indexCount );
	oDesc.m_numTriangles    = a_indexCount / 3;
	oDesc.m_indexSize       = sizeof( TUINT16 );
	oDesc.m_numStrips       = 1;
	oDesc.m_pStripInfo      = TNULL;
	oDesc.m_materialIndices = TNULL;
	oDesc.m_primitiveType   = TPhysicsPrimitiveType_List;
	oDesc.m_bCustomMesh     = TFALSE;
	oDesc.m_uiVertexShift   = 0xff;
	oDesc.m_vAABBCentre     = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	oDesc.m_fBoundingRadius = 1.0f;

	SetMeshDesc( oDesc );
}

// $deBlob: FUNCTION 007342a0
void TPhysicsMeshContainer::SetMeshDesc( const TPhysicsMeshDesc& a_meshDesc )
{
	TASSERT( m_collisionData == TNULL );

	if ( !m_meshInfo )
		m_meshInfo = new ( TPhysicsManager::GetHeap() ) TPhysicsMeshInfo();

	m_meshInfo->Set( a_meshDesc );

	TINT iNumMaterials = GetNumMaterials();

	if ( iNumMaterials > 0 )
		m_pMaterials = TSTATICCAST( TPhysicsMeshMaterial, TMemalign( 16, iNumMaterials * sizeof( TPhysicsMeshMaterial ) ) );
}

// $deBlob: FUNCTION 00734390
void TPhysicsMeshContainer::SetCollisionData( TModelCollisionMesh* a_pCollisionData )
{
	TASSERT( m_meshInfo == TNULL );
	m_collisionData = a_pCollisionData;

	TINT iNumMaterials = GetNumMaterials();

	if ( iNumMaterials > 0 )
		m_pMaterials = TSTATICCAST( TPhysicsMeshMaterial, TMemalign( 16, iNumMaterials * sizeof( TPhysicsMeshMaterial ) ) );
}

TOSHI_NAMESPACE_END

#pragma once
#include "Render/TModelCollision.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// The surface of the triangles of a mesh body that use it (0x10 bytes): an hkpMeshMaterial (the collision filter
// info) followed by the game's surface parameters
//-----------------------------------------------------------------------------
struct TPhysicsMeshMaterial
{
	TUINT32 m_uiFilterInfo; // 0x00 hkpMeshMaterial::m_filterInfo
	TUINT32 m_uiSurface;    // 0x04 ACollisionCommon surface type
	TFLOAT  m_fFriction;    // 0x08
	TFLOAT  m_fRestitution; // 0x0c
};

// How the indices of a mesh make its triangles
enum TPhysicsPrimitiveType : TUINT8
{
	TPhysicsPrimitiveType_List   = 1,
	TPhysicsPrimitiveType_Strips = 2,
};

//-----------------------------------------------------------------------------
// A mesh made at runtime: copied into a TPhysicsMeshInfo by
// TPhysicsMeshContainer::SetMeshDesc.
//-----------------------------------------------------------------------------
struct TPhysicsMeshDesc
{
	TPhysicsMeshDesc();

	TVector4             m_vAABBExtents;    // 0x00
	TVector4             m_vAABBCentre;     // 0x10
	TFLOAT               m_fBoundingRadius; // 0x20
	TFLOAT*              m_vertices;        // 0x24
	TUINT16              m_vertexStride;    // 0x28
	TUINT16              m_vertexCount;     // 0x2a
	TUINT16              m_uiUnk2C;         // 0x2c
	TUINT16              m_uiUnk2E;         // 0x2e
	void*                m_indices;         // 0x30
	TUINT16              m_indexStride;     // 0x34
	TUINT16              m_indexCount;      // 0x36
	TUINT16              m_indexSize;       // 0x38 sizeof( TUINT8 ), sizeof( TUINT16 ) or sizeof( TUINT32 )
	TUINT16              m_numStrips;       // 0x3a
	void*                m_pStripInfo;      // 0x3c
	TUINT8*              m_materialIndices; // 0x40
	TUINT16              m_numMaterials;    // 0x44
	TUINT16              m_uiUnk46;         // 0x46
	TINT                 m_numTriangles;    // 0x48
	TUINT8               m_primitiveType;   // 0x4c (TPhysicsPrimitiveType)
	TBOOL                m_bCustomMesh;     // 0x4d
	TUINT8               m_uiVertexShift;   // 0x4e
	TUINT8               m_uiUnk4F;         // 0x4f
	TModelCollisionMopp* m_moppCode;        // 0x50
};

//-----------------------------------------------------------------------------
// The mesh of a TPhysicsMeshContainer made from a TPhysicsMeshDesc (0x50 bytes)
//-----------------------------------------------------------------------------
struct TPhysicsMeshInfo
{
	enum Flags : TUINT8
	{
		Flags_ExternalMopp = BITFLAG( 0 ), // the MOPP code isn't owned
		Flags_ExternalData = BITFLAG( 1 ), // the vertices/indices aren't owned
	};

	TPhysicsMeshInfo();

	void Set( const TPhysicsMeshDesc& a_meshDesc );

	TVector4             m_vAABBExtents;     // 0x00
	TVector4             m_vAABBCentre;      // 0x10
	TFLOAT               m_fBoundingRadius;  // 0x20
	void*                m_pIndices;         // 0x24
	TUINT16              m_uiIndexSize;      // 0x28
	TUINT16              m_uiIndexStride;    // 0x2a
	TFLOAT*              m_pVertices;        // 0x2c
	TUINT16              m_uiUnk30;          // 0x30
	TUINT16              m_uiVertexStride;   // 0x32
	TUINT16              m_uiNumStrips;      // 0x34
	TUINT16              m_uiNumMaterials;   // 0x36
	void*                m_pStripInfo;       // 0x38
	TModelCollisionMopp* m_pMoppCode;        // 0x3c
	TINT                 m_iNumTriangles;    // 0x40
	TUINT16              m_uiNumVertices;    // 0x44
	TUINT16              m_uiNumIndices;     // 0x46
	TUINT8*              m_pMaterialIndices; // 0x48
	TUINT8               m_uiPrimitiveType;  // 0x4c (TPhysicsPrimitiveType)
	TBOOL                m_bCustomMesh;      // 0x4d
	TUINT8               m_uiVertexShift;    // 0x4e
	TUINT8               m_uiFlags;          // 0x4f Flags
};

//-----------------------------------------------------------------------------
// Purpose: the shape data of a mesh body (TShapeBaseClass type 6, Source\Physics\Havok\TPhysicsMeshContainer.cpp,
// 0x10 bytes): either a mesh made at runtime (m_meshInfo) or a collision mesh of a model TRB (m_collisionData), and
// the per-surface materials of its triangles. TPhysicsManager::AddToSimulation builds a TPhysicsStandardMeshShape or
// TPhysicsCustomMeshShape over it.
//-----------------------------------------------------------------------------
class TPhysicsMeshContainer
{
public:
	static TPhysicsMeshContainer* Create();

	static void Destroy( TPhysicsMeshContainer* a_pContainer );

	~TPhysicsMeshContainer();

	TPhysicsMeshMaterial* GetMaterial( TINT a_iIndex );

	void CreateMesh32( TFLOAT* a_pVertices, TUINT16 a_uiVertexStride, TUINT16 a_uiVertexCount, TUINT32* a_pIndices, TUINT a_indexCount, TModelCollisionMopp* a_pMoppCode );

	void CreateMesh16( TFLOAT* a_pVertices, TUINT16 a_uiVertexStride, TUINT16 a_uiVertexCount, TUINT16* a_pIndices, TUINT a_indexCount, TModelCollisionMopp* a_pMoppCode );

	void SetMeshDesc( const TPhysicsMeshDesc& a_meshDesc );

	void SetCollisionData( TModelCollisionMesh* a_pCollisionData );

	// $deBlob: FUNCTION 00730a30
	TINT GetPrimitiveType() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_uiPrimitiveType;
		return ( m_collisionData->m_ePrimitiveType == TModelCollisionMesh::PrimitiveType_Strips ) ? TPhysicsPrimitiveType_Strips : TPhysicsPrimitiveType_List;
	}

	// $deBlob: FUNCTION 00730aa0
	TINT GetNumTriangles() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_iNumTriangles;
		if ( m_collisionData->m_ePrimitiveType == TModelCollisionMesh::PrimitiveType_List ) return m_collisionData->m_iNumIndices / 3;
		return m_collisionData->m_iNumIndices - m_collisionData->m_iNumStrips * 2;
	}

	// $deBlob: FUNCTION 00730910
	void* GetIndices() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_pIndices;
		return m_collisionData->m_pIndices;
	}

	// $deBlob: FUNCTION 00730830
	TINT GetIndexSize() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_uiIndexSize;
		return m_collisionData->m_iIndexSize;
	}

	// $deBlob: FUNCTION 007308b0
	TINT GetIndexStride() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_uiIndexStride;
		return m_collisionData->m_iIndexStride;
	}

	// $deBlob: FUNCTION 00730b70
	TINT GetNumStrips() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_uiNumStrips;
		return m_collisionData->m_iNumStrips;
	}

	// $deBlob: FUNCTION 00730c50
	void* GetStripInfo() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_pStripInfo;
		return m_collisionData->m_pStripInfo;
	}

	// $deBlob: FUNCTION 00730e50
	TFLOAT* GetVertices() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_pVertices;
		return m_collisionData->m_pVertices;
	}

	// $deBlob: FUNCTION 00730bc0
	TINT GetNumVertices() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_uiNumVertices;
		return m_collisionData->m_iNumVertices;
	}

	// $deBlob: FUNCTION 00730d80
	TINT GetUnk58() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_uiUnk30;
		return TINT8( m_collisionData->m_uiUnk58 );
	}

	// $deBlob: FUNCTION 00730df0
	TINT GetVertexStride() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_uiVertexStride;
		return m_collisionData->m_iVertexStride;
	}

	// $deBlob: FUNCTION 005a6170
	TBOOL IsCustomMesh() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_bCustomMesh;
		return m_collisionData->m_eMeshType == 2;
	}

	// The shift of the compressed vertices of a custom mesh (TINT16, scaled by 1 / ( 1 << shift ))
	// $deBlob: FUNCTION 00730d00
	TINT GetVertexShift() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_uiVertexShift;
		return TINT8( m_collisionData->m_uiVertexShift );
	}

	// $deBlob: FUNCTION 007309d0
	TUINT8* GetMaterialIndices() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_pMaterialIndices;
		return m_collisionData->m_pMaterialIndices;
	}

	// $deBlob: FUNCTION 00730b10
	TINT GetNumMaterials() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_uiNumMaterials;
		return m_collisionData->m_iNumMaterials;
	}

	// $deBlob: FUNCTION 00730630
	const TVector4& GetAABBExtents() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_vAABBExtents;
		return m_collisionData->m_vAABBExtents;
	}

	// $deBlob: FUNCTION 007305d0
	const TVector4& GetAABBCentre() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_vAABBCentre;
		return m_collisionData->m_vAABBCentre;
	}

	// $deBlob: FUNCTION 00730680
	TFLOAT GetBoundingRadius() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_fBoundingRadius;
		return m_collisionData->m_fBoundingRadius;
	}

	// $deBlob: FUNCTION 00730960
	TModelCollisionMopp* GetMoppCode() const
	{
		TASSERT( m_meshInfo != TNULL || m_collisionData != TNULL );
		if ( m_meshInfo ) return m_meshInfo->m_pMoppCode;
		return m_collisionData->m_pMoppCode;
	}

	TBOOL IsConvex() const { return m_bConvex; }
	// $deBlob: FUNCTION 00733f50
	TPhysicsMeshMaterial* GetMaterials() const { return m_pMaterials; }

private:
	TPhysicsMeshContainer()
	    : m_pMaterials( TNULL ), m_meshInfo( TNULL ), m_collisionData( TNULL ), m_bConvex( TFALSE )
	{}

public:
	TPhysicsMeshMaterial* m_pMaterials;    // 0x00 one per surface (GetNumMaterials)
	TPhysicsMeshInfo*     m_meshInfo;      // 0x04
	TModelCollisionMesh*  m_collisionData; // 0x08
	TBOOL                 m_bConvex;       // 0x0c AddToSimulation builds a convex shape instead of a MOPP mesh
};

TOSHI_NAMESPACE_END

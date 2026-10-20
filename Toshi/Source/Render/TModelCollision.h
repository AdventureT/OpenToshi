#pragma once
#include "TCollisionCommon.h"
#include "Toshi/TString8.h"
#include "Render/TTMDBase.h"
#include "Math/TVector3.h"
#include "Math/TVector4.h"
#include "Math/TMatrix44.h"
#include "ToshiTools/T2DynamicVector.h"

TOSHI_NAMESPACE_START

struct TModelCollisionGroup
{
	TModelCollisionGroup();
	~TModelCollisionGroup();

	TCollisionCommon::TOSHICGROUPFLAG GetGroupFlag() const { return TCollisionCommon::TOSHICGROUPFLAG( 1U << ( eCollGroup % 32U ) ); }

	TString8                                   strName;
	TUINT                                      uiUnk1     = 0;
	TUINT                                      uiNumFaces = 0;
	T2DynamicVector<TTMDBase::CollisionTypeS1> vecS1;
	TUINT                                      uiUnk3 = 0;
	TCollisionCommon::TOSHICGROUP              eCollGroup;
};

class TModelCollisionData
{
public:
	friend class TModel;

public:
	TModelCollisionData();
	~TModelCollisionData();

	void GetTriangleVertices( TUINT a_uiFaceId, Toshi::TVector3* a_rVertices[ 3 ] );
	TINT GetGroupForTriangleImp( TUINT a_uiFaceId );

	TINT                  GetBoneID() const { return m_iBoneID; }
	TVector3*             GetVertices() const { return m_pVertices; }
	TUINT                 GetNumVertices() const { return m_uiNumVertices; }
	TUINT16*              GetIndices() const { return m_pIndices; }
	TUINT                 GetNumIndices() const { return m_uiNumIndices; }
	TINT                  GetNumCollGroups() const { return m_vecCollGroups.Size(); }
	TModelCollisionGroup& GetCollGroup( TINT a_iIndex ) { return m_vecCollGroups[ a_iIndex ]; }

protected:
	TINT                                  m_iBoneID;
	TVector3*                             m_pVertices;
	TUINT                                 m_uiNumVertices;
	TUINT16*                              m_pIndices;
	TUINT                                 m_uiNumIndices;
	T2DynamicVector<TModelCollisionGroup> m_vecCollGroups;
};

//-----------------------------------------------------------------------------
// de Blob's model collision ("Collision" TRB symbol of a model, TModel::m_pCollision): a list of collision meshes with
// their MOPP code, used as is by TPhysicsMeshContainer (its m_collisionData). Barnyard's TModelCollisionData above is a
// different format.
//-----------------------------------------------------------------------------

// The precompiled MOPP code of a collision mesh (hkpMoppCode::CodeInfo and the byte code)
struct TModelCollisionMopp
{
	TFLOAT  m_aOffset[ 4 ];   // 0x00 hkpMoppCode::CodeInfo::m_offset
	TINT    m_iByteCodeSize;  // 0x10
	TUINT8* m_pByteCode;      // 0x14
};

// A surface (material) of a collision mesh (0x1c bytes)
struct TModelCollisionMaterial
{
	const TCHAR* m_szName;       // 0x00 the surface type (ACollisionCommon::GetSurfaceType)
	TUINT8       m_aUnk04[ 0x18 ]; // 0x04
};

// A collision mesh (0xa0 bytes)
struct TModelCollisionMesh
{
	enum PrimitiveType : TINT8
	{
		PrimitiveType_Strips = 1,
		PrimitiveType_List   = 3,
	};

	// $deBlob: FUNCTION 005a5e60
	const TVector4& GetAABBCentre() const { return m_vAABBCentre; }
	// $deBlob: FUNCTION 005a5e70
	const TVector4& GetAABBExtents() const { return m_vAABBExtents; }
	// $deBlob: FUNCTION 005a5e80
	TFLOAT GetBoundingRadius() const { return m_fBoundingRadius; }

	TMatrix44                m_mTransform;      // 0x00 relative to the model
	TINT                     m_iBone;           // 0x40 -1: static
	TFLOAT*                  m_pVertices;       // 0x44
	TINT                     m_iNumVertices;    // 0x48
	TINT                     m_iNumStrips;      // 0x4c
	void*                    m_pIndices;        // 0x50
	TINT                     m_iNumIndices;     // 0x54
	TUINT8                   m_uiUnk58;         // 0x58
	TINT8                    m_iVertexStride;   // 0x59
	TINT8                    m_eMeshType;       // 0x5a 2: TPhysicsCustomMeshShape
	TUINT8                   m_uiVertexShift;   // 0x5b
	TINT8                    m_iIndexSize;      // 0x5c
	TINT8                    m_iIndexStride;    // 0x5d
	TINT8                    m_ePrimitiveType;  // 0x5e PrimitiveType
	TUINT8                   m_uiUnk5F;         // 0x5f
	TINT                     m_iNumMaterials;   // 0x60
	TModelCollisionMaterial* m_pMaterials;      // 0x64
	TUINT32                  m_uiUnk68;         // 0x68
	TModelCollisionMopp*     m_pMoppCode;       // 0x6c
	TVector4                 m_vAABBExtents;    // 0x70
	TVector4                 m_vAABBCentre;     // 0x80
	void*                    m_pStripInfo;      // 0x90
	TUINT8*                  m_pMaterialIndices; // 0x94 one per triangle
	TFLOAT                   m_fBoundingRadius; // 0x98
	TUINT32                  m_uiUnk9C;         // 0x9c
};

class TModelCollision
{
public:
	TINT                 m_iNumCollisionModels; // 0x00
	TModelCollisionMesh* m_pCollisionModels;    // 0x04
};

TOSHI_NAMESPACE_END

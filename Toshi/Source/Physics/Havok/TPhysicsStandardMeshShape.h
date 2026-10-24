#pragma once
#include <Common/Base/hkBase.h>
#include <Physics/Collide/Shape/Deprecated/Mesh/hkpMeshShape.h>

TOSHI_NAMESPACE_START

class TPhysicsMeshContainer;

//-----------------------------------------------------------------------------
// Purpose: the Havok shape of a mesh body (TPhysicsManager::AddToSimulation, shape type 6) whose triangles Havok can
// read itself (8/16/32 bit index lists or strips): an hkpMeshShape with one subpart made from the container, its
// bounding box from the container's precomputed one and the filter info of a triangle from its surface material or
// else from the body.
// NOTE: Havok 5.5's hkpMeshShape keeps a
// bounding box of its subparts (+0x30/+0x40) that addSubpart updates; 6.5's has none, the box is kept in
// m_vAabbHalfExtents/m_vAabbCenter
//-----------------------------------------------------------------------------
class TPhysicsStandardMeshShape
    : public hkpMeshShape
{
public:
	TPhysicsStandardMeshShape( TPhysicsMeshContainer* a_pContainer );

	// $deBlob: FUNCTION 0072f8d0
	// $deBlob: FUNCTION 0072e630
	// $deBlob: FUNCTION 0072f6ab
	virtual ~TPhysicsStandardMeshShape() override = default;

	virtual void getAabbImpl( const hkTransform& a_rLocalToWorld, hkReal a_fTolerance, hkAabb& a_rOut ) const override;

	virtual void addSubpart( const Subpart& a_rPart ) override;

	virtual hkUint32 getCollisionFilterInfo( hkpShapeKey a_uiKey ) const override;

	void SetRadius( hkReal a_fRadius ) { m_radius = a_fRadius; }

	// The world box of a container's local box (getAabbImpl, also used by TPhysicsCustomMeshShape)
	static void GetContainerAabb( const TPhysicsMeshContainer* a_pContainer, const hkTransform& a_rLocalToWorld, hkReal a_fTolerance, hkAabb& a_rOut );

	// Grows the box by the container's box (addSubpart, also used by TPhysicsCustomMeshShape)
	static void AddContainerAabb( const TPhysicsMeshContainer* a_pContainer, const hkVector4& a_rScaling, hkReal a_fRadius, hkVector4& a_rHalfExtents, hkVector4& a_rCenter );

	// The filter info of a triangle (getCollisionFilterInfo, also used by TPhysicsCustomMeshShape)
	static hkUint32 GetFilterInfo( const hkpMeshShape* a_pShape, hkpShapeKey a_uiKey );

public:
	hkVector4              m_vAabbHalfExtents; // 0x30 in Havok 5.5
	hkVector4              m_vAabbCenter;      // 0x40 in Havok 5.5
	TPhysicsMeshContainer* m_pContainer;       // 0xc0
};

TOSHI_NAMESPACE_END

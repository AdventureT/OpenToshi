#pragma once
#include <Common/Base/hkBase.h>
#include <Physics/Collide/Shape/Deprecated/Mesh/hkpMeshShape.h>

TOSHI_NAMESPACE_START

class TPhysicsMeshContainer;

//-----------------------------------------------------------------------------
// Purpose: the Havok shape of a mesh body whose triangles Havok can't read itself (TPhysicsMeshContainer::IsCustomMesh
// or 8 bit indices): like TPhysicsStandardMeshShape, but the triangles (keys = triangle indices, degenerate ones
// skipped) are built by getChildShape from the container.
//-----------------------------------------------------------------------------
class TPhysicsCustomMeshShape
    : public hkpMeshShape
{
public:
	TPhysicsCustomMeshShape( TPhysicsMeshContainer* a_pContainer );

	// $deBlob: FUNCTION 0072f800
	// $deBlob: FUNCTION 0072e5d0
	// $deBlob: FUNCTION 0072f683
	virtual ~TPhysicsCustomMeshShape() override = default;

	virtual void getAabbImpl( const hkTransform& a_rLocalToWorld, hkReal a_fTolerance, hkAabb& a_rOut ) const override;

	virtual void addSubpart( const Subpart& a_rPart ) override;

	virtual hkpShapeKey getFirstKey() const override;

	virtual hkpShapeKey getNextKey( hkpShapeKey a_uiOldKey ) const override;

	virtual hkUint32 getCollisionFilterInfo( hkpShapeKey a_uiKey ) const override;

	virtual const hkpShape* getChildShape( hkpShapeKey a_uiKey, ShapeBuffer& a_rBuffer ) const override;

	void SetRadius( hkReal a_fRadius ) { m_radius = a_fRadius; }

public:
	hkVector4              m_vAabbHalfExtents; // 0x30 in Havok 5.5
	hkVector4              m_vAabbCenter;      // 0x40 in Havok 5.5
	TPhysicsMeshContainer* m_pContainer;       // 0xc0
};

TOSHI_NAMESPACE_END

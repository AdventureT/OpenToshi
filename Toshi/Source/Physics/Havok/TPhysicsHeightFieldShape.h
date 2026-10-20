#pragma once
#include <Common/Base/hkBase.h>
#include <Physics/Collide/Shape/HeightField/SampledHeightField/hkpSampledHeightFieldShape.h>

TOSHI_NAMESPACE_START

class TPhysicsHFContainer;

//-----------------------------------------------------------------------------
// Purpose: the Havok shape of a height field body (shape type 8): the heights are read from a TPhysicsHFContainer.
// NOTE: Havok 5.5's virtuals are
// getHeightAt/getTriangleFlip, 6.x renamed them getHeightAtImpl/getTriangleFlipImpl
//-----------------------------------------------------------------------------
class TPhysicsHeightFieldShape
    : public hkpSampledHeightFieldShape
{
public:
	TPhysicsHeightFieldShape( const hkpSampledHeightFieldBaseCinfo& a_rCinfo, TPhysicsHFContainer* a_pContainer )
	    : hkpSampledHeightFieldShape( a_rCinfo )
	    , m_pContainer( a_pContainer )
	{}

	// $deBlob: FUNCTION 0072f840
	virtual ~TPhysicsHeightFieldShape() override = default;

	virtual void collideSpheres( const CollideSpheresInput& a_rInput, SphereCollisionOutput* a_pOutputArray ) const override;

	virtual hkReal getHeightAtImpl( int a_x, int a_z ) const override;

	// $deBlob: FUNCTION 00732380
	virtual hkBool getTriangleFlipImpl() const override { return true; }

	// Hide the base's forwarding inlines so that Havok's collideSpheres template (impl.IMPL::getHeightAt) reads the
	// container directly, as retail's 5.5 build does (non-virtual)
	hkReal                 getHeightAt( int a_x, int a_z ) const;
	HK_FORCE_INLINE hkBool getTriangleFlip() const { return true; }

public:
	TPhysicsHFContainer* m_pContainer; // 0x60 in Havok 5.5
};

TOSHI_NAMESPACE_END

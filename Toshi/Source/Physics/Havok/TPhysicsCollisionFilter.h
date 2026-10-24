#pragma once
#include <Common/Base/hkBase.h>
#include <Physics/Collide/Filter/hkpCollisionFilter.h>

TOSHI_NAMESPACE_START

// The collision filter of TPhysicsManager's worlds. Collision filter info: bits 0-11 the layers the object collides
// with, 12-23 its own layers, 24-31 a group (objects of the same non-zero group never collide)
class TPhysicsManagerCollisionFilter
    : public hkpCollisionFilter
{
public:
	HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_COLLIDE );

	// $deBlob: FUNCTION 0072f880
	// $deBlob: FUNCTION 0072f68b
	// $deBlob: FUNCTION 0072f693
	// $deBlob: FUNCTION 0072f69b
	// $deBlob: FUNCTION 0072f6a3
	virtual ~TPhysicsManagerCollisionFilter() = default;

	// hkpCollidableCollidableFilter
	virtual hkBool isCollisionEnabled( const hkpCollidable& a, const hkpCollidable& b ) const override;

	// hkpShapeCollectionFilter
	virtual hkBool isCollisionEnabled( const hkpCollisionInput& input, const hkpCdBody& a, const hkpCdBody& b, const HK_SHAPE_CONTAINER& bContainer, hkpShapeKey bKey ) const override;
	virtual hkBool isCollisionEnabled( const hkpCollisionInput& input, const hkpCdBody& collectionBodyA, const hkpCdBody& collectionBodyB, const HK_SHAPE_CONTAINER& containerShapeA, const HK_SHAPE_CONTAINER& containerShapeB, hkpShapeKey keyA, hkpShapeKey keyB ) const override;

	// hkpRayShapeCollectionFilter
	virtual hkBool isCollisionEnabled( const hkpShapeRayCastInput& aInput, const hkpShape& bShape, const HK_SHAPE_CONTAINER& bContainer, hkpShapeKey bKey ) const override;

	// hkpRayCollidableFilter
	virtual hkBool isCollisionEnabled( const hkpWorldRayCastInput& a, const hkpCollidable& collidableB ) const override;

	// a's layers against b's mask only
	static hkBool IsCollisionEnabledOneWay( hkUint32 a_uiInfoA, hkUint32 a_uiInfoB );

	// Either way round
	static hkBool IsCollisionEnabledTwoWay( hkUint32 a_uiInfoA, hkUint32 a_uiInfoB );
};

TOSHI_NAMESPACE_END

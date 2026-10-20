#include "ToshiPCH.h"
#include "TPhysicsCollisionFilter.h"

#include <Physics/Collide/Agent/Collidable/hkpCollidable.h>
#include <Physics/Collide/Shape/hkpShapeContainer.h>
#include <Physics/Collide/Shape/Compound/Collection/hkpShapeCollection.h>
#include <Physics/Collide/Shape/Query/hkpShapeRayCastInput.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>
#include <Physics/Dynamics/Entity/hkpEntity.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include "TPhysicsObjectContainer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// NOTE: retail reads the motion type of the collidable's owner as an hkpEntity without checking the owner's type
// (5.5 layout: owner + 0xd8); the same here through 6.5's accessors
static hkpMotion::MotionType GetOwnerMotionType( const hkpCollidable& a_rCollidable )
{
	hkpEntity* pEntity = static_cast<hkpEntity*>( a_rCollidable.getOwner() );
	return pEntity->getMotion()->getType();
}

static hkUint32 GetRootFilterInfo( const hkpCdBody& a_rBody )
{
	const hkpCdBody* pBody = &a_rBody;

	while ( pBody->getParent() )
		pBody = pBody->getParent();

	return static_cast<const hkpCollidable*>( pBody )->getCollisionFilterInfo();
}

// $deBlob: FUNCTION 00735b30
hkBool TPhysicsManagerCollisionFilter::IsCollisionEnabledOneWay( hkUint32 a_uiInfoA, hkUint32 a_uiInfoB )
{
	if ( ( a_uiInfoA >> 12 & a_uiInfoB & 0xfff ) == 0 )
		return false;

	if ( ( a_uiInfoA >> 24 ) != 0 && ( a_uiInfoB >> 24 ) != 0 )
		return ( a_uiInfoA >> 24 ) != ( a_uiInfoB >> 24 );

	return true;
}

// $deBlob: FUNCTION 00735ad0
hkBool TPhysicsManagerCollisionFilter::IsCollisionEnabledTwoWay( hkUint32 a_uiInfoA, hkUint32 a_uiInfoB )
{
	if ( ( a_uiInfoA >> 12 & a_uiInfoB & 0xfff ) == 0 && ( a_uiInfoB >> 12 & a_uiInfoA & 0xfff ) == 0 )
		return false;

	if ( ( a_uiInfoA >> 24 ) != 0 && ( a_uiInfoB >> 24 ) != 0 )
		return ( a_uiInfoA >> 24 ) != ( a_uiInfoB >> 24 );

	return true;
}

// $deBlob: FUNCTION 00735790
hkBool TPhysicsManagerCollisionFilter::isCollisionEnabled( const hkpCollidable& a, const hkpCollidable& b ) const
{
	hkpMotion::MotionType eTypeA = GetOwnerMotionType( a );
	hkpMotion::MotionType eTypeB = GetOwnerMotionType( b );

	if ( eTypeA == hkpMotion::MOTION_KEYFRAMED && eTypeB == hkpMotion::MOTION_FIXED )
		return IsCollisionEnabledOneWay( a.getCollisionFilterInfo(), b.getCollisionFilterInfo() );

	if ( eTypeB == hkpMotion::MOTION_KEYFRAMED && eTypeA == hkpMotion::MOTION_FIXED )
		return IsCollisionEnabledOneWay( b.getCollisionFilterInfo(), a.getCollisionFilterInfo() );

	if ( eTypeA == hkpMotion::MOTION_KEYFRAMED && eTypeB == hkpMotion::MOTION_KEYFRAMED )
		return false;

	return IsCollisionEnabledTwoWay( a.getCollisionFilterInfo(), b.getCollisionFilterInfo() );
}

// $deBlob: FUNCTION 00735860
hkBool TPhysicsManagerCollisionFilter::isCollisionEnabled( const hkpCollisionInput& input, const hkpCdBody& a, const hkpCdBody& b, const HK_SHAPE_CONTAINER& bContainer, hkpShapeKey bKey ) const
{
	hkUint32 uiInfoB = bContainer.getCollisionFilterInfo( bKey );

	if ( uiInfoB == 0 )
		uiInfoB = GetRootFilterInfo( b );

	return IsCollisionEnabledTwoWay( GetRootFilterInfo( a ), uiInfoB );
}

// $deBlob: FUNCTION 007358f0
hkBool TPhysicsManagerCollisionFilter::isCollisionEnabled( const hkpCollisionInput& input, const hkpCdBody& collectionBodyA, const hkpCdBody& collectionBodyB, const HK_SHAPE_CONTAINER& containerShapeA, const HK_SHAPE_CONTAINER& containerShapeB, hkpShapeKey keyA, hkpShapeKey keyB ) const
{
	hkUint32 uiInfoA = containerShapeA.getCollisionFilterInfo( keyA );

	if ( uiInfoA == 0 )
		uiInfoA = GetRootFilterInfo( collectionBodyA );

	hkUint32 uiInfoB = containerShapeB.getCollisionFilterInfo( keyB );

	if ( uiInfoB == 0 )
		uiInfoB = GetRootFilterInfo( collectionBodyB );

	return IsCollisionEnabledTwoWay( uiInfoA, uiInfoB );
}

// $deBlob: FUNCTION 007359a0
hkBool TPhysicsManagerCollisionFilter::isCollisionEnabled( const hkpShapeRayCastInput& aInput, const hkpShape& bShape, const HK_SHAPE_CONTAINER& bContainer, hkpShapeKey bKey ) const
{
	hkUint32 uiInfoB = bContainer.getCollisionFilterInfo( bKey );

	if ( uiInfoB == 0 )
	{
		// The filter info of the body of the shape's TPhysicsObjectContainer (its user data)
		TPhysicsObjectContainer* POC = reinterpret_cast<TPhysicsObjectContainer*>( bShape.getUserData() );

		// NOTE: Havok 6.5's MOPP ray casts pass the child shape (a triangle without user data) where 5.5 passed the
		// shape collection: the collection's user data then
		if ( POC == TNULL )
			POC = reinterpret_cast<TPhysicsObjectContainer*>( static_cast<const hkpShapeCollection&>( bContainer ).getUserData() );

		TASSERT( POC != TNULL );
		uiInfoB = POC->m_body->getCollidable()->getCollisionFilterInfo();
	}

	return IsCollisionEnabledOneWay( aInput.m_filterInfo, uiInfoB );
}

// $deBlob: FUNCTION 00735a40
hkBool TPhysicsManagerCollisionFilter::isCollisionEnabled( const hkpWorldRayCastInput& a, const hkpCollidable& collidableB ) const
{
	return IsCollisionEnabledOneWay( a.m_filterInfo, collidableB.getCollisionFilterInfo() );
}

TOSHI_NAMESPACE_END

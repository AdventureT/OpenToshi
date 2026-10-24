#include "ToshiPCH.h"
#include "TPhysicsTriggerShape.h"
#include "TPhysicsManager.h"

#include <Physics/Dynamics/Entity/hkpRigidBody.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// The container of a collidable's rigid body (its shape's user data)
// NOTE: retail dereferences a null body when the collidable isn't an entity's
static TPhysicsObjectContainer* GetContainer( const hkpCollidable* a_pCollidable )
{
	hkpRigidBody* pBody = hkGetRigidBody( a_pCollidable );
	return reinterpret_cast<TPhysicsObjectContainer*>( pBody->getCollidable()->getShape()->getUserData() );
}

// $deBlob: FUNCTION 007363f0
void TPhysicsTriggerShape::phantomEnterEvent( const hkpCollidable* a_pPhantomColl, const hkpCollidable* a_pOtherColl, const hkpCollisionInput& a_rEnv )
{
	m_pManager->OnTriggerEnter( GetContainer( a_pPhantomColl ), GetContainer( a_pOtherColl ) );
}

// $deBlob: FUNCTION 00736450
void TPhysicsTriggerShape::phantomLeaveEvent( const hkpCollidable* a_pPhantomColl, const hkpCollidable* a_pOtherColl )
{
	m_pManager->OnTriggerLeave( GetContainer( a_pPhantomColl ), GetContainer( a_pOtherColl ) );
}

TOSHI_NAMESPACE_END

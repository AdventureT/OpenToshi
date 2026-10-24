#pragma once
#include <Common/Base/hkBase.h>
#include <Physics/Collide/Shape/Misc/PhantomCallback/hkpPhantomCallbackShape.h>

TOSHI_NAMESPACE_START

class TPhysicsManager;

// The phantom shape of trigger bodies (TPhysicsObjectContainer::PMF_FIXED): reports the bodies entering and leaving
// the body's shape to the physics manager
class TPhysicsTriggerShape
    : public hkpPhantomCallbackShape
{
public:
	// $deBlob: FUNCTION 007363c0
	TPhysicsTriggerShape( TPhysicsManager* a_pManager )
	    : m_pManager( a_pManager )
	{}

	// $deBlob: FUNCTION 007364f0
	virtual ~TPhysicsTriggerShape() = default;

	virtual void phantomEnterEvent( const hkpCollidable* a_pPhantomColl, const hkpCollidable* a_pOtherColl, const hkpCollisionInput& a_rEnv ) override;
	virtual void phantomLeaveEvent( const hkpCollidable* a_pPhantomColl, const hkpCollidable* a_pOtherColl ) override;

private:
	TPhysicsManager* m_pManager; // 0x10
};

TOSHI_NAMESPACE_END

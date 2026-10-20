#pragma once
#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/Collide/hkpCollisionListener.h>
#include <Physics/Dynamics/Entity/hkpEntityListener.h>

TOSHI_NAMESPACE_START

class TPhysicsManager;

//-----------------------------------------------------------------------------
// Purpose: records the contact points of a body with a responder (TPhysicsObjectContainer::m_pListener) as collision
// callbacks of the physics manager (dispatched by TPhysicsManager::DispatchCollisionCallbacks); created by
// TPhysicsManager::AddToSimulation, deletes itself when its Havok body is deleted. Name from its RTTI. NOTE: retail
// is built against Havok 5.5's listener interfaces; the 6.5 ones have the same callbacks
//-----------------------------------------------------------------------------
class TPhysicsManagerCollisionListener
    : public hkpCollisionListener
    , public hkpEntityListener
{
public:
	TPhysicsManagerCollisionListener( hkpEntity* a_pEntity, TPhysicsManager* a_pManager );

	// $deBlob: FUNCTION 00736160
	virtual ~TPhysicsManagerCollisionListener() override = default;

	// hkpCollisionListener
	virtual void contactPointAddedCallback( hkpContactPointAddedEvent& a_rEvent ) override;
	virtual void contactPointConfirmedCallback( hkpContactPointConfirmedEvent& a_rEvent ) override;
	virtual void contactPointRemovedCallback( hkpContactPointRemovedEvent& a_rEvent ) override;
	virtual void contactProcessCallback( hkpContactProcessEvent& a_rEvent ) override;

	// hkpEntityListener
	virtual void entityDeletedCallback( hkpEntity* a_pEntity ) override;

private:
	TPhysicsManager* m_pManager; // 0x08
};

TOSHI_NAMESPACE_END

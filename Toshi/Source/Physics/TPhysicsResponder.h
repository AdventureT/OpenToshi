#pragma once

TOSHI_NAMESPACE_START

class TPhysicsObjectContainer;
struct TPhysicsCollisionInfo;

// Receives the events of a physics body (TPhysicsObjectContainer::m_pListener), called by TPhysicsManager after each
// step; all handlers do nothing by default
// NOTE: handler names and argument types are guesses from TPhysicsManager's calls (argument counts from retail's RET n)
class TPhysicsResponder
{
public:
	// $deBlob: FUNCTION 005802b0
	virtual ~TPhysicsResponder() = default;

	// The collision callbacks recorded by TPhysicsManagerCollisionListener (TPhysicsManager::DispatchCollisionCallback),
	// a_pInfo is the collision as seen by this body
	// $deBlob: FUNCTION 00580660
	virtual void OnCollisionAdded( TPhysicsCollisionInfo* a_pInfo ) {} // +0x04
	// $deBlob: FUNCTION 00580670
	virtual void OnCollisionConfirmed( TPhysicsCollisionInfo* a_pInfo ) {} // +0x08
	// $deBlob: FUNCTION 00580640
	virtual void OnCollisionProcess( TPhysicsCollisionInfo* a_pInfo ) {} // +0x0c
	// Any other type (none is recorded by the listener)
	// $deBlob: FUNCTION 00580650
	virtual void OnCollisionUnk10( TPhysicsCollisionInfo* a_pInfo ) {} // +0x10

	// $deBlob: FUNCTION 00580690
	// A force was applied to the body (TPhysicsManager::ApplyForce, ApplyPointForce): the body, the force, the point of application
	virtual void OnForceApplied( void* a_pBody, void* a_pForce, void* a_pPoint ) {} // +0x14
	// $deBlob: FUNCTION 00580630
	// An acceleration was applied to the body (TPhysicsManager::ApplyForceAtPoint): the body, the acceleration, the point
	virtual void OnAccelerationApplied( void* a_pBody, void* a_pAcceleration, void* a_pPoint ) {} // +0x18

	// A constraint of the body broke (the TPhysicsConstraintContainer, this body, the other body, the force)
	// $deBlob: FUNCTION 00580680
	virtual void OnConstraintBroken( void* a_pConstraint, TPhysicsObjectContainer* a_pBody, TPhysicsObjectContainer* a_pOther, TFLOAT a_fForce ) {} // +0x1c

	// a_pOther entered/left the trigger body a_pBody
	// $deBlob: FUNCTION 005806a0
	virtual void OnTriggerEnter( TPhysicsObjectContainer* a_pBody, TPhysicsObjectContainer* a_pOther ) {} // +0x20
	// $deBlob: FUNCTION 005806b0
	virtual void OnTriggerLeave( TPhysicsObjectContainer* a_pBody, TPhysicsObjectContainer* a_pOther ) {} // +0x24
};

TOSHI_NAMESPACE_END

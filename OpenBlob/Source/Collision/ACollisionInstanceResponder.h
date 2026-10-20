#pragma once
#include <Physics/TPhysicsResponder.h>

class ACollisionInstance2;

//-----------------------------------------------------------------------------
// Purpose: the responder of a body of an ACollisionInstance2: hands the collisions of the body to the collision
// response.
//-----------------------------------------------------------------------------
class ACollisionInstanceResponder
    : public Toshi::TPhysicsResponder
{
public:
	ACollisionInstanceResponder( ACollisionInstance2* a_pInstance );

	// $deBlob: FUNCTION 005a8180
	// $deBlob: FUNCTION 005a8170
	virtual ~ACollisionInstanceResponder() override = default;

	// $deBlob: FUNCTION 005a81c0
	virtual void OnCollisionAdded( Toshi::TPhysicsCollisionInfo* a_pInfo ) override {}   // +0x04 DOES_NOTHING
	virtual void OnCollisionConfirmed( Toshi::TPhysicsCollisionInfo* a_pInfo ) override; // +0x08
	// $deBlob: FUNCTION 005a81b0
	virtual void OnCollisionProcess( Toshi::TPhysicsCollisionInfo* a_pInfo ) override {} // +0x0c DOES_NOTHING
	// $deBlob: FUNCTION 005a81d0
	virtual void OnTriggerEnter( Toshi::TPhysicsObjectContainer* a_pBody, Toshi::TPhysicsObjectContainer* a_pOther ) override {} // +0x20 DOES_NOTHING
	// $deBlob: FUNCTION 005a81e0
	virtual void OnTriggerLeave( Toshi::TPhysicsObjectContainer* a_pBody, Toshi::TPhysicsObjectContainer* a_pOther ) override {} // +0x24 DOES_NOTHING

	void SetUnk08( TINT a_iUnk08 ) { m_iUnk08 = a_iUnk08; }

protected:
	ACollisionInstance2* m_pInstance; // 0x04
	TINT                 m_iUnk08;    // 0x08 the PhysicsMotion's m_iUnk00
};

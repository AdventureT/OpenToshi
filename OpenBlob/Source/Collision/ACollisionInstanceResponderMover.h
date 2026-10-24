#pragma once
#include "ACollisionInstanceResponder.h"

class ACollisionInstanceMover;

//-----------------------------------------------------------------------------
// Purpose: the responder of a body with an ACollisionInstanceMover: the player (or an object of type 3) touching a body
// with the right collision group makes the mover act (ACollisionInstanceMover::OnHit).
//-----------------------------------------------------------------------------
class ACollisionInstanceResponderMover
    : public ACollisionInstanceResponder
{
public:
	// $deBlob: FUNCTION 005a81f0
	ACollisionInstanceResponderMover( ACollisionInstance2* a_pInstance )
	    : ACollisionInstanceResponder( a_pInstance )
	    , m_pMover( TNULL )
	{
	}

	// $deBlob: FUNCTION 005a82a0
	// $deBlob: FUNCTION 005a8290
	virtual ~ACollisionInstanceResponderMover() override = default;

	virtual void OnCollisionAdded( Toshi::TPhysicsCollisionInfo* a_pInfo ) override; // +0x04

	// $deBlob: FUNCTION 005a66f0
	void SetMover( ACollisionInstanceMover* a_pMover ) { m_pMover = a_pMover; }

private:
	ACollisionInstanceMover* m_pMover; // 0x0C
};

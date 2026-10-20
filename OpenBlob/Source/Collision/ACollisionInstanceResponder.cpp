#include "pch.h"
#include "ACollisionInstanceResponder.h"
#include "Sound/APhysicsSoundManager.h"

#include <Physics/Havok/TPhysicsManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005a8130
ACollisionInstanceResponder::ACollisionInstanceResponder( ACollisionInstance2* a_pInstance )
    : m_pInstance( a_pInstance )
    , m_iUnk08( 0 )
{
}

// $deBlob: FUNCTION 005a8150
void ACollisionInstanceResponder::OnCollisionConfirmed( TPhysicsCollisionInfo* a_pInfo )
{
	APhysicsSoundManager::GetSingleton()->OnCollision( a_pInfo, 0 );
}

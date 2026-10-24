#include "ToshiPCH.h"
#include "TPhysicsBrokenConstraintListener.h"
#include "TPhysicsManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 007356e0
void TPhysicsBrokenConstraintListener::constraintBreakingCallback( hkpBreakableConstraintEvent& a_rEvent )
{
	m_pManager->OnConstraintBroken( a_rEvent.m_constraintInstance, a_rEvent.m_forceMagnitude );
}

TOSHI_NAMESPACE_END

#pragma once
#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/Constraint/Breakable/hkpBreakableListener.h>

TOSHI_NAMESPACE_START

class TPhysicsManager;

// Queues broken constraints on the physics manager (created by TPhysicsManager::Initialise)
class TPhysicsBrokenConstraintListener
    : public hkpBreakableListener
{
public:
	HK_DECLARE_SYSTEM_ALLOCATOR();

	// $deBlob: FUNCTION 007356c0
	TPhysicsBrokenConstraintListener( TPhysicsManager* a_pManager )
	    : m_pManager( a_pManager )
	{}

	// $deBlob: FUNCTION 00735730
	virtual ~TPhysicsBrokenConstraintListener() = default;

	virtual void constraintBreakingCallback( hkpBreakableConstraintEvent& a_rEvent ) override;

private:
	TPhysicsManager* m_pManager; // 0x4
};

TOSHI_NAMESPACE_END

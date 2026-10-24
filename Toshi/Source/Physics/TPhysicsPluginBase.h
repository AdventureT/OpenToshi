#pragma once

TOSHI_NAMESPACE_START

class TPhysicsManager;

// A step listener of TPhysicsManager (m_apStepListeners): called before and after each step of its world
class TPhysicsPluginBase
{
public:
	TPhysicsPluginBase()
	    : m_iWorld( 0 )
	{}

	// $deBlob: FUNCTION 00751480
	// $deBlob: FUNCTION 007512f0
	virtual ~TPhysicsPluginBase() = default;

	virtual void OnStartStep( TFLOAT a_fTimeStep, TPhysicsManager* a_pManager ) = 0; // +0x4
	virtual void OnFinishStep( TPhysicsManager* a_pManager )                    = 0; // +0x8

	TINT GetWorld() const { return m_iWorld; }

public:
	TINT m_iWorld; // 0x4
};

TOSHI_NAMESPACE_END

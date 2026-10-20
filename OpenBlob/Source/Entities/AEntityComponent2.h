#pragma once
#include <Toshi/T2DList.h>
#include "Entities/AEntityIOPacket.h"

class ABaseEntity;

//-----------------------------------------------------------------------------
// Purpose: the base of the entity components: the model, pose, state, FX breakpoint, sound, AI... Components an entity
// owns and ticks.
//-----------------------------------------------------------------------------
class AEntityComponent2
    : public Toshi::T2DList<AEntityComponent2>::Node
{
public:
	AEntityComponent2( ABaseEntity* a_pEntity );
	virtual ~AEntityComponent2(); // +0x00

	// DOES_NOTHING (shared stub)
	// $deBlob: FUNCTION 007c4390
	virtual void Tick( TFLOAT a_fDeltaTime ) {} // +0x04
	// DOES_NOTHING (shared stub)
	// $deBlob: FUNCTION 0075d480
	virtual void ResetComponent() {} // +0x08
	// Shared by every class with a TBOOL at 0x10
	// $deBlob: FUNCTION 0075d470
	virtual TBOOL IsEnabled() { return m_bEnabled; } // +0x0C
	// DOES_NOTHING (shared stub)
	// $deBlob: FUNCTION 0075d460
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) {} // +0x10

	ABaseEntity* GetEntity() const { return m_pEntity; }
	void         SetEnabled( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }

protected:
	ABaseEntity* m_pEntity;  // 0x0C
	TBOOL        m_bEnabled; // 0x10
};                           // 0x14

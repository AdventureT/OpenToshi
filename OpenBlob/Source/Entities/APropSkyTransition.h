#pragma once
#include "Entities/ABaseEntity.h"

//-----------------------------------------------------------------------------
// Purpose: the trigger of the sky transition to space (no members of its own): MESSAGE_TRIGGER starts the environment's
// space sky (AEnvironmentFX::StartSkySpace).
//-----------------------------------------------------------------------------
class APropSkyTransition
    : public ABaseEntity
{
public:
	TDECLARE_CLASS( APropSkyTransition, ABaseEntity );

public:
	APropSkyTransition();
	virtual ~APropSkyTransition() override; // +0x08

	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
};

#pragma once
#include "Entities/APropTrigger.h"

//-----------------------------------------------------------------------------
// Purpose: an instant death volume: kills the blob that triggers it (sends it the kill message, AEntityIOPacket
// MESSAGE_DEATHZONE), e.g. the sea around the levels (no members of its own).
//-----------------------------------------------------------------------------
class APropTriggerInstantDeath
    : public APropTrigger
{
public:
	TDECLARE_CLASS( APropTriggerInstantDeath, APropTrigger );

public:
	APropTriggerInstantDeath();
	virtual ~APropTriggerInstantDeath() override; // +0x08

	virtual TBOOL CanObjectTrigger( Toshi::TObject* a_pObject ) override; // +0xE0
	virtual void  Trigger() override;                                     // +0xE4
};

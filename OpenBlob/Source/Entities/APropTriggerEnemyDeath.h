#pragma once
#include "Entities/APropTrigger.h"

//-----------------------------------------------------------------------------
// Purpose: a volume that kills the enemies that trigger it (sends them the kill message, AEntityIOPacket
// MESSAGE_DEATHZONE, no members of its own).
//-----------------------------------------------------------------------------
class APropTriggerEnemyDeath
    : public APropTrigger
{
public:
	TDECLARE_CLASS( APropTriggerEnemyDeath, APropTrigger );

public:
	APropTriggerEnemyDeath();
	virtual ~APropTriggerEnemyDeath() override; // +0x08

	virtual TBOOL CanObjectTrigger( Toshi::TObject* a_pObject ) override; // +0xE0
	virtual void  Trigger() override;                                     // +0xE4
};

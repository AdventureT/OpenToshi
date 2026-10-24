#pragma once
#include "Entities/AProp.h"

//-----------------------------------------------------------------------------
// Purpose: a switch prop (plus ATriggerUtil's 6 at 0x9C): MESSAGE_PLAYERINTERACT triggers its targets (only once with
// "TriggerOnce"); it can be interacted with.
// NOTE: names from the entity params ("TriggerOnce")
//-----------------------------------------------------------------------------
class APropSwitch
    : public AProp
{
public:
	TDECLARE_CLASS( APropSwitch, AProp );

public:
	APropSwitch();
	virtual ~APropSwitch() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C (also ATriggerUtil +0x04)
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C

	// $deBlob: FUNCTION 0079a910
	virtual TBOOL IsInteractable() override { return TTRUE; } // +0x94

private:
	TBOOL m_bTriggered;   // 0xB4
	TBOOL m_bTriggerOnce; // 0xB5 "TriggerOnce"
};

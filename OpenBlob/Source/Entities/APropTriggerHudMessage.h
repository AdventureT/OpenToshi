#pragma once
#include "Entities/APropTriggerHelp.h"

//-----------------------------------------------------------------------------
// Purpose: a help trigger that shows a HUD message when the blob interacts with it.
// NOTE: names from the entity params ("InteractionType")
//-----------------------------------------------------------------------------
class APropTriggerHudMessage
    : public APropTriggerHelp
{
public:
	TDECLARE_CLASS( APropTriggerHudMessage, APropTriggerHelp );

public:
	APropTriggerHudMessage();
	virtual ~APropTriggerHudMessage() override; // +0x08

	// $deBlob: FUNCTION 0079c3c0
	// $deBlob: FUNCTION 0079c5b7
	virtual void PostCreate() override { APropTriggerHelp::PostCreate(); }                            // +0x20
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual void Trigger() override;                                                                  // +0xE4

	// $deBlob: FUNCTION 006564b0
	TINT8 GetInteractionType() const { return m_iInteractionType; }

private:
	TINT8 m_iInteractionType; // 0xD0 "InteractionType", -1: a plain help trigger
};

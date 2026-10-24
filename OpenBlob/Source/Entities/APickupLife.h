#pragma once
#include "Entities/APickup.h"

//-----------------------------------------------------------------------------
// Purpose: an extra life, pickup type 4. Its model is "PICKUP_Blob".
//-----------------------------------------------------------------------------
class APickupLife
    : public APickup
{
public:
	TDECLARE_CLASS( APickupLife, APickup );

public:
	APickupLife();
	virtual ~APickupLife() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C (also ATriggerUtil +0x04)
	virtual void InitPooled( TINT a_eType ) override;                      // +0x30
	virtual void OnPoolRelease() override;                                 // +0xD0
	virtual void OnCollected( ABaseEntity* a_pCollector ) override;        // +0xDC

	ListNode& GetLifeNode() { return m_oLifeNode; }

private:
	ListNode m_oLifeNode; // 0x15C
};

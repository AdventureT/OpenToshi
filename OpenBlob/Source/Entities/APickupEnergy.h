#pragma once
#include "Entities/APickup.h"

//-----------------------------------------------------------------------------
// Purpose: gives the collecting blob energy, pickup type 5.
// NOTE: names from the entity params ("Energy")
//-----------------------------------------------------------------------------
class APickupEnergy
    : public APickup
{
public:
	TDECLARE_CLASS( APickupEnergy, APickup );

public:
	APickupEnergy();
	virtual ~APickupEnergy() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C (also ATriggerUtil +0x04)
	virtual void OnCollected( ABaseEntity* a_pCollector ) override;        // +0xDC

	// $deBlob: FUNCTION 007b0fa0
	TINT16 GetEnergy() const { return m_iEnergy; }

private:
	TINT16 m_iEnergy; // 0x15C
};

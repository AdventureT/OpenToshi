#pragma once
#include "Entities/APickup.h"

//-----------------------------------------------------------------------------
// Purpose: a power-up pickup (APickupPowerup\APickupPowerUp.cpp), pickup type 0: one of PT_POWERUPS_COUNT power-ups
// ("PickupType") lasting "EffectTime"; collecting it starts it in the collector's APlayerManager::BlobData.
//-----------------------------------------------------------------------------
class APickupPowerUp
    : public APickup
{
public:
	TDECLARE_CLASS( APickupPowerUp, APickup );

	static TCONSTEXPR_VAR TINT PT_POWERUPS_COUNT = 4; // APickupTypes

public:
	APickupPowerUp();
	virtual ~APickupPowerUp() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C (also ATriggerUtil +0x04)
	virtual void OnCollected( ABaseEntity* a_pCollector ) override;        // +0xDC

	// $deBlob: FUNCTION 007afba0
	TINT GetPowerUpType() const
	{
		TASSERT( m_iPowerUpType >= 0 && m_iPowerUpType < PT_POWERUPS_COUNT );
		return m_iPowerUpType;
	}

	// $deBlob: FUNCTION 007afb90
	TFLOAT GetEffectTime() const { return m_fEffectTime; }

private:
	TINT8  m_iPowerUpType; // 0x15C -1
	TFLOAT m_fEffectTime;  // 0x160 -1
};

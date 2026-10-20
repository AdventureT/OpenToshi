#pragma once
#include "Entities/APickup.h"

//-----------------------------------------------------------------------------
// Purpose: a bonus pickup (APickupBonus\APickupBonus.cpp), pickup type 1: one of the PT_BONUS_COUNT pickups of one of
// the PT_TOTAL_BONUS_SETS sets ("PickupType", "BonusSet"); collecting it sets its bit in the collector's
// APlayerManager::BlobData, a completed set gives 2000 points.
//-----------------------------------------------------------------------------
class APickupBonus
    : public APickup
{
public:
	TDECLARE_CLASS( APickupBonus, APickup );

	static TCONSTEXPR_VAR TINT PT_BONUS_COUNT      = 5; // APickupTypes
	static TCONSTEXPR_VAR TINT PT_TOTAL_BONUS_SETS = 8; // APickupTypes

public:
	APickupBonus();
	virtual ~APickupBonus() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C (also ATriggerUtil +0x04)
	virtual void OnCollected( ABaseEntity* a_pCollector ) override;        // +0xDC

	// $deBlob: FUNCTION 007b04d0
	TINT GetBonusType() const { return m_iBonusType; }

	// $deBlob: FUNCTION 007b04c0
	TINT8 GetBonusSet() const { return m_iBonusSet; }

private:
	TINT8 m_iBonusType; // 0x15C -1
	TINT8 m_iBonusSet;  // 0x15D -1
};

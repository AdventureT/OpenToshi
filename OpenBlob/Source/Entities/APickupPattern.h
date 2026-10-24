#pragma once
#include "Entities/APickup.h"

//-----------------------------------------------------------------------------
// Purpose: a pickup that unlocks one of the building patterns.
//-----------------------------------------------------------------------------
class APickupPattern
    : public APickup
{
public:
	TDECLARE_CLASS( APickupPattern, APickup );

	static TCONSTEXPR_VAR TINT PT_PATTERN_COUNT_NORMAL = 16; // APickupTypes

public:
	APickupPattern();
	virtual ~APickupPattern() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C (also ATriggerUtil +0x04)
	virtual void OnCollected( ABaseEntity* a_pCollector ) override;        // +0xDC

	// $deBlob: FUNCTION 007affc0
	TINT GetPatternType() const
	{
		TASSERT( m_iPatternType >= 0 && m_iPatternType < PT_PATTERN_COUNT_NORMAL );
		return m_iPatternType;
	}

private:
	TINT8 m_iPatternType; // 0x15C -1
};

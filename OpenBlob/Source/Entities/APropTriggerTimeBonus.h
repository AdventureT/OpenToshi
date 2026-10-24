#pragma once
#include "Entities/APickup.h"

//-----------------------------------------------------------------------------
// Purpose: adds seconds to the level's time limit (APropTriggerTimeBonus\APropTriggerTimeBonus.cpp), pickup type 6.
// Despite its name it's an APickup.
//-----------------------------------------------------------------------------
class APropTriggerTimeBonus
    : public APickup
{
public:
	TDECLARE_CLASS( APropTriggerTimeBonus, APickup );

	enum TYPE
	{
		TYPE_30Seconds,
		TYPE_60Seconds,
		TYPE_180Seconds,
	};

public:
	APropTriggerTimeBonus();
	virtual ~APropTriggerTimeBonus() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C (also ATriggerUtil +0x04)
	virtual void InitPooled( TINT a_eType ) override;                      // +0x30
	virtual void OnCollected( ABaseEntity* a_pCollector ) override;        // +0xDC

	// $deBlob: FUNCTION 00751a90
	TINT GetSeconds() const { return m_iSeconds; }

private:
	TWCHAR m_wszMessage[ 0x80 ]; // 0x15C the HUD message
	TINT   m_iType;              // 0x25C TYPE
	TINT   m_iSeconds;           // 0x260
};

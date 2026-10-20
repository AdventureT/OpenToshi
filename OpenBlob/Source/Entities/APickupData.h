#pragma once
#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Purpose: the pickup settings: the "Pickups_data_EntityData" symbol of the entity data TRB. Created by
// AEntityFramework::Startup.
//-----------------------------------------------------------------------------
class APickupData
    : public Toshi::TSingleton<APickupData>
{
public:
	// The layout of Pickups_data_EntityData (only the read fields are mapped)
	struct Data
	{
		TFLOAT m_fUnk00;       // 0x00
		TFLOAT m_fUnk04;       // 0x04 a distance (squared by APickup::OnDataLoad)
		TFLOAT m_afUnk08[ 4 ]; // 0x08
		TFLOAT m_fDropTime;    // 0x18 the drop flight's time out (APickup::Drop)
	};

public:
	APickupData();
	~APickupData();

	const Data* GetData() const { return m_pData; }

	// Creates a pickup of the type a_eType (AIOEntity's callback 0x43, AEnemy's drop)
	// NOTE: empty on PC (returns TNULL), no pickup is ever created this way
	static class APickup* CreatePickup( TINT a_eType );

private:
	TUINT32 m_uiUnk00; // 0x00
	TUINT32 m_uiUnk04; // 0x04
	Data*   m_pData;   // 0x08
};

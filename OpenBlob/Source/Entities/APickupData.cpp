#include "pch.h"
#include "APickupData.h"
#include "AEntityData.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APickupData ) == 0xC );

// $deBlob: FUNCTION 007dfa10
APickupData::APickupData()
{
	m_uiUnk00 = 0;
	m_uiUnk04 = 0;
	m_pData   = TSTATICCAST( Data, AEntityData::GetSingleton()->GetEntityData( "Pickups_data_EntityData" ) );
}

// $deBlob: FUNCTION 007dfab0
// $deBlob: FUNCTION 0078da60
APickupData::~APickupData()
{
	m_pData = TNULL;
}

// $deBlob: FUNCTION 007dfb10
APickup* APickupData::CreatePickup( TINT a_eType )
{
	return TNULL;
}

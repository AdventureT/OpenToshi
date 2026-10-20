#include "pch.h"
#include "APropTriggerInstantDeath.h"
#include "AEntityIOPacket.h"
#include "ABlobEntity/ABlobEntity.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropTriggerInstantDeath ) == 0xC0 );

// $deBlob: FUNCTION 0079ca90
// $deBlob: FUNCTION 0079caa0
// $deBlob: FUNCTION 0079cb20
// $deBlob: FUNCTION 0079cba0
// $deBlob: FUNCTION 0079cbb0
// $deBlob: FUNCTION 00540e60
// $deBlob: FUNCTION 00859e20
TDEFINE_CLASS( APropTriggerInstantDeath );

// $deBlob: FUNCTION 0079cbc0
APropTriggerInstantDeath::APropTriggerInstantDeath()
{
}

// $deBlob: FUNCTION 0079cbe0
// $deBlob: FUNCTION 0079cca5
// $deBlob: FUNCTION 0079ccb0
APropTriggerInstantDeath::~APropTriggerInstantDeath()
{
}

// $deBlob: FUNCTION 0079cc70
TBOOL APropTriggerInstantDeath::CanObjectTrigger( TObject* a_pObject )
{
	return APropTrigger::CanObjectTrigger( a_pObject ) && a_pObject->IsA( &TGetClass( ABlobEntity ) );
}

// $deBlob: FUNCTION 0079cc00
void APropTriggerInstantDeath::Trigger()
{
	AEntityIOPacket oPacket;
	oPacket.m_eMessage = AEntityIOPacket::MESSAGE_DEATHZONE;
	oPacket.m_iData    = 0;

	ABaseEntity* pEntity = TREINTERPRETCAST( ABaseEntity*, m_uiTriggeredBy );
	TASSERT( pEntity, "Instant death volume triggered with no calling entity" );

	if ( pEntity )
		pEntity->HandleEntityIO( this, oPacket );
}

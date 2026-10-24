#include "pch.h"
#include "APropTriggerEnemyDeath.h"
#include "AEntityIOPacket.h"
#include "Enemies/AEnemy.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropTriggerEnemyDeath ) == 0xC0 );

// $deBlob: FUNCTION 0079cd00
// $deBlob: FUNCTION 0079cd10
// $deBlob: FUNCTION 0079cd90
// $deBlob: FUNCTION 0079ce10
// $deBlob: FUNCTION 0079ce20
// $deBlob: FUNCTION 00540ea0
// $deBlob: FUNCTION 00859e30
TDEFINE_CLASS( APropTriggerEnemyDeath );

// $deBlob: FUNCTION 0079ce30
APropTriggerEnemyDeath::APropTriggerEnemyDeath()
{
}

// $deBlob: FUNCTION 0079ce50
// $deBlob: FUNCTION 0079cebf
APropTriggerEnemyDeath::~APropTriggerEnemyDeath()
{
}

// $deBlob: FUNCTION 0079cea0
TBOOL APropTriggerEnemyDeath::CanObjectTrigger( TObject* a_pObject )
{
	// NOTE: retail doesn't call APropTrigger::CanObjectTrigger
	return a_pObject->IsA( &TGetClass( AEnemy ) ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 0079ce70
void APropTriggerEnemyDeath::Trigger()
{
	AEntityIOPacket oPacket;
	oPacket.m_eMessage = AEntityIOPacket::MESSAGE_DEATHZONE;
	oPacket.m_iData    = 0;

	// NOTE: unlike APropTriggerInstantDeath, no check of the calling entity
	TREINTERPRETCAST( ABaseEntity*, m_uiTriggeredBy )->HandleEntityIO( this, oPacket );
}

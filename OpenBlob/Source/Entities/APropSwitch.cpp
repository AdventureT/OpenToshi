#include "pch.h"
#include "APropSwitch.h"
#include "AEntityParams.h"
#include "AEntityIOPacket.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropSwitch ) == 0xB8 );

// $deBlob: FUNCTION 0079a6b0
// $deBlob: FUNCTION 0079a6c0
// $deBlob: FUNCTION 0079a750
// $deBlob: FUNCTION 0079a7d0
// $deBlob: FUNCTION 0079a7e0
// $deBlob: FUNCTION 00540d20
// $deBlob: FUNCTION 00859dd0
TDEFINE_CLASS( APropSwitch );

// $deBlob: FUNCTION 0079a7f0
APropSwitch::APropSwitch()
{
	m_bTriggered   = TFALSE;
	m_bTriggerOnce = TFALSE;
}

// $deBlob: FUNCTION 0079a820
// $deBlob: FUNCTION 0079a8b9
APropSwitch::~APropSwitch()
{
}

// $deBlob: FUNCTION 0079a840
// $deBlob: FUNCTION 0079a913
void APropSwitch::ProcessParams( const AEntityParams& a_rParams )
{
	AProp::ProcessParams( a_rParams );
	a_rParams.GetValue( m_bTriggerOnce, "TriggerOnce" );
}

// $deBlob: FUNCTION 0079a870
void APropSwitch::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_PLAYERINTERACT && ( !m_bTriggered || !m_bTriggerOnce ) )
	{
		m_bTriggered = TTRUE;
		TriggerAllTargets( this );
	}

	AProp::HandleEntityIO( a_pCaller, a_rPacket );
}

#include "pch.h"
#include "APropTriggerHudMessage.h"
#include "AEntityParams.h"
#include "AEntityIOPacket.h"
#include "ABlobEntity/ABlobEntity.h"
#include "ABlobEntity/ABlobZJumpComponent.h"
#include "GameInterface/APlayerManager.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropTriggerHudMessage ) == 0xD4 );

// $deBlob: FUNCTION 0079c280
// $deBlob: FUNCTION 0079c290
// $deBlob: FUNCTION 0079c320
// $deBlob: FUNCTION 0079c3a0
// $deBlob: FUNCTION 0079c3b0
// $deBlob: FUNCTION 00540de0
// $deBlob: FUNCTION 00859e00
TDEFINE_CLASS( APropTriggerHudMessage );

// $deBlob: FUNCTION 0079c520
APropTriggerHudMessage::APropTriggerHudMessage()
{
	m_iInteractionType = -1;
}

// $deBlob: FUNCTION 0079c570
// $deBlob: FUNCTION 0079c550
// $deBlob: FUNCTION 0079c565
APropTriggerHudMessage::~APropTriggerHudMessage()
{
}

// $deBlob: FUNCTION 0079c3d0
// $deBlob: FUNCTION 0079c5c2
void APropTriggerHudMessage::ProcessParams( const AEntityParams& a_rParams )
{
	a_rParams.GetValue( m_iInteractionType, "InteractionType" );
	APropTriggerHelp::ProcessParams( a_rParams );
}

// $deBlob: FUNCTION 0079c400
void APropTriggerHudMessage::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( m_iInteractionType != -1 )
	{
		if ( TINT( a_rPacket.m_eMessage ) == 2 )
		{
			// Blob left: the message is no longer its
			if ( TUINT8( a_pCaller->GetFlags() >> 4 ) & 1 )
			{
				TSTATICCAST( ABlobEntity, a_pCaller )->m_pHudMessageTrigger = TNULL;
				return;
			}
		}
		else if ( TINT( a_rPacket.m_eMessage ) == 7 && ( TUINT8( a_pCaller->GetFlags() >> 4 ) & 1 ) )
		{
			if ( TSTATICCAST( ABlobEntity, a_pCaller )->m_pZJump->IsLockedOnLevelCompleteTrigger() )
			{
				TSTATICCAST( ABlobEntity, a_pCaller )->m_pHudMessageTrigger = TNULL;
				TPhysicsManager::GetSingleton()->RemoveFromSimulation( m_pPhysicsData->m_pBody, TFALSE );
				return;
			}
		}

		APropTrigger::HandleEntityIO( a_pCaller, a_rPacket );
		return;
	}

	APropTriggerHelp::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 0079c4e0
void APropTriggerHudMessage::Reset( ABaseEntity* a_pEntity )
{
	if ( m_iInteractionType != -1 )
	{
		TPhysicsManager::GetSingleton()->AddToSimulation( m_pPhysicsData->m_pBody, TFALSE );
	}

	APropTriggerHelp::Reset( a_pEntity );
}

// $deBlob: FUNCTION 0079c4b0
void APropTriggerHudMessage::Trigger()
{
	if ( m_iInteractionType != -1 )
	{
		TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) )->m_pHudMessageTrigger = this;
		return;
	}

	APropTriggerHelp::Trigger();
}

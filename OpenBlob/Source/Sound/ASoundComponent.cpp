#include "pch.h"
#include "ASoundComponent.h"
#include "Sound/ASoundInterface.h"

#include <fmod_event.hpp>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007cc190
ASoundComponent::ASoundComponent( AIOEntity* a_pOwner )
    : AIOEntityComponent( a_pOwner, TYPE )
{
	m_pEvent   = TNULL;
	m_eGroupID = ASoundMap::GROUPID_INVALID;
	m_eEventID = ASoundMap::EVENTID_INVALID;
	m_iState   = 0;
	m_uiFlags |= 1;
}

// $deBlob: FUNCTION 007cc3c0
// $deBlob: FUNCTION 007cc1d0
ASoundComponent::~ASoundComponent()
{
	if ( m_pEvent != TNULL )
		m_pEvent->stop( true );
}

// $deBlob: FUNCTION 007cc240
void ASoundComponent::SetIOEntityData( void* a_pData, TTRB* a_pTRB )
{
	IOEntityData* pIOData = TSTATICCAST( IOEntityData, a_pData );

	TBOOL bCorrectType = pIOData->iType == TYPE;
	TASSERT( TTRUE == bCorrectType );

	Data* pData = pIOData->pData;
	m_eEventID  = pData->eEventID;
	m_eGroupID  = pData->eGroupID;
	m_iState    = pData->iState;
}

// $deBlob: FUNCTION 007cc2a0
TBOOL ASoundComponent::ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther )
{
	if ( !( m_uiFlags & 1 ) )
		return TFALSE;

	if ( a_pMessage[ 0 ] == ACTION_Play )
	{
		if ( m_iState != 1 )
			TASSERT( TNULL == m_pEvent );

		m_pEvent = ASoundInterface::PlayEvent( m_eGroupID, m_eEventID, FMOD_EVENT_DEFAULT, TTRUE );
		m_iState = 1;
		return TTRUE;
	}

	if ( a_pMessage[ 0 ] == ACTION_Stop )
	{
		if ( m_pEvent != TNULL )
		{
			m_pEvent->stop( false );
			m_pEvent = TNULL;
			m_iState = 0;
		}

		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007cc350
void ASoundComponent::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_iState == 1 && m_pEvent == TNULL )
	{
		m_pEvent = ASoundInterface::PlayEvent( m_eGroupID, m_eEventID, FMOD_EVENT_DEFAULT, TTRUE );
		m_iState = 1;
	}
}

// $deBlob: FUNCTION 007cc380
void ASoundComponent::SetPlaying( TBOOL a_bPlaying )
{
	m_iState = a_bPlaying != TFALSE;
}

// $deBlob: FUNCTION 007cc3a0
void ASoundComponent::Play()
{
	m_pEvent = ASoundInterface::PlayEvent( m_eGroupID, m_eEventID, FMOD_EVENT_DEFAULT, TTRUE );
	m_iState = 1;
}

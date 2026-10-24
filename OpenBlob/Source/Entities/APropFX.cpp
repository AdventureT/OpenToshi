#include "pch.h"
#include "APropFX.h"
#include "AEntityParams.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropFX ) == 0x174 );

// $deBlob: FUNCTION 007a1d40
// $deBlob: FUNCTION 007a1d50
// $deBlob: FUNCTION 007a1df0
// $deBlob: FUNCTION 007a1e80
// $deBlob: FUNCTION 007a1e90
// $deBlob: FUNCTION 00540fa0
// $deBlob: FUNCTION 00859e70
TDEFINE_CLASS( APropFX );

// $deBlob: FUNCTION 007a1ea0
APropFX::APropFX()
{
	m_uiFXFlags = 0;
	m_bOn       = TFALSE;
}

// $deBlob: FUNCTION 007a2220
// $deBlob: FUNCTION 007a1f10
APropFX::~APropFX()
{
	if ( !m_oFX.IsFinished() )
		m_oFX.StopInstant();
}

// $deBlob: FUNCTION 007a1f90
void APropFX::Tick( TFLOAT a_fDeltaTime )
{
	ABaseEntity::Tick( a_fDeltaTime );
	m_oFX.Update( a_fDeltaTime );

	TUINT32 uiLOD = GetLOD();

	if ( uiLOD == FLAG_MediumLOD || uiLOD == FLAG_HighLOD )
	{
		if ( !( m_uiFXFlags & FXFLAG_On ) )
		{
			if ( !m_oFX.IsFinished() )
				m_oFX.Stop();
		}
		else if ( m_oFX.IsFinished() )
		{
			if ( ( m_uiFXFlags & FXFLAG_Triggered ) && ( m_uiFXFlags & FXFLAG_StopAfterTriggered ) )
			{
				m_uiFXFlags &= ~( FXFLAG_Triggered | FXFLAG_On );
				return;
			}

			m_oFX.TriggerEffect( m_oMatrix );
			m_uiFXFlags |= FXFLAG_Triggered;
		}
	}
}

// $deBlob: FUNCTION 007a2190
void APropFX::OnLODChanging( TINT a_iLOD )
{
	if ( a_iLOD & FLAG_LowLOD )
	{
		if ( !m_oFX.IsFinished() )
			m_oFX.StopInstant();

		m_uiFXFlags &= ~FXFLAG_Triggered;
	}
}

// $deBlob: FUNCTION 007a2110
void APropFX::ProcessParams( const AEntityParams& a_rParams )
{
	ABaseEntity::ProcessParams( a_rParams );

	TBOOL bStopAfterTriggered = TFALSE;
	a_rParams.GetValue( bStopAfterTriggered, "StopAfterTriggered" );

	if ( bStopAfterTriggered == TTRUE )
		m_uiFXFlags |= FXFLAG_StopAfterTriggered;

	a_rParams.GetValue( m_bOn, "On" );

	if ( m_bOn == TTRUE )
		m_uiFXFlags |= FXFLAG_On;

	const TCHAR* szEffectName;
	a_rParams.GetString( szEffectName, "EffectName" );
	m_oFX.Initialize( szEffectName );
}

// $deBlob: FUNCTION 007a2080
void APropFX::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_TOGGLE:
			if ( m_uiFXFlags & FXFLAG_On )
				m_uiFXFlags &= ~FXFLAG_On;
			else
				m_uiFXFlags |= FXFLAG_On;
			break;
		case AEntityIOPacket::MESSAGE_STOP:
			m_uiFXFlags &= ~FXFLAG_On;
			break;
		case AEntityIOPacket::MESSAGE_ENTER:
			m_uiFXFlags |= FXFLAG_On;
			break;
	}
}

// $deBlob: FUNCTION 007a21c0
void APropFX::Reset( ABaseEntity* a_pEntity )
{
	if ( !m_oFX.IsFinished() )
		m_oFX.StopInstant();

	if ( m_bOn )
		m_uiFXFlags |= FXFLAG_On;
	else
		m_uiFXFlags &= ~FXFLAG_On;

	m_uiFXFlags &= ~FXFLAG_Triggered;
	AInvisibleEntity::Reset( a_pEntity );
}

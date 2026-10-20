#include "ToshiPCH.h"
#include "TInputInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: TDEFINE_CLASS generates GetClass, CreateObject, CreateObjectInPlace (abstract: both return TNULL)
// $deBlob: FUNCTION 00679b30
// $deBlob: FUNCTION 00679b40
// $deBlob: FUNCTION 00679b50
// $deBlob: FUNCTION 00679b60
// $deBlob: FUNCTION 00679b70
// $deBlob: FUNCTION 0053e890
// $deBlob: FUNCTION 00858e10
TDEFINE_CLASS_NORUNTIME( TInputDevice );

// $deBlob: FUNCTION 00679b80
// $Barnyard: FUNCTION 006d98e0
TInputDevice::TInputDevice()
    : m_Repeats( 0, 16 ), m_aDoodads( 0, 16 )
{
	// NOTE: the TArray constructors allocate their buffers
	m_bUnknown        = TFALSE;
	m_bIsAcquired     = TFALSE;
	m_pInputInterface = TNULL;

	// NOTE: there's no bounds check, a 15th device overwrites s_uiDeviceCount
	m_uiDeviceIndex               = s_uiDeviceCount++;
	s_aDevices[ m_uiDeviceIndex ] = this;
}

// $deBlob: FUNCTION 00679cb0
// $Barnyard: FUNCTION 006d9700
void TInputDevice::SetInputInterface( TInputInterface* a_pInterface )
{
	m_pInputInterface = a_pInterface;
}

// $deBlob: FUNCTION 0067a9f0
TBOOL TInputDevice::Flush()
{
	return TTRUE;
}

// $deBlob: FUNCTION 00679c40
// $Barnyard: FUNCTION 006d9710
TInputDevice::~TInputDevice()
{
	// NOTE: the device stays registered in s_aDevices
}

// $deBlob: FUNCTION 0067a2e0
// $Barnyard: FUNCTION 006d9740
TINT TInputDevice::ProcessRepeats( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	TINT iNumProcessed = 0;

	for ( TINT i = 0; i < m_Repeats.Size(); )
	{
		auto& rRepeat = m_Repeats[ i ];

		if ( IsDown( rRepeat.iDoodad ) )
		{
			rRepeat.fLeftTime -= a_fDeltaTime;

			if ( rRepeat.fLeftTime <= 0.0f )
			{
				rRepeat.fLeftTime = rRepeat.fRepeatTime;
				ThrowRepeatEvent( a_rEmitter, &rRepeat, a_fDeltaTime );
				iNumProcessed += 1;
			}

			i++;
		}
		else
		{
			// The last one takes its place and gets checked next
			if ( m_Repeats.Size() > 1 )
			{
				auto& rLastRepeat   = m_Repeats[ m_Repeats.Size() - 1 ];
				rRepeat.iDoodad     = rLastRepeat.iDoodad;
				rRepeat.fLeftTime   = rLastRepeat.fLeftTime;
				rRepeat.fRepeatTime = rLastRepeat.fRepeatTime;
			}

			if ( 0 < m_Repeats.Size() )
			{
				m_Repeats.Pop();
			}
		}
	}

	return iNumProcessed;
}

// $deBlob: FUNCTION 00679cc0
// $Barnyard: FUNCTION 006d9950
TBOOL TInputDevice::StartRepeat( Doodad a_iDoodad, TFLOAT a_fFirstRepeatTime, TFLOAT a_fRepeatTime )
{
	for ( auto it = m_Repeats.Begin(); it.IsOver() == TFALSE; ++it )
	{
		if ( it->iDoodad == a_iDoodad )
		{
			it->fRepeatTime = a_fRepeatTime;
			return TTRUE;
		}
	}

	RepeatInfo oRepeat;
	oRepeat.iDoodad     = a_iDoodad;
	oRepeat.fLeftTime   = a_fFirstRepeatTime;
	oRepeat.fRepeatTime = a_fRepeatTime;
	m_Repeats.Push( oRepeat );

	return TTRUE;
}

// $deBlob: FUNCTION 00679e20
// $Barnyard: FUNCTION 006d9a00
void TInputDevice::StopRepeat( Doodad a_iDoodad )
{
	if ( m_Repeats.Size() != 0 )
	{
		auto it = m_Repeats.Begin();

		TINT iIndex = 0;
		while ( it->iDoodad != a_iDoodad )
		{
			++it;
			++iIndex;
			if ( it.IsOver() ) return;
		}

		if ( 1 < m_Repeats.Size() )
		{
			auto& rLastRepeat = m_Repeats[ m_Repeats.Size() - 1 ];
			it->iDoodad       = rLastRepeat.iDoodad;
			it->fLeftTime     = rLastRepeat.fLeftTime;
			it->fRepeatTime   = rLastRepeat.fRepeatTime;
		}

		if ( 0 < m_Repeats.Size() )
		{
			m_Repeats.Pop();
		}
	}
}

// $deBlob: FUNCTION 00679ec0
// $Barnyard: FUNCTION 006d9a70
void TInputDevice::StopAllRepeats()
{
	// NOTE: TArray::Clear frees the buffer
	m_Repeats.Clear();
}

// $deBlob: FUNCTION 00679aa0
TBOOL TInputDevice::IsForceFeedbackDevice()
{
	return TFALSE;
}

// $deBlob: FUNCTION 0067a2a0
// $Barnyard: FUNCTION 006d9800
void TInputDevice::ThrowRepeatEvent( EventEmitter& emitter, RepeatInfo* repeatInfo, TFLOAT flt )
{
	emitter.Throw(
	    TInputInterface::InputEvent(
	        this,
	        repeatInfo->iDoodad,
	        TInputInterface::EVENT_TYPE_REPEAT
	    )
	);
}

// $deBlob: FUNCTION 00679f00
TBOOL TInputDevice::AddDoodad( Doodad a_iDoodad )
{
	for ( auto it = m_aDoodads.Begin(); it.IsOver() == TFALSE; ++it )
	{
		if ( *it == a_iDoodad )
			return TTRUE;
	}

	m_aDoodads.Push( a_iDoodad );
	return TTRUE;
}

// $deBlob: FUNCTION 0067a030
void TInputDevice::RemoveDoodad( Doodad a_iDoodad )
{
	if ( m_aDoodads.Size() != 0 )
	{
		auto it = m_aDoodads.Begin();

		while ( *it != a_iDoodad )
		{
			++it;
			if ( it.IsOver() ) return;
		}

		if ( 1 < m_aDoodads.Size() )
			*it = m_aDoodads[ m_aDoodads.Size() - 1 ];

		if ( 0 < m_aDoodads.Size() )
			m_aDoodads.Pop();
	}
}

// $deBlob: FUNCTION 0067a0b0
// NOTE: retail's takes an unused stack argument (ret 4)
void TInputDevice::ClearDoodads( TINT a_iUnused )
{
	// NOTE: unlike TArray::Clear it always frees the buffer
	m_aDoodads.FreeData();
}

// $deBlob: FUNCTION 0067a0e0
TBOOL TInputDevice::HasDoodad( Doodad a_iDoodad ) const
{
	for ( TINT i = 0; i < m_aDoodads.Size(); i++ )
	{
		if ( m_aDoodads[ i ] == a_iDoodad )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00679ae0
TBOOL TInputDevice::IsGamepadTypeConnected( GAMEPADTYPE a_eGamepadType )
{
	for ( TINT i = 0; i < s_uiDeviceCount; i++ )
	{
		TInputDevice* pDevice = s_aDevices[ i ];

		if ( pDevice->GetGamepadType() == a_eGamepadType && pDevice->IsEnabled() )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00679ac0
TInputDevice::GAMEPADTYPE TInputDevice::GetLastGamepadType()
{
	return ms_eLastGamepadType;
}

// $deBlob: FUNCTION 00679ad0
// $deBlob: FUNCTION 0067a400
void TInputDevice::SetLastGamepadType( GAMEPADTYPE a_eGamepadType )
{
	ms_eLastGamepadType = a_eGamepadType;
}

// $deBlob: FUNCTION 0067a140
void TInputDevice::SavePlayerDevices()
{
	TUtil::MemCopy( ms_aSavedPlayerDevices, ms_aPlayerDevices, sizeof( ms_aPlayerDevices ) );
}

// $deBlob: FUNCTION 0067a160
void TInputDevice::RestorePlayerDevices()
{
	if ( ms_aSavedPlayerDevices[ 0 ] > -1 || ms_aSavedPlayerDevices[ 1 ] > -1 || ms_aSavedPlayerDevices[ 2 ] > -1 || ms_aSavedPlayerDevices[ 3 ] > -1 )
	{
		TUtil::MemCopy( ms_aPlayerDevices, ms_aSavedPlayerDevices, sizeof( ms_aPlayerDevices ) );
		TUtil::MemSet( ms_aSavedPlayerDevices, -1, sizeof( ms_aSavedPlayerDevices ) );
	}
}

// $deBlob: FUNCTION 0067a1b0
TInputDevice* TInputDevice::GetPlayerDevice( TINT a_iPlayer )
{
	TINT iDevice = ms_aPlayerDevices[ a_iPlayer ];

	if ( iDevice >= 0 )
		return s_aDevices[ iDevice ];

	return TNULL;
}

// $deBlob: FUNCTION 0067a1e0
TINT TInputDevice::GetPlayerDeviceIndex( TINT a_iPlayer )
{
	return ms_aPlayerDevices[ a_iPlayer ];
}

// $deBlob: FUNCTION 0067a200
void TInputDevice::ResetPlayerDevices()
{
	for ( TINT i = 0; i < MAX_DEVICE_COUNT; i++ )
		ms_aPlayerDevices[ i ] = -1;
}

// $deBlob: FUNCTION 0067a230
void TInputDevice::SetPlayerDevice( TINT a_iPlayer, TInputDevice* a_pDevice )
{
	TINT playerId = a_iPlayer;
	TASSERT( playerId >= 0 && playerId < 4 );

	ms_aPlayerDevices[ a_iPlayer ] = ( a_pDevice != TNULL ) ? TINT( a_pDevice->m_uiDeviceIndex ) : -1;
}

TOSHI_NAMESPACE_END

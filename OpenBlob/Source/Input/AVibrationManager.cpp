#include "pch.h"
#include "AVibrationManager.h"

#include <Input/TInputInterface.h>
#include <Platform/Windows/Input/TInputDeviceController_Wiin.h>
#include <Platform/Windows/Input/TInputDeviceController_XInput.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const AVibrationManager::VIBRATE_PRIORITY AVibrationManager::VIBRATE_PRIORITY_None = 999;

// $deBlob: FUNCTION 005e05c0
// $deBlob: FUNCTION 005e1270
AVibrationManager::AVibrationManager()
{
	for ( TINT i = 0; i < MAX_VIBRATIONS; i++ )
		new ( &m_aVibrations[ i ] ) VibrateRef();

	m_iLastVibrationId = -1;
	m_pVibrators       = TNULL;
	m_iNumVibrators    = 0;
	m_uiFlags          = 0;

	Create();
}

// $deBlob: FUNCTION 005e06d0
// $deBlob: FUNCTION 005e1490
AVibrationManager::~AVibrationManager()
{
	ResetAll();

	if ( m_pVibrators != TNULL )
		delete[] m_pVibrators;
}

// $deBlob: FUNCTION 005e09f0
void AVibrationManager::Update( TFLOAT a_fDeltaTime )
{
	// NOTE: on Windows the rest of the update is compiled out, only the call is left (its result is unused)
	IsAllVibrationsDisabled();
}

// $deBlob: FUNCTION 005e0b40
void AVibrationManager::PauseImmediate()
{
	m_uiFlags |= FLAGS_PAUSED;

	for ( TINT i = 0; i < m_iNumVibrators; i++ )
		m_pVibrators[ i ].Pause( TTRUE );
}

// $deBlob: FUNCTION 005e0a00
void AVibrationManager::ResetAll()
{
	for ( TINT i = 0; i < MAX_VIBRATIONS; i++ )
		StopVibration( i );

	for ( TINT i = 0; i < m_iNumVibrators; i++ )
		m_pVibrators[ i ].Reset();
}

// $deBlob: FUNCTION 005e0ab0
void AVibrationManager::StopVibration( TINT a_iVibrationId )
{
	m_aVibrations[ a_iVibrationId ].Reset();
}

// $deBlob: FUNCTION 005e0790
TBOOL AVibrationManager::CheckHigherPriority( VIBRATE_PRIORITY a_eNew, VIBRATE_PRIORITY a_eCurrent )
{
	if ( a_eCurrent == VIBRATE_PRIORITY_None )
		return TTRUE;

	return a_eCurrent > a_eNew;
}

// $deBlob: FUNCTION 005e0f30
TINT AVibrationManager::AddActiveVibration( VibrateRef& a_rVibration )
{
	TINT iId = GetNewVibrationId();

	if ( iId == VibrationIdInvalid )
		return VibrationIdInvalid;

	m_aVibrations[ iId ] = a_rVibration;
	return iId;
}

// $deBlob: FUNCTION 005e0fd0
TINT AVibrationManager::GetNewVibrationId()
{
	// NOTE: retail loops forever when all 32 are busy and no id was handed out yet (m_iLastVibrationId == -1);
	// unreachable on Windows, nothing adds vibrations
	TINT iLast = m_iLastVibrationId;
	TINT iId   = iLast;

	do
	{
		iId++;

		if ( iId == MAX_VIBRATIONS )
			iId = 0;

		if ( iId == iLast )
			return VibrationIdInvalid;

	} while ( m_aVibrations[ iId ].m_fTime >= 0.0f );

	if ( iId == VibrationIdInvalid )
		return VibrationIdInvalid;

	m_iLastVibrationId = iId;
	return iId;
}

// $deBlob: FUNCTION 005e1070
AVibrationManager::SCRIPT AVibrationManager::GetScriptId( VIBRATE_STRENGTH a_eStrength, VIBRATE_DURATION a_eDuration )
{
	if ( a_eStrength == VIBRATE_STRENGTH_High )
	{
		if ( a_eDuration == VIBRATE_DURATION_Long ) return SCRIPT_HighLong;
		if ( a_eDuration == VIBRATE_DURATION_Medium ) return SCRIPT_HighMedium;
		if ( a_eDuration == VIBRATE_DURATION_Short ) return SCRIPT_HighShort;
	}
	else if ( a_eStrength == VIBRATE_STRENGTH_Medium )
	{
		if ( a_eDuration == VIBRATE_DURATION_Long ) return SCRIPT_MediumLong;
		if ( a_eDuration == VIBRATE_DURATION_Medium ) return SCRIPT_MediumMedium;
		if ( a_eDuration == VIBRATE_DURATION_Short ) return SCRIPT_MediumShort;
	}
	else if ( a_eStrength == VIBRATE_STRENGTH_Low )
	{
		if ( a_eDuration == VIBRATE_DURATION_Long ) return SCRIPT_LowLong;
		if ( a_eDuration == VIBRATE_DURATION_Medium ) return SCRIPT_LowMedium;
		if ( a_eDuration == VIBRATE_DURATION_Short ) return SCRIPT_LowShort;
	}

	return SCRIPT_Invalid;
}

// $deBlob: FUNCTION 005e1060
TFLOAT AVibrationManager::GetScriptTime( SCRIPT a_eScript )
{
	// NOTE: Windows has no vibration scripts
	return -1.0f;
}

// $deBlob: FUNCTION 005e0ea0
void AVibrationManager::Create()
{
	m_uiFlags |= FLAGS_ENABLED;
	m_pVibrators    = new TVibrator[ MAX_VIBRATORS ];
	m_iNumVibrators = MAX_VIBRATORS;
}

// $deBlob: FUNCTION 005e0b80
void AVibrationManager::EnableAllVibrators( TBOOL a_bEnable )
{
	for ( TINT i = 0; i < m_iNumVibrators; i++ )
	{
		if ( m_pVibrators[ i ].GetDevice() != TNULL )
			m_pVibrators[ i ].Enable( a_bEnable );
	}

	if ( a_bEnable )
		m_uiFlags |= FLAGS_ENABLED;
	else
		m_uiFlags &= ~FLAGS_ENABLED;
}

// $deBlob: FUNCTION 005e1240
TBOOL AVibrationManager::IsVibratorEnabled( TINT a_iVibratorID )
{
	if ( a_iVibratorID != VibrationIdInvalid && m_pVibrators[ a_iVibratorID ].GetDevice() != TNULL )
		return m_pVibrators[ a_iVibratorID ].IsEnabled();

	return TFALSE;
}

// $deBlob: FUNCTION 005e0bf0
TBOOL AVibrationManager::IsVibratorEnabled( const AInputManager2::AInputDeviceHandle& a_rHandle )
{
	return IsVibratorEnabled( FindVibratorID( a_rHandle ) );
}

// $deBlob: FUNCTION 005e11d0
TBOOL AVibrationManager::EnableVibrator( TBOOL a_bEnable, TINT a_iVibratorID )
{
	TASSERT( VibrationIdInvalid != a_iVibratorID, "AVibrationManager::EnableVibrator - vibrator id is invalid\n" );

	if ( m_pVibrators[ a_iVibratorID ].GetDevice() != TNULL )
	{
		m_pVibrators[ a_iVibratorID ].Enable( a_bEnable );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005e0c40
TBOOL AVibrationManager::EnableVibrator( TBOOL a_bEnable, const AInputManager2::AInputDeviceHandle& a_rHandle )
{
	TINT iVibratorID = FindVibratorID( a_rHandle );

	if ( iVibratorID == VibrationIdInvalid )
		return TFALSE;

	return EnableVibrator( a_bEnable, iVibratorID );
}

// $deBlob: FUNCTION 005e11a0
TINT AVibrationManager::FindVibratorID( const AInputManager2::AInputDeviceHandle& a_rHandle )
{
	for ( TINT i = 0; i < MAX_VIBRATORS; i++ )
	{
		if ( m_pVibrators[ i ].GetDevice() == a_rHandle.m_pDevice )
			return i;
	}

	return VibrationIdInvalid;
}

// $deBlob: FUNCTION 005e0c80
TVibrator* AVibrationManager::Alloc( TInputDeviceController* a_pController )
{
	TASSERT( ( a_pController == TNULL ) || a_pController->IsA( &TGetClass( TInputDeviceController ) ) );

	TINT iVibratorIndex = a_pController->GetPortIndex();
	TASSERT( iVibratorIndex < MAX_VIBRATORS );

	TVibrator* pVibrator = &m_pVibrators[ iVibratorIndex ];

	if ( pVibrator->GetDevice() == TNULL )
	{
		if ( pVibrator->Create( a_pController ) == TTRUE )
		{
			pVibrator->Enable( m_uiFlags & FLAGS_ENABLED );
			return pVibrator;
		}

		pVibrator->Destroy();
		pVibrator = TNULL;
	}

	return pVibrator;
}

// $deBlob: FUNCTION 005e0d70
void AVibrationManager::Free( TVibrator* a_pVibrator )
{
	if ( a_pVibrator != TNULL )
		a_pVibrator->Destroy();
}

// $deBlob: FUNCTION 005e0d90
TVibrator* AVibrationManager::GetVibrator( TInputDeviceController* a_pController )
{
	for ( TINT i = 0; i < m_iNumVibrators; i++ )
	{
		if ( a_pController == m_pVibrators[ i ].GetDevice() )
			return &m_pVibrators[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 005e1110
TVibrator* AVibrationManager::GetVibrator( AInputManager2::INPUTDEVICE a_eInputDevice )
{
	TASSERT( AInputManager2::INPUTDEVICE_Invalid != a_eInputDevice );
	TASSERT( (TINT)a_eInputDevice < MAX_VIBRATORS );

	return &m_pVibrators[ a_eInputDevice ];
}

// $deBlob: FUNCTION 005e0dd0
TBOOL AVibrationManager::IsAllVibrationsDisabled()
{
	for ( TINT i = 0; i < MAX_VIBRATORS; i++ )
	{
		TVibrator* pVibrator = GetVibrator( AInputManager2::INPUTDEVICE( i ) );

		if ( pVibrator != TNULL && pVibrator->IsEnabled() )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005e0e80
TBOOL AVibrationManager::IsVibrationsDisabled( AInputManager2::INPUTDEVICE a_eInputDevice )
{
	TVibrator* pVibrator = GetVibrator( a_eInputDevice );

	if ( pVibrator != TNULL && pVibrator->IsEnabled() )
		return TFALSE;

	return TTRUE;
}

// $deBlob: FUNCTION 005e07b0
void AVibrationManager::VibrateOnce( TINT a_iPlayer, TINT a_iStrength )
{
	TINT iPlayer = a_iPlayer;
	TASSERT( iPlayer >= 0 && iPlayer <= 4 );

	TInputDevice* pDevice = TInputDevice::GetPlayerDevice( iPlayer );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: playing with keyboard and mouse no player has a device (the keyboard is never a player device, ABlobSplash
	// gives player 0 the first controller only if it's enabled), so retail fails this assert on the first hard landing
	// of every keyboard session; its assert handler is empty, it just returns. Without a controller there's nothing to
	// vibrate, so the assert is left out
#else
	TASSERT( pDevice != TNULL );
#endif

	if ( pDevice == TNULL )
		return;

	if ( pDevice->IsA( &TGetClass( TInputWiinDeviceController ) ) )
	{
		TINT iMilliseconds = 0;

		switch ( a_iStrength )
		{
			case 1:
			case 4:
			case 7:
				iMilliseconds = 200;
				break;
			case 2:
			case 5:
			case 8:
				iMilliseconds = 400;
				break;
			case 3:
			case 6:
			case 9:
				iMilliseconds = 600;
				break;
		}

		TSTATICCAST( TInputWiinDeviceController, pDevice )->Rumble( iMilliseconds );
	}
	else if ( pDevice->IsA( &TGetClass( TInputDeviceController_XInput ) ) )
	{
		TUINT uiMilliseconds = 0;
		TUINT uiForce        = 0;

		switch ( a_iStrength )
		{
			case 1:
				uiMilliseconds = 200;
				uiForce        = 0x5555;
				break;
			case 2:
				uiMilliseconds = 400;
				uiForce        = 0x5555;
				break;
			case 3:
				uiMilliseconds = 600;
				uiForce        = 0x5555;
				break;
			case 4:
				uiMilliseconds = 200;
				uiForce        = 0xAAAA;
				break;
			case 5:
				uiMilliseconds = 400;
				uiForce        = 0xAAAA;
				break;
			case 6:
				uiMilliseconds = 600;
				uiForce        = 0xAAAA;
				break;
			case 7:
				uiMilliseconds = 200;
				uiForce        = 0xFFFF;
				break;
			case 8:
				uiMilliseconds = 400;
				uiForce        = 0xFFFF;
				break;
			case 9:
				uiMilliseconds = 600;
				uiForce        = 0xFFFF;
				break;
		}

		TSTATICCAST( TInputDeviceController_XInput, pDevice )->Vibrate( uiMilliseconds, uiForce );
	}
}

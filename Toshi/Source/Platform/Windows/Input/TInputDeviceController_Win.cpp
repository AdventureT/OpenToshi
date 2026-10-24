#include "ToshiPCH.h"
#include "TInputDeviceController_Win.h"
#include "Platform/DX11/TMSWindow_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TINT TInputDXDeviceController::ms_iDeadZone = 7500;

// $deBlob: FUNCTION 0067b7f0
// $deBlob: FUNCTION 0067b800
// $deBlob: FUNCTION 0067b840
// $deBlob: FUNCTION 0067b880
// $deBlob: FUNCTION 0067b890
// $deBlob: FUNCTION 0053e910
// $deBlob: FUNCTION 00858e30
TDEFINE_CLASS( TInputDXDeviceController );

// The buttons, sticks and virtual buttons IsAnyButtonDown checks (0x008c6a30)
static const TInputDevice::Doodad s_aAnyButtonDoodads[] = {
	TInputDeviceController::BUTTON_CONTEXT_1,
	TInputDeviceController::BUTTON_CONTEXT_2,
	TInputDeviceController::BUTTON_CONTEXT_3,
	TInputDeviceController::BUTTON_CONTEXT_4,
	TInputDeviceController::BUTTON_SHOULDER_LEFT_2,
	TInputDeviceController::BUTTON_SHOULDER_RIGHT_2,
	TInputDeviceController::BUTTON_SHOULDER_LEFT_1,
	TInputDeviceController::BUTTON_SHOULDER_RIGHT_1,
	TInputDeviceController::BUTTON_SELECT,
	TInputDeviceController::BUTTON_STICK_LEFT,
	TInputDeviceController::BUTTON_STICK_RIGHT,
	TInputDeviceController::BUTTON_START,
	TInputDeviceController::BUTTON_DPAD_UP,
	TInputDeviceController::BUTTON_DPAD_RIGHT,
	TInputDeviceController::BUTTON_DPAD_DOWN,
	TInputDeviceController::BUTTON_DPAD_LEFT,
	TInputDeviceController::STICK_LEFT,
	TInputDeviceController::VIRTBTN_STICK_LEFT_UP,
	TInputDeviceController::VIRTBTN_STICK_LEFT_RIGHT,
	TInputDeviceController::VIRTBTN_STICK_LEFT_DOWN,
	TInputDeviceController::VIRTBTN_STICK_LEFT_LEFT,
	TInputDeviceController::STICK_RIGHT,
	TInputDeviceController::VIRTBTN_STICK_RIGHT_UP,
	TInputDeviceController::VIRTBTN_STICK_RIGHT_RIGHT,
	TInputDeviceController::VIRTBTN_STICK_RIGHT_DOWN,
	TInputDeviceController::VIRTBTN_STICK_RIGHT_LEFT,
	TInputDeviceController::VIRTSTK_DPAD,
};

// $deBlob: FUNCTION 0067b8a0
// $deBlob: FUNCTION 0067c640
// $Barnyard: FUNCTION 006d4570
TInputDXDeviceController::TInputDXDeviceController()
{
	m_poDXInputDevice = TNULL;
	TUtil::MemClear( &m_oJoyState, sizeof( m_oJoyState ) );
	TUtil::MemClear( &m_oOldJoyState, sizeof( m_oOldJoyState ) );
	m_uiButtons    = 0;
	m_uiOldButtons = 0;

	for ( TINT i = 0; i < TARRAYSIZE( m_afAxes ); i++ )
		m_afAxes[ i ] = 0.0f;

	m_bPolled         = TFALSE;
	m_bIsPSController = TFALSE;
}

// $deBlob: FUNCTION 0067b8d0
// $deBlob: FUNCTION 0067c6d0
// $Barnyard: FUNCTION 006d49f0
TInputDXDeviceController::~TInputDXDeviceController()
{
}

// $deBlob: FUNCTION 0067b900
// $Barnyard: FUNCTION 006d4aa0
TBOOL TInputDXDeviceController::Acquire()
{
	m_bPolled = TFALSE;

#ifdef DEBLOB_COMMUNITY_PATCH
	// Background test runs never read the user's controllers
	if ( TMSWindow::IsBackgroundMode() )
	{
		SetAcquired( TFALSE );
		return TFALSE;
	}
#endif

	if ( FAILED( m_poDXInputDevice->Acquire() ) )
	{
		if ( IsAcquired() )
		{
			TInputInterface::DeviceStatusEvent oEvent( this, TInputInterface::DEVICESTATUS_DISCONNECTED );
			GetInputInterface()->GetDeviceStatusEmitter().Throw( &oEvent );
		}

		SetAcquired( TFALSE );
		return TFALSE;
	}

	if ( !IsAcquired() )
	{
		TInputInterface::DeviceStatusEvent oEvent( this, TInputInterface::DEVICESTATUS_CONNECTED );
		GetInputInterface()->GetDeviceStatusEmitter().Throw( &oEvent );
	}

	SetAcquired( TTRUE );
	return TTRUE;
}

// $deBlob: FUNCTION 0067b980
// $Barnyard: FUNCTION 006d4b20
TBOOL TInputDXDeviceController::Unacquire()
{
	SetAcquired( TFALSE );
	m_bPolled = TFALSE;

	return SUCCEEDED( m_poDXInputDevice->Unacquire() ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 0067b9a0
// $Barnyard: FUNCTION 006d45c0
void TInputDXDeviceController::Release()
{
	if ( m_poDXInputDevice )
	{
		m_poDXInputDevice->Release();
		m_poDXInputDevice = TNULL;
	}
}

// $deBlob: FUNCTION 0067b9c0
// $Barnyard: FUNCTION 006d4cf0
void TInputDXDeviceController::Update( TFLOAT a_fDeltaTime )
{
	if ( FAILED( m_poDXInputDevice->Poll() ) )
		return;

	m_oOldJoyState = m_oJoyState;

	if ( FAILED( m_poDXInputDevice->GetDeviceState( sizeof( DIJOYSTATE ), &m_oJoyState ) ) )
		return;

	m_uiOldButtons = m_uiButtons;
	m_uiButtons    = 0;

	for ( TINT i = 0; i < 32; i++ )
		m_uiButtons |= ( m_oJoyState.rgbButtons[ i ] != 0 ) << i;

	m_afAxes[ 0 ] = NormaliseStick( m_oJoyState.lX );
	m_afAxes[ 1 ] = NormaliseStick( m_oJoyState.lY );
	m_afAxes[ 2 ] = NormaliseStick( m_oJoyState.lZ );
	m_afAxes[ 3 ] = NormaliseStick( m_oJoyState.lRx );
	m_afAxes[ 4 ] = NormaliseStick( m_oJoyState.lRy );
	m_afAxes[ 5 ] = NormaliseStick( m_oJoyState.lRz );

	// The POV hat moves the left stick when it's centred (the angles are in hundredths of a degree)
	if ( m_afAxes[ 0 ] == 0.0f )
	{
		DWORD dwPOV = m_oJoyState.rgdwPOV[ 0 ];

		if ( dwPOV == 22500 || dwPOV == 27000 || dwPOV == 31500 )
		{
			m_afAxes[ 0 ]    = -1.0f;
			m_oJoyState.lX = 0;
		}
		else if ( dwPOV == 4500 || dwPOV == 9000 || dwPOV == 13500 )
		{
			m_afAxes[ 0 ]    = 1.0f;
			m_oJoyState.lX = 0xFFFF;
		}
	}

	if ( m_afAxes[ 1 ] == 0.0f )
	{
		DWORD dwPOV = m_oJoyState.rgdwPOV[ 0 ];

		if ( dwPOV == 31500 || dwPOV == 0 || dwPOV == 4500 )
		{
			m_afAxes[ 1 ]    = -1.0f;
			m_oJoyState.lY = 0;
		}
		else if ( dwPOV == 13500 || dwPOV == 18000 || dwPOV == 22500 )
		{
			m_afAxes[ 1 ]    = 1.0f;
			m_oJoyState.lY = 0xFFFF;
		}
	}

	UpdateVirtualButtons();
	m_bPolled = TTRUE;
}

// $deBlob: FUNCTION 0067bc80
// $Barnyard: FUNCTION 006d4be0
TINT TInputDXDeviceController::ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	if ( !m_bPolled )
		return 0;

	// Devices of no player don't throw events, the first one that gets used is player 0's while no player has one
	if ( GetPlayer() < 0 && !ArePlayerDevicesLocked() )
	{
		if ( !HasNoPlayerDevices() )
			return 0;

		SetPlayerDevice( 0, this );
	}

	if ( IsAnyButtonDown() && !ArePlayerDevicesLocked() )
		SetLastGamepadType( GetGamepadType() );

	TINT iNumProcessed = ProcessRepeats( a_rEmitter, a_fDeltaTime );
	iNumProcessed += ProcessVirtualButtons( a_rEmitter, a_fDeltaTime );

	for ( TINT i = 0; i < NUM_DX_BUTTONS; i++ )
	{
		if ( ( m_oJoyState.rgbButtons[ i ] & 0x80 ) && !( m_oOldJoyState.rgbButtons[ i ] & 0x80 ) )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, TranslateDXButtonToDoodad( i, m_bIsPSController ), TInputInterface::EVENT_TYPE_GONE_DOWN ) );
			iNumProcessed++;
		}
		else if ( !( m_oJoyState.rgbButtons[ i ] & 0x80 ) && ( m_oOldJoyState.rgbButtons[ i ] & 0x80 ) )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, TranslateDXButtonToDoodad( i, m_bIsPSController ), TInputInterface::EVENT_TYPE_GONE_UP ) );
			iNumProcessed++;
		}
	}

	// NOTE: the right stick of the events is always Z and the Z rotation, GetAxisInt uses the X and Y rotations of HID gamepads
	if ( m_afAxes[ 0 ] != 0.0f || m_afAxes[ 1 ] != 0.0f )
	{
		a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_LEFT, TInputInterface::EVENT_TYPE_VIRTUAL, m_afAxes[ 0 ], m_afAxes[ 1 ] ) );
		iNumProcessed++;
	}

	if ( m_afAxes[ 2 ] != 0.0f || m_afAxes[ 5 ] != 0.0f )
	{
		a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_RIGHT, TInputInterface::EVENT_TYPE_VIRTUAL, m_afAxes[ 2 ], m_afAxes[ 5 ] ) );
		iNumProcessed++;
	}

	return iNumProcessed;
}

// $deBlob: FUNCTION 0067beb0
// $Barnyard: FUNCTION 006d47c0
TBOOL TInputDXDeviceController::IsDown( Doodad a_iDoodad ) const
{
	TINT iButton = TranslateDoodadToDXButton( a_iDoodad, m_bIsPSController );

	if ( iButton >= 0 )
		return m_oJoyState.rgbButtons[ iButton ] >> 7;

	return TInputDeviceController::IsDown( a_iDoodad );
}

// $deBlob: FUNCTION 0067bef0
// $Barnyard: FUNCTION 006d47f0
TBOOL TInputDXDeviceController::WasDown( Doodad a_iDoodad ) const
{
	TINT iButton = TranslateDoodadToDXButton( a_iDoodad, m_bIsPSController );

	if ( iButton >= 0 )
		return m_oOldJoyState.rgbButtons[ iButton ] >> 7;

	return TInputDeviceController::WasDown( a_iDoodad );
}

// $deBlob: FUNCTION 0067bf30
// $Barnyard: FUNCTION 006d4820
TINT TInputDXDeviceController::GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const
{
	TINT iResult = 0;

	if ( a_iDoodad == STICK_LEFT )
	{
		if ( a_iAxis == 0 )
			iResult = m_oJoyState.lX - STICK_SATURATE;
		else if ( a_iAxis == 1 )
			iResult = m_oJoyState.lY - STICK_SATURATE;
	}
	else if ( a_iDoodad == STICK_RIGHT )
	{
		if ( m_oDeviceInstance.dwDevType != DEVTYPE_HID_GAMEPAD )
		{
			if ( a_iAxis == 0 )
				iResult = m_oJoyState.lZ - STICK_SATURATE;
			else if ( a_iAxis == 1 )
				iResult = m_oJoyState.lRz - STICK_SATURATE;
		}
		else
		{
			if ( a_iAxis == 0 )
				iResult = m_oJoyState.lRx - STICK_SATURATE;
			else if ( a_iAxis == 1 )
				iResult = m_oJoyState.lRy - STICK_SATURATE;
		}
	}

	// NOTE: the axes are centred on STICK_SATURATE, not STICK_CENTRE
	if ( iResult <= ms_iDeadZone && iResult >= -ms_iDeadZone )
		return 0;

	return iResult;
}

// $deBlob: FUNCTION 0067bfd0
// $Barnyard: FUNCTION 006d4890
TFLOAT TInputDXDeviceController::GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const
{
	return GetAxisInt( a_iDoodad, a_iAxis ) / 32000.0f;
}

// $deBlob: FUNCTION 0067c000
// $Barnyard: FUNCTION 006d4a20
TBOOL const TInputDXDeviceController::BindToDIDevice( HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDXDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice )
{
	TASSERT( a_poDXDeviceInstance != NULL );
	TASSERT( a_poDXInputDevice != NULL );

	m_bIsPSController = IsPSController( a_poDXDeviceInstance );
	Initialise();

	TUtil::MemCopy( &m_oDeviceInstance, a_poDXDeviceInstance, sizeof( DIDEVICEINSTANCEA ) );
	m_poDXInputDevice = a_poDXInputDevice;

	m_oDIDevCaps.dwSize = sizeof( DIDEVCAPS );
	a_poDXInputDevice->GetCapabilities( &m_oDIDevCaps );

	if ( m_poDXInputDevice->SetDataFormat( &c_dfDIJoystick ) != DI_OK )
		return TFALSE;

	// NOTE: no cooperative level, a_hMainWindow isn't used
	m_poDXInputDevice->EnumObjects( EnumerateObjectCallback, this, DIDFT_AXIS | DIDFT_BUTTON );
	m_bPolled = TFALSE;

	return TTRUE;
}

// $deBlob: FUNCTION 0067c130
// $Barnyard: FUNCTION 006d45e0
TInputDevice::Doodad TInputDXDeviceController::TranslateDXButtonToDoodad( TINT a_iButton, TBOOL a_bPSController )
{
	if ( a_bPSController )
	{
		// Square, Cross, Circle, Triangle, L1, R1, L2, R2, Share, Options, L3, R3
		switch ( a_iButton )
		{
			case 0: return BUTTON_CONTEXT_3;
			case 1: return BUTTON_CONTEXT_1;
			case 2: return BUTTON_CONTEXT_2;
			case 3: return BUTTON_CONTEXT_4;
			case 4: return BUTTON_SHOULDER_LEFT_1;
			case 5: return BUTTON_SHOULDER_RIGHT_1;
			case 6: return BUTTON_SHOULDER_LEFT_2;
			case 7: return BUTTON_SHOULDER_RIGHT_2;
			case 8: return BUTTON_SELECT;
			case 9: return BUTTON_START;
			case 10: return BUTTON_STICK_LEFT;
			case 11: return BUTTON_STICK_RIGHT;
			case 12: return BUTTON_DPAD_UP;
			case 13: return BUTTON_DPAD_RIGHT;
			case 14: return BUTTON_DPAD_DOWN;
			case 15: return BUTTON_DPAD_LEFT;
			default: return 0;
		}
	}

	switch ( a_iButton )
	{
		case 0: return BUTTON_CONTEXT_1;
		case 1: return BUTTON_CONTEXT_3;
		case 2: return BUTTON_CONTEXT_2;
		case 3: return BUTTON_CONTEXT_4;
		case 4: return BUTTON_SHOULDER_LEFT_1;
		case 5: return BUTTON_SHOULDER_RIGHT_1;
		case 6: return BUTTON_SELECT;
		case 7: return BUTTON_START;
		case 8: return BUTTON_STICK_LEFT;
		case 9: return BUTTON_STICK_RIGHT;
		case 10: return BUTTON_SHOULDER_LEFT_2;
		case 11: return BUTTON_SHOULDER_RIGHT_2;
		case 12: return BUTTON_DPAD_UP;
		case 13: return BUTTON_DPAD_RIGHT;
		case 14: return BUTTON_DPAD_DOWN;
		case 15: return BUTTON_DPAD_LEFT;
		default: return 0;
	}
}

// $deBlob: FUNCTION 0067c270
// $Barnyard: FUNCTION 006d46c0
TINT TInputDXDeviceController::TranslateDoodadToDXButton( Doodad a_iDoodad, TBOOL a_bPSController )
{
	if ( a_bPSController )
	{
		switch ( a_iDoodad )
		{
			case BUTTON_DPAD_UP: return 12;
			case BUTTON_DPAD_RIGHT: return 13;
			case BUTTON_DPAD_DOWN: return 14;
			case BUTTON_DPAD_LEFT: return 15;
			case BUTTON_SELECT: return 8;
			case BUTTON_START: return 9;
			case BUTTON_STICK_LEFT: return 10;
			case BUTTON_STICK_RIGHT: return 11;
			case BUTTON_CONTEXT_1: return 1;
			case BUTTON_CONTEXT_2: return 2;
			case BUTTON_CONTEXT_3: return 0;
			case BUTTON_CONTEXT_4: return 3;
			case BUTTON_SHOULDER_LEFT_1: return 4;
			case BUTTON_SHOULDER_LEFT_2: return 6;
			case BUTTON_SHOULDER_RIGHT_1: return 5;
			case BUTTON_SHOULDER_RIGHT_2: return 7;
			default: return -1;
		}
	}

	switch ( a_iDoodad )
	{
		case BUTTON_DPAD_UP: return 12;
		case BUTTON_DPAD_RIGHT: return 13;
		case BUTTON_DPAD_DOWN: return 14;
		case BUTTON_DPAD_LEFT: return 15;
		case BUTTON_SELECT: return 6;
		case BUTTON_START: return 7;
		case BUTTON_STICK_LEFT: return 8;
		case BUTTON_STICK_RIGHT: return 9;
		case BUTTON_CONTEXT_1: return 0;
		case BUTTON_CONTEXT_2: return 2;
		case BUTTON_CONTEXT_3: return 1;
		case BUTTON_CONTEXT_4: return 3;
		case BUTTON_SHOULDER_LEFT_1: return 4;
		case BUTTON_SHOULDER_LEFT_2: return 10;
		case BUTTON_SHOULDER_RIGHT_1: return 5;
		case BUTTON_SHOULDER_RIGHT_2: return 11;
		default: return -1;
	}
}

// $deBlob: FUNCTION 0067c3c0
TBOOL TInputDXDeviceController::IsAnyButtonDown() const
{
	if ( !m_bPolled )
		return TFALSE;

	for ( TINT i = 0; i < TARRAYSIZE( s_aAnyButtonDoodads ); i++ )
	{
		if ( IsDown( s_aAnyButtonDoodads[ i ] ) )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0067c400
TBOOL TInputDXDeviceController::IsPSController( LPCDIDEVICEINSTANCEA a_poDXDeviceInstance )
{
	TASSERT( a_poDXDeviceInstance != NULL );

	const GUID& rGUID = a_poDXDeviceInstance->guidProduct;

	TCHAR szGUID[ 40 ] = {};
	sprintf(
	    szGUID,
	    "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
	    rGUID.Data1,
	    rGUID.Data2,
	    rGUID.Data3,
	    rGUID.Data4[ 0 ],
	    rGUID.Data4[ 1 ],
	    rGUID.Data4[ 2 ],
	    rGUID.Data4[ 3 ],
	    rGUID.Data4[ 4 ],
	    rGUID.Data4[ 5 ],
	    rGUID.Data4[ 6 ],
	    rGUID.Data4[ 7 ]
	);

	// PID and VID of the product (the rest is "PIDVID")
#ifdef DEBLOB_COMMUNITY_PATCH
	// Retail looks for the DualShock 4 (2nd revision) in lower case and never finds it in the upper case string
	return T2String8::FindString( szGUID, "09CC054C-0000-0000-0000-504944564944" ) != TNULL ||
#else
	return T2String8::FindString( szGUID, "09cc054c-0000-0000-0000-504944564944" ) != TNULL ||
#endif
	       T2String8::FindString( szGUID, "05C4054C-0000-0000-0000-504944564944" ) != TNULL ||
	       T2String8::FindString( szGUID, "00050925-0000-0000-0000-504944564944" ) != TNULL ||
	       T2String8::FindString( szGUID, "0268054C-0000-0000-0000-504944564944" ) != TNULL ||
	       T2String8::FindString( szGUID, "03088888-0000-0000-0000-504944564944" ) != TNULL;
}

// $deBlob: FUNCTION 0067c570
// $Barnyard: FUNCTION 006d4b40
TFLOAT TInputDXDeviceController::NormaliseStick( TINT a_iValue )
{
	a_iValue -= STICK_CENTRE;

	if ( a_iValue < 0 )
	{
		a_iValue = TMath::Min( a_iValue, -ms_iDeadZone );
		a_iValue = TMath::Max( a_iValue, -STICK_SATURATE );
		return TFLOAT( a_iValue + ms_iDeadZone ) / TFLOAT( STICK_SATURATE - ms_iDeadZone );
	}

	a_iValue = TMath::Max( a_iValue, ms_iDeadZone );
	a_iValue = TMath::Min( a_iValue, STICK_SATURATE );
	return TFLOAT( a_iValue - ms_iDeadZone ) / TFLOAT( STICK_SATURATE - ms_iDeadZone );
}

// $deBlob: FUNCTION 0067c530
// $Barnyard: FUNCTION 006d45b0
BOOL CALLBACK TInputDXDeviceController::EnumerateObjectCallback( LPCDIDEVICEOBJECTINSTANCEA a_poDeviceInstance, LPVOID a_pData )
{
	TInputDXDeviceController* poDXController = TSTATICCAST( TInputDXDeviceController, a_pData );
	TASSERT( poDXController != NULL );

	return DIENUM_CONTINUE;
}

TOSHI_NAMESPACE_END

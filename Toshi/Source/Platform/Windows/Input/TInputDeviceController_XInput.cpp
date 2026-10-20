#include "ToshiPCH.h"
#include "TInputDeviceController_XInput.h"
#include "Platform/DX11/TMSWindow_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

#pragma comment( lib, "xinput9_1_0.lib" )

TOSHI_NAMESPACE_START

TINT TInputDeviceController_XInput::ms_iDeadZone = 7500;

// $deBlob: FUNCTION 0067e410
// $deBlob: FUNCTION 0067e420
// $deBlob: FUNCTION 0067e440
// $deBlob: FUNCTION 0067e460
// $deBlob: FUNCTION 0067e470
// $deBlob: FUNCTION 0053ea40
// $deBlob: FUNCTION 00858e70
TDEFINE_CLASS( TInputDeviceController_XInput );

// The buttons ProcessEvents throws events for (0x008c72d0)
static const TInputDevice::Doodad s_aEventDoodads[] = {
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
};

// The buttons, sticks and virtual buttons IsAnyButtonDown checks (0x008c7310)
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

// The XINPUT_GAMEPAD buttons of the bits of m_uiButtons (a function-local static of Update, 0x009a4ca8)
struct XInputButtonMasks
{
	// $deBlob: FUNCTION 0067ee20
	XInputButtonMasks()
	{
		aMasks[ 0 ] = XINPUT_GAMEPAD_A;
		aMasks[ 1 ] = XINPUT_GAMEPAD_B;
		aMasks[ 2 ] = XINPUT_GAMEPAD_X;
		aMasks[ 3 ] = XINPUT_GAMEPAD_Y;
		aMasks[ 4 ] = XINPUT_GAMEPAD_LEFT_SHOULDER;
		aMasks[ 5 ] = XINPUT_GAMEPAD_RIGHT_SHOULDER;
		aMasks[ 6 ] = XINPUT_GAMEPAD_BACK;
		aMasks[ 7 ] = XINPUT_GAMEPAD_START;
		aMasks[ 8 ] = 0;
		aMasks[ 9 ] = 0;
	}

	TINT aMasks[ TInputDeviceController_XInput::NUM_BUTTON_BITS ];
};

// $deBlob: FUNCTION 0067e480
// $deBlob: FUNCTION 0067eea0
// $deBlob: FUNCTION 0067ee70
TInputDeviceController_XInput::TInputDeviceController_XInput( TINT a_iUserIndex )
{
	m_dwUserIndex = a_iUserIndex;
	m_bConnected  = TTRUE;
	m_bPolled     = TFALSE;
	m_bVibrating  = TFALSE;

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves the states and the vibration end time uninitialised
	m_uiVibrationEndTime = 0;
	TUtil::MemClear( &m_oState, sizeof( m_oState ) );
	TUtil::MemClear( &m_oOldState, sizeof( m_oOldState ) );
#endif

	m_uiButtons    = 0;
	m_uiOldButtons = 0;

	for ( TINT i = 0; i < TARRAYSIZE( m_afAxes ); i++ )
		m_afAxes[ i ] = 0.0f;
}

// $deBlob: FUNCTION 0067e500
// $deBlob: FUNCTION 0067eee0
TInputDeviceController_XInput::~TInputDeviceController_XInput()
{
}

TBOOL TInputDeviceController_XInput::Acquire()
{
#ifdef DEBLOB_COMMUNITY_PATCH
	// Background test runs never read the user's controllers
	if ( TMSWindow::IsBackgroundMode() )
	{
		SetAcquired( TFALSE );
		return TFALSE;
	}
#endif

	SetAcquired( TTRUE );
	return TTRUE;
}

// $deBlob: FUNCTION 0067e530
void TInputDeviceController_XInput::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bVibrating && GetTickCount64() >= m_uiVibrationEndTime )
	{
		XINPUT_VIBRATION oVibration = {};
		XInputSetState( m_dwUserIndex, &oVibration );
		m_uiVibrationEndTime = GetTickCount64();
		m_bVibrating         = TFALSE;
	}

	static XInputButtonMasks s_oButtonMasks;

	m_oOldState = m_oState;

	if ( XInputGetState( m_dwUserIndex, &m_oState ) != ERROR_SUCCESS )
	{
		m_bConnected = TFALSE;
		UpdateVirtualButtons();
		m_bPolled = TTRUE;
		return;
	}

	m_bConnected   = TTRUE;
	m_uiOldButtons = m_uiButtons;
	m_uiButtons    = 0;

	for ( TINT i = 0; i < NUM_BUTTON_BITS; i++ )
	{
		if ( s_oButtonMasks.aMasks[ i ] & m_oState.Gamepad.wButtons )
			m_uiButtons |= 1 << i;
	}

	if ( m_oState.Gamepad.bLeftTrigger > TRIGGER_THRESHOLD ) m_uiButtons |= 1 << 10;
	if ( m_oState.Gamepad.bRightTrigger > TRIGGER_THRESHOLD ) m_uiButtons |= 1 << 11;

	// Dead zone of the sticks
	if ( TUINT( m_oState.Gamepad.sThumbLX + ( THUMB_DEADZONE - 1 ) ) <= 2 * ( THUMB_DEADZONE - 1 ) ) m_oState.Gamepad.sThumbLX = 0;
	if ( TUINT( m_oState.Gamepad.sThumbLY + ( THUMB_DEADZONE - 1 ) ) <= 2 * ( THUMB_DEADZONE - 1 ) ) m_oState.Gamepad.sThumbLY = 0;
	if ( TUINT( m_oState.Gamepad.sThumbRX + ( THUMB_DEADZONE - 1 ) ) <= 2 * ( THUMB_DEADZONE - 1 ) ) m_oState.Gamepad.sThumbRX = 0;
	if ( TUINT( m_oState.Gamepad.sThumbRY + ( THUMB_DEADZONE - 1 ) ) <= 2 * ( THUMB_DEADZONE - 1 ) ) m_oState.Gamepad.sThumbRY = 0;

	m_afAxes[ 0 ] = m_oState.Gamepad.sThumbLX / 32767.0f;
	m_afAxes[ 1 ] = m_oState.Gamepad.sThumbLY / -32767.0f;
	m_afAxes[ 3 ] = m_oState.Gamepad.sThumbRX / 32767.0f;
	m_afAxes[ 4 ] = m_oState.Gamepad.sThumbRY / -32767.0f;

	UpdateVirtualButtons();
	m_bPolled = TTRUE;
}

// $deBlob: FUNCTION 0067e770
TINT TInputDeviceController_XInput::ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
{
	if ( !m_bPolled )
		return 0;

	// Devices of no player don't throw events, the first enabled one that gets used is player 0's while no player has one
	if ( !IsEnabled() || GetPlayer() < 0 )
	{
		if ( !ArePlayerDevicesLocked() )
		{
			if ( !IsEnabled() || GetPlayer() >= 0 || !HasNoPlayerDevices() )
				return 0;

			SetPlayerDevice( 0, this );
		}
	}

	if ( IsAnyButtonDown() && !ArePlayerDevicesLocked() )
		SetLastGamepadType( Gamepad_XBox );

	TINT iNumProcessed = ProcessRepeats( a_rEmitter, a_fDeltaTime );
	iNumProcessed += ProcessVirtualButtons( a_rEmitter, a_fDeltaTime );

	for ( TINT i = 0; i < TARRAYSIZE( s_aEventDoodads ); i++ )
	{
		TINT  iMask  = TranslateDoodadToButton( s_aEventDoodads[ i ] );
		TBOOL bDown  = ( m_oState.Gamepad.wButtons & iMask ) != 0;
		TBOOL bWasDown = ( m_oOldState.Gamepad.wButtons & iMask ) != 0;

		if ( bDown && !bWasDown )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, s_aEventDoodads[ i ], TInputInterface::EVENT_TYPE_GONE_DOWN ) );
			iNumProcessed++;
		}
		else if ( !bDown && bWasDown )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, s_aEventDoodads[ i ], TInputInterface::EVENT_TYPE_GONE_UP ) );
			iNumProcessed++;
		}
	}

	a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_LEFT, TInputInterface::EVENT_TYPE_VIRTUAL, m_afAxes[ 0 ], m_afAxes[ 1 ] ) );

#ifdef DEBLOB_COMMUNITY_PATCH
	// Retail sends the unused axes 2 and 5 (the layout of the DirectInput controller): the right stick events are always zero
	a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_RIGHT, TInputInterface::EVENT_TYPE_VIRTUAL, m_afAxes[ 3 ], m_afAxes[ 4 ] ) );
#else
	a_rEmitter.Throw( TInputInterface::InputEvent( this, STICK_RIGHT, TInputInterface::EVENT_TYPE_VIRTUAL, m_afAxes[ 2 ], m_afAxes[ 5 ] ) );
#endif

	return iNumProcessed + 2;
}

// $deBlob: FUNCTION 0067e960
TBOOL TInputDeviceController_XInput::IsDown( Doodad a_iDoodad ) const
{
	TINT iMask = TranslateDoodadToButton( a_iDoodad );

	if ( iMask >= 0 )
		return ( iMask & m_oState.Gamepad.wButtons ) != 0;

	return TInputDeviceController::IsDown( a_iDoodad );
}

// $deBlob: FUNCTION 0067e9a0
TBOOL TInputDeviceController_XInput::WasDown( Doodad a_iDoodad ) const
{
	TINT iMask = TranslateDoodadToButton( a_iDoodad );

	if ( iMask >= 0 )
		return ( iMask & m_oOldState.Gamepad.wButtons ) != 0;

	return TInputDeviceController::WasDown( a_iDoodad );
}

// $deBlob: FUNCTION 0067e9e0
TINT TInputDeviceController_XInput::GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const
{
	TINT iResult = 0;

	if ( a_iDoodad == STICK_LEFT )
	{
		if ( a_iAxis == 0 )
			iResult = m_oState.Gamepad.sThumbLX;
		else
			iResult = -m_oState.Gamepad.sThumbLY;
	}
	else if ( a_iDoodad == STICK_RIGHT )
	{
		if ( a_iAxis == 0 )
			iResult = m_oState.Gamepad.sThumbRX;
		else
			iResult = -m_oState.Gamepad.sThumbRY;
	}

	if ( iResult <= ms_iDeadZone && iResult >= -ms_iDeadZone )
		return 0;

	return iResult;
}

// $deBlob: FUNCTION 0067ea40
TFLOAT TInputDeviceController_XInput::GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const
{
	return GetAxisInt( a_iDoodad, a_iAxis ) * ( 1.0f / 32768.0f );
}

// $deBlob: FUNCTION 0067ea70
TBOOL TInputDeviceController_XInput::IsAnyButtonDown() const
{
	if ( !m_bConnected )
		return TFALSE;

	for ( TINT i = 0; i < TARRAYSIZE( s_aAnyButtonDoodads ); i++ )
	{
		if ( IsDown( s_aAnyButtonDoodads[ i ] ) )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0067eab0
TBOOL TInputDeviceController_XInput::Vibrate( TUINT a_uiMilliseconds, TUINT a_uiForce )
{
	TUINT uiForce = a_uiForce;
	TUINT ms      = a_uiMilliseconds;
	TASSERT( uiForce >= 0 && uiForce <= 65535 );
	TASSERT( ms <= 2000 );

	XINPUT_VIBRATION oVibration;
	memset( &oVibration, 0, sizeof( oVibration ) );
	oVibration.wLeftMotorSpeed  = WORD( a_uiForce );
	oVibration.wRightMotorSpeed = WORD( a_uiForce );
	XInputSetState( m_dwUserIndex, &oVibration );

	m_uiVibrationEndTime = GetTickCount64() + a_uiMilliseconds;
	m_bVibrating = uiForce != 0;

	return m_bVibrating;
}

// $deBlob: FUNCTION 0067eb70
TINT TInputDeviceController_XInput::TranslateDoodadToButton( Doodad a_iDoodad )
{
	// NOTE: the stick buttons have no masks of their own (9 and 10 are D-pad combinations), the first shoulder
	// buttons are the stick buttons
	switch ( a_iDoodad )
	{
		case BUTTON_DPAD_UP: return XINPUT_GAMEPAD_DPAD_UP;
		case BUTTON_DPAD_RIGHT: return XINPUT_GAMEPAD_DPAD_RIGHT;
		case BUTTON_DPAD_DOWN: return XINPUT_GAMEPAD_DPAD_DOWN;
		case BUTTON_DPAD_LEFT: return XINPUT_GAMEPAD_DPAD_LEFT;
		case BUTTON_SELECT: return XINPUT_GAMEPAD_BACK;
		case BUTTON_START: return XINPUT_GAMEPAD_START;
		case BUTTON_STICK_LEFT: return 9;
		case BUTTON_STICK_RIGHT: return 10;
		case BUTTON_CONTEXT_1: return XINPUT_GAMEPAD_A;
		case BUTTON_CONTEXT_2: return XINPUT_GAMEPAD_B;
		case BUTTON_CONTEXT_3: return XINPUT_GAMEPAD_X;
		case BUTTON_CONTEXT_4: return XINPUT_GAMEPAD_Y;
		case BUTTON_SHOULDER_LEFT_1: return XINPUT_GAMEPAD_LEFT_THUMB;
		case BUTTON_SHOULDER_LEFT_2: return XINPUT_GAMEPAD_LEFT_SHOULDER;
		case BUTTON_SHOULDER_RIGHT_1: return XINPUT_GAMEPAD_RIGHT_THUMB;
		case BUTTON_SHOULDER_RIGHT_2: return XINPUT_GAMEPAD_RIGHT_SHOULDER;
		default: return -1;
	}
}

// $deBlob: FUNCTION 0067ec70
TInputDevice::Doodad TInputDeviceController_XInput::TranslateButtonToDoodad( TINT a_iButton )
{
	switch ( a_iButton )
	{
		case XINPUT_GAMEPAD_DPAD_UP: return BUTTON_DPAD_UP;
		case XINPUT_GAMEPAD_DPAD_DOWN: return BUTTON_DPAD_DOWN;
		case XINPUT_GAMEPAD_DPAD_LEFT: return BUTTON_DPAD_LEFT;
		case XINPUT_GAMEPAD_DPAD_RIGHT: return BUTTON_DPAD_RIGHT;
		case 9: return BUTTON_STICK_LEFT;
		case 10: return BUTTON_STICK_RIGHT;
		case XINPUT_GAMEPAD_START: return BUTTON_START;
		case XINPUT_GAMEPAD_BACK: return BUTTON_SELECT;
		case XINPUT_GAMEPAD_LEFT_THUMB: return BUTTON_SHOULDER_LEFT_1;
		case XINPUT_GAMEPAD_RIGHT_THUMB: return BUTTON_SHOULDER_RIGHT_1;
		case XINPUT_GAMEPAD_LEFT_SHOULDER: return BUTTON_SHOULDER_LEFT_2;
		case XINPUT_GAMEPAD_RIGHT_SHOULDER: return BUTTON_SHOULDER_RIGHT_2;
		case XINPUT_GAMEPAD_A: return BUTTON_CONTEXT_1;
		case XINPUT_GAMEPAD_B: return BUTTON_CONTEXT_2;
		case XINPUT_GAMEPAD_X: return BUTTON_CONTEXT_3;
		case XINPUT_GAMEPAD_Y: return BUTTON_CONTEXT_4;
		default: return 0;
	}
}

// $deBlob: FUNCTION 0067ee00
TINT TInputDeviceController_XInput::ApplyDeadZone( TINT a_iValue, TINT a_iDeadZone )
{
	if ( a_iValue >= a_iDeadZone || a_iValue <= -a_iDeadZone )
		return a_iValue;

	return 0;
}

TOSHI_NAMESPACE_END

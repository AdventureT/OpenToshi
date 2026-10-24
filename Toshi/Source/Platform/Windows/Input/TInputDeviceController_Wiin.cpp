#include "ToshiPCH.h"
#include "TInputDeviceController_Wiin.h"
#include "Thread/TThread.h"
#include "Thread/TMutex.h"
#include "Thread/TMutexLock.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TINT TInputWiinDeviceController::ms_iDeadZone = 7500;

// NOTE: the wiimote code is WiiYourself! (third-party); the small inline
// wiimote accessors it inlines here (IsConnected, ConnectionLost, MotionPlusConnected...) are part of it too

// The thread that owns the remotes that wait for a connection (one slot per remote index, each with its mutex).
// NOTE: Main only sleeps, nothing connects the remotes of the slots
class WiimoteThread : public TThread
{
public:
	static TCONSTEXPR_VAR TINT MAX_REMOTES = 4;

	WiimoteThread();

	virtual void Main() override;

	void  Start();
	void  SetRemote( wiimote* a_pRemote, TINT a_iSlot );
	void  RemoveRemote( TINT a_iSlot );
	TBOOL HasRemote( TINT a_iSlot ) const;

public:
	TBOOL    m_bStarted;                 // 0x14
	TMutex   m_aMutexes[ MAX_REMOTES ];  // 0x18
	TBOOL    m_bExit;                    // 0x28 Main returns once it's set
	TUINT    m_uiUnknown;                // 0x2C
	wiimote* m_apRemotes[ MAX_REMOTES ]; // 0x30
};

// Dynamic initializer: only the constructor's mutex/remote loop is left (0x00981a70 m_aMutexes, 0x00981a88 m_apRemotes), the rest is constant-initialized
// $deBlob: FUNCTION 0053ea10
static WiimoteThread s_oWiimoteThread;

// $deBlob: FUNCTION 0067e160
WiimoteThread::WiimoteThread()
{
	m_bStarted  = TFALSE;
	m_bExit     = TFALSE;
	m_uiUnknown = 0;

	for ( TINT i = 0; i < MAX_REMOTES; i++ )
	{
		m_aMutexes[ i ].Create();
		m_apRemotes[ i ] = TNULL;
	}
}

// $deBlob: FUNCTION 0067e300
void WiimoteThread::Main()
{
	while ( !m_bExit )
		Sleep( 1000 );
}

// $deBlob: FUNCTION 0067e350
void WiimoteThread::Start()
{
	if ( !m_bStarted )
	{
		m_bStarted = TTRUE;
		Create( 0, THREAD_PRIORITY_NORMAL, FLAGS_NONE );
	}
}

// $deBlob: FUNCTION 0067e1f0
void WiimoteThread::SetRemote( wiimote* a_pRemote, TINT a_iSlot )
{
	if ( m_apRemotes[ a_iSlot ] == TNULL )
	{
		TMutexLock oLock( m_aMutexes[ a_iSlot ] );
		m_apRemotes[ a_iSlot ] = a_pRemote;
	}
}

// $deBlob: FUNCTION 0067e240
void WiimoteThread::RemoveRemote( TINT a_iSlot )
{
	TMutexLock oLock( m_aMutexes[ a_iSlot ] );

	if ( m_apRemotes[ a_iSlot ] != TNULL )
	{
		m_apRemotes[ a_iSlot ]->Disconnect();
		m_apRemotes[ a_iSlot ] = TNULL;
	}
}

// $deBlob: FUNCTION 0067e2e0
TBOOL WiimoteThread::HasRemote( TINT a_iSlot ) const
{
	return m_apRemotes[ a_iSlot ] != TNULL;
}

// $deBlob: FUNCTION 0067d190
// $deBlob: FUNCTION 0067d1a0
// $deBlob: FUNCTION 0067d1c0
// $deBlob: FUNCTION 0067d1e0
// $deBlob: FUNCTION 0067d1f0
// $deBlob: FUNCTION 0053e9d0
// $deBlob: FUNCTION 00858e60
TDEFINE_CLASS( TInputWiinDeviceController );

// NOTE: the remote index is 1-4 and the slots of the thread are 0-3, retail indexes the thread's arrays with
// m_iIndex - 1 (they're addressed from 0x00981a6c and 0x00981a84)
#define WIIMOTE_SLOT( INDEX ) ( ( INDEX ) - 1 )

// $deBlob: FUNCTION 0067d200
TInputWiinDeviceController::TInputWiinDeviceController()
{
	m_bPolled = TFALSE;
	m_pRemote = TNULL;
	m_iIndex  = 0;
	m_uiUnk1  = 0;
	m_uiUnk2  = 0;

	TUtil::MemClear( m_abButtons, sizeof( m_abButtons ) );
	TUtil::MemClear( m_abOldButtons, sizeof( m_abOldButtons ) );

	m_pRemote = new wiimote();
	s_oWiimoteThread.Start();

	TASSERT( m_pRemote != TNULL );
}

// $deBlob: FUNCTION 0067d2c0
// $deBlob: FUNCTION 0067e1c0
TInputWiinDeviceController::~TInputWiinDeviceController()
{
	if ( m_pRemote != TNULL )
	{
		s_oWiimoteThread.RemoveRemote( WIIMOTE_SLOT( m_iIndex ) );

		delete m_pRemote;
		m_pRemote = TNULL;
	}
}

// $deBlob: FUNCTION 0067d340
TBOOL TInputWiinDeviceController::Initialise( TINT a_iIndex )
{
	m_pRemote->ChangedCallback      = StateChangedCallback;
	m_pRemote->CallbackTriggerFlags = state_change_flags( 0x7FFFE01 ) /* CONNECTED and every extension flag */;
	m_iIndex                        = a_iIndex;

	return TTRUE;
}

// $deBlob: FUNCTION 0067d390
TBOOL TInputWiinDeviceController::Acquire()
{
	if ( m_pRemote->IsConnected() && !s_oWiimoteThread.HasRemote( WIIMOTE_SLOT( m_iIndex ) ) )
	{
		if ( !IsAcquired() )
		{
			TInputInterface::DeviceStatusEvent oEvent( this, TInputInterface::DEVICESTATUS_CONNECTED );
			GetInputInterface()->GetDeviceStatusEmitter().Throw( &oEvent );

			m_pRemote->SetLEDs( BYTE( m_iIndex ) );

			if ( HasNunchuk() )
				TUtil::Log( "Added Wiimote Controller: '%I64u' (Nunchuk: YES, LED index: %d)", m_pRemote->UniqueID, m_iIndex );
			else
				TUtil::Log( "Added Wiimote Controller: '%I64u' (Nunchuk: NO, LED index: %d)", m_pRemote->UniqueID, m_iIndex );
		}

		m_bPolled = TTRUE;
		SetAcquired( TTRUE );
		return TTRUE;
	}

	if ( m_pRemote->ConnectionLost() )
	{
		// Start over with a new wiimote
		s_oWiimoteThread.RemoveRemote( WIIMOTE_SLOT( m_iIndex ) );

		delete m_pRemote;
		m_pRemote = TNULL;
		m_pRemote = new wiimote();
		TASSERT( m_pRemote != TNULL );

		m_pRemote->ChangedCallback      = StateChangedCallback;
		m_pRemote->CallbackTriggerFlags = state_change_flags( 0x7FFFE01 ) /* CONNECTED and every extension flag */;

		if ( IsAcquired() )
		{
			TInputInterface::DeviceStatusEvent oEvent( this, TInputInterface::DEVICESTATUS_DISCONNECTED );
			GetInputInterface()->GetDeviceStatusEmitter().Throw( &oEvent );
		}

		// NOTE: the times aren't used
		GetTimeMicroseconds();
		s_oWiimoteThread.SetRemote( m_pRemote, WIIMOTE_SLOT( m_iIndex ) );
		GetTimeMicroseconds();

		m_bPolled = TFALSE;
		SetAcquired( TFALSE );
		return TFALSE;
	}

	m_bPolled = TFALSE;

	if ( IsAcquired() )
	{
		TInputInterface::DeviceStatusEvent oEvent( this, TInputInterface::DEVICESTATUS_DISCONNECTED );
		GetInputInterface()->GetDeviceStatusEmitter().Throw( &oEvent );
	}

	// Hand the remote to the thread to wait for its connection
	GetTimeMicroseconds();
	s_oWiimoteThread.SetRemote( m_pRemote, WIIMOTE_SLOT( m_iIndex ) );
	GetTimeMicroseconds();

	SetAcquired( TFALSE );
	return TFALSE;
}

// $deBlob: FUNCTION 0067d620
TBOOL TInputWiinDeviceController::Unacquire()
{
	if ( IsAcquired() )
	{
		TInputInterface::DeviceStatusEvent oEvent( this, TInputInterface::DEVICESTATUS_DISCONNECTED );
		GetInputInterface()->GetDeviceStatusEmitter().Throw( &oEvent );
	}

	SetAcquired( TFALSE );
	m_bPolled = TFALSE;
	return TTRUE;
}

// $deBlob: FUNCTION 0067d660
void TInputWiinDeviceController::Release()
{
	if ( m_pRemote != TNULL )
		m_pRemote->Disconnect();
}

// $deBlob: FUNCTION 0067e370
void TInputWiinDeviceController::UnpackButtons( const wiimote_state& a_rState, TBOOL* a_pButtons )
{
	const WORD uiBits = a_rState.Button.Bits;

	a_pButtons[ 0 ]  = ( uiBits & wiimote_state::buttons::_A ) != 0;
	a_pButtons[ 1 ]  = ( uiBits & wiimote_state::buttons::_B ) != 0;
	a_pButtons[ 2 ]  = ( uiBits & wiimote_state::buttons::PLUS ) != 0;
	a_pButtons[ 3 ]  = ( uiBits & wiimote_state::buttons::MINUS ) != 0;
	a_pButtons[ 4 ]  = ( uiBits & wiimote_state::buttons::ONE ) != 0;
	a_pButtons[ 5 ]  = ( uiBits & wiimote_state::buttons::TWO ) != 0;
	a_pButtons[ 6 ]  = ( uiBits & wiimote_state::buttons::HOME ) != 0;
	a_pButtons[ 7 ]  = ( uiBits & wiimote_state::buttons::UP ) != 0;
	a_pButtons[ 8 ]  = ( uiBits & wiimote_state::buttons::DOWN ) != 0;
	a_pButtons[ 9 ]  = ( uiBits & wiimote_state::buttons::LEFT ) != 0;
	a_pButtons[ 10 ] = ( uiBits & wiimote_state::buttons::RIGHT ) != 0;
}

// $deBlob: FUNCTION 0067d670
void TInputWiinDeviceController::Update( TFLOAT a_fDeltaTime )
{
	// Remotes that wait for their connection in the thread aren't read
	if ( m_pRemote == TNULL || s_oWiimoteThread.HasRemote( WIIMOTE_SLOT( m_iIndex ) ) )
	{
		m_bPolled = TFALSE;
		return;
	}

	if ( m_pRemote->RefreshState() == NO_CHANGE )
		return;

	m_oOldState = m_oState;
	m_oState    = *m_pRemote;

	TBOOL bPolled = !m_pRemote->ConnectionLost() && HasNunchuk();
	m_bPolled     = bPolled;

	if ( bPolled )
	{
		if ( HasNunchuk() )
		{
			UnpackButtons( m_oState, m_abButtons );
			UnpackButtons( m_oOldState, m_abOldButtons );

			// C and Z behind the remote's buttons
			m_abButtons[ 12 ]    = m_oState.Nunchuk.C;
			m_abButtons[ 13 ]    = m_oState.Nunchuk.Z;
			m_abOldButtons[ 12 ] = m_oOldState.Nunchuk.C;
			m_abOldButtons[ 13 ] = m_oOldState.Nunchuk.Z;
		}

		UpdateVirtualButtons();
		m_bPolled = TTRUE;
	}
}

// $deBlob: FUNCTION 0067d780
TINT TInputWiinDeviceController::ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime )
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

	if ( !HasNunchuk() )
		return iNumProcessed;

	// NOTE: C and Z (12 and 13) have no doodad, their events are thrown with doodad 0
	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
	{
		if ( i == NUM_REMOTE_BUTTONS ) continue;

		if ( m_abButtons[ i ] && !m_abOldButtons[ i ] )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, TranslateButtonToDoodad( i ), TInputInterface::EVENT_TYPE_GONE_DOWN ) );
			iNumProcessed++;
		}
		else if ( !m_abButtons[ i ] && m_abOldButtons[ i ] )
		{
			a_rEmitter.Throw( TInputInterface::InputEvent( this, TranslateButtonToDoodad( i ), TInputInterface::EVENT_TYPE_GONE_UP ) );
			iNumProcessed++;
		}
	}

	// The Nunchuk's joystick is the D-pad: a press when it gets pushed, repeated every 150 ms after the first 300 ms
	// NOTE: while it's centred it throws a release every frame
	static TUINT64 s_uiNextHorizontal = 0;
	static TBOOL   s_bRepeatHorizontal;
	static TBOOL   s_bLeft;
	static TUINT64 s_uiNextVertical = 0;
	static TBOOL   s_bRepeatVertical;
	static TBOOL   s_bUp;

	const TUINT64 uiNow = GetTickCount64();

	if ( uiNow > s_uiNextHorizontal )
	{
		const TFLOAT fX = m_oState.Nunchuk.Joystick.X;
		Doodad       iDoodad;
		TInputInterface::EVENT_TYPE eType;

		if ( fX >= 0.7f )
		{
			TUINT uiDelay       = s_bRepeatHorizontal ? 150 : 300;
			s_bRepeatHorizontal = TTRUE;
			s_bLeft             = TFALSE;
			s_uiNextHorizontal  = uiNow + uiDelay;
			iDoodad             = HasNunchuk() ? BUTTON_DPAD_RIGHT : -1;
			eType               = TInputInterface::EVENT_TYPE_GONE_DOWN;
		}
		else if ( fX > -0.7f )
		{
			s_bRepeatHorizontal = TFALSE;
			iDoodad             = TranslateButtonToDoodad( s_bLeft ? 9 : 10 );
			eType               = TInputInterface::EVENT_TYPE_GONE_UP;
		}
		else
		{
			TUINT uiDelay       = s_bRepeatHorizontal ? 150 : 300;
			s_bRepeatHorizontal = TTRUE;
			s_bLeft             = TTRUE;
			s_uiNextHorizontal  = uiNow + uiDelay;
			iDoodad             = HasNunchuk() ? BUTTON_DPAD_LEFT : -1;
			eType               = TInputInterface::EVENT_TYPE_GONE_DOWN;
		}

		a_rEmitter.Throw( TInputInterface::InputEvent( this, iDoodad, eType ) );
		iNumProcessed++;
	}

	if ( uiNow > s_uiNextVertical )
	{
		const TFLOAT fY = m_oState.Nunchuk.Joystick.Y;
		Doodad       iDoodad;
		TInputInterface::EVENT_TYPE eType;

		if ( fY >= 0.7f )
		{
			TUINT uiDelay     = s_bRepeatVertical ? 150 : 300;
			s_bRepeatVertical = TTRUE;
			s_bUp             = TTRUE;
			s_uiNextVertical  = uiNow + uiDelay;
			iDoodad           = HasNunchuk() ? BUTTON_DPAD_UP : -1;
			eType             = TInputInterface::EVENT_TYPE_GONE_DOWN;
		}
		else if ( fY > -0.7f )
		{
			s_bRepeatVertical = TFALSE;
			iDoodad           = TranslateButtonToDoodad( s_bUp ? 7 : 8 );
			eType             = TInputInterface::EVENT_TYPE_GONE_UP;
		}
		else
		{
			TUINT uiDelay     = s_bRepeatVertical ? 150 : 300;
			s_bRepeatVertical = TTRUE;
			s_bUp             = TFALSE;
			s_uiNextVertical  = uiNow + uiDelay;
			iDoodad           = HasNunchuk() ? BUTTON_DPAD_DOWN : -1;
			eType             = TInputInterface::EVENT_TYPE_GONE_DOWN;
		}

		a_rEmitter.Throw( TInputInterface::InputEvent( this, iDoodad, eType ) );
		iNumProcessed++;
	}

	// The pointer: the middle of the first two IR dots (thrown with doodad 0)
	if ( m_oState.IR.Dot[ 0 ].bVisible || m_oState.IR.Dot[ 1 ].bVisible )
	{
		a_rEmitter.Throw( TInputInterface::InputEvent(
		    this,
		    0,
		    TInputInterface::EVENT_TYPE_MOVED,
		    ( m_oState.IR.Dot[ 0 ].X + 0.0f + m_oState.IR.Dot[ 1 ].X ) * 0.5f,
		    ( m_oState.IR.Dot[ 0 ].Y + 0.0f + m_oState.IR.Dot[ 1 ].Y ) * 0.5f
		) );

		iNumProcessed++;
	}

	return iNumProcessed;
}

// $deBlob: FUNCTION 0067dc10
TINT TInputWiinDeviceController::GetButtonCount() const
{
	return HasNunchuk() ? NUM_REMOTE_BUTTONS : -1;
}

// $deBlob: FUNCTION 0067dc30
TINT TInputWiinDeviceController::GetAxisCount() const
{
	return HasNunchuk() ? 1 : -1;
}

// $deBlob: FUNCTION 0067dc50
TBOOL TInputWiinDeviceController::IsDown( Doodad a_iDoodad ) const
{
	TINT iButton = TranslateDoodadToButton( a_iDoodad );

	if ( iButton < 0 )
		return TFALSE;

	if ( !HasNunchuk() )
		return TInputDeviceController::IsDown( a_iDoodad );

	return m_abButtons[ iButton ];
}

// $deBlob: FUNCTION 0067dcb0
TBOOL TInputWiinDeviceController::WasDown( Doodad a_iDoodad ) const
{
	TINT iButton = TranslateDoodadToButton( a_iDoodad );

	if ( iButton < 0 )
		return TFALSE;

	// NOTE: retail calls TInputDeviceController::IsDown here, not WasDown
	if ( !HasNunchuk() )
		return TInputDeviceController::IsDown( a_iDoodad );

	return m_abOldButtons[ iButton ];
}

// $deBlob: FUNCTION 0067dd10
TINT TInputWiinDeviceController::GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const
{
	TINT iResult = 0;

	// NOTE: the joystick's -1 - 1 range is centred on 32000 like the DirectInput axes: the result is always about -32000
	if ( HasNunchuk() && a_iDoodad == STICK_LEFT )
	{
		if ( a_iAxis == 0 )
			iResult = TINT( m_oState.Nunchuk.Joystick.X ) - 32000;
		else if ( a_iAxis == 1 )
			iResult = TINT( m_oState.Nunchuk.Joystick.Y ) - 32000;
	}

	if ( iResult <= ms_iDeadZone && iResult >= -ms_iDeadZone )
		return 0;

	return iResult;
}

// $deBlob: FUNCTION 0067dd70
TFLOAT TInputWiinDeviceController::GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const
{
	return GetAxisInt( a_iDoodad, a_iAxis ) / 32000.0f;
}

// $deBlob: FUNCTION 0067dda0
TInputDevice::Doodad TInputWiinDeviceController::TranslateButtonToDoodad( TINT a_iButton ) const
{
	if ( !HasNunchuk() )
		return -1;

	switch ( a_iButton )
	{
		case 0: return BUTTON_CONTEXT_1;  // A
		case 1: return BUTTON_SELECT;     // B
		case 2: return BUTTON_START;      // Plus
		case 3: return BUTTON_CONTEXT_2;  // Minus
		case 4: return BUTTON_CONTEXT_3;  // One
		case 5: return BUTTON_CONTEXT_4;  // Two
		case 7: return BUTTON_DPAD_UP;    // Up
		case 8: return BUTTON_DPAD_DOWN;  // Down
		case 9: return BUTTON_DPAD_LEFT;  // Left
		case 10: return BUTTON_DPAD_RIGHT; // Right
		default: return 0;
	}
}

// $deBlob: FUNCTION 0067de60
TINT TInputWiinDeviceController::TranslateDoodadToButton( Doodad a_iDoodad ) const
{
	if ( !HasNunchuk() )
		return -1;

	switch ( a_iDoodad )
	{
		case BUTTON_DPAD_UP: return 7;
		case BUTTON_DPAD_RIGHT: return 10;
		case BUTTON_DPAD_DOWN: return 8;
		case BUTTON_DPAD_LEFT: return 9;
		case BUTTON_SELECT: return 1;
		case BUTTON_START: return 2;
		case BUTTON_CONTEXT_1: return 0;
		case BUTTON_CONTEXT_2: return 3;
		case BUTTON_CONTEXT_3: return 4;
		case BUTTON_CONTEXT_4: return 5;
		default: return -1;
	}
}

// $deBlob: FUNCTION 0067df20
TBOOL TInputWiinDeviceController::IsAnyButtonDown() const
{
	if ( !HasNunchuk() )
		return TFALSE;

	for ( TINT i = 0; i < NUM_REMOTE_BUTTONS; i++ )
	{
		if ( m_abButtons[ i ] )
			return TTRUE;
	}

	for ( TINT i = 12; i < NUM_BUTTONS; i++ )
	{
		if ( m_abButtons[ i ] )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0067df70
void TInputWiinDeviceController::Rumble( TINT a_iMilliseconds )
{
	m_pRemote->RumbleForAsync( a_iMilliseconds );
}

// $deBlob: FUNCTION 0067df80
TUINT64 TInputWiinDeviceController::GetTimeMicroseconds()
{
	static LARGE_INTEGER s_oFrequency;
	static LARGE_INTEGER s_oStart;
	static TBOOL         s_bInitialised;

	if ( !s_bInitialised )
	{
		QueryPerformanceFrequency( &s_oFrequency );
		QueryPerformanceCounter( &s_oStart );
		s_bInitialised = TTRUE;
	}

	LARGE_INTEGER oNow;
	QueryPerformanceCounter( &oNow );

	return TUINT64( TDOUBLE( oNow.QuadPart - s_oStart.QuadPart ) / TDOUBLE( s_oFrequency.QuadPart ) * 1000000.0 );
}

// $deBlob: FUNCTION 0067e010
void TInputWiinDeviceController::StateChangedCallback( wiimote& a_rRemote, state_change_flags a_eChanged, const wiimote_state& a_rNewState )
{
	// NOTE: like the callback of WiiYourself!'s demo: the report type follows the plugged in extension, a rumble
	// tells that an extension got plugged in or out. Retail's WiiYourself! is another version than ours (wiimote's
	// layout differs, its extension_type has BALANCE_BOARD = 7), so the checks use the names.
	if ( a_eChanged & CONNECTED )
	{
		if ( a_rNewState.ExtensionType != wiimote_state::BALANCE_BOARD )
			a_rRemote.SetReportType( a_rNewState.bExtension ? wiimote::IN_BUTTONS_ACCEL_IR_EXT : wiimote::IN_BUTTONS_ACCEL_IR );
	}

	if ( a_eChanged & MOTIONPLUS_DETECTED )
	{
		if ( a_rRemote.ExtensionType == wiimote_state::NONE )
			a_rRemote.EnableMotionPlus();
	}
	else if ( a_eChanged & MOTIONPLUS_EXTENSION_CONNECTED )
	{
		if ( a_rRemote.MotionPlusEnabled() )
			a_rRemote.DisableMotionPlus();
	}
	else if ( a_eChanged & MOTIONPLUS_EXTENSION_DISCONNECTED )
	{
		if ( a_rRemote.MotionPlusConnected() )
			a_rRemote.EnableMotionPlus();
	}
	else if ( a_eChanged & ( NUNCHUK_CONNECTED | CLASSIC_CONNECTED | BALANCE_CONNECTED | MOTIONPLUS_ENABLED ) )
	{
		a_rRemote.RumbleForAsync( 150 );

		if ( !a_rRemote.IsBalanceBoard() )
			a_rRemote.SetReportType( wiimote::IN_BUTTONS_ACCEL_IR_EXT );
	}
	else if ( a_eChanged & EXTENSION_DISCONNECTED )
	{
		a_rRemote.RumbleForAsync( 150 );
		a_rRemote.SetReportType( wiimote::IN_BUTTONS_ACCEL_IR );
	}
}

TOSHI_NAMESPACE_END

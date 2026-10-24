#pragma once
#include "Includes.h"
#include "Input/TInputDeviceController.h"

#include <wiimote.h>

TOSHI_NAMESPACE_START

// A Wii remote (with a Nunchuk) through WiiYourself! (gl.tter's wiimote class, Toshi/Vendor/WiiYourself).
// TInputDXInterface::Initialise creates four of them. NOTE: nothing in retail ever calls wiimote::Connect (the
// WiimoteThread only sleeps), so the remotes never get connected and Acquire always fails
class TInputWiinDeviceController : public TInputDeviceController
{
public:
	TDECLARE_CLASS( TInputWiinDeviceController, TInputDeviceController );

	static TCONSTEXPR_VAR TINT NUM_REMOTE_BUTTONS = 11; // A, B, Plus, Minus, One, Two, Home, Up, Down, Left, Right
	static TCONSTEXPR_VAR TINT NUM_BUTTONS        = 14; // and C and Z of the Nunchuk at 12 and 13

public:
	TInputWiinDeviceController();
	~TInputWiinDeviceController();

	// The state ABlobInputMap::Update reads directly
	const wiimote_state& GetState() const { return m_oState; }
	// $deBlob: FUNCTION 00642480
	TINT                 GetIndex() const { return m_iIndex; }
	TBOOL                IsButtonStateDown( TINT a_iButton ) const { return m_abButtons[ a_iButton ]; }

	//-----------------------------------------------------------------------------
	// TInputDevice
	//-----------------------------------------------------------------------------
	virtual TBOOL Acquire() override;
	virtual TBOOL Unacquire() override;
	virtual void  Release() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;
	virtual TINT  ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime ) override;
	virtual TINT  GetButtonCount() const override;
	virtual TINT  GetAxisCount() const override;
	// $deBlob: FUNCTION 0067e280
	virtual GAMEPADTYPE GetGamepadType() const override { return Gamepad_Wii; }
	virtual TBOOL       IsDown( Doodad a_iDoodad ) const override;
	virtual TINT        GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TFLOAT      GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const override;
	// $deBlob: FUNCTION 0067e2c0
	virtual TBOOL IsEnabled() const override { return m_bPolled; }

	//-----------------------------------------------------------------------------
	// TInputDeviceController
	//-----------------------------------------------------------------------------
	virtual TBOOL WasDown( Doodad a_iDoodad ) const override;
	// $deBlob: FUNCTION 0067e290
	virtual TINT GetPortIndex() override { return -1; }

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// Sets the index of the remote (1-4: its slot in the WiimoteThread and its LEDs) and the state callback
	virtual TBOOL Initialise( TINT a_iIndex ); // +0x70
	// $deBlob: FUNCTION 0067d380
	virtual TBOOL Deinitialise() { return TTRUE; } // +0x74

	// Index in the button states (by the doodad), -1 if it has none or no Nunchuk is plugged in
	TINT   TranslateDoodadToButton( Doodad a_iDoodad ) const;
	Doodad TranslateButtonToDoodad( TINT a_iButton ) const;

	// Whether a Nunchuk (or a Classic Controller) is plugged in
	TBOOL HasNunchuk() const { return m_pRemote->ExtensionType == wiimote_state::NUNCHUK || m_pRemote->ExtensionType == wiimote_state::CLASSIC; }

	// Whether one of the remote's or the Nunchuk's buttons is down
	TBOOL IsAnyButtonDown() const;

	// Rumbles the remote for a_iMilliseconds
	void Rumble( TINT a_iMilliseconds );

	// Microseconds since the first call (QueryPerformanceCounter)
	static TUINT64 GetTimeMicroseconds();

	static TINT ms_iDeadZone; // of GetAxisInt

private:
	// The changed callback of the wiimotes (sets the report type when extensions get plugged in and out)
	static void StateChangedCallback( wiimote& a_rRemote, state_change_flags a_eChanged, const wiimote_state& a_rNewState );

	// Unpacks the button bits of a_rState
	static void UnpackButtons( const wiimote_state& a_rState, TBOOL* a_pButtons );

private:
	// NOTE: retail's WiiYourself! has a bigger wiimote_state (0x1E0 bytes, the buttons are unpacked into it at 0x06
	// and the Nunchuk's C and Z are copied behind it), our 1.15 one is 0x1C4: the states and the unpacked buttons are
	// members of their own, so the layout differs from 0x58 on (retail: 0x430 bytes)
	wiimote_state m_oState;                      // 0x058
	wiimote_state m_oOldState;                   // 0x238
	TBOOL         m_abButtons[ NUM_BUTTONS ];
	TBOOL         m_abOldButtons[ NUM_BUTTONS ];
	TBOOL         m_bPolled;                     // 0x418
	wiimote*      m_pRemote;                     // 0x41C
	TINT          m_iIndex;                      // 0x420
	TUINT32       m_uiUnk1;                      // 0x428
	TUINT32       m_uiUnk2;                      // 0x42C
};

TOSHI_NAMESPACE_END

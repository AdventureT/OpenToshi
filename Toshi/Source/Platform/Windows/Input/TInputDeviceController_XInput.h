#pragma once
#include "Includes.h"
#include "Input/TInputDeviceController.h"

TOSHI_NAMESPACE_START

// One of the four XInput user indices, TInputDXInterface::Initialise creates all four
class TInputDeviceController_XInput : public TInputDeviceController
{
public:
	TDECLARE_CLASS( TInputDeviceController_XInput, TInputDeviceController );

	static TCONSTEXPR_VAR TINT  NUM_BUTTONS       = 16;
	static TCONSTEXPR_VAR TINT  NUM_BUTTON_BITS   = 10;
	static TCONSTEXPR_VAR TUINT TRIGGER_THRESHOLD = 30;   // XINPUT_GAMEPAD_TRIGGER_THRESHOLD
	static TCONSTEXPR_VAR TINT  THUMB_DEADZONE    = 7500; // the sticks are cut below it in Update

public:
	TInputDeviceController_XInput( TINT a_iUserIndex = 0 );
	~TInputDeviceController_XInput();

	// The state ABlobInputMap::Update reads directly
	TUINT32 GetButtonBits() const { return m_uiButtons; }
	TUINT32 GetOldButtonBits() const { return m_uiOldButtons; }
	TFLOAT  GetAxis( TINT a_iAxis ) const { return m_afAxes[ a_iAxis ]; }

	//-----------------------------------------------------------------------------
	// TInputDevice
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0067ef10
	virtual TBOOL Acquire() override;
	// $deBlob: FUNCTION 0067ef80
	virtual TBOOL Unacquire() override
	{
		SetAcquired( TFALSE );
		return TTRUE;
	}
	// $deBlob: FUNCTION 0067ef70
	virtual void Release() override {}
	virtual void Update( TFLOAT a_fDeltaTime ) override;
	virtual TINT ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime ) override;
	// $deBlob: FUNCTION 0067ef30
	virtual TINT GetButtonCount() const override { return 8; }
	// $deBlob: FUNCTION 0067ef20
	virtual TINT GetAxisCount() const override { return 2; }
	// $deBlob: FUNCTION 0067ef40
	virtual GAMEPADTYPE GetGamepadType() const override { return Gamepad_XBox; }
	virtual TBOOL       IsDown( Doodad a_iDoodad ) const override;
	virtual TINT        GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TFLOAT      GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const override;
	// $deBlob: FUNCTION 0067ef60
	virtual TBOOL IsEnabled() const override { return m_bConnected; }

	//-----------------------------------------------------------------------------
	// TInputDeviceController
	//-----------------------------------------------------------------------------
	virtual TBOOL WasDown( Doodad a_iDoodad ) const override;
	// $deBlob: FUNCTION 0067ef50
	virtual TINT GetPortIndex() override { return -1; }

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0067e510
	virtual TBOOL Initialise() { return TTRUE; } // +0x70
	// $deBlob: FUNCTION 0067e520
	virtual TBOOL Deinitialise() { return TTRUE; } // +0x74

	// Runs both motors at a_uiForce (0-65535) for a_uiMilliseconds (at most 2 seconds), Update stops them
	TBOOL Vibrate( TUINT a_uiMilliseconds, TUINT a_uiForce );

	// Whether one of the buttons, sticks or virtual buttons is down
	TBOOL IsAnyButtonDown() const;

	// XINPUT_GAMEPAD button mask of a doodad, -1 if it isn't a button
	static TINT   TranslateDoodadToButton( Doodad a_iDoodad );
	static Doodad TranslateButtonToDoodad( TINT a_iButton );

	static TINT ApplyDeadZone( TINT a_iValue, TINT a_iDeadZone );

	static TINT ms_iDeadZone; // of GetAxisInt

private:
	DWORD        m_dwUserIndex;         // 0x58
	TBOOL        m_bConnected;          // 0x5C XInputGetState succeeded
	TBOOL        m_bPolled;             // 0x5D
	TUINT64      m_uiVibrationEndTime;  // 0x60 GetTickCount64
	TBOOL        m_bVibrating;          // 0x68
	XINPUT_STATE m_oState;              // 0x6C
	XINPUT_STATE m_oOldState;           // 0x7C
	TUINT32      m_uiButtons;           // 0x8C bits of s_aButtonMasks, 10: left trigger, 11: right trigger
	TUINT32      m_uiOldButtons;        // 0x90
	TFLOAT       m_afAxes[ 6 ];         // 0x94 left X and Y, (unused), right X and Y, (unused)
	TUINT32      m_uiUnused;            // 0xAC
};

TSTATICASSERT( sizeof( TInputDeviceController_XInput ) == 0xB0 );

TOSHI_NAMESPACE_END

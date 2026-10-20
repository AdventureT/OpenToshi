#pragma once
#include "Includes.h"
#include "Input/TInputDeviceController.h"

TOSHI_NAMESPACE_START

// DirectInput controller; TInputDXInterface only creates them for PlayStation controllers (IsPSController), the others
// are read through XInput (TInputDeviceController_XInput)
class TInputDXDeviceController : public TInputDeviceController
{
public:
	TDECLARE_CLASS( TInputDXDeviceController, TInputDeviceController );

	static TCONSTEXPR_VAR TINT NUM_DX_BUTTONS = 16;

	static TCONSTEXPR_VAR TINT STICK_CENTRE   = 0x8000;
	static TCONSTEXPR_VAR TINT STICK_SATURATE = 32000;

	// DIDEVICEINSTANCE::dwDevType of a standard HID gamepad (DI8DEVTYPE_GAMEPAD | DI8DEVTYPEGAMEPAD_STANDARD << 8 | DIDEVTYPE_HID):
	// its right stick is on the X and Y rotations instead of Z and the Z rotation
	static TCONSTEXPR_VAR DWORD DEVTYPE_HID_GAMEPAD = 0x10215;

public:
	TInputDXDeviceController();
	~TInputDXDeviceController();

	// The state ABlobInputMap::Update reads directly
	TUINT32 GetButtonBits() const { return m_uiButtons; }
	TUINT32 GetOldButtonBits() const { return m_uiOldButtons; }
	TFLOAT  GetAxis( TINT a_iAxis ) const { return m_afAxes[ a_iAxis ]; }

	//-----------------------------------------------------------------------------
	// TInputDevice
	//-----------------------------------------------------------------------------
	virtual TBOOL Acquire() override;
	virtual TBOOL Unacquire() override;
	virtual void  Release() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;
	virtual TINT  ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime ) override;
	// $deBlob: FUNCTION 0067be90
	virtual TINT GetButtonCount() const override { return m_oDIDevCaps.dwButtons; }
	// $deBlob: FUNCTION 0067bea0
	virtual TINT GetAxisCount() const override { return m_oDIDevCaps.dwAxes; }
	// $deBlob: FUNCTION 0067c700
	virtual GAMEPADTYPE GetGamepadType() const override { return m_bIsPSController ? Gamepad_PS4 : Gamepad_XBox; }
	virtual TBOOL       IsDown( Doodad a_iDoodad ) const override;
	virtual TINT        GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TFLOAT      GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const override;
	// $deBlob: FUNCTION 0067c740
	virtual TBOOL IsEnabled() const override { return m_bPolled; }

	//-----------------------------------------------------------------------------
	// TInputDeviceController
	//-----------------------------------------------------------------------------
	virtual TBOOL WasDown( Doodad a_iDoodad ) const override;
	// $deBlob: FUNCTION 0067c730
	virtual TINT GetPortIndex() override { return -1; }

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0067b8e0
	virtual TBOOL Initialise() { return TTRUE; } // +0x70
	// $deBlob: FUNCTION 0067b8f0
	virtual TBOOL Deinitialise() { return TTRUE; } // +0x74

	TBOOL const BindToDIDevice( HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDXDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice );

	// $deBlob: FUNCTION 0067c0f0
	Doodad TranslateDXButtonToDoodad( TINT a_iButton ) const { return TranslateDXButtonToDoodad( a_iButton, m_bIsPSController ); }
	// $deBlob: FUNCTION 0067c110
	TINT TranslateDoodadToDXButton( Doodad a_iDoodad ) const { return TranslateDoodadToDXButton( a_iDoodad, m_bIsPSController ); }

	static Doodad TranslateDXButtonToDoodad( TINT a_iButton, TBOOL a_bPSController );
	static TINT   TranslateDoodadToDXButton( Doodad a_iDoodad, TBOOL a_bPSController );

	// Whether the product GUID is one of the Sony controllers (DualShock 3/4, PS Vita...) or of some known clones
	static TBOOL IsPSController( LPCDIDEVICEINSTANCEA a_poDXDeviceInstance );

	// Whether one of the buttons, sticks or virtual buttons is down
	TBOOL IsAnyButtonDown() const;

	static TINT ms_iDeadZone;

private:
	static TFLOAT        NormaliseStick( TINT a_iValue );
	static BOOL CALLBACK EnumerateObjectCallback( LPCDIDEVICEOBJECTINSTANCEA a_poDeviceInstance, LPVOID a_pData );

private:
	DIDEVICEINSTANCEA     m_oDeviceInstance;    // 0x058
	DIDEVCAPS             m_oDIDevCaps;         // 0x29C
	IDirectInputDevice8A* m_poDXInputDevice;    // 0x2C8
	DIJOYSTATE            m_oJoyState;          // 0x2CC
	DIJOYSTATE            m_oOldJoyState;       // 0x31C
	TUINT32               m_uiButtons;          // 0x36C bit i: rgbButtons[ i ] is down
	TUINT32               m_uiOldButtons;       // 0x370
	TFLOAT                m_afAxes[ 6 ];        // 0x374 normalised lX, lY, lZ, lRx, lRy and lRz (the POV hat moves X and Y)
	TBOOL                 m_bPolled;            // 0x38C
	TBOOL                 m_bIsPSController;    // 0x38D
};

TSTATICASSERT( sizeof( TInputDXDeviceController ) == 0x390 );

TOSHI_NAMESPACE_END

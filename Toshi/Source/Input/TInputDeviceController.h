/**
 * @file TInputDeviceController.h
 * Controller input device interface for the Toshi engine
 * 
 * This file defines the controller input device that provides:
 * - Game controller input handling
 * - Button and axis state management
 * - Controller-specific input mapping
 * - Force feedback support
 */

#pragma once
#include "Input/TInputInterface.h"

TOSHI_NAMESPACE_START

class TInputDeviceController : public TInputDevice
{
public:
	TDECLARE_CLASS( TInputDeviceController, TInputDevice );

public:
	static TCONSTEXPR_VAR TUINT NUM_BUTTONS = 8;

	enum : Doodad
	{
		BUTTON_DPAD_UP            = 0x10001,
		BUTTON_DPAD_RIGHT         = 0x10002,
		BUTTON_DPAD_DOWN          = 0x10003,
		BUTTON_DPAD_LEFT          = 0x10004,
		BUTTON_SELECT             = 0x10005,
		BUTTON_START              = 0x10006,
		BUTTON_STICK_LEFT         = 0x10007,
		BUTTON_STICK_RIGHT        = 0x10008,
		BUTTON_CONTEXT_1          = 0x10009,
		BUTTON_CONTEXT_2          = 0x1000a,
		BUTTON_CONTEXT_3          = 0x1000b,
		BUTTON_CONTEXT_4          = 0x1000c,
		BUTTON_SHOULDER_LEFT_1    = 0x1000d,
		BUTTON_SHOULDER_LEFT_2    = 0x1000e,
		BUTTON_SHOULDER_RIGHT_1   = 0x1000f,
		BUTTON_SHOULDER_RIGHT_2   = 0x10010,
		VIRTBTN_STICK_LEFT_UP     = 0x10011,
		VIRTBTN_STICK_LEFT_RIGHT  = 0x10012,
		VIRTBTN_STICK_LEFT_DOWN   = 0x10013,
		VIRTBTN_STICK_LEFT_LEFT   = 0x10014,
		VIRTBTN_STICK_RIGHT_UP    = 0x10015,
		VIRTBTN_STICK_RIGHT_RIGHT = 0x10016,
		VIRTBTN_STICK_RIGHT_DOWN  = 0x10017,
		VIRTBTN_STICK_RIGHT_LEFT  = 0x10018,
		STICK_LEFT                = 0x10019,
		STICK_RIGHT               = 0x1001a,
		VIRTSTK_DPAD              = 0x1001b,
	};

	enum VIBRATION_ACTUATOR
	{
		VIBRATION_ACTUATOR_0, // the two motors, AInputHandler's positional rumble uses the second one
		VIBRATION_ACTUATOR_1,
	};

public:
	TInputDeviceController();
	~TInputDeviceController();

	virtual TBOOL        GetDoodadProperties( Doodad a_iDoodad, DoodadProperties& a_rProperties ) const override;
	virtual const TCHAR* GetButtonFromDoodad( Doodad a_iDoodad ) const override;
	virtual TBOOL        IsDown( Doodad a_iDoodad ) const override;

	// NOTE: no controller overrides it, the XInput controller vibrates through its own Vibrate
	virtual void  SetVibration( VIBRATION_ACTUATOR a_eActuator, TFLOAT a_fValue );       // +0x5C
	virtual TBOOL WasDown( Doodad a_iDoodad ) const;                                     // +0x60
	virtual TINT  GetPortIndex() = 0;                                                    // +0x64, every controller returns -1
	virtual TBOOL Unknown2();                                                            // +0x68
	virtual TINT  ProcessVirtualButtons( EventEmitter& a_rEmitter, float a_fDeltaTime ); // +0x6C

	// Maps a button index of the game's button layouts (0-11) to the PlayStation layout if a_bPSLayout is set
	// (swaps 0-2 and moves 6-7 behind 8-11), -1 for invalid indices
	static TINT RemapButtonIndex( TINT a_iButton, TBOOL a_bPSLayout );

protected:
	// Turns the sticks into the virtual direction buttons (0x10011-0x10018) and the D-pad into m_fMag1/m_fMag2
	void UpdateVirtualButtons();

private:
	TBOOL* m_CurrentState; // 0x40
	TBOOL* m_OldState;     // 0x44
	TFLOAT m_fMag1;        // 0x48
	TFLOAT m_fMag2;        // 0x4C
	TFLOAT m_fOldMag1;     // 0x50
	TFLOAT m_fOldMag2;     // 0x54
};

TSTATICASSERT( sizeof( TInputDeviceController ) == 0x58 );

TOSHI_NAMESPACE_END

#pragma once
#include "AInputMap.h"
#include "AStack.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2Vector.h>
#include <Math/TVector2.h>
#include <Input/TInputInterface.h>

//-----------------------------------------------------------------------------
// Purpose: the game's input (AInputManager2_Win.cpp): creates the input interface, keeps the connected controllers, the
// controllers of the players, the input map and the input context stack.
//-----------------------------------------------------------------------------
class AInputManager2 : public Toshi::TSingleton<AInputManager2>
{
public:
	// The devices: the controllers (the order they were found in) and the keyboard
	typedef TINT INPUTDEVICE;
	enum INPUTDEVICE_ : INPUTDEVICE
	{
		INPUTDEVICE_Controller1 = 0,
		INPUTDEVICE_Controller2 = 1,
		INPUTDEVICE_Controller3 = 2,
		INPUTDEVICE_Controller4 = 3,
		INPUTDEVICE_Count       = 4,
		INPUTDEVICE_Keyboard    = 5,
		INPUTDEVICE_Invalid     = 6,
	};

	static TCONSTEXPR_VAR TINT MAX_NUM_CONTROLLERS = INPUTDEVICE_Count;

	struct AInputDeviceHandle
	{
		AInputDeviceHandle()
		    : m_pDevice( TNULL ), m_iId( 0 ) {}
		AInputDeviceHandle( Toshi::TInputDevice* a_pDevice, TINT a_iId )
		    : m_pDevice( a_pDevice ), m_iId( a_iId ) {}

		TBOOL IsValid() const { return m_pDevice != TNULL; }

		TBOOL operator==( const AInputDeviceHandle& a_rOther ) const { return m_pDevice == a_rOther.m_pDevice && m_iId == a_rOther.m_iId; }
		TBOOL operator!=( const AInputDeviceHandle& a_rOther ) const { return !( *this == a_rOther ); }

		Toshi::TInputDevice* m_pDevice; // 0x00
		TINT                 m_iId;     // 0x04
	};

	// The result of GetButtonDownWithResult: the device that pressed the button (an empty handle for the keyboard)
	struct ButtonResult
	{
		TBOOL              m_bDown;  // 0x0
		AInputDeviceHandle m_Handle; // 0x4
	};

public:
	AInputManager2();
	virtual ~AInputManager2();

	// Processes the input events and updates the controllers
	void Update( TFLOAT a_fDeltaTime );

	// The number of connected controllers, at least 1
	TINT GetConnectedDeviceCount();

	// The handle of a connected controller, TNULL if a_pDevice isn't one
	AInputDeviceHandle* GetDeviceHandle( Toshi::TInputDevice* a_pDevice );

	void                    SetContext( AInputMap::INPUTCONTEXT a_eContext );
	AInputMap::INPUTCONTEXT GetContext();
	void                    RemoveCurrentContext();

	// The controllers of the players
	void  AddInGameController( const AInputDeviceHandle& a_Handle );
	void  RemoveInGameController( const AInputDeviceHandle& a_Handle );
	void  RemoveAllInGameControllers();
	TINT  GetNumInGameControllers() const { return m_InGameControllers.Size(); }
	TBOOL InGameDevicesCheck();

	// TTRUE if a_rHandle may be used (no device is locked, or it's the locked one)
	TBOOL CheckIfValidDevice( const AInputDeviceHandle& a_rHandle );

	// The button configurations (0 or 1) of the devices
	void SetDeviceButtonConfig( TINT a_eConfig );
	void LoadDeviceButtonConfigFromSave( TUINT8 a_uiInputConfig );
	void SetPerDeviceButtonConfig( TINT a_iDevice, TINT a_eConfig );
	TINT GetPerDeviceButtonConfig( TINT a_iDevice );
	void ResetPerDeviceButtonConfig();

	// TTRUE if a doodad bound to a_eButton in a_pButtonMap is down on a_pDevice (remembers the device)
	TBOOL GetButtonDown( AInputMap::INPUTBUTTON a_eButton, AInputMap::ButtonMap* a_pButtonMap, Toshi::TInputDevice* a_pDevice );

	static const TCHAR* Debug_GetDeviceName( INPUTDEVICE a_eDevice );

	AInputDeviceHandle GetLockedDevice() const { return m_oLockedDevice; }
	void               SetLockedDevice( const AInputDeviceHandle& a_rHandle ) { m_oLockedDevice = a_rHandle; } // 0x180 (AInGameStateHelper::OnUpdate)

	// AInputManager2_Win.cpp
	AInputDeviceHandle& GetControllerHandle( INPUTDEVICE a_eDevice );

	void GetCameraAimAxis( Toshi::TVector2& a_rAxis, const AInputDeviceHandle& a_rHandle );
	void GetCameraMoveAxis( Toshi::TVector2& a_rAxis, const AInputDeviceHandle& a_rHandle );
	void GetPlayerMoveAxis( Toshi::TVector2& a_rAxis, const AInputDeviceHandle& a_rHandle );
	void GetAnalogStickMoveAxis( Toshi::TVector2& a_rAxis, const AInputDeviceHandle& a_rHandle, TINT a_eStick );

	static void  GetCursorXY( TINT& a_rX, TINT& a_rY );
	static void  GetCursorDeltaXY( TINT& a_rX, TINT& a_rY );
	static TBOOL IsLeftMouseDown();
	static TBOOL IsRightMouseDown();

	// The buttons of the current context: any controller (and the keyboard)
	TBOOL        GetButtonDown( AInputMap::INPUTBUTTON a_eButton );
	TBOOL        GetButtonDown( AInputMap::INPUTBUTTON a_eButton, const AInputDeviceHandle& a_rHandle );
	ButtonResult GetButtonDownWithResult( AInputMap::INPUTBUTTON a_eButton );

	TBOOL IsAnyButtonPressed();

	AInputMap& GetInputMap() { return m_InputMap; }

private:
	// Puts the enabled controllers in m_aControllers
	void UpdateControllers();

	// Gamepad type of the last used device: keyboard when no controller is connected (shared by the constructor and
	// Update)
	void CheckConnectedControllers();

private:
	AInputDeviceHandle                     m_aControllers[ MAX_NUM_CONTROLLERS ]; // 0x04
	Toshi::T2Vector<AInputDeviceHandle, 4> m_InGameControllers;                   // 0x24
	AInputMap                              m_InputMap;                            // 0x48
	AInputDeviceHandle                     m_oLockedDevice;                       // 0x180
	Toshi::TInputDevice*                   m_pLastButtonDevice;                   // 0x188 the device of the last GetButtonDown
	TINT                                   m_eButtonConfig;                       // 0x18C
	TINT                                   m_aDeviceButtonConfigs[ 4 ];           // 0x190
	AStack<AInputMap::INPUTCONTEXT, 10>    m_InputContextStack;                   // 0x1A0
};

TSTATICASSERT( sizeof( AInputManager2 ) == 0x1CC );

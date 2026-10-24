/**
 * @file TInputInterface.h
 * Input system interface for the Toshi engine
 *
 * This file defines the core input system that provides:
 * - Input device management (keyboard, mouse, controllers)
 * - Input event handling
 * - Input state tracking
 * - Input device abstraction
 */

#pragma once
#include "Toshi/TNodeList.h"
#include "Toshi/TArray.h"
#include "Toshi/TString8.h"

TOSHI_NAMESPACE_START

class TInputDevice;

class TInputInterface : public TObject
    , public TSingleton<TInputInterface>
{
public:
	TDECLARE_CLASS( TInputInterface, TObject );

public:
	enum EVENT_TYPE
	{
		EVENT_TYPE_GONE_DOWN, // "GONEDOWN"
		EVENT_TYPE_GONE_UP,   // "GONEUP"
		EVENT_TYPE_REPEAT,    // "REPEAT"
		EVENT_TYPE_VIRTUAL,   // "MOVEDABS": sticks, the D-pad and the keyboard's virtual stick (absolute magnitudes)
		EVENT_TYPE_MOVED      // "MOVEDREL": the mouse cursor and wheel (relative magnitudes)
	};

	class InputEvent
	{
	public:
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType );
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TWCHAR* a_wszString );
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1 );
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1, TINT a_iMagnitude2 );
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1 );
		InputEvent( TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1, TFLOAT a_fMagnitude2 );

		TBOOL StartRepeat( TFLOAT a_fFirstRepeatTime, TFLOAT a_fRepeatTime ) const;

		// Starts repeating the event right away, every frame
		TBOOL StartRepeat() const;

		TINT   GetMagnitudeInt( TINT a_iAxis ) const;
		TFLOAT GetMagnitudeFloat( TINT a_iAxis ) const;

		// Debug text of the event: "<doodad> - <event type>" and the magnitudes, e.g. "KEY_A - GONEDOWN"
		TString8 GetString() const;

		static const TCHAR* GetEventTypeName( EVENT_TYPE a_eEventType );

		TINT          GetDoodad() const { return m_iDoodad; }
		EVENT_TYPE    GetEventType() const { return m_eEventType; }
		TInputDevice* GetSource() const { return m_pSource; }
		TINT8         GetAxisCount() const { return m_iAxisCount; }

		TBOOL IsMagnitudeFloat() const { return m_bIsMagnitudeFloat; }
		TBOOL IsMagnitudeInt() const { return !m_bIsMagnitudeFloat; }

	public:
		TINT          m_iDoodad;           // 0x00
		EVENT_TYPE    m_eEventType;        // 0x04
		TBOOL         m_bIsMagnitudeFloat; // 0x08
		TINT8         m_iAxisCount;        // 0x09
		TInputDevice* m_pSource;           // 0x0C
		TWCHAR        m_wszString[ 4 ];    // 0x10

		union
		{
			TFLOAT Floats[ 2 ];
			TINT   Ints[ 2 ];
		} m_Magnitude; // 0x18 De blob 0x10 JPOG
	};

	using EventEmitter = TEmitter<TInputInterface, TInputInterface::InputEvent>;

	// Thrown through the device status emitter when a device gets connected or disconnected
	// (the controllers do it when acquiring starts failing or succeeding again)
	enum DEVICESTATUS
	{
		DEVICESTATUS_CONNECTED,
		DEVICESTATUS_DISCONNECTED,
	};

	struct DeviceStatusEvent
	{
		// $Barnyard: FUNCTION 006c3b80
		DeviceStatusEvent( TInputDevice* a_pDevice, DEVICESTATUS a_eStatus )
		    : pDevice( a_pDevice ), eStatus( a_eStatus ) {}

		TInputDevice* pDevice; // 0x0
		DEVICESTATUS  eStatus; // 0x4
	};

	using DeviceStatusEmitter = TEmitter<TInputInterface, DeviceStatusEvent>;

public:
	TInputInterface();
	~TInputInterface();

	virtual TBOOL Initialise()   = 0;                       // +0x0C
	virtual TBOOL Deinitialise() = 0;                       // +0x10
	virtual TBOOL AcquireAll();                             // +0x14
	virtual TBOOL UnacquireAll();                           // +0x18
	virtual TBOOL FlushAll();                               // +0x1C
	virtual void  SetExclusiveMode( TBOOL a_bIsExclusive ); // +0x20
	virtual TBOOL GetExclusiveMode() const;                 // +0x24
	virtual TINT  ProcessEvents( TFLOAT a_fDeltaTime );     // +0x28
	virtual void  StopAllRepeats();                         // +0x2C

	void AddDevice( TInputDevice* device );
	void RemoveDevice( TInputDevice* device );

	TBOOL ReleaseAllDevices();

	// Throws an event of a_pSource through the input event emitter like the device itself would
	void SendEvent( TInputDevice* a_pSource, TINT a_iDoodad, EVENT_TYPE a_eEventType );

	TInputDevice* GetDeviceByIndex( const TClass* a_pClass, TUINT a_uiIndex );

	EventEmitter&            GetInputEventEmitter() { return m_InputEventEmitter; }
	DeviceStatusEmitter&     GetDeviceStatusEmitter() { return m_DeviceStatusEmitter; }
	TNodeList<TInputDevice>& GetDeviceList() { return m_DeviceList; }

	// $deBlob: FUNCTION 00591490
	// $deBlob: FUNCTION 005abdf0
	// $deBlob: FUNCTION 005d6be0
	// $deBlob: FUNCTION 005d6c50
	// $deBlob: FUNCTION 005dcc30
	// $deBlob: FUNCTION 00642370
	template <class C>
	C* GetDeviceByIndex( TUINT a_uiIndex = 0 )
	{
		C* pDevice = TSTATICCAST( C, GetDeviceByIndex( &TGetClass( C ), a_uiIndex ) );
		TASSERT( pDevice == TNULL || pDevice->GetClass()->IsA( &TGetClass( C ) ) );
		return pDevice;
	}

private:
	TBOOL                   m_bIsExclusiveMode;    // 0x04
	TNodeList<TInputDevice> m_DeviceList;          // 0x08
	EventEmitter            m_InputEventEmitter;   // 0x18
	DeviceStatusEmitter     m_DeviceStatusEmitter; // 0x24
};

class TInputDevice : public TObject
    , public TNodeList<TInputDevice>::TNode
{
public:
	TDECLARE_CLASS( TInputDevice, TObject );

public:
	static TCONSTEXPR_VAR TINT INPUT_DEVICE_MOUSE_BUTTONS = 3;
	static TCONSTEXPR_VAR TINT INPUT_DEVICE_MOUSE_WHEEL   = 4;

	// Players a device can be assigned to (SetPlayerDevice asserts it, the arrays have room for every device)
	static TCONSTEXPR_VAR TINT MAX_PLAYERS = 4;

	using EventEmitter = TInputInterface::EventEmitter;
	using Doodad       = TINT;

	// What kind of device it is (GetGamepadType); the game picks its button prompts and input maps by it
	enum GAMEPADTYPE
	{
		Gamepad_Wii,      // TInputWiinDeviceController
		Gamepad_XBox,     // XInput controllers and DirectInput controllers that aren't PlayStation ones
		Gamepad_Keyboard, // keyboard and mouse (the default of the last used device type)
		Gamepad_PS4,      // DirectInput PlayStation controllers (TInputDXDeviceController::IsPSController)
	};

	struct DoodadProperties
	{
		TINT  m_iNumAxes; // 0 for buttons and keys, 1 for the mouse wheel, 2 for sticks and the cursor
		TBOOL m_bFlag;
	};

	struct RepeatInfo
	{
		Doodad iDoodad;
		TFLOAT fLeftTime;
		TFLOAT fRepeatTime;
	};

public:
	TInputDevice();
	~TInputDevice();

	virtual TBOOL        Acquire()                  = 0;                                                      // +0x0C
	virtual TBOOL        Unacquire()                = 0;                                                      // +0x10
	virtual void         Release()                  = 0;                                                      // +0x14
	virtual void         Update( TFLOAT deltaTime ) = 0;                                                      // +0x18
	virtual TBOOL        Flush();                                                                             // +0x1C
	virtual TINT         ProcessEvents( EventEmitter& emitter, TFLOAT deltaTime )                       = 0;  // +0x20
	virtual TINT         GetButtonCount() const                                                         = 0;  // +0x24
	virtual TINT         GetAxisCount() const                                                           = 0;  // +0x28
	virtual TBOOL        GetDoodadProperties( Doodad a_iDoodad, DoodadProperties& a_rProperties ) const = 0;  // +0x2C
	virtual TBOOL        StartRepeat( Doodad a_iDoodad, TFLOAT a_fFirstRepeatTime, TFLOAT a_fRepeatTime );    // +0x30
	virtual void         StopRepeat( Doodad a_iDoodad );                                                      // +0x34
	virtual void         StopAllRepeats();                                                                    // +0x38
	virtual TBOOL        IsForceFeedbackDevice();                                                             // +0x3C
	virtual GAMEPADTYPE  GetGamepadType() const                            = 0;                               // +0x40
	virtual const TCHAR* GetButtonFromDoodad( Doodad a_iDoodad ) const     = 0;                               // +0x44
	virtual TBOOL        IsDown( Doodad a_iDoodad ) const                  = 0;                               // +0x48
	virtual TINT         GetAxisInt( Doodad a_iDoodad, TINT axis ) const   = 0;                               // +0x4C
	virtual TFLOAT       GetAxisFloat( Doodad a_iDoodad, TINT axis ) const = 0;                               // +0x50
	virtual TBOOL        IsEnabled() const                                 = 0;                               // +0x54
	virtual void         ThrowRepeatEvent( EventEmitter& emitter, RepeatInfo* repeatInfo, TFLOAT deltaTime ); // +0x58

	TBOOL IsAcquired() const
	{
		return m_bIsAcquired;
	}

	void SetAcquired( TBOOL a_bAcquired )
	{
		m_bIsAcquired = a_bAcquired;
	}

	TInputInterface* GetInputInterface()
	{
		return m_pInputInterface;
	}

	void SetInputInterface( TInputInterface* a_pInterface );

	TUINT GetDeviceIndex() const { return m_uiDeviceIndex; }

	TBOOL AddDoodad( Doodad a_iDoodad );
	void  RemoveDoodad( Doodad a_iDoodad );
	void  ClearDoodads( TINT a_iUnused = 0 );
	TBOOL HasDoodad( Doodad a_iDoodad ) const;

protected:
	TINT ProcessRepeats( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime );

public:
	static TInputDevice** GetRegisteredDevices()
	{
		return s_aDevices;
	}

	static TINT GetNumRegisteredDevices()
	{
		return s_uiDeviceCount;
	}

	// Whether an enabled device of that type is registered
	static TBOOL IsGamepadTypeConnected( GAMEPADTYPE a_eGamepadType );

	// The type of the device that was used last (the controllers set it when one of their buttons is down)
	static GAMEPADTYPE GetLastGamepadType();
	static void        SetLastGamepadType( GAMEPADTYPE a_eGamepadType );

	// Players and their devices (by the index of the registered device, -1 for none)
	static TInputDevice* GetPlayerDevice( TINT a_iPlayer );
	static TINT          GetPlayerDeviceIndex( TINT a_iPlayer );
	static void          SetPlayerDevice( TINT a_iPlayer, TInputDevice* a_pDevice );
	static void          ResetPlayerDevices();
	static void          SavePlayerDevices();
	static void          RestorePlayerDevices();

	// The player assignments themselves (0x9819b0, MAX_DEVICE_COUNT ints), copied by the pause menu
	static TINT* GetPlayerDeviceIndices() { return ms_aPlayerDevices; }
	static TUINT GetPlayerDeviceIndicesSize() { return sizeof( ms_aPlayerDevices ); }

	// Returns the player the device is assigned to, -1 if none
	// $deBlob: FUNCTION 005c8e80
	TINT GetPlayer() const
	{
		for ( TINT i = 0; i < MAX_DEVICE_COUNT; i++ )
		{
			if ( ms_aPlayerDevices[ i ] == TINT( m_uiDeviceIndex ) )
				return i;
		}

		return -1;
	}

	// Whether no player has a device
	static TBOOL HasNoPlayerDevices()
	{
		return ms_aPlayerDevices[ 0 ] < 0 && ms_aPlayerDevices[ 1 ] < 0 && ms_aPlayerDevices[ 2 ] < 0 && ms_aPlayerDevices[ 3 ] < 0;
	}

	static void  SetPlayerDevicesLocked( TBOOL a_bLocked ) { ms_bPlayerDevicesLocked = a_bLocked; }
	static TBOOL ArePlayerDevicesLocked() { return ms_bPlayerDevicesLocked; }

	// The device of the last button TXUIInput passed to XUI (read by the game's menus)
	static void SetLastXUIDeviceIndex( TINT a_iIndex ) { ms_iLastXUIDeviceIndex = a_iIndex; }
	static TINT GetLastXUIDeviceIndex() { return ms_iLastXUIDeviceIndex; }

protected:
	static TCONSTEXPR_VAR TINT MAX_DEVICE_COUNT = 14;

	static TBOOL         ms_bPlayerDevicesLocked;
	static TINT          ms_iLastXUIDeviceIndex;
	static TInputDevice* s_aDevices[ MAX_DEVICE_COUNT ];
	static TINT          s_uiDeviceCount;
	static TINT          ms_aPlayerDevices[ MAX_DEVICE_COUNT ];
	static TINT          ms_aSavedPlayerDevices[ MAX_DEVICE_COUNT ];
	static GAMEPADTYPE   ms_eLastGamepadType;

protected:
	// NOTE: 0x10 is never written by the engine
	TUINT32            m_uiUnk1;          // 0x10
	TUINT              m_uiDeviceIndex;   // 0x14 index in s_aDevices
	TArray<RepeatInfo> m_Repeats;         // 0x18
	TArray<Doodad>     m_aDoodads;        // 0x28
	TBOOL              m_bUnknown;        // 0x38
	TBOOL              m_bIsAcquired;     // 0x39
	TInputInterface*   m_pInputInterface; // 0x3C
};

TSTATICASSERT( sizeof( TInputInterface ) == 0x30 );
TSTATICASSERT( sizeof( TInputDevice ) == 0x40 );
TSTATICASSERT( sizeof( TInputInterface::InputEvent ) == 0x20 );

TOSHI_NAMESPACE_END

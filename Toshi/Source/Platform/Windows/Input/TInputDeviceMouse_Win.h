#pragma once
#include "Includes.h"
#include "Input/TInputDeviceMouse.h"

TOSHI_NAMESPACE_START

class TInputDXDeviceMouse : public TInputDeviceMouse
{
public:
	TDECLARE_CLASS( TInputDXDeviceMouse, TInputDeviceMouse );

public:
	// $deBlob: FUNCTION 0067f030
	// $Barnyard: FUNCTION 006d3c30
	TInputDXDeviceMouse()
	{
		m_poDXInputDevice = NULL;
	}

	// $deBlob: FUNCTION 0067f050
	// $deBlob: FUNCTION 0067fb80
	// $Barnyard: FUNCTION 006d4530
	virtual ~TInputDXDeviceMouse()
	{
		Release();
	}

	virtual TBOOL Acquire() override;
	virtual TBOOL Unacquire() override;
	virtual void  Release() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;
	virtual TBOOL Flush() override;
	virtual TINT  ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime ) override;
	virtual TINT  GetButtonCount() const override;
	virtual TINT  GetAxisCount() const override;
	// $deBlob: FUNCTION 0067fbd0
	virtual GAMEPADTYPE GetGamepadType() const override { return Gamepad_Keyboard; }
	virtual TBOOL       IsDown( Doodad a_iDoodad ) const override;
	virtual TINT        GetAxisInt( Doodad a_iDoodad, TINT axis ) const override;
	virtual TFLOAT      GetAxisFloat( Doodad a_iDoodad, TINT axis ) const override;
	// $deBlob: FUNCTION 0067fbe0
	virtual TBOOL IsEnabled() const override { return TTRUE; }

	virtual TBOOL Initialise();                       // +0x5C
	virtual TBOOL Deinitialise();                     // +0x60
	virtual void  RefreshDirect();                    // +0x64
	virtual TBOOL WasDown( Doodad a_iDoodad ) const; // +0x68

	TBOOL const BindToDIDevice( HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDXDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL a_bExclusive );

	void SetExclusive( HWND a_hWindow, TBOOL a_bExclusive );
	void SetCurrentPosition( TINT a_iX, TINT a_iY );

	TINT GetPositionX() const { return m_iPositionX; }
	TINT GetPositionY() const { return m_iPositionY; }

	const DIDEVICEINSTANCEA& GetDeviceInstance() const
	{
		return m_oDeviceInstance;
	}

private:
	TBOOL                 m_bReverseButtons;   // 0x40
	DWORD                 m_dwButtonCurrent;   // 0x44
	DWORD                 m_dwButtonPrevious;  // 0x48
	TINT                  m_iPositionX;        // 0x4C
	TINT                  m_iPositionY;        // 0x50
	TINT                  m_iPositionZ;        // 0x54
	TINT                  m_iDeltaPositionX;   // 0x58
	TINT                  m_iDeltaPositionY;   // 0x5C
	TINT                  m_iDeltaPositionZ;   // 0x60
	TINT                  m_iUnk1;             // 0x64
	TINT                  m_iUnk2;             // 0x68
	TINT                  m_iUnk3;             // 0x6C
	TINT                  m_iUnk4;             // 0x70
	TINT                  m_iUnk5;             // 0x74
	TINT                  m_iUnk6;             // 0x78
	TINT                  m_iUnk7;             // 0x7C
	TINT                  m_iWheelGranularity; // 0x80 DIPROP_GRANULARITY of the wheel
	DIDEVICEINSTANCEA     m_oDeviceInstance;   // 0x84
	DIDEVCAPS             m_DIDevCaps;         // 0x2C8
	IDirectInputDevice8A* m_poDXInputDevice;   // 0x2F4
};

TSTATICASSERT( sizeof( TInputDXDeviceMouse ) == 0x2F8 );

TOSHI_NAMESPACE_END

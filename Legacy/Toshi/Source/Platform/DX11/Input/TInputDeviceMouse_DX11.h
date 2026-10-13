#pragma once
#include "Includes.h"
#include "Toshi/Input/TInputDeviceMouse.h"

TOSHI_NAMESPACE_START

class TInputDXDeviceMouse : public TInputDeviceMouse
{
public:
	union Axis
	{
		TINT   m_iX, m_iY;
		TFLOAT m_fX, m_fY;
	};

public:
	// $deBlob: FUNCTION 0067f030
	TInputDXDeviceMouse() { m_poDXInputDevice = NULL; }

	virtual ~TInputDXDeviceMouse() { Deinitialise(); }

	virtual TBOOL    Acquire() override;
	virtual TBOOL    Unacquire() override;
	virtual void     Release() override;
	virtual void     Update(TFLOAT deltaTime) override;
	virtual TBOOL    Flush() override;
	virtual TINT     ProcessEvents(EventEmitter& emitter, TFLOAT deltaTime) override;
	virtual TINT     GetButtonCount() const override;
	virtual TINT     GetAxisCount() const override;
	virtual Platform GetPlatform() const override;
	virtual TBOOL    IsDown(TINT doodad) const override;
	virtual TINT     GetAxisInt(TINT doodad, TINT axis) const override;
	virtual TFLOAT   GetAxisFloat(TINT doodad, TINT axis) const override;
	virtual TBOOL    IsEnabled() const override;
	virtual TBOOL    Initialise();
	virtual TBOOL    Deinitialise();
	virtual void     RefreshDirect();
	virtual TBOOL    WasDown(TINT doodad) const;

	TBOOL const BindToDIDevice(HWND a_mainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL exclusive);

public:
	static constexpr size_t sm_ciMouseBufferSize = 0x20000000U;

private:
	TBOOL                 m_bReverseButtons;
	DWORD                 m_dwButtonCurrent;
	DWORD                 m_dwButtonPrevious;
	POINT                 m_CursorPos;
	Axis                  m_aAxis;
	TINT                  m_field0x80;
	TBOOL                 m_bUnk;            // 0x35
	TFLOAT                m_fWheelAxis;      // 0x60 very unsure
	TBOOL                 m_bInitiliased;    // 0x80
	DIDEVICEINSTANCEA     m_oDeviceInstance; // 0x084
	DIDEVCAPS             m_DIDevCaps;       // 0x2C8
	IDirectInputDevice8A* m_poDXInputDevice; // 0x2F4
};

TOSHI_NAMESPACE_END

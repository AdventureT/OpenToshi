#pragma once
#include "TInputInterface.h"

TOSHI_NAMESPACE_START

class TInputDeviceMouse : public TInputDevice
{
public:
	static const TINT BUTTON_1              = 0x30001;
	static const TINT BUTTON_2              = 0x30002;
	static const TINT BUTTON_3              = 0x30003;
	static const TINT BUTTON_4              = 0x30004;
	static const TINT BUTTON_5              = 0x30005;
	static const TINT BUTTON_6              = 0x30006;
	static const TINT BUTTON_7              = 0x30007;
	static const TINT BUTTON_8              = 0x30008;
	static const TINT BUTTON_WHEEL_FORWARD  = 0x30009;
	static const TINT BUTTON_WHEEL_BACKWARD = 0x3000A;
	static const TINT AXIS_CURSOR           = 0x3000B;
	static const TINT AXIS_WHEEL            = 0x3000C;

public:
	TInputDeviceMouse() {}

	virtual TBOOL        GetDoodadProperties(TINT doodad, TInputDevice::DoodadProperties& props) const override;
	virtual const TCHAR* GetButtonFromDoodad(TINT doodad) const override;
};

TOSHI_NAMESPACE_END

#pragma once
#include "Toshi/Input/TInputInterface.h"

TOSHI_NAMESPACE_START

class TInputDeviceController : public TInputDevice
{
public:
	TInputDeviceController()
	{
		m_things  = (TUINT8*)TMalloc(8);
		m_things2 = (TUINT8*)TMalloc(8);
		m_things  = { 0 };
		m_things2 = { 0 };
	}

private:
	TUINT8* m_things;  // 0x3C
	TUINT8* m_things2; // 0x40
};

TOSHI_NAMESPACE_END

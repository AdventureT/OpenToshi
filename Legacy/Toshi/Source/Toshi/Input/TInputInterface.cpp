#include "ToshiPCH.h"
#include "TInputInterface.h"
#include "TInputDeviceKeyboard.h"
#include "TInputDeviceMouse.h"

TOSHI_NAMESPACE_START

TInputDevice* TInputInterface::GetDeviceByIndex(TClass* pClass, size_t index)
{
	if (m_DeviceList.Count() == 0) return TNULL;
	for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
	{
		if (it->GetClass()->IsA(pClass))
		{
			if (index == 0)
			{
				return it;
			}
			else
			{
				index -= 1;
			}
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 006776c0
void TInputInterface::AddDevice(TInputDevice* device)
{
	if (device != TNULL)
	{
		m_DeviceList.InsertTail(*device);
		device->SetInputInterface(this);
	}
}

// $deBlob: FUNCTION 006776f0
void TInputInterface::RemoveDevice(TInputDevice* device)
{
	if (device != TNULL)
	{
		m_DeviceList.Remove(*device);
		device->SetInputInterface(TNULL);
	}
}

// $deBlob: FUNCTION 00677460
TBOOL TInputInterface::Deinitialise()
{
	m_DeviceList.DeleteAll();
	return TTRUE;
}

// $deBlob: FUNCTION 00677510
TBOOL TInputInterface::AcquireAll()
{
	TBOOL bRet = TTRUE;
	for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
	{
		bRet &= it->Acquire();
	}
	return bRet;
}

// $deBlob: FUNCTION 00677550
TBOOL TInputInterface::UnacquireAll()
{
	TBOOL bRet = TTRUE;
	for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
	{
		bRet &= it->Unacquire();
	}
	return bRet;
}

// $deBlob: FUNCTION 00677590
TBOOL TInputInterface::FlushAll()
{
	TBOOL bRet = TTRUE;
	for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
	{
		bRet &= it->Flush();
	}
	return bRet;
}

// $deBlob: FUNCTION 006775d0
void TInputInterface::SetExclusiveMode(TBOOL mode)
{
	m_bIsExclusiveMode = mode;
}

// $deBlob: FUNCTION 006775e0
TBOOL TInputInterface::GetExclusiveMode() const
{
	return m_bIsExclusiveMode;
}

// $deBlob: FUNCTION 006775f0
TINT TInputInterface::ProcessEvents(TFLOAT deltaTime)
{
	TINT iNumProcessed = 0;

	for (auto it = m_DeviceList.Tail(); it != m_DeviceList.End(); --it)
	{
		if (it->IsAcquired())
		{
			it->Update(deltaTime);
		}
	}

	for (auto it = m_DeviceList.Tail(); it != m_DeviceList.End(); --it)
	{
		if (it->IsAcquired())
		{
			iNumProcessed += it->ProcessEvents(m_Emitter1, deltaTime);
		}
	}

	return iNumProcessed;
}

// $deBlob: FUNCTION 00677690
void TInputInterface::StopAllRepeats()
{
	for (auto it = m_DeviceList.Begin(); it != m_DeviceList.End(); ++it)
	{
		it->StopAllRepeats();
	}
}

TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType)
{
	m_pSource           = a_pDevice;
	m_iDoodad           = a_iDoodad;
	m_eEventType        = a_eEventType;
	m_bIsMagnitudeFloat = TFALSE;
	m_iAxisCount        = 0;
	m_wszString[0]      = L'\0';
}

TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TWCHAR* a_wszString)
{
	m_pSource           = a_pDevice;
	m_iDoodad           = a_iDoodad;
	m_eEventType        = a_eEventType;
	m_bIsMagnitudeFloat = TFALSE;
	m_iAxisCount        = 0;
	TStringManager::String16Copy(m_wszString, a_wszString, 3);
	m_Magnitude.Ints[0] = 0;
}

TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1)
{
	m_pSource           = a_pDevice;
	m_iDoodad           = a_iDoodad;
	m_eEventType        = a_eEventType;
	m_Magnitude.Ints[0] = a_iMagnitude1;
	m_bIsMagnitudeFloat = TFALSE;
	m_iAxisCount        = 1;
	m_wszString[0]      = L'\0';
}

TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1, TINT a_iMagnitude2)
{
	m_pSource           = a_pDevice;
	m_iDoodad           = a_iDoodad;
	m_eEventType        = a_eEventType;
	m_Magnitude.Ints[0] = a_iMagnitude1;
	m_Magnitude.Ints[1] = a_iMagnitude2;
	m_bIsMagnitudeFloat = TFALSE;
	m_iAxisCount        = 2;
	m_wszString[0]      = L'\0';
}

TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1)
{
	m_pSource             = a_pDevice;
	m_iDoodad             = a_iDoodad;
	m_eEventType          = a_eEventType;
	m_Magnitude.Floats[0] = a_fMagnitude1;
	m_bIsMagnitudeFloat   = TTRUE;
	m_iAxisCount          = 1;
	m_wszString[0]        = L'\0';
}

TInputInterface::InputEvent::InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1, TFLOAT a_fMagnitude2)
{
	m_pSource             = a_pDevice;
	m_iDoodad             = a_iDoodad;
	m_eEventType          = a_eEventType;
	m_Magnitude.Floats[0] = a_fMagnitude1;
	m_Magnitude.Floats[1] = a_fMagnitude2;
	m_bIsMagnitudeFloat   = TTRUE;
	m_iAxisCount          = 2;
	m_wszString[0]        = L'\0';
}

// $deBlob: FUNCTION 006778d0
TINT TInputInterface::InputEvent::GetMagnitudeInt(TINT a_iAxis)
{
	TASSERT(a_iAxis >= 0 && a_iAxis < GetAxisCount());

	if (IsMagnitudeInt())
	{
		return m_Magnitude.Ints[a_iAxis];
	}
	else
	{
		if (m_Magnitude.Floats[a_iAxis] < -0.5f)
		{
			return -1;
		}

		return 0.5f < m_Magnitude.Floats[a_iAxis];
	}
}

// $deBlob: FUNCTION 00677950
TFLOAT TInputInterface::InputEvent::GetMagnitudeFloat(TINT a_iAxis)
{
	TASSERT(a_iAxis >= 0 && a_iAxis < GetAxisCount());

	if (IsMagnitudeFloat())
	{
		return m_Magnitude.Floats[a_iAxis];
	}

	return (TFLOAT)m_Magnitude.Ints[a_iAxis];
}

TOSHI_NAMESPACE_END

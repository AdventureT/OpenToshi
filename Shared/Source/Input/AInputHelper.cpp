#include "pch.h"
#include "AInputHelper.h"
#include <Toshi/Input/TInputDeviceKeyboard.h>
#include "AInputManager2.h"

TOSHI_NAMESPACE_USING

void AInputHelper::AddMapping(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eInputDevice, TBOOL bVal, float a_fRepeatTime)
{
	Toshi::T2Vector<ButtonInfo, 4> buttonInfoVector;

	if (bVal)
	{
		TASSERT(0.01666f <= a_fRepeatTime);
	}

	AInputManager2* inputMng = AInputManager2::GetSingleton();

	if (m_eInputContext == AInputMap::INPUTCONTEXT_UNK12)
	{
		m_eInputContext = inputMng->GetContext();
	}

	auto pButtonMap = inputMng->GetInputMap().GetButtonMap(inputMng->GetContext());
	auto foundMap1 = pButtonMap->Find(TSTATICCAST(AInputMap::ActionButton, a_eInputButton));
	auto endOfMap = &pButtonMap->End()->GetSecond();

	if (foundMap1 != endOfMap)
	{
		for (auto j = foundMap1->Begin(); j != foundMap1->End(); j++)
		{
			buttonInfoVector.PushBack({ *j, bVal ? 6 : 2, 0.0f, a_fRepeatTime });
		}
	}

	uint32_t buttonDevice = MakeButtonDevice(a_eInputButton, a_eInputDevice);

	m_oButtonMap.Insert(buttonDevice, buttonInfoVector);
}

void AInputHelper::AddMapping(AInputMap::INPUTBUTTON a_eInputButton, TBOOL bVal, float a_fRepeatTime)
{
	for (size_t i = 0; i < AInputManager2::INPUTDEVICE_Count; i++)
	{
		AddMapping(a_eInputButton, i, bVal, a_fRepeatTime);
	}

	AddMapping(a_eInputButton, AInputManager2::INPUTDEVICE_Keyboard, bVal, a_fRepeatTime);
}

TBOOL AInputHelper::IsJustDown(AInputMap::INPUTBUTTON a_eInputButton)
{
	for (size_t i = 0; i < AInputManager2::INPUTDEVICE_Count; i++)
	{
		if (IsJustDown(a_eInputButton, i))
		{
			return TTRUE;
		}
	}

	return IsJustDown(a_eInputButton, AInputManager2::INPUTDEVICE_Keyboard);
}

TBOOL AInputHelper::IsJustDown(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eInputDevice)
{
	auto inputMng = AInputManager2::GetSingleton();

	if (a_eInputDevice != AInputManager2::INPUTDEVICE_Keyboard &&
		inputMng->CheckIfValidDevice(
			inputMng->GetControllerHandle(a_eInputDevice)
		))
	{
		return TFALSE;
	}

	auto buttonDevice = m_oButtonMap.Find(MakeButtonDevice(a_eInputButton, a_eInputDevice));
	auto buttonMapEnd = &m_oButtonMap.End()->GetSecond();

	if (buttonDevice == buttonMapEnd)
	{
		return TFALSE;
	}

	auto t = *buttonDevice;

	for (size_t i = buttonDevice->Size(); i != 0; i--)
	{
		auto value = t[i];
		if (HASFLAG(value.m_iFlag & FLAG_ISDOWN))
		{
			return TTRUE;
		}
	}

	return TFALSE;

}

void AInputHelper::Update(float fVal)
{
	AInputMap::INPUTBUTTON eInputButton;
	AInputManager2::INPUTDEVICE eInputDevice;

	for (auto i = m_oButtonMap.Begin(); i != m_oButtonMap.End(); i++)
	{
		auto key = i->GetFirst();
		auto value = i->GetSecond();
		GetInputButtonDevice(key, eInputButton, eInputDevice);
		for (size_t i = 0; i < value.Size(); i++)
		{
			UpdateButtonInfo(&value[i], eInputDevice);
		}
	}
}

void AInputHelper::UpdateButtonInfo(Toshi::TInputDevice* a_pDevice, ButtonInfo* a_pButtonInfo)
{
	bool isDown = a_pDevice->IsDown(a_pButtonInfo->m_iDoodad);
	if (m_eInputContext != AInputManager2::GetSingleton()->GetContext())
	{
		isDown = false;
	}

	if (!HASFLAG(a_pButtonInfo->m_iFlag & FLAG_UNK2) && isDown)
	{
		a_pButtonInfo->m_iFlag &= FLAG_ISDOWN;
		a_pButtonInfo->m_iFlag &= FLAG_UNK2;
	}
	else
	{
		isDown ? a_pButtonInfo->m_iFlag &= FLAG_UNK2 : a_pButtonInfo->m_iFlag &= ~FLAG_ISDOWN;
	}

	if (!HASFLAG(a_pButtonInfo->m_iFlag & FLAG_UNK4) || !isDown)
	{
		a_pButtonInfo->m_iCurRepeatTime = 0.0f;
	}
	else
	{
		while (a_pButtonInfo->m_iRepeatTime <= a_pButtonInfo->m_iCurRepeatTime)
		{
			a_pButtonInfo->m_iFlag &= FLAG_ISDOWN;
			a_pButtonInfo->m_iCurRepeatTime -= a_pButtonInfo->m_iRepeatTime;
		}
	}
}

void AInputHelper::UpdateButtonInfo(ButtonInfo* a_pButtonInfo, AInputManager2::INPUTDEVICE a_eInputDevice)
{
	TInputDevice* device = a_eInputDevice == AInputManager2::INPUTDEVICE_Keyboard ? 
		TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>() : 
		AInputManager2::GetSingleton()->GetControllerHandle(a_eInputDevice).m_pDevice;

	if (device)
	{
		UpdateButtonInfo(device, a_pButtonInfo);
	}

}

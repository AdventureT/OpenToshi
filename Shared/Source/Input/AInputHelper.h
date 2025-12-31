#pragma once
#include <Input/AInputManager2.h>

class AInputHelper
{
public:
	// $deBlob: FUNCTION 005d9cc0
	AInputHelper()
	{
		m_eInputContext = AInputMap::INPUTCONTEXT_UNK12;
	}

	enum FLAG
	{
		FLAG_UNK,
		FLAG_ISDOWN,
		FLAG_UNK2,
		FLAG_UNK3,
		FLAG_UNK4,
	};

	struct ButtonInfo
	{
		int   m_iDoodad;        // 0x0
		int   m_iFlag;          // 0x4
		float m_iRepeatTime;    // 0x8
		float m_iCurRepeatTime; // 0xC
	};

	void AddMapping(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eInputDevice, TBOOL bVal, float a_fRepeatTime);
	void AddMapping(AInputMap::INPUTBUTTON a_eInputButton, TBOOL bVal, float a_fRepeatTime);

	TBOOL IsJustDown(AInputMap::INPUTBUTTON a_eInputButton);
	TBOOL IsJustDown(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eInputDevice);

	void Update(float fVal = 0.0f);

	// $deBlob: FUNCTION 005d9d70
	static uint32_t MakeButtonDevice(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eDevice)
	{
		TASSERT(0 == (0xFFFF0000 & a_eInputButton));
		TASSERT(0 == (0xFFFF0000 & a_eDevice));
		return (a_eInputButton << 16 | a_eDevice);
	}

	// $deBlob: FUNCTION 005d9df0
	static void GetInputButtonDevice(uint32_t a_iButtonDevice, AInputMap::INPUTBUTTON& a_eInputButton, AInputManager2::INPUTDEVICE& a_eInputDevice)
	{
		a_eInputButton = a_iButtonDevice >> 16;
		a_eInputDevice = a_iButtonDevice & 0xFFFF;
	}

	void UpdateButtonInfo(Toshi::TInputDevice* a_pDevice, ButtonInfo* a_pButtonInfo);
	void UpdateButtonInfo(ButtonInfo* a_pButtonInfo, AInputManager2::INPUTDEVICE a_eInputDevice);

	typedef Toshi::TComparator<int> ButtonMapComparator;

	AInputMap::INPUTCONTEXT                                                     m_eInputContext; // 0x4
	Toshi::T2Map<uint32_t, Toshi::T2Vector<ButtonInfo, 4>, ButtonMapComparator> m_oButtonMap;    // 0x8
};

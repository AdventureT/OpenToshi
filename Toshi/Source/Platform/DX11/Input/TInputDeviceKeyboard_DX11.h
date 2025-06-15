#pragma once
#include "Toshi/Input/TInputDeviceKeyboard.h"
#include "Toshi2/T2Array.h"

#include "Includes.h"

TOSHI_NAMESPACE_START

class TInputDXDeviceKeyboard : public TInputDeviceKeyboard
{
public:
	struct KeyInfo
	{
		TWCHAR* wszKeyName;
		DWORD   dwKeyScanCode;
	};

public:
	TInputDXDeviceKeyboard();

	virtual TBOOL   Acquire() override;
	virtual TBOOL   Unacquire() override;
	virtual void    Release() override;
	virtual void    Update(TFLOAT deltaTime) override;
	virtual TBOOL   Flush() override;
	virtual TINT    ProcessEvents(EventEmitter& emitter, TFLOAT deltaTime) override;
	virtual TINT    GetButtonCount() const override;
	virtual TBOOL   IsDown(TINT doodad) const override;
	virtual TBOOL   IsEnabled() const override;
	virtual TBOOL   IsShiftDown() const override;
	virtual TBOOL   IsControlDown() const override;
	virtual TBOOL   IsAltDown() const override;
	virtual TBOOL   WasDown(TINT doodad) const override;
	virtual TWCHAR* TranslateDoodadToCharacter(TINT doodad) const override;
	virtual TBOOL   Initialise();
	virtual TBOOL   Deinitialise();
	virtual void    RefreshDirect();

protected:
	TBOOL       HandleKeyChange(TEmitter<TInputInterface, TInputInterface::InputEvent>& a_Emitter, TINT a_iKeyIndex, TUINT8 a_ui8KeyState);
	static TINT TranslateDXToDoodad(TINT doodad);
	static TINT TranslateDoodadToDX(TINT dxkey);

public:
	const TBOOL          BindToDIDevice(HWND a_hMainWindow, LPCDIDEVICEINSTANCEA a_poDeviceInstance, IDirectInputDevice8A* a_poDXInputDevice, TBOOL a_bExclusive);
	static BOOL CALLBACK EnumObjectCallback(LPCDIDEVICEOBJECTINSTANCEA a_poObjectInstance, LPVOID a_pvRef);

private:
	TINT                                   m_iSomeNum;        // 0x070
	TUINT8*                                m_pKeyStates1;     // 0x074
	TUINT8*                                m_pKeyStates2;     // 0x078
	TUINT8                                 m_aBuffer[512];    // 0x07C
	T2Array<KeyInfo, KEYBOARD_NUM_DOODADS> m_aKeys;           // 0x27C
	TBOOL                                  m_bIsUpdating;     // 0x67C
	DIDEVICEINSTANCEA                      m_oDeviceInstance; // 0x680
	DIDEVCAPS                              m_DIDevCaps;       // 0x8C4
	IDirectInputDevice8A*                  m_poDXInputDevice; // 0x8F0
	TBOOL                                  m_bFlag;           // 0x8F4
};

TOSHI_NAMESPACE_END

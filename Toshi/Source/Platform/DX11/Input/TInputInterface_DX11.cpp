#include "ToshiPCH.h"
#include "TInputInterface_DX11.h"
#include "TInputDeviceController_DX11.h"
#include "TInputDeviceMouse_DX11.h"
#include "TInputDeviceController_XInput.h"
#include "TInputDeviceController_Wiin.h"
#include "TInputDeviceKeyboard_DX11.h"

TOSHI_NAMESPACE_START

TBOOL TInputDXInterface::Initialise()
{
	TASSERT(TNULL == m_poDirectInput8);
	HRESULT hRes = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&m_poDirectInput8, NULL);

	if (SUCCEEDED(hRes))
	{
		if (m_hMainWindow == NULL)
		{
			m_hMainWindow = FindWindowA("TRenderD3D", NULL);
		}
		hRes = m_poDirectInput8->EnumDevices(DI8DEVCLASS_ALL, TInputDXInterface::EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY);
		if (hRes == DI_OK)
		{
			for (size_t i = 0; i < 4; i++)
			{
				//AddDevice(new TInputDeviceController_XInput());
				//AddDevice(new TInputWiinDeviceController());
			}

			UINT numJoyDevs = joyGetNumDevs();

			for (size_t i = 0; i < numJoyDevs; i++)
			{
				JOYCAPSA joyCaps;
				MMRESULT res = joyGetDevCapsA(i, &joyCaps, sizeof(joyCaps));
				if (res == 0)
				{
					TUtil::Log("Joystick[%d] \'%s\' \'%s\'", i, joyCaps.szPname, joyCaps.szRegKey);
				}
			}

			return TTRUE;
		}
		Deinitialise();
	}

	return TFALSE;
}

TBOOL TInputDXInterface::Deinitialise()
{
	TInputInterface::Deinitialise();

	if (m_poDirectInput8 != NULL)
	{
		m_poDirectInput8->Release();
		m_poDirectInput8 = NULL;
	}

	return TTRUE;
}

TBOOL TInputDXInterface::LostDevice()
{
	auto input = GetDirectInput();
	if (input != NULL)
	{
		return input->EnumDevices(DI8DEVCLASS_ALL, EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY) != DI_OK;
	}
	return TFALSE;
}

BOOL TInputDXInterface::EnumerateDeviceCallback(LPCDIDEVICEINSTANCEA a_poDeviceInstance, LPVOID poDXInputInterface)
{
	TInputDXInterface*        pInputInterface = (TInputDXInterface*)poDXInputInterface;
	LPDIRECTINPUTDEVICE8A     inputDevice;
	TInputDXDeviceMouse*      inputMouse;
	TInputDXDeviceController* inputController;
	TBOOL                     addMouse    = TFALSE;
	TBOOL                     addKeyboard = TFALSE;

	TASSERT(poDXInputInterface != NULL);
	TASSERT(a_poDeviceInstance != NULL);

	// Check if any slots are left
	if (ms_iNumDevices >= MAXDEVICESNUM) return FALSE;

	// Check if this device is not registered yet
	for (size_t i = 0; i < ms_iNumDevices; i++)
	{
		if (ms_RegisteredDevices[i] == a_poDeviceInstance->guidInstance)
		{
			return TRUE;
		}
	}

	// Add the device to the list of registered
	size_t iDeviceIndex                = ms_iNumDevices++;
	ms_RegisteredDevices[iDeviceIndex] = a_poDeviceInstance->guidInstance;

	switch (GET_DIDEVICE_TYPE(a_poDeviceInstance->dwDevType))
	{
		case DI8DEVTYPE_MOUSE:
		{
			inputMouse = pInputInterface->GetDeviceByIndex<TInputDXDeviceMouse>();

			HRESULT hRes = pInputInterface->m_poDirectInput8->CreateDevice(GUID_SysMouse, &inputDevice, NULL);

			if (hRes != DI_OK) return DIENUM_CONTINUE;

			if (inputMouse == TNULL)
			{
				inputMouse = new TInputDXDeviceMouse();
				addMouse   = TTRUE;
				TBOOL bRes = inputMouse->BindToDIDevice(pInputInterface->GetMainWindow(), a_poDeviceInstance, inputDevice, pInputInterface->m_bExclusive);

				if (bRes)
				{
					DIPROPDWORD oProperty;
					oProperty.diph.dwSize       = sizeof(DIPROPDWORD);
					oProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
					oProperty.diph.dwObj        = 0;
					oProperty.diph.dwHow        = DIPH_DEVICE;
					oProperty.dwData            = TInputDXDeviceMouse::sm_ciMouseBufferSize;

					HRESULT hr = inputDevice->SetProperty(DIPROP_BUFFERSIZE, &oProperty.diph);

					if (FAILED(hr)) return DIENUM_STOP;

					if (addMouse)
					{
						inputMouse->Initialise();
						inputMouse->Acquire();
						pInputInterface->AddDevice(inputMouse);
					}
				}
				if (!addMouse)
				{
					pInputInterface->RemoveDevice(inputMouse);
				}
			}
			TIMPLEMENT();
			break;
		}
		case DI8DEVTYPE_KEYBOARD:
		{
			HRESULT hRes = pInputInterface->m_poDirectInput8->CreateDevice(a_poDeviceInstance->guidInstance, &inputDevice, NULL);

			if (hRes != DI_OK) return DIENUM_CONTINUE;

			TInputDXDeviceKeyboard* pKeyboard = new TInputDXDeviceKeyboard();
			addKeyboard                       = TTRUE;

			TBOOL bRes = pKeyboard->BindToDIDevice(pInputInterface->GetMainWindow(), a_poDeviceInstance, inputDevice, pInputInterface->m_bExclusive);

			if (bRes)
			{
				DIPROPDWORD oProperty;
				oProperty.diph.dwSize       = sizeof(DIPROPDWORD);
				oProperty.diph.dwHeaderSize = sizeof(DIPROPHEADER);
				oProperty.diph.dwObj        = 0;
				oProperty.diph.dwHow        = DIPH_DEVICE;

				HRESULT hRes = inputDevice->SetProperty(DIPROP_BUFFERSIZE, &oProperty.diph);

				if (FAILED(hRes)) return DIENUM_STOP;

				hRes = inputDevice->SetCooperativeLevel(pInputInterface->GetMainWindow(), pInputInterface->GetExclusiveMode() ? (DISCL_NOWINKEY | DISCL_FOREGROUND | DISCL_EXCLUSIVE) : (DISCL_NONEXCLUSIVE | DISCL_BACKGROUND));

				if (hRes == S_OK)
				{
					pKeyboard->Initialise();
					pKeyboard->Acquire();
					pInputInterface->AddDevice(pKeyboard);
					return DIENUM_CONTINUE;
				}
				else
				{
					delete pKeyboard;
					return DIENUM_CONTINUE;
				}
			}
			break;
		}
		case DI8DEVTYPE_JOYSTICK:
		case DI8DEVTYPE_GAMEPAD:
		case DI8DEVTYPE_1STPERSON:
		{
			TIMPLEMENT();

			TCHAR fmtStr[37];
			Toshi::T2String8::Format(
			    fmtStr, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", a_poDeviceInstance->guidProduct.Data1, a_poDeviceInstance->guidProduct.Data2, a_poDeviceInstance->guidProduct.Data3, a_poDeviceInstance->guidProduct.Data4[0],
			    a_poDeviceInstance->guidProduct.Data4[1], a_poDeviceInstance->guidProduct.Data4[2], a_poDeviceInstance->guidProduct.Data4[3], a_poDeviceInstance->guidProduct.Data4[4], a_poDeviceInstance->guidProduct.Data4[5],
			    a_poDeviceInstance->guidProduct.Data4[6], a_poDeviceInstance->guidProduct.Data4[7]
			);

			if (!TInputDXDeviceController::IsDirectInputController(a_poDeviceInstance))
			{
				TUtil::Log("Added XInput Controller: \'%s\' (%s) - NON-PSX", a_poDeviceInstance->tszProductName, fmtStr);
				return DIENUM_CONTINUE;
			}

			TUtil::Log("Added Direct Input Controller: \'%s\' (%s) - NON-PSX", a_poDeviceInstance->tszProductName, fmtStr);

			inputController = pInputInterface->GetDeviceByIndex<TInputDXDeviceController>();
			HRESULT hRes    = pInputInterface->m_poDirectInput8->CreateDevice(a_poDeviceInstance->guidInstance, &inputDevice, NULL);

			if (hRes != DI_OK) return DIENUM_CONTINUE;

			if (inputController == TNULL)
			{
				// new DXController
				//inputController = new TInputDXDeviceController();
				//inputController->BindToDIDevice(pInputInterface->GetMainWindow(), a_poDeviceInstance, inputDevice);
			}

			break;
		}
		default: break;
	}

	return DIENUM_CONTINUE;
}

TOSHI_NAMESPACE_END

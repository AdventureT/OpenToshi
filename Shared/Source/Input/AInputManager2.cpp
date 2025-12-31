#include "pch.h"
#include "AInputManager2.h"
#include "Locale/ALocaleManager.h"

#include <Toshi/Input/TInputDeviceController.h>

#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/Input/TInputInterface_DX11.h"
#endif // TOSHI_RENDERER_DX11

using namespace Toshi;

// $deBlob: FUNCTION 005daef0
AInputManager2::AInputManager2()
    : m_aControllers()
{
	TUtil::MemClear(m_aControllers, sizeof(m_aControllers));

	auto  pInputInterface = new TInputDXInterface();
	TBOOL result          = pInputInterface->Initialise();

	TASSERT(result == TTRUE);

	for (size_t i = 0; i < INPUTDEVICE_Invalid; i++)
	{
		pInputInterface->AcquireAll();
		pInputInterface->ProcessEvents(0.1f);
		UpdateControllers();

		if (!HasDeviceOfPlatform(Platform::Unk2) &&
		    !HasDeviceOfPlatform(Platform::Unk3) &&
		    !HasDeviceOfPlatform(Platform::Wii))
		{
			ALocaleManager::SetPlatform(Platform::PC);
			TTODO("Do something with platform settings");
		}

		Sleep(200);
	}

	pInputInterface->AcquireAll();
	pInputInterface->ProcessEvents(0.1f);
	UpdateControllers();

	if (HasDeviceOfPlatform(Platform::Unk2)) return;
	if (HasDeviceOfPlatform(Platform::Unk3)) return;
	if (HasDeviceOfPlatform(Platform::Wii)) return;

	ALocaleManager::SetPlatform(Platform::PC);
	TTODO("Do something with platform settings again...");
}

AInputManager2::~AInputManager2()
{
	TIMPLEMENT();
}

// $deBlob: FUNCTION 005db260
void AInputManager2::Update(float a_fDeltaTime)
{
	auto pInputInterface = TInputInterface::GetSingleton();

	pInputInterface->AcquireAll();
	pInputInterface->ProcessEvents(a_fDeltaTime);
	UpdateControllers();

	if (!HasDeviceOfPlatform(Platform::Unk2) &&
	    !HasDeviceOfPlatform(Platform::Unk3) &&
	    !HasDeviceOfPlatform(Platform::Wii))
	{
		ALocaleManager::SetPlatform(Platform::PC);
		TTODO("Do something with platform settings");
	}
}

// $deBlob: FUNCTION 005db480
void AInputManager2::AddInGameController(const AInputDeviceHandle& a_Handle)
{
	TASSERT(a_Handle.IsValid());

	/*if (m_pInputDeviceHandles.Find(a_Handle) != m_pInputDeviceHandles.End())
	{
		m_pInputDeviceHandles.PushBack(a_Handle);
	}*/
}

// $deBlob: FUNCTION 005db540
void AInputManager2::RemoveInGameController(const AInputDeviceHandle& a_Handle)
{
	TASSERT(a_Handle.IsValid());

	//if (m_pInputDeviceHandles.Find(a_Handle) != m_pInputDeviceHandles.End())
	//{
	//	//m_pInputDeviceHandles.Erase(a_Handle);
	//}
}

// $deBlob: FUNCTION 005dc060
AInputManager2::AInputDeviceHandle AInputManager2::GetControllerHandle(INPUTDEVICE a_eDevice)
{
	TASSERT(0 <= a_eDevice);
	TASSERT(INPUTDEVICE_Count > a_eDevice);
	TASSERT(INPUTDEVICE_Invalid != a_eDevice);
	return m_aControllers[a_eDevice];
}

// $deBlob: FUNCTION 005dcba0
void AInputManager2::UpdateControllers()
{
	size_t iDeviceId          = 0;
	size_t iDeviceSearchIndex = 0;

	auto pInputInterface = TInputInterface::GetSingleton();

	for (size_t i = 0; i < INPUTDEVICE_Count; i++)
	{
		m_aControllers[i].m_pDevice = TNULL;
		m_aControllers[i].m_iId     = 0;

		auto pController = pInputInterface->GetDeviceByIndex<TInputDeviceController>(iDeviceSearchIndex);

		while (pController)
		{
			if (pController->IsEnabled())
			{
				m_aControllers[i].m_iId     = iDeviceId++;
				m_aControllers[i].m_pDevice = pController;
				break;
			}

			iDeviceSearchIndex++;
			pController = pInputInterface->GetDeviceByIndex<TInputDeviceController>(iDeviceSearchIndex);
		}

		iDeviceSearchIndex++;
	}
}

// $deBlob: FUNCTION 00679ae0
TBOOL AInputManager2::HasDeviceOfPlatform(Platform a_ePlatform) const
{
	auto pRegisteredDevices = TInputDevice::GetRegisteredDevices();

	for (size_t i = 0; i < TInputDevice::GetNumRegisteredDevices(); i++)
	{
		auto pDevice = pRegisteredDevices[i];

		if (pDevice->GetPlatform() == a_ePlatform && pDevice->IsEnabled())
		{
			return TTRUE;
		}
	}

	return TFALSE;
}

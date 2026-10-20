#include "ToshiPCH.h"
#include "TInputInterface_Win.h"
#include "TInputDeviceController_Win.h"
#include "TInputDeviceController_XInput.h"
#include "TInputDeviceController_Wiin.h"
#include "TInputDeviceMouse_Win.h"
#include "TInputDeviceKeyboard_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TInputDXInterface* TInputDXInterface::ms_pTheInterface = TNULL;
GUID               TInputDXInterface::ms_aRegisteredDevices[ MAXDEVICESNUM ];
TINT               TInputDXInterface::ms_iNumRegisteredDevices;

// $deBlob: FUNCTION 0067fbf0
// $deBlob: FUNCTION 0067fc00
// $deBlob: FUNCTION 0067fc40
// $deBlob: FUNCTION 0067fc80
// $deBlob: FUNCTION 0067fc90
// $deBlob: FUNCTION 0053eac0
// $deBlob: FUNCTION 00858e90
TDEFINE_CLASS( TInputDXInterface );

// $deBlob: FUNCTION 0067fca0
TInputDXInterface::KPADThread::KPADThread( TINT a_iSleepTime )
{
	m_iSleepTime = TMath::Max( a_iSleepTime, 16 );
	m_bUnknown   = TFALSE;
	m_oMutex.Create();
	Create( 0, THREAD_PRIORITY_HIGHEST, FLAGS_NONE );
}

// $deBlob: FUNCTION 0067fd00
TInputDXInterface::KPADThread::~KPADThread()
{
	m_oMutex.Destroy();
}

// $deBlob: FUNCTION 0067fd20
void TInputDXInterface::KPADThread::Main()
{
	while ( TTRUE )
	{
		m_oMutex.Lock();
		m_oTimer.Update();
		m_oMutex.Unlock();
		Sleep( m_iSleepTime );
	}
}

// $deBlob: FUNCTION 0067fd50
TInputDXInterface::TInputDXInterface()
{
	m_poDirectInput8 = NULL;
	m_hMainWindow    = NULL;
	m_bExclusive     = TFALSE;
	ms_pTheInterface = this;
}

// $deBlob: FUNCTION 0067fd80
// $deBlob: FUNCTION 006804b0
// $Barnyard: FUNCTION 006d7430
TInputDXInterface::~TInputDXInterface()
{
	ms_pTheInterface = TNULL;
	ReleaseAllDevices();

	if ( m_poDirectInput8 )
	{
		m_poDirectInput8->Release();
		m_poDirectInput8 = TNULL;
	}
}

// $deBlob: FUNCTION 0067fdc0
// $Barnyard: FUNCTION 006d7770
TBOOL TInputDXInterface::Initialise()
{
	TASSERT( NULL == m_poDirectInput8 );
	HRESULT hRes = DirectInput8Create( GetModuleHandleA( NULL ), DIRECTINPUT_VERSION, IID_IDirectInput8A, (LPVOID*)&m_poDirectInput8, NULL );

	if ( FAILED( hRes ) )
		return TFALSE;

	if ( m_hMainWindow == NULL )
	{
		// NOTE: TMSWindow registers its window class with the name of TRenderDX11's TClass, the boot sets the
		// window with SetMainWindow instead
		m_hMainWindow = FindWindowA( "TRenderD3D", NULL );
	}

	if ( m_poDirectInput8->EnumDevices( DI8DEVCLASS_ALL, EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY ) != DI_OK )
	{
		Deinitialise();
		return TFALSE;
	}

	for ( TINT i = 0; i < 4; i++ )
	{
		AddDevice( new TInputDeviceController_XInput( i ) );

		TInputWiinDeviceController* pWiimote = new TInputWiinDeviceController();

		if ( pWiimote != TNULL )
			pWiimote->Initialise( i + 1 );

		AddDevice( pWiimote );
	}

	TINT iNumJoysticks = joyGetNumDevs();

	for ( TINT i = 0; i < iNumJoysticks; i++ )
	{
		JOYCAPSA oCaps;

		if ( joyGetDevCapsA( i, &oCaps, sizeof( oCaps ) ) == JOYERR_NOERROR )
			TUtil::Log( "Joystick[%d] '%s' '%s'", i, oCaps.szPname, oCaps.szRegKey );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0067ff40
// $Barnyard: FUNCTION 006d7390
TBOOL TInputDXInterface::Deinitialise()
{
	TInputInterface::ReleaseAllDevices();

	if ( m_poDirectInput8 != NULL )
	{
		m_poDirectInput8->Release();
		m_poDirectInput8 = NULL;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0067ff70
// $Barnyard: FUNCTION 006d77e0
void TInputDXInterface::SetExclusiveMode( TBOOL a_bIsExclusive )
{
	m_bExclusive = a_bIsExclusive;

	// NOTE: no check for a missing mouse
	auto pMouse = GetDeviceByIndex<TInputDXDeviceMouse>( 0 );
	pMouse->Unacquire();
	pMouse->SetExclusive( m_hMainWindow, m_bExclusive );
	pMouse->Acquire();

	TInputInterface::SetExclusiveMode( a_bIsExclusive );
}

// $deBlob: FUNCTION 0067ffc0
void TInputDXInterface::SetExclusive( TBOOL a_bIsExclusive )
{
	m_bExclusive = a_bIsExclusive;

	auto pMouse = GetDeviceByIndex<TInputDXDeviceMouse>( 0 );
	pMouse->Unacquire();
	pMouse->SetExclusive( m_hMainWindow, m_bExclusive );
	pMouse->Acquire();
}

// $deBlob: FUNCTION 00680030
TBOOL TInputDXInterface::LostDevice()
{
	auto input = GetDirectInput();

	if ( input != NULL )
	{
		return input->EnumDevices( DI8DEVCLASS_ALL, EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY ) != DI_OK;
	}

	return TFALSE;
}

// Fills a_szBuffer (37 characters) with the GUID like "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX"
// NOTE: inlined in EnumerateDeviceCallback and TInputDXDeviceController::IsPSController
static void FormatGUID( TCHAR* a_szBuffer, const GUID& a_rGUID )
{
	TUtil::MemSet( a_szBuffer, 0, 37 );
	sprintf(
	    a_szBuffer,
	    "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
	    a_rGUID.Data1,
	    a_rGUID.Data2,
	    a_rGUID.Data3,
	    a_rGUID.Data4[ 0 ],
	    a_rGUID.Data4[ 1 ],
	    a_rGUID.Data4[ 2 ],
	    a_rGUID.Data4[ 3 ],
	    a_rGUID.Data4[ 4 ],
	    a_rGUID.Data4[ 5 ],
	    a_rGUID.Data4[ 6 ],
	    a_rGUID.Data4[ 7 ]
	);
}

// $deBlob: FUNCTION 00680070
// $Barnyard: FUNCTION 006d7470
BOOL TInputDXInterface::EnumerateDeviceCallback( LPCDIDEVICEINSTANCEA a_poDeviceInstance, LPVOID poDXInputInterface )
{
	TInputDXInterface* pInputInterface = TSTATICCAST( TInputDXInterface, poDXInputInterface );

	TASSERT( poDXInputInterface != NULL );
	TASSERT( a_poDeviceInstance != NULL );

	if ( ms_iNumRegisteredDevices >= MAXDEVICESNUM )
		return DIENUM_STOP;

	// Every device instance is handled once (LostDevice enumerates the devices again)
	for ( TINT i = 0; i < ms_iNumRegisteredDevices; i++ )
	{
		if ( ms_aRegisteredDevices[ i ] == a_poDeviceInstance->guidInstance )
			return DIENUM_CONTINUE;
	}

	ms_aRegisteredDevices[ ms_iNumRegisteredDevices++ ] = a_poDeviceInstance->guidInstance;

	auto pInput8 = pInputInterface->m_poDirectInput8;

	switch ( GET_DIDEVICE_TYPE( a_poDeviceInstance->dwDevType ) )
	{
		case DI8DEVTYPE_MOUSE:
		{
			LPDIRECTINPUTDEVICE8A pInputDevice;
			auto                  pMouse      = pInputInterface->GetDeviceByIndex<TInputDXDeviceMouse>( 0 );
			TBOOL                 bIsNewMouse = TFALSE;

			if ( pInput8->CreateDevice( GUID_SysMouse, &pInputDevice, NULL ) != DI_OK )
				return DIENUM_CONTINUE;

			if ( pMouse == TNULL )
			{
				pMouse = new TInputDXDeviceMouse;

				if ( pMouse == TNULL )
				{
					pInputDevice->Release();
					return DIENUM_CONTINUE;
				}

				bIsNewMouse = TTRUE;
			}
			else if ( pMouse->GetDeviceInstance().dwDevType & DIDEVTYPE_HID )
			{
				pInputDevice->Release();
				return DIENUM_CONTINUE;
			}

			if ( !pMouse->BindToDIDevice( pInputInterface->m_hMainWindow, a_poDeviceInstance, pInputDevice, pInputInterface->m_bExclusive ) )
			{
				if ( !bIsNewMouse )
					pInputInterface->RemoveDevice( pMouse );

				delete pMouse;
				return DIENUM_CONTINUE;
			}

			DIPROPDWORD oProperty;
			oProperty.diph.dwSize       = sizeof( DIPROPDWORD );
			oProperty.diph.dwHeaderSize = sizeof( DIPROPHEADER );
			oProperty.diph.dwObj        = 0;
			oProperty.diph.dwHow        = DIPH_DEVICE;
			oProperty.dwData            = 0x20;

			if ( FAILED( pInputDevice->SetProperty( DIPROP_BUFFERSIZE, &oProperty.diph ) ) )
				return DIENUM_STOP;

			if ( bIsNewMouse == TTRUE )
			{
				pMouse->Initialise();
				pMouse->Acquire();
				pInputInterface->AddDevice( pMouse );
			}

			return DIENUM_CONTINUE;
		}
		case DI8DEVTYPE_KEYBOARD:
		{
			LPDIRECTINPUTDEVICE8A pInputDevice;

			if ( pInput8->CreateDevice( a_poDeviceInstance->guidInstance, &pInputDevice, NULL ) != DI_OK )
				return DIENUM_CONTINUE;

			TInputDXDeviceKeyboard* pKeyboard = new TInputDXDeviceKeyboard();

			if ( pKeyboard == TNULL )
			{
				pInputDevice->Release();
				return DIENUM_CONTINUE;
			}

			if ( !pKeyboard->BindToDIDevice( pInputInterface->GetMainWindow(), a_poDeviceInstance, pInputDevice ) )
			{
				delete pKeyboard;
				return DIENUM_CONTINUE;
			}

			DIPROPDWORD oProperty;
			oProperty.diph.dwSize       = sizeof( DIPROPDWORD );
			oProperty.diph.dwHeaderSize = sizeof( DIPROPHEADER );
			oProperty.diph.dwObj        = 0;
			oProperty.diph.dwHow        = DIPH_DEVICE;
			oProperty.dwData            = 0x20;

			if ( FAILED( pInputDevice->SetProperty( DIPROP_BUFFERSIZE, &oProperty.diph ) ) )
				return DIENUM_STOP;

			// NOTE: unlike Barnyard the non-exclusive keyboard reads the keys in the background too
			HRESULT hRes = pInputDevice->SetCooperativeLevel(
			    pInputInterface->GetMainWindow(),
			    pInputInterface->GetExclusiveMode() == TTRUE ? ( DISCL_NOWINKEY | DISCL_FOREGROUND | DISCL_EXCLUSIVE ) : ( DISCL_BACKGROUND | DISCL_NONEXCLUSIVE )
			);

			if ( hRes != DI_OK )
			{
				delete pKeyboard;
				return DIENUM_CONTINUE;
			}

			pKeyboard->Initialise();
			pKeyboard->Acquire();
			pInputInterface->AddDevice( pKeyboard );

			return DIENUM_CONTINUE;
		}
		case DI8DEVTYPE_JOYSTICK:
		case DI8DEVTYPE_GAMEPAD:
		case DI8DEVTYPE_1STPERSON:
		{
			TCHAR szGUID[ 37 ];
			FormatGUID( szGUID, a_poDeviceInstance->guidProduct );

			// Only the PlayStation controllers are DirectInput ones, the others come in through XInput
			if ( !TInputDXDeviceController::IsPSController( a_poDeviceInstance ) )
			{
				TUtil::Log( "Added XInput Controller: '%s' (%s) - NON-PSX", a_poDeviceInstance->tszProductName, szGUID );
				return DIENUM_CONTINUE;
			}

			TUtil::Log( "Added Direct Input Controller: '%s' (%s) - PSX", a_poDeviceInstance->tszProductName, szGUID );

			LPDIRECTINPUTDEVICE8A pInputDevice;

			if ( pInput8->CreateDevice( a_poDeviceInstance->guidInstance, &pInputDevice, NULL ) != DI_OK )
				return DIENUM_CONTINUE;

			TInputDXDeviceController* pController = new TInputDXDeviceController();

			if ( pController == TNULL )
			{
				pInputDevice->Release();
				return DIENUM_CONTINUE;
			}

			if ( !pController->BindToDIDevice( pInputInterface->m_hMainWindow, a_poDeviceInstance, pInputDevice ) )
			{
				delete pController;
				return DIENUM_CONTINUE;
			}

			pController->Initialise();
			pInputInterface->AddDevice( pController );
			pController->Acquire();

			return DIENUM_CONTINUE;
		}
		default:
			break;
	}

	return DIENUM_CONTINUE;
}

TOSHI_NAMESPACE_END

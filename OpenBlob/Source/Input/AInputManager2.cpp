#include "pch.h"
#include "AInputManager2.h"
#include "GameInterface/APlayerManager.h"

#include <Platform/Windows/Input/TInputInterface_Win.h>
#include <Input/TInputDeviceController.h>

#ifdef TRENDERINTERFACE_DX11
#  include <Platform/DX11/TRender_DX11.h>
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005daef0
AInputManager2::AInputManager2()
{
	// The handles (m_aControllers, m_InGameControllers' items, m_oLockedDevice) are cleared by their constructor
	m_pLastButtonDevice = TNULL;
	m_eButtonConfig     = 0;

	// NOTE: retail clears only the first two handles (0x10 bytes)
	TUtil::MemClear( m_aControllers, 0x10 );

	TInputDXInterface* pInput = new TInputDXInterface();

#ifdef TRENDERINTERFACE_DX11
	// NOTE: retail's TInputDXInterface::Initialise finds the window by its class name "TRenderD3D"
	pInput->SetMainWindow( TRenderDX11::Interface()->GetMSWindow()->GetHWND() );
#endif

	TBOOL result = TInputInterface::GetSingleton()->Initialise();
	TASSERT( result == TTRUE );

	for ( TINT i = 5; i != 0; i-- )
	{
		TInputInterface::GetSingleton()->AcquireAll();
		TInputInterface::GetSingleton()->ProcessEvents( 0.1f );
		UpdateControllers();
		CheckConnectedControllers();
		Sleep( 200 );
	}

	TInputInterface::GetSingleton()->AcquireAll();
	TInputInterface::GetSingleton()->ProcessEvents( 0.1f );
	UpdateControllers();
	CheckConnectedControllers();
}

// $deBlob: FUNCTION 005db1b0
// $deBlob: FUNCTION 005dbc90
AInputManager2::~AInputManager2()
{
	// NOTE: retail deletes the input interface without Deinitialise
	if ( TInputInterface::GetSingleton() != TNULL )
		delete TInputInterface::GetSingleton();
}

// $deBlob: FUNCTION 005db260
void AInputManager2::Update( TFLOAT a_fDeltaTime )
{
	TInputInterface::GetSingleton()->AcquireAll();
	TInputInterface::GetSingleton()->ProcessEvents( a_fDeltaTime );
	UpdateControllers();
	CheckConnectedControllers();
}

void AInputManager2::CheckConnectedControllers()
{
	if ( TInputDevice::IsGamepadTypeConnected( TInputDevice::Gamepad_XBox ) ) return;
	if ( TInputDevice::IsGamepadTypeConnected( TInputDevice::Gamepad_PS4 ) ) return;
	if ( TInputDevice::IsGamepadTypeConnected( TInputDevice::Gamepad_Wii ) ) return;

	TBOOL bSeveralPlayers = APlayerManager::IsSingletonCreated() && APlayerManager::GetSingleton()->GetNumPlayers() > 1;

	TInputDevice::SetLastGamepadType( TInputDevice::Gamepad_Keyboard );

	if ( !TInputDevice::ArePlayerDevicesLocked() && !bSeveralPlayers )
		TInputDevice::ResetPlayerDevices();
}

// $deBlob: FUNCTION 005db2f0
TINT AInputManager2::GetConnectedDeviceCount()
{
	TINT iCount = 0;

	for ( TINT i = 0; i < MAX_NUM_CONTROLLERS; i++ )
	{
		if ( m_aControllers[ i ].m_pDevice != TNULL )
			iCount++;
	}

	// NOTE: retail counts in a TUINT8
	if ( TUINT8( iCount ) == 0 )
		return 1;

	return TUINT8( iCount );
}

// $deBlob: FUNCTION 005db330
AInputManager2::AInputDeviceHandle* AInputManager2::GetDeviceHandle( TInputDevice* a_pDevice )
{
	for ( TINT i = 0; i < MAX_NUM_CONTROLLERS; i++ )
	{
		if ( a_pDevice == m_aControllers[ i ].m_pDevice )
			return &m_aControllers[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 005db370
void AInputManager2::SetContext( AInputMap::INPUTCONTEXT a_eContext )
{
	m_InputContextStack.Push( a_eContext );
}

// $deBlob: FUNCTION 005db3a0
AInputMap::INPUTCONTEXT AInputManager2::GetContext()
{
	if ( m_InputContextStack.Empty() )
		return AInputMap::INPUTCONTEXT_None;

	return m_InputContextStack.Top();
}

// $deBlob: FUNCTION 005db3f0
void AInputManager2::RemoveCurrentContext()
{
	TBOOL bIsEmpty = m_InputContextStack.Empty();
	TASSERT( TFALSE == bIsEmpty );

	m_InputContextStack.Pop();
}

// $deBlob: FUNCTION 005db480
void AInputManager2::AddInGameController( const AInputDeviceHandle& a_Handle )
{
	TASSERT( a_Handle.IsValid() );

	for ( auto it = m_InGameControllers.Begin(); it != m_InGameControllers.End(); it++ )
	{
		if ( it->m_pDevice == a_Handle.m_pDevice )
			return;
	}

	m_InGameControllers.PushBack( a_Handle );
}

// $deBlob: FUNCTION 005db540
void AInputManager2::RemoveInGameController( const AInputDeviceHandle& a_Handle )
{
	TASSERT( a_Handle.IsValid() );

	for ( auto it = m_InGameControllers.Begin(); it != m_InGameControllers.End(); it++ )
	{
		if ( it->m_pDevice == a_Handle.m_pDevice )
		{
			m_InGameControllers.Erase( it );
			return;
		}
	}
}

// $deBlob: FUNCTION 005db5d0
void AInputManager2::RemoveAllInGameControllers()
{
	m_InGameControllers.Clear();
}

// TTRUE while every controller of the players is enabled (connected)
// $deBlob: FUNCTION 005db600
TBOOL AInputManager2::InGameDevicesCheck()
{
	for ( auto it = m_InGameControllers.Begin(); it != m_InGameControllers.End(); it++ )
	{
		if ( !it->m_pDevice->IsEnabled() )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005db730
TBOOL AInputManager2::CheckIfValidDevice( const AInputDeviceHandle& a_rHandle )
{
	if ( m_oLockedDevice.m_pDevice == TNULL )
		return TTRUE;

	return a_rHandle.m_pDevice == m_oLockedDevice.m_pDevice;
}

// $deBlob: FUNCTION 005db770
void AInputManager2::SetDeviceButtonConfig( TINT a_eConfig )
{
	m_eButtonConfig = a_eConfig;

	for ( TINT i = 0; i < 4; i++ )
		m_aDeviceButtonConfigs[ i ] = a_eConfig;
}

// $deBlob: FUNCTION 005db7a0
void AInputManager2::LoadDeviceButtonConfigFromSave( TUINT8 a_uiInputConfig )
{
	TASSERT( 0 == a_uiInputConfig || 1 == a_uiInputConfig );
	SetDeviceButtonConfig( a_uiInputConfig != 0 );
}

// $deBlob: FUNCTION 005db810
void AInputManager2::SetPerDeviceButtonConfig( TINT a_iDevice, TINT a_eConfig )
{
	m_aDeviceButtonConfigs[ a_iDevice ] = a_eConfig;
}

// $deBlob: FUNCTION 005db830
TINT AInputManager2::GetPerDeviceButtonConfig( TINT a_iDevice )
{
	return m_aDeviceButtonConfigs[ a_iDevice ];
}

// $deBlob: FUNCTION 005db850
void AInputManager2::ResetPerDeviceButtonConfig()
{
	for ( TINT i = 0; i < 4; i++ )
		m_aDeviceButtonConfigs[ i ] = m_eButtonConfig;
}

// $deBlob: FUNCTION 005db890
TBOOL AInputManager2::GetButtonDown( AInputMap::INPUTBUTTON a_eButton, AInputMap::ButtonMap* a_pButtonMap, TInputDevice* a_pDevice )
{
	if ( a_pButtonMap == TNULL )
		return TFALSE;

	auto itBinds = a_pButtonMap->Find( a_eButton );

	if ( itBinds == a_pButtonMap->End() )
		return TFALSE;

	AInputMap::BindVector oBinds = itBinds->GetSecond();

	for ( auto it = oBinds.Begin(); it != oBinds.End(); it++ )
	{
		if ( a_pDevice->IsDown( *it ) == TTRUE )
		{
			m_pLastButtonDevice = a_pDevice;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005db990
const TCHAR* AInputManager2::Debug_GetDeviceName( INPUTDEVICE a_eDevice )
{
	if ( a_eDevice == INPUTDEVICE_Controller1 )
		return "Input Device 1";

	if ( a_eDevice == INPUTDEVICE_Controller2 )
		return "Input Device 2";

	return "Invalid Device Name";
}

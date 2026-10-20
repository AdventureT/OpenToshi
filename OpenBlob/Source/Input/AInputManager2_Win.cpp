#include "pch.h"
#include "AInputManager2.h"

#include <Input/TInputDeviceController.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceMouse.h>
#include <Platform/Windows/Input/TInputDeviceMouse_Win.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005dc060
AInputManager2::AInputDeviceHandle& AInputManager2::GetControllerHandle( INPUTDEVICE a_eDevice )
{
	TASSERT( 0 <= a_eDevice );
	TASSERT( INPUTDEVICE_Count > a_eDevice );
	TASSERT( INPUTDEVICE_Invalid != a_eDevice );

	return m_aControllers[ a_eDevice ];
}

// The right stick and the mouse
// $deBlob: FUNCTION 005dc120
void AInputManager2::GetCameraAimAxis( TVector2& a_rAxis, const AInputDeviceHandle& a_rHandle )
{
	a_rAxis.x = 0.0f;
	a_rAxis.y = 0.0f;

	TInputDeviceController* pController = TSTATICCAST( TInputDeviceController, a_rHandle.m_pDevice );

	if ( pController != TNULL )
	{
		TASSERT( pController->IsA( &TGetClass( TInputDeviceController ) ) );
		a_rAxis.x = pController->GetAxisFloat( TInputDeviceController::STICK_RIGHT, 0 );
		a_rAxis.y = pController->GetAxisFloat( TInputDeviceController::STICK_RIGHT, 1 );
	}

	TInputDeviceMouse* pMouse = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceMouse>( 0 );

	if ( pMouse != TNULL )
	{
		a_rAxis.x += pMouse->GetAxisFloat( TInputDeviceMouse::AXIS_CURSOR, 0 );
		a_rAxis.y += pMouse->GetAxisFloat( TInputDeviceMouse::AXIS_CURSOR, 1 );
	}
}

// The left stick and I/K/J/L
// $deBlob: FUNCTION 005dc1f0
void AInputManager2::GetCameraMoveAxis( TVector2& a_rAxis, const AInputDeviceHandle& a_rHandle )
{
	a_rAxis.x = 0.0f;
	a_rAxis.y = 0.0f;

	TInputDeviceController* pController = TSTATICCAST( TInputDeviceController, a_rHandle.m_pDevice );

	if ( pController != TNULL )
	{
		TASSERT( pController->IsA( &TGetClass( TInputDeviceController ) ) );
		a_rAxis.x = pController->GetAxisFloat( TInputDeviceController::STICK_LEFT, 0 );
		a_rAxis.y = -pController->GetAxisFloat( TInputDeviceController::STICK_LEFT, 1 );
	}

	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	if ( pKeyboard != TNULL )
	{
		if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_I ) ) a_rAxis.y += 1.0f;
		if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_K ) ) a_rAxis.y -= 1.0f;
		if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_J ) ) a_rAxis.x -= 1.0f;
		if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_L ) ) a_rAxis.x += 1.0f;
	}
}

// The left stick and the arrows (slow with shift), only in the in-game context
// $deBlob: FUNCTION 005dc340
void AInputManager2::GetPlayerMoveAxis( TVector2& a_rAxis, const AInputDeviceHandle& a_rHandle )
{
	a_rAxis.x = 0.0f;
	a_rAxis.y = 0.0f;

	if ( GetContext() != AInputMap::INPUTCONTEXT_InGame )
		return;

	TInputDeviceController* pController = TSTATICCAST( TInputDeviceController, a_rHandle.m_pDevice );
	TASSERT( pController == TNULL || pController->IsA( &TGetClass( TInputDeviceController ) ) );

	if ( pController != TNULL )
	{
		a_rAxis.x = pController->GetAxisFloat( TInputDeviceController::STICK_LEFT, 0 );
		a_rAxis.y = -pController->GetAxisFloat( TInputDeviceController::STICK_LEFT, 1 );
	}

	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	if ( pKeyboard != TNULL )
	{
		if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_UP ) )
			a_rAxis.y = pKeyboard->IsShiftDown() ? 0.1f : 1.0f;

		if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_DOWN ) )
			a_rAxis.y = pKeyboard->IsShiftDown() ? -0.1f : -1.0f;

		if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_LEFT ) )
			a_rAxis.x = pKeyboard->IsShiftDown() ? -0.1f : -1.0f;

		if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_RIGHT ) )
			a_rAxis.x = pKeyboard->IsShiftDown() ? 0.1f : 1.0f;
	}
}

// One stick of the controller (0 the left one), only in the in-game context
// $deBlob: FUNCTION 005dc4d0
void AInputManager2::GetAnalogStickMoveAxis( TVector2& a_rAxis, const AInputDeviceHandle& a_rHandle, TINT a_eStick )
{
	a_rAxis.x = 0.0f;
	a_rAxis.y = 0.0f;

	if ( GetContext() != AInputMap::INPUTCONTEXT_InGame )
		return;

	TInputDeviceController* pController = TSTATICCAST( TInputDeviceController, a_rHandle.m_pDevice );

	if ( pController == TNULL )
		return;

	TASSERT( pController->IsA( &TGetClass( TInputDeviceController ) ) );

	if ( a_eStick == 0 )
	{
		a_rAxis.x = pController->GetAxisFloat( TInputDeviceController::STICK_LEFT, 0 );
		a_rAxis.y = -pController->GetAxisFloat( TInputDeviceController::STICK_LEFT, 1 );
		return;
	}

	a_rAxis.x = pController->GetAxisFloat( TInputDeviceController::STICK_RIGHT, 0 );
	a_rAxis.y = -pController->GetAxisFloat( TInputDeviceController::STICK_RIGHT, 1 );
}

// $deBlob: FUNCTION 005dc5c0
void AInputManager2::GetCursorXY( TINT& a_rX, TINT& a_rY )
{
	a_rY = 0;
	a_rX = 0;

	TInputDXDeviceMouse* pMouse = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDXDeviceMouse>( 0 );

	if ( pMouse != TNULL )
	{
		a_rX = pMouse->GetPositionX();
		a_rY = pMouse->GetPositionY();
	}
}

// $deBlob: FUNCTION 005dc600
void AInputManager2::GetCursorDeltaXY( TINT& a_rX, TINT& a_rY )
{
	TInputDXDeviceMouse* pMouse = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDXDeviceMouse>( 0 );

	if ( pMouse != TNULL )
	{
		a_rX = pMouse->GetAxisInt( TInputDeviceMouse::AXIS_CURSOR, 0 );
		a_rY = pMouse->GetAxisInt( TInputDeviceMouse::AXIS_CURSOR, 1 );
	}
}

// $deBlob: FUNCTION 005dc650
TBOOL AInputManager2::IsLeftMouseDown()
{
	TInputDeviceMouse* pMouse = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceMouse>( 0 );

	if ( pMouse != TNULL )
		return pMouse->IsDown( TInputDeviceMouse::BUTTON_1 );

	return TFALSE;
}

// $deBlob: FUNCTION 005dc680
TBOOL AInputManager2::IsRightMouseDown()
{
	TInputDeviceMouse* pMouse = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceMouse>( 0 );

	if ( pMouse != TNULL )
		return pMouse->IsDown( TInputDeviceMouse::BUTTON_2 );

	return TFALSE;
}

// NOTE: retail only checks the keyboard inside the loop when a controller is connected, then once more after it
// $deBlob: FUNCTION 005dc6b0
TBOOL AInputManager2::GetButtonDown( AInputMap::INPUTBUTTON a_eButton )
{
	for ( TINT i = 0; i < MAX_NUM_CONTROLLERS; i++ )
	{
		AInputDeviceHandle& rHandle = m_aControllers[ i ];

		if ( rHandle.m_pDevice == TNULL )
			continue;

		TBOOL                 bDown     = TFALSE;
		TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

		if ( pKeyboard != TNULL )
			bDown = GetButtonDown( a_eButton, m_InputMap.GetButtonMap( GetContext() ), pKeyboard );

		if ( rHandle.m_pDevice != TNULL && CheckIfValidDevice( rHandle ) )
			bDown |= GetButtonDown( a_eButton, m_InputMap.GetButtonMap( GetContext() ), rHandle.m_pDevice );

		if ( bDown )
			return TTRUE;
	}

	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	if ( pKeyboard == TNULL )
		return TFALSE;

	return GetButtonDown( a_eButton, m_InputMap.GetButtonMap( GetContext() ), pKeyboard );
}

// $deBlob: FUNCTION 005dc850
TBOOL AInputManager2::GetButtonDown( AInputMap::INPUTBUTTON a_eButton, const AInputDeviceHandle& a_rHandle )
{
	TBOOL                 bDown     = TFALSE;
	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	if ( pKeyboard != TNULL )
		bDown = GetButtonDown( a_eButton, m_InputMap.GetButtonMap( GetContext() ), pKeyboard );

	if ( a_rHandle.m_pDevice != TNULL && CheckIfValidDevice( a_rHandle ) == TTRUE )
		bDown |= GetButtonDown( a_eButton, m_InputMap.GetButtonMap( GetContext() ), a_rHandle.m_pDevice );

	return bDown;
}

// $deBlob: FUNCTION 005dc8d0
AInputManager2::ButtonResult AInputManager2::GetButtonDownWithResult( AInputMap::INPUTBUTTON a_eButton )
{
	ButtonResult oResult;
	oResult.m_bDown            = TFALSE;
	oResult.m_Handle.m_pDevice = TNULL;
	oResult.m_Handle.m_iId     = 0;

	for ( TINT i = 0; i < MAX_NUM_CONTROLLERS; i++ )
	{
		AInputDeviceHandle& rHandle = m_aControllers[ i ];

		if ( rHandle.m_pDevice == TNULL )
			continue;

		TBOOL                 bDown     = TFALSE;
		TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

		if ( pKeyboard != TNULL )
			bDown = GetButtonDown( a_eButton, m_InputMap.GetButtonMap( GetContext() ), pKeyboard );

		if ( rHandle.m_pDevice != TNULL && CheckIfValidDevice( rHandle ) )
			bDown |= GetButtonDown( a_eButton, m_InputMap.GetButtonMap( GetContext() ), rHandle.m_pDevice );

		if ( bDown )
		{
			oResult.m_bDown  = TTRUE;
			oResult.m_Handle = m_aControllers[ i ];
			return oResult;
		}
	}

	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	if ( pKeyboard != TNULL )
		oResult.m_bDown = GetButtonDown( a_eButton, m_InputMap.GetButtonMap( GetContext() ), pKeyboard );

	return oResult;
}

// Any button of a controller or any key
// $deBlob: FUNCTION 005dcad0
TBOOL AInputManager2::IsAnyButtonPressed()
{
	for ( TINT i = 0; i < MAX_NUM_CONTROLLERS; i++ )
	{
		if ( m_aControllers[ i ].m_pDevice == TNULL )
			continue;

		for ( TINT iDoodad = 0x10000; iDoodad < 0x1001C; iDoodad++ )
		{
			if ( m_aControllers[ i ].m_pDevice->IsDown( iDoodad ) == TTRUE )
				return TTRUE;
		}
	}

	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	if ( pKeyboard != TNULL )
	{
		for ( TINT iDoodad = 0x20000; iDoodad < 0x20080; iDoodad++ )
		{
			if ( pKeyboard->IsDown( iDoodad ) == TTRUE )
				return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005dcba0
void AInputManager2::UpdateControllers()
{
	TInputInterface* pInput             = TInputInterface::GetSingleton();
	TINT             iDeviceSearchIndex = 0;
	TINT             iNumControllers    = 0;

	for ( TINT i = 0; i < MAX_NUM_CONTROLLERS; i++ )
	{
		AInputDeviceHandle* pHandle = &m_aControllers[ i ];
		pHandle->m_pDevice          = TNULL;
		pHandle->m_iId              = 0;

		TInputDevice* pDevice = pInput->GetDeviceByIndex( &TGetClass( TInputDeviceController ), iDeviceSearchIndex );

		while ( pDevice != TNULL )
		{
			if ( pDevice->IsEnabled() )
			{
				pHandle->m_iId     = iNumControllers++;
				pHandle->m_pDevice = pDevice;
				break;
			}

			pDevice = pInput->GetDeviceByIndex( &TGetClass( TInputDeviceController ), ++iDeviceSearchIndex );
		}

		iDeviceSearchIndex++;
	}
}

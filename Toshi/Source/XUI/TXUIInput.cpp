#include "ToshiPCH.h"
#include "TXUIInput.h"
#include "TXUI.h"
#include "TXUIElement.h"

#include "Input/TInputInterface.h"
#include "Input/TInputDeviceKeyboard.h"
#include "Input/TInputDeviceMouse.h"
#include "Input/TInputDeviceController.h"
#include "TXUIList.h"
#include "TXUIScene.h"
#include "TXUILabel.h"
#include "Render/TRender.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/Input/TInputDeviceKeyboard_Win.h"
#  include "Platform/Windows/Input/TInputDeviceMouse_Win.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006d6f50
TXUIInput::TXUIInput()
{
	m_pRepeatElement = TNULL;
	m_bCursorVisible = TTRUE;
	m_bEnabled       = TTRUE;
	m_pClickElement  = TNULL;
	m_iMouseX        = 0;
	m_iMouseY        = 0;

	Connect( TInputInterface::GetSingleton()->GetInputEventEmitter(), this, OnInputEvent, 0 );
}

// $deBlob: FUNCTION 006d6fa0
// $deBlob: FUNCTION 006be620
TXUIInput::~TXUIInput()
{
	Disconnect();
}

// $deBlob: FUNCTION 006d6fb0
TBOOL TXUIInput::Tick( TFLOAT a_fDeltaTime )
{
	return TFALSE;
}

// $deBlob: FUNCTION 006d6fc0
TBOOL __stdcall TXUIInput::OnInputEvent( void* a_pCaller, void* a_pOwner, void* a_pData )
{
	return TSTATICCAST( TXUIInput, a_pCaller )->ProcessInputEvent( TSTATICCAST( TInputInterface, a_pOwner ), TSTATICCAST( TInputInterface::InputEvent, a_pData ) );
}

// The input event handler: turns the TInputInterface events into TXUI::HandleButton/HandleButtonRelease/HandleAnalog
// calls. Returning TFALSE stops the emitter (the next listeners don't get the event).
// $deBlob: FUNCTION 006d6fe0
TBOOL TXUIInput::ProcessInputEvent( TInputInterface* a_pInputInterface, TInputInterface::InputEvent* a_pEvent )
{
	if ( !m_bEnabled )
		return TFALSE;

	TXUI*         pXUI               = TXUI::GetSingleton();
	TINT          iDefaultController = pXUI->GetDefaultController();
	TInputDevice* pSource            = a_pEvent->GetSource();

	if ( !pSource->IsA( &TGetClass( TInputDXDeviceMouse ) ) )
	{
		TINT eEventType = a_pEvent->GetEventType();

		if ( pSource->IsA( &TGetClass( TInputDXDeviceKeyboard ) ) && ( eEventType == TInputInterface::EVENT_TYPE_GONE_DOWN || eEventType == TInputInterface::EVENT_TYPE_REPEAT ) )
		{
			a_pEvent->StartRepeat( 0.3f, 0.1f );
			TINT iDoodad = a_pEvent->GetDoodad();

			// The arrows don't navigate while the mouse is over the item of a list
			if ( m_pRepeatElement != TNULL && m_pRepeatElement->IsA( &TGetClass( TXUIList ) ) && TSTATICCAST( TXUIList, m_pRepeatElement )->GetRayTraceItem() != TNULL )
			{
				if ( iDoodad == TInputDeviceKeyboard::KEY_LEFT ) return TFALSE;
				if ( iDoodad == TInputDeviceKeyboard::KEY_RIGHT ) return TFALSE;
				if ( iDoodad == TInputDeviceKeyboard::KEY_UP ) return TFALSE;
				if ( iDoodad == TInputDeviceKeyboard::KEY_DOWN ) return TFALSE;
			}

			TINT iButton = -1;

			if ( iDoodad == TInputDeviceKeyboard::KEY_RETURN || iDoodad == TInputDeviceKeyboard::KEY_SPACE )
				iButton = TXUIElement::XUIButton_Accept;
			else if ( iDoodad == TInputDeviceKeyboard::KEY_LEFT )
				iButton = TXUIElement::XUIButton_Left;
			else if ( iDoodad == TInputDeviceKeyboard::KEY_RIGHT )
				iButton = TXUIElement::XUIButton_Right;
			else if ( iDoodad == TInputDeviceKeyboard::KEY_UP )
				iButton = TXUIElement::XUIButton_Up;
			else if ( iDoodad == TInputDeviceKeyboard::KEY_DOWN )
				iButton = TXUIElement::XUIButton_Down;
			else if ( iDoodad == TInputDeviceKeyboard::KEY_B || iDoodad == TInputDeviceKeyboard::KEY_ESCAPE )
				iButton = TXUIElement::XUIButton_Back;

			// Accept and back don't repeat
			if ( eEventType == TInputInterface::EVENT_TYPE_REPEAT )
			{
				if ( iButton == TXUIElement::XUIButton_Accept ) return TTRUE;
				if ( iButton == TXUIElement::XUIButton_Back ) return TTRUE;
			}

			if ( iButton == -1 )
				return TTRUE;

			TXUIControl* pEngagement;

			if ( !TInputDevice::ArePlayerDevicesLocked() || pXUI->GetActiveScene() == TNULL || ( pEngagement = pXUI->GetActiveScene()->FindChildXUIControl( L"MPEngagement" ) ) == TNULL )
			{
				TInputDevice::SetLastXUIDeviceIndex( pSource->GetDeviceIndex() );
				return pXUI->HandleButton( iButton, iDefaultController );
			}

			return pEngagement->HandleButton( iButton, pSource->GetDeviceIndex() );
		}

		if ( !pSource->IsA( &TGetClass( TInputDeviceController ) ) )
			return TTRUE;

		if ( eEventType == TInputInterface::EVENT_TYPE_GONE_DOWN || eEventType == TInputInterface::EVENT_TYPE_REPEAT )
		{
			a_pEvent->StartRepeat( 0.3f, 0.1f );
			TINT iDoodad = a_pEvent->GetDoodad();
			TINT iButton = -1;

			if ( iDoodad == TInputDeviceController::BUTTON_DPAD_LEFT || iDoodad == TInputDeviceController::VIRTBTN_STICK_LEFT_LEFT )
				iButton = TXUIElement::XUIButton_Left;
			else if ( iDoodad == TInputDeviceController::BUTTON_DPAD_RIGHT || iDoodad == TInputDeviceController::VIRTBTN_STICK_LEFT_RIGHT )
				iButton = TXUIElement::XUIButton_Right;
			else if ( iDoodad == TInputDeviceController::BUTTON_DPAD_UP || iDoodad == TInputDeviceController::VIRTBTN_STICK_LEFT_UP )
				iButton = TXUIElement::XUIButton_Up;
			else if ( iDoodad == TInputDeviceController::BUTTON_DPAD_DOWN || iDoodad == TInputDeviceController::VIRTBTN_STICK_LEFT_DOWN )
				iButton = TXUIElement::XUIButton_Down;
			else if ( iDoodad == TInputDeviceController::BUTTON_CONTEXT_1 )
				iButton = TXUIElement::XUIButton_Accept;
			else if ( iDoodad == TInputDeviceController::BUTTON_CONTEXT_2 )
				iButton = TXUIElement::XUIButton_Back;

			// Accept and back don't repeat
			if ( eEventType == TInputInterface::EVENT_TYPE_REPEAT )
			{
				if ( iButton == TXUIElement::XUIButton_Accept ) return TTRUE;
				if ( iButton == TXUIElement::XUIButton_Back ) return TTRUE;
			}

			if ( iButton == -1 )
				return TTRUE;

			TINT         iPlayer = pSource->GetPlayer();
			TXUIControl* pEngagement;

			if ( !TInputDevice::ArePlayerDevicesLocked() || pXUI->GetActiveScene() == TNULL || ( pEngagement = pXUI->GetActiveScene()->FindChildXUIControl( L"MPEngagement" ) ) == TNULL )
			{
				TInputDevice::SetLastXUIDeviceIndex( pSource->GetDeviceIndex() );

				// Only the player of the default controller navigates
				TINT iController = TXUI::GetSingleton()->GetDefaultController();

				if ( iPlayer != iController )
					return TFALSE;

				pXUI->SetDefaultController( 0 );
				TBOOL bHandled = pXUI->HandleButton( iButton, 0 );
				pXUI->SetDefaultController( iController );

				return bHandled;
			}

			return pEngagement->HandleButton( iButton, pSource->GetDeviceIndex() );
		}

		if ( eEventType != TInputInterface::EVENT_TYPE_MOVED )
			return TTRUE;

		// The cursor of the controller: the stick's position over 1.25 times the screen, relative to its centre, and the
		// element under it
		TFLOAT                  fStickX        = a_pEvent->GetMagnitudeFloat( 0 );
		TFLOAT                  fStickY        = a_pEvent->GetMagnitudeFloat( 1 );
		TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();
		TFLOAT                  fWidth         = TFLOAT( TINT( pDisplayParams->uiWidth ) ) * 1.25f;
		TFLOAT                  fHeight        = TFLOAT( TINT( pDisplayParams->uiHeight ) ) * 1.25f;

		m_iMouseX = TINT( fWidth * fStickX - fWidth * 0.5f );
		m_iMouseY = TINT( fHeight * fStickY - fHeight * 0.5f );

		TXUIScene* pActiveScene   = pXUI->GetActiveScene();
		TBOOL      bTransitioning = pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked();

		// The capturing control gets all of it
		if ( !bTransitioning && pXUI->GetCapturing() != TNULL )
		{
			TInputDevice::SetLastXUIDeviceIndex( pSource->GetDeviceIndex() );
			pXUI->GetCapturing()->HandleAnalog( m_iMouseX, m_iMouseY, iDefaultController );
			return TTRUE;
		}

		if ( pActiveScene == TNULL || !pActiveScene->IsAcceptingInput() || !pActiveScene->IsVisible() || bTransitioning )
		{
			m_pRepeatElement = TNULL;
			return TTRUE;
		}

		TInputDevice::SetLastXUIDeviceIndex( pSource->GetDeviceIndex() );

		TVector2     vecPoint = TVector2( TFLOAT( m_iMouseX ), TFLOAT( m_iMouseY ) );
		TVector2     vecScale( 1.0f, 1.0f );
		TXUIElement* pElement = pActiveScene->RayTraceDeepest( vecPoint, vecScale, iDefaultController );

		if ( pElement == TNULL )
		{
			m_pRepeatElement = TNULL;
			return TTRUE;
		}

		// A control that isn't a TXUILabel or a scene
		TBOOL bControl = pElement->IsA( &TGetClass( TXUIControl ) ) && !pElement->IsA( &TGetClass( TXUILabel ) ) && !pElement->IsA( &TGetClass( TXUIScene ) );
		TBOOL bVisible = pElement->IsVisible();

		if ( !bControl || !bVisible || !TSTATICCAST( TXUIControl, pElement )->IsEnabled() )
		{
			m_pRepeatElement = TNULL;
			return TTRUE;
		}

		if ( pElement != m_pRepeatElement )
		{
			m_pRepeatElement = pElement;

			// The scenes up the chain are told about the new element under the cursor
			if ( !TSTATICCAST( TXUIControl, pElement )->IsFocused() )
			{
				TXUIControl* pChild = TSTATICCAST( TXUIControl, pElement );

				for ( TXUIScene* pScene = pChild->FindParentScene(); pScene != TNULL; pScene = pScene->FindParentScene() )
				{
					pScene->OnMouseOverChange( pChild, iDefaultController );
					pChild = pScene;
				}
			}
		}

		if ( m_pRepeatElement != TNULL )
			TSTATICCAST( TXUIControl, m_pRepeatElement )->HandleAnalog( m_iMouseX, m_iMouseY, iDefaultController );

		return TTRUE;
	}

	// The mouse
	if ( TInputDevice::ArePlayerDevicesLocked() )
		return TFALSE;

	if ( a_pEvent->GetDoodad() == TInputDeviceMouse::BUTTON_1 )
	{
		if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
		{
			if ( m_pRepeatElement == TNULL )
				return TFALSE;

			return pXUI->HandleButton( TXUIElement::XUIButton_Accept, iDefaultController );
		}

		if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			return pXUI->HandleButtonRelease( TXUIElement::XUIButton_Accept, iDefaultController );
	}

	if ( a_pEvent->GetEventType() != TInputInterface::EVENT_TYPE_MOVED )
		return TTRUE;

	// The cursor: the position of the mouse relative to the centre of the screen, the element under it
	TInputDXDeviceMouse*    pMouse         = TSTATICCAST( TInputDXDeviceMouse, pSource );
	TINT                    iPositionX     = pMouse->GetPositionX();
	TINT                    iPositionY     = pMouse->GetPositionY();
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

	m_iMouseX = iPositionX - TINT( pDisplayParams->uiWidth ) / 2;
	m_iMouseY = iPositionY - TINT( pDisplayParams->uiHeight ) / 2;

	TXUIScene* pActiveScene   = pXUI->GetActiveScene();
	TBOOL      bTransitioning = pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked();

	// The capturing control gets all of it
	if ( !bTransitioning && pXUI->GetCapturing() != TNULL )
	{
		pXUI->GetCapturing()->HandleAnalog( m_iMouseX, m_iMouseY, iDefaultController );
		return TTRUE;
	}

	if ( pActiveScene == TNULL || !pActiveScene->IsAcceptingInput() || !pActiveScene->IsVisible() || bTransitioning )
	{
		m_pRepeatElement = TNULL;
		return TTRUE;
	}

	TVector2     vecPoint = TVector2( TFLOAT( m_iMouseX ), TFLOAT( m_iMouseY ) );
	TVector2     vecScale( 1.0f, 1.0f );
	TXUIElement* pElement = pActiveScene->RayTraceDeepest( vecPoint, vecScale, iDefaultController );

	if ( pElement == TNULL )
	{
		m_pRepeatElement = TNULL;
		return TTRUE;
	}

	// A control that isn't a TXUILabel or a scene
	TBOOL bControl = pElement->IsA( &TGetClass( TXUIControl ) ) && !pElement->IsA( &TGetClass( TXUILabel ) ) && !pElement->IsA( &TGetClass( TXUIScene ) );

	TBOOL bVisible = pElement->IsVisible();

	if ( !bControl || !bVisible || !TSTATICCAST( TXUIControl, pElement )->IsEnabled() )
	{
		m_pRepeatElement = TNULL;
		return TTRUE;
	}

	if ( pElement != m_pRepeatElement )
	{
		m_pRepeatElement = pElement;

		// The scenes up the chain are told about the new element under the cursor
		if ( !TSTATICCAST( TXUIControl, pElement )->IsFocused() )
		{
			TXUIControl* pChild = TSTATICCAST( TXUIControl, pElement );

			for ( TXUIScene* pScene = pChild->FindParentScene(); pScene != TNULL; pScene = pScene->FindParentScene() )
			{
				pScene->OnMouseOverChange( pChild, iDefaultController );
				pChild = pScene;
			}
		}
	}

	TInputDevice::SetLastXUIDeviceIndex( pSource->GetDeviceIndex() );
	TSTATICCAST( TXUIControl, m_pRepeatElement )->HandleAnalog( m_iMouseX, m_iMouseY, iDefaultController );

	return TTRUE;
}

// $deBlob: FUNCTION 006d77e0
void TXUIInput::PlayClickAnimation( TINT a_iController )
{
	if ( m_pClickElement != TNULL )
		m_pClickElement->PlayAnimation( L"Click", L"EndClick" );
}

TOSHI_NAMESPACE_END

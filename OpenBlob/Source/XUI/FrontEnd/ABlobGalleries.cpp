#include "pch.h"
#include "ABlobGalleries.h"
#include "ABlobFrontEnd.h"
#include "XUI/ElementTypes/ABlobCarouselList.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerProfileManager.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIBackButton.h>
#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <Platform/Windows/Input/TInputDeviceMouse_Win.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The offset of the select button from the carousel (shared with ABlobLevelSelection)
static const TFLOAT s_fSelectOffsetX = 51.0f;
static const TFLOAT s_fSelectOffsetY = 12.0f;

// The last button XUI got came from the keyboard or a controller
static TBOOL IsLastXUIDeviceKeyboardOrController()
{
	TInputDevice* pDevice = TInputDevice::GetRegisteredDevices()[ TInputDevice::GetLastXUIDeviceIndex() ];

	// NOTE: retail tests the mouse too and doesn't use the result
	pDevice->IsA( &TGetClass( TInputDXDeviceMouse ) );
	TBOOL bKeyboard   = pDevice->IsA( &TGetClass( TInputDeviceKeyboard ) );
	TBOOL bController = pDevice->IsA( &TGetClass( TInputDeviceController ) );

	return bKeyboard || bController;
}

// $deBlob: FUNCTION 006310f0
// $deBlob: FUNCTION 00631100
// $deBlob: FUNCTION 006311b0
// $deBlob: FUNCTION 00631260
// $deBlob: FUNCTION 00631270
// $deBlob: FUNCTION 0053d490
// $deBlob: FUNCTION 008588a0
TDEFINE_CLASS( ABlobGalleries );

// $deBlob: FUNCTION 00631280
ABlobGalleries::ABlobGalleries()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopExtrasB )
{
	m_pLeft     = TNULL;
	m_pRight    = TNULL;
	m_pCarousel = TNULL;
	m_pSelect   = TNULL;
	m_pTitle    = TNULL;
}

// $deBlob: FUNCTION 006318a0
// $deBlob: FUNCTION 006318c0
// $deBlob: FUNCTION 006318b2
ABlobGalleries::~ABlobGalleries()
{
}

// $deBlob: FUNCTION 006312d0
void ABlobGalleries::OnCreated()
{
	ABlobMenu::OnCreated();

	// NOTE: the result isn't used
	APlayerProfileManager::GetSingleton()->GetCurrentProfile();

	m_pLeft     = FindChildXUIControl( 7 );
	m_pRight    = FindChildXUIControl( 6 );
	m_pCarousel = TSTATICCAST( ABlobCarouselList, FindChildXUIControl( 4 ) );
	m_pSelect   = FindChildXUIControl( 5 );
	m_pCarousel->SetDescription( m_pSelect );
	m_pSelect->SetPosition( s_fSelectOffsetX + m_pCarousel->GetPositionX(), s_fSelectOffsetY + m_pCarousel->GetPositionY() );

	m_pTitle = FindChildXUIControl( 8 );
	m_pTitle->SetText( m_pCarousel->GetFocussedItemText(), -1 );
}

// $deBlob: FUNCTION 006313d0
TBOOL ABlobGalleries::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( a_pSource != m_pBackButton && IsLastXUIDeviceKeyboardOrController() )
	{
		m_pRight->SetFocus( TFALSE, TFALSE, TTRUE );
		m_pLeft->SetFocus( TFALSE, TFALSE, TTRUE );
		m_pSelect->SetFocus( TTRUE, TFALSE, TTRUE );
		FindParentScene()->SetFocus( m_pSelect, a_iController );
	}

	TXUIControl* pArrow;
	TBOOL        bVisible;

	if ( m_pRight == a_pSource )
	{
		if ( m_pCarousel->ScrollRight() )
		{
			pArrow   = m_pLeft;
			bVisible = TTRUE;
		}
		else
		{
			pArrow   = m_pRight;
			bVisible = TFALSE;
		}
	}
	else if ( m_pLeft == a_pSource )
	{
		if ( m_pCarousel->ScrollLeft() )
		{
			pArrow   = m_pRight;
			bVisible = TTRUE;
		}
		else
		{
			pArrow   = m_pLeft;
			bVisible = TFALSE;
		}
	}
	else
	{
		if ( m_pSelect == a_pSource )
		{
			APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
			TINT            iGallery = m_pCarousel->GetFocussedItemIndex();

			if ( ( ( 1 << iGallery ) & pProfile->GetData()->m_uiUnlockedGalleryItems ) == 0 )
			{
				// Locked
				m_pCarousel->PulseLocked();
				return TTRUE;
			}

			AGameStatus::GetSingleton()->SetSelectedGallery( iGallery );

			TXUIScene* pGallery = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "gallery.xur" ), TUINT32( 0 ) );
			TSTATICCAST( ABlobFrontEnd, FindParentScene() )->HideForGallery();
			TXUI::GetSingleton()->TransitionTo( this, pGallery );
		}

		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	pArrow->SetVisible( bVisible );
	m_pTitle->SetText( m_pCarousel->GetFocussedItemText(), -1 );

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}

// $deBlob: FUNCTION 006316a0
TBOOL ABlobGalleries::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( m_pCarousel->IsScrolling() )
		return TXUIScene::HandleButton( a_eButton, a_iController );

	TXUIControl* pFocus;

	if ( a_eButton == XUIButton_Left )
	{
		if ( !m_pCarousel->IsFirstItemFocussed() )
			PlayFocusSound( m_pLeft );

		TBOOL bScrolled = m_pCarousel->ScrollLeft();
		( bScrolled ? m_pRight : m_pLeft )->SetVisible( bScrolled );
		m_pTitle->SetText( m_pCarousel->GetFocussedItemText(), -1 );

		m_pRight->SetFocus( TFALSE, TFALSE, TTRUE );
		m_pLeft->SetFocus( TTRUE, TFALSE, TTRUE );
		m_pSelect->SetFocus( TFALSE, TFALSE, TTRUE );
		pFocus = m_pLeft;
	}
	else if ( a_eButton == XUIButton_Right )
	{
		if ( !m_pCarousel->IsLastItemFocussed() )
			PlayFocusSound( m_pRight );

		TBOOL bScrolled = m_pCarousel->ScrollRight();
		( bScrolled ? m_pLeft : m_pRight )->SetVisible( bScrolled );
		m_pTitle->SetText( m_pCarousel->GetFocussedItemText(), -1 );

		m_pRight->SetFocus( TTRUE, TFALSE, TTRUE );
		m_pLeft->SetFocus( TFALSE, TFALSE, TTRUE );
		m_pSelect->SetFocus( TFALSE, TFALSE, TTRUE );
		pFocus = m_pRight;
	}
	else
	{
		if ( a_eButton == XUIButton_Up )
		{
			m_pRight->SetFocus( TFALSE, TFALSE, TTRUE );
			m_pLeft->SetFocus( TFALSE, TFALSE, TTRUE );
			m_pSelect->SetFocus( TTRUE, TFALSE, TTRUE );
			FindParentScene()->SetFocus( m_pSelect, a_iController );
		}

		return TXUIScene::HandleButton( a_eButton, a_iController );
	}

	FindParentScene()->SetFocus( pFocus, a_iController );
	return TTRUE;
}

// $deBlob: FUNCTION 006315e0
void ABlobGalleries::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransExtrasAExtrasB, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 00631650
void ABlobGalleries::TransBackTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->ShowAfterGallery();
	pFrontEnd->SlideDown();

	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 00631680
void ABlobGalleries::TransFrom()
{
	TSTATICCAST( ABlobFrontEnd, FindParentScene() )->SlideUp();
	ABlobMenu::TransFrom();
}

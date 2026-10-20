#include "pch.h"
#include "ABlobCutscenes.h"
#include "ABlobFrontEnd.h"
#include "XUI/ElementTypes/ABlobCarouselList.h"
#include "GameInterface/APlayerProfileManager.h"
#include "GUI/AGUIManager.h"
#include "GUI/AGUICursor.h"
#include "Movie/AMoviePlayer.h"
#include "Sound/AMusicManager.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIInput.h>
#include <XUI/TXUICanvas.h>
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

// $deBlob: FUNCTION 0062ef50
// $deBlob: FUNCTION 0062ef60
// $deBlob: FUNCTION 0062efd0
// $deBlob: FUNCTION 0062f040
// $deBlob: FUNCTION 0062f050
// $deBlob: FUNCTION 0053d350
// $deBlob: FUNCTION 00858850
TDEFINE_CLASS( ABlobCutscenes );

const TCHAR* ABlobCutscenes::sm_aszCutscenes[ NUM_CUTSCENES ] = {
	"00_ChromaCity",
	"01_MeetProf",
	"02_GrayEsc",
	"03_NewsBridge",
	"05_TANK",
	"04_JetBike",
	"06_EliteInky",
	"07_InkTVJaming",
	"08_NewNeedsInk",
	"09_InkyBunker",
	"10_Spaceship",
	"11_Triumph",
};

// $deBlob: FUNCTION 0062f060
ABlobCutscenes::ABlobCutscenes()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopExtrasB )
{
	m_bMoviePlaying = TFALSE;
	m_pCarousel     = TNULL;
	m_pSelect       = TNULL;
	m_pLeft         = TNULL;
	m_pRight        = TNULL;
	m_pTitle        = TNULL;
}

// $deBlob: FUNCTION 0062fa20
// $deBlob: FUNCTION 0062fa40
// $deBlob: FUNCTION 0062fa32
ABlobCutscenes::~ABlobCutscenes()
{
}

// $deBlob: FUNCTION 0062f0c0
void ABlobCutscenes::OnCreated()
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

// $deBlob: FUNCTION 0062f1c0
TBOOL ABlobCutscenes::HandleButton( TINT a_eButton, TINT a_iController )
{

	if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() == TTRUE && a_eButton == XUIButton_Back )
	{
		// Back stops the cutscene
		AMoviePlayer::GetSingleton()->StopMovie();
		TXUI::GetSingleton()->GetCanvas()->SetVisible( TTRUE );
		TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
		m_bMoviePlaying = TFALSE;
		AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.0f );

		if ( m_iBackgroundMovie != -1 )
			TSTATICCAST( ABlobFrontEnd, FindParentScene() )->PlayBackgroundMovie( m_iBackgroundMovie, ABlobFrontEnd::BGMOVIE_None, TTRUE, TTRUE, TFALSE );

		TXUI::GetSingleton()->GetInput()->SetCursorVisible( TTRUE );
		return TTRUE;
	}

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
		// NOTE: retail plays the left arrow's focus sound here too
		if ( !m_pCarousel->IsLastItemFocussed() )
			PlayFocusSound( m_pLeft );

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

// $deBlob: FUNCTION 0062f460
void ABlobCutscenes::Tick( TFLOAT a_fDeltaTime )
{

	if ( TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransBackFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() && m_bMoviePlaying )
	{

		if ( !AMoviePlayer::GetSingleton()->IsMoviePlaying() )
		{
			// The cutscene is over: the menu is back
			if ( !TXUI::GetSingleton()->GetCanvas()->IsVisible() )
			{
				ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
				SetVisible( TTRUE );
				pFrontEnd->SetCursorTrailsVisible( TTRUE );
				TXUI::GetSingleton()->GetCanvas()->SetVisible( TTRUE );
				AGUIManager::GetSingleton()->GetCursor()->SetVisible( TTRUE, TTRUE );
				TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );

				if ( m_iBackgroundMovie != -1 )
					pFrontEnd->PlayBackgroundMovie( m_iBackgroundMovie, ABlobFrontEnd::BGMOVIE_None, TTRUE, TTRUE, TFALSE );

				TXUI::GetSingleton()->GetInput()->SetCursorVisible( TTRUE );
				m_bMoviePlaying = TFALSE;
				AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.0f );
			}
		}
		else
		{
			// Escape, or A/B/Start of a controller (A only for the Xbox/PlayStation pads), skips the cutscene
			TBOOL bSkip  = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 )->IsDown( TInputDeviceKeyboard::KEY_ESCAPE );
			TINT  iCount = TINT( TInputInterface::GetSingleton()->GetDeviceList().Count() );

			for ( TINT i = 0; !bSkip && i < iCount; i++ )
			{
				TInputDevice* pController = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceController>( i );

				if ( pController != TNULL && pController->IsEnabled() &&
				     ( ( TInputDevice::GetLastGamepadType() != TInputDevice::Gamepad_Wii && pController->IsDown( TInputDeviceController::BUTTON_CONTEXT_1 ) ) || pController->IsDown( TInputDeviceController::BUTTON_CONTEXT_2 ) || pController->IsDown( TInputDeviceController::BUTTON_START ) ) )
				{
					bSkip = TTRUE;
				}
			}

			if ( bSkip )
			{
				ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
				AMoviePlayer::GetSingleton()->StopMovie();
				SetVisible( TTRUE );
				pFrontEnd->SetCursorTrailsVisible( TTRUE );
				TXUI::GetSingleton()->GetCanvas()->SetVisible( TTRUE );
				TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );

				if ( m_iBackgroundMovie != -1 )
					pFrontEnd->PlayBackgroundMovie( m_iBackgroundMovie, ABlobFrontEnd::BGMOVIE_None, TTRUE, TTRUE, TFALSE );

				m_bMoviePlaying = TFALSE;
				AGUIManager::GetSingleton()->GetCursor()->SetVisible( TTRUE, TTRUE );
				AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.0f );
				TXUI::GetSingleton()->GetInput()->SetCursorVisible( TTRUE );
			}
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0062f700
TBOOL ABlobCutscenes::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
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
		if ( a_pSource == m_pSelect )
		{
			APlayerProfile* pProfile  = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
			TINT            iCutscene = m_pCarousel->GetFocussedItemIndex();

			if ( ( pProfile->GetData()->m_iUnlockedMovies & ( 1 << iCutscene ) ) == 0 )
			{
				// Locked
				m_pCarousel->PulseLocked();
				return TTRUE;
			}

			TASSERT( m_pFrontEnd->IsA( &TGetClass( ABlobFrontEnd ) ) );
			ABlobFrontEnd* pFrontEnd = m_pFrontEnd;
			pFrontEnd->StopBackgroundMovie();
			AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.0f );

			if ( AMoviePlayer::GetSingleton()->PlayMovie( sm_aszCutscenes[ iCutscene ], 0, AMoviePlayer::PlayFlags_None ) )
			{
				TXUI::GetSingleton()->GetInput()->SetCursorVisible( TFALSE );
				TXUI::GetSingleton()->GetCanvas()->SetVisible( TFALSE );
				TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
				SetVisible( TFALSE );
				pFrontEnd->SetCursorTrailsVisible( TFALSE );
				AGUIManager::GetSingleton()->GetCursor()->SetVisible( TFALSE, TTRUE );
				m_bMoviePlaying = TTRUE;
			}
		}

		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	pArrow->SetVisible( bVisible );
	m_pTitle->SetText( m_pCarousel->GetFocussedItemText(), -1 );

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}

// $deBlob: FUNCTION 0062f9b0
void ABlobCutscenes::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransExtrasAExtrasB, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );

	TXUIScene::TransTo();
}

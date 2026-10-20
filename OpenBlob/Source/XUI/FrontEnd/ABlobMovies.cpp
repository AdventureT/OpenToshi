#include "pch.h"
#include "ABlobMovies.h"
#include "ABlobFrontEnd.h"
#include "XUI/ElementTypes/ABlobCarouselList.h"
#include "GameInterface/AGameStateController.h"
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
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The offset of the select button from the carousel (shared with ABlobLevelSelection)
static const TFLOAT s_fSelectOffsetX = 51.0f;
static const TFLOAT s_fSelectOffsetY = 12.0f;

// The fade to black before a movie and the music's fade back
static const TFLOAT s_fFadeTime = 0.5f;

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

// $deBlob: FUNCTION 00637620
// $deBlob: FUNCTION 00637630
// $deBlob: FUNCTION 006376a0
// $deBlob: FUNCTION 00637710
// $deBlob: FUNCTION 00637720
// $deBlob: FUNCTION 0053d6d0
// $deBlob: FUNCTION 00858930
TDEFINE_CLASS( ABlobMovies );

const ABlobMovies::Movie ABlobMovies::sm_aMovies[ NUM_MOVIES ] = {
	{ "Blob_GamersDay_Final_M", 773 },
	{ "Intro_De_Blob_M", 775 },
	{ "Intro_M", 772 },
	{ "inky_wii_M", 851 },
	{ "zepphead", 849 },
	{ "paintribbon", 854 },
	{ "wakeywakey", 855 },
	{ "BlobTest_ZAttack_Cam2", 852 },
	{ "BlobTest_Grapple_Scenario", 853 },
	{ "Blob Slam Rear Cam", 856 },
};

// $deBlob: FUNCTION 00637730
ABlobMovies::ABlobMovies()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopExtrasB )
{
	m_bMoviePlaying = TFALSE;
	m_bStartMovie   = TFALSE;
	m_fMusicTime    = -1.0f;
	m_pCarousel     = TNULL;
	m_pSelect       = TNULL;
	m_pLeft         = TNULL;
	m_pRight        = TNULL;
	m_pTitle        = TNULL;
}

// $deBlob: FUNCTION 006381d0
// $deBlob: FUNCTION 006381f0
// $deBlob: FUNCTION 006381e2
ABlobMovies::~ABlobMovies()
{
}

// $deBlob: FUNCTION 00637790
void ABlobMovies::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pLeft     = FindChildXUIControl( 7 );
	m_pRight    = FindChildXUIControl( 6 );
	m_pCarousel = TSTATICCAST( ABlobCarouselList, FindChildXUIControl( 4 ) );
	m_pSelect   = FindChildXUIControl( 5 );
	m_pCarousel->SetDescription( m_pSelect );
	m_pSelect->SetPosition( s_fSelectOffsetX + m_pCarousel->GetPositionX(), s_fSelectOffsetY + m_pCarousel->GetPositionY() );

	m_pTitle = FindChildXUIControl( 8 );
	m_pTitle->SetText( m_pCarousel->GetFocussedItemText(), -1 );
}

// $deBlob: FUNCTION 00637880
TBOOL ABlobMovies::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( a_pSource != m_pBackButton && IsLastXUIDeviceKeyboardOrController() )
	{
		m_pRight->SetFocus( TFALSE, TFALSE, TTRUE );
		m_pLeft->SetFocus( TFALSE, TFALSE, TTRUE );
		m_pSelect->SetFocus( TTRUE, TFALSE, TTRUE );
		FindParentScene()->SetFocus( m_pSelect, a_iController );
	}

	if ( m_bStartMovie == TTRUE || AMoviePlayer::GetSingleton()->IsMoviePlaying() == TTRUE )
		return TTRUE;

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
			APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
			TINT            iMovie   = m_pCarousel->GetFocussedItemIndex();

			if ( ( pProfile->GetData()->m_iUnlockedVideos & ( 1 << iMovie ) ) == 0 )
			{
				// Locked
				m_pCarousel->PulseLocked();
				return TTRUE;
			}

			TStringManager::String8Copy( m_szMovie, sm_aMovies[ iMovie ].m_szName, -1 );
			TINT iLength = TStringManager::String8Length( m_szMovie );

			if ( iLength > 2 )
			{
				// "_M": the music stays paused while the movie plays
				if ( m_szMovie[ iLength - 2 ] == '_' && m_szMovie[ iLength - 1 ] == 'M' )
				{
					m_szMovie[ iLength - 2 ] = '\0';
					m_fMusicTime             = 0.0f;
				}
				else
				{
					m_fMusicTime = -1.0f;
				}
			}

			// The movie starts in Tick once the screen is black
			AGameStateController::GetSingleton()->FadeToBlack( s_fFadeTime, 0 );
			m_bStartMovie = TTRUE;
			TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
			PlayPressSound( a_pSource );
			return TTRUE;
		}

		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	pArrow->SetVisible( bVisible );
	m_pTitle->SetText( m_pCarousel->GetFocussedItemText(), -1 );

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}

// $deBlob: FUNCTION 00637af0
TBOOL ABlobMovies::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( m_bStartMovie == TTRUE )
		return TTRUE;

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

// $deBlob: FUNCTION 00637d00
void ABlobMovies::Tick( TFLOAT a_fDeltaTime )
{

	if ( TXUI::GetSingleton()->GetTransToScene() != TNULL || TXUI::GetSingleton()->GetTransBackFromScene() != TNULL || TXUI::GetSingleton()->IsInputLocked() )
	{
		TXUIScene::Tick( a_fDeltaTime );
		return;
	}


	if ( m_bStartMovie )
	{
		TFLOAT fMusicFade;

		if ( !AGameStateController::GetSingleton()->IsFading( 0 ) )
		{
			// The screen is black: the movie starts
			TASSERT( m_pFrontEnd->IsA( &TGetClass( ABlobFrontEnd ) ) );
			ABlobFrontEnd* pFrontEnd = m_pFrontEnd;
			pFrontEnd->StopBackgroundMovie();

			if ( !AMoviePlayer::GetSingleton()->PlayMovie( m_szMovie, 0, AMoviePlayer::PlayFlags_None ) )
				goto LAB_UpdateMovie;

			AGameStateController::GetSingleton()->SetFade( 0.0f, 0 );
			TXUI::GetSingleton()->GetInput()->SetCursorVisible( TFALSE );
			TXUI::GetSingleton()->GetCanvas()->SetVisible( TFALSE );
			SetVisible( TFALSE );
			pFrontEnd->SetCursorTrailsVisible( TFALSE );
			m_bMoviePlaying = TTRUE;
			m_bStartMovie   = TFALSE;
			AGUIManager::GetSingleton()->GetCursor()->SetVisible( TFALSE, TTRUE );

			if ( m_fMusicTime == -1.0f )
				goto LAB_UpdateMovie;

			fMusicFade = 0.0f;
		}
		else
		{
			if ( m_fMusicTime == -1.0f )
				goto LAB_UpdateMovie;

			m_fMusicTime += a_fDeltaTime;

			if ( !AMusicManager::GetSingleton()->IsMusicPaused() )
				goto LAB_UpdateMovie;

			fMusicFade = 1.0f;
		}

		AMusicManager::GetSingleton()->PauseMusic( TTRUE, fMusicFade );
	}

LAB_UpdateMovie:
	if ( !m_bMoviePlaying )
	{
		if ( AMusicManager::GetSingleton()->IsMusicPaused() )
			AMusicManager::GetSingleton()->PauseMusic( TFALSE, s_fFadeTime );
	}
	else if ( !AMoviePlayer::GetSingleton()->IsMoviePlaying() )
	{
		// The movie is over: the menu is back
		if ( !TXUI::GetSingleton()->GetCanvas()->IsVisible() )
		{
			ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
			SetVisible( TTRUE );
			pFrontEnd->SetCursorTrailsVisible( TTRUE );
			TXUI::GetSingleton()->GetCanvas()->SetVisible( TTRUE );

			if ( m_iBackgroundMovie != -1 )
				pFrontEnd->PlayBackgroundMovie( m_iBackgroundMovie, ABlobFrontEnd::BGMOVIE_None, TTRUE, TTRUE, TFALSE );

			TXUI::GetSingleton()->GetInput()->SetCursorVisible( TTRUE );
			m_bMoviePlaying = TFALSE;
			AGUIManager::GetSingleton()->GetCursor()->SetVisible( TTRUE, TTRUE );
			AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.0f );
			m_fMusicTime  = -1.0f;
			m_bStartMovie = TFALSE;
			TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
		}
	}
	else
	{
		// Escape, or A/B/Start of a controller (A only for the Xbox/PlayStation pads), skips the movie
		TBOOL bSkip  = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 )->IsDown( TInputDeviceKeyboard::KEY_ESCAPE );
		TINT  iCount = TINT( TInputInterface::GetSingleton()->GetDeviceList().Count() );

		for ( TINT i = 0; i < iCount; i++ )
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
			AMoviePlayer::GetSingleton()->StopMovie();

			ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
			SetVisible( TTRUE );
			pFrontEnd->SetCursorTrailsVisible( TTRUE );
			TXUI::GetSingleton()->GetCanvas()->SetVisible( TTRUE );

			if ( m_iBackgroundMovie != -1 )
				pFrontEnd->PlayBackgroundMovie( m_iBackgroundMovie, ABlobFrontEnd::BGMOVIE_None, TTRUE, TTRUE, TFALSE );

			m_bMoviePlaying = TFALSE;
			m_bStartMovie   = TFALSE;
			AGUIManager::GetSingleton()->GetCursor()->SetVisible( TTRUE, TTRUE );
			AMusicManager::GetSingleton()->PauseMusic( TFALSE, s_fFadeTime );
			m_fMusicTime = -1.0f;
			TXUI::GetSingleton()->GetInput()->SetCursorVisible( TTRUE );
			TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00638160
void ABlobMovies::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransExtrasAExtrasB, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );

	TXUIScene::TransTo();
}

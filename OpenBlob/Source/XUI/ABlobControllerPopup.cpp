#include "pch.h"
#include "ABlobControllerPopup.h"
#include "GameInterface/APlayerManager.h"
#include <Input/TInputDeviceKeyboard.h>
#include "GameInterface/AGameStateController.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundLoadHelper.h"
#include "GameController/AGameStatus.h"
#include "Input/ABlobInputMap.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "GameInterface/AInGameState.h"
#include "GUI/AGUIManager.h"
#include "GUI/AGUICursor.h"
#include "Movie/AMoviePlayer.h"
#include "Sound/AMusicManager.h"

#include <XUI/TXUI.h>
#include <XUI/TXUICanvas.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00626430
// $deBlob: FUNCTION 00626440
// $deBlob: FUNCTION 006264b0
// $deBlob: FUNCTION 00626520
// $deBlob: FUNCTION 00626530
// $deBlob: FUNCTION 0053d050
// $deBlob: FUNCTION 00858790
TDEFINE_CLASS( ABlobControllerPopup );

// $deBlob: FUNCTION 00626540
ABlobControllerPopup::ABlobControllerPopup()
{
	m_eState            = STATE_Hidden;
	m_iPlayer           = -1;
	m_iUnk138           = -1;
	m_bWasInGamePaused  = TFALSE;
	m_bDisabled         = TFALSE;
	m_iNumHiddenGUIs    = 0;
	m_bFadeWasEnabled   = TFALSE;
	m_fFade             = 0.0f;
	m_bPausedMovie      = TFALSE;
	m_bCanvasWasVisible = TFALSE;
	m_bIgnoreInputLock  = TFALSE;

	for ( TINT i = 0; i < 4; i++ )
	{
		m_aPlayers[ i ].m_bHasDevice = TFALSE;
		m_aPlayers[ i ].m_iUnk4      = 0;
	}
}

// $deBlob: FUNCTION 00626e70
// $deBlob: FUNCTION 00626e65
ABlobControllerPopup::~ABlobControllerPopup()
{
}

// $deBlob: FUNCTION 006265f0
void ABlobControllerPopup::Tick( TFLOAT a_fDeltaTime )
{
	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00626610
void ABlobControllerPopup::OnCreated()
{
	PlayAnimation( L"Normal", L"EndNormal" );
}

// $deBlob: FUNCTION 00626620
void ABlobControllerPopup::TransTo()
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	// Pauses the GUI sounds
	if ( ASoundLoadHelper::GetSingleton() != TNULL && ASoundLoadHelper::GetSingleton()->IsGUILoaded() )
		ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID_GUI, TTRUE );

	// The canvas is shown while the popup is
	m_bCanvasWasVisible = TFALSE;
	TXUI* pXUI          = TXUI::GetSingleton();

	if ( pXUI != TNULL && pXUI->GetCanvas() != TNULL )
	{
		m_bCanvasWasVisible = pXUI->GetCanvas()->IsVisible();
		pXUI->GetCanvas()->SetVisible( TTRUE );
	}

	// No fade while the popup is shown
	if ( pController != TNULL )
	{
		m_bFadeWasEnabled = pController->IsFadeEnabled( 0 );
		m_fFade           = pController->GetFade( 0 );
		pController->SetFade( 0.0f, 0 );
	}

	m_bPausedMovie = TFALSE;

	// The movie is paused
	AMoviePlayer* pMovie = AMoviePlayer::GetSingleton();

	if ( pMovie != TNULL && pMovie->IsMoviePlaying() && !pMovie->IsMoviePaused() )
	{
		pMovie->PauseMovie( TTRUE );
		m_bPausedMovie = TTRUE;
	}

	if ( !IsLinked() )
		TXUI::GetSingleton()->GetCanvas()->AddScene( this );

	m_eState = STATE_TransTo;

	// The level is paused (its sounds too) and the cursor is hidden
	if ( pController != TNULL )
	{
		AGameState* pState = pController->GetCurrentGameState();

		if ( pState->GetClass() == &TGetClass( AInGameState ) )
		{
			AInGameState* pInGameState = TSTATICCAST( AInGameState, pState );
			m_bWasInGamePaused         = pInGameState->IsPaused();

			if ( !m_bWasInGamePaused )
			{
				pInGameState->SetPaused( TTRUE );
				AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.0f );
				ASoundInterface::SetCategoryPausedAll( TTRUE );
			}
		}
	}

	if ( AGUIManager::GetSingleton() != TNULL )
	{
		// NOTE: retail reads the first cursor four times
		for ( TINT i = 4; i != 0; i-- )
		{
			AGUICursor* pCursor = AGUIManager::GetSingleton()->GetCursor();

			if ( pCursor != TNULL )
			{
				if ( pCursor->IsVisible() )
					m_iNumHiddenGUIs++;

				pCursor->SetVisible( TFALSE, TFALSE );
			}
		}
	}

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 006267c0
void ABlobControllerPopup::EndTransTo()
{
	m_eState = STATE_Shown;
	TXUIScene::EndTransTo();
}

// $deBlob: FUNCTION 006267d0
void ABlobControllerPopup::TransBackFrom()
{
	// Resumes the GUI sounds
	if ( ASoundLoadHelper::GetSingleton() != TNULL && ASoundLoadHelper::GetSingleton()->IsGUILoaded() )
		ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID_GUI, TFALSE );

	m_eState = STATE_TransBackFrom;
	TXUIScene::TransBackFrom();
}

// $deBlob: FUNCTION 00626800
void ABlobControllerPopup::EndTransBackFrom()
{
	m_eState = STATE_Hidden;

	AGameStateController* pController = AGameStateController::GetSingleton();
	TXUI*                 pXUI        = TXUI::GetSingleton();

	if ( pXUI != TNULL && pXUI->GetCanvas() != TNULL )
		pXUI->GetCanvas()->SetVisible( m_bCanvasWasVisible );

	m_bCanvasWasVisible = TFALSE;

	// The cursor is shown again
	if ( AGUIManager::GetSingleton() != TNULL )
	{
		// NOTE: retail sets the first cursor four times, the last time hidden unless it hid four
		for ( TINT i = 0; i < 4; i++ )
		{
			TBOOL bVisible = i < m_iNumHiddenGUIs;
			AGUIManager::GetSingleton()->GetCursor()->SetVisible( bVisible, bVisible );
		}

		m_iNumHiddenGUIs = 0;
	}

	if ( pController != TNULL )
	{
		// The level is resumed, the popup leaves the canvas
		AGameState* pState = pController->GetCurrentGameState();

		if ( pState->GetClass() == &TGetClass( AInGameState ) )
		{
			AInGameState* pInGameState = TSTATICCAST( AInGameState, pState );

			if ( !m_bWasInGamePaused && pInGameState->IsPaused() )
			{
				pInGameState->SetPaused( TFALSE );
				AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.0f );
				ASoundInterface::SetCategoryPausedAll( TFALSE );
			}

			RemoveSelf();
		}


		pController->SetFade( m_fFade, 0 );
		pController->EnableFade( m_bFadeWasEnabled, 0 );
	}

	// The movie goes on
	AMoviePlayer* pMovie = AMoviePlayer::GetSingleton();

	if ( m_bPausedMovie && pMovie != TNULL && pMovie->IsMoviePlaying() && pMovie->IsMoviePaused() )
		pMovie->PauseMovie( TFALSE );


	m_bPausedMovie = TFALSE;
	TXUIScene::EndTransBackFrom();
}

TBOOL ABlobControllerPopup::CanTransition() const
{
	TXUI* pXUI = TXUI::GetSingleton();
	return pXUI->GetTransToScene() == TNULL && pXUI->GetTransBackFromScene() == TNULL && ( m_bIgnoreInputLock || !pXUI->IsInputLocked() );
}

// $deBlob: FUNCTION 00626980
TBOOL ABlobControllerPopup::UpdateInGame( TFLOAT a_fDeltaTime )
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( pState->GetClass() == &TGetClass( AInGameState ) );

	// The players of the level: the popup is shown for the first one without a device ("Message": string 0x7F8 with the
	// player's number, or 0x7F7 in a single player game)
	TINT iPlayer = -1;

	if ( !m_bDisabled )
	{
		for ( TINT i = 0; i < APlayerManager::GetSingleton()->GetNumPlayers(); i++ )
		{
			TINT          iController = APlayerManager::GetSingleton()->GetController( TUINT8( i ) );
			TInputDevice* pDevice     = TInputDevice::GetPlayerDevice( i );
			TInputDevice* pKeyboard   = TInputInterface::GetSingleton()->GetDeviceByIndex( &TGetClass( TInputDeviceKeyboard ), 0 );

			TINT iKeyboardPlayer;

			for ( iKeyboardPlayer = 0; iKeyboardPlayer < 14; iKeyboardPlayer++ )
			{
				if ( TInputDevice::GetPlayerDeviceIndex( iKeyboardPlayer ) == pKeyboard->GetDeviceIndex() )
					break;
			}

			if ( iKeyboardPlayer >= 14 )
				iKeyboardPlayer = -1;

			TBOOL bEnabled = pDevice != TNULL && pDevice->IsEnabled();

			if ( iKeyboardPlayer == i || bEnabled || APlayerManager::GetSingleton()->GetNumPlayers() < 2 )
			{
				m_aPlayers[ iController ].m_bHasDevice = TTRUE;
			}
			else
			{
				if ( iPlayer < 0 )
					iPlayer = iController;

				m_aPlayers[ iController ].m_bHasDevice = TFALSE;
			}
		}

		if ( iPlayer >= 0 )
		{
			TBOOL bSinglePlayer = AGameStatus::GetSingleton()->GetMultiPlayerGameType() == -1;

			if ( CanTransition() && ( m_eState == STATE_TransBackFrom || m_eState == STATE_Hidden ) )
			{
				ABlobInputMap::GetSingleton()->ClearPlayers();

				if ( bSinglePlayer && TXUI::GetSingleton()->GetActiveScene() != TNULL && TXUI::GetSingleton()->GetActiveScene()->IsA( &TGetClass( ABlobHud ) ) )
					TXUI::GetSingleton()->TransitionTo( TNULL, this );
				else
					TXUI::GetSingleton()->TransitionToPopup( TNULL, this );
			}

			TXUIControl* pMessage = FindChildXUIControl( L"Message" );

			if ( !bSinglePlayer )
			{
				TStringManager::String16Format( m_wszMessage, 0x80, T2Locale::GetSingleton()->GetString( 0x7F8 ), iPlayer + 1 );
				pMessage->SetText( m_wszMessage, -1 );
			}
			else
			{
				pMessage->SetText( T2Locale::GetSingleton()->GetString( 0x7F7 ), -1 );
			}

			m_iPlayer = iPlayer;
			return TFALSE;
		}
	}

	if ( CanTransition() && ( m_eState == STATE_TransTo || m_eState == STATE_Shown ) )
	{
		m_iPlayer = -1;
		m_iUnk138 = -1;
		TXUI::GetSingleton()->TransitionBack( this );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00626c40
TBOOL ABlobControllerPopup::UpdateFrontEnd( TFLOAT a_fDeltaTime )
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( pState->GetClass() != &TGetClass( AInGameState ) );

	TINT iPlayer = -1;

	if ( !m_bDisabled )
	{
		// The players of the menus (at least one): the first one without a device (whose device isn't the keyboard
		// or an enabled one, with 2+ players)
		TINT iNumPlayers = APlayerManager::GetSingleton()->GetNumPlayers();

		if ( iNumPlayers < 1 )
			iNumPlayers = 1;

		for ( TINT i = 0; i < iNumPlayers; i++ )
		{
			TInputDevice* pDevice   = TInputDevice::GetPlayerDevice( i );
			TInputDevice* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex( &TGetClass( TInputDeviceKeyboard ), 0 );

			TINT iKeyboardPlayer;

			for ( iKeyboardPlayer = 0; iKeyboardPlayer < 14; iKeyboardPlayer++ )
			{
				if ( TInputDevice::GetPlayerDeviceIndex( iKeyboardPlayer ) == pKeyboard->GetDeviceIndex() )
					break;
			}

			if ( iKeyboardPlayer >= 14 )
				iKeyboardPlayer = -1;

			TBOOL bEnabled = pDevice != TNULL && pDevice->IsEnabled();

			if ( iKeyboardPlayer == i || bEnabled || iNumPlayers < 2 )
			{
				m_aPlayers[ i ].m_bHasDevice = TTRUE;
			}
			else
			{
				m_aPlayers[ i ].m_bHasDevice = TFALSE;

				if ( iPlayer < 0 )
					iPlayer = i;
			}
		}
	}

	if ( iPlayer >= 0 )
	{
		if ( CanTransition() && ( m_eState == STATE_TransBackFrom || m_eState == STATE_Hidden ) )
			TXUI::GetSingleton()->TransitionToPopup( TNULL, this );

		FindChildXUIControl( L"Message" )->SetText( T2Locale::GetSingleton()->GetString( 0x7F7 ), -1 );
		m_iPlayer = iPlayer;
		return TFALSE;
	}

	if ( CanTransition() && ( m_eState == STATE_TransTo || m_eState == STATE_Shown ) )
	{
		m_iPlayer = -1;
		m_iUnk138 = -1;
		TXUI::GetSingleton()->TransitionBack( this );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00626e40
void ABlobControllerPopup::SetDisabled( TBOOL a_bDisabled )
{
	m_bDisabled = a_bDisabled;
}

// $deBlob: FUNCTION 00626e50
void ABlobControllerPopup::SetIgnoreInputLock( TBOOL a_bIgnore )
{
	m_bIgnoreInputLock = a_bIgnore;
}

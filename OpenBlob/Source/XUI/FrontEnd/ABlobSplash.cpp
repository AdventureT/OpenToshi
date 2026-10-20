#include "pch.h"
#include "ABlobSplash.h"
#include "XUI/ABlobSaveLoadConfirm.h"
#include "XUI/ABlobCursorTrail.h"
#include "GUI/AGUIManager.h"
#include "ABlobFrontEnd.h"
#include "XUI/ABlobConfirmationMenu.h"
#include "AApplication.h"
#include "GameInterface/APlayerProfileManager.h"
#include "MemoryCard/SaveLoad.h"
#include "AAchievements.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIBackButton.h>
#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <Platform/Windows/Input/TInputDeviceController_Wiin.h>
#include "Movie/AMoviePlayer.h"
#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TINT  ABlobSplash::s_iSaveSlot          = 0;
TBOOL ABlobSplash::s_bNunchukPopupShown = TFALSE;

// $deBlob: FUNCTION 0063a860
// $deBlob: FUNCTION 0063a870
// $deBlob: FUNCTION 0063a8e0
// $deBlob: FUNCTION 0063a950
// $deBlob: FUNCTION 0063a960
// $deBlob: FUNCTION 0053d7d0
// $deBlob: FUNCTION 00858970
TDEFINE_CLASS( ABlobSplash );

// The name of the new profile
static const TWCHAR s_wszDefaultProfileName[] = L"DeBlobSave";

// $deBlob: FUNCTION 0063a970
ABlobSplash::ABlobSplash()
    : ABlobMenu( -1 )
{
	m_bStartPressed   = TFALSE;
	m_bReady          = TFALSE;
	m_pElement1       = TNULL;
	m_pElement2       = TNULL;
	m_fStartTimer     = -1.0f;
	m_uiUnk150        = 0;
	m_fUnk154         = -1.0f;
	m_fIncorrectTimer = 0.0f;
	m_bShown          = TFALSE;
	m_bUnk15D         = TFALSE;
	m_bSaving         = TFALSE;
	m_bProfileCreated = TFALSE;
	m_bPopup          = TFALSE;
	m_pPopup          = TNULL;
}

// $deBlob: FUNCTION 0063b5c0
// $deBlob: FUNCTION 0063aa40
// $deBlob: FUNCTION 0063b5ad
ABlobSplash::~ABlobSplash()
{
}

// $deBlob: FUNCTION 0063aa60
void ABlobSplash::Tick( TFLOAT a_fDeltaTime )
{

	if ( TXUI::GetSingleton()->GetTransToScene() != TNULL || TXUI::GetSingleton()->GetTransBackFromScene() != TNULL || TXUI::GetSingleton()->IsInputLocked() )
	{
		TXUIScene::Tick( a_fDeltaTime );
		return;
	}

	if ( m_bSaving )
	{
		SaveLoad::GetInstance()->Update( a_fDeltaTime );

		if ( SaveLoad::GetInstance()->GetError() != SaveLoad::ERROR_NONE )
			OnProfileSaved( SaveLoad::GetInstance()->GetError() );
	}

	if ( m_bProfileCreated )
		m_bStartPressed = TTRUE;

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	if ( ( !pFrontEnd->IsBackgroundMoviePlaying() || pFrontEnd->GetBackgroundMovie() != 1 ) &&
	     ( !pFrontEnd->IsBackgroundMoviePlaying() || pFrontEnd->GetBackgroundMovie() != 0 ) &&
	     !m_bAttractMovie )
	{
		pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_LoopTitle, ABlobFrontEnd::BGMOVIE_None, TTRUE, TFALSE, TFALSE );
		PlayAnimation( L"Show", L"EndShow" );
		m_bShown = TTRUE;
	}

	if ( m_bShown && !m_oTimelineControl.IsPlaying() )
		m_bReady = TTRUE;

	TUINT32 uiState = GetTimelineState();

	if ( !m_bStartPressed && m_bReady == TTRUE && uiState != State_TransTo && uiState != State_TransFrom && uiState != State_TransBackFrom && uiState != State_TransBackTo && !m_bAttractMovie )
	{

		// The nunchuk of the Wii remotes
		TINT iNumControllers = TINT( TInputInterface::GetSingleton()->GetDeviceList().Count() );

		for ( TINT i = 0; i < iNumControllers; i++ )
		{
			TInputDeviceController* pController = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceController>( i );

			if ( pController != TNULL && pController->IsEnabled() && !s_bNunchukPopupShown )
			{
				if ( pController->GetClass()->IsA( &TGetClass( TInputWiinDeviceController ) ) && !IsNunchukConnected( pController ) )
				{
					m_bPopup                = TTRUE;
					s_bNunchukPopupShown    = TTRUE;
					const TWCHAR* wszText   = T2Locale::GetSingleton()->GetString( 2057 );
					const TWCHAR* wszButton = T2Locale::GetSingleton()->GetString( 3 );

					m_pPopup = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "saveloadconfirm2.xur" ), TUINT32( 0 ) );

					TXUIElement* pOk = m_pPopup->FindChildXUIElement( L"Ok" );

					if ( pOk != TNULL )
					{
						pOk->SetScale( 0.0f, 0.0f );
					}

					TXUIControl* pConfirm = m_pPopup->FindChildXUIControl( L"Confirm" );
					pConfirm->SetText( wszText, -1 );

					TSTATICCAST( ABlobSaveLoadConfirm, pConfirm )->SetParentMenu( this );

					m_pPopup->FindChildXUIControl( L"Ok" )->SetText( wszButton, -1 );
					TXUI::GetSingleton()->TransitionToPopup( TNULL, m_pPopup );
				}
			}
		}

		// Any key
		TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
		TBOOL                 bKeyDown  = TFALSE;

		for ( TINT iKey = TInputDeviceKeyboard::KEY_ESCAPE; iKey <= TInputDeviceKeyboard::KEY_F15; iKey++ )
		{
			if ( pKeyboard->IsDown( iKey ) )
			{
				bKeyDown = TTRUE;
				break;
			}
		}

		TInputDevice* pDevice = TNULL;

		if ( bKeyDown )
		{
			m_bStartPressed = TTRUE;
			m_fStartTimer   = 0.0f;

			// NOTE: retail doesn't check the first controller for TNULL
			pDevice = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceController>( 0 );

			if ( !pDevice->IsEnabled() )
				pDevice = TNULL;
		}
		else
		{
			// Any of the buttons A, B, X, Y, Start, Back of a controller
			TINT iNumDevices = TINT( TInputInterface::GetSingleton()->GetDeviceList().Count() );

			for ( TINT i = 0; i < iNumDevices; i++ )
			{
				TInputDevice* pController = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceController>( i );

				if ( pController != TNULL && pController->IsEnabled() &&
				     ( pController->IsDown( TInputDeviceController::BUTTON_CONTEXT_1 ) || pController->IsDown( TInputDeviceController::BUTTON_CONTEXT_2 ) || pController->IsDown( TInputDeviceController::BUTTON_CONTEXT_3 ) ||
				       pController->IsDown( TInputDeviceController::BUTTON_CONTEXT_4 ) || pController->IsDown( TInputDeviceController::BUTTON_SELECT ) || pController->IsDown( TInputDeviceController::BUTTON_START ) ) )
				{
					m_bStartPressed = TTRUE;
					m_fStartTimer   = 0.0f;
					pDevice         = pController;
					break;
				}
			}
		}

		if ( pDevice != TNULL )
			TInputDevice::SetPlayerDevice( 0, pDevice );
	}

	if ( m_bSaving )
		m_bStartPressed = TFALSE;

	if ( m_bStartPressed == TTRUE )
	{
		m_fStartTimer -= a_fDeltaTime;

		if ( m_fStartTimer <= 0.0f )
		{
			APlayerProfileManager::GetSingleton()->SetCurrentProfile( s_iSaveSlot );
			g_oTheApp.SetCurrentSaveSlot( s_iSaveSlot );

			if ( APlayerProfileManager::GetSingleton()->IsProfileUsed( s_iSaveSlot ) )
			{
				AAchievements::GetSingleton()->Reset();

				TXUIScene* pActiveScene = TXUI::GetSingleton()->GetActiveScene();
				TASSERT( pActiveScene->IsA( &TGetClass( ABlobFrontEnd ) ) );
				TSTATICCAST( ABlobFrontEnd, pActiveScene )->SetProfileName( APlayerProfileManager::GetSingleton()->GetCurrentProfileName() );

				TXUIScene* pMainMenu = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "mainmenu.xur" ), TUINT32( 0 ) );

				ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
				pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransEnterNameMainMenu, ABlobFrontEnd::BGMOVIE_LoopMainMenu, TFALSE, TFALSE, TFALSE );
				m_bSaving         = TFALSE;
				m_bProfileCreated = TFALSE;

				TXUIScene* pParent = FindParentScene();

				if ( pParent != TNULL )
				{
					pMainMenu->RemoveSelf();
					pParent->AddChildTail( pMainMenu );
				}

				m_iTransitionIndex = 1;
				m_bStartPressed    = TFALSE;
				m_bReady           = TFALSE;

				ABlobCursorTrail::ResetAll();

				TXUI::GetSingleton()->TransitionTo( this, pMainMenu );
			}
			else
			{
				m_bStartPressed = TFALSE;
				CreateProfile();
			}
		}
	}
	else
	{
		m_fIncorrectTimer += a_fDeltaTime;

		if ( m_fIncorrectTimer > 5.0f )
		{
			m_fIncorrectTimer = 0.0f;
			m_pElement2->PlayAnimation( L"Incorrect", L"EndIncorrect" );
		}
	}

	UpdateAttractMode( a_fDeltaTime );
	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0063b0f0
void ABlobSplash::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pElement1 = FindChildXUIElement( 1 );
	m_pElement2 = FindChildXUIElement( 2 );
	m_pLabel    = FindChildXUIElement( L"XuiLabel1" );

	if ( m_pLabel != TNULL )
	{
		swprintf( m_wszLabel, 256, T2Locale::GetSingleton()->GetString( 2031 ), 128 );
		TSTATICCAST( TXUIControl, m_pLabel )->SetText( m_wszLabel, -1 );
		m_pLabel->SetScale( 2.0f, 2.0f );
	}
}

// $deBlob: FUNCTION 0063b220
TBOOL ABlobSplash::HandleButton( TINT a_eButton, TINT a_iController )
{

	if ( TXUI::GetSingleton()->GetTransToScene() != TNULL || TXUI::GetSingleton()->GetTransBackFromScene() != TNULL || TXUI::GetSingleton()->IsInputLocked() )
		return TFALSE;

	if ( a_eButton == XUIButton_Back )
	{
		// Quit the game?
		ABlobConfirmationMenu* pConfirmation = TSTATICCAST( ABlobConfirmationMenu, TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "confirmation.xur" ), L"Confirmation", TFALSE ) );

		TXUIScene* pParent = FindParentScene();

		if ( pParent != TNULL )
		{
			pConfirmation->RemoveSelf();
			pParent->AddChildTail( pConfirmation );
		}

		pConfirmation->SetParentMenu( this );
		pConfirmation->SetTitle( T2Locale::GetSingleton()->GetString( 75 ) );
		pConfirmation->SetText( T2Locale::GetSingleton()->GetString( 196 ) );

		TXUI::GetSingleton()->TransitionToPopup( this, pConfirmation );
	}

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 0063b040
void ABlobSplash::TransTo()
{
	m_fIdleTime = 0.0f;

	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );
	AGUIManager::GetSingleton()->SetCursorActive( TTRUE );

	AMoviePlayer::GetSingleton()->StopMovie();

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_CutTitle, ABlobFrontEnd::BGMOVIE_LoopTitle, TFALSE, TFALSE, TFALSE );
	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 0063b180
void ABlobSplash::TransBackTo()
{
	m_bUnk15D   = TFALSE;
	m_fIdleTime = 0.0f;

	AMoviePlayer::GetSingleton()->StopMovie();

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_CutTitle, ABlobFrontEnd::BGMOVIE_LoopTitle, TFALSE, TFALSE, TFALSE );
	ResetAnimation();
	TXUIScene::TransBackTo();
}

// Deletes the scenes before the splash screen
// $deBlob: FUNCTION 0063b0a0
void ABlobSplash::EndTransTo()
{
	TXUIScene* pScene = m_pPrevScene;

	while ( pScene != TNULL )
	{
		TXUIScene* pDelete = pScene;
		pScene             = pScene->GetPrevScene();
		delete pDelete;
	}

	m_pPrevScene = TNULL;
	ABlobMenu::EndTransTo();
}

// $deBlob: FUNCTION 0063b1d0
void ABlobSplash::OnConfirmationClosed( TBOOL a_bYes )
{
	if ( m_bPopup )
	{
		TXUI::GetSingleton()->RemovePopup( m_pPopup );
		m_bPopup = TFALSE;
		return;
	}

	if ( a_bYes )
		g_oTheApp.Destroy();
}

// $deBlob: FUNCTION 0063b320
void ABlobSplash::OnProfileSaved( TINT a_iResult )
{
	TASSERT( TTRUE == m_bSaving );
	m_bSaving = TFALSE;

	if ( a_iResult == 1 )
	{
		TSTATICCAST( ABlobFrontEnd, TXUI::GetSingleton()->GetActiveScene() )->SetProfileName( m_wszID );
		APlayerProfileManager::GetSingleton()->SetCurrentProfileName( m_wszID );
		m_bProfileCreated = TTRUE;
		return;
	}

	if ( m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TTRUE );

	// The save failed: the slot is restored
	EnSaveData* pSaveData = TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData );
	TINT        iSlot     = g_oTheApp.m_iCurrentSaveSlot;
	TASSERT( iSlot >= 0 && iSlot < EnSaveData::SAVEGAME_SLOT_COUNT );
	TUtil::MemCopy( &pSaveData->GetSaveData()->m_aSaveGameSlots[ iSlot ], &m_oSlotBackup, sizeof( EnSaveData::SaveGameSlot ) );
}

// $deBlob: FUNCTION 0063b430
void ABlobSplash::CreateProfile()
{
	if ( m_bSaving )
		return;

	EnSaveData* pSaveData = TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData );
	TINT        iSlot     = s_iSaveSlot;
	TASSERT( iSlot >= 0 && iSlot < EnSaveData::SAVEGAME_SLOT_COUNT );

	EnSaveData::SaveGameSlot* pSlot = &pSaveData->GetSaveData()->m_aSaveGameSlots[ iSlot ];
	TASSERT( TFALSE == pSlot->m_bUsedSlot );

	TUtil::MemCopy( &m_oSlotBackup, pSlot, sizeof( EnSaveData::SaveGameSlot ) );
	pSlot->m_oData.Reset();
	pSlot->m_bUsedSlot = TTRUE;
	TStringManager::String16Copy( pSlot->m_oData.m_wszName, s_wszDefaultProfileName, 12 );

	SaveLoad::GetInstance()->SetCaller( this );
	m_bSaving = TTRUE;

	if ( m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TFALSE );

	// The save finishes in Tick (OnProfileSaved)
	if ( SaveLoad::GetInstance()->IsEnabled() )
	{
		SaveLoad::GetInstance()->SaveProfile( 0, 1 );
		return;
	}

	TASSERT( TTRUE == m_bSaving );
	m_bSaving = TFALSE;

	TSTATICCAST( ABlobFrontEnd, TXUI::GetSingleton()->GetActiveScene() )->SetProfileName( m_wszID );
	APlayerProfileManager::GetSingleton()->SetCurrentProfileName( m_wszID );
	m_bProfileCreated = TTRUE;
}

// $deBlob: FUNCTION 0063b5f0
TBOOL ABlobSplash::IsNunchukConnected( TInputDevice* a_pDevice )
{
	return TSTATICCAST( TInputWiinDeviceController, a_pDevice )->HasNunchuk();
}

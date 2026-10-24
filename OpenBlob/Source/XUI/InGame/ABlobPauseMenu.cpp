#include "pch.h"
#include "Input/ABlobInputMap.h"
#include "Challenges/AChallengeManager.h"
#include "Sound/AStreamingSoundPlayer.h"
#include "Cheats/ACheatManager.h"
#include "ABlobPauseMenu.h"
#include "ABlobInGame.h"
#include "XUI/ABlobConfirmationMenu.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GUI/AGUIManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"
#include "ALevelInformation.h"
#include "Locale/ALocaleManager.h"
#include "Challenges/AUberChallenge.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Cameras/ABlobFollowCamera.h"
#include "XUI/InGame/HUD/ABlobVisionHud.h"

#include <Input/TInputInterface.h>
#include <T2Locale/T2Locale.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING


// $deBlob: FUNCTION 00649220
// $deBlob: FUNCTION 00649230
// $deBlob: FUNCTION 006492a0
// $deBlob: FUNCTION 00649310
// $deBlob: FUNCTION 00649320
// $deBlob: FUNCTION 0053de10
// $deBlob: FUNCTION 00858b00
TDEFINE_CLASS( ABlobPauseMenu );

// $deBlob: FUNCTION 00649330
ABlobPauseMenu::ABlobPauseMenu()
    : ABlobMenu( -1 )
    , m_pResume( TNULL )
    , m_pOptions( TNULL )
    , m_pQuit( TNULL )
    , m_pChallenge( TNULL )
    , m_pReset( TNULL )
    , m_uiCursorMask( 0 )
    , m_bFadeEnabled( TFALSE )
    , m_fFade( 0.0f )
    , m_iConfirmAction( CONFIRM_None )
    , m_bSoundStateSaved( TFALSE )
    , m_pConfirmation( TNULL )
    , m_pSavedController( TNULL )
    , m_iController( 0 )
{
}

// $deBlob: FUNCTION 0064a1c0
// $deBlob: FUNCTION 0064a1b2
// $deBlob: FUNCTION 0064a1a0
ABlobPauseMenu::~ABlobPauseMenu()
{
}

// $deBlob: FUNCTION 006493c0
void ABlobPauseMenu::OnCreated()
{
	m_pResume = FindChildXUIControl( L"Resume" );
	m_pQuit   = FindChildXUIControl( L"Quit" );
	// NOTE: retail tests the multiplayer byte of AGameStatus (the same flag that picks mppausemenu.xur)
	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
		m_pChallenge = FindChildXUIControl( L"Challenge" );
	m_pReset   = FindChildXUIControl( L"Reset" );
	m_pOptions = FindChildXUIControl( L"Options" );

	ABlobMenu::OnCreated();
}

// $deBlob: FUNCTION 00649430
TBOOL ABlobPauseMenu::OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController )
{
	ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	if ( a_pSource == m_pResume )
	{
		Resume();
		return TTRUE;
	}

	if ( a_pSource == m_pQuit )
	{
		// Quit: the confirmation answers with CONFIRM_Quit (leave the level)
		m_pConfirmation = CreateConfirmation();
		m_pConfirmation->SetFadeOnYes( TTRUE );
		m_pConfirmation->SetTitle( Toshi::T2Locale::GetSingleton()->GetString( 76 ) );
		m_pConfirmation->SetText( Toshi::T2Locale::GetSingleton()->GetString( 185 ) );
		m_pConfirmation->SetParentMenu( this );
		m_iConfirmAction = CONFIRM_Quit;
		m_pConfirmation->HideElement1();
		ShowConfirmation();
		return TTRUE;
	}

	if ( a_pSource == m_pChallenge )
	{
		m_pConfirmation = CreateConfirmation();
		m_pConfirmation->SetTitle( Toshi::T2Locale::GetSingleton()->GetString( 183 ) );

		if ( IsChallengeAvailable() )
		{
			m_pConfirmation->SetText( Toshi::T2Locale::GetSingleton()->GetString( 301 ) );
		}
		else
		{
			m_pConfirmation->SetText( Toshi::T2Locale::GetSingleton()->GetString( 184 ) );
			m_pConfirmation->SetMessageOnly();
		}

		m_pConfirmation->SetParentMenu( this );
		m_iConfirmAction = CONFIRM_Challenge;
		m_pConfirmation->HideElement1();
		m_pConfirmation->SetCloseOnYes( TFALSE );
		ShowConfirmation();
		return TTRUE;
	}

	if ( a_pSource == m_pReset )
	{
		m_pConfirmation = CreateConfirmation();
		m_pConfirmation->SetTitle( Toshi::T2Locale::GetSingleton()->GetString( 383 ) );
		m_pConfirmation->SetText( Toshi::T2Locale::GetSingleton()->GetString( 301 ) );
		m_pConfirmation->SetParentMenu( this );
		m_iConfirmAction = CONFIRM_Reset;
		m_pConfirmation->HideElement1();
		m_pConfirmation->SetFadeOnYes( TTRUE );
		ShowConfirmation();
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00649fa0
TBOOL ABlobPauseMenu::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton == XUIButton_Back )
	{
		Resume();
		return TTRUE;
	}

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 00649830
void ABlobPauseMenu::Tick( TFLOAT a_fDeltaTime )
{
	TXUIScene::Tick( a_fDeltaTime );
}

// The player devices (TInputDevice::ms_aPlayerDevices) as one value
struct PlayerDevices
{
	TINT aiDevices[ 14 ];
};

// $deBlob: FUNCTION 0064a0d0
void ABlobPauseMenu::SaveDefaultController()
{
	m_iController      = Toshi::TXUI::GetSingleton()->GetDefaultController();
	m_pSavedController = new TUINT8[ sizeof( PlayerDevices ) ];
	*TSTATICCAST( PlayerDevices, m_pSavedController ) = *TREINTERPRETCAST( PlayerDevices*, TInputDevice::GetPlayerDeviceIndices() );
}

// $deBlob: FUNCTION 0064a130
void ABlobPauseMenu::RestoreDefaultController()
{
	if ( m_pSavedController != TNULL )
	{
		*TREINTERPRETCAST( PlayerDevices*, TInputDevice::GetPlayerDeviceIndices() ) = *TSTATICCAST( PlayerDevices, m_pSavedController );
		Toshi::TXUI::GetSingleton()->SetDefaultController( m_iController );
		delete[] TSTATICCAST( TUINT8, m_pSavedController );
		m_pSavedController = TNULL;
	}
}

// $deBlob: FUNCTION 00649850
void ABlobPauseMenu::Pause()
{
	SaveDefaultController();

	TInputDevice* pDevice = TInputDevice::GetPlayerDevice( m_iController );
	TInputDevice::SetPlayerDevice( 0, pDevice );
	Toshi::TXUI::GetSingleton()->SetDefaultController( 0 );

	for ( TINT i = 1; i < 4; i++ )
		m_apFocus[ i ] = TNULL;

	m_iConfirmAction = CONFIRM_None;

	m_bFadeEnabled = AGameStateController::GetSingleton()->IsFadeEnabled( 0 );
	m_fFade        = AGameStateController::GetSingleton()->GetFade( 0 );
	AGameStateController::GetSingleton()->SetFade( 0.0f, 0 );

	m_bSoundStateSaved = TFALSE;

	if ( ACheatManager::GetSingleton() != TNULL )
		m_bSoundStateSaved = ACheatManager::GetSingleton()->SetMessageVisible( TFALSE );

	TInputInterface::GetSingleton()->StopAllRepeats();

	m_uiCursorMask = AGUIManager::GetSingleton()->GetCursorMask();
	AGUIManager::GetSingleton()->SetCursorActive( TTRUE );
	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, 0 );
	AGUIManager::GetSingleton()->SetCursorVisibleCount( 1 );

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	if ( !pState->IsA( &TGetClass( AInGameState ) ) )
	{
		TDebug_AssertHandler( "TTRUE == pState->IsA(TGetClass(AInGameState))", "..\\..\\Source\\XUI\\InGame\\ABlobPauseMenu.cpp", 212, "TASSERT" );
	}
	TSTATICCAST( AInGameState, pState )->SetPaused( TTRUE );

	ASoundInterface::SetCategoryPaused( (ASoundMap::CATEGORYID)2, TTRUE );

	AStreamingSoundPlayer::GetSingleton()->SetPaused( TTRUE, TFALSE );

	// The sprint levels pause the music, the others slow it down
	if ( ALevelInformation::GetSingleton()->GetLevelHeader()->m_pLevelProps[ AGameStatus::GetSingleton()->GetCurrentLevel() ].m_sprintLevel != 0 )
		AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.0f );
	else
		AMusicManager::GetSingleton()->SetPitch( 0.1f, 0.7f, TTRUE );

	SetFocus( m_pResume, m_iController );

	// The info of the current challenge (not in the sprint levels): its difficulty, type and summary
	AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();

	if ( pChallenge == TNULL )
		return;

	if ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel != 0 )
		return;

	AChallenge::Type eType = pChallenge->GetType();

	if ( eType == AChallenge::Type_Uber )
		pChallenge = TSTATICCAST( AUberChallenge, pChallenge )->GetCurrentSubChallenge();

	TXUIControl* pDifficulty = FindChildXUIControl( L"Difficulty" );
	TUINT32      uiFlags     = pChallenge->GetData()->m_uiFlags;
	TINT         iStars;

	if ( uiFlags & AChallenge::Data::Flags_DifficultyEasy )
		iStars = 0;
	else if ( uiFlags & AChallenge::Data::Flags_DifficultyMedium )
		iStars = 1;
	else
		iStars = ( TUINT8( uiFlags ) >> 2 ) & 2;

	if ( iStars == 0 )
		pDifficulty->GetVisual()->PlayAnimation( L"Show1Star", L"EndShow1Star" );
	else if ( iStars == 1 )
		pDifficulty->GetVisual()->PlayAnimation( L"Show2Stars", L"EndShow2Stars" );
	else if ( iStars == 2 )
		pDifficulty->GetVisual()->PlayAnimation( L"Show3Stars", L"EndShow3Stars" );

	TXUIControl* pType = FindChildXUIControl( L"Type" );

	switch ( eType )
	{
		case AChallenge::Type_Paint:
		case AChallenge::Type_Civilian:
		case AChallenge::Type_Car:
			pType->GetVisual()->PlayAnimation( L"Paint", L"Paint" );
			break;
		case AChallenge::Type_Landmark:
			pType->GetVisual()->PlayAnimation( L"Landmark", L"Landmark" );
			break;
		case AChallenge::Type_Combat:
			pType->GetVisual()->PlayAnimation( L"Combat", L"Combat" );
			break;
		case AChallenge::Type_Race:
			pType->GetVisual()->PlayAnimation( L"Race", L"Race" );
			break;
		default:
			break;
	}

	TXUIControl*    pSummary   = FindChildXUIControl( L"Summary" );
	ALocaleManager* pLocale    = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );
	TINT            iSummaryID = TINT( pChallenge->GetData()->m_auiStringIDs[ 0 ] );

	if ( iSummaryID == -1 )
	{
		pSummary->SetText( L"", -1 );
	}
	else
	{
		pSummary->SetText( pLocale->GetString( iSummaryID ), -1 );
		ALocaleManager::StringFormat* pFormat = pLocale->GetStringFormat( iSummaryID );

		if ( pFormat != TNULL )
		{
			pSummary->SetText2( pLocale->GetOverwrittenFormatString( iSummaryID, pFormat->m_wszFormat ), pLocale->GetFormatUnk04() );
			return;
		}
	}

	pSummary->SetText2( TNULL, 0 );
}

// $deBlob: FUNCTION 00649c80
void ABlobPauseMenu::Unpause()
{
	RestoreDefaultController();

	if ( !AGameStateController::GetSingleton()->IsFadeEnabled( 0 ) )
	{
		TFLOAT fFade = AGameStateController::GetSingleton()->GetFade( 0 );
		if ( fFade <= 0.0f )
		{
			AGameStateController::GetSingleton()->SetFade( m_fFade, 0 );
			AGameStateController::GetSingleton()->EnableFade( m_bFadeEnabled, 0 );
		}
	}

	if ( m_bSoundStateSaved && ACheatManager::GetSingleton() != TNULL )
	{
		if ( 0.0f < ACheatManager::GetSingleton()->GetMessageTime() )
			ACheatManager::GetSingleton()->SetMessageVisible( TTRUE );

		m_bSoundStateSaved = TFALSE;
	}

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	if ( !pState->IsA( &TGetClass( AInGameState ) ) )
	{
		TDebug_AssertHandler( "TTRUE == pState->IsA(TGetClass(AInGameState))", "..\\..\\Source\\XUI\\InGame\\ABlobPauseMenu.cpp", 122, "TASSERT" );
	}

	AGUIManager::GetSingleton()->SetCursorVisible( m_uiCursorMask > 0, 0 );
	AGUIManager::GetSingleton()->SetCursorVisibleCount( m_uiCursorMask );

	AInGameState* pInGame = TSTATICCAST( AInGameState, pState );
	if ( !pInGame->IsWaitingToStart() )
	{
		pInGame->UpdateMusicPause();
		pInGame->SetPaused( TFALSE );
		AStreamingSoundPlayer::GetSingleton()->SetPaused( TFALSE, TFALSE ); // the sound pause of the level is lifted
	}

	// The jump button counts as held (no jump on the resume)
	ABlobInputMap::GetSingleton()->SetButtonsHeld( 0, 8 );
}

// $deBlob: FUNCTION 00649dc0
void ABlobPauseMenu::OnConfirmationClosed( TBOOL a_bYes )
{
	if ( !a_bYes )
		return;

	if ( m_iConfirmAction == CONFIRM_Quit )
	{
		// Quit: the level ends, AInGameState leaves it (the menu is not resumed)
		RestoreDefaultController();

		AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
		if ( !pState->IsA( &TGetClass( AInGameState ) ) )
		{
			TDebug_AssertHandler( "TTRUE == pState->IsA(TGetClass(AInGameState))", "..\\..\\Source\\XUI\\InGame\\ABlobPauseMenu.cpp", 576, "TASSERT" );
		}
		TSTATICCAST( AInGameState, pState )->SetLevelEnded( TTRUE );
	}
	else if ( m_iConfirmAction == CONFIRM_Reset )
	{
		// Reset: the level restarts
		AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
		if ( !pState->IsA( &TGetClass( AInGameState ) ) )
		{
			TDebug_AssertHandler( "TTRUE == pState->IsA(TGetClass(AInGameState))", "..\\..\\Source\\XUI\\InGame\\ABlobPauseMenu.cpp", 582, "TASSERT" );
		}
		TSTATICCAST( AInGameState, pState )->RestartLevel();

		ABlobInGame* pParent = TSTATICCAST( ABlobInGame, FindParentScene() );
		if ( !pParent->IsA( &TGetClass( ABlobInGame ) ) )
		{
			TDebug_AssertHandler( "pParent->IsA(TGetClass(ABlobInGame))", "..\\..\\Source\\XUI\\InGame\\ABlobPauseMenu.cpp", 587, "TASSERT" );
		}
		pParent->SetReopenPause( TTRUE );
		m_uiCursorMask = 0;
		Resume();
	}
	else if ( m_iConfirmAction == CONFIRM_Challenge )
	{
		// Challenge: the challenge is started from the pause menu
		if ( IsChallengeAvailable() )
		{
			AChallengeManager::GetSingleton()->QuitChallenge( TFALSE );

			ABlobInGame* pParent = TSTATICCAST( ABlobInGame, FindParentScene() );
			if ( !pParent->IsA( &TGetClass( ABlobInGame ) ) )
			{
				TDebug_AssertHandler( "pParent->IsA(TGetClass(ABlobInGame))", "..\\..\\Source\\XUI\\InGame\\ABlobPauseMenu.cpp", 605, "TASSERT" );
			}
			pParent->SetReopenPause( TTRUE );
			Resume();
		}
	}
}

// $deBlob: FUNCTION 00649fc0
void ABlobPauseMenu::Resume()
{
	// Leaving the pause menu from the blob vision ends the vision
	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

		if ( pBlob->m_bInBlobVision != 0 )
		{
			pBlob->m_bInBlobVision = 0;
			pBlob->GetAnimationComponent()->SetTargetAlpha( 1.0f, 10.0f );

			ACamera* pCam = pBlob->GetViewCamera();
			TASSERT( pCam->IsA( &TGetClass( ABlobFollowCamera ) ) );
			TSTATICCAST( ABlobFollowCamera, pCam )->SetBlobVision( TTRUE );

			// The vision HUD under the pause menu goes away
			TXUIScene* pParent = FindParentScene();

			if ( pParent != TNULL )
			{
				TXUIScene* pPrevScene = pParent->GetPrevScene();

				if ( pPrevScene != TNULL && pPrevScene->IsA( &TGetClass( ABlobVisionHud ) ) )
				{
					TXUIScene* pVisionPrev = pPrevScene->GetPrevScene();
					delete pPrevScene;
					pParent->SetPrevScene( pVisionPrev );
				}
			}
		}
	}

	Toshi::TXUI::GetSingleton()->TransitionBack( TSTATICCAST( TXUIScene, FindParentScene() ) );
}

// Shows the confirmation popup over the pause menu
void ABlobPauseMenu::ShowConfirmation()
{
	m_pConfirmation->SetPosition( m_pConfirmation->GetPositionX() * 2.0f, m_pConfirmation->GetPositionY() * 2.0f );
	Toshi::TXUI::GetSingleton()->TransitionTo( this, m_pConfirmation );
}

// Finds and creates the confirmation popup, and parents it to the scene
ABlobConfirmationMenu* ABlobPauseMenu::CreateConfirmation()
{
	Toshi::TXUIResource*   pResource     = Toshi::TXUI::GetSingleton()->FindResource( "confirmation.xur" );
	ABlobConfirmationMenu* pConfirmation = TSTATICCAST( ABlobConfirmationMenu, Toshi::TXUI::GetSingleton()->CreateScene( pResource, L"Confirmation", TFALSE ) );

	// The popup goes to the pause menu's parent (the in-game scene), a sibling of the pause menu: the pause menu hides
	// under its TransFrom, the popup must stay visible
	TXUIScene* pParent = FindParentScene();
	if ( pParent != TNULL )
	{
		pConfirmation->Unlink();
		pParent->AddChildTail( pConfirmation );
	}

	return pConfirmation;
}

// The current challenge may be quit (its data's Flags_CanQuit)
TBOOL ABlobPauseMenu::IsChallengeAvailable() const
{
	AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();
	return pChallenge != TNULL && ( pChallenge->GetData()->m_uiFlags & AChallenge::Data::Flags_CanQuit ) != 0;
}

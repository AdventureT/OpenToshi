#include "pch.h"
#include "ABlobMainMenu.h"
#include "Sound/AMusicManager.h"
#include "Sound/AGUIMusicController.h"
#include "GUI/AGUIManager.h"
#include "ABlobFrontEnd.h"
#include "ABlobSplash.h"
#include "AApplication.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/APlayerProfileManager.h"
#include "XUI/ABlobConfirmationMenu.h"
#include "GameController/AGameStatus.h"
#include "Sound/ASoundInterface.h"
#include "AAchievements.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIInput.h>
#include <T2Locale/T2Locale.h>
#include <Input/TInputInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00635f20
// $deBlob: FUNCTION 00635f30
// $deBlob: FUNCTION 00635fa0
// $deBlob: FUNCTION 00636010
// $deBlob: FUNCTION 00636020
// $deBlob: FUNCTION 0053d650
// $deBlob: FUNCTION 00858910
TDEFINE_CLASS( ABlobMainMenu );

// $deBlob: FUNCTION 00636030
ABlobMainMenu::ABlobMainMenu()
    : ABlobMenu( -1 )
{
	m_pStory        = TNULL;
	m_pFreePaint    = TNULL;
	m_pBlobParty    = TNULL;
	m_pExtras       = TNULL;
	m_pQuit         = TNULL;
	m_pFreePaintNew = TNULL;
	m_pExtrasNew    = TNULL;
	m_pBlobPartyNew = TNULL;
	m_pDebug        = TNULL;
	m_pOptions      = TNULL;
	m_bBeatScene    = TFALSE;
	m_bFadingIn     = TFALSE;
}

// Shows and activates the cursor
// $deBlob: FUNCTION 00636990
void ABlobMainMenu::ShowCursor()
{
	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );
	AGUIManager::GetSingleton()->SetCursorActive( TTRUE );
}

// Plays the menu's background movie and makes this the beat scene
// $deBlob: FUNCTION 006369b0
void ABlobMainMenu::PlayMenuMovie()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_LoopMainMenu, ABlobFrontEnd::BGMOVIE_None, TTRUE, TFALSE, TFALSE );
	m_bBeatScene = TTRUE;
}

// $deBlob: FUNCTION 006369e0
// $deBlob: FUNCTION 006360c0
// $deBlob: FUNCTION 006369d2
ABlobMainMenu::~ABlobMainMenu()
{
}

// $deBlob: FUNCTION 00636680
void ABlobMainMenu::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bFadingIn )
	{
		AGameStateController* pController = AGameStateController::GetSingleton();

		if ( !pController->IsFading( 0 ) && pController->GetFade( 0 ) <= 0.0f )
		{
			m_bFadingIn = TFALSE;
			TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
		}
	}

	UpdateAttractMode( a_fDeltaTime );
	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00636490
void ABlobMainMenu::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible == TTRUE )
	{
		Show();
	}
	else
	{
		Hide();

		if ( a_bVisible == TFALSE )
			return;
	}

	TInputDevice::RestorePlayerDevices();
	TXUI::GetSingleton()->SetDefaultController( 0 );

	AAchievements::GetSingleton()->SetMenuState( 0 );
}

// $deBlob: FUNCTION 006360e0
void ABlobMainMenu::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pOptions   = FindChildXUIControl( 7 );
	m_pDebug     = FindChildXUIControl( 3 );
	m_pFreePaint = FindChildXUIControl( 9 );
	m_pBlobParty = FindChildXUIControl( 8 );
	m_pExtras    = FindChildXUIControl( 5 );
	m_pQuit      = FindChildXUIControl( 14 );

	AAchievements::GetSingleton()->CheckAchievements( AAchievements::ACHIEVEMENT_ALL );

	m_pStory        = FindChildXUIControl( 6 );
	m_pFreePaintNew = FindChildXUIControl( 11 );
	m_pExtrasNew    = FindChildXUIControl( 10 );
	m_pBlobPartyNew = FindChildXUIControl( 12 );

	UpdateNewFlagLabel( m_pFreePaintNew, EnSaveData::NEWFLAG_FreePaint );
	UpdateNewFlagLabel( m_pExtrasNew, EnSaveData::NEWFLAG_Extras );
	UpdateNewFlagLabel_extended( m_pBlobPartyNew, EnSaveData::NEWFLAGEXT_BlobParty );

	ToggleDebugButton();
}

// Toggles the visibility of the debug button and moves the focus off it
// $deBlob: FUNCTION 006364d0
void ABlobMainMenu::ToggleDebugButton()
{
	m_pDebug->SetVisible( !m_pDebug->IsVisible() );

	TINT iController = TXUI::GetSingleton()->GetDefaultController();

	if ( GetFocus( iController ) == m_pDebug )
	{
		SetFocus( m_pStory, iController );

		TXUIScene* pFocussedScene = TXUI::GetSingleton()->GetActiveScene();
		TASSERT( pFocussedScene && pFocussedScene->IsA( &TGetClass( ABlobFrontEnd ) ) );

		SetFocus( m_pStory, iController );
		m_pStory->SetFocus( TTRUE, TFALSE, TTRUE );
		m_pDebug->SetFocus( TFALSE, TFALSE, TTRUE );
	}
}

// $deBlob: FUNCTION 006361d0
TBOOL ABlobMainMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( m_bAttractMovie )
		return TFALSE;

	EnSaveData::SlotData* pData = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();

	if ( a_pSource == m_pFreePaint || a_pSource == m_pStory )
	{
		if ( a_pSource == m_pFreePaint )
		{
			pData->m_uiNewFlags &= ~0x20;

			ASoundInterface::PlayEvent( ASoundMap::GROUPID_FRONT_END, ASoundMap::EVENTID( 65 ), FMOD_EVENT_DEFAULT, TTRUE );
			AGameStatus::GetSingleton()->SetLevelMode( AGameStatus::Mode_FreePaint );
		}
		else
		{
			AGameStatus::GetSingleton()->SetLevelMode( AGameStatus::Mode_Story );
			ASoundInterface::PlayEvent( ASoundMap::GROUPID_FRONT_END, ASoundMap::EVENTID( 65 ), FMOD_EVENT_DEFAULT, TTRUE );
		}

		// The level selection (created with frontendcarousel.trb)
		TXUIScene* pLevelSelection = TXUI::GetSingleton()->FindCachedScene( L"LevelSelection" );
		pLevelSelection->SetTransitionVisual( TXUIScene::Transition_To, L"TransTo3" );
		m_iTransitionIndex = 5;

		TXUIScene* pParent = FindParentScene();
		TASSERT( pParent->IsA( &TGetClass( ABlobFrontEnd ) ) );
		pParent->AddChildTail( pLevelSelection );

		AAchievements::GetSingleton()->SetMenuState( 1 );

		TXUI::GetSingleton()->TransitionTo( this, pLevelSelection );
		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	if ( a_pSource == m_pBlobParty )
	{
		pData->m_uiNewFlags &= ~0x10;
		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	if ( a_pSource == m_pExtras )
	{
		AAchievements::GetSingleton()->SetMenuState( 2 );

		pData->m_uiNewFlagsExtended &= ~0x200;
		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	if ( a_pSource == m_pQuit )
	{
		// Quit the game?
		TXUI*                  pXUI          = TXUI::GetSingleton();
		ABlobConfirmationMenu* pConfirmation = TSTATICCAST( ABlobConfirmationMenu, pXUI->CreateScene( pXUI->FindResource( "confirmation.xur" ), L"Confirmation", TFALSE ) );

		TXUIScene* pParent = FindParentScene();

		if ( pParent != TNULL )
		{
			pConfirmation->RemoveSelf();
			pParent->AddChildTail( pConfirmation );
		}

		pConfirmation->SetParentMenu( this );
		pConfirmation->SetTitle( T2Locale::GetSingleton()->GetString( 75 ) );
		pConfirmation->SetText( T2Locale::GetSingleton()->GetString( 196 ) );

		pXUI->TransitionToPopup( this, pConfirmation );
	}

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}

// $deBlob: FUNCTION 00636600
TBOOL ABlobMainMenu::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( TXUI::GetSingleton()->GetTransToScene() != TNULL || TXUI::GetSingleton()->GetTransBackFromScene() != TNULL || TXUI::GetSingleton()->IsInputLocked() || a_eButton != XUIButton_Back )
		return TXUIScene::HandleButton( a_eButton, a_iController );

	// Back to the title screen
	TSTATICCAST( ABlobFrontEnd, FindParentScene() )->ResetCursorTrails();
	TXUI::GetSingleton()->TransitionBack( this );

	return TTRUE;
}

// $deBlob: FUNCTION 00636700
void ABlobMainMenu::TransTo()
{
	if ( m_pPrevScene == TNULL )
	{
		ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
		pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_LoopMainMenu, ABlobFrontEnd::BGMOVIE_None, TTRUE, TFALSE, TFALSE );
	}

	m_fIdleTime = 0.0f;

	ShowCursor();

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 00636750
void ABlobMainMenu::TransBackTo()
{
	ShowCursor();

	UpdateNewFlagLabel( m_pFreePaintNew, EnSaveData::NEWFLAG_FreePaint );
	UpdateNewFlagLabel( m_pExtrasNew, EnSaveData::NEWFLAG_Extras );
	UpdateNewFlagLabel_extended( m_pBlobPartyNew, EnSaveData::NEWFLAGEXT_BlobParty );

	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 006367c0
void ABlobMainMenu::EndTransTo()
{
	if ( m_pPrevScene != TNULL )
	{
		AMusicManager::GetSingleton()->GetGUIMusicController()->SetBeatScene( this );

		PlayMenuMovie();
	}

	// Deletes the scenes back to the title screen
	TXUIScene* pScene = m_pPrevScene;

	while ( pScene != TNULL && !pScene->IsA( &TGetClass( ABlobSplash ) ) )
	{
		TXUIScene* pPrevScene = pScene->GetPrevScene();
		delete pScene;
		pScene = pPrevScene;
	}

	m_pPrevScene = pScene;
	TASSERT( GetPrevScene() && GetPrevScene()->IsA( &TGetClass( ABlobSplash ) ) );

	RevealNewFlagLabel( m_pFreePaintNew, EnSaveData::NEWFLAG_FreePaint );
	RevealNewFlagLabel( m_pExtrasNew, EnSaveData::NEWFLAG_Extras );
	RevealNewFlagLabel_extended( m_pBlobPartyNew, EnSaveData::NEWFLAGEXT_BlobParty );

	ABlobMenu::EndTransTo();
}

// $deBlob: FUNCTION 006368c0
void ABlobMainMenu::EndTransBackTo()
{
	RevealNewFlagLabel( m_pFreePaintNew, EnSaveData::NEWFLAG_FreePaint );
	RevealNewFlagLabel( m_pExtrasNew, EnSaveData::NEWFLAG_Extras );
	RevealNewFlagLabel_extended( m_pBlobPartyNew, EnSaveData::NEWFLAGEXT_BlobParty );

	PlayMenuMovie();

	ABlobMenu::EndTransBackTo();
}

// $deBlob: FUNCTION 00636660
void ABlobMainMenu::OnConfirmationClosed( TBOOL a_bYes )
{
	if ( a_bYes )
		g_oTheApp.Destroy();
}

// $deBlob: FUNCTION 00636970
void ABlobMainMenu::EndAttractMode()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->SetBackgroundMovie( 8 );
	ABlobMenu::EndAttractMode();
}

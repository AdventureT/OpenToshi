#include "pch.h"
#include "AInGameStateHelper.h"
#include "AInGameState.h"
#include "Challenges/AChallengeManager.h"
#include "Entities/APropTriggerHelp.h"
#include "GameController/AGameStatus.h"
#include "Input/ABlobInputMap.h"
#include "Input/AInputManager2.h"
#include "Input/TInputDeviceKeyboard.h"
#include "XUI/InGame/ABlobInGame.h"
#include "XUI/InGame/ABlobPauseMenu.h"
#include "XUI/InGame/HUD/ABlobChallengeEndHud.h"
#include "XUI/InGame/HUD/ABlobLifeLostHud.h"

#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005c8ac0
AInGameStateHelper::AInGameStateHelper( AInGameState* a_pState )
    : m_pState( a_pState )
{
}

// $deBlob: FUNCTION 005c8af0
// $deBlob: FUNCTION 005c8e50
AInGameStateHelper::~AInGameStateHelper()
{
}

// Opens the pause menu when the pause button (or Escape) is pressed: the "InGame" scene (ingame.xur) gets the pause
// menu as its child, the game pauses when it transitions to with Usage_Pause
// $deBlob: FUNCTION 005c8b00
TINT AInGameStateHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_oInputHelper.Update( a_fDeltaTime );

	AInputHelper::ButtonResult oPressed       = m_oInputHelper.IsJustDownWithResult( 6 );
	AInputManager2*            pInputManager  = AInputManager2::GetSingleton();
	AGameStatus*               pStatus        = AGameStatus::GetSingleton();
	TXUI*                      pXUI           = TXUI::GetSingleton();

	// In multiplayer any controller may open the menu, so the locked device is cleared
	if ( pStatus->IsMultiplayer() )
	{
		AInputManager2::AInputDeviceHandle oNoDevice = { TNULL, 0 };
		pInputManager->SetLockedDevice( oNoDevice );
	}

	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
	TBOOL bEscape = pKeyboard->IsDown( TInputDeviceKeyboard::KEY_ESCAPE );
	TBOOL bPause  = m_oInputHelper.IsJustDown( 6 ) || bEscape;

	// The scenes that keep the pause from opening: Blob's life lost HUD, and a challenge that offers Retry
	TBOOL bBlocked = TFALSE;
	TXUIScene* pActiveScene = pXUI->GetActiveScene();
	if ( pActiveScene != TNULL )
	{
		if ( pActiveScene->IsA( &TGetClass( ABlobLifeLostHud ) ) == TTRUE )
		{
			bBlocked = TTRUE;
		}
		else if ( pActiveScene->IsA( &TGetClass( ABlobChallengeEndHud ) ) == TTRUE && TSTATICCAST( ABlobChallengeEndHud, pActiveScene )->IsRetrying() )
		{
			bBlocked = TTRUE;
		}
	}

	if ( bBlocked )
		bPause = TFALSE;
	else if ( bPause && pStatus->IsLevelComplete() )
		bPause = TFALSE;

	if ( APropTriggerHelp::GetFullscreenTopic() != -1 || !bPause )
		return 0;

	if ( pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked() )
		return 0;

	// The player's input is dropped while the menu is open, and the controller that paused it becomes the default
	ABlobInputMap::GetSingleton()->ClearPlayers();

	TINT iController = -1;
	if ( oPressed.m_Handle.m_pDevice != TNULL )
		iController = oPressed.m_Handle.m_pDevice->GetPlayer();

	if ( iController >= 0 )
		pXUI->SetDefaultController( iController );

	TXUIResource* pInGameResource = pXUI->FindResource( "ingame.xur" );
	ABlobInGame*  pInGame         = TSTATICCAST( ABlobInGame, pXUI->CreateScene( pInGameResource, L"InGame", TFALSE ) );

	TXUIResource*   pPauseResource = pXUI->FindResource( pStatus->IsMultiplayer() ? "mppausemenu.xur" : "pausemenu.xur" );
	ABlobPauseMenu* pPauseMenu     = TSTATICCAST( ABlobPauseMenu, pXUI->CreateScene( pPauseResource, L"PauseMenu", TFALSE ) );

	// A focus left over in the new menu loses it before the menu takes the focus
	if ( pPauseMenu->GetFocus( 0 ) != TNULL )
	{
		pPauseMenu->GetFocus( 0 )->SetFocus( TFALSE, TFALSE, TTRUE );
		pPauseMenu->SetFocus( TNULL, 0 );
	}
	pPauseMenu->SetFocus( TTRUE, TFALSE, TTRUE );

	pInGame->SetUsage( ABlobInGame::Usage_Pause );
	pInGame->SetFocus( pPauseMenu, 0 );
	pInGame->AddChildTail( pPauseMenu );

	// The transition plays the matching animation of the pause menu: the multiplayer one, the sprint, the challenge
	// or the normal level
	const TWCHAR* wszTransTo;
	const TWCHAR* wszEndTransTo;
	if ( pStatus->IsMultiplayer() )
	{
		wszTransTo    = L"TransTo";
		wszEndTransTo = L"EndTransTo";
	}
	else if ( pStatus->GetLevelData().m_pLevelProps->m_sprintLevel != 0 || pStatus->GetLevelMode() == AGameStatus::Mode_FreePaint )
	{
		wszTransTo    = L"TransToSprint";
		wszEndTransTo = L"EndTransToSprint";
	}
	else
	{
		AChallengeManager* pChallengeManager = AChallengeManager::GetSingleton();
		AChallenge*        pChallenge        = pChallengeManager->GetCurrentChallenge();

		if ( pChallenge == TNULL || pChallengeManager->IsEnding() || pChallenge->IsStarting() )
		{
			wszTransTo    = L"TransToNormal";
			wszEndTransTo = L"EndTransToNormal";
		}
		else
		{
			wszTransTo    = L"TransToChallenge";
			wszEndTransTo = L"EndTransToChallenge";
		}
	}

	pPauseMenu->PlayAnimation( wszTransTo, wszEndTransTo );
	pXUI->TransitionTo( TNULL, pInGame );

	pInputManager->SetLockedDevice( oPressed.m_Handle );

	return 0;
}

// $deBlob: FUNCTION 005c8de0
void AInGameStateHelper::OnInsertion()
{
	AInputManager2* pInputManager = AInputManager2::GetSingleton();
	pInputManager->SetContext( AInputMap::INPUTCONTEXT_InGame );
	m_oInputHelper.AddMapping( 0, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( 6, TFALSE, -1.0f );
	pInputManager->RemoveCurrentContext();
}

// $deBlob: FUNCTION 005c8e30
void AInGameStateHelper::OnRemoval()
{
	m_oInputHelper.Destroy();
}

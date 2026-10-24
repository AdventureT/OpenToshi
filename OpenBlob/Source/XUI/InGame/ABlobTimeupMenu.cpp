#include "pch.h"
#include "ABlobTimeupMenu.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include <XUI/TXUI.h>
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "GUI/AGUIManager.h"
#include "GUI/AGUICursor.h"
#include "XUI/ABlobConfirmationMenu.h"
#include <XUI/TXUIInput.h>
#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The "TTRUE == pState->IsA(TGetClass(AInGameState))" asserts of OnCreated and OnConfirmationClosed fire once each
static TBOOL s_bInGameStateAssertFired  = TFALSE;
static TBOOL s_bInGameStateAssertFiredB = TFALSE;

// $deBlob: FUNCTION 0064ccd0
// $deBlob: FUNCTION 0064cce0
// $deBlob: FUNCTION 0064ce10
// $deBlob: FUNCTION 0064ce20
// $deBlob: FUNCTION 0053df50
// $deBlob: FUNCTION 00858b50
TDEFINE_CLASS( ABlobTimeupMenu );

// $deBlob: FUNCTION 0064ce30
// $deBlob: FUNCTION 0064cd80
ABlobTimeupMenu::ABlobTimeupMenu()
    : ABlobMenu( -1 )
{
	m_pRetryButton = TNULL;
	m_pQuitButton  = TNULL;
	m_bConfirming  = TFALSE;
}

// $deBlob: FUNCTION 0064d380
// $deBlob: FUNCTION 0064d360
// $deBlob: FUNCTION 0064d372
ABlobTimeupMenu::~ABlobTimeupMenu()
{
}

// $deBlob: FUNCTION 0064d200
void ABlobTimeupMenu::Tick( TFLOAT a_fDeltaTime )
{
	TBOOL bBusy = TXUI::GetSingleton()->GetTransToScene() != TNULL || TXUI::GetSingleton()->GetTransFromScene() != TNULL || TXUI::GetSingleton()->IsInputLocked();

	if ( !m_bConfirming || AGameStateController::GetSingleton()->IsFading( 0 ) || AGameStateController::GetSingleton()->GetFade( 0 ) != 1.0f || bBusy )
	{
		// The fade in is over: the input is back
		if ( !AGameStateController::GetSingleton()->IsFading( 0 ) && AGameStateController::GetSingleton()->GetFade( 0 ) == 0.0f && !bBusy )
		{
			TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
			AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TFALSE );
		}
	}
	else
	{
		// Retry faded out: the level restarts and the menu goes back
		TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
		AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TFALSE );

		AInGameState* pInGameState = TSTATICCAST( AInGameState, AGameStateController::GetSingleton()->FindState( &TGetClass( AInGameState ) ) );
		pInGameState->RestartLevel();

		TXUIScene* pScene = m_pPrevScene;

		while ( pScene != TNULL )
		{
			TXUIScene* pPrev = pScene->GetPrevScene();
			delete pScene;
			pScene = pPrev;
		}

		m_pPrevScene = TNULL;
		TXUI::GetSingleton()->TransitionBack( FindParentScene() );
		m_uiSceneFlags |= 1;
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0064ce70
void ABlobTimeupMenu::OnCreated()
{
	ABlobMenu::OnCreated();

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	if ( !s_bInGameStateAssertFired )
	{
		TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
		s_bInGameStateAssertFired = TTRUE;
	}

	TSTATICCAST( AInGameState, pState )->SetPaused( TTRUE );

	AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.0f );
	AMusicManager::GetSingleton()->PlayGameOver();
	ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID( 2 ), TTRUE );
	AGUIManager::GetSingleton()->GetCursor()->SetVisible( TTRUE, TTRUE );

	m_pRetryButton = FindChildXUIControl( 5 );
	m_pQuitButton  = FindChildXUIControl( 4 );
	m_pElement2    = FindChildXUIElement( 2 );
	m_pElement3    = FindChildXUIElement( 3 );

	// Element 3 instead of element 2: no lives left, a sprint without a medal, or a won free paint level
	TINT  iGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();
	TBOOL bVisible3;

	if ( ( iGameType < 0 && APlayerManager::GetSingleton()->GetBlobData( 0 )->m_uiLives == 0 ) ||
	     ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel != 0 && ( AGameStatus::GetSingleton()->GetLevelData().m_aPlayerData[ 0 ].m_auiUnk3C[ 0 ] & 7 ) == 0 ) ||
	     ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint && AGameStatus::GetSingleton()->IsLevelWon() ) )
	{
		m_pElement2->SetVisible( TFALSE );
		bVisible3 = TTRUE;
	}
	else
	{
		m_pElement2->SetVisible( TTRUE );
		bVisible3 = TFALSE;
	}

	m_pElement3->SetVisible( bVisible3 );

	AGameStateController::GetSingleton()->FadeFromBlack( 0.5f, 0 );
	AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TTRUE );
}

// $deBlob: FUNCTION 0064cff0
TBOOL ABlobTimeupMenu::OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController )
{
	if ( m_bConfirming )
		return TTRUE;

	if ( a_pSource == m_pRetryButton )
	{
		// Retry: fades out, Tick restarts the level
		AGUIManager::GetSingleton()->GetCursor()->SetVisible( TFALSE, TTRUE );
		AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TTRUE );
		AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
		ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID( 2 ), TFALSE );
		m_bConfirming = TTRUE;
		TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
		return TTRUE;
	}

	if ( a_pSource == m_pQuitButton )
	{
		// Quit: asks first (OnConfirmationClosed)
		TXUIResource*          pResource     = TXUI::GetSingleton()->FindResource( "confirmation.xur" );
		ABlobConfirmationMenu* pConfirmation = TSTATICCAST( ABlobConfirmationMenu, TXUI::GetSingleton()->CreateScene( pResource, L"Confirmation", TFALSE ) );
		pConfirmation->SetFadeOnYes( TTRUE );

		TXUIElement* pParent = FindParentControl();

		if ( pParent != TNULL )
		{
			pConfirmation->Unlink();
			pParent->AddChildTail( pConfirmation );
		}

		pConfirmation->SetTitle( T2Locale::GetSingleton()->GetString( 0x4C ) );
		pConfirmation->SetText( T2Locale::GetSingleton()->GetString( 0xB9 ) );
		pConfirmation->SetParentMenu( this );
		pConfirmation->SetPosition( pConfirmation->GetPositionX() * 2.0f, pConfirmation->GetPositionY() * 2.0f );
		TXUI::GetSingleton()->TransitionToPopup( this, pConfirmation );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0064d190
void ABlobTimeupMenu::OnConfirmationClosed( TBOOL a_bYes )
{
	if ( a_bYes )
	{
		AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
		if ( !s_bInGameStateAssertFiredB )
		{
			TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
			s_bInGameStateAssertFiredB = TTRUE;
		}

		ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID( 2 ), TFALSE );
		TSTATICCAST( AInGameState, pState )->SetLevelEnded( TTRUE );
	}
}

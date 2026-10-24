#include "pch.h"
#include "ABlobMultiplayerGameOver.h"
#include "XUI/ABlobConfirmationMenu.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"

#include <XUI/TXUI.h>
#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 006486d0
// $deBlob: FUNCTION 006486e0
// $deBlob: FUNCTION 00648780
// $deBlob: FUNCTION 00648810
// $deBlob: FUNCTION 00648820
// $deBlob: FUNCTION 0053dd90
// $deBlob: FUNCTION 00858ae0
TDEFINE_CLASS( ABlobMultiplayerGameOver );

// $deBlob: FUNCTION 00648830
ABlobMultiplayerGameOver::ABlobMultiplayerGameOver()
    : ABlobMenu( -1 )
{
	m_pRestart    = TNULL;
	m_pQuit       = TNULL;
	m_bRestarting = TFALSE;
}

// $deBlob: FUNCTION 00648bb0
// $deBlob: FUNCTION 00648bc2
// $deBlob: FUNCTION 00648bd0
ABlobMultiplayerGameOver::~ABlobMultiplayerGameOver()
{
}

// $deBlob: FUNCTION 00648870
void ABlobMultiplayerGameOver::OnCreated()
{
	ABlobMenu::OnCreated();
	m_pRestart = FindChildXUIControl( 6 );
	m_pQuit    = FindChildXUIControl( 4 );
}

// $deBlob: FUNCTION 006488a0
TBOOL ABlobMultiplayerGameOver::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	if ( a_pSource == m_pRestart )
	{
		// Restarts the level once the screen is black (Tick)
		AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TTRUE );
		AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
		m_bRestarting = TTRUE;
		return TTRUE;
	}

	if ( a_pSource == m_pQuit )
	{
		// Quit the match?
		ABlobConfirmationMenu* pConfirmation = TSTATICCAST( ABlobConfirmationMenu, TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "confirmation.xur" ), L"Confirmation", TFALSE ) );
		pConfirmation->SetFadeOnYes( TTRUE );

		TXUIScene* pParent = FindParentScene();

		if ( pParent != TNULL )
		{
			pConfirmation->RemoveSelf();
			pParent->AddChildTail( pConfirmation );
		}

		pConfirmation->SetTitle( T2Locale::GetSingleton()->GetString( 76 ) );
		pConfirmation->SetText( T2Locale::GetSingleton()->GetString( 185 ) );
		pConfirmation->SetParentMenu( this );
		pConfirmation->HideElement1();
		pConfirmation->SetPosition( pConfirmation->GetPositionX() * 2.0f, pConfirmation->GetPositionY() * 2.0f );

		TXUI::GetSingleton()->TransitionTo( this, pConfirmation );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00648a20
void ABlobMultiplayerGameOver::OnConfirmationClosed( TBOOL a_bYes )
{
	if ( a_bYes )
	{
		AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
		TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
		TSTATICCAST( AInGameState, pState )->SetLevelEnded( TTRUE );
	}
}

// $deBlob: FUNCTION 00648a90
void ABlobMultiplayerGameOver::Tick( TFLOAT a_fDeltaTime )
{

	if ( m_bRestarting && !AGameStateController::GetSingleton()->IsFading( 0 ) && AGameStateController::GetSingleton()->GetFade( 0 ) == 1.0f )
	{
		AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TFALSE );

		AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
		TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
		TSTATICCAST( AInGameState, pState )->RestartLevel();

		// The scenes opened before this one are deleted
		TXUIScene* pScene = m_pPrevScene;

		while ( pScene != TNULL )
		{
			TXUIScene* pPrev = pScene->GetPrevScene();
			delete pScene;
			pScene = pPrev;
		}

		m_pPrevScene = TNULL;
		TXUI::GetSingleton()->TransitionBack( FindParentScene() );
		MarkForDelete();
	}

	TXUIScene::Tick( a_fDeltaTime );
}

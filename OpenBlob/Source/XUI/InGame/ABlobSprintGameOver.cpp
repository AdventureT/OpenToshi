#include "pch.h"
#include "ABlobSprintGameOver.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GUI/AGUIManager.h"
#include "GUI/AGUICursor.h"

#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobSprintGameOver ) == 0x14C );

// $deBlob: FUNCTION 0064bd70
// $deBlob: FUNCTION 0064bd80
// $deBlob: FUNCTION 0064be20
// $deBlob: FUNCTION 0064beb0
// $deBlob: FUNCTION 0064bec0
// $deBlob: FUNCTION 0053ded0
// $deBlob: FUNCTION 00858b30
TDEFINE_CLASS( ABlobSprintGameOver );

// $deBlob: FUNCTION 0064bed0
ABlobSprintGameOver::ABlobSprintGameOver()
    : ABlobMenu( -1 )
{
	m_pRestart = TNULL;
	m_pQuit    = TNULL;
	m_bFading  = TFALSE;
	m_bRestart = TFALSE;
}

// $deBlob: FUNCTION 0064c180
// $deBlob: FUNCTION 0064c192
// $deBlob: FUNCTION 0064c1a0
ABlobSprintGameOver::~ABlobSprintGameOver()
{
}

// $deBlob: FUNCTION 0064c000
void ABlobSprintGameOver::Tick( TFLOAT a_fDeltaTime )
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( m_bFading && !pController->IsFading( 0 ) && pController->GetFade( 0 ) == 1.0f )
	{
		m_bFading = TFALSE;

		if ( !m_bRestart )
		{
			AGameState* pState = pController->GetCurrentGameState();
			TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
			TSTATICCAST( AInGameState, pState )->EndLevelFromResults();
		}
		else
		{
			AGUIManager::GetSingleton()->GetCursor()->SetVisible( TFALSE, TTRUE );

			AGameState* pState = pController->GetCurrentGameState();
			TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
			TSTATICCAST( AInGameState, pState )->RestartLevel();

			// Destroys the scenes this one was opened from
			TXUIScene* pScene = m_pPrevScene;

			while ( pScene != TNULL )
			{
				TXUIScene* pPrev = pScene->GetPrevScene();
				delete pScene;
				pScene = pPrev;
			}

			m_pPrevScene = TNULL;
			TXUI::GetSingleton()->TransitionBack( FindParentScene() );
			m_uiSceneFlags |= SceneFlags_Delete;
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0064bf10
void ABlobSprintGameOver::OnCreated()
{
	ABlobMenu::OnCreated();
	m_pRestart = FindChildXUIControl( 5 );
	m_pQuit    = FindChildXUIControl( 6 );
}

// $deBlob: FUNCTION 0064bf40
TBOOL ABlobSprintGameOver::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( m_bFading )
		return TFALSE;

	ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	if ( a_pSource == m_pRestart )
	{
		m_bFading  = TTRUE;
		m_bRestart = TTRUE;
	}
	else
	{
		if ( a_pSource != m_pQuit )
			return TFALSE;

		m_bFading = TTRUE;
	}

	AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TTRUE );
	AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
	AGameStateController::GetSingleton()->SetRenderFadesFirst( TFALSE );

	SetEnabled( TFALSE );
	m_apFocus[ 0 ] = TNULL;
	return TTRUE;
}

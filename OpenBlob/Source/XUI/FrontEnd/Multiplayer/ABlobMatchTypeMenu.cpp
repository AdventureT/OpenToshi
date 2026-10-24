#include "pch.h"
#include "ABlobMatchTypeMenu.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobMatchTypeMenu ) == 0x154 );

// $deBlob: FUNCTION 0063eec0
// $deBlob: FUNCTION 0063eed0
// $deBlob: FUNCTION 0063ef60
// $deBlob: FUNCTION 0063efe0
// $deBlob: FUNCTION 0063eff0
// $deBlob: FUNCTION 0053dad0
// $deBlob: FUNCTION 00858a30
TDEFINE_CLASS( ABlobMatchTypeMenu );

// $deBlob: FUNCTION 0063f000
ABlobMatchTypeMenu::ABlobMatchTypeMenu()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopMPPlayer )
{
	m_bInGame = TFALSE;
}

// $deBlob: FUNCTION 0063f360
// $deBlob: FUNCTION 0063f372
// $deBlob: FUNCTION 0063f380
ABlobMatchTypeMenu::~ABlobMatchTypeMenu()
{
}

// $deBlob: FUNCTION 0063f030
void ABlobMatchTypeMenu::OnCreated()
{
	// NOTE: retail doesn't call ABlobMenu::OnCreated
	TWCHAR* wszName = TStringManager::GetTempString16();

	for ( TINT i = 0; i < NUM_MATCH_TYPES; i++ )
	{
		TStringManager::String16Format( wszName, L"MatchType%d", i + 1 );
		m_apMatchTypes[ i ] = FindChildXUIControl( wszName );
	}
}

// $deBlob: FUNCTION 0063f080
TBOOL ABlobMatchTypeMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	TINT iType;

	for ( iType = 0; a_pSource != m_apMatchTypes[ iType ]; )
	{
		if ( ++iType >= NUM_MATCH_TYPES )
			return TFALSE;
	}

	AGameStatus::GetSingleton()->SetMultiPlayerGameType( iType );

	TXUIScene* pTopLevelScene = TXUI::GetSingleton()->GetActiveScene();

	if ( TStringManager::String16Compare( pTopLevelScene->GetID(), L"FrontEnd" ) == 0 )
	{
		TXUIScene* pScene = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "multiplayerlevelselection.xur" ), TUINT32( 0 ) );
		pScene->SetVisible( TFALSE );

		TXUIScene* pParentScene = FindParentScene();

		if ( pParentScene != TNULL )
			pParentScene->AddChildTail( pScene );

		TXUI::GetSingleton()->TransitionTo( this, pScene );
		return TTRUE;
	}

	m_bInGame = TTRUE;
	TASSERT( 0 == TStringManager::String16Compare( pTopLevelScene->GetID(), L"InGame" ) );

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pState )->RestartLevel();

	// Destroys the menus down to the HUD
	TXUIScene* pScene = m_pPrevScene;

	while ( pScene != TNULL && !pScene->IsA( &TGetClass( ABlobHud ) ) )
	{
		TXUIScene* pPrev = pScene->GetPrevScene();
		delete pScene;
		pScene = pPrev;
	}

	m_pPrevScene = pScene;
	TXUI::GetSingleton()->TransitionBack( this );
	return TTRUE;
}

// $deBlob: FUNCTION 0063f270
void ABlobMatchTypeMenu::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransMPPlayerMPColour, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	pFrontEnd->SlideRight();

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 0063f2e0
void ABlobMatchTypeMenu::TransBackTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TINT           iMovie    = pFrontEnd->GetRandomBackgroundMovie( ABlobFrontEnd::BGMOVIE_BackGen01 );

	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( iMovie, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	pFrontEnd->ResetCursorTrails();

	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 0063f230
void ABlobMatchTypeMenu::EndTransBackFrom()
{
	TXUIScene* pParentScene = FindParentScene();

	if ( m_bInGame && pParentScene != TNULL && TStringManager::String16Compare( pParentScene->GetID(), L"InGame" ) == 0 )
		pParentScene->SetVisible( TFALSE );

	TXUIScene::EndTransBackFrom();
}

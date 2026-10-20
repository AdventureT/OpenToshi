#include "pch.h"
#include "ABlobHelpFullScreen.h"
#include "ABlobHelp.h"
#include "HUD/ABlobHud.h"
#include "HUD/ABlobLetterBoxHud.h"
#include "ABlobInGame.h"
#include "Entities/APropTriggerHelp.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerProfileManager.h"
#include "GUI/AGUIManager.h"
#include "Input/ABlobInputMap.h"
#include "Locale/ALocaleManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"
#include "XUI/ABlobCursorTrail.h"
#include "XUI/ABlobMenu.h"
#include "XUI/ElementTypes/ABlobTextReveal.h"

#include <Input/TInputInterface.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIBackButton.h>
#include <XUI/TXUIInput.h>
#include <XUI/TXUITabScene.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobHelpFullScreen ) == 0x150 );

// The strings of the Next button: "Done" on the last page, "Next" on the others
static TCONSTEXPR_VAR TINT s_iDoneString = 0xDE;
static TCONSTEXPR_VAR TINT s_iNextString = 0xDF;

// The tab scene's button that turns the page
static TCONSTEXPR_VAR TINT s_iNextPageButton = TXUIElement::XUIButton_Right;

// The colour of the tab scene's background
static TCONSTEXPR_VAR TUINT32 s_uiTabBackgroundColour = 0xFFA8C8D3;

// The button that closed the help (cleared from the input of player 0 when the help is closed over the HUD)
static TCONSTEXPR_VAR TUINT32 s_uiCloseButton = 0x4;

// The page strings of each topic
static const TINT s_aiPages1[]  = { 523, 524, 525, 526, 560 };
static const TINT s_aiPages2[]  = { 528, 529, 530 };
static const TINT s_aiPages3[]  = { 500, 501, 502, 503 };
static const TINT s_aiPages4[]  = { 490, 491, 558 };
static const TINT s_aiPages5[]  = { 562, 563, 564, 565 };
static const TINT s_aiPages6[]  = { 493, 494, 495 };
static const TINT s_aiPages7[]  = { 519, 520, 521 };
static const TINT s_aiPages8[]  = { 497, 498 };
static const TINT s_aiPages9[]  = { 738, 739, 740 };
static const TINT s_aiPages10[] = { 562, 563, 565 };
static const TINT s_aiPages11[] = { 515, 516, 517 };
static const TINT s_aiPages12[] = { 537, 538, 539 };
static const TINT s_aiPages13[] = { 554, 555, 556, 557 };
static const TINT s_aiPages14[] = { 549, 550, 551, 552 };
static const TINT s_aiPages15[] = { 545, 546, 547 };
static const TINT s_aiPages16[] = { 541, 542, 543 };
static const TINT s_aiPages17[] = { 532, 533, 534, 535 };

const ABlobHelpFullScreen::PageStrings ABlobHelpFullScreen::paPageStrings[ NUM_TOPICS ] = {
	{ TNULL, 0 },
	{ s_aiPages1, TARRAYSIZE( s_aiPages1 ) },
	{ s_aiPages2, TARRAYSIZE( s_aiPages2 ) },
	{ s_aiPages3, TARRAYSIZE( s_aiPages3 ) },
	{ s_aiPages4, TARRAYSIZE( s_aiPages4 ) },
	{ s_aiPages5, TARRAYSIZE( s_aiPages5 ) },
	{ s_aiPages6, TARRAYSIZE( s_aiPages6 ) },
	{ s_aiPages7, TARRAYSIZE( s_aiPages7 ) },
	{ s_aiPages8, TARRAYSIZE( s_aiPages8 ) },
	{ s_aiPages9, TARRAYSIZE( s_aiPages9 ) },
	{ s_aiPages10, TARRAYSIZE( s_aiPages10 ) },
	{ s_aiPages11, TARRAYSIZE( s_aiPages11 ) },
	{ s_aiPages12, TARRAYSIZE( s_aiPages12 ) },
	{ s_aiPages13, TARRAYSIZE( s_aiPages13 ) },
	{ s_aiPages14, TARRAYSIZE( s_aiPages14 ) },
	{ s_aiPages15, TARRAYSIZE( s_aiPages15 ) },
	{ s_aiPages16, TARRAYSIZE( s_aiPages16 ) },
	{ s_aiPages17, TARRAYSIZE( s_aiPages17 ) },
};

// $deBlob: FUNCTION 00644960
// $deBlob: FUNCTION 00644970
// $deBlob: FUNCTION 006449e0
// $deBlob: FUNCTION 00644a50
// $deBlob: FUNCTION 00644a60
// $deBlob: FUNCTION 0053dc90
// $deBlob: FUNCTION 00858aa0
TDEFINE_CLASS( ABlobHelpFullScreen );

// $deBlob: FUNCTION 00644a70
ABlobHelpFullScreen::ABlobHelpFullScreen()
{
	m_pTrigger  = TNULL;
	m_pMessage  = TNULL;
	m_pContinue = TNULL;
	m_pNext     = TNULL;
	m_pPrev     = TNULL;
	m_pTabScene = TNULL;
	m_bMessage  = TFALSE;
	m_iTopic    = -1;
	m_bClosing  = TFALSE;
}

// $deBlob: FUNCTION 00644ae0
// $deBlob: FUNCTION 006459e0
// $deBlob: FUNCTION 006459ce
ABlobHelpFullScreen::~ABlobHelpFullScreen()
{
	APropTriggerHelp::CloseFullscreenHelp();

	if ( m_pPrevScene != TNULL && ( m_pPrevScene->IsA( &TGetClass( ABlobHud ) ) || m_pPrevScene->IsA( &TGetClass( ABlobLetterBoxHud ) ) ) )
		Unpause();
}

// $deBlob: FUNCTION 00644b80
void ABlobHelpFullScreen::OnCreated()
{
	// NOTE: retail doesn't call TXUIScene::OnCreated
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pState )->SetPaused( TTRUE );

	// NOTE: SetMessage is only called after the scene is created, so retail never finds these two (the message's
	// Continue button stays as the XUR has it)
	if ( m_bMessage )
	{
		m_pMessage  = TSTATICCAST( ABlobTextReveal, FindChildXUIControl( L"Message" ) );
		m_pContinue = FindChildXUIControl( L"Continue" );

		if ( m_pContinue != TNULL )
			m_pContinue->SetVisible( TFALSE );
	}

	m_pNext = FindChildXUIControl( L"Next" );
	m_pPrev = FindChildXUIControl( L"Prev" );

	if ( m_pPrev != TNULL )
		m_pPrev->SetVisible( TFALSE );

	m_pTabScene = TSTATICCAST( TXUITabScene, FindChildXUIControl( L"TabScene" ) );

	if ( m_pTabScene != TNULL )
	{
		m_pTabBackground = m_pTabScene->FindChildXUIElement( L"XuiNineGridColored" );

		if ( m_pTabBackground != TNULL )
			m_pTabBackground->SetColour( s_uiTabBackgroundColour );
	}

	if ( m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TFALSE );

	// NOTE: retail stores the fixed point scale 0x1660166
	SetScale( 1.4f, 1.4f );

	TFLOAT fWidth  = 0.0f;
	TFLOAT fHeight = 0.0f;
	GetDimensions( fWidth, fHeight );

	TXUIElement* pBg1 = FindChildXUIElement( L"bg1" );

	if ( pBg1 != TNULL )
		pBg1->SetPosition( pBg1->GetPositionX() - 250.0f, pBg1->GetPositionY() );

	TXUIElement* pBg2 = FindChildXUIElement( L"bg2" );

	if ( pBg2 != TNULL && fHeight != 1080.0f )
		pBg2->SetPosition( pBg2->GetPositionX() - 250.0f, pBg2->GetPositionY() );
}

// $deBlob: FUNCTION 00644dc0
TBOOL ABlobHelpFullScreen::HandleButton( TINT a_eButton, TINT a_iController )
{
	// NOTE: retail reads the tab scene before its TNULL check below
	if ( m_pTabScene->GetTimelineState() == State_TransTab )
		return TFALSE;

	if ( a_eButton != XUIButton_Accept )
		return TXUIScene::HandleButton( a_eButton, a_iController );

	if ( m_bMessage )
	{
		if ( m_pMessage != TNULL && m_pMessage->IsRevealing() )
		{
			m_pMessage->FinishReveal( TTRUE );

			if ( m_pContinue != TNULL )
				m_pContinue->SetVisible( TTRUE );

			return TTRUE;
		}

		TXUI::GetSingleton()->TransitionBack( this );
		return TTRUE;
	}

	if ( m_pTabScene == TNULL )
		return TTRUE;

	TUINT32 uiNumTabs    = m_pTabScene->GetNumTabs();
	TUINT32 uiCurrentTab = m_pTabScene->GetCurrentTab();

	if ( uiCurrentTab < uiNumTabs - 1 )
	{
		TXUIControl* pMessage = m_pTabScene->GetTab( uiCurrentTab + 1 )->FindChildXUIControl( L"Message" );

		if ( pMessage != TNULL && pMessage->IsA( &TGetClass( ABlobTextReveal ) ) )
			TSTATICCAST( ABlobTextReveal, pMessage )->FinishReveal( TFALSE );

		if ( uiCurrentTab == uiNumTabs - 2 && m_iTopic < NUM_CONFIRM_TOPICS )
		{
			m_oTimelineControl.PlayAnimation( L"Confirm", L"EndConfirm" );
			m_pNext->SetFocus( TTRUE, TFALSE, TTRUE );
			SetFocus( m_pNext, a_iController );
		}

		return m_pTabScene->HandleButton( s_iNextPageButton, a_iController );
	}

	if ( uiCurrentTab != uiNumTabs - 1 || m_iTopic < NUM_CONFIRM_TOPICS )
		return TTRUE;

	if ( m_pPrevScene != TNULL && m_pPrevScene->IsA( &TGetClass( ABlobLetterBoxHud ) ) )
	{
		CloseWithFade();
		return TTRUE;
	}

	TXUI::GetSingleton()->TransitionBack( this );
	return TTRUE;
}

void ABlobHelpFullScreen::CloseWithFade()
{
	SetTransitionVisual( Transition_BackFrom, L"TransNull" );
	m_bClosing = TTRUE;

	AGameStateController* pController = AGameStateController::GetSingleton();
	pController->SetFade( 0.0f, 0 );
	pController->SetRenderFadesWhenPaused( TTRUE );
	pController->FadeToBlack( 0.5f, 0 );

	TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
}

// $deBlob: FUNCTION 00644fd0
TBOOL ABlobHelpFullScreen::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( m_pTabScene->GetTimelineState() == State_TransTab )
		return TFALSE;

	ABlobMenu::PlayPressSound( a_pSource );

	if ( a_pSource == m_pPrev )
	{
		// Back to the first page
		m_pPrev->SetVisible( TFALSE );
		m_pNext->SetText( T2Locale::GetSingleton()->GetString( s_iNextString ), -1 );

		TXUIControl* pMessage = m_pTabScene->GetTab( 0 )->FindChildXUIControl( L"Message" );

		if ( pMessage != TNULL && pMessage->IsA( &TGetClass( ABlobTextReveal ) ) )
			TSTATICCAST( ABlobTextReveal, pMessage )->FinishReveal( TFALSE );

		m_oTimelineControl.PlayAnimation( L"EndTransTo", L"EndTransTo" );
		SetFocus( TNULL, a_iController );
		m_pPrev->SetFocus( TFALSE, TFALSE, TTRUE );
		return m_pTabScene->HandleButton( s_iNextPageButton, a_iController );
	}

	if ( a_pSource != m_pNext )
		return TFALSE;

	TUINT32 uiCurrentTab = m_pTabScene->GetCurrentTab();

	if ( uiCurrentTab == m_pTabScene->GetNumTabs() - 1 )
	{
		// The last page: the help is closed
		TXUIScene* pPrevScene = m_pPrevScene;
		ABlobCursorTrail::ResetAll();

		if ( pPrevScene != TNULL && pPrevScene->IsA( &TGetClass( ABlobLetterBoxHud ) ) )
		{
			CloseWithFade();
			return TTRUE;
		}

		TXUI::GetSingleton()->TransitionBack( this );
		return TTRUE;
	}

	TXUIControl* pMessage = m_pTabScene->GetTab( uiCurrentTab )->FindChildXUIControl( L"Message" );

	if ( pMessage != TNULL && pMessage->IsA( &TGetClass( ABlobTextReveal ) ) )
		TSTATICCAST( ABlobTextReveal, pMessage )->FinishReveal( TTRUE );

	pMessage = m_pTabScene->GetTab( m_pTabScene->GetCurrentTab() + 1 )->FindChildXUIControl( L"Message" );

	if ( pMessage != TNULL && pMessage->IsA( &TGetClass( ABlobTextReveal ) ) )
		TSTATICCAST( ABlobTextReveal, pMessage )->FinishReveal( TFALSE );

	return m_pTabScene->HandleButton( s_iNextPageButton, a_iController );
}

// $deBlob: FUNCTION 00645260
TBOOL ABlobHelpFullScreen::OnNotifySetFocus( TXUIElement* a_pSource )
{
	ABlobMenu::PlayFocusSound( a_pSource );
	return TFALSE;
}

// $deBlob: FUNCTION 00645280
TBOOL ABlobHelpFullScreen::OnNotifyValueChanged( TXUIElement* a_pSource )
{
	if ( a_pSource == m_pTabScene )
	{
		// The page changed: its text is revealed again
		TXUIControl* pMessage = m_pTabScene->GetTab( m_pTabScene->GetCurrentTab() )->FindChildXUIControl( L"Message" );

		if ( pMessage != TNULL && pMessage->IsA( &TGetClass( ABlobTextReveal ) ) )
			TSTATICCAST( ABlobTextReveal, pMessage )->RestartReveal();

		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006452e0
void ABlobHelpFullScreen::TransBackFrom()
{
	TXUIScene::TransBackFrom();

	TXUIScene* pPrevScene = m_pPrevScene;

	if ( pPrevScene != TNULL && pPrevScene->IsA( &TGetClass( ABlobLetterBoxHud ) ) )
		pPrevScene->FindChildXUIElement( 1 )->SetVisible( TTRUE );

	if ( m_pPrevScene != TNULL && m_pPrevScene->IsA( &TGetClass( ABlobHelp ) ) )
	{
		// The help menu is under the help: the cursor trails go back to the in-game scene with the next Tick
		ABlobInGame::SetRestoreCursorTrails();
		return;
	}

	ABlobInGame* pInGame = TSTATICCAST( ABlobInGame, TXUI::GetSingleton()->FindCachedScene( L"InGame" ) );
	pInGame->AttachCursorTrails();
	pInGame->SetCursorTrailsVisible( TFALSE );
}

// $deBlob: FUNCTION 00645370
void ABlobHelpFullScreen::EndTransTo()
{
	// The load transition shown while the TRB was streamed is deleted
	TXUIScene* pPrevScene = m_pPrevScene;

	if ( pPrevScene != TNULL && TStringManager::String16Compare( pPrevScene->GetID(), L"Transition", -1 ) == 0 )
	{
		m_pPrevScene = pPrevScene->GetPrevScene();
		delete pPrevScene;
	}

	ABlobInGame* pInGame = TSTATICCAST( ABlobInGame, TXUI::GetSingleton()->FindCachedScene( L"InGame" ) );
	pInGame->SetCursorTrailsVisible( TTRUE );

	TXUIScene::EndTransTo();
}

// $deBlob: FUNCTION 006453d0
void ABlobHelpFullScreen::EndTransBackFrom()
{
	if ( m_pPrevScene != TNULL && ( m_pPrevScene->IsA( &TGetClass( ABlobHud ) ) || m_pPrevScene->IsA( &TGetClass( ABlobLetterBoxHud ) ) ) )
	{
		AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
		TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
		AInGameState* pInGameState = TSTATICCAST( AInGameState, pState );

		if ( !pInGameState->IsWaitingToStart() )
		{
			pInGameState->UpdateMusicPause();
			pInGameState->SetPaused( TFALSE );
		}

		ABlobInputMap::GetSingleton()->SetButtonsHeld( 0, s_uiCloseButton );

		if ( m_pTrigger != TNULL )
			m_pTrigger->TriggerAllTargets( TNULL );
	}

	APropTriggerHelp::ResetHelpState();
	TXUIScene::EndTransBackFrom();
}

// $deBlob: FUNCTION 006454b0
void ABlobHelpFullScreen::TransTo()
{
	// The cursor trails of the in-game scene are shown over the help's background
	TXUIElement* pBackground = FindChildXUIElement( L"bg-gradient" );
	ABlobInGame* pInGame     = TSTATICCAST( ABlobInGame, TXUI::GetSingleton()->FindCachedScene( L"InGame" ) );

	if ( m_pPrevScene != TNULL && m_pPrevScene->IsA( &TGetClass( ABlobHelp ) ) )
	{
		// From the help menu: they're moved by the in-game scene's next Tick
		ABlobInGame::SetCursorTrailsParentLater( this, pBackground );
		pInGame->SetCursorTrailsVisible( TFALSE );
		TXUIScene::TransTo();
		return;
	}

	pInGame->AttachCursorTrailsTo( this, pBackground );
	pInGame->SetCursorTrailsVisible( TFALSE );
	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 00645540
void ABlobHelpFullScreen::SetMessage( const TWCHAR* a_wszMessage )
{
	if ( TStringManager::String16Compare( GetID(), L"HelpFullscreen", -1 ) == 0 )
	{
		TXUIControl* pText = FindChildXUIControl( 4 );

		if ( pText != TNULL )
			pText->SetText( a_wszMessage, -1 );
	}

	m_bMessage = TTRUE;
}

// $deBlob: FUNCTION 00645590
void ABlobHelpFullScreen::Tick( TFLOAT a_fDeltaTime )
{
	TXUI::GetSingleton()->TickInput( a_fDeltaTime );

	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( m_bClosing && !pController->IsFading( 0 ) && pController->GetFade( 0 ) >= 1.0f )
	{
		// Faded to black: the help is closed
		m_bClosing = TFALSE;
		AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TFALSE );
		TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
		TXUI::GetSingleton()->TransitionBack( this );
	}

	if ( m_bMessage )
	{
		if ( m_pMessage != TNULL && !m_pMessage->IsRevealing() && m_pContinue != TNULL )
			m_pContinue->SetVisible( TTRUE );
	}
	else if ( m_pTabScene->GetCurrentTab() == m_pTabScene->GetNumTabs() - 1 && m_pPrev != TNULL && !m_pPrev->IsVisible() )
	{
		// The last page: Prev goes back to the first page and Next closes the help
		m_pPrev->SetVisible( TTRUE );
		m_pNext->SetText( T2Locale::GetSingleton()->GetString( s_iDoneString ), -1 );
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00645ab0
void ABlobHelpFullScreen::SetTrigger( APropTriggerHelp* a_pTrigger )
{
	m_pTrigger = a_pTrigger;
}

// $deBlob: FUNCTION 006456d0
void ABlobHelpFullScreen::Pause()
{
	TInputInterface::GetSingleton()->StopAllRepeats();
	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pState )->SetPaused( TTRUE );

	AMusicManager* pMusicManager = AMusicManager::GetSingleton();

	if ( pMusicManager != TNULL && !pMusicManager->IsSilent() && !pMusicManager->IsInked() )
		pMusicManager->PlaySnapshot( "instruction" );

	ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID_INGAME, TTRUE );
}

// $deBlob: FUNCTION 00645780
void ABlobHelpFullScreen::Unpause()
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
	AInGameState* pInGameState = TSTATICCAST( AInGameState, pState );

	AMusicManager* pMusicManager = AMusicManager::GetSingleton();

	if ( pMusicManager != TNULL && !pMusicManager->IsSilent() && !pMusicManager->IsInked() )
		pMusicManager->ReleaseSnapshot();

	ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID_INGAME, TFALSE );
	AGUIManager::GetSingleton()->SetCursorVisible( TFALSE, TTRUE );

	if ( !pInGameState->IsWaitingToStart() )
	{
		pInGameState->UpdateMusicPause();
		pInGameState->SetPaused( TFALSE );
	}
}

// $deBlob: FUNCTION 00645830
void ABlobHelpFullScreen::SetTopic( TINT a_iTopic )
{
	m_iTopic = TINT8( a_iTopic );
	APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData()->SetHelpTopicSeen( a_iTopic );

	TUINT32 uiNumPages = m_pTabScene->GetNumTabs();

	if ( a_iTopic < NUM_CONFIRM_TOPICS )
	{
		// The last tab is the confirm page
		TASSERT( m_pTabScene->GetNumTabs() <= paPageStrings[ a_iTopic ].m_iNumPages + 1 );
		uiNumPages--;
	}

	ALocaleManager* pLocale = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );

	for ( TINT i = 0; i < TINT( uiNumPages ); i++ )
	{
		TXUIControl* pMessage = m_pTabScene->GetTab( i )->FindChildXUIControl( L"Message" );
		TINT         iString  = paPageStrings[ a_iTopic ].m_piStrings[ i ];

		ALocaleManager::StringFormat* pFormat = pLocale->GetStringFormat( iString );

		if ( pFormat != TNULL && pFormat->m_wszFormat != TNULL )
			pMessage->SetText2( pLocale->GetOverwrittenFormatString( iString, pFormat->m_wszFormat ), pLocale->GetFormatUnk04() );

		pMessage->SetText( pLocale->GetString( iString ), -1 );
	}
}

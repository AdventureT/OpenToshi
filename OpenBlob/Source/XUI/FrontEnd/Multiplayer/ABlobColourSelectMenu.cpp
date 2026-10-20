#include "pch.h"
#include "ABlobColourSelectMenu.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "GameInterface/APlayerManager.h"
#include "GUI/AGUIManager.h"
#include "GUI/AGUICursor.h"

#include <T2Locale/T2Locale.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIFigure.h>
#include <XUI/TXUIVisual.h>

#include <stdio.h>
#include <wchar.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobColourSelectMenu ) == 0x388 );

// The colour buttons, their "selected" markers and their colours (ABlobColourComponent)
static const TINT s_aiButtonUIDs[ ABlobColourSelectMenu::NUM_COLOURS ]   = { 16, 18, 17, 15, 19, 14, 13 };
static const TINT s_aiSelectedUIDs[ ABlobColourSelectMenu::NUM_COLOURS ] = { 10, 8, 9, 11, 7, 12, 6 };
static const TINT s_aiColours[ ABlobColourSelectMenu::NUM_COLOURS ]      = { 3, 5, 1, 2, 7, 6, 4 };

// The title texts
static TWCHAR s_wszTitle[ 0x100 ];
static TWCHAR s_wszTitlePressed[ 0x100 ];

// The components of the last colour (never read)
static TINT s_aiCreatedColour[ 4 ];
static TINT s_aiPressedColour[ 4 ];

// $deBlob: FUNCTION 0063dee0
// $deBlob: FUNCTION 0063def0
// $deBlob: FUNCTION 0063df60
// $deBlob: FUNCTION 0063dfd0
// $deBlob: FUNCTION 0063dfe0
// $deBlob: FUNCTION 0053da50
// $deBlob: FUNCTION 00858a10
TDEFINE_CLASS( ABlobColourSelectMenu );

// $deBlob: FUNCTION 0063dff0
ABlobColourSelectMenu::ABlobColourSelectMenu()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopMPGameType )
{
	m_pOK      = TNULL;
	m_uiUnk37C = 0;
	m_iPlayer  = 0;

	for ( TINT i = 0; i < NUM_COLOURS; i++ )
		m_apButtons[ i ] = TNULL;
}

// $deBlob: FUNCTION 0063e070
// $deBlob: FUNCTION 0063ecb8
// $deBlob: FUNCTION 0063ecc0
ABlobColourSelectMenu::~ABlobColourSelectMenu()
{
	AGUIManager::GetSingleton()->SetCursorActive( TTRUE );

	SetAcceptsAnyController( TFALSE );

	for ( TINT i = 0; i < 4; i++ )
		SetFocus( TNULL, i );

	TXUIScene* pParentScene = FindParentScene();

	if ( pParentScene != TNULL )
	{
		pParentScene->SetAcceptsAnyController( TFALSE );

		for ( TINT i = 0; i < 4; i++ )
			pParentScene->SetFocus( TNULL, i );
	}

	TXUI::GetSingleton()->SetDefaultController( 0 );
}

// $deBlob: FUNCTION 0063e910
void ABlobColourSelectMenu::Tick( TFLOAT a_fDeltaTime )
{
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() && !m_pOK->IsVisible() )
	{
		// Every player has chosen: "OK" shows
		TINT iNumChosen = 0;

		for ( TINT i = 0; i < NUM_COLOURS; i++ )
		{
			if ( m_apSelected[ i ]->IsVisible() )
				iNumChosen++;

			if ( iNumChosen == APlayerManager::GetSingleton()->GetNumPlayersInline() )
			{
				FindChildXUIControl( 20 )->PlayAnimation( L"ShowOk", L"EndShowOk" );
				break;
			}
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0063e460
void ABlobColourSelectMenu::OnCreated()
{
	m_iPlayer = 0;
	m_iUnk380 = 0;
	UpdateTitle( s_wszTitle );

	ABlobMenu::OnCreated();

	m_pOK     = FindChildXUIControl( 21 );
	m_iUnk380 = 0;

	for ( TINT i = 0; i < NUM_COLOURS; i++ )
	{
		m_apButtons[ i ]  = FindChildXUIControl( s_aiButtonUIDs[ i ] );
		m_apSelected[ i ] = FindChildXUIControl( s_aiSelectedUIDs[ i ] );
		m_apSelected[ i ]->SetVisible( TFALSE );

		// The yellow one has the focus
		if ( s_aiColours[ i ] == 3 )
			m_apFocus[ 0 ] = m_apButtons[ i ];

		TUINT32 uiColour       = ABlobColourComponent::ms_auiColours[ s_aiColours[ i ] ];
		s_aiCreatedColour[ 3 ] = ( uiColour >> 24 ) & 0xFF;
		s_aiCreatedColour[ 2 ] = uiColour & 0xFF;
		s_aiCreatedColour[ 1 ] = ( uiColour >> 8 ) & 0xFF;
		s_aiCreatedColour[ 0 ] = ( uiColour >> 16 ) & 0xFF;
		uiColour               = ( uiColour & 0xFF ) | ( ( ( ( uiColour >> 8 ) & 0xFF ) | ( ( ( ( uiColour >> 16 ) & 0xFF ) | ( ( uiColour >> 24 ) << 8 ) ) << 8 ) ) << 8 );

		TSTATICCAST( TXUIFigure, m_apButtons[ i ]->GetVisual()->FindChildXUIElement( L"button" ) )->SetFillColour( uiColour );
		TSTATICCAST( TXUIFigure, m_apSelected[ i ]->GetVisual()->FindChildXUIElement( L"button" ) )->SetFillColour( uiColour );
	}

	TINT iNumPlayers = APlayerManager::GetSingleton()->GetNumPlayers();

	for ( TINT i = 0; i < APlayerManager::GetSingleton()->GetNumPlayers(); i++ )
		APlayerManager::GetSingleton()->SetPlayerColour( i, 0 );

	AGUIManager::GetSingleton()->SetCursorVisibleCount( iNumPlayers );
	AGUIManager::GetSingleton()->SetCursorActiveCount( iNumPlayers );
}

// $deBlob: FUNCTION 0063e1c0
TBOOL ABlobColourSelectMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	// NOTE: retail's debug output
	printf( "OnPress(%d)\n", a_iController );

	if ( a_pSource == m_pOK )
	{
		TXUI* pXUI = TXUI::GetSingleton();
		TASSERT( a_iController == TXUI::GetSingleton()->GetDefaultController() );

		TXUIScene* pScene = pXUI->CreateScene( pXUI->FindResource( "multiplayerlevelselection.xur" ), TUINT32( 0 ) );
		FindParentScene()->AddChildTail( pScene );
		pXUI->TransitionTo( this, pScene );
		return TTRUE;
	}

	for ( TINT i = 0; i < NUM_COLOURS; i++ )
	{
		if ( a_pSource != m_apButtons[ i ] )
			continue;

		m_apButtons[ i ]->SetVisible( TFALSE );
		m_apSelected[ i ]->SetVisible( TTRUE );

		TUINT32 uiColour       = ABlobColourComponent::ms_auiColours[ s_aiColours[ i ] ];
		s_aiPressedColour[ 2 ] = uiColour & 0xFF;
		s_aiPressedColour[ 1 ] = ( uiColour >> 8 ) & 0xFF;
		s_aiPressedColour[ 0 ] = ( uiColour >> 16 ) & 0xFF;
		s_aiPressedColour[ 3 ] = ( uiColour >> 24 ) & 0xFF;

		// The player's label on the marker
		switch ( m_iPlayer )
		{
			case 0:
				m_apSelected[ i ]->SetText( T2Locale::GetSingleton()->GetString( 173 ), -1 );
				break;
			case 1:
				m_apSelected[ i ]->SetText( T2Locale::GetSingleton()->GetString( 174 ), -1 );
				break;
			case 2:
				m_apSelected[ i ]->SetText( T2Locale::GetSingleton()->GetString( 175 ), -1 );
				break;
			case 3:
				m_apSelected[ i ]->SetText( T2Locale::GetSingleton()->GetString( 176 ), -1 );
				break;
		}

		APlayerManager::GetSingleton()->SetPlayerColour( m_iPlayer, s_aiColours[ i ] );

		if ( ++m_iPlayer >= APlayerManager::GetSingleton()->GetNumPlayers() )
		{
			OnNotifyPress( m_pOK, a_iController );
			return TFALSE;
		}

		UpdateTitle( s_wszTitlePressed );
		HandleButton( XUIButton_Right, a_iController );
		AGUIManager::GetSingleton()->GetCursor()->SetMPColour( ABlobColourComponent::ms_auiColours[ s_aiColours[ i ] ] );
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0063ebe0
TBOOL ABlobColourSelectMenu::OnNotifySetFocus( TXUIElement* a_pSource )
{
	ABlobMenu::OnNotifySetFocus( a_pSource );

	for ( TINT i = 0; i < NUM_COLOURS; i++ )
		if ( a_pSource != m_apButtons[ i ] )
			m_apButtons[ i ]->SetFocus( TFALSE, TFALSE, TTRUE );
		else
			m_apButtons[ i ]->SetFocus( TTRUE, TFALSE, TTRUE );

	TSTATICCAST( TXUIControl, a_pSource )->SetFocus( TTRUE, TFALSE, TTRUE );
	return TFALSE;
}

// $deBlob: FUNCTION 0063e9c0
TBOOL ABlobColourSelectMenu::HandleButton( TINT a_eButton, TINT a_iController )
{
	// NOTE: retail's debug output
	printf( "HandleButtonDown(%d,%d)\n", a_eButton, a_iController );

	// The focussed button (visual state 2 or 9)
	TINT iButton;

	for ( iButton = 0;; iButton++ )
	{
		if ( iButton > 6 )
			return TXUIScene::HandleButton( a_eButton, a_iController );

		TUINT32 uiState = m_apButtons[ iButton ]->GetTimelineState();

		if ( uiState == TimelineState_Focus || uiState == TimelineState_TransTab )
			break;
	}

	// The grid: 0-3 above, 4-6 below
	switch ( a_eButton )
	{
		case XUIButton_Accept:
			OnNotifyPress( m_apButtons[ iButton ], a_iController );
			break;
		case XUIButton_Up:
		case XUIButton_Down:
			if ( iButton < 4 )
				iButton = Wrap( iButton + 4, 7 );
			else
				iButton = Wrap( iButton - 4, 4 );

			if ( m_apButtons[ iButton ]->IsVisible() )
				HandleButton( XUIButton_Left, a_iController );

			OnNotifySetFocus( m_apButtons[ iButton ] );
			break;
		case XUIButton_Left:
			do
			{
				if ( iButton < 4 )
					iButton = Wrap( iButton - 1, 4 );
				else
					iButton = Wrap( iButton - 5, 3 ) + 4;
			} while ( !m_apButtons[ iButton ]->IsVisible() );

			OnNotifySetFocus( m_apButtons[ iButton ] );
			break;
		case XUIButton_Right:
			do
			{
				if ( iButton < 4 )
					iButton = Wrap( iButton + 1, 4 );
				else
					iButton = Wrap( iButton - 3, 3 ) + 4;
			} while ( !m_apButtons[ iButton ]->IsVisible() );

			OnNotifySetFocus( m_apButtons[ iButton ] );
			break;
		default:
			return TXUIScene::HandleButton( a_eButton, a_iController );
	}

	FindParentScene()->SetFocus( m_apButtons[ iButton ], a_iController );
	return TTRUE;
}

// $deBlob: FUNCTION 0063e660
void ABlobColourSelectMenu::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransMPPlayerMPColour, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	pFrontEnd->SlideLeft();

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 0063e650
void ABlobColourSelectMenu::TransFrom()
{
	ABlobMenu::TransFrom();
}

// $deBlob: FUNCTION 0063e6d0
void ABlobColourSelectMenu::TransBackTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TINT           iMovie    = pFrontEnd->GetRandomBackgroundMovie( ABlobFrontEnd::BGMOVIE_BackGen01 );

	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( iMovie, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );


	for ( TINT i = 0; i < APlayerManager::GetSingleton()->GetNumPlayersInline(); i++ )
	{
		APlayerManager::GetSingleton()->SetPlayerColour( i, 0 );
		AGUIManager::GetSingleton()->GetCursor()->SetMPColourFlag( TFALSE );
	}

	for ( TINT i = 0; i < NUM_COLOURS; i++ )
	{
		m_apButtons[ i ]->SetVisible( TTRUE );
		m_apSelected[ i ]->SetVisible( TFALSE );
		m_apButtons[ i ]->SetFocus( TFALSE, TFALSE, TTRUE );
		m_apSelected[ i ]->SetFocus( TFALSE, TFALSE, TTRUE );
	}

	TXUIScene* pOKScene = TSTATICCAST( TXUIScene, FindChildXUIControl( 20 ) );
	pOKScene->PlayAnimation( L"Normal", L"EndNormal" );
	pOKScene->SetFocus( TNULL, 0 );

	m_pOK->SetFocus( TFALSE, TFALSE, TTRUE );

	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 0063e840
void ABlobColourSelectMenu::TransBackFrom()
{
	TXUIScene* pParentScene = FindParentScene();

	if ( pParentScene != TNULL && pParentScene->IsA( &TGetClass( ABlobFrontEnd ) ) )
		TSTATICCAST( ABlobFrontEnd, pParentScene )->ResetCursorTrails();

	ABlobMenu::TransBackFrom();
}

// $deBlob: FUNCTION 0063e8e0
void ABlobColourSelectMenu::EndTransTo()
{
	AGUIManager::GetSingleton()->SetCursorVisibleCount( APlayerManager::GetSingleton()->GetNumPlayersInline() );
	ABlobMenu::EndTransTo();
}

// $deBlob: FUNCTION 0063e880
void ABlobColourSelectMenu::EndTransBackTo()
{
	RestartChoice();
	ABlobMenu::EndTransBackTo();
}

// $deBlob: FUNCTION 0063ec40
void ABlobColourSelectMenu::RestartChoice()
{
	m_iPlayer = 0;
	m_iUnk380 = 0;
	UpdateTitle( s_wszTitle );
}

void ABlobColourSelectMenu::UpdateTitle( TWCHAR* a_wszBuffer )
{
	TXUIControl* pTitle = FindChildXUIControl( 5 );

	if ( pTitle != TNULL )
	{
		swprintf( a_wszBuffer, 0x100, L"CHOOSE COLOR P%d", m_iPlayer + 1 );
		pTitle->SetText( a_wszBuffer, -1 );
	}
}

// $deBlob: FUNCTION 0063eca0
TINT ABlobColourSelectMenu::Wrap( TINT a_iValue, TINT a_iCount )
{
	return ( a_iCount + a_iValue % a_iCount ) % a_iCount;
}

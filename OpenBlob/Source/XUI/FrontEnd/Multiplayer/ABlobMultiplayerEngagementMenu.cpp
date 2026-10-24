#include "pch.h"
#include "ABlobMultiplayerEngagementMenu.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "GUI/AGUIManager.h"
#include "GUI/AGUICursor.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIFigure.h>
#include <XUI/TXUIVisual.h>
#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <Platform/Windows/Input/TInputDeviceController_Wiin.h>
#include <T2Locale/T2Locale.h>

#include <stdio.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The controls of the scene per slot
static const TINT s_aiJoinPanels[ ABlobMultiplayerEngagementMenu::NUM_SLOTS ]                                                  = { 90, 87, 84, 81 }; // "press to join"
static const TINT s_aiJoinedPanels[ ABlobMultiplayerEngagementMenu::NUM_SLOTS ]                                                = { 61, 41, 21, 1 };  // the colour choice
static const TINT s_aiColourButtons[ ABlobMultiplayerEngagementMenu::NUM_SLOTS * ABlobMultiplayerEngagementMenu::NUM_COLOURS ] = {
	74,
	76,
	75,
	73,
	77,
	72,
	71,
	54,
	56,
	55,
	53,
	57,
	52,
	51,
	34,
	36,
	35,
	33,
	37,
	32,
	31,
	14,
	16,
	15,
	13,
	17,
	12,
	11,
};
static const TINT s_aiSelectedColourButtons[ ABlobMultiplayerEngagementMenu::NUM_SLOTS * ABlobMultiplayerEngagementMenu::NUM_COLOURS ] = {
	68,
	66,
	67,
	69,
	65,
	70,
	64,
	48,
	46,
	47,
	49,
	45,
	50,
	44,
	28,
	26,
	27,
	29,
	25,
	30,
	24,
	8,
	6,
	7,
	9,
	5,
	10,
	4,
};
static const TINT s_aiColours[ ABlobMultiplayerEngagementMenu::NUM_COLOURS ]     = { 3, 5, 1, 2, 7, 6, 4 }; // ABlobColourComponent's colours of the buttons
static const TINT s_aiStartButtons[ ABlobMultiplayerEngagementMenu::NUM_SLOTS ]  = { 79, 59, 39, 19 };
static const TINT s_aiSlotLabels[ ABlobMultiplayerEngagementMenu::NUM_SLOTS ]    = { 63, 43, 23, 3 };
static const TINT s_aiJoinTexts[ ABlobMultiplayerEngagementMenu::NUM_SLOTS ]     = { 92, 89, 86, 83 }; // blinking
static const TINT s_aiTakenWarnings[ ABlobMultiplayerEngagementMenu::NUM_SLOTS ] = { 80, 60, 40, 20 };

// The rest position of the start button isn't known yet
static TCONSTEXPR_VAR TFLOAT START_Y_UNSET = -9999.0f;

// a_iValue modulo a_iMod, positive
// $deBlob: FUNCTION 00642350
static TINT WrapIndex( TINT a_iValue, TINT a_iMod )
{
	return ( a_iMod + a_iValue % a_iMod ) % a_iMod;
}

// The text of a joined slot: "Player P1 - Keyboard", "Player P1 - Controller 3", "Player P1 - Wiimote 1"
static void FormatSlotText( TWCHAR* a_wszText, TINT a_iSlot, TInputDevice* a_pDevice )
{
	T2Locale* pLocale = T2Locale::GetSingleton();

	if ( a_pDevice->IsA( &TGetClass( TInputDeviceKeyboard ) ) )
	{
		swprintf( a_wszText, 256, L"%ls P%d - %ls", pLocale->GetString( 457 ), a_iSlot + 1, pLocale->GetString( 859 ) );
	}
	else if ( a_pDevice->IsA( &TGetClass( TInputDeviceController ) ) )
	{
		if ( a_pDevice->GetGamepadType() == TInputDevice::Gamepad_Wii )
			swprintf( a_wszText, 256, L"%ls P%d - Wiimote %d", pLocale->GetString( 457 ), a_iSlot + 1, TSTATICCAST( TInputWiinDeviceController, a_pDevice )->GetIndex() );
		else
			swprintf( a_wszText, 256, L"%ls P%d - %ls %d", pLocale->GetString( 457 ), a_iSlot + 1, pLocale->GetString( 858 ), a_pDevice->GetDeviceIndex() );
	}
	else
	{
		swprintf( a_wszText, 256, L"%ls P%d", pLocale->GetString( 457 ), a_iSlot + 1 );
	}
}

// $deBlob: FUNCTION 0063fea0
// $deBlob: FUNCTION 0063feb0
// $deBlob: FUNCTION 0063ff20
// $deBlob: FUNCTION 0063ff90
// $deBlob: FUNCTION 0063ffa0
// $deBlob: FUNCTION 0053db50
// $deBlob: FUNCTION 00858a50
TDEFINE_CLASS( ABlobMultiplayerEngagementMenu );

// $deBlob: FUNCTION 0063ffb0
ABlobMultiplayerEngagementMenu::ABlobMultiplayerEngagementMenu()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopMPGameType )
{
	m_pStart       = TNULL;
	m_iEntryDevice = TInputDevice::GetPlayerDeviceIndices()[ 0 ];
	TInputDevice::SavePlayerDevices();

	TUtil::MemClear( m_apColours, sizeof( m_apColours ) );
	TUtil::MemClear( m_apSelectedColours, sizeof( m_apSelectedColours ) );

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		m_aiDevice[ i ] = -1;
		m_aiColour[ i ] = -1;
		m_aiFocus[ i ]  = -1;
	}

	m_fBlinkAlpha       = 1.0f;
	m_bBlinkIn          = TFALSE;
	m_fStartY           = START_Y_UNSET;
	m_fStartBounceDelay = 0.0f;
	m_fStartBounceSpeed = 0.0f;
	m_bStartBouncing    = TFALSE;
	swprintf( m_wszStartText, 256, L"" );

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		m_apTakenWarnings[ i ]    = TNULL;
		m_afTakenWarningTime[ i ] = 0.0f;
	}
}

// $deBlob: FUNCTION 006400e0
// $deBlob: FUNCTION 00642440
// $deBlob: FUNCTION 0064242e
ABlobMultiplayerEngagementMenu::~ABlobMultiplayerEngagementMenu()
{
	AGUIManager::GetSingleton()->SetCursorActive( TTRUE );

	SetAcceptsAnyController( TFALSE );

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
		SetFocus( TNULL, i );

	TXUIScene* pParent = FindParentScene();

	if ( pParent != TNULL )
	{
		pParent->SetAcceptsAnyController( TFALSE );

		for ( TINT i = 0; i < NUM_SLOTS; i++ )
			pParent->SetFocus( TNULL, i );
	}

	TInputDevice::SetPlayerDevicesLocked( TFALSE );
	TUtil::Log( "NFO: Multiplayer flag disabled\n" );
	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );
	TXUI::GetSingleton()->SetDefaultController( 0 );
}

// $deBlob: FUNCTION 00640250
TBOOL ABlobMultiplayerEngagementMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( a_pSource == m_pStart )
	{
		TXUIScene* pLevelSelection = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "multiplayerlevelselection.xur" ), 0 );
		FindParentScene()->AddChildTail( pLevelSelection );
		TXUI::GetSingleton()->TransitionTo( this, pLevelSelection );

		printf( "NFO: Multiplayer flag disabled\n" );
		TInputDevice::SetPlayerDevicesLocked( TFALSE );
		AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );

		// The players' colours and count (the slots are filled from the first, see RemoveDevice)
		TINT iNumPlayers = 1;
		APlayerManager::GetSingleton()->SetPlayerColour( 0, s_aiColours[ m_aiColour[ 0 ] ] );

		if ( m_aiColour[ 1 ] >= 0 )
		{
			iNumPlayers = 2;
			APlayerManager::GetSingleton()->SetPlayerColour( 1, s_aiColours[ m_aiColour[ 1 ] ] );
		}

		if ( m_aiColour[ 2 ] >= 0 )
		{
			iNumPlayers++;
			APlayerManager::GetSingleton()->SetPlayerColour( 2, s_aiColours[ m_aiColour[ 2 ] ] );
		}

		if ( m_aiColour[ 3 ] >= 0 )
		{
			iNumPlayers++;
			APlayerManager::GetSingleton()->SetPlayerColour( 3, s_aiColours[ m_aiColour[ 3 ] ] );
		}

		APlayerManager::GetSingleton()->SetNumPlayers( iNumPlayers );
		AGUIManager::GetSingleton()->SetCursorVisibleCount( iNumPlayers );
		AGUIManager::GetSingleton()->SetCursorActiveCount( iNumPlayers );
		return TTRUE;
	}

	// The pressed colour of a slot choosing one is shown as chosen
	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		if ( a_iController != m_aiDevice[ i ] )
			continue;

		for ( TINT j = 0; j < NUM_COLOURS; j++ )
		{
			TXUIControl* pColour = m_apColours[ i ][ j ];

			if ( a_pSource == pColour && m_aiState[ i ] == SlotState_Colour && a_pSource->IsVisible() )
			{
				pColour->SetVisible( TFALSE );
				m_apSelectedColours[ i ][ j ]->SetVisible( TTRUE );
			}
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00640440
void ABlobMultiplayerEngagementMenu::OnCreated()
{
	Reset();
	ABlobMenu::OnCreated();

	// NOTE: every slot's start button is hidden, the last one found (slot 0's) is kept
	for ( TINT i = NUM_SLOTS - 1; i >= 0; i-- )
	{
		m_pStart = FindChildXUIControl( s_aiStartButtons[ i ] );
		m_pStart->SetVisible( TFALSE );
	}

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		m_apTakenWarnings[ i ] = FindChildXUIElement( s_aiTakenWarnings[ i ] );
		m_apTakenWarnings[ i ]->SetOpacity( 0.0f );

		for ( TINT j = 0; j < NUM_COLOURS; j++ )
		{
			TINT iButton = i * NUM_COLOURS + j;

			m_apColours[ i ][ j ]         = FindChildXUIControl( s_aiColourButtons[ iButton ] );
			m_apSelectedColours[ i ][ j ] = FindChildXUIControl( s_aiSelectedColourButtons[ iButton ] );
			m_apSelectedColours[ i ][ j ]->SetVisible( TFALSE );

			// NOTE: retail splits the colour into four static bytes and puts it back together
			TUINT32 uiColour = ABlobColourComponent::ms_auiColours[ s_aiColours[ j ] ];

			TSTATICCAST( TXUIFigure, m_apColours[ i ][ j ]->GetVisual()->FindChildXUIElement( L"button" ) )->SetFillColour( uiColour );
			TSTATICCAST( TXUIFigure, m_apSelectedColours[ i ][ j ]->GetVisual()->FindChildXUIElement( L"button" ) )->SetFillColour( uiColour );
		}
	}
}

// $deBlob: FUNCTION 006405d0
void ABlobMultiplayerEngagementMenu::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransMPPlayerMPColour, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	pFrontEnd->SlideLeft();

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 006405c0
void ABlobMultiplayerEngagementMenu::TransFrom()
{
	ABlobMenu::TransFrom();
}

// $deBlob: FUNCTION 00640640
void ABlobMultiplayerEngagementMenu::TransBackTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TINT           iMovie    = pFrontEnd->GetRandomBackgroundMovie( ABlobFrontEnd::BGMOVIE_BackGen01 );

	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( iMovie, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );

	for ( TINT i = 0; i < APlayerManager::MAX_PLAYERS; i++ )
	{
		APlayerManager::GetSingleton()->SetPlayerColour( i, 0 );
		AGUIManager::GetSingleton()->GetCursor()->SetMPColourFlag( TFALSE );
	}

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
		ResetSlot( i );

	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 00640740
void ABlobMultiplayerEngagementMenu::TransBackFrom()
{
	TXUIScene* pParent = FindParentScene();

	if ( pParent != TNULL && pParent->IsA( &TGetClass( ABlobFrontEnd ) ) )
	{
		ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, pParent );
		pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransMPPlayerMPColour, ABlobFrontEnd::BGMOVIE_LoopMainMenu, TFALSE, TFALSE, TFALSE );
		pFrontEnd->SlideRight();
		pFrontEnd->ResetCursorTrails();
	}

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
		ResetSlot( i );

	ABlobMenu::TransBackFrom();
}

// $deBlob: FUNCTION 006407a0
void ABlobMultiplayerEngagementMenu::EndTransBackTo()
{
	Reset();
	ABlobMenu::EndTransBackTo();
}

// $deBlob: FUNCTION 006407b0
void ABlobMultiplayerEngagementMenu::EndTransTo()
{
	SetAcceptsAnyController( TTRUE );

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
		SetFocus( TNULL, i );

	TXUIScene* pParent = FindParentScene();

	if ( pParent != TNULL )
	{
		pParent->SetAcceptsAnyController( TTRUE );

		for ( TINT i = 0; i < NUM_SLOTS; i++ )
			pParent->SetFocus( TNULL, i );
	}

	AGUIManager::GetSingleton()->SetCursorVisibleCount( 0 );
	ABlobMenu::EndTransTo();
}

// $deBlob: FUNCTION 00640890
void ABlobMultiplayerEngagementMenu::Tick( TFLOAT a_fDeltaTime )
{
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() )
	{
		// The slots whose colour another slot has too
		TBOOL abSameColour[ NUM_SLOTS ] = { TFALSE, TFALSE, TFALSE, TFALSE };
		TBOOL bSameColour               = TFALSE;

		for ( TINT i = 0; i < NUM_SLOTS; i++ )
		{
			for ( TINT j = i + 1; j < NUM_SLOTS; j++ )
			{
				if ( m_aiColour[ i ] == m_aiColour[ j ] && m_aiColour[ i ] >= 0 )
				{
					abSameColour[ i ] = TTRUE;
					abSameColour[ j ] = TTRUE;
					bSameColour       = TTRUE;
				}
			}
		}

		// Their chosen colour spins
		for ( TINT i = 0; i < NUM_SLOTS; i++ )
		{
			if ( !abSameColour[ i ] )
			{
				for ( TINT j = 0; j < NUM_COLOURS; j++ )
				{
					m_apSelectedColours[ i ][ j ]->SetZRotation( 0.0f );
					m_apSelectedColours[ i ][ j ]->SetScale( 1.0f, 0.95f );
				}
			}
			else
			{
				TXUIControl* pSelected = m_apSelectedColours[ i ][ m_aiColour[ i ] ];
				pSelected->SetZRotation( a_fDeltaTime * 16.0f + pSelected->GetZRotation() );
				pSelected->SetScale( 0.8f, 0.75f );
			}
		}

		// The start button: the first player and every other joined one are ready, two players at least
		if ( !bSameColour && FindChildXUIElement( s_aiJoinedPanels[ 0 ] )->IsVisible() &&
		     ( FindChildXUIElement( s_aiJoinedPanels[ 1 ] )->IsVisible() || FindChildXUIElement( s_aiJoinedPanels[ 2 ] )->IsVisible() || FindChildXUIElement( s_aiJoinedPanels[ 3 ] )->IsVisible() ) &&
		     m_aiState[ 0 ] == SlotState_Ready &&
		     ( m_aiDevice[ 1 ] < 0 || m_aiState[ 1 ] == SlotState_Ready ) &&
		     ( m_aiDevice[ 2 ] < 0 || m_aiState[ 2 ] == SlotState_Ready ) &&
		     ( m_aiDevice[ 3 ] < 0 || m_aiState[ 3 ] == SlotState_Ready ) )
		{
			if ( !m_pStart->IsVisible() )
			{
				m_pStart->SetVisible( TTRUE );
				FindChildXUIControl( 78 )->PlayAnimation( L"ShowOk", L"EndShowOk" );
				m_pStart->SetFocus( TTRUE, TTRUE, TTRUE );
			}
		}
		else if ( m_pStart->IsVisible() )
		{
			m_pStart->SetVisible( TFALSE );

			TFLOAT fStartY      = m_fStartY;
			m_fStartBounceDelay = 0.0f;
			m_fStartBounceSpeed = 0.0f;
			m_bStartBouncing    = TFALSE;

			if ( fStartY != START_Y_UNSET )
				m_pStart->SetPosition( m_pStart->GetPositionX(), fStartY );
		}

		// A controller pressing accept joins, a disabled one leaves
		TINT iNumDevices = TInputInterface::GetSingleton()->GetDeviceList().Count();

		for ( TINT i = 0; i < iNumDevices; i++ )
		{
			TInputDeviceController* pController = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceController>( i );

			if ( pController == TNULL )
				continue;

			if ( !pController->IsEnabled() )
			{
				RemoveDevice( pController );
			}
			else
			{
				TINT iDevice = pController->GetDeviceIndex();

				if ( m_aiDevice[ 0 ] != iDevice && m_aiDevice[ 1 ] != iDevice && m_aiDevice[ 2 ] != iDevice && m_aiDevice[ 3 ] != iDevice &&
				     pController->IsDown( TInputDeviceController::BUTTON_CONTEXT_1 ) )
				{
					AddDevice( pController );
					HandleButton( XUIButton_Accept, pController->GetDeviceIndex() );
				}
			}
		}

		// The "join" texts blink
		if ( !m_bBlinkIn )
		{
			m_fBlinkAlpha -= a_fDeltaTime;

			if ( m_fBlinkAlpha <= 0.5f )
			{
				m_fBlinkAlpha = 0.5f;
				m_bBlinkIn    = TTRUE;
			}
		}
		else
		{
			m_fBlinkAlpha += a_fDeltaTime;

			if ( 1.0f <= m_fBlinkAlpha )
			{
				m_fBlinkAlpha = 1.0f;
				m_bBlinkIn    = TFALSE;
			}
		}

		// The start button bounces every two seconds
		if ( m_pStart->IsVisible() )
		{
			if ( m_fStartY == START_Y_UNSET )
				m_fStartY = m_pStart->GetPositionY();

			TFLOAT fX = m_pStart->GetPositionX();
			TFLOAT fY = m_pStart->GetPositionY();

			if ( !m_bStartBouncing )
			{
				if ( 0.0f < m_fStartBounceDelay )
				{
					m_fStartBounceDelay -= a_fDeltaTime;
				}
				else
				{
					m_fStartBounceSpeed = -150.0f;
					m_bStartBouncing    = TTRUE;
				}
			}

			if ( m_bStartBouncing )
			{
				TFLOAT fNewY = m_fStartBounceSpeed * a_fDeltaTime + fY;
				m_pStart->SetPosition( fX, fNewY );
				m_fStartBounceSpeed = a_fDeltaTime * 3.0f * 150.0f + m_fStartBounceSpeed;

				TFLOAT fStartY = m_fStartY;

				if ( fStartY <= fNewY )
				{
					m_fStartBounceSpeed = 0.0f;
					m_bStartBouncing    = TFALSE;
					m_fStartBounceDelay = 2.0f;
					m_pStart->SetPosition( fX, fStartY );
				}
			}
		}

		for ( TINT i = 0; i < NUM_SLOTS; i++ )
		{
			// The "colour taken" warning fades out
			if ( m_afTakenWarningTime[ i ] <= 0.0f )
			{
				TFLOAT fOpacity = m_apTakenWarnings[ i ]->GetFlags_Opacity();

				if ( 0.0f < fOpacity )
				{
					fOpacity -= a_fDeltaTime;

					if ( fOpacity < 0.0f )
						fOpacity = 0.0f;

					m_apTakenWarnings[ i ]->SetOpacity( fOpacity );
				}
			}
			else
			{
				m_afTakenWarningTime[ i ] -= a_fDeltaTime;
			}

			// The focus follows m_aiFocus (the mouse can focus another colour)
			TINT iFocused;

			for ( iFocused = 0; iFocused < NUM_COLOURS; iFocused++ )
			{
				TUINT32 uiState = m_apColours[ i ][ iFocused ]->GetTimelineState();

				if ( uiState == TimelineState_Focus || uiState == TimelineState_TransTab )
					break;
			}

			if ( iFocused == NUM_COLOURS )
				iFocused = 8;

			if ( m_aiFocus[ i ] >= 0 && m_aiFocus[ i ] != iFocused )
				FocusColour( m_aiFocus[ i ], i );

			TXUIControl* pJoinText = FindChildXUIControl( s_aiJoinTexts[ i ] );

			if ( pJoinText != TNULL )
				pJoinText->SetOpacity( m_fBlinkAlpha );
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00640fb0
TBOOL ABlobMultiplayerEngagementMenu::HandleButton( TINT a_eButton, TINT a_iController )
{
	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	// Back with no player leaves the menu
	if ( m_aiDevice[ 3 ] < 0 && m_aiDevice[ 2 ] < 0 && m_aiDevice[ 1 ] < 0 && m_aiDevice[ 0 ] < 0 && a_eButton == XUIButton_Back &&
	     ( a_iController == pKeyboard->GetDeviceIndex() || a_iController == m_iEntryDevice ) )
	{
		printf( "Player %d state: EXIT\n", a_iController );
		return TXUIScene::HandleButton( XUIButton_Back, TXUI::GetSingleton()->GetDefaultController() );
	}

	// The keyboard joins with accept
	if ( a_iController == pKeyboard->GetDeviceIndex() && a_eButton == XUIButton_Accept && pKeyboard->GetPlayer() < 0 )
		AddDevice( pKeyboard );

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		if ( m_aiDevice[ i ] != a_iController )
			continue;

		const TCHAR* szState = TNULL;

		switch ( a_eButton )
		{
			case XUIButton_Accept:
				if ( m_aiState[ i ] == SlotState_Engagement )
				{
					TInputDevice* pController = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDevice>( a_iController );
					TASSERT( pController );
					AddDevice( pController );
					m_aiState[ i ] = SlotState_Colour;
					szState        = "Player %d state: COLOR\n";
				}
				else if ( m_aiState[ i ] == SlotState_Colour )
				{
					TXUIControl* pColour = m_apColours[ i ][ m_aiFocus[ i ] ];

					if ( pColour->GetFlags_Opacity() == 1.0f )
					{
						OnNotifyPress( pColour, a_iController );
						m_aiState[ i ]  = SlotState_Ready;
						m_aiColour[ i ] = m_aiFocus[ i ];
						UpdateTakenColours();
						szState = "Player %d state: READY\n";
					}
					else
					{
						// Another slot has the colour
						m_apTakenWarnings[ i ]->SetPosition( pColour->GetPositionX(), pColour->GetPositionY() );
						m_apTakenWarnings[ i ]->SetOpacity( 1.0f );
						m_afTakenWarningTime[ i ] = 0.3f;
						szState                   = "Player %d tried to select a yet selected color\n";
					}
				}
				else
				{
					if ( !m_pStart->IsVisible() || i != 0 )
						break;

					OnNotifyPress( m_pStart, a_iController );
					szState = "Player %d state: START!\n";
				}

				printf( szState, i );
				break;
			case XUIButton_Back:
				if ( m_aiState[ i ] == SlotState_Ready )
				{
					m_aiState[ i ] = SlotState_Colour;
					printf( "Player %d state: COLOR\n", i );

					for ( TINT j = 0; j < NUM_COLOURS; j++ )
					{
						m_apColours[ i ][ j ]->SetVisible( TTRUE );
						m_apSelectedColours[ i ][ j ]->SetVisible( TFALSE );
					}

					m_aiColour[ i ] = -1;
					UpdateTakenColours();
				}
				else
				{
					if ( m_aiState[ i ] == SlotState_Colour )
					{
						m_aiState[ i ] = SlotState_Engagement;

						TInputDevice* pController = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDevice>( a_iController );
						TASSERT( pController );
						RemoveDevice( pController );
						szState = "Player %d state: ENGAGEMENT\n";
					}
					else
					{
						TInputDevice* pController = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDevice>( a_iController );
						TASSERT( pController );
						RemoveDevice( pController );
						szState = "Player %d state: EXIT\n";
					}

					printf( szState, i );
				}
				break;
			case XUIButton_Up:
			case XUIButton_Down:
				// Up and down: the colours are in a row of four over a row of three
				if ( m_aiState[ i ] == SlotState_Colour )
				{
					TINT iFocus = m_aiFocus[ i ];

					if ( iFocus == 3 )
					{
						m_aiFocus[ i ] = 6;
						FocusColour( m_aiFocus[ i ], i );
					}
					else
					{
						if ( iFocus < 4 )
							iFocus = WrapIndex( iFocus + 4, 7 );
						else
							iFocus = WrapIndex( iFocus - 4, 4 );

						m_aiFocus[ i ] = iFocus;
						FocusColour( iFocus, i );
					}
				}
				break;
			case XUIButton_Left:
				if ( m_aiState[ i ] == SlotState_Colour )
				{
					TINT iFocus;

					do
					{
						if ( m_aiFocus[ i ] < 4 )
							iFocus = WrapIndex( m_aiFocus[ i ] - 1, 4 );
						else
							iFocus = WrapIndex( m_aiFocus[ i ] - 5, 3 ) + 4;

						m_aiFocus[ i ] = iFocus;
					} while ( !m_apColours[ i ][ iFocus ]->IsVisible() );

					FocusColour( iFocus, i );
				}
				break;
			case XUIButton_Right:
				if ( m_aiState[ i ] == SlotState_Colour )
				{
					TINT iFocus;

					do
					{
						if ( m_aiFocus[ i ] < 4 )
							iFocus = WrapIndex( m_aiFocus[ i ] + 1, 4 );
						else
							iFocus = WrapIndex( m_aiFocus[ i ] - 3, 3 ) + 4;

						m_aiFocus[ i ] = iFocus;
					} while ( !m_apColours[ i ][ iFocus ]->IsVisible() );

					FocusColour( iFocus, i );
				}
				break;
			default:
				return TXUIScene::HandleButton( a_eButton, a_iController );
		}

		FindParentScene()->SetFocus( m_apColours[ i ][ m_aiFocus[ i ] ], i );
		FocusColour( m_aiFocus[ i ], i );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006415e0
TBOOL ABlobMultiplayerEngagementMenu::OnNotifySetFocus( TXUIElement* a_pSource )
{
	return ABlobMenu::OnNotifySetFocus( a_pSource );
}

// $deBlob: FUNCTION 006415f0
void ABlobMultiplayerEngagementMenu::Reset()
{
	printf( "NFO: Multiplayer flag enabled\n" );
	TInputDevice::SetPlayerDevicesLocked( TTRUE );
	AGUIManager::GetSingleton()->SetCursorVisible( TFALSE, TFALSE );
	TInputDevice::ResetPlayerDevices();

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		m_aiDevice[ i ] = -1;
		m_aiColour[ i ] = -1;
	}

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
		m_aiState[ i ] = SlotState_Engagement;

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		ResetSlot( i );

		TXUIControl* pJoinText = FindChildXUIControl( s_aiJoinTexts[ i ] );

		if ( pJoinText != TNULL )
			pJoinText->SetText( T2Locale::GetSingleton()->GetString( 874 ), -1 );

		TXUIElement* pElement = FindChildXUIElement( s_aiJoinedPanels[ i ] );

		if ( pElement != TNULL )
			pElement->SetVisible( TFALSE );

		pElement = FindChildXUIElement( s_aiJoinPanels[ i ] );

		if ( pElement != TNULL )
			pElement->SetVisible( TTRUE );

		pElement = FindChildXUIElement( s_aiTakenWarnings[ i ] );

		if ( pElement != TNULL )
			pElement->SetOpacity( 0.0f );
	}

	TFLOAT fStartY      = m_fStartY;
	m_fStartBounceDelay = 0.0f;
	m_fStartBounceSpeed = 0.0f;
	m_bStartBouncing    = TFALSE;

	if ( fStartY != START_Y_UNSET )
		m_pStart->SetPosition( m_pStart->GetPositionX(), fStartY );
}

// $deBlob: FUNCTION 006417b0
void ABlobMultiplayerEngagementMenu::AddDevice( TInputDevice* a_pDevice )
{
	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		if ( m_aiDevice[ i ] == a_pDevice->GetDeviceIndex() )
		{
			// Slot 0's device sets the buttons shown by the prompts
			if ( i != 0 )
				return;

			if ( a_pDevice->IsA( &TGetClass( TInputDeviceKeyboard ) ) )
				TInputDevice::SetLastGamepadType( TInputDevice::Gamepad_Keyboard );
			else if ( a_pDevice->IsA( &TGetClass( TInputDeviceController ) ) )
				TInputDevice::SetLastGamepadType( a_pDevice->GetGamepadType() );

			return;
		}
	}

	TINT iSlot;

	for ( iSlot = 0; iSlot < NUM_SLOTS; iSlot++ )
	{
		if ( m_aiDevice[ iSlot ] < 0 )
			break;
	}

	if ( iSlot == NUM_SLOTS )
		return;

	printf( "NFO: Slot %d setted to %d\n", iSlot, a_pDevice->GetDeviceIndex() );
	TInputDevice::SetPlayerDevice( iSlot, a_pDevice );
	FindChildXUIElement( s_aiJoinedPanels[ iSlot ] )->SetVisible( TTRUE );
	FindChildXUIElement( s_aiJoinPanels[ iSlot ] )->SetVisible( TFALSE );
	m_aiDevice[ iSlot ] = a_pDevice->GetDeviceIndex();
	FindParentScene()->SetFocus( m_apColours[ iSlot ][ 0 ], iSlot );

	TWCHAR* wszText = TNULL;

	if ( FindChildXUIControl( s_aiSlotLabels[ iSlot ] ) != TNULL )
	{
		wszText = m_awszSlotText[ iSlot ];
		FormatSlotText( wszText, iSlot, a_pDevice );
		FindChildXUIControl( s_aiSlotLabels[ iSlot ] )->SetText( wszText, -1 );
	}

	if ( iSlot == 0 )
	{
		swprintf( m_wszStartText, 256, L"%lc %ls", 0x80, T2Locale::GetSingleton()->GetString( 465 ) );

		if ( a_pDevice->IsA( &TGetClass( TInputDeviceKeyboard ) ) )
			TInputDevice::SetLastGamepadType( TInputDevice::Gamepad_Keyboard );
		else if ( a_pDevice->IsA( &TGetClass( TInputDeviceController ) ) )
			TInputDevice::SetLastGamepadType( a_pDevice->GetGamepadType() );
		else
			TASSERT( TFALSE );

		m_pStart->SetText( m_wszStartText, -1 );
	}

	UpdateTakenColours();
}

// $deBlob: FUNCTION 00641b70
void ABlobMultiplayerEngagementMenu::RemoveDevice( TInputDevice* a_pDevice )
{
	TINT iSlot;

	for ( iSlot = 0; iSlot < NUM_SLOTS; iSlot++ )
	{
		if ( m_aiDevice[ iSlot ] == a_pDevice->GetDeviceIndex() )
			break;
	}

	if ( iSlot == NUM_SLOTS )
		return;

	TUtil::Log( "NFO: Slot %d removed from %d\n", iSlot, a_pDevice->GetDeviceIndex() );
	ResetSlot( iSlot );

	for ( TINT i = iSlot + 1; i < NUM_SLOTS; i++ )
	{
		if ( m_aiDevice[ i ] >= 0 )
			MoveSlot( i, i - 1 );
	}

	UpdateTakenColours();
}

// $deBlob: FUNCTION 00641c00
void ABlobMultiplayerEngagementMenu::ResetSlot( TINT a_iSlot )
{
	printf( "NFO: Reset slot %d\n", a_iSlot );

	for ( TINT j = 0; j < NUM_COLOURS; j++ )
	{
		if ( m_apColours[ a_iSlot ][ j ] != TNULL )
		{
			m_apColours[ a_iSlot ][ j ]->SetFocus( TFALSE, TFALSE, TTRUE );
			m_apColours[ a_iSlot ][ j ]->SetVisible( TTRUE );
		}

		if ( m_apSelectedColours[ a_iSlot ][ j ] != TNULL )
		{
			m_apSelectedColours[ a_iSlot ][ j ]->SetFocus( TFALSE, TFALSE, TTRUE );
			m_apSelectedColours[ a_iSlot ][ j ]->SetVisible( TFALSE );
		}
	}

	TXUIElement* pWarning           = m_apTakenWarnings[ a_iSlot ];
	m_afTakenWarningTime[ a_iSlot ] = 0.0f;

	if ( pWarning != TNULL )
		pWarning->SetOpacity( 0.0f );

	FindChildXUIElement( s_aiJoinedPanels[ a_iSlot ] )->SetVisible( TFALSE );
	FindChildXUIElement( s_aiJoinPanels[ a_iSlot ] )->SetVisible( TTRUE );

	if ( FindChildXUIControl( s_aiSlotLabels[ a_iSlot ] ) != TNULL )
	{
		swprintf( m_awszSlotText[ a_iSlot ], 256, L"CHOOSE COLOR P%d", a_iSlot + 1 );
		FindChildXUIControl( s_aiSlotLabels[ a_iSlot ] )->SetText( m_awszSlotText[ a_iSlot ], -1 );
	}

	TInputDevice::SetPlayerDevice( a_iSlot, TNULL );
	m_aiDevice[ a_iSlot ] = -1;
	m_aiColour[ a_iSlot ] = -1;
	m_aiState[ a_iSlot ]  = SlotState_Engagement;
	m_aiFocus[ a_iSlot ]  = a_iSlot;

	UpdateTakenColours();
}

// $deBlob: FUNCTION 00641d60
void ABlobMultiplayerEngagementMenu::MoveSlot( TINT a_iFrom, TINT a_iTo )
{
	printf( "NFO: Move slot %d to slot %d\n", a_iFrom, a_iTo );

	TInputDevice* pDevice = TInputDevice::GetPlayerDevice( a_iFrom );
	TWCHAR*       wszText = m_awszSlotText[ a_iTo ];
	FormatSlotText( wszText, a_iTo, pDevice );
	FindChildXUIControl( s_aiSlotLabels[ a_iTo ] )->SetText( wszText, -1 );

	TInputDevice::SetPlayerDevice( a_iTo, TInputDevice::GetPlayerDevice( a_iFrom ) );

	TXUIElement* pWarning         = m_apTakenWarnings[ a_iTo ];
	m_aiDevice[ a_iTo ]           = m_aiDevice[ a_iFrom ];
	m_aiColour[ a_iTo ]           = m_aiColour[ a_iFrom ];
	m_aiState[ a_iTo ]            = m_aiState[ a_iFrom ];
	m_aiFocus[ a_iTo ]            = m_aiFocus[ a_iFrom ];
	m_afTakenWarningTime[ a_iTo ] = 0.0f;

	if ( pWarning != TNULL )
		pWarning->SetOpacity( 0.0f );

	FindChildXUIElement( s_aiJoinedPanels[ a_iTo ] )->SetVisible( FindChildXUIElement( s_aiJoinedPanels[ a_iFrom ] )->IsVisible() );
	FindChildXUIElement( s_aiJoinPanels[ a_iTo ] )->SetVisible( FindChildXUIElement( s_aiJoinPanels[ a_iFrom ] )->IsVisible() );

	for ( TINT j = 0; j < NUM_COLOURS; j++ )
	{
		if ( m_apColours[ a_iTo ][ j ] != TNULL )
		{
			TXUIControl* pFrom = m_apColours[ a_iFrom ][ j ];
			m_apColours[ a_iTo ][ j ]->SetVisible( pFrom->IsVisible() );

			TUINT32 uiState = pFrom->GetTimelineState();
			m_apColours[ a_iTo ][ j ]->SetFocus( uiState == TimelineState_Focus || uiState == TimelineState_TransTab, TTRUE, TTRUE );
		}

		if ( m_apSelectedColours[ a_iTo ][ j ] != TNULL )
		{
			TXUIControl* pFrom = m_apSelectedColours[ a_iFrom ][ j ];
			m_apSelectedColours[ a_iTo ][ j ]->SetVisible( pFrom->IsVisible() );

			TUINT32 uiState = pFrom->GetTimelineState();
			m_apSelectedColours[ a_iTo ][ j ]->SetFocus( uiState == TimelineState_Focus || uiState == TimelineState_TransTab, TTRUE, TTRUE );
		}
	}

	FindParentScene()->SetFocus( TNULL, a_iTo );
	ResetSlot( a_iFrom );

	if ( a_iTo == 0 )
	{
		swprintf( m_wszStartText, 256, L"%lc %ls", 0x80, T2Locale::GetSingleton()->GetString( 465 ) );

		if ( pDevice->IsA( &TGetClass( TInputDeviceKeyboard ) ) )
			TInputDevice::SetLastGamepadType( TInputDevice::Gamepad_Keyboard );
		else if ( pDevice->IsA( &TGetClass( TInputDeviceController ) ) )
			TInputDevice::SetLastGamepadType( pDevice->GetGamepadType() );

		m_pStart->SetText( m_wszStartText, -1 );
	}

	UpdateTakenColours();
}

// $deBlob: FUNCTION 006421f0
void ABlobMultiplayerEngagementMenu::UpdateTakenColours()
{
	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		for ( TINT j = 0; j < NUM_COLOURS; j++ )
		{
			if ( m_apColours[ i ][ j ] != TNULL )
				m_apColours[ i ][ j ]->SetOpacity( 1.0f );
		}
	}

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		for ( TINT j = 0; j < NUM_COLOURS; j++ )
		{
			if ( m_aiColour[ i ] >= 0 && m_aiColour[ i ] == j )
			{
				for ( TINT k = 0; k < NUM_SLOTS; k++ )
				{
					if ( i != k && m_apColours[ k ][ j ] != TNULL )
						m_apColours[ k ][ j ]->SetOpacity( 0.125f );
				}
			}
		}
	}
}

// $deBlob: FUNCTION 006422b0
void ABlobMultiplayerEngagementMenu::FocusColour( TINT a_iColour, TINT a_iSlot )
{
	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		if ( a_iSlot == i )
		{
			for ( TINT j = 0; j < NUM_COLOURS; j++ )
				m_apColours[ i ][ j ]->SetFocus( TFALSE, TFALSE, TTRUE );

			m_apColours[ i ][ a_iColour ]->SetFocus( TTRUE, TTRUE, TTRUE );
			m_afTakenWarningTime[ i ] = 0.0f;
		}
	}
}

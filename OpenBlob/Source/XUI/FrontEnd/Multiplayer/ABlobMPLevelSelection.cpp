#include "pch.h"
#include "ABlobMPLevelSelection.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "XUI/ElementTypes/ABlobCarouselList.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/AXUIState.h"
#include "GameInterface/APlayerProfileManager.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIVisual.h>
#include <XUI/TXUIBackButton.h>
#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <Platform/Windows/Input/TInputDeviceMouse_Win.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The offset of the select button from the carousel (shared with ABlobLevelSelection)
static const TFLOAT s_fSelectOffsetX = 51.0f;
static const TFLOAT s_fSelectOffsetY = 12.0f;

// The last button XUI got came from the keyboard or a controller
static TBOOL IsLastXUIDeviceKeyboardOrController()
{
	TInputDevice* pDevice = TInputDevice::GetRegisteredDevices()[ TInputDevice::GetLastXUIDeviceIndex() ];

	// NOTE: retail tests the mouse too and doesn't use the result
	pDevice->IsA( &TGetClass( TInputDXDeviceMouse ) );
	TBOOL bKeyboard   = pDevice->IsA( &TGetClass( TInputDeviceKeyboard ) );
	TBOOL bController = pDevice->IsA( &TGetClass( TInputDeviceController ) );

	return bKeyboard || bController;
}

// $deBlob: FUNCTION 0063f3b0
// $deBlob: FUNCTION 0063f3c0
// $deBlob: FUNCTION 0063f430
// $deBlob: FUNCTION 0063f4a0
// $deBlob: FUNCTION 0063f4b0
// $deBlob: FUNCTION 0053db10
// $deBlob: FUNCTION 00858a40
TDEFINE_CLASS( ABlobMPLevelSelection );

const TCHAR* ABlobMPLevelSelection::sm_aszLevels[ NUM_LEVELS ] = {
	"MP_Test_Walkways",
	"MP_Test_Walkways",
	"MP_Test_Walkways",
	"MP_Test_Oneway",
	"MP_Test_Oneway",
	"MP_Test_Oneway",
	"MP_Coliseum",
	"MP_Coliseum",
	"MP_Tower_of_Power",
};

const TINT ABlobMPLevelSelection::sm_aiLevelNames[ NUM_LEVELS ]        = { 154, 154, 154, 155, 155, 155, 156, 156, 160 };
const TINT ABlobMPLevelSelection::sm_aiLevelDescriptions[ NUM_LEVELS ] = { 403, 405, 404, 403, 405, 404, 403, 404, 403 };
const TINT ABlobMPLevelSelection::sm_aiLevelGameTypes[ NUM_LEVELS ]    = { 0, 1, 2, 0, 1, 2, 0, 2, 0 };

// $deBlob: FUNCTION 0063f4c0
ABlobMPLevelSelection::ABlobMPLevelSelection()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopMPLevelSel )
{
	m_pName        = TNULL;
	m_pDescription = TNULL;
	m_pLeft        = TNULL;
	m_pRight       = TNULL;
	m_pCarousel    = TNULL;
	m_pSelect      = TNULL;
	m_pNew         = TNULL;
}

// $deBlob: FUNCTION 0063fe20
// $deBlob: FUNCTION 0063fe40
// $deBlob: FUNCTION 0063fe32
ABlobMPLevelSelection::~ABlobMPLevelSelection()
{
}

// $deBlob: FUNCTION 0063f530
void ABlobMPLevelSelection::OnCreated()
{
	// NOTE: the result isn't used
	APlayerProfileManager::GetSingleton()->GetCurrentProfile();

	m_pName        = FindChildXUIControl( 3 );
	m_pDescription = FindChildXUIControl( 2 );
	m_pLeft        = FindChildXUIControl( 6 );
	m_pRight       = FindChildXUIControl( 5 );
	m_pCarousel    = TSTATICCAST( ABlobCarouselList, FindChildXUIControl( 7 ) );
	m_pSelect      = FindChildXUIControl( 8 );
	m_pNew         = FindChildXUIControl( 10 );
	m_pNew->SetVisible( TFALSE );
	m_pCarousel->SetDescription( m_pSelect );

	UpdateLevelInfo( 0 );
	m_pSelect->SetPosition( s_fSelectOffsetX + m_pCarousel->GetPositionX(), s_fSelectOffsetY + m_pCarousel->GetPositionY() );

	// From a level (the pause menu) the menu has its own transitions
	if ( AGameStateController::GetSingleton()->GetCurrentGameState()->IsA( &TGetClass( AInGameState ) ) )
	{
		SetTransitionVisual( Transition_To, L"TransTo" );
		SetTransitionVisual( Transition_BackFrom, L"TransBackFrom" );
	}

	ABlobMenu::OnCreated();
}

// $deBlob: FUNCTION 0063f690
TBOOL ABlobMPLevelSelection::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( a_pSource != m_pBackButton && IsLastXUIDeviceKeyboardOrController() )
	{
		m_pRight->SetFocus( TFALSE, TFALSE, TTRUE );
		m_pLeft->SetFocus( TFALSE, TFALSE, TTRUE );
		m_pSelect->SetFocus( TTRUE, TFALSE, TTRUE );
		FindParentScene()->SetFocus( m_pSelect, a_iController );
	}

	if ( m_pRight == a_pSource )
	{
		m_pCarousel->ScrollRight();
	}
	else if ( m_pLeft == a_pSource )
	{
		m_pCarousel->ScrollLeft();
	}
	else
	{
		if ( m_pSelect == a_pSource )
		{
			TINT            iLevel   = m_pCarousel->GetFocussedItemIndex();
			APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();

			if ( pProfile != TNULL )
			{
				TUINT32 uiBit = 1 << iLevel;

				if ( pProfile->GetData()->m_iUnlockedBlobParty & uiBit )
				{
					pProfile->GetData()->m_uiNewFlagsExtended &= ~uiBit;
					StartLevel( iLevel );
					return TTRUE;
				}
			}

			// Locked
			m_pCarousel->PulseLocked();
			return TTRUE;
		}

		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	UpdateLevelInfo( m_pCarousel->GetFocussedItemIndex() );
	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}

// $deBlob: FUNCTION 0063f820
TBOOL ABlobMPLevelSelection::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( m_pCarousel->IsScrolling() )
		return TXUIScene::HandleButton( a_eButton, a_iController );

	TXUIControl* pFocus;

	if ( a_eButton == XUIButton_Left )
	{
		if ( !m_pCarousel->IsFirstItemFocussed() )
			PlayFocusSound( m_pLeft );

		TBOOL bScrolled = m_pCarousel->ScrollLeft();
		( bScrolled ? m_pRight : m_pLeft )->SetVisible( bScrolled );
		UpdateLevelInfo( m_pCarousel->GetFocussedItemIndex() );

		m_pRight->SetFocus( TFALSE, TFALSE, TTRUE );
		m_pLeft->SetFocus( TTRUE, TFALSE, TTRUE );
		m_pSelect->SetFocus( TFALSE, TFALSE, TTRUE );
		pFocus = m_pLeft;
	}
	else if ( a_eButton == XUIButton_Right )
	{
		if ( !m_pCarousel->IsLastItemFocussed() )
			PlayFocusSound( m_pRight );

		TBOOL bScrolled = m_pCarousel->ScrollRight();
		( bScrolled ? m_pLeft : m_pRight )->SetVisible( bScrolled );
		UpdateLevelInfo( m_pCarousel->GetFocussedItemIndex() );

		m_pRight->SetFocus( TTRUE, TFALSE, TTRUE );
		m_pLeft->SetFocus( TFALSE, TFALSE, TTRUE );
		m_pSelect->SetFocus( TFALSE, TFALSE, TTRUE );
		pFocus = m_pRight;
	}
	else
	{
		if ( a_eButton == XUIButton_Up )
		{
			m_pRight->SetFocus( TFALSE, TFALSE, TTRUE );
			m_pLeft->SetFocus( TFALSE, TFALSE, TTRUE );
			m_pSelect->SetFocus( TTRUE, TFALSE, TTRUE );
			FindParentScene()->SetFocus( m_pSelect, a_iController );
		}

		return TXUIScene::HandleButton( a_eButton, a_iController );
	}

	FindParentScene()->SetFocus( pFocus, a_iController );
	return TTRUE;
}

// $deBlob: FUNCTION 0063fa00
void ABlobMPLevelSelection::TransTo()
{
	TXUIScene* pParent = FindParentScene();

	if ( pParent != TNULL && pParent->IsA( &TGetClass( ABlobFrontEnd ) ) )
	{
		TASSERT( -1 != m_iBackgroundMovie );
		TSTATICCAST( ABlobFrontEnd, pParent )->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransMPColourMPGameType, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
		TSTATICCAST( ABlobFrontEnd, pParent )->SlideLeft();
	}

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 0063fa90
void ABlobMPLevelSelection::TransBackFrom()
{
	TXUIScene* pParent = FindParentScene();

	if ( pParent != TNULL && pParent->IsA( &TGetClass( ABlobFrontEnd ) ) )
		TSTATICCAST( ABlobFrontEnd, pParent )->ResetCursorTrails();

	ABlobMenu::TransBackFrom();
}

// $deBlob: FUNCTION 0063fad0
void ABlobMPLevelSelection::EndTransBackFrom()
{
	TXUIScene* pParent = FindParentScene();

	if ( pParent != TNULL && pParent->IsA( &TGetClass( ABlobFrontEnd ) ) )
		TSTATICCAST( ABlobFrontEnd, pParent )->SetCursorTrailsVisible( TTRUE );

	TXUIScene::EndTransBackFrom();
}

// $deBlob: FUNCTION 0063fb10
void ABlobMPLevelSelection::StartLevel( TINT a_iLevelIndex )
{
	TASSERT( a_iLevelIndex >= 0 && a_iLevelIndex < NUM_LEVELS );
	AGameStatus::GetSingleton()->SetMultiPlayerGameType( sm_aiLevelGameTypes[ a_iLevelIndex ] );

	TXUIScene* pTopLevelScene = TXUI::GetSingleton()->GetActiveScene();

	if ( TStringManager::String16Compare( pTopLevelScene->GetID(), L"FrontEnd", -1 ) != 0 )
	{
		// From a level
		TASSERT( 0 == TStringManager::String16Compare( pTopLevelScene->GetID(), L"InGame", -1 ) );

		AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
		TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );

		TSTATICCAST( AInGameState, pState )->SetNextLevel( sm_aszLevels[ a_iLevelIndex ], sm_aiLevelNames[ a_iLevelIndex ], a_iLevelIndex );
		TSTATICCAST( AInGameState, pState )->SetLoadNextLevel( TTRUE );
		return;
	}

	// From the frontend
	AGameStatus::GetSingleton()->SetLevelMode( 0 );

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AXUIState ) ) );

	AXUIState* pXUIState = TSTATICCAST( AXUIState, pState );
	TINT       iName     = sm_aiLevelNames[ a_iLevelIndex ];

	if ( iName == -1 )
		pXUIState->SetNextLevel( sm_aszLevels[ a_iLevelIndex ], L"", a_iLevelIndex );
	else
		pXUIState->SetNextLevel( sm_aszLevels[ a_iLevelIndex ], T2Locale::GetSingleton()->GetString( iName ), a_iLevelIndex );

	pXUIState->SetNextState( AXUIState::NextState_LoadLevel );
}

// $deBlob: FUNCTION 0063fcf0
void ABlobMPLevelSelection::UpdateLevelInfo( TINT a_iLevelIndex )
{
	m_pName->SetText( T2Locale::GetSingleton()->GetString( sm_aiLevelNames[ a_iLevelIndex ] ), -1 );
	m_pDescription->SetText( T2Locale::GetSingleton()->GetString( sm_aiLevelDescriptions[ a_iLevelIndex ] ), -1 );

	// NOTE: the result isn't used
	m_pCarousel->GetFocussedItemIndex();

	EnSaveData::SlotData* pData = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();
	TUINT32               uiBit = 1 << a_iLevelIndex;

	if ( pData->m_uiNewFlagsExtendedUnseen & uiBit )
	{
		// The "New" label is revealed the first time the level is shown
		TASSERT( ( pData->m_uiNewFlagsExtended & uiBit ) != 0 && "pProfile->GetData()->GetNewFlag_extended(iFLag)" );
		pData->m_uiNewFlagsExtendedUnseen &= ~uiBit;
		m_pNew->SetVisible( TTRUE );
		m_pNew->GetVisual()->FindChildXUIElement( L"Reveal" )->PlayAnimation( L"Reveal", L"EndReveal" );
		return;
	}

	m_pNew->SetVisible( ( pData->m_uiNewFlagsExtended & uiBit ) != 0 );
}

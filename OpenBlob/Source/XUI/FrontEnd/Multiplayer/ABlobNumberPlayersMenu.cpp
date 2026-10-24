#include "pch.h"
#include "ABlobNumberPlayersMenu.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "GameInterface/APlayerManager.h"
#include "GUI/AGUIManager.h"

#include <Input/TInputInterface.h>
#include <Input/TInputDeviceController.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobNumberPlayersMenu ) == 0x150 );

// $deBlob: FUNCTION 006424d0
// $deBlob: FUNCTION 006424e0
// $deBlob: FUNCTION 00642560
// $deBlob: FUNCTION 006425e0
// $deBlob: FUNCTION 006425f0
// $deBlob: FUNCTION 0053db90
// $deBlob: FUNCTION 00858a60
TDEFINE_CLASS( ABlobNumberPlayersMenu );

// $deBlob: FUNCTION 00642600
ABlobNumberPlayersMenu::ABlobNumberPlayersMenu()
    : ABlobMenu( ABlobFrontEnd::BGMOVIE_LoopMPPlayer )
{
}

// $deBlob: FUNCTION 00642ac0
// $deBlob: FUNCTION 00642ad2
// $deBlob: FUNCTION 00642ae0
ABlobNumberPlayersMenu::~ABlobNumberPlayersMenu()
{
}

// $deBlob: FUNCTION 00642730
void ABlobNumberPlayersMenu::Tick( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < NUM_CHOICES; i++ )
		m_apChoices[ i ]->SetEnabled( i + 1 < m_iNumControllers );

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00642620
void ABlobNumberPlayersMenu::OnCreated()
{
	ABlobMenu::OnCreated();

	m_apChoices[ 0 ] = FindChildXUIControl( 4 );
	m_apChoices[ 1 ] = FindChildXUIControl( 3 );
	m_apChoices[ 2 ] = FindChildXUIControl( 2 );

	m_iNumControllers = 0;

	for ( TINT i = 0; i < 4; i++ )
	{
		TInputDevice* pDevice = TInputInterface::GetSingleton()->GetDeviceByIndex( &TGetClass( TInputDeviceController ), i );

		if ( pDevice != TNULL && pDevice->IsEnabled() )
			m_iNumControllers++;
	}
}

// $deBlob: FUNCTION 006426a0
TBOOL ABlobNumberPlayersMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	for ( TINT iChoice = 0; iChoice < NUM_CHOICES; iChoice++ )
	{
		if ( a_pSource == m_apChoices[ iChoice ] )
		{
			APlayerManager::GetSingleton()->SetNumPlayers( iChoice + 2 );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00642a10
TBOOL ABlobNumberPlayersMenu::OnNotifySetFocus( TXUIElement* a_pSource )
{
	ABlobMenu::OnNotifySetFocus( a_pSource );

	for ( TINT i = 0; i < NUM_CHOICES; i++ )
		if ( a_pSource != m_apChoices[ i ] )
			m_apChoices[ i ]->SetFocus( TFALSE, TFALSE, TTRUE );
		else
			m_apChoices[ i ]->SetFocus( TTRUE, TFALSE, TTRUE );

	TSTATICCAST( TXUIControl, a_pSource )->SetFocus( TTRUE, TFALSE, TTRUE );

	TXUIScene* pParentScene = FindParentScene();

	if ( pParentScene != TNULL )
	{
		SetFocus( TSTATICCAST( TXUIControl, a_pSource ), 0 );
		pParentScene->SetFocus( TSTATICCAST( TXUIControl, a_pSource ), 0 );
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00642900
TBOOL ABlobNumberPlayersMenu::HandleButton( TINT a_eButton, TINT a_iController )
{
	TINT iNumControllers = m_iNumControllers;

	if ( iNumControllers <= 2 )
	{
		if ( a_eButton == XUIButton_Accept || a_eButton == XUIButton_Back )
			return TXUIScene::HandleButton( a_eButton, a_iController );

		return TTRUE;
	}

	// The focussed choice (visual state 2 or 9)
	TINT iFocus;

	for ( iFocus = 0;; iFocus++ )
	{
		if ( iFocus >= NUM_CHOICES )
			return TXUIScene::HandleButton( a_eButton, a_iController );

		TUINT32 uiState = m_apChoices[ iFocus ]->GetTimelineState();

		if ( uiState == TimelineState_Focus || uiState == TimelineState_TransTab )
			break;
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail wraps the index with the number of controllers, with 4 controllers the 4th "choice" is
	// m_iNumControllers read as a control; there are only NUM_CHOICES choices
	iNumControllers = NUM_CHOICES;
#endif

	switch ( a_eButton )
	{
		case XUIButton_Up:
		case XUIButton_Left:
			OnNotifySetFocus( m_apChoices[ Wrap( iFocus - 1, iNumControllers ) ] );
			return TTRUE;
		case XUIButton_Down:
		case XUIButton_Right:
			OnNotifySetFocus( m_apChoices[ Wrap( iFocus + 1, iNumControllers ) ] );
			return TTRUE;
		case XUIButton_Accept:
			return OnNotifyPress( m_apChoices[ iFocus ], a_iController );
		default:
			return TXUIScene::HandleButton( a_eButton, a_iController );
	}
}

// $deBlob: FUNCTION 006427a0
void ABlobNumberPlayersMenu::TransTo()
{
	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );

	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransMainMenuMPPlayer, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	pFrontEnd->SlideLeft();
	ShowCursor();

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 00642820
void ABlobNumberPlayersMenu::TransBackTo()
{
	// NOTE: retail stores 1 without SetNumPlayers' assert
	APlayerManager::GetSingleton()->SetNumPlayers( 1 );

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TINT           iMovie    = pFrontEnd->GetRandomBackgroundMovie( ABlobFrontEnd::BGMOVIE_BackGen01 );
	pFrontEnd->ResetCursorTrails();

	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( iMovie, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );
	ShowCursor();

	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 006428c0
void ABlobNumberPlayersMenu::TransBackFrom()
{
	// NOTE: retail stores 1 without SetNumPlayers' assert
	APlayerManager::GetSingleton()->SetNumPlayers( 1 );

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->SlideRight();
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_BackMPPlayerMainMenu, ABlobFrontEnd::BGMOVIE_LoopMainMenu, TFALSE, TFALSE, TFALSE );

	ABlobMenu::TransBackFrom();
}

// $deBlob: FUNCTION 00642a90
void ABlobNumberPlayersMenu::ShowCursor()
{
	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );
}

// $deBlob: FUNCTION 00642aa0
TINT ABlobNumberPlayersMenu::Wrap( TINT a_iValue, TINT a_iCount )
{
	return ( a_iCount + a_iValue % a_iCount ) % a_iCount;
}

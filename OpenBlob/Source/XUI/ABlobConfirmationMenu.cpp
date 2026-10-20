#include "pch.h"
#include "ABlobConfirmationMenu.h"
#include "GameInterface/AGameStateController.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIInput.h>
#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00625920
// $deBlob: FUNCTION 00625930
// $deBlob: FUNCTION 006259a0
// $deBlob: FUNCTION 00625a10
// $deBlob: FUNCTION 00625a20
// $deBlob: FUNCTION 0053d010
// $deBlob: FUNCTION 00858780
TDEFINE_CLASS( ABlobConfirmationMenu );

// $deBlob: FUNCTION 00625cc0
ABlobConfirmationMenu::ABlobConfirmationMenu()
    : ABlobMenu( -1 )
{
	m_pYes        = TNULL;
	m_pNo         = TNULL;
	m_pParentMenu = TNULL;
	m_pTitle      = TNULL;
	m_bFadeOnYes  = TFALSE;
	m_bCloseOnYes = TTRUE;
	m_bFading     = TFALSE;
}

// $deBlob: FUNCTION 00625d70
// $deBlob: FUNCTION 00625d20
// $deBlob: FUNCTION 00625d52
ABlobConfirmationMenu::~ABlobConfirmationMenu()
{
}

// $deBlob: FUNCTION 00625a30
void ABlobConfirmationMenu::OnCreated()
{
	m_pYes   = FindChildXUIControl( 5 );
	m_pNo    = FindChildXUIControl( 4 );
	m_pTitle = FindChildXUIControl( 6 );
	m_pText  = FindChildXUIControl( 7 );

	m_pTitle->SetText( L"", -1 );
	m_pText->SetText( L"", -1 );

	m_pElement1 = FindChildXUIElement( 1 );

	ABlobMenu::OnCreated();
}

// $deBlob: FUNCTION 00625ab0
TBOOL ABlobConfirmationMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	PlayPressSound( a_pSource );

	if ( m_pParentMenu != TNULL )
	{
		if ( !m_bFadeOnYes || a_pSource != m_pYes )
		{
			m_pParentMenu->OnConfirmationClosed( a_pSource == m_pYes );

			if ( !m_bCloseOnYes )
				return TTRUE;
		}
		else
		{
			// Fades to black, Tick answers once it's black
			AGameStateController::GetSingleton()->ResetFades();
			AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TTRUE );
			AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
			TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
			m_bFading = TTRUE;
		}
	}

	if ( !m_bFadeOnYes && m_bCloseOnYes == TTRUE && a_pSource == m_pYes )
	{
		TXUI::GetSingleton()->TransitionBack( this );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00625b80
void ABlobConfirmationMenu::SetTitle( const TWCHAR* a_wszTitle )
{
	m_pTitle->SetText( a_wszTitle, -1 );
}

// $deBlob: FUNCTION 00625ba0
void ABlobConfirmationMenu::SetText( const TWCHAR* a_wszText )
{
	m_pText->SetText( a_wszText, -1 );
}

// $deBlob: FUNCTION 00625bc0
void ABlobConfirmationMenu::SetMessageOnly()
{
	m_pYes->SetVisible( TFALSE );

	m_pNo->SetNav( TXUIControl::Nav_Up, TNULL );
	m_pNo->SetNavId( TXUIControl::Nav_Up, TNULL );
	m_pNo->SetNav( TXUIControl::Nav_Down, TNULL );
	m_pNo->SetNavId( TXUIControl::Nav_Down, TNULL );

	m_pNo->SetText( T2Locale::GetSingleton()->GetString( 3 ), -1 );
}

// $deBlob: FUNCTION 00625c30
void ABlobConfirmationMenu::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bFadeOnYes && m_bFading )
	{
		if ( !AGameStateController::GetSingleton()->IsFading( 0 ) && AGameStateController::GetSingleton()->GetFade( 0 ) == 1.0f )
		{
			TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
			m_pParentMenu->OnConfirmationClosed( TTRUE );
			m_bFading = TFALSE;
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

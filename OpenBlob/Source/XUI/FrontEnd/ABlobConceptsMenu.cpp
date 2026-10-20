#include "pch.h"
#include "ABlobConceptsMenu.h"
#include "Movie/AMoviePlayer.h"

#include <GUI/T2GUI.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIButton.h>
#include <XUI/TXUICanvas.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobConceptsMenu ) == 0x220 );

// $deBlob: FUNCTION 0062e1f0
// $deBlob: FUNCTION 0062e200
// $deBlob: FUNCTION 0062e270
// $deBlob: FUNCTION 0062e2e0
// $deBlob: FUNCTION 0062e2f0
// $deBlob: FUNCTION 0053d2d0
// $deBlob: FUNCTION 00858830
TDEFINE_CLASS( ABlobConceptsMenu );

// $deBlob: FUNCTION 0062e610
ABlobConceptsMenu::ABlobConceptsMenu()
    : ABlobMenu( -1 )
{
	m_pPrevFocus    = TNULL;
	m_bPlayingMovie = TFALSE;
	m_oBackground.SetVisible( TFALSE );
	T2GUI::GetSingleton()->GetRootElement()->AddChildTail( &m_oBackground );
}

// $deBlob: FUNCTION 0062e300
// $deBlob: FUNCTION 0062e6a2
// $deBlob: FUNCTION 0062e6b0
ABlobConceptsMenu::~ABlobConceptsMenu()
{
	m_oBackground.RemoveSelf();
	m_oBackground.Destroy();
}

// $deBlob: FUNCTION 0062e590
void ABlobConceptsMenu::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bPlayingMovie == TTRUE && !AMoviePlayer::GetSingleton()->IsMoviePlaying() )
	{
		TXUI::GetSingleton()->GetCanvas()->SetVisible( TTRUE );
		SetFocus( m_pPrevFocus, TXUI::GetSingleton()->GetDefaultController() );
		m_pPrevFocus    = TNULL;
		m_bPlayingMovie = TFALSE;
		SetVisible( TTRUE );
	}

	// NOTE: retail doesn't call ABlobMenu's Tick (there's none) but TXUIScene's
	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0062e3b0
TBOOL ABlobConceptsMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( !a_pSource->IsExactly( &TGetClass( TXUIButton ) ) )
		return TFALSE;

	// NOTE: the name is converted into the scratch string, which every GetTempString8 returns
	TStringManager::StringUnicodeToChar( TStringManager::GetTempString8(), a_pSource->GetID(), -1 );
	TCHAR* szName = TStringManager::GetTempString8();

	if ( TStringManager::String8FindString( szName, "bik_" ) != TNULL )
	{
		if ( AMoviePlayer::GetSingleton()->PlayMovie( szName + 4, 0, AMoviePlayer::PlayFlags_None ) )
		{
			m_bPlayingMovie = TTRUE;
			m_pPrevFocus    = GetFocus( TXUI::GetSingleton()->GetDefaultController() );
			SetFocus( TNULL, TXUI::GetSingleton()->GetDefaultController() );
			TXUI::GetSingleton()->GetCanvas()->SetVisible( TFALSE );
			SetVisible( TFALSE );
			return TTRUE;
		}
	}
	else
	{
		// NOTE: our A2GUIBackground::Create takes one parameter (retail's has two)
		m_oBackground.Create( TStringManager::GetTempString8() );
		m_oBackground.SetVisible( TTRUE );
		m_pPrevFocus = GetFocus( TXUI::GetSingleton()->GetDefaultController() );
		SetFocus( TNULL, TXUI::GetSingleton()->GetDefaultController() );
		TXUI::GetSingleton()->GetCanvas()->SetVisible( TFALSE );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0062e4d0
TBOOL ABlobConceptsMenu::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( ( !m_oBackground.IsVisible() && m_bPlayingMovie != TTRUE ) || a_eButton != XUIButton_Back )
		return TXUIScene::HandleButton( a_eButton, a_iController );

	if ( m_oBackground.IsVisible() )
	{
		m_oBackground.SetVisible( TFALSE );
	}
	else if ( m_bPlayingMovie && AMoviePlayer::GetSingleton()->IsMoviePlaying() == TTRUE )
	{
		AMoviePlayer::GetSingleton()->StopMovie();
		m_bPlayingMovie = TFALSE;
		SetVisible( TTRUE );
	}

	SetFocus( m_pPrevFocus, TXUI::GetSingleton()->GetDefaultController() );
	m_pPrevFocus = TNULL;
	TXUI::GetSingleton()->GetCanvas()->SetVisible( TTRUE );
	return TTRUE;
}

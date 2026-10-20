#include "pch.h"
#include "ABlobInGame.h"
#include "ABlobPauseMenu.h"
#include "HUD/ABlobHud.h"
#include "AApplication.h"
#include "XUI/ABlobCursorTrail.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIElement.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TXUIElement* ABlobInGame::ms_pCursorTrailsAfter;
Toshi::TXUIElement* ABlobInGame::ms_pCursorTrailsParent;
TBOOL               ABlobInGame::ms_bMoveCursorTrails;
TBOOL               ABlobInGame::ms_bRestoreCursorTrails;

// $deBlob: FUNCTION 00645ac0
// $deBlob: FUNCTION 00645ad0
// $deBlob: FUNCTION 00645b40
// $deBlob: FUNCTION 00645bb0
// $deBlob: FUNCTION 00645bc0
// $deBlob: FUNCTION 0053dcd0
// $deBlob: FUNCTION 00858ab0
TDEFINE_CLASS( ABlobInGame );

// $deBlob: FUNCTION 00645fe0
ABlobInGame::ABlobInGame()
    : m_eUsage( Usage_None )
    , m_pUnk134( TNULL )
    , m_iUnk148( 0 )
    , m_iUnk24C( 0 )
    , m_bReopenPause( TFALSE )
{
	for ( TINT i = 0; i < TARRAYSIZE( m_apCursorTrails ); i++ )
		m_apCursorTrails[ i ] = TNULL;
}

// $deBlob: FUNCTION 00646050
// $deBlob: FUNCTION 00646060
// $deBlob: FUNCTION 00645bd0
ABlobInGame::~ABlobInGame()
{
}

// $deBlob: FUNCTION 00645bf0
void ABlobInGame::OnCreated()
{
	// NOTE: retail doesn't call TXUIScene::OnCreated
	m_pUnk134 = FindChildXUIElement( 5 );

	TINT i = 0;

	for ( TINT iUID = 9; iUID > 5; iUID-- )
	{
		m_apCursorTrails[ i ] = TSTATICCAST( ABlobCursorTrail, FindChildXUIControl( iUID ) );
		m_apCursorTrails[ i ]->SetIndex( i );
		m_apCursorTrails[ i ]->CreateEffects();
		i++;
	}
}

// $deBlob: FUNCTION 00645d40
void ABlobInGame::TransTo()
{
	TASSERT( m_eUsage != Usage_None );

	switch ( m_eUsage )
	{
		case Usage_Pause:
		{
			ABlobPauseMenu* pPauseMenu = TSTATICCAST( ABlobPauseMenu, FindChildXUIControl( L"PauseMenu" ) );
			pPauseMenu->Pause();
			break;
		}
		default:
			break;
	}

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 00645cc0
void ABlobInGame::EndTransTo()
{
	for ( TINT i = 0; i < TARRAYSIZE( m_apCursorTrails ); i++ )
	{
		if ( m_apCursorTrails[ i ] != TNULL )
			m_apCursorTrails[ i ]->SetVisible( TTRUE );
	}

	g_oTheApp.SetRenderWorld( TFALSE );
	TXUIScene::EndTransTo();
}

// $deBlob: FUNCTION 00645d00
void ABlobInGame::TransBackFrom()
{
	for ( TINT i = 0; i < TARRAYSIZE( m_apCursorTrails ); i++ )
	{
		if ( m_apCursorTrails[ i ] != TNULL )
			m_apCursorTrails[ i ]->SetVisible( TFALSE );
	}

	g_oTheApp.SetRenderWorld( TTRUE );
	TXUIScene::TransBackFrom();
}

// $deBlob: FUNCTION 00645da0
void ABlobInGame::EndTransBackFrom()
{
	if ( m_pPrevScene == TNULL )
		Toshi::TXUI::GetSingleton()->SetFocussedScene( TNULL );

	if ( m_bReopenPause )
	{
		// The pause menu's confirmation popup is deleted
		TSTATICCAST( Toshi::TXUIScene, FindChildXUIControl( L"Confirmation" ) )->MarkForDelete();
		m_bReopenPause = TFALSE;

		Toshi::TXUIControl* pPauseMenu = FindChildXUIControl( L"PauseMenu" );
		pPauseMenu->SetOpacity( 1.0f );
		pPauseMenu->SetVisible( TTRUE );
	}

	if ( m_eUsage == Usage_Pause )
	{
		ABlobPauseMenu* pPauseMenu = TSTATICCAST( ABlobPauseMenu, FindChildXUIControl( L"PauseMenu" ) );
		pPauseMenu->Unpause();
		pPauseMenu->Unlink();
	}

	m_eUsage = Usage_None;
	TXUIScene::EndTransBackFrom();
}

// $deBlob: FUNCTION 00645e60
void ABlobInGame::CloseScenesToHud()
{
	TXUIScene* pScene = m_pPrevScene;

	if ( pScene == TNULL )
		return;

	while ( pScene != TNULL && !pScene->IsA( &TGetClass( ABlobHud ) ) )
	{
		TXUIScene* pPrev = pScene->GetPrevScene();
		delete pScene;
		pScene = pPrev;
	}

	m_pPrevScene = pScene;
}

// $deBlob: FUNCTION 00645ee0
void ABlobInGame::AttachCursorTrailsTo( TXUIElement* a_pParent, TXUIElement* a_pAfter )
{
	for ( TINT i = 0; i < TARRAYSIZE( m_apCursorTrails ); i++ )
		a_pParent->AddChildAfter( m_apCursorTrails[ i ], a_pAfter, TTRUE );
}

// $deBlob: FUNCTION 00645eb0
void ABlobInGame::SetCursorTrailsVisible( TBOOL a_bVisible )
{
	for ( TINT i = 0; i < TARRAYSIZE( m_apCursorTrails ); i++ )
		m_apCursorTrails[ i ]->SetVisible( a_bVisible );
}

// $deBlob: FUNCTION 00645f10
void ABlobInGame::AttachCursorTrails()
{
	for ( TINT i = 0; i < TARRAYSIZE( m_apCursorTrails ); i++ )
		AddChildAfter( m_apCursorTrails[ i ], m_pUnk134, TTRUE );
}

// $deBlob: FUNCTION 00645f40
void ABlobInGame::Tick( TFLOAT a_fDeltaTime )
{
	TXUIScene::Tick( a_fDeltaTime );

	if ( ms_bMoveCursorTrails )
	{
		for ( TINT i = 0; i < TARRAYSIZE( m_apCursorTrails ); i++ )
			ms_pCursorTrailsParent->AddChildAfter( m_apCursorTrails[ i ], ms_pCursorTrailsAfter, TTRUE );

		ms_pCursorTrailsParent = TNULL;
		ms_pCursorTrailsAfter  = TNULL;
		ms_bMoveCursorTrails   = TFALSE;
	}
	else if ( ms_bRestoreCursorTrails )
	{
		AttachCursorTrails();
		ms_bRestoreCursorTrails = TFALSE;
	}
}

// $deBlob: FUNCTION 00645c40
TBOOL ABlobInGame::PlayBackgroundMovie( const TWCHAR* a_wszMovie, TBOOL a_bLoop )
{
	return TTRUE;
}

// $deBlob: FUNCTION 00645c60
void ABlobInGame::SetVisible( TBOOL a_bVisible )
{
	g_oTheApp.SetRenderWorld( !a_bVisible );

	for ( TINT i = 0; i < TARRAYSIZE( m_apCursorTrails ); i++ )
	{
		if ( m_apCursorTrails[ i ] != TNULL )
			m_apCursorTrails[ i ]->SetVisible( a_bVisible );
	}

	T2GUIElement::SetVisible( a_bVisible );
}

#include "pch.h"
#include "ABlobVistasMenu.h"
#include "ABlobInGame.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/ABossEndSeqState.h"
#include "GameInterface/APlayerManager.h"
#include "Cameras/ACameraManager.h"
#include "Sound/AMusicManager.h"
#include "GUI/AGUIManager.h"
#include "GUI/AGUICursor.h"

#include <XUI/TXUI.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The names of the worlds by SPLevel (the same table as ABlobLevelSelection's)
static const TINT s_aiWorldNames[ AGameStatus::SPLevel_NUMOF ] = { 124, 125, 126, 127, 128, 129, 130, 131, 132, 133 };

// $deBlob: FUNCTION 0064d3b0
// $deBlob: FUNCTION 0064d3c0
// $deBlob: FUNCTION 0064d450
// $deBlob: FUNCTION 0064d4e0
// $deBlob: FUNCTION 0064d4f0
// $deBlob: FUNCTION 0053df90
// $deBlob: FUNCTION 00858b60
TDEFINE_CLASS( ABlobVistasMenu );

// $deBlob: FUNCTION 0064d500
ABlobVistasMenu::ABlobVistasMenu()
    : ABlobMenu( -1 )
{
	m_pNavButton = TNULL;
	m_pLevelName = TNULL;
}

// $deBlob: FUNCTION 0064d850
// $deBlob: FUNCTION 0064d830
// $deBlob: FUNCTION 0064d842
ABlobVistasMenu::~ABlobVistasMenu()
{
}

// $deBlob: FUNCTION 0064d530
void ABlobVistasMenu::OnCreated()
{
	AGameState* pGameState = AGameStateController::GetSingleton()->GetCurrentGameState();

	if ( pGameState->IsA( &TGetClass( AInGameState ) ) )
	{
		TASSERT( TFALSE == AGameStatus::GetSingleton()->IsMultiplayer() );
		TSTATICCAST( AInGameState, pGameState )->SetPaused( TTRUE );
		AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.75f );
		AMusicManager::GetSingleton()->PlaySnapshot( "vistacam" );
		m_pNavButton = FindChildXUIControl( L"XuiNavButton1" );
	}
	else
	{
		// NOTE: retail doesn't use the result
		pGameState->IsA( &TGetClass( ABossEndSeqState ) );
	}

	m_pLevelName = FindChildXUIControl( L"LevelName2" );

	TINT iLevel = AGameStatus::GetSingleton()->GetSPLevel();

	if ( iLevel == -1 )
		m_pLevelName->SetText( L"", -1 );
	else
		m_pLevelName->SetText( T2Locale::GetSingleton()->GetString( s_aiWorldNames[ iLevel ] ), -1 );

	AGUIManager::GetSingleton()->GetCursor()->SetVisible( TTRUE, TTRUE );

	TWCHAR* wszName = TStringManager::GetTempString16();

	for ( TINT i = 0; i < MAX_VISTAS; i++ )
	{
		TStringManager::String16Format( wszName, L"XuiButton%d", i + 1 );
		m_apButtons[ i ] = FindChildXUIControl( wszName );

		ANedCameraFunction::FixedCam* pCamera = ACameraManager::GetSingleton()->GetNedCameras().FindVistaCam( i );

		if ( pCamera != TNULL )
		{
			APlayerManager::CalculateVistaScore( 0, pCamera );
			m_apButtons[ i ]->SetText( T2Locale::GetSingleton()->GetString( APlayerManager::GetVistaScore( 0 )->m_iRank ), -1 );
		}
		else
		{
			m_apButtons[ i ]->SetVisible( TFALSE );
		}
	}
}

// $deBlob: FUNCTION 0064d6f0
TBOOL ABlobVistasMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	for ( TINT i = 0; i < MAX_VISTAS; i++ )
	{
		if ( a_pSource == m_apButtons[ i ] && ACameraManager::GetSingleton()->GetNedCameras().FindVistaCam( i ) != TNULL )
		{
			APlayerManager::GetSingleton()->StartVista( i );
			APlayerManager::GetSingleton()->UpdateVista( 0.0f );
		}
	}

	if ( m_pNavButton == TNULL || a_pSource != m_pNavButton )
		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
	{
		AGameState* pGameState = AGameStateController::GetSingleton()->GetCurrentGameState();

		if ( pGameState->IsA( &TGetClass( AInGameState ) ) )
		{
			TSTATICCAST( AInGameState, pGameState )->SetLevelEnded( TTRUE );
			return TTRUE;
		}

		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	TXUIScene* pScoreMenu = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "scoremenu.xur" ), 0 );

	TXUIScene* pParent = FindParentScene();

	if ( pParent != TNULL )
		pParent->AddChildTail( pScoreMenu );

	TXUI::GetSingleton()->TransitionTo( this, pScoreMenu );
	return TTRUE;
}

// $deBlob: FUNCTION 0064d810
void ABlobVistasMenu::EndTransTo()
{
	// NOTE: the parent scene is the in-game scene
	ABlobInGame* pInGame = TSTATICCAST( ABlobInGame, FindParentScene() );
	pInGame->PlayBackgroundMovie( L"POSTGAME_CAMERAS_LOOP.bik", TTRUE );
	ABlobMenu::EndTransTo();
}

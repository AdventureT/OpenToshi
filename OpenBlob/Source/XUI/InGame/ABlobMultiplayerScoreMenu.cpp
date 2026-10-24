#include "pch.h"
#include "ABlobMultiplayerScoreMenu.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "GUI/AGUIManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"

#include <XUI/TXUIControl.h>
#include <XUI/TXUIFigure.h>
#include <XUI/TXUIGroup.h>
#include <XUI/TXUIVisual.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The controls of the rows
static const TINT s_aiScoreTexts[ ABlobMultiplayerScoreMenu::NUM_ROWS ]    = { 19, 15, 11, 7 };
static const TINT s_aiNameTexts[ ABlobMultiplayerScoreMenu::NUM_ROWS ]     = { 22, 18, 14, 10 };
static const TINT s_aiColourFigures[ ABlobMultiplayerScoreMenu::NUM_ROWS ] = { 20, 16, 12, 8 };

// The locale strings of the players' names
static const TINT s_aiPlayerNames[ APlayerManager::MAX_PLAYERS ] = { 173, 174, 175, 176 };

// $deBlob: FUNCTION 00648c20
// $deBlob: FUNCTION 00648c30
// $deBlob: FUNCTION 00648cb0
// $deBlob: FUNCTION 00648d30
// $deBlob: FUNCTION 00648d40
// $deBlob: FUNCTION 0053ddd0
// $deBlob: FUNCTION 00858af0
TDEFINE_CLASS( ABlobMultiplayerScoreMenu );

// $deBlob: FUNCTION 00648d50
ABlobMultiplayerScoreMenu::ABlobMultiplayerScoreMenu()
    : ABlobMenu( -1 )
{
#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves the members to OnCreated
	for ( TINT i = 0; i < NUM_MEDALS; i++ )
		m_apMedals[ i ] = TNULL;
#endif
}

// $deBlob: FUNCTION 006491a0
// $deBlob: FUNCTION 006491b2
// $deBlob: FUNCTION 006491c0
ABlobMultiplayerScoreMenu::~ABlobMultiplayerScoreMenu()
{
}

// $deBlob: FUNCTION 00648d70
void ABlobMultiplayerScoreMenu::OnCreated()
{
	ABlobMenu::OnCreated();

	// The game is paused, the vista camera's snapshot plays
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pState )->SetPaused( TTRUE );

	AMusicManager::GetSingleton()->PlaySnapshot( "vistacam" );
	ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID( 2 ), TTRUE );

	TINT iNumDesiredPlayes = APlayerManager::GetSingleton()->GetNumPlayers();
	TASSERT( iNumDesiredPlayes >= 1 );
	AGUIManager::GetSingleton()->SetCursorVisibleCount( iNumDesiredPlayes );

	TASSERT( AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );

	TXUIGroup* pPlayers    = TSTATICCAST( TXUIGroup, FindChildXUIElement( 6 ) );
	TINT       iNumPlayers = APlayerManager::GetSingleton()->GetNumPlayers();

	for ( TINT i = 0; i < NUM_MEDALS; i++ )
		m_apMedals[ i ] = FindChildXUIElement( i + 0x17 );

	if ( iNumPlayers == 2 )
		m_apMedals[ 2 ]->SetOpacity( 0.0f );

	TASSERT( m_apMedals[ 0 ]->IsA( &TGetClass( TXUIControl ) ) );
	TSTATICCAST( TXUIControl, m_apMedals[ 0 ] )->GetVisual()->FindChildXUIElement( L"Medals" )->PlayAnimation( L"Gold", L"Gold" );

	// The layout of the player count, its named frame stops the timeline there
	TINT iRow = 0;

	if ( iNumPlayers == 2 )
	{
		GetTimelineControl().PlayAnimation( L"2Player", L"2Player" );
		pPlayers->GetTimelineControl()->FindNamedFrame( L"2P" )->m_eCommand = XURXUINamedFrameData::Command_Stop;
	}
	else if ( iNumPlayers == 3 )
	{
		GetTimelineControl().PlayAnimation( L"3Player", L"3Player" );
		pPlayers->GetTimelineControl()->FindNamedFrame( L"3P" )->m_eCommand = XURXUINamedFrameData::Command_Stop;
	}
	else
	{
		if ( iNumPlayers == 4 )
		{
			GetTimelineControl().PlayAnimation( L"4Player", L"4Player" );
			pPlayers->GetTimelineControl()->FindNamedFrame( L"4P" )->m_eCommand = XURXUINamedFrameData::Command_Stop;
		}

		if ( iNumPlayers < 1 )
			goto HIDE_ROWS;
	}

	// The players by rank: points, name and colour
	for ( iRow = 0; iRow < iNumPlayers; iRow++ )
	{
		TXUIControl*             pScore = TSTATICCAST( TXUIControl, FindChildXUIElement( s_aiScoreTexts[ iRow ] ) );
		TXUIControl*             pName  = TSTATICCAST( TXUIControl, FindChildXUIElement( s_aiNameTexts[ iRow ] ) );
		const AGameStatus::Rank& rRank  = AGameStatus::GetSingleton()->GetRankResult( iRow );

		TStringManager::String16Format( m_awszScores[ iRow ], 16, L"%d", rRank.m_iScore );
		pScore->SetText( m_awszScores[ iRow ], -1 );
		pScore->SetOpacity( 1.0f );
		pName->SetOpacity( 1.0f );
		pName->SetText( T2Locale::GetSingleton()->GetString( s_aiPlayerNames[ rRank.m_iPlayer ] ), -1 );

		TUINT32 uiColour = ABlobColourComponent::ms_auiColours[ APlayerManager::GetSingleton()->GetPlayerColour( rRank.m_iPlayer ) ];

		// NOTE: retail splits the colour into its bytes and puts it back together
		TSTATICCAST( TXUIFigure, FindChildXUIElement( s_aiColourFigures[ iRow ] ) )->SetFillColour( uiColour );
	}

	if ( NUM_ROWS - 1 < iRow )
		return;

HIDE_ROWS:
	// The rows without a player
	for ( ; iRow < NUM_ROWS; iRow++ )
	{
		FindChildXUIElement( s_aiScoreTexts[ iRow ] )->SetOpacity( 0.0f );
		FindChildXUIElement( s_aiNameTexts[ iRow ] )->SetOpacity( 0.0f );
	}
}

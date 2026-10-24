#include "pch.h"
#include "ABlobHud_4P.h"
#include "ABlobMultiplayerHud.h"
#include "ABlobScoreHud.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "Input/ABlobInputMap.h"
#include "Sound/ASoundInterface.h"

#include <XUI/TXUI.h>
#include <XUI/TXUICanvas.h>
#include <XUI/TXUIGroup.h>
#include <Render/TRender.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00654f40
// $deBlob: FUNCTION 00654f50
// $deBlob: FUNCTION 00654fc0
// $deBlob: FUNCTION 00655030
// $deBlob: FUNCTION 00655040
// $deBlob: FUNCTION 0053e280
// $deBlob: FUNCTION 00858c10
TDEFINE_CLASS( ABlobHud_4P );

// $deBlob: FUNCTION 00655050
ABlobHud_4P::ABlobHud_4P()
{
	for ( TINT i = 0; i < NUM_PLAYERS; i++ )
		m_apPanels[ i ] = TNULL;

	m_pReadySetGo = TNULL;
	m_pWinner     = TNULL;

	for ( TINT i = 0; i < NUM_PLAYERS; i++ )
		m_apScoreHuds[ i ] = TNULL;
}

// $deBlob: FUNCTION 006550d0
// $deBlob: FUNCTION 00655820
// $deBlob: FUNCTION 00655830
ABlobHud_4P::~ABlobHud_4P()
{
	if ( AGameStatus::GetSingleton() != TNULL )
		AGameStatus::GetSingleton()->SetItMarker( TNULL );
}

// $deBlob: FUNCTION 00655140
void ABlobHud_4P::OnCreated()
{
	m_pLevelTimer = FindChildXUIControl( 0x2C );
	m_pLevelTimer->SetVisible( AGameStatus::GetSingleton()->GetLevelData().m_fTimeLimit != -1.0f );

	// NOTE: the result isn't used
	APlayerManager::GetSingleton()->GetNumPlayers();

	m_apScoreHuds[ 0 ] = SetupPlayerPanel( 0, m_apPanels[ 0 ], 0x1F, 0x21, 0x20, 0x26, 0x27, 0x24 );
	m_apScoreHuds[ 1 ] = SetupPlayerPanel( 1, m_apPanels[ 1 ], 0x15, 0x17, 0x16, 0x1C, 0x1D, 0x1A );
	m_apScoreHuds[ 2 ] = SetupPlayerPanel( 2, m_apPanels[ 2 ], 0xB, 0xD, 0xC, 0x12, 0x13, 0x10 );
	m_apScoreHuds[ 3 ] = SetupPlayerPanel( 3, m_apPanels[ 3 ], 1, 3, 2, 8, 9, 6 );

	SetPlayerLockElement( 0, m_apPanels[ 0 ], 0x23 );
	SetPlayerLockElement( 1, m_apPanels[ 1 ], 0x19 );
	SetPlayerLockElement( 2, m_apPanels[ 2 ], 0xF );
	SetPlayerLockElement( 3, m_apPanels[ 3 ], 5 );

	// The lock elements of the right halves move with the canvas when it isn't 16:9
	TFLOAT fCanvasX = TXUI::GetSingleton()->GetCanvas()->GetPositionX();

	if ( TRender::GetSingleton()->GetAspectRatio() != TRender::ASPECT_RATIO_16_9 )
		TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 1 ) )->GetZJumpComponent()->SetLockElementOffsetX( fCanvasX );

	if ( TRender::GetSingleton()->GetAspectRatio() != TRender::ASPECT_RATIO_16_9 )
		TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 3 ) )->GetZJumpComponent()->SetLockElementOffsetX( fCanvasX );

	m_pReadySetGo = FindChildXUIElement( 0x3C );

	// No input until the match starts
	ABlobInputMap::GetSingleton()->DisableFor( -1.0f );

	AGameState* pCurrState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pCurrState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pCurrState )->SetMultiplayerHud( this );

	m_pWinner = FindChildXUIControl( 0x2D );
	AGameStatus::GetSingleton()->SetItMarker( FindChildXUIElement( 0x71 ) );

	ABlobHud::OnCreated();
}

// $deBlob: FUNCTION 00655660
void ABlobHud_4P::Tick( TFLOAT a_fDeltaTime )
{
	// NOTE: element 0x3C is an XuiGroup, retail reads its timeline's playing flag (+0xB2)
	if ( m_bUnk134 && !TSTATICCAST( TXUIGroup, m_pReadySetGo )->GetTimelineControl()->IsPlaying() )
		OnReadySetGoEnded();

	ABlobHud::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00655620
void ABlobHud_4P::StartReadySetGo()
{
	m_bUnk134 = TTRUE;
	ASoundInterface::PlayEvent( ASoundMap::GROUPID( 0x10 ), ASoundMap::EVENTID( 0xE8 ), FMOD_EVENT_DEFAULT, TTRUE );
	m_pReadySetGo->PlayAnimation( L"ReadySetGo", L"EndReadySetGo" );
}

// $deBlob: FUNCTION 00655700
void ABlobHud_4P::ShowWinner( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::MAX_PLAYERS );

	switch ( a_iPlayer )
	{
		case 0: PlayAnimation( L"Player1Win", L"Player1Win" ); break;
		case 1: PlayAnimation( L"Player2Win", L"Player2Win" ); break;
		case 2: PlayAnimation( L"Player3Win", L"Player3Win" ); break;
		case 3: PlayAnimation( L"Player4Win", L"Player4Win" ); break;
	}

	m_pWinner->PlayAnimation( L"Winner", L"EndWinner" );
}

// $deBlob: FUNCTION 006557b0
TXUIElement* ABlobHud_4P::GetMessageElement( void** a_ppMessageOwner )
{
	*a_ppMessageOwner = FindChildXUIElement( 0x70 );
	return FindChildXUIElement( 0x6F );
}

// $deBlob: FUNCTION 006557d0
void ABlobHud_4P::Reset()
{
	m_apScoreHuds[ 0 ]->Reset();
	m_apScoreHuds[ 1 ]->Reset();
	m_apScoreHuds[ 2 ]->Reset();
	m_apScoreHuds[ 3 ]->Reset();

	if ( m_pReadySetGo != TNULL )
		m_pReadySetGo->PlayAnimation( L"Normal", L"EndNormal" );

	ABlobHud::Reset();
}

// $deBlob: FUNCTION 006558b0
ABlobMultiPlayerHud* ABlobHud_4P::GetPlayer1Hud()
{
	return m_apPanels[ 0 ];
}

// $deBlob: FUNCTION 006558c0
ABlobMultiPlayerHud* ABlobHud_4P::GetPlayer2Hud()
{
	return m_apPanels[ 1 ];
}

// $deBlob: FUNCTION 006558d0
ABlobMultiPlayerHud* ABlobHud_4P::GetPlayer3Hud()
{
	return m_apPanels[ 2 ];
}

// $deBlob: FUNCTION 006558e0
ABlobMultiPlayerHud* ABlobHud_4P::GetPlayer4Hud()
{
	return m_apPanels[ 3 ];
}

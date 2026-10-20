#include "pch.h"
#include "ABlobHud_3P.h"
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

// $deBlob: FUNCTION 006546f0
// $deBlob: FUNCTION 00654700
// $deBlob: FUNCTION 00654770
// $deBlob: FUNCTION 006547e0
// $deBlob: FUNCTION 006547f0
// $deBlob: FUNCTION 0053e240
// $deBlob: FUNCTION 00858c00
TDEFINE_CLASS( ABlobHud_3P );

// $deBlob: FUNCTION 00654800
ABlobHud_3P::ABlobHud_3P()
{
	for ( TINT i = 0; i < NUM_PLAYERS; i++ )
		m_apPanels[ i ] = TNULL;

	m_pReadySetGo = TNULL;
	m_pWinner     = TNULL;

	for ( TINT i = 0; i < NUM_PLAYERS; i++ )
		m_apScoreHuds[ i ] = TNULL;
}

// $deBlob: FUNCTION 00654870
// $deBlob: FUNCTION 00654e85
// $deBlob: FUNCTION 00654e90
ABlobHud_3P::~ABlobHud_3P()
{
	if ( AGameStatus::GetSingleton() != TNULL )
		AGameStatus::GetSingleton()->SetItMarker( TNULL );
}

// $deBlob: FUNCTION 006548e0
void ABlobHud_3P::OnCreated()
{
	m_pLevelTimer = FindChildXUIControl( 0x33 );
	m_pLevelTimer->SetVisible( AGameStatus::GetSingleton()->GetLevelData().m_fTimeLimit != -1.0f );

	// NOTE: the result isn't used
	APlayerManager::GetSingleton()->GetNumPlayers();

	m_apScoreHuds[ 0 ] = SetupPlayerPanel( 0, m_apPanels[ 0 ], 0x26, 0x28, 0x27, 0x2D, 0x2E, 0x2B );
	m_apScoreHuds[ 1 ] = SetupPlayerPanel( 1, m_apPanels[ 1 ], 0x1C, 0x1E, 0x1D, 0x23, 0x24, 0x21 );
	m_apScoreHuds[ 2 ] = SetupPlayerPanel( 2, m_apPanels[ 2 ], 0x12, 0x14, 0x13, 0x19, 0x1A, 0x17 );

	SetPlayerLockElement( 0, m_apPanels[ 0 ], 0x2A );
	SetPlayerLockElement( 1, m_apPanels[ 1 ], 0x20 );
	SetPlayerLockElement( 2, m_apPanels[ 2 ], 0x16 );

	// The lock element of the right half moves with the canvas when it isn't 16:9
	if ( TRender::GetSingleton()->GetAspectRatio() != TRender::ASPECT_RATIO_16_9 )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 1 ) );
		pBlob->GetZJumpComponent()->SetLockElementOffsetX( TXUI::GetSingleton()->GetCanvas()->GetPositionX() );
	}

	m_pReadySetGo = FindChildXUIElement( 0x43 );

	// No input until the match starts
	ABlobInputMap::GetSingleton()->DisableFor( -1.0f );

	AGameState* pCurrState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pCurrState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pCurrState )->SetMultiplayerHud( this );

	m_pWinner = FindChildXUIControl( 0x34 );
	AGameStatus::GetSingleton()->SetItMarker( FindChildXUIElement( 0x78 ) );

	ABlobHud::OnCreated();
}

// $deBlob: FUNCTION 00654cf0
void ABlobHud_3P::Tick( TFLOAT a_fDeltaTime )
{
	// NOTE: element 0x43 is an XuiGroup, retail reads its timeline's playing flag (+0xB2)
	if ( m_bUnk134 && !TSTATICCAST( TXUIGroup, m_pReadySetGo )->GetTimelineControl()->IsPlaying() )
		OnReadySetGoEnded();

	ABlobHud::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00654cb0
void ABlobHud_3P::StartReadySetGo()
{
	m_bUnk134 = TTRUE;
	ASoundInterface::PlayEvent( ASoundMap::GROUPID( 0x10 ), ASoundMap::EVENTID( 0xE8 ), FMOD_EVENT_DEFAULT, TTRUE );
	m_pReadySetGo->PlayAnimation( L"ReadySetGo", L"EndReadySetGo" );
}

// $deBlob: FUNCTION 00654d80
void ABlobHud_3P::ShowWinner( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < 3 );

	switch ( a_iPlayer )
	{
		case 0:
			PlayAnimation( L"Player1Win", L"Player1Win" );
			break;
		case 1:
			PlayAnimation( L"Player2Win", L"Player2Win" );
			break;
		case 2:
			PlayAnimation( L"Player3Win", L"Player3Win" );
			break;
	}

	m_pWinner->PlayAnimation( L"Winner", L"EndWinner" );
}

// $deBlob: FUNCTION 00654e20
TXUIElement* ABlobHud_3P::GetMessageElement( void** a_ppMessageOwner )
{
	*a_ppMessageOwner = FindChildXUIElement( 0x77 );
	return FindChildXUIElement( 0x76 );
}

// $deBlob: FUNCTION 00654e40
void ABlobHud_3P::Reset()
{
	m_apScoreHuds[ 0 ]->Reset();
	m_apScoreHuds[ 1 ]->Reset();
	m_apScoreHuds[ 2 ]->Reset();

	if ( m_pReadySetGo != TNULL )
		m_pReadySetGo->PlayAnimation( L"Normal", L"EndNormal" );

	ABlobHud::Reset();
}

// $deBlob: FUNCTION 00654f10
ABlobMultiPlayerHud* ABlobHud_3P::GetPlayer1Hud()
{
	return m_apPanels[ 0 ];
}

// $deBlob: FUNCTION 00654f20
ABlobMultiPlayerHud* ABlobHud_3P::GetPlayer2Hud()
{
	return m_apPanels[ 1 ];
}

// $deBlob: FUNCTION 00654f30
ABlobMultiPlayerHud* ABlobHud_3P::GetPlayer3Hud()
{
	return m_apPanels[ 2 ];
}

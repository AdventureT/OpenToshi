#include "pch.h"
#include "ABlobHud_2P.h"
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

// $deBlob: FUNCTION 00653ed0
// $deBlob: FUNCTION 00653ee0
// $deBlob: FUNCTION 00653f90
// $deBlob: FUNCTION 00654040
// $deBlob: FUNCTION 00654050
// $deBlob: FUNCTION 0053e200
// $deBlob: FUNCTION 00858bf0
TDEFINE_CLASS( ABlobHud_2P );

// $deBlob: FUNCTION 00654060
ABlobHud_2P::ABlobHud_2P()
{
	m_apPanels[ 0 ]    = TNULL;
	m_apPanels[ 1 ]    = TNULL;
	m_pWinner          = TNULL;
	m_apScoreHuds[ 0 ] = TNULL;
	m_apScoreHuds[ 1 ] = TNULL;

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves it to OnCreated
	m_pReadySetGo = TNULL;
#endif
}

// $deBlob: FUNCTION 006540b0
// $deBlob: FUNCTION 0065459a
// $deBlob: FUNCTION 006545b0
ABlobHud_2P::~ABlobHud_2P()
{
	if ( AGameStatus::GetSingleton() != TNULL )
		AGameStatus::GetSingleton()->SetItMarker( TNULL );
}

// $deBlob: FUNCTION 00654120
void ABlobHud_2P::OnCreated()
{
	m_pLevelTimer = FindChildXUIControl( 0x17 );
	m_pLevelTimer->SetVisible( AGameStatus::GetSingleton()->GetLevelData().m_fTimeLimit != -1.0f );

	m_apScoreHuds[ 0 ] = SetupPlayerPanel( 0, m_apPanels[ 0 ], 0xB, 0xD, 0xC, 0x12, 0x13, 0x10 );
	m_apScoreHuds[ 1 ] = SetupPlayerPanel( 1, m_apPanels[ 1 ], 1, 3, 2, 8, 9, 6 );

	SetPlayerLockElement( 0, m_apPanels[ 0 ], 0xF );
	SetPlayerLockElement( 1, m_apPanels[ 1 ], 5 );

	// The lock element of the right half moves with the canvas when it isn't 16:9
	if ( TRender::GetSingleton()->GetAspectRatio() != TRender::ASPECT_RATIO_16_9 )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 1 ) );
		pBlob->GetZJumpComponent()->SetLockElementOffsetX( TXUI::GetSingleton()->GetCanvas()->GetPositionX() );
	}

	m_pReadySetGo = FindChildXUIElement( 0x27 );

	// No input until the match starts
	ABlobInputMap::GetSingleton()->DisableFor( -1.0f );

	AGameState* pCurrState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pCurrState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pCurrState )->SetMultiplayerHud( this );

	m_pWinner = FindChildXUIControl( 0x18 );
	AGameStatus::GetSingleton()->SetItMarker( FindChildXUIElement( 0x5C ) );

	ABlobHud::OnCreated();
}

// $deBlob: FUNCTION 006543f0
void ABlobHud_2P::Tick( TFLOAT a_fDeltaTime )
{
	// NOTE: element 0x27 is an XuiGroup, retail reads its timeline's playing flag (+0xB2)
	if ( m_bUnk134 && !TSTATICCAST( TXUIGroup, m_pReadySetGo )->GetTimelineControl()->IsPlaying() )
		OnReadySetGoEnded();

	ABlobHud::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00654470
void ABlobHud_2P::StartReadySetGo()
{
	m_bUnk134 = TTRUE;
	ASoundInterface::PlayEvent( ASoundMap::GROUPID( 0x10 ), ASoundMap::EVENTID( 0xE8 ), FMOD_EVENT_DEFAULT, TTRUE );
	m_pReadySetGo->PlayAnimation( L"ReadySetGo", L"EndReadySetGo" );
}

// $deBlob: FUNCTION 006544b0
void ABlobHud_2P::ShowWinner( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < 2 );

	switch ( a_iPlayer )
	{
		case 0:
			PlayAnimation( L"Player1Win", L"Player1Win" );
			break;
		case 1:
			PlayAnimation( L"Player2Win", L"Player2Win" );
			break;
	}

	m_pWinner->PlayAnimation( L"Winner", L"EndWinner" );
}

// $deBlob: FUNCTION 00654540
TXUIElement* ABlobHud_2P::GetMessageElement( void** a_ppMessageOwner )
{
	*a_ppMessageOwner = FindChildXUIElement( 0x5B );
	return FindChildXUIElement( 0x5A );
}

// $deBlob: FUNCTION 00654560
void ABlobHud_2P::Reset()
{
	m_apScoreHuds[ 0 ]->Reset();
	m_apScoreHuds[ 1 ]->Reset();

	if ( m_pReadySetGo != TNULL )
		m_pReadySetGo->PlayAnimation( L"Normal", L"EndNormal" );

	ABlobHud::Reset();
}

// $deBlob: FUNCTION 00654630
ABlobMultiPlayerHud* ABlobHud_2P::GetPlayer1Hud()
{
	return m_apPanels[ 0 ];
}

// $deBlob: FUNCTION 00654640
ABlobMultiPlayerHud* ABlobHud_2P::GetPlayer2Hud()
{
	return m_apPanels[ 1 ];
}

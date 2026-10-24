#include "pch.h"
#include "ABlobDebugMPMode.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/APlayerManager.h"
#include "GameInterface/AXUIState.h"

#include <XUI/TXUIListItem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobDebugMPMode ) == 0xF0 );

// $deBlob: FUNCTION 0063cb00
// $deBlob: FUNCTION 0063cb10
// $deBlob: FUNCTION 0063cb90
// $deBlob: FUNCTION 0063cc10
// $deBlob: FUNCTION 0063cc20
// $deBlob: FUNCTION 0053d910
// $deBlob: FUNCTION 008589c0
TDEFINE_CLASS( ABlobDebugMPMode );

// $deBlob: FUNCTION 0063cd70
ABlobDebugMPMode::ABlobDebugMPMode()
{
}

// $deBlob: FUNCTION 0063cd90
// $deBlob: FUNCTION 0063cd95
// $deBlob: FUNCTION 0063cda0
ABlobDebugMPMode::~ABlobDebugMPMode()
{
}

// $deBlob: FUNCTION 0063cc30
void ABlobDebugMPMode::OnItemPressed( TXUIListItem* a_pItem )
{
	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		if ( a_pItem != &m_pItems[ i ] )
			continue;

		TASSERT( i >= 0 && i <= AGameStatus::GAMETYPE_MP_COUNT );
		AGameStatus::GetSingleton()->SetMultiPlayerGameType( i );

		// The default colours of the players
		APlayerManager* pPlayerManager = APlayerManager::GetSingleton();

		switch ( pPlayerManager->GetNumPlayers() )
		{
			case 4:
				pPlayerManager->SetPlayerColour( 3, 6 );
				TFALLTHROUGH;
			case 3:
				pPlayerManager->SetPlayerColour( 2, 2 );
				TFALLTHROUGH;
			case 2:
				pPlayerManager->SetPlayerColour( 1, 3 );
				TFALLTHROUGH;
			case 1:
				pPlayerManager->SetPlayerColour( 0, 4 );
				break;
		}

		AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
		TASSERT( TTRUE == pState->IsA( &TGetClass( AXUIState ) ) );
		TSTATICCAST( AXUIState, pState )->SetNextState( AXUIState::NextState_LoadLevel );
	}

	TXUICommonList::OnItemPressed( a_pItem );
}

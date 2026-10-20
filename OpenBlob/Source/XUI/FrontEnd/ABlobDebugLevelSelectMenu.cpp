#include "pch.h"
#include "ABlobDebugLevelSelectMenu.h"
#include "Lists/ABlobDebugMenuList.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AXUIState.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobDebugLevelSelectMenu ) == 0x144 );

// $deBlob: FUNCTION 0062fa90
// $deBlob: FUNCTION 0062faa0
// $deBlob: FUNCTION 0062fb30
// $deBlob: FUNCTION 0062fbb0
// $deBlob: FUNCTION 0062fbc0
// $deBlob: FUNCTION 0053d390
// $deBlob: FUNCTION 00858860
TDEFINE_CLASS( ABlobDebugLevelSelectMenu );

// $deBlob: FUNCTION 0062fd10
ABlobDebugLevelSelectMenu::ABlobDebugLevelSelectMenu()
    : ABlobMenu( -1 )
{
	m_pSelect = TNULL;
}

// $deBlob: FUNCTION 0062fd40
// $deBlob: FUNCTION 0062fd52
// $deBlob: FUNCTION 0062fd60
ABlobDebugLevelSelectMenu::~ABlobDebugLevelSelectMenu()
{
}

// $deBlob: FUNCTION 0062fbd0
void ABlobDebugLevelSelectMenu::OnCreated()
{
	// NOTE: retail doesn't call ABlobMenu::OnCreated
	m_pSelect = FindChildXUIControl( 2 );
}

// $deBlob: FUNCTION 0062fbf0
TBOOL ABlobDebugLevelSelectMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( a_pSource != m_pSelect )
		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AXUIState ) ) );
	AXUIState* pXUIState = TSTATICCAST( AXUIState, pState );

	const TCHAR* szLevel = pXUIState->GetNextLevel();

	if ( TStringManager::String8Length( szLevel ) >= 3 && szLevel[ 0 ] == 'M' && szLevel[ 1 ] == 'P' && szLevel[ 2 ] == '_' && !ABlobDebugMenuList::s_bMusicTester )
	{
		TXUI* pXUI = TXUI::GetSingleton();
		pXUI->TransitionTo( this, pXUI->CreateScene( pXUI->FindResource( "debugnumberplayers.xur" ), TUINT32( 0 ) ) );
		return TTRUE;
	}

	AGameStatus::GetSingleton()->SetLevelMode( -1 );

	if ( ABlobDebugMenuList::s_bMusicTester )
	{
		if ( pXUIState->GetLevelIndex() != -1 )
			pXUIState->SetNextState( AXUIState::NextState_Unk3 );
	}
	else
	{
		pXUIState->SetNextState( AXUIState::NextState_LoadLevel );
	}

	return TTRUE;
}

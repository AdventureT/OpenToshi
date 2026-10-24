#include "pch.h"
#include "ABlobLanguageMenu.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AXUIState.h"
#include "GUI/AGUIManager.h"
#include "Locale/ALocaleManager.h"
#include "SteamManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 006328f0
// $deBlob: FUNCTION 00632900
// $deBlob: FUNCTION 00632980
// $deBlob: FUNCTION 00632a00
// $deBlob: FUNCTION 00632a10
// $deBlob: FUNCTION 0053d5d0
// $deBlob: FUNCTION 008588f0
TDEFINE_CLASS( ABlobLanguageMenu );

ABlobLanguageMenu::Language ABlobLanguageMenu::sm_aLanguages[ NUM_LANGUAGES ] = {
	{ 3, 0, 0x1BA, TNULL },
	{ 4, 7, 0x1BB, TNULL },
	{ 6, 6, 0x1BD, TNULL },
};

// $deBlob: FUNCTION 00633040
ABlobLanguageMenu::ABlobLanguageMenu()
    : ABlobMenu( -1 )
{
}

// $deBlob: FUNCTION 00633080
// $deBlob: FUNCTION 00633060
// $deBlob: FUNCTION 00633072
ABlobLanguageMenu::~ABlobLanguageMenu()
{
}

// $deBlob: FUNCTION 00632a80
void ABlobLanguageMenu::Tick( TFLOAT a_fDeltaTime )
{
	TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() )->SetLanguage( SteamManager::GetLanguageId() );

	AXUIState::SetSkin1( "commonskin.trb", "commonskin.xur" );
	AXUIState::SetSkin2( "frontendskin.trb", "frontendskin.xur" );
	AXUIState::SetCommonSkin( "common.trb" );
	AXUIState::SetSkinWithFont( "frontend.trb", TFALSE );

	// The skins were reloaded: the cursor is created again from the new skin
	AGUIManager::GetSingleton()->Reinitialize();

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( pState->IsA( &TGetClass( AXUIState ) ) );

	TSTATICCAST( AXUIState, pState )->SetNextState( AXUIState::NextState_SaveLoadStart );
}

// $deBlob: FUNCTION 00632a20
void ABlobLanguageMenu::OnCreated()
{
	m_iSelectedLanguage = -1;
}

// $deBlob: FUNCTION 00632a30
TBOOL ABlobLanguageMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	for ( TINT i = 0; i < NUM_LANGUAGES; i++ )
	{
		if ( a_pSource == sm_aLanguages[ i ].m_pButton )
		{
			m_iSelectedLanguage = i;
			return TTRUE;
		}
	}

	return TFALSE;
}

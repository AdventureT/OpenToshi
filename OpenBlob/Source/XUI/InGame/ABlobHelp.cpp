#include "pch.h"
#include "ABlobHelp.h"
#include "Entities/APropTriggerHelp.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "Locale/ALocaleManager.h"

#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUIBackButton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobHelp ) == 0x160 );

const TINT ABlobHelp::ms_aiButtonTopics[ NUM_TOPIC_BUTTONS ] = { 6, 4, 8, 3, 10, 9, 7, 1 };

// $deBlob: FUNCTION 00644630
// $deBlob: FUNCTION 00644640
// $deBlob: FUNCTION 006446c0
// $deBlob: FUNCTION 00644740
// $deBlob: FUNCTION 00644750
// $deBlob: FUNCTION 0053dc50
// $deBlob: FUNCTION 00858a90
TDEFINE_CLASS( ABlobHelp );

// $deBlob: FUNCTION 006448f0
ABlobHelp::ABlobHelp()
    : ABlobMenu( -1 )
{
}

// $deBlob: FUNCTION 00644910
// $deBlob: FUNCTION 00644930
// $deBlob: FUNCTION 00644922
ABlobHelp::~ABlobHelp()
{
}

// $deBlob: FUNCTION 00644760
void ABlobHelp::OnCreated()
{
	ABlobMenu::OnCreated();

	for ( TINT i = 0; i < NUM_TOPIC_BUTTONS; i++ )
		m_apTopicButtons[ i ] = FindChildXUIControl( 12 - i );
}

// $deBlob: FUNCTION 00644790
TBOOL ABlobHelp::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( APropTriggerHelp::GetFullscreenTopic() != -1 )
		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	TINT iButton = 0;

	while ( a_pSource != m_apTopicButtons[ iButton ] )
	{
		if ( ++iButton >= NUM_TOPIC_BUTTONS )
			return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
	}

	// The topic's TRB is streamed by AInGameState, then the full screen help is opened over this menu
	TCHAR* szFileName = TStringManager::GetTempString8();
	TStringManager::String8Format( szFileName, "data/xui/%s/%s.trb", TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() )->GetCurLocaleCode(), APropTriggerHelp::GetFullscreenTopicName( ms_aiButtonTopics[ iButton ] ) );
	APropTriggerHelp::SetFullscreenTopic( TINT8( ms_aiButtonTopics[ iButton ] ) );
	APropTriggerHelp::SetHelpMenu( this );

	AGameState* pGameState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( pGameState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pGameState )->RequestStreamedTRB( APropTriggerHelp::GetFullscreenTRB(), szFileName, TNULL );

	if ( m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TFALSE );

	ABlobMenu::PlayPressSound( a_pSource );
	return TTRUE;
}

// $deBlob: FUNCTION 006448b0
void ABlobHelp::EndTransBackTo()
{
	if ( m_pBackButton != TNULL )
		m_pBackButton->SetEnabled( TTRUE );

	ABlobMenu::EndTransBackTo();
}

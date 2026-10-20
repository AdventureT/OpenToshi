#include "pch.h"
#include "ABlobHelpHud.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/APropTriggerHelp.h"
#include "GameInterface/APlayerManager.h"
#include "Input/ABlobInputMap.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundInterface.h"

#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobHelpHud ) == 0x148 );

// The button that closes the popup (ABlobInputMap::PlayerInput::m_uiButtons)
static TCONSTEXPR_VAR TUINT32 s_uiCloseButton = 0x10000;

// $deBlob: FUNCTION 00651440
// $deBlob: FUNCTION 00651450
// $deBlob: FUNCTION 006514e0
// $deBlob: FUNCTION 00651570
// $deBlob: FUNCTION 00651580
// $deBlob: FUNCTION 0053e110
// $deBlob: FUNCTION 00858bc0
TDEFINE_CLASS( ABlobHelpHud );

// $deBlob: FUNCTION 00651590
ABlobHelpHud::ABlobHelpHud()
    : m_fDisplayTime( -1.0f )
    , m_bInkHelp( TFALSE )
{
}

// $deBlob: FUNCTION 006518f0
// $deBlob: FUNCTION 006518e5
ABlobHelpHud::~ABlobHelpHud()
{
}

// $deBlob: FUNCTION 006515c0
void ABlobHelpHud::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_fDisplayTime > 0.0f )
	{
		ABlobInputMap* pInputMap = ABlobInputMap::GetSingleton();
		TUINT32        uiButtons = pInputMap->GetButtons( 0 );

		TBOOL bClose = !pInputMap->IsDisabled() && ( ( uiButtons ^ pInputMap->GetPrevButtons( 0 ) ) & uiButtons & s_uiCloseButton );

		if ( !bClose && m_bInkHelp )
		{
			ABlobEntity* pBlob     = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
			TUINT32      uiColour = pBlob->GetColourComponent()->GetCurrentColour();

			// The blob isn't ink any more
			if ( !( uiColour & ABlobColourComponent::COLOUR_BLACK ) && uiColour != ABlobColourComponent::COLOUR_BLACK )
				bClose = TTRUE;
		}

		if ( bClose )
		{
			m_bInkHelp     = TFALSE;
			m_fDisplayTime = 0.0f;
		}

		m_fDisplayTime -= a_fDeltaTime;

		if ( m_fDisplayTime <= 0.0f )
		{
			m_pVisual->PlayAnimation( L"Hide", L"EndNormal" );
			APropTriggerHelp::ResetHelpState();
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00651690
void ABlobHelpHud::OnCreated()
{
	TWCHAR* wszName = TStringManager::GetTempString16();

	for ( TINT i = 0; i < iMAX; )
	{
		i++;
		TStringManager::String16Format( wszName, 0x10, L"RevHead%d", i );
		m_apRevHeads[ i - 1 ] = m_pVisual->FindChildXUIElement( wszName );
	}
}

// $deBlob: FUNCTION 006516e0
void ABlobHelpHud::ShowMessage( TINT a_iStringId, TFLOAT a_fDisplayTime, TINT a_iSpeaker, const TWCHAR* a_wszText )
{
	m_bInkHelp = a_iStringId == INK_HELP_STRING;

	ALocaleManager* pLocale = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );
	SetText( a_wszText ? a_wszText : pLocale->GetString( a_iStringId ), -1 );

	ALocaleManager::StringFormat* pFormat;

	if ( a_wszText == TNULL && ( pFormat = pLocale->GetStringFormat( a_iStringId ) ) != TNULL )
		SetText2( pLocale->GetOverwrittenFormatString( a_iStringId, pFormat->m_wszFormat ), pLocale->GetFormatUnk04() );
	else
		SetText2( TNULL, 0 );

	m_fDisplayTime = a_fDisplayTime;
	m_pVisual->PlayAnimation( L"Show", L"EndShow" );

	for ( TINT i = 0; i < iMAX; i++ )
		m_apRevHeads[ i ]->SetOpacity( 0.0f );

	TASSERT( a_iSpeaker >= 0 && a_iSpeaker < iMAX );
	m_apRevHeads[ a_iSpeaker ]->SetOpacity( 1.0f );

	// The voice of each speaker
	ASoundMap::EVENTID aeSpeakerSounds[ iMAX ];
	aeSpeakerSounds[ 0 ] = ASoundMap::EVENTID( 0x4A );
	aeSpeakerSounds[ 1 ] = ASoundMap::EVENTID( 0x48 );
	aeSpeakerSounds[ 2 ] = ASoundMap::EVENTID( 0x4C );
	aeSpeakerSounds[ 3 ] = ASoundMap::EVENTID( 0x49 );
	ASoundInterface::PlayEvent( ASoundMap::GROUPID_GUI, aeSpeakerSounds[ a_iSpeaker ], FMOD_EVENT_DEFAULT, TTRUE );
}

// $deBlob: FUNCTION 00651840
void ABlobHelpHud::Hide()
{
	m_pVisual->PlayAnimation( L"Hide", L"EndNormal" );
	m_fDisplayTime = -1.0f;
	m_bInkHelp     = TFALSE;

	for ( TINT i = 0; i < iMAX; i++ )
		m_apRevHeads[ i ]->SetOpacity( 0.0f );
}

// $deBlob: FUNCTION 00651890
void ABlobHelpHud::HideNow()
{
	m_pVisual->PlayAnimation( L"EndNormal", L"EndNormal" );
	m_fDisplayTime = -1.0f;
	m_bInkHelp     = TFALSE;

	for ( TINT i = 0; i < iMAX; i++ )
		m_apRevHeads[ i ]->SetOpacity( 0.0f );
}

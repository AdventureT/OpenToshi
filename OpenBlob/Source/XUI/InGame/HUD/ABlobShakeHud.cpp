#include "pch.h"
#include "ABlobShakeHud.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/Enemies/AEnemyGrappler.h"
#include "GameInterface/APlayerManager.h"
#include "Locale/ALocaleManager.h"

#include <Input/TInputInterface.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobShakeHud ) == 0x13C );

// The texts: "SHAKE!!" (consoles), the PC key and its action
static TCONSTEXPR_VAR TINT s_iShakeString     = 0x32C;
static TCONSTEXPR_VAR TINT s_iShakeKeyString  = 0x36D;
static TCONSTEXPR_VAR TINT s_iShakeNameString = 0x1EC;

// The PC prompt
static TWCHAR s_wszShakeText[ 0x100 ];

// $deBlob: FUNCTION 00659bd0
// $deBlob: FUNCTION 00659be0
// $deBlob: FUNCTION 00659c70
// $deBlob: FUNCTION 00659d00
// $deBlob: FUNCTION 00659d10
// $deBlob: FUNCTION 0053e540
// $deBlob: FUNCTION 00858cc0
TDEFINE_CLASS( ABlobShakeHud );

// $deBlob: FUNCTION 0065a0f0
ABlobShakeHud::ABlobShakeHud()
    : m_pProgress( TNULL )
    , m_eMode( MODE_NONE )
{
}

// $deBlob: FUNCTION 0065a130
// $deBlob: FUNCTION 0065a125
ABlobShakeHud::~ABlobShakeHud()
{
}

// $deBlob: FUNCTION 00659d20
void ABlobShakeHud::OnCreated()
{
	m_pProgress = TSTATICCAST( TXUIProgressBar, FindChildXUIControl( 0xDA ) );
	m_pPrompt   = FindChildXUIControl( 0xDB );

	ALocaleManager* pLocale = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );
	m_pProgress->GetVisual()->FindChildXUIControl( L"Message" )->SetText( pLocale->GetString( s_iShakeString ), -1 );

	m_pProgress->GetVisual()->PlayAnimation( L"EndShakeComplete", L"EndShakeComplete" );
	m_pPrompt->GetVisual()->PlayAnimation( L"EndShakeComplete", L"EndShakeComplete" );
	m_pProgress->SetValue( 0 );
}

// $deBlob: FUNCTION 00659dc0
void ABlobShakeHud::Tick( TFLOAT a_fDeltaTime )
{
	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
	{
		ABlobEntity* pBlob    = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
		TBOOL        bActive  = TTRUE;
		TBOOL        bDone    = TFALSE;
		TINT         eNewMode = MODE_NONE;

		if ( pBlob->GetBlobState() == ABlobEntity::STATE_HOTPLATE )
			eNewMode = MODE_HOTPLATE;
		else if ( pBlob->GetBlobState() == ABlobEntity::STATE_LMARK && pBlob->GetShakeProgress() < 100 )
			eNewMode = MODE_LANDMARK;
		else if ( pBlob->GetNumLeechbotsAttached() > 0 )
			eNewMode = MODE_GRAPPLERS;
		else
			bActive = TFALSE;

		if ( bActive )
		{
			TBOOL bStart = m_eMode == MODE_NONE;

			if ( bStart )
				m_pProgress->SetValue( 0 );

			m_eMode = eNewMode;

			if ( bStart )
				ShowPrompt();
		}
		else if ( m_eMode != MODE_NONE )
		{
			// Shaken off
			bDone = TTRUE;
		}

		if ( m_eMode == MODE_GRAPPLERS )
		{
			m_pProgress->SetValue( AEnemyGrappler::GetStruggleProgress( pBlob ) );
		}
		else if ( m_eMode == MODE_LANDMARK )
		{
			if ( bDone )
				m_pProgress->SetValue( 100 );
			else
				m_pProgress->SetValue( pBlob->GetShakeProgress() );
		}

		if ( bDone )
		{
			m_pPrompt->GetVisual()->PlayAnimation( L"ShakeComplete", L"EndShakeComplete" );
			m_pProgress->GetVisual()->PlayAnimation( L"ShakeComplete", L"EndShakeComplete" );
			m_eMode = MODE_NONE;
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

void ABlobShakeHud::ShowPrompt()
{
	// Gamepad_Wii shows the console glyph, the others show the PC key
	TBOOL bConsole = TInputDevice::GetLastGamepadType() == TInputDevice::Gamepad_Wii;

	m_pPrompt->GetVisual()->FindChildXUIElement( L"Shake" )->SetVisible( bConsole );
	m_pPrompt->GetVisual()->FindChildXUIElement( L"Shake_pc" )->SetVisible( !bConsole );

	TXUIControl*    pMessage = TSTATICCAST( TXUIControl, m_pProgress->GetVisual()->FindChildXUIElement( L"Message" ) );
	ALocaleManager* pLocale  = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );

	if ( TInputDevice::GetLastGamepadType() == TInputDevice::Gamepad_Wii )
	{
		pMessage->SetText( pLocale->GetString( s_iShakeString ), -1 );
	}
	else
	{
		const TWCHAR* wszName = pLocale->GetString( s_iShakeNameString );
		const TWCHAR* wszKey  = pLocale->GetString( s_iShakeKeyString );
		swprintf( s_wszShakeText, 0x100, L"%ls %ls", wszKey, wszName );
		pMessage->SetText( s_wszShakeText, -1 );
	}

	m_pPrompt->GetVisual()->PlayAnimation( L"StartShake", L"EndStartShake" );
	m_pProgress->GetVisual()->PlayAnimation( L"StartShake", L"EndStartShake" );
}

// $deBlob: FUNCTION 0065a080
void ABlobShakeHud::Reset()
{
	m_pProgress->GetVisual()->PlayAnimation( L"EndShakeComplete", L"EndShakeComplete" );
	m_pPrompt->GetVisual()->PlayAnimation( L"EndShakeComplete", L"EndShakeComplete" );
	m_pProgress->SetValue( 0 );
}

// $deBlob: FUNCTION 0065a0d0
void ABlobShakeHud::SetHotplateProgress( TINT a_iProgress )
{
	if ( m_eMode == MODE_HOTPLATE )
		m_pProgress->SetValue( a_iProgress );
}

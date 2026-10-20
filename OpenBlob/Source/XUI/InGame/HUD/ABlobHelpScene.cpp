#include "pch.h"
#include "ABlobHelpScene.h"
#include "ABlobHelpHud.h"
#include "ABlobHud.h"
#include "Buildings/ABuildingManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/APropTriggerHelp.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerManager.h"
#include "Input/ABlobInputMap.h"
#include "Locale/ALocaleManager.h"
#include "Memory/AMemory.h"
#include "Sound/ASoundInterface.h"
#include "XUI/ElementTypes/ABlobTextReveal.h"

#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobHelpScene ) == 0x1C0 );

// The strings of the continue button: "Close" on the last page, "Next" on the others
static TCONSTEXPR_VAR TINT s_iCloseString = 0xB4;
static TCONSTEXPR_VAR TINT s_iNextString  = 0xB5;

// The button that closes the help (cleared from the input when the scene closes)
static TCONSTEXPR_VAR TUINT32 s_uiCloseButton = 0x4;

// $deBlob: FUNCTION 00651920
// $deBlob: FUNCTION 00651930
// $deBlob: FUNCTION 006519a0
// $deBlob: FUNCTION 00651a10
// $deBlob: FUNCTION 00651a20
// $deBlob: FUNCTION 0053e150
// $deBlob: FUNCTION 00858bd0
TDEFINE_CLASS( ABlobHelpScene );

// $deBlob: FUNCTION 00651a30
ABlobHelpScene::ABlobHelpScene()
    : m_pTrigger( TNULL )
    , m_pSpeech( TNULL )
    , m_pContinue( TNULL )
    , m_pText( TNULL )
    , m_bPaged( TFALSE )
    , m_iNumPages( 0 )
    , m_iCurrentPage( 0 )
    , m_iStringId( -1 )
{
}

// $deBlob: FUNCTION 00651aa0
// $deBlob: FUNCTION 006522e0
// $deBlob: FUNCTION 006522ce
ABlobHelpScene::~ABlobHelpScene()
{
	for ( TINT i = 0; i < m_iNumPages; i++ )
		TFree( m_apPages[ i ] );

	APropTriggerHelp::ResetHelpState();
}

// $deBlob: FUNCTION 00651b30
void ABlobHelpScene::OnCreated()
{
	m_pSpeech   = FindChildXUIElement( 1 );
	m_pContinue = FindChildXUIControl( 0xC );
	m_pContinue->SetVisible( TFALSE );
	m_pText = TSTATICCAST( ABlobTextReveal, FindChildXUIControl( 5 ) );

	FindChildXUIElement( 7 )->SetOpacity( 1.0f );
	FindChildXUIElement( 6 )->SetOpacity( 0.0f );
	FindChildXUIElement( 0xB )->SetOpacity( 0.0f );
}

// $deBlob: FUNCTION 00651bc0
void ABlobHelpScene::Tick( TFLOAT a_fDeltaTime )
{
	TXUIScene* pActiveScene  = TXUI::GetSingleton()->GetActiveScene();
	TXUIScene* pTransTo      = TXUI::GetSingleton()->GetTransToScene();
	TBOOL      bInTransition = pTransTo != TNULL || TXUI::GetSingleton()->GetTransFromScene() != TNULL || TXUI::GetSingleton()->IsInputLocked();

	if ( pActiveScene == this || ( bInTransition && pTransTo == this ) )
	{
		// The game is paused: the players and the buildings keep updating
		APlayerManager::GetSingleton()->Update( a_fDeltaTime );
		ABuildingManager::GetSingleton()->Update( a_fDeltaTime );

		if ( !m_pText->IsRevealing() )
			m_pContinue->SetVisible( TTRUE );

		TXUIScene::Tick( a_fDeltaTime );
		return;
	}

	// Waits for the HUD to be the active scene
	if ( bInTransition || pActiveScene == TNULL || !pActiveScene->IsA( &TGetClass( ABlobHud ) ) )
		return;

	ABlobHelpHud* pHelpHud = APropTriggerHelp::GetHelpHud();

	if ( pHelpHud != TNULL && APropTriggerHelp::IsHelpHudShown() )
	{
		pHelpHud->Hide();
		APropTriggerHelp::SetHelpHudShown( TFALSE );
	}

	TXUI::GetSingleton()->TransitionTo( TNULL, this );

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pState )->SetPaused( TTRUE );
	ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID_INGAME, TTRUE );
}

// $deBlob: FUNCTION 00651d40
void ABlobHelpScene::SetHelpText( TINT a_iStringId, const TWCHAR* a_wszText )
{
	m_iStringId = ( a_wszText == TNULL ) ? a_iStringId : -1;

	ALocaleManager* pLocale = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );

	if ( m_iStringId == -1 && a_wszText == TNULL )
		return;

	const TWCHAR* wszText    = a_wszText ? a_wszText : pLocale->GetString( m_iStringId );
	const TWCHAR* pSeparator = TStringManager::String16FindChar( wszText, L'*' );
	m_bPaged                 = pSeparator != TNULL;

	if ( pSeparator != TNULL )
	{
		m_iNumPages = 1;

		for ( pSeparator = TStringManager::String16FindChar( wszText, L'*' ); pSeparator != TNULL; pSeparator = TStringManager::String16FindChar( pSeparator + 1, L'*' ) )
		{
			m_aiPageOffsets[ m_iNumPages ] = TINT16( pSeparator - wszText ) + 1;
			m_iNumPages++;
		}

		TASSERT( m_iNumPages < MAXPAGES, "Too many pages in help text! maximum %d pages allowed\n" );

		SplitString( wszText, L"*", m_apPages );
		m_iCurrentPage = 0;
		m_pText->SetText( m_apPages[ 0 ], -1 );
	}
	else
	{
		m_iCurrentPage = 0;
		m_pText->SetText( wszText, -1 );
	}

	if ( a_wszText != TNULL )
	{
		m_pText->SetText2( TNULL, 0 );
		return;
	}

	ALocaleManager::StringFormat* pFormat = pLocale->GetStringFormat( m_iStringId );

	if ( pFormat == TNULL )
	{
		m_pText->SetText2( TNULL, 0 );
		return;
	}

	m_pText->SetText2( pLocale->GetOverwrittenFormatString( m_iStringId, pFormat->m_wszFormat ), pLocale->GetFormatUnk04() );
}

// $deBlob: FUNCTION 00651f10
void ABlobHelpScene::ShowCollectableCount()
{
	for ( TINT i = 0; i < iMAXFACES; i++ )
		FindChildXUIElement( i + 7 )->SetOpacity( 0.0f );

	FindChildXUIElement( 6 )->SetOpacity( 1.0f );

	TXUIControl* pCount = FindChildXUIControl( 0xB );
	pCount->SetOpacity( 1.0f );

	AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();
	TStringManager::String16Format( m_wszCount, L"%d/%d", rLevelData.GetNumHelpCollected(), rLevelData.GetNumHelpCollectables() );
	pCount->SetText( m_wszCount, -1 );
}

// $deBlob: FUNCTION 00651fc0
void ABlobHelpScene::SetSpeaker( TINT a_iSpeaker )
{
	TXUIElement* apFaces[ iMAXFACES ];

	for ( TINT i = 0; i < iMAXFACES; i++ )
	{
		apFaces[ i ] = FindChildXUIElement( i + 7 );
		apFaces[ i ]->SetOpacity( 0.0f );
	}

	TASSERT( a_iSpeaker >= 0 && a_iSpeaker < iMAXFACES );
	apFaces[ a_iSpeaker ]->SetOpacity( 1.0f );
}

// $deBlob: FUNCTION 00652060
TBOOL ABlobHelpScene::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton != XUIButton_Accept )
		return TXUIScene::HandleButton( a_eButton, a_iController );

	// The first press shows the whole text
	if ( m_pText->IsRevealing() )
	{
		m_pText->FinishReveal( TTRUE );
		return TTRUE;
	}

	if ( m_bPaged && m_iCurrentPage < m_iNumPages - 1 )
	{
		m_iCurrentPage++;
		m_pText->SetText( m_apPages[ m_iCurrentPage ], -1 );

		ALocaleManager*               pLocale = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );
		ALocaleManager::StringFormat* pFormat;

		if ( m_iStringId >= 0 && ( pFormat = pLocale->GetStringFormat( m_iStringId ) ) != TNULL && pFormat->m_wszFormat != TNULL )
		{
			const TWCHAR* wszFormat = pLocale->GetOverwrittenFormatString( m_iStringId, pFormat->m_wszFormat );

			// NOTE: retail adds the page's character offset as a byte offset
			m_pText->SetText2( TREINTERPRETCAST( const TWCHAR*, TREINTERPRETCAST( const TUINT8*, wszFormat ) + m_aiPageOffsets[ m_iCurrentPage ] ), pLocale->GetFormatUnk04() );
		}

		m_pContinue->SetText( pLocale->GetString( ( m_iCurrentPage == m_iNumPages - 1 ) ? s_iCloseString : s_iNextString ), -1 );
		m_pContinue->SetVisible( TFALSE );
		return TTRUE;
	}

	Close();
	TXUI::GetSingleton()->TransitionBack( this );
	return TTRUE;
}

// $deBlob: FUNCTION 006521b0
void ABlobHelpScene::TransBackFrom()
{
	m_pSpeech->PlayAnimation( L"HideSpeech", L"EndHideSpeech" );
	TXUIScene::TransBackFrom();
}

// $deBlob: FUNCTION 006521d0
void ABlobHelpScene::EndTransTo()
{
	m_pSpeech->PlayAnimation( L"ShowSpeech", L"EndShowSpeech" );
	TXUIScene::EndTransTo();
}

// $deBlob: FUNCTION 006521f0
void ABlobHelpScene::Close()
{
	ABlobInputMap::GetSingleton()->DisableFor( 0.5f );
	ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID_INGAME, TFALSE );

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pState )->SetPaused( TFALSE );

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	pBlob->SetUntargetableTime( TMath::Max( pBlob->GetUntargetableTime(), 1.0f ) );

	if ( m_pTrigger != TNULL )
		m_pTrigger->TriggerAllTargets( TNULL );

	ABlobInputMap::GetSingleton()->ClearButtons( 0, s_uiCloseButton );
}

// $deBlob: FUNCTION 00643fb0
void ABlobHelpScene::SplitString( const TWCHAR* a_wszText, const TWCHAR* a_wszSeparator, TWCHAR** a_ppOutStrings )
{
	// NOTE: retail's copy is in another object, it's only used here
	TINT          iNumStrings  = 0;
	TSIZE         uiSepLength  = TStringManager::String16Length( a_wszSeparator );
	const TWCHAR* wszStart     = a_wszText;
	const TWCHAR* wszSeparator = TStringManager::String16FindString( wszStart, const_cast<TWCHAR*>( a_wszSeparator ) );

	while ( wszSeparator != TNULL )
	{
		TSIZE   uiSize  = ( ( wszSeparator - wszStart ) + 1 ) * sizeof( TWCHAR );
		TWCHAR* wszPage = TSTATICCAST( TWCHAR, TMalloc( uiSize, AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) );
		TUtil::MemCopy( wszPage, wszStart, uiSize );
		wszPage[ uiSize / sizeof( TWCHAR ) - 1 ] = L'\0';
		a_ppOutStrings[ iNumStrings++ ]          = wszPage;

		wszStart     = wszSeparator + uiSepLength;
		wszSeparator = TStringManager::String16FindString( wszStart, const_cast<TWCHAR*>( a_wszSeparator ) );
	}

	TSIZE   uiSize  = ( TStringManager::String16Length( wszStart ) + 1 ) * sizeof( TWCHAR );
	TWCHAR* wszPage = TSTATICCAST( TWCHAR, TMalloc( uiSize, AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) );
	TUtil::MemCopy( wszPage, wszStart, uiSize );
	wszPage[ uiSize / sizeof( TWCHAR ) - 1 ] = L'\0';
	a_ppOutStrings[ iNumStrings ]            = wszPage;
}

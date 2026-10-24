#include "pch.h"
#include "ABlobLoadingScreen.h"
#include "XUI/ABlobCursorTrail.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/APlayerManager.h"
#include "GUI/AGUIManager.h"
#include "GameInterface/ALoadLevelState.h"
#include "Locale/ALocaleManager.h"

#include <Input/TInputInterface.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUIScene.h>
#include <XUI/TXUIVisual.h>

#include <XUI/TXUI.h>
#include <XUI/TXUIButton.h>
#include "Sound/ASoundLoadHelper.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The number of tip layers of each SPLevel's loading screen
static const TINT s_aNumLayers[ AGameStatus::SPLevel_NUMOF ] = { 5, 7, 8, 8, 9, 9, 9, 9, 11, 9 };

// The tips of the loading screen with no SPLevel
static const TINT s_aDefaultTips[ 5 ] = { 166, 170, 165, 168, 167 };

// The tips of the SPLevel Default
static const TINT s_aLevelTipsDefault[ 5 ] = { 934, 938, 933, 936, 935 };
// The tips of the SPLevel Uptown
static const TINT s_aLevelTipsUptown[ 7 ] = { 657, 658, 659, 660, 661, 662, 663 };
// The tips of the SPLevel Downtown
static const TINT s_aLevelTipsDowntown[ 8 ] = { 664, 665, 666, 667, 668, 669, 670, 671 };
// The tips of the SPLevel ChromaDam
static const TINT s_aLevelTipsChromaDam[ 8 ] = { 672, 673, 674, 675, 676, 677, 678, 679 };
// The tips of the SPLevel Guggentraz
static const TINT s_aLevelTipsGuggentraz[ 9 ] = { 680, 681, 682, 683, 684, 685, 686, 687, 688 };
// The tips of the SPLevel TheDocklands
static const TINT s_aLevelTipsTheDocklands[ 9 ] = { 689, 690, 691, 692, 693, 694, 695, 696, 697 };
// The tips of the SPLevel HangingGardens
static const TINT s_aLevelTipsHangingGardens[ 9 ] = { 698, 699, 700, 701, 702, 703, 704, 705, 706 };
// The tips of the SPLevel ChromaCity
static const TINT s_aLevelTipsChromaCity[ 9 ] = { 707, 708, 709, 710, 711, 712, 713, 714, 715 };
// The tips of the SPLevel MinistryOfInk
static const TINT s_aLevelTipsMinistryOfInk[ 11 ] = { 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726 };
// The tips of the SPLevel LakeRaydia
static const TINT s_aLevelTipsLakeRaydia[ 9 ] = { 727, 728, 729, 730, 731, 732, 733, 734, 735 };

// The tips of each SPLevel's layers
static const TINT* s_apLevelTips[ AGameStatus::SPLevel_NUMOF ] = {
	s_aLevelTipsDefault,
	s_aLevelTipsUptown,
	s_aLevelTipsDowntown,
	s_aLevelTipsChromaDam,
	s_aLevelTipsGuggentraz,
	s_aLevelTipsTheDocklands,
	s_aLevelTipsHangingGardens,
	s_aLevelTipsChromaCity,
	s_aLevelTipsMinistryOfInk,
	s_aLevelTipsLakeRaydia,
};

// Free paint: the layer order and the tips
static const TINT s_aFreePaintLayers[ 4 ] = { 2, 3, 0, 1 };
static const TINT s_aFreePaintTips[ 4 ]   = { 303, 304, 305, 306 };

// The tips of each multiplayer game type, 4 per type
static const TINT s_aMultiplayerTips[ 16 ] = {
	830, 831, 832, 833,
	838, 839, 840, 841,
	834, 835, 836, 837,
	-1, -1, -1, -1
};

// Sprints: the layer order and the tips, 4 per sprint
static const TINT s_aSprintLayers[ 80 ] = {
	2, 1, 0, 3,
	3, 2, 1, 0,
	2, 0, 1, 3,
	2, 0, 3, 1,
	2, 3, 1, 0,
	2, 1, 0, 3,
	2, 1, 0, 3,
	2, 3, 1, 0,
	2, 0, 3, 1,
	2, 1, 3, 0,
	2, 0, 3, 1,
	0, 3, 2, 1,
	2, 3, 0, 1,
	2, 1, 0, 3,
	1, 2, 3, 0,
	2, 0, 3, 1,
	2, 1, 0, 3,
	3, 0, 1, 2,
	2, 1, 0, 3,
	2, 1, 0, 3
};

static const TINT s_aSprintTips[ 80 ] = {
	1807, 1808, 1809, 1810,
	597, 598, 599, 600,
	605, 606, 607, 608,
	601, 602, 603, 604,
	609, 610, 611, 612,
	1849, 1850, 1851, 1852,
	1797, 1798, 1799, 1800,
	613, 614, 615, 616,
	617, 618, 619, 620,
	621, 622, 623, 624,
	625, 626, 627, 628,
	629, 630, 631, 632,
	633, 634, 635, 636,
	637, 638, 639, 640,
	641, 642, 643, 644,
	645, 646, 647, 648,
	1818, 1819, 1820, 1821,
	649, 650, 651, 652,
	653, 654, 655, 656,
	1783, 1784, 1785, 1786
};

// $deBlob: FUNCTION 00646d30
// $deBlob: FUNCTION 00646d40
// $deBlob: FUNCTION 00646dc0
// $deBlob: FUNCTION 00646e40
// $deBlob: FUNCTION 00646e50
// $deBlob: FUNCTION 0053dd50
// $deBlob: FUNCTION 00858ad0
TDEFINE_CLASS( ABlobLoadingScreen );

// $deBlob: FUNCTION 00648340
ABlobLoadingScreen::ABlobLoadingScreen()
{
	// NOTE: retail leaves the members to OnCreated
}

// $deBlob: FUNCTION 00648460
// $deBlob: FUNCTION 00646e60
// $deBlob: FUNCTION 006483ed
ABlobLoadingScreen::~ABlobLoadingScreen()
{
	TFree( m_apLayerButtons );
	TFree( m_apSpeechBubbles );
	TFree( m_apLayers );
}

// $deBlob: FUNCTION 00646ea0
void ABlobLoadingScreen::OnCreated()
{
	m_iNumLayers     = -1;
	m_iCurLayer      = 0;
	m_iLayerOrder    = 0;
	m_fTipAnimTime   = 0.75f;
	m_bUnk18C        = TFALSE;
	m_uiUnk164       = 0;
	m_bContinueShown = TFALSE;

	m_pContinue = FindChildXUIElement( L"Continue" );
	m_pContinue->SetVisible( TFALSE );
	m_pPointRemote      = FindChildXUIElement( L"PointRemote" );
	m_pLoadingIndicator = FindChildXUIElement( L"LoadingIndicator" );
	m_pLevelName        = FindChildXUIControl( L"LevelName" );

	AGameStatus*                        pGameStatus = AGameStatus::GetSingleton();
	ALevelInformation::LevelProperties* pProperties = pGameStatus->GetLevelProperties();
	TINT                                iSPLevel    = pGameStatus->GetSPLevel();
	TINT                                eLevelMode  = pGameStatus->GetLevelMode();

	m_iNumLayers = 5;

	if ( pProperties->m_sprintLevel == 0 && !pGameStatus->IsMultiplayer() && iSPLevel != -1 )
	{
		if ( eLevelMode == AGameStatus::Mode_FreePaint )
			m_iNumLayers = 4;
		else
			m_iNumLayers = s_aNumLayers[ iSPLevel ];
	}
	else if ( pGameStatus->IsMultiplayer() || pProperties->m_sprintLevel != 0 )
	{
		m_iNumLayers = 4;
	}

	m_apLayerButtons  = TSTATICCAST( TXUIControl*, TMalloc( m_iNumLayers * sizeof( TXUIControl* ), TXUI::ms_pXUIMemoryBlock ) );
	m_apSpeechBubbles = TSTATICCAST( TXUIControl*, TMalloc( m_iNumLayers * sizeof( TXUIControl* ), TXUI::ms_pXUIMemoryBlock ) );
	m_apLayers        = TSTATICCAST( TXUIControl*, TMalloc( m_iNumLayers * sizeof( TXUIControl* ), TXUI::ms_pXUIMemoryBlock ) );

	TWCHAR* wszBuffer            = TStringManager::GetTempString16();
	TINT    iMultiPlayerGameType = pGameStatus->GetMultiPlayerGameType();
	TINT    iSprint              = pGameStatus->GetSelectedSlot();
	TBOOL   bSprint              = pProperties->m_sprintLevel != 0;

	if ( bSprint || eLevelMode == AGameStatus::Mode_FreePaint )
	{
		TASSERT( m_iNumLayers == 4 );

		m_apLayerButtons[ 0 ] = FindChildXUIControl( L"Button_Arty" );
		m_apLayerButtons[ 1 ] = FindChildXUIControl( L"Button_Zip" );
		m_apLayerButtons[ 2 ] = FindChildXUIControl( L"Button_Prof" );
		m_apLayerButtons[ 3 ] = FindChildXUIControl( L"Button_Bif" );
		m_apPulses[ 0 ]       = FindChildXUIElement( L"pulse_arty" );
		m_apPulses[ 1 ]       = FindChildXUIElement( L"pulse_zip" );
		m_apPulses[ 2 ]       = FindChildXUIElement( L"pulse_prof" );
		m_apPulses[ 3 ]       = FindChildXUIElement( L"pulse_bif" );

		for ( TINT i = 0; i < m_iNumLayers; i++ )
		{
			m_apLayerButtons[ i ]->SetEnabled( TFALSE );
			m_apLayerButtons[ i ]->SetOpacity( 0.0f );
			m_apPulses[ i ]->SetVisible( TFALSE );
		}
	}

	for ( TINT i = 0; i < m_iNumLayers; i++ )
	{
		if ( bSprint )
		{
			TINT iLayer = s_aSprintLayers[ iSprint * 4 + i ];
			TStringManager::String16Format( wszBuffer, L"Layer%d", iLayer + 1 );
			m_apLayers[ iLayer ]        = FindChildXUIControl( wszBuffer );
			m_apSpeechBubbles[ iLayer ] = m_apLayers[ iLayer ]->FindChildXUIControl( L"SpeechBubble" );
		}
		else if ( eLevelMode == AGameStatus::Mode_FreePaint )
		{
			TINT iLayer = s_aFreePaintLayers[ i ];
			TStringManager::String16Format( wszBuffer, L"Layer%d", iLayer + 1 );
			m_apLayers[ iLayer ]        = FindChildXUIControl( wszBuffer );
			m_apSpeechBubbles[ iLayer ] = m_apLayers[ iLayer ]->FindChildXUIControl( L"SpeechBubble" );
		}
		else
		{
			TStringManager::String16Format( wszBuffer, L"Layer%d", i + 1 );
			m_apLayers[ i ]        = FindChildXUIControl( wszBuffer );
			m_apLayerButtons[ i ]  = m_apLayers[ i ]->FindChildXUIControl( L"Button" );
			m_apSpeechBubbles[ i ] = m_apLayers[ i ]->FindChildXUIControl( L"SpeechBubble" );
		}

		if ( bSprint )
			SetLayerTip( s_aSprintLayers[ iSprint * 4 + i ], s_aSprintTips[ iSprint * 4 + i ] );
		else if ( pGameStatus->IsMultiplayer() )
			SetLayerTip( i, s_aMultiplayerTips[ iMultiPlayerGameType * 4 + i ] );
		else if ( eLevelMode == -1 || eLevelMode == AGameStatus::Mode_Story )
			SetLayerTip( i, ( iSPLevel == -1 ) ? s_aDefaultTips[ i ] : s_apLevelTips[ iSPLevel ][ i ] );
		else if ( eLevelMode == AGameStatus::Mode_FreePaint )
			SetLayerTip( s_aFreePaintLayers[ i ], s_aFreePaintTips[ i ] );
	}

	if ( bSprint || eLevelMode == AGameStatus::Mode_FreePaint )
	{
		TINT iNextLayer;

		if ( bSprint && eLevelMode != AGameStatus::Mode_FreePaint )
		{
			TASSERT( iSprint >= 0 );
			TINT iIndex = m_iLayerOrder + iSprint * 4;
			m_iCurLayer = s_aSprintLayers[ iIndex ];
			iNextLayer  = s_aSprintLayers[ iIndex + 1 ];
		}
		else
		{
			// NOTE: retail reads s_aFreePaintLayers[ m_iLayerOrder + 1 ] (the table is followed by the tips)
			m_iCurLayer = s_aFreePaintLayers[ m_iLayerOrder ];
			iNextLayer  = s_aFreePaintLayers[ m_iLayerOrder + 1 ];
		}

		TASSERT( iNextLayer >= 0 && iNextLayer < m_iNumLayers );
		m_apPulses[ iNextLayer ]->SetVisible( TTRUE );
		m_apLayerButtons[ iNextLayer ]->SetEnabled( TTRUE );
		m_apLayerButtons[ iNextLayer ]->SetOpacity( 1.0f );
		AddChildTail( m_apLayers[ iNextLayer ] );
		AddChildTail( m_apPulses[ iNextLayer ] );
		AddChildTail( m_apLayerButtons[ iNextLayer ] );
	}

	TSTATICCAST( TXUIScene, m_apLayers[ m_iCurLayer ] )->GetTimelineControl().PlayAnimation( L"Intro", L"EndIntro" );
	m_apSpeechBubbles[ m_iCurLayer ]->GetVisual()->PlayAnimation( L"Show", L"EndShow" );
	m_apLayerButtons[ m_iCurLayer ]->GetVisual()->PlayAnimation( L"Focus", L"EndFocus" );

	AGameState* pGameState = AGameStateController::GetSingleton()->GetCurrentGameState();

	if ( pGameState->IsA( &TGetClass( ALoadLevelState ) ) )
		m_pLoadLevelState = pGameState;

	for ( TINT i = 0; i < 4; i++ )
	{
		if ( i == 0 )
			TStringManager::String16Format( wszBuffer, L"CursorTrail" );
		else
			TStringManager::String16Format( wszBuffer, L"CursorTrail%d", i + 1 );

		m_apCursorTrails[ i ] = TSTATICCAST( ABlobCursorTrail, FindChildXUIControl( wszBuffer ) );
		m_apCursorTrails[ i ]->SetIndex( i );
		m_apCursorTrails[ i ]->CreateEffects();
	}

	m_uiUnk184 = 0;

	TINT eLanguage = T2Locale::GetSingleton()->GetLanguage();

	if ( eLanguage == ALocaleManager::ALANG_JAPANESE || eLanguage == ALocaleManager::ALANG_KOREAN || eLanguage == ALocaleManager::ALANG_CHINESE )
		m_pLoadingIndicator->SetScale( 230.0f / 256.0f, 230.0f / 256.0f );

	HideButtonPrompts( this );

	SetScale( 1.5f, 1.5f );

	TFLOAT fWidth  = 0.0f;
	TFLOAT fHeight = 0.0f;
	GetDimensions( fWidth, fHeight );
	SetPosition( GetPositionX() - fWidth * 0.25f, GetPositionY() - fHeight * 0.25f );
}

// $deBlob: FUNCTION 00647d90
void ABlobLoadingScreen::SetLayerTip( TINT a_iLayer, TINT a_iStringId )
{

	m_apSpeechBubbles[ a_iLayer ]->SetText( T2Locale::GetSingleton()->GetString( a_iStringId ), -1 );

	ALocaleManager*               pLocale = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );
	ALocaleManager::StringFormat* pFormat = pLocale->GetStringFormat( a_iStringId );

	if ( pFormat != TNULL )
	{
		m_apSpeechBubbles[ a_iLayer ]->SetText2( pLocale->GetOverwrittenFormatString( a_iStringId, pFormat->m_wszFormat ), pLocale->GetFormatUnk04() );
		return;
	}

	m_apSpeechBubbles[ a_iLayer ]->SetText2( TNULL, 0 );
}

// The button glyphs of each gamepad type (TInputDevice::GetLastGamepadType) shown by "XuiLabel_Interact"
static const TWCHAR s_awszInteractGlyphs[][ 4 ] = {
	{ 0x0087, L' ', 0, 0 },
	{ 0x03F8, L' ', 0, 0 },
	{ 0x0408, L' ', 0, 0 },
	{ 0x044C, L' ', 0, 0 },
};

// Hides the button prompt images of every platform (and, in single player, the interact label); in multiplayer the
// interact label shows the glyph of the last used gamepad
// $deBlob: FUNCTION 00647e40
void ABlobLoadingScreen::HideButtonPrompts( TXUIScene* a_pScene )
{
	const TWCHAR* awszElements[] = {
		L"XuiImage1_wii",
		L"XuiImage1_360",
		L"XuiImage1_key",
		L"XuiImage1_ps4",
		L"XuiFigure_wii",
		L"XuiFigure_360",
		L"XuiFigure_key",
		L"XuiFigure_ps4",
		L"XuiLabel_Interact",
	};

	TBOOL bMultiplayer = APlayerManager::GetSingleton()->GetNumPlayersInline() > 1;

	ForEachElement( a_pScene, TTRUE, [ &bMultiplayer, &awszElements ]( TXUIElement* a_pElement ) -> TBOOL {
		// $deBlob: FUNCTION 00647f40
		if ( a_pElement->GetID() == TNULL )
			return TTRUE;

		TINT iNumElements = bMultiplayer ? 8 : 9;

		for ( TINT i = 0; i < iNumElements; i++ )
		{
			if ( TStringManager::String16Compare( a_pElement->GetID(), awszElements[ i ], -1 ) == 0 )
			{
				a_pElement->SetVisible( TFALSE );
				a_pElement->SetScale( 0.0f, 0.0f );
			}
		}

		if ( bMultiplayer && TStringManager::String16Compare( a_pElement->GetID(), awszElements[ 8 ], -1 ) == 0 && a_pElement->IsA( &TGetClass( TXUIControl ) ) )
			TSTATICCAST( TXUIControl, a_pElement )->SetText( s_awszInteractGlyphs[ TInputDevice::GetLastGamepadType() ], -1 );

		return TTRUE;
	} );
}

// $deBlob: FUNCTION 006477e0
void ABlobLoadingScreen::SetLevelName( const TWCHAR* a_wszName )
{
	m_pLevelName->SetText( a_wszName, -1 );
}

// Shows "Continue" once the level is loaded
// $deBlob: FUNCTION 00647630
void ABlobLoadingScreen::TransTo()
{
	TINT iNumPlayers = APlayerManager::GetSingleton()->GetNumPlayersInline();

	if ( iNumPlayers < 1 )
		iNumPlayers = 1;

	AGUIManager::GetSingleton()->SetCursorActive( TTRUE );
	AGUIManager::GetSingleton()->SetCursorVisibleCount( iNumPlayers );
	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 00647670
void ABlobLoadingScreen::Tick( TFLOAT a_fDeltaTime )
{
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetTransToScene() == TNULL && pXUI->GetTransBackFromScene() == TNULL && !pXUI->IsInputLocked() )
	{
		if ( !m_bContinueShown && TSTATICCAST( ALoadLevelState, m_pLoadLevelState )->GetLoadState() == ALoadLevelState::STATE_WaitForOK )
		{
			m_pContinue->SetVisible( TTRUE );
			m_pPointRemote->SetVisible( TFALSE );
			m_pLoadingIndicator->SetVisible( TFALSE );
			m_bContinueShown = TTRUE;
		}

		if ( m_fTipAnimTime >= 0.0f )
			m_fTipAnimTime -= a_fDeltaTime;

		TXUIScene::Tick( a_fDeltaTime );
		HideButtonPrompts( this );
		return;
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00647730
TBOOL ABlobLoadingScreen::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( m_bContinueShown && a_eButton == XUIButton_Accept )
	{
		AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
		TSTATICCAST( ALoadLevelState, m_pLoadLevelState )->Continue();
		return TTRUE;
	}

	OnNotifyPress( m_apLayerButtons[ m_iCurLayer ], a_iController );
	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// Hides the speech bubbles and the buttons of every layer
static void HideLayers( TXUIControl** a_apSpeechBubbles, TXUIControl** a_apLayerButtons, TINT a_iNumLayers )
{
	for ( TINT i = 0; i < a_iNumLayers; i++ )
	{
		a_apSpeechBubbles[ i ]->GetVisual()->SetVisible( TFALSE );
		a_apLayerButtons[ i ]->GetVisual()->SetVisible( TFALSE );
	}
}

// $deBlob: FUNCTION 00647800
TBOOL ABlobLoadingScreen::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	// Still animating the last tip
	if ( m_fTipAnimTime > 0.0f )
		return TTRUE;

	TINT iLayer = 0;

	if ( m_iNumLayers <= 0 )
		return TFALSE;

	while ( m_apLayerButtons[ iLayer ] != a_pSource )
	{
		if ( ++iLayer >= m_iNumLayers )
			return TFALSE;
	}

	// The pressed layer's tip is done
	TSTATICCAST( TXUIScene, m_apLayers[ iLayer ] )->GetTimelineControl().StopAnimation();
	m_apLayerButtons[ iLayer ]->SetFocus( TFALSE, TFALSE, TTRUE );
	m_apLayerButtons[ iLayer ]->SetEnabled( TFALSE );

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	TINT         eLevelMode  = pGameStatus->GetLevelMode();
	TINT         iSprint     = pGameStatus->GetSelectedSlot();
	TXUIElement* pShow;

	if ( pGameStatus->GetLevelProperties()->m_sprintLevel != 0 || eLevelMode == AGameStatus::Mode_FreePaint )
	{
		// Sprints and free paint: the layers come in the order of the tables
		TBOOL bLast = ++m_iLayerOrder >= 4;

		if ( m_iLayerOrder >= 4 )
		{
			m_bUnk18C     = TTRUE;
			m_iLayerOrder = 0;
		}

		if ( eLevelMode == AGameStatus::Mode_FreePaint )
		{
			m_iCurLayer = s_aFreePaintLayers[ m_iLayerOrder ];
		}
		else
		{
			TASSERT( iSprint != -1 );
			m_iCurLayer = s_aSprintLayers[ iSprint * 4 + m_iLayerOrder ];
		}

		if ( bLast || m_bUnk18C )
		{
			HideLayers( m_apSpeechBubbles, m_apLayerButtons, m_iNumLayers );
			goto show_current;
		}

		AddChildTail( m_apLayers[ m_iCurLayer ] );

		if ( m_iLayerOrder >= m_iNumLayers - 1 )
			goto play;

		// The next layer's button appears
		TINT iNextLayer;

		if ( eLevelMode == AGameStatus::Mode_FreePaint )
		{
			iNextLayer = s_aFreePaintLayers[ m_iLayerOrder + 1 ];
		}
		else
		{
			TASSERT( iSprint != -1 );
			iNextLayer = s_aSprintLayers[ iSprint * 4 + m_iLayerOrder + 1 ];
		}

		AddChildTail( m_apLayers[ iNextLayer ] );
		AddChildTail( m_apPulses[ iNextLayer ] );
		AddChildTail( m_apLayerButtons[ iNextLayer ] );
		m_apLayerButtons[ iNextLayer ]->SetEnabled( TTRUE );
		m_apLayerButtons[ iNextLayer ]->SetOpacity( 1.0f );

		pShow = m_apPulses[ iNextLayer ];
		goto show;
	}
	else
	{
		// The next layer, back to the first after the last
		if ( iLayer >= m_iNumLayers - 1 )
		{
			m_bUnk18C   = TTRUE;
			m_iCurLayer = 0;
			HideLayers( m_apSpeechBubbles, m_apLayerButtons, m_iNumLayers );
			goto show_current;
		}

		m_iCurLayer = iLayer + 1;

		if ( m_bUnk18C )
		{
			HideLayers( m_apSpeechBubbles, m_apLayerButtons, m_iNumLayers );
			goto show_current;
		}

		AddChildTail( m_apLayers[ iLayer + 1 ] );
		goto play;
	}

show_current:
	m_apSpeechBubbles[ m_iCurLayer ]->GetVisual()->SetVisible( TTRUE );
	pShow = m_apLayerButtons[ m_iCurLayer ]->GetVisual();

show:
	pShow->SetVisible( TTRUE );

play:
	if ( !m_bUnk18C )
		TSTATICCAST( TXUIScene, m_apLayers[ m_iCurLayer ] )->GetTimelineControl().PlayAnimation( L"Intro", L"EndIntro" );

	m_apSpeechBubbles[ m_iCurLayer ]->GetVisual()->PlayAnimation( L"Show", L"EndShow" );
	m_apLayerButtons[ m_iCurLayer ]->GetVisual()->PlayAnimation( L"Focus", L"EndFocus" );

	m_fTipAnimTime = m_bUnk18C ? 0.5f : 0.75f;
	return TTRUE;
}

// $deBlob: FUNCTION 00647c80
TBOOL ABlobLoadingScreen::OnNotifySetFocus( TXUIElement* a_pSource )
{
	if ( !a_pSource->IsA( &TGetClass( TXUIButton ) ) )
		return TFALSE;

	if ( ASoundLoadHelper::GetSingleton() != TNULL )
		ASoundLoadHelper::GetSingleton()->PlayLoadingSound();

	AddChildTail( a_pSource );
	return TTRUE;
}

// $deBlob: FUNCTION 00647cd0
TBOOL ABlobLoadingScreen::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	if ( a_eMessage != XUIMessage_NamedFrame || pGameStatus->GetSelectedSlot() == -1 )
		return TXUIControl::SendMessage( a_eMessage, a_wszParam );

	TASSERT( AGameStatus::GetSingleton()->GetLevelData().m_pLevelProps->m_sprintLevel || AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint );

	if ( TStringManager::String16Compare( L"EndIntro", a_wszParam ) != 0 )
		return TXUIControl::SendMessage( a_eMessage, a_wszParam );

	// The intro of the challenge's tip is over: its button can be pressed
	m_apLayerButtons[ m_iCurLayer ]->SetEnabled( TTRUE );
	return TTRUE;
}

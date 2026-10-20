#include "pch.h"
#include "ABlobHud.h"
#include "Challenges/AChallengeManager.h"
#include "ABlobGoalHud.h"
#include "ABlobHelpHud.h"
#include "ABlobScoreFloatHud.h"
#include "ABlobLifeHud.h"
#include "ABlobMedalsHud.h"
#include "ABlobScoreHud.h"
#include "ABlobShakeHud.h"
#include "ABlobSizeHud.h"
#include "ABlobPlacingHud.h"
#include "ABlobMultiplayerHud.h"
#include "Input/ABlobInputMap.h"
#include "Sound/AMusicManager.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "Entities/APropTriggerHelp.h"
#include "Locale/ALocaleManager.h"
#include "ARandom.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIGroup.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ABlobHud* ABlobHud::sm_pSingleton = TNULL;

// The colour of the reticle
// $deBlob: FUNCTION 0053e1d0
static TUINT32 s_uiReticleColour = TXUIElement::MyMakeARGB( 1.0f, 1.0f, 1.0f, 1.0f );

// The time the medal banner stays before it fades out
static TCONSTEXPR_VAR TFLOAT s_fMedalBannerTime = 5.0f;

// $deBlob: FUNCTION 00652320
// $deBlob: FUNCTION 00652330
// $deBlob: FUNCTION 006523a0
// $deBlob: FUNCTION 00652410
// $deBlob: FUNCTION 00652420
// $deBlob: FUNCTION 0053e190
// $deBlob: FUNCTION 00858be0
TDEFINE_CLASS( ABlobHud );

// $deBlob: FUNCTION 00653a70
ABlobHud::ABlobHud()
    : m_pLevelTimer( TNULL )
    , m_bUnk134( TFALSE )
    , m_pChallengeTimer( TNULL )
    , m_pChallengeHudElement( TNULL )
    , m_uiUnk240( 0 )
    , m_pTargetReticle( TNULL )
    , m_pMaskParent( TNULL )
    , m_pGoalHud( TNULL )
    , m_uiPendingShowcases( 0 )
    , m_uiShownShowcases( 0 )
    , m_pUnk25C( TNULL )
    , m_pUnk260( TNULL )
    , m_pSplats( TNULL )
    , m_pHoldTimeline( TNULL )
    , m_fStopTime( 0.0f )
    , m_uiPendingAchievements( 0 )
    , m_pGp( TNULL )
    , m_pAchievementName( TNULL )
    , m_pShakeHud( TNULL )
    , m_pMedalsBanner( TNULL )
    , m_iBannerQueueSize( 0 )
    , m_bBannerShowing( TFALSE )
    , m_fMedalBannerTime( 0.0f )
    , m_pLifeHud( TNULL )
    , m_pMessageHud( TNULL )
{
	// NOTE: retail leaves m_pMain, m_pAchievementBanner, m_apSplats, m_bTargeted, m_fTargetedTime, m_apDrops and m_pCircle to OnCreated
	m_pMain              = TNULL;
	m_pAchievementBanner = TNULL;

	TASSERT( sm_pSingleton == TNULL );
	sm_pSingleton = this;

	for ( TINT i = 0; i < MAXBANNERS; i++ )
		m_aBannerQueue[ i ] = TNULL;
}

// $deBlob: FUNCTION 00652430
// $deBlob: FUNCTION 00653c00
// $deBlob: FUNCTION 00653bea
ABlobHud::~ABlobHud()
{
	AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();
	rLevelData.m_pGoalHud              = TNULL;
	rLevelData.m_pScoreHud             = TNULL;
	rLevelData.SetMessageControl( TNULL );

	AChallengeManager::GetSingleton()->SetHudElement( TNULL );
	APropTriggerHelp::SetHelpHud( TNULL );

	sm_pSingleton = TNULL;
}

// $deBlob: FUNCTION 006524d0
void ABlobHud::OnCreated()
{
	m_bUnk134 = TFALSE;
	AGameStateController::GetSingleton()->SetRenderFadesFirst( TFALSE );
	m_uiSceneFlags &= ~SceneFlags_AcceptsInput;

	void*        pMessageOwner = TNULL;
	TXUIElement* pMessage      = GetMessageElement( &pMessageOwner );

	if ( pMessageOwner != TNULL && pMessage != TNULL )
		pMessage->SetVisible( TFALSE );

	m_bTargeted             = TFALSE;
	m_fTargetedTime         = 0.0f;
	m_uiPendingAchievements = 0;

	if ( APlayerManager::GetSingleton()->GetNumPlayers() != 1 )
		return;

	m_pShakeHud          = FindChildXUIControl( 0xD9 );
	m_pHoldTimeline      = FindChildXUIElement( 0x154 );
	m_fStopTime          = TSTATICCAST( TXUIVisual, m_pHoldTimeline )->GetTimelineControl()->GetNamedFrameTime( L"Stop" );
	m_pUnk25C            = FindChildXUIControl( 0xFD );
	m_pUnk260            = FindChildXUIControl( 0xE4 );
	m_pAchievementBanner = FindChildXUIElement( 0x122 );

	m_pGp              = FindChildXUIControl( 0x124 )->GetVisual()->FindChildXUIElement( L"gp" );
	m_pAchievementName = m_pAchievementBanner->FindChildXUIElement( L"AchievementName" );

	m_pSplats = FindChildXUIControl( 0xCE );
	m_pSplats->SetVisible( TFALSE );
	TSTATICCAST( TXUIScene, m_pSplats )->GetTimelineControl().ResetAnimation();

	TWCHAR* wszName = TStringManager::GetTempString16();

	for ( TINT i = 0; i < 10; )
	{
		i++;
		TStringManager::String16Format( wszName, L"Splat%d", i );
		m_apSplats[ i - 1 ] = m_pSplats->FindChildXUIControl( wszName );
	}

	m_pTargetReticle = FindChildXUIControl( 0xE1 );
	m_apDrops[ 0 ]   = m_pTargetReticle->GetVisual()->FindChildXUIElement( L"drop1" );
	m_apDrops[ 1 ]   = m_pTargetReticle->GetVisual()->FindChildXUIElement( L"drop2" );
	m_apDrops[ 2 ]   = m_pTargetReticle->GetVisual()->FindChildXUIElement( L"drop3" );
	m_apDrops[ 3 ]   = m_pTargetReticle->GetVisual()->FindChildXUIElement( L"drop4" );
	m_pCircle        = m_pTargetReticle->GetVisual()->FindChildXUIElement( L"Circle" );

	m_pLevelTimer          = FindChildXUIControl( 5 );
	m_pChallengeTimer      = FindChildXUIControl( 0x9E );
	m_pChallengeHudElement = FindChildXUIControl( 0x45 );

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	TINT         eLevelMode  = pGameStatus->GetLevelMode();
	m_pMain                  = FindChildXUIElement( 1 );

	if ( eLevelMode == AGameStatus::Mode_FreePaint )
	{
		m_pMain->PlayAnimation( L"FreePaintHud", L"EndFreePaintHud" );
	}
	else
	{
		if ( pGameStatus->GetLevelProperties()->m_sprintLevel != 0 )
			m_pLevelTimer->SetOpacity( 0.0f );

		AChallengeManager::GetSingleton()->SetHudElement( m_pChallengeHudElement );

		if ( pGameStatus->GetLevelProperties()->m_sprintLevel != 0 )
			m_pMain->PlayAnimation( L"EndNormalToChallenge", L"EndNormalToChallenge" );
	}

	APropTriggerHelp::SetHelpHud( TSTATICCAST( ABlobHelpHud, FindChildXUIControl( 0x153 ) ) );

	m_pMessageHud = FindChildXUIControl( 0xCB );
	pGameStatus->GetLevelData().SetMessageControl( m_pMessageHud );

	TXUIElement* pScoreHud                  = FindChildXUIElement( 0x9F );
	pGameStatus->GetLevelData().m_pScoreHud = TSTATICCAST( ABlobScoreHud, pScoreHud );

	ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetPlayerBlob( 0 );

	m_pGoalHud = FindChildXUIElement( 0x19 );
	TStringManager::String16Format( m_wszGoalCount, 0x80, L"%d", *TREINTERPRETCAST( TINT*, TREINTERPRETCAST( TUINT8*, pGameStatus->GetLevelProperties() ) + 0x38 ) );
	m_pGoalHud->SetPresenterText( m_wszGoalCount, 0, -1 );
	pGameStatus->GetLevelData().m_pGoalHud = m_pGoalHud;

	// The size HUD (ABlobSizeHud) and the size of the message owner's blob
	TINT iSize = -1;

	// Blob's Z-jump lock element (control 0xE0) and the size HUD's paint points
	if ( pBlob != TNULL )
	{
		ABlobEntity* pPlayerBlob = TSTATICCAST( ABlobEntity, pBlob );
		pPlayerBlob->GetZJumpComponent()->SetLockElement( FindChildXUIControl( 0xE0 ) );
		iSize = pPlayerBlob->GetPaintPoints();
	}

	ABlobSizeHud* pSizeHud = TSTATICCAST( ABlobSizeHud, FindChildXUIElement( 0xAA ) );
	pSizeHud->SetPlayer( 0 );

	if ( iSize >= 0 )
		pSizeHud->SetSize( iSize );

	pSizeHud->SetColoured( 0, TNULL );
	pSizeHud->SetColoured( 1, FindChildXUIElement( 0x18 ) );
	pSizeHud->SetColoured( 2, TNULL );
	pSizeHud->SetColoured( 3, TNULL );

	m_pLifeHud = FindChildXUIElement( 0xCC );

	TSTATICCAST( ABlobLifeHud, m_pLifeHud )->SetPlayer( 0 );

	for ( TINT i = 0; i < 4; i++ )
	{
		TXUIControl* pControl = FindChildXUIControl( i + 0xDC );
		TASSERT( pControl && pControl->IsA( &TGetClass( ABlobScoreFloatHud ) ), "There are not 4 scorefloathud elements in the HUD scene" );

		if ( pControl != TNULL && pControl->IsA( &TGetClass( ABlobScoreFloatHud ) ) )
		{
			ABlobScoreFloatHud* pFloatHud = TSTATICCAST( ABlobScoreFloatHud, pControl );
			pFloatHud->SetPlayer( 0 );
			pGameStatus->GetLevelData().SetScoreFloatHud( i, pFloatHud );
		}
	}

	GetTimelineControl().PlayAnimation( L"169", L"End169" );
	m_pMedalsBanner = FindChildXUIElement( 0x142 );
	TSTATICCAST( ABlobMedalsHud, m_pMedalsBanner )->SetMedal( FindChildXUIElement( 0x143 ) );

	if ( pGameStatus->GetSPLevel() == 0 && pGameStatus->GetLevelMode() != AGameStatus::Mode_FreePaint )
		m_pMain->PlayAnimation( L"StartSPLevel", L"EndStartSPLevel" );

	m_pMaskParent = FindChildXUIControl( 2 );
	m_pMaskParent->FindChildXUIElement( L"Mask" )->SetColour( 0xFF000000 );
}

// $deBlob: FUNCTION 00652a00
void ABlobHud::Tick( TFLOAT a_fDeltaTime )
{
	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
	{
		GetTimelineControl().PlayAnimation( L"169", L"End169" );

		// The targeting reticle follows the blob while it has a target
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

		if ( pBlob->GetNumTurretsTargettingMe() != 0 )
		{
			if ( !m_bTargeted )
			{
				m_bTargeted = TTRUE;
				m_pTargetReticle->GetVisual()->PlayAnimation( L"Targeted", L"EndTargeted" );

				for ( TINT i = 0; i < 4; i++ )
					m_apDrops[ i ]->SetColour( s_uiReticleColour );

				m_pCircle->SetColour( s_uiReticleColour );
			}
		}
		else if ( m_bTargeted )
		{
			m_bTargeted = TFALSE;
			m_pTargetReticle->GetVisual()->PlayAnimation( L"StopTargeted", L"EndStopTargeted" );
			m_fTargetedTime = 1.0f;

			for ( TINT i = 0; i < 4; i++ )
				m_apDrops[ i ]->SetColour( s_uiReticleColour );

			m_pCircle->SetColour( s_uiReticleColour );
		}

		if ( m_fTargetedTime > 0.0f )
			m_fTargetedTime -= a_fDeltaTime;

		if ( m_bTargeted || m_fTargetedTime > 0.0f )
		{
			ACamera*   pCamera   = APlayerManager::GetSingleton()->GetBlobCamera( pBlob );
			TViewport* pViewport = ARenderer::GetSingleton()->GetViewportManager()->GetCameraTViewport( pCamera );

			if ( pViewport != TNULL )
			{
				TVector4 vPosition = pBlob->GetBasePosition();
				ARenderer::GetSingleton()->ProjectToCanvas( vPosition, pViewport );

				TXUIControl* pReticle = m_pTargetReticle;
				TFLOAT       fX       = vPosition.x - pReticle->GetWidth() * 0.5f;
				TFLOAT       fY       = vPosition.y - m_pTargetReticle->GetHeight() * 0.5f;
				pReticle->SetPosition( fX, fY );
			}
		}


		if ( TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() &&
		     TXUI::GetSingleton()->GetActiveScene()->IsA( &TGetClass( ABlobHud ) ) )
		{

			if ( m_uiPendingShowcases != 0 && AGameStatus::GetSingleton()->GetSPLevel() == 0 && AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
			{
				ShowShowcase( m_uiPendingShowcases );
				m_uiPendingShowcases = 0;
			}

			UpdateBanners( a_fDeltaTime );
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00652c90
void ABlobHud::ShowMessage( const TWCHAR* a_wszMessage )
{
	void*        pMessageOwner = TNULL;
	TXUIElement* pMessage      = GetMessageElement( &pMessageOwner );

	if ( pMessageOwner == TNULL || pMessage == TNULL )
		return;

	if ( a_wszMessage != TNULL )
	{
		pMessage->SetVisible( TTRUE );
		pMessage->PlayAnimation( L"msgstart", L"msgend" );

		// The owner (a control of the multiplayer HUDs) shows the text
		TSTATICCAST( TXUIControl, pMessageOwner )->SetTextPointer( a_wszMessage );
		return;
	}

	pMessage->SetVisible( TFALSE );
}

// $deBlob: FUNCTION 00652d00
void ABlobHud::Reset()
{
	void* pMessageOwner = TNULL;

	m_bUnk134 = TFALSE;
	AGameStateController::GetSingleton()->SetRenderFadesFirst( TFALSE );
	m_uiPendingAchievements = 0;

	TXUIElement* pMessage = GetMessageElement( &pMessageOwner );

	if ( pMessageOwner != TNULL && pMessage != TNULL )
		pMessage->SetVisible( TFALSE );

	m_uiShownShowcases = 0;


	if ( AGameStatus::GetSingleton()->GetSPLevel() == 0 && m_pMain != TNULL && AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
		m_pMain->PlayAnimation( L"StartSPLevel", L"EndStartSPLevel" );

	if ( m_pMaskParent != TNULL )
		m_pMaskParent->PlayAnimation( L"Normal", L"EndNormal" );

	if ( m_pGoalHud != TNULL )
		TSTATICCAST( ABlobGoalHud, m_pGoalHud )->Reset();

	if ( m_pSplats != TNULL )
	{
		TSTATICCAST( TXUIScene, m_pSplats )->GetTimelineControl().ResetAnimation();
		m_pSplats->SetVisible( TFALSE );
	}

	if ( m_pHoldTimeline != TNULL )
		m_pHoldTimeline->SetVisible( TFALSE );

	if ( !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer )
	{
		FindChildXUIControl( 0xCD )->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );
		TSTATICCAST( TXUIScene, FindChildXUIControl( 0x153 ) )->GetTimelineControl().PlayAnimation( L"Normal", L"EndNormal" );
		TSTATICCAST( TXUIScene, FindChildXUIControl( 0x142 ) )->GetTimelineControl().PlayAnimation( L"Normal", L"EndNormal" );
		TSTATICCAST( TXUIScene, m_pUnk25C )->GetTimelineControl().PlayAnimation( L"Normal", L"EndNormal" );
		TSTATICCAST( TXUIScene, m_pUnk260 )->GetTimelineControl().PlayAnimation( L"Normal", L"EndNormal" );
		TSTATICCAST( TXUIControl, m_pLifeHud )->GetVisual()->GetTimelineControl()->PlayAnimation( L"Normal", L"EndNormal" );
		TSTATICCAST( TXUIGroup, m_pGp )->GetTimelineControl()->PlayAnimation( L"Normal", L"EndNormal" );
		TSTATICCAST( TXUIGroup, m_pAchievementBanner )->GetTimelineControl()->PlayAnimation( L"Normal", L"EndNormal" );

		TSTATICCAST( ABlobShakeHud, FindChildXUIControl( 0xD9 ) )->Reset();

		if ( APropTriggerHelp::GetHelpHud() != TNULL )
			APropTriggerHelp::GetHelpHud()->HideNow();

		if ( APropTriggerHelp::GetLetterBoxHelpHud() != TNULL )
			APropTriggerHelp::GetLetterBoxHelpHud()->HideNow();
	}

	m_bTargeted     = TFALSE;
	m_fTargetedTime = 0.0f;

	if ( m_pTargetReticle != TNULL )
	{
		m_pTargetReticle->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );

		for ( TINT i = 0; i < 4; i++ )
		{
			m_apDrops[ i ]->SetColour( s_uiReticleColour );
			m_apDrops[ i ]->SetOpacity( 1.0f );
		}

		m_pCircle->SetColour( s_uiReticleColour );
		m_pCircle->SetOpacity( 1.0f );
	}

	m_iBannerQueueSize = 0;
	m_bBannerShowing   = TFALSE;
	m_fMedalBannerTime = 0.0f;

	for ( TINT i = 0; i < MAXBANNERS; i++ )
		m_aBannerQueue[ i ] = TNULL;
}

// $deBlob: FUNCTION 00652fe0
void ABlobHud::QueueUnk25CBanner()
{
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetActiveScene() != this || pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked() )
		return;

	TXUIElement* pBanner = m_pUnk25C;

	if ( pBanner == TNULL )
		return;

	if ( m_iBannerQueueSize > 0 )
	{
		for ( TINT i = 0; i < MAXBANNERS; i++ )
		{
			if ( m_aBannerQueue[ i ] != TNULL && m_aBannerQueue[ i ] == pBanner )
				return;
		}
	}

	if ( m_iBannerQueueSize < MAXBANNERS )
		m_aBannerQueue[ m_iBannerQueueSize++ ] = pBanner;
}

// $deBlob: FUNCTION 00653060
void ABlobHud::QueueUnk260Banner()
{
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetActiveScene() != this || pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked() )
		return;

	TXUIElement* pBanner = m_pUnk260;

	if ( pBanner == TNULL )
		return;

	if ( m_iBannerQueueSize > 0 )
	{
		for ( TINT i = 0; i < MAXBANNERS; i++ )
		{
			if ( m_aBannerQueue[ i ] != TNULL && m_aBannerQueue[ i ] == pBanner )
				return;
		}
	}

	if ( m_iBannerQueueSize < MAXBANNERS )
		m_aBannerQueue[ m_iBannerQueueSize++ ] = pBanner;
}

// $deBlob: FUNCTION 006530e0
void ABlobHud::AddPendingAchievements( TUINT32 a_uiAchievements )
{
	m_uiPendingAchievements |= a_uiAchievements;

	if ( !( a_uiAchievements & Achievements_Medals ) )
	{
		if ( m_iBannerQueueSize < MAXBANNERS )
			m_aBannerQueue[ m_iBannerQueueSize++ ] = m_pAchievementBanner;
	}
	else if ( m_iBannerQueueSize < MAXBANNERS )
	{
		m_aBannerQueue[ m_iBannerQueueSize++ ] = m_pMedalsBanner;
	}
}

// $deBlob: FUNCTION 00653130
void ABlobHud::ShowSplats( TUINT32 a_uiUnused )
{
	m_pSplats->SetVisible( TTRUE );
}

// $deBlob: FUNCTION 00653140
void ABlobHud::HideSplats()
{
	m_pSplats->SetVisible( TFALSE );
}

// $deBlob: FUNCTION 00653150
void ABlobHud::ShowRandomSplat()
{
	TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

	if ( rRandom.GetFloat() < 0.5f )
	{
		TXUIControl* pSplat = m_apSplats[ rRandom.GetInt( 10 ) ];

		if ( !pSplat->GetVisual()->GetTimelineControl()->IsPlaying() )
		{
			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() );
			TUINT32      uiColour;

			if ( ( ( pBlob->GetBlobFlags() >> 0x15 ) & 1 ) == 0 )
			{
				uiColour = pBlob->GetPaintColour();

				if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
					uiColour = ABlobColourComponent::COLOUR_BLACK;
			}
			else
			{
				uiColour = ABlobColourComponent::GetRandomColour();
			}

			uiColour = ABlobColourComponent::ms_auiColours[ uiColour ];
			pSplat->GetVisual()->FindChildXUIElement( L"Splat" )->SetColour( 0xFF000000 | ( uiColour & 0x00FFFFFF ) );
			pSplat->SetVisible( TTRUE );
			pSplat->GetVisual()->PlayAnimation( L"Display", L"EndDisplay" );
		}
	}
}

// $deBlob: FUNCTION 00653290
void ABlobHud::OnFire()
{
	if ( m_pTargetReticle == TNULL || !m_bTargeted )
		return;

	m_pTargetReticle->GetVisual()->StopAnimation();
	m_pTargetReticle->GetVisual()->PlayAnimation( L"Fire", L"EndFire" );

	static TUINT32 s_uiFireColour = TXUIElement::MyMakeARGB( 1.0f, 0.0f, 0.0f, 1.0f );

	for ( TINT i = 0; i < 4; i++ )
		m_apDrops[ i ]->SetColour( s_uiFireColour );

	m_pCircle->SetColour( s_uiFireColour );
}

// $deBlob: FUNCTION 00653350
void ABlobHud::TransFrom()
{
	AGameStateController::GetSingleton()->SetRenderFadesFirst( TFALSE );
	TXUIScene::TransFrom();
}

// $deBlob: FUNCTION 00653370
void ABlobHud::TransBackTo()
{
	AGameState* pState   = AGameStateController::GetSingleton()->GetCurrentGameState();
	TBOOL       bWaiting = TFALSE;

	if ( pState->IsA( &TGetClass( AInGameState ) ) )
	{
		TASSERT( ( pState == TNULL ) || pState->IsA( &TGetClass( AInGameState ) ) );
		bWaiting = TSTATICCAST( AInGameState, pState )->IsWaitingToStart();
	}

	if ( AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer && !bWaiting )
		AGameStateController::GetSingleton()->SetRenderFadesFirst( TTRUE );
	else
		AGameStateController::GetSingleton()->SetRenderFadesFirst( TFALSE );

	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 00653430
void ABlobHud::QueueBanner( TXUIElement* a_pBanner )
{
	if ( m_iBannerQueueSize < MAXBANNERS )
		m_aBannerQueue[ m_iBannerQueueSize++ ] = a_pBanner;
}

// $deBlob: FUNCTION 00653460
void ABlobHud::UpdateBanners( TFLOAT a_fDeltaTime )
{
	if ( m_iBannerQueueSize <= 0 )
		return;

	if ( !m_bBannerShowing )
	{
		ShowNextBanner();
		return;
	}

	TBOOL bPlaying;

	if ( m_aBannerQueue[ 0 ]->IsA( &TGetClass( TXUIScene ) ) )
	{
		if ( m_aBannerQueue[ 0 ] == m_pMedalsBanner && AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel == 0 && m_fMedalBannerTime > 0.0f )
		{
			m_fMedalBannerTime -= a_fDeltaTime;

			if ( m_fMedalBannerTime < 0.0f )
				m_pMedalsBanner->PlayAnimation( L"FadeOut", L"EndFadeOut" );

			return;
		}

		bPlaying = TSTATICCAST( TXUIScene, m_aBannerQueue[ 0 ] )->GetTimelineControl().IsPlaying();
	}
	else
	{
		TASSERT( m_aBannerQueue[ 0 ]->IsA( &TGetClass( TXUIGroup ) ) );
		bPlaying = TSTATICCAST( TXUIGroup, m_aBannerQueue[ 0 ] )->GetTimelineControl()->IsPlaying();
	}

	if ( bPlaying )
		return;

	m_bBannerShowing = TFALSE;

	if ( m_iBannerQueueSize > 1 )
	{
		for ( TINT i = 1; i < m_iBannerQueueSize; i++ )
			m_aBannerQueue[ i - 1 ] = m_aBannerQueue[ i ];

		ShowNextBanner();
	}

	m_aBannerQueue[ m_iBannerQueueSize - 1 ] = TNULL;
	m_iBannerQueueSize--;
	TASSERT( m_iBannerQueueSize >= 0 && m_iBannerQueueSize < MAXBANNERS );
}

// $deBlob: FUNCTION 00653600
void ABlobHud::ShowNextBanner()
{
	TXUIElement* pBanner = m_aBannerQueue[ 0 ];

	if ( pBanner->IsA( &TGetClass( TXUIScene ) ) )
	{
		if ( pBanner != m_pMedalsBanner )
		{
			pBanner->PlayAnimation( L"Display", L"EndDisplay" );
			m_bBannerShowing = TTRUE;
			return;
		}

		TASSERT( ( m_uiPendingAchievements & Achievements_Bronze ) || ( m_uiPendingAchievements & Achievements_Silver ) || ( m_uiPendingAchievements & Achievements_Gold ) );
		ABlobMedalsHud* pMedals = TSTATICCAST( ABlobMedalsHud, m_pMedalsBanner );
		pMedals->PlayAnimation( L"GotMedal", L"EndGotMedal" );
		pMedals->GetMedal()->PlayAnimation( L"MedalAchieved", L"EndMedalAchieved" );

		if ( ( m_uiPendingAchievements & Achievements_Bronze ) == 0 )
		{
			if ( ( m_uiPendingAchievements & Achievements_Silver ) == 0 )
			{
				if ( ( m_uiPendingAchievements & Achievements_Gold ) != 0 )
				{
					pMedals->GetMedals()->PlayAnimation( L"Gold", L"Gold" );
					pMedals->GetMedalText()->SetTextPointer( T2Locale::GetSingleton()->GetString( 0xe2 ) );
					m_uiPendingAchievements &= ~Achievements_Gold;
				}
			}
			else
			{
				pMedals->GetMedals()->PlayAnimation( L"Silver", L"Silver" );
				pMedals->GetMedalText()->SetTextPointer( T2Locale::GetSingleton()->GetString( 0xe1 ) );
				m_uiPendingAchievements &= ~Achievements_Silver;
			}
		}
		else
		{
			pMedals->GetMedals()->PlayAnimation( L"Bronze", L"Bronze" );
			pMedals->GetMedalText()->SetTextPointer( T2Locale::GetSingleton()->GetString( 0xe0 ) );
			m_uiPendingAchievements &= ~Achievements_Bronze;
		}

		m_bBannerShowing = TTRUE;

		if ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel == 0 )
			m_fMedalBannerTime = s_fMedalBannerTime;

		return;
	}

	TASSERT( m_aBannerQueue[ 0 ]->IsA( &TGetClass( TXUIGroup ) ) );

	if ( pBanner != m_pAchievementBanner )
	{
		pBanner->PlayAnimation( L"Display", L"EndDisplay" );
		m_bBannerShowing = TTRUE;
		return;
	}

	// The next pending award: its medal in "gp" and its title
	for ( TINT i = 0; i < AGameStatus::s_iNumAwards; i++ )
	{
		const AGameStatus::Award& rAward = AGameStatus::s_aAwards[ i ];

		if ( rAward.m_uiFlag & m_uiPendingAchievements )
		{
			m_uiPendingAchievements &= ~rAward.m_uiFlag;

			TWCHAR* wszAnim = TStringManager::GetTempString16();
			TStringManager::String16Format( wszAnim, L"Medal%lsOn", rAward.m_wszName );
			m_pAchievementBanner->PlayAnimation( L"Display", L"EndDisplay" );
			m_pGp->PlayAnimation( wszAnim, wszAnim );
			TSTATICCAST( TXUIControl, m_pAchievementName )->SetTextPointer( T2Locale::GetSingleton()->GetString( rAward.m_iString ) );
			m_bBannerShowing = TTRUE;
			return;
		}
	}

	m_bBannerShowing = TTRUE;
}

// $deBlob: FUNCTION 006538e0
TBOOL ABlobHud::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	if ( !m_bTargeted || a_eMessage != XUIMessage_NamedFrame || TStringManager::String16Compare( a_wszParam, L"EndFire" ) != 0 )
		return TXUIControl::SendMessage( a_eMessage, a_wszParam );

	// The reticle gets its colour back after firing
	for ( TINT i = 0; i < 4; i++ )
		m_apDrops[ i ]->SetColour( s_uiReticleColour );

	m_pCircle->SetColour( s_uiReticleColour );
	return TTRUE;
}

// $deBlob: FUNCTION 00653960
void ABlobHud::ShowShowcase( TUINT32 a_uiShowcase )
{
	TUINT32 uiShown = m_uiShownShowcases;

	if ( a_uiShowcase & uiShown )
		return;

	const TWCHAR* wszStart = TNULL;
	const TWCHAR* wszEnd   = TNULL;

	switch ( a_uiShowcase )
	{
		case SHOWCASE_Timer:
			wszEnd   = L"EndShowcaseTimer";
			wszStart = L"ShowcaseTimer";
			break;
		case SHOWCASE_Energy:
			wszEnd             = L"EndShowcaseEnergy";
			m_uiShownShowcases = uiShown | SHOWCASE_Timer | SHOWCASE_Size;
			wszStart           = L"ShowcaseEnergy";
			break;
		case SHOWCASE_Size:
			wszEnd             = L"EndShowcaseSize";
			m_uiShownShowcases = uiShown | SHOWCASE_Timer;
			wszStart           = L"ShowcaseSize";
			break;
		case SHOWCASE_Gate:
			wszEnd             = L"EndShowcaseGate";
			m_uiShownShowcases = uiShown | SHOWCASE_Timer | SHOWCASE_Energy | SHOWCASE_Size;
			wszStart           = L"ShowcaseGate";
			break;
	}

	if ( wszStart != TNULL )
		m_pMain->PlayAnimation( wszStart, wszEnd );

	m_uiShownShowcases |= a_uiShowcase;

	// The timer starts once it's shown
	if ( !( uiShown & SHOWCASE_Timer ) && ( m_uiShownShowcases & SHOWCASE_Timer ) )
		AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
}

ABlobScoreHud* ABlobHud::SetupPlayerPanel( TINT a_iPlayer, ABlobMultiPlayerHud*& a_rpPanel, TINT a_iPanel, TINT a_iScore, TINT a_iSizeColoured, TINT a_iSize, TINT a_iPlacing, TINT a_iScoreFloat )
{
	a_rpPanel = TSTATICCAST( ABlobMultiPlayerHud, FindChildXUIElement( a_iPanel ) );
	a_rpPanel->SetPlayer( TUINT8( a_iPlayer ) );

	ABlobScoreHud* pScoreHud = TSTATICCAST( ABlobScoreHud, a_rpPanel->FindChildXUIElement( a_iScore ) );
	pScoreHud->SetPlayer( TUINT8( a_iPlayer ) );

	TXUIElement*  pSizeColoured = FindChildXUIElement( a_iSizeColoured );
	ABlobSizeHud* pSizeHud      = TSTATICCAST( ABlobSizeHud, a_rpPanel->FindChildXUIElement( a_iSize ) );
	pSizeHud->SetPlayer( TUINT8( a_iPlayer ) );
	pSizeHud->SetColoured( 0, pSizeColoured );
	pSizeHud->SetColoured( 1, TNULL );
	pSizeHud->SetColoured( 2, TNULL );
	pSizeHud->SetColoured( 3, TNULL );

	TSTATICCAST( ABlobPlacingHud, a_rpPanel->FindChildXUIElement( a_iPlacing ) )->SetPlayer( TUINT8( a_iPlayer ) );

	ABlobScoreFloatHud* pScoreFloatHud = TSTATICCAST( ABlobScoreFloatHud, a_rpPanel->FindChildXUIControl( a_iScoreFloat ) );
	pScoreFloatHud->SetPlayer( TINT8( a_iPlayer ) );
	AGameStatus::GetSingleton()->GetLevelData().SetScoreFloatHud( a_iPlayer, pScoreFloatHud );

	return pScoreHud;
}

void ABlobHud::SetPlayerLockElement( TINT a_iPlayer, ABlobMultiPlayerHud* a_pPanel, TINT a_iLock )
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( a_iPlayer ) ) );
	pBlob->GetZJumpComponent()->SetLockElement( a_pPanel->FindChildXUIControl( a_iLock ) );
}

void ABlobHud::OnReadySetGoEnded()
{
	ABlobInputMap::GetSingleton()->Enable();
	m_bUnk134 = TFALSE;
	AGameStateController::GetSingleton()->SetRenderFadesFirst( TTRUE );

	// The panels of the HUD (two to four) show their player marker
	ABlobMultiPlayerHud* apPanels[ 4 ] = { GetPlayer1Hud(), GetPlayer2Hud(), GetPlayer3Hud(), GetPlayer4Hud() };

	for ( TINT i = 0; i < 4; i++ )
	{
		if ( apPanels[ i ] != TNULL )
			apPanels[ i ]->ShowPlayerMarker();
	}

	// The level timer starts
	AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
	AMusicManager::GetSingleton()->ResumeMusic();
}

// $deBlob: FUNCTION 00653c30
void ABlobHud::StartReadySetGo()
{
}

// $deBlob: FUNCTION 00653e40
void ABlobHud::ShowWinner( TINT a_iPlayer )
{
}

// $deBlob: FUNCTION 00653e50
TXUIElement* ABlobHud::GetMessageElement( void** a_ppMessageOwner )
{
	return TNULL;
}

// $deBlob: FUNCTION 00653c40
ABlobMultiPlayerHud* ABlobHud::GetPlayer1Hud()
{
	return TNULL;
}

// $deBlob: FUNCTION 00653c50
ABlobMultiPlayerHud* ABlobHud::GetPlayer2Hud()
{
	return TNULL;
}

// $deBlob: FUNCTION 00653c60
ABlobMultiPlayerHud* ABlobHud::GetPlayer3Hud()
{
	return TNULL;
}

// $deBlob: FUNCTION 00653c70
ABlobMultiPlayerHud* ABlobHud::GetPlayer4Hud()
{
	return TNULL;
}

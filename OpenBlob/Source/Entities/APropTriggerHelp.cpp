#include "pch.h"
#include "APropTriggerHelp.h"
#include "AEntityParams.h"
#include "AEntityIOPacket.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Options/AOptions.h"
#include "Challenges/AChallengeManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "Locale/ALocaleManager.h"
#include "Memory/AMemory.h"
#include "XUI/InGame/ABlobInGame.h"
#include "XUI/InGame/ABlobHelpFullScreen.h"
#include "XUI/InGame/HUD/ABlobHelpHud.h"
#include "XUI/InGame/HUD/ABlobHelpScene.h"

#include <Render/TAssetInit.h>
#include <XUI/TXUI.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TString8.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL             APropTriggerHelp::ms_bHelpDisabled;
TBOOL             APropTriggerHelp::ms_bHelpShown;
TBOOL             APropTriggerHelp::ms_bHelpBlocking;
TBOOL             APropTriggerHelp::ms_bHelpHudShown;
TBOOL             APropTriggerHelp::ms_bUnk9B720C;
TBOOL             APropTriggerHelp::ms_bUnk9B720D;
ABlobHelpHud*     APropTriggerHelp::ms_pHelpHud;
ABlobHelpHud*     APropTriggerHelp::ms_pLetterBoxHelpHud;
TUINT32           APropTriggerHelp::ms_uiShownContextHelps;
Toshi::TXUIScene* APropTriggerHelp::ms_pHelpMenu;
// $deBlob: FUNCTION 00540880
// $deBlob: FUNCTION 00859c90
Toshi::TTRB APropTriggerHelp::sm_oTRB;
TINT8       APropTriggerHelp::sm_iCurrFullscreenTopic     = -1;
TINT        APropTriggerHelp::sm_iCurrFullscreenMessageID = -1;

TSTATICASSERT( sizeof( APropTriggerHelp ) == 0xD0 );

// The messages of the help triggers
static TCONSTEXPR_VAR AEntityIOPacket::MESSAGE s_eMessageEnable   = AEntityIOPacket::MESSAGE_ENABLE; // sent to the help trigger targets of a help trigger
static TCONSTEXPR_VAR AEntityIOPacket::MESSAGE s_eMessageEnable2  = AEntityIOPacket::MESSAGE_CHALLENGESTART;
static TCONSTEXPR_VAR AEntityIOPacket::MESSAGE s_eMessageDisable1 = AEntityIOPacket::MESSAGE_CHALLENGEINIT;
static TCONSTEXPR_VAR AEntityIOPacket::MESSAGE s_eMessageDisable2 = AEntityIOPacket::MESSAGE_CHALLENGERESET;
static TCONSTEXPR_VAR AEntityIOPacket::MESSAGE s_eMessageDisable3 = AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE;

const TCHAR* const APropTriggerHelp::sm_aszFullscreenTopics[ NUM_FULLSCREEN_TOPICS ] = {
	"helpfullscreen",
	"help_zjump",
	"help_blobvision",
	"help_compass",
	"help_getcolour",
	"help_inktcameras",
	"help_jump",
	"help_landmarks",
	"help_mixcolour",
	"help_fightback",
	"help_cameras",
	"help_inkies",
	"help_heavyinkies",
	"help_eliteinkies",
	"help_jetbikes",
	"help_turrets",
	"help_tanks",
	"help_leechbots",
};

// $deBlob: FUNCTION 007617c0
// $deBlob: FUNCTION 007617d0
// $deBlob: FUNCTION 00761840
// $deBlob: FUNCTION 007618a0
// $deBlob: FUNCTION 007618b0
// $deBlob: FUNCTION 00540840
// $deBlob: FUNCTION 00859c80
TDEFINE_CLASS( APropTriggerHelp );

// $deBlob: FUNCTION 007618c0
APropTriggerHelp::APropTriggerHelp()
{
	m_iLocaleID    = -1;
	m_iSSCamID     = -1;
	m_iTopic       = -1;
	m_iHelpType    = -1;
	m_iSpeaker     = 0;
	m_uiHelpFlags  = 0;
	m_fDisplayTime = 5.0f;

	ms_uiShownContextHelps = 0;
	sm_oTRB.SetMemoryFunctions( AMemory::XUITRBAllocator, AMemory::XUITRBDeallocator, TNULL );
}

// $deBlob: FUNCTION 00761960
// $deBlob: FUNCTION 0076269d
// $deBlob: FUNCTION 007626b0
APropTriggerHelp::~APropTriggerHelp()
{
	ResetHelpState();
	ms_uiShownContextHelps = 0;
	sm_oTRB.Close();
}

// $deBlob: FUNCTION 007619f0
// $deBlob: FUNCTION 0076275f
void APropTriggerHelp::ProcessParams( const AEntityParams& a_rParams )
{
	a_rParams.GetValue( m_iLocaleID, "LocaleID" );

	if ( !a_rParams.GetValue( m_iSSCamID, "SSCAM_ID" ) )
		m_iSSCamID = -1;

	if ( m_iLocaleID < 0 )
		a_rParams.GetValue( m_iLocaleID, "STRING_ID" );

	a_rParams.GetValue( m_iTopic, "Topic" );

	TINT8 iTriggerType = 0;
	a_rParams.GetValue( iTriggerType, "TriggerType" );

	if ( iTriggerType == 0 )
		m_uiHelpFlags |= HELPFLAG_NoTriggerType;

	a_rParams.GetValue( m_iHelpType, "HelpType" );
	a_rParams.GetValue( m_fDisplayTime, "DisplayTime" );
	a_rParams.GetValue( m_iSpeaker, "Speaker" );

	if ( m_iHelpType == HELPTYPE_Collectable )
		AGameStatus::GetSingleton()->GetLevelData().AddHelpCollectable();

	APropTrigger::ProcessParams( a_rParams );
	SetHash( m_iHelpType == HELPTYPE_LetterBox );
}

// $deBlob: FUNCTION 00761ae0
void APropTriggerHelp::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	TBOOL bTriggeredOnce = ( m_uiTriggerFlags & TRIGGERFLAG_TriggerOnce ) && ( m_uiTriggerFlags & TRIGGERFLAG_Triggered );

	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TRIGGER && m_iHelpType == HELPTYPE_Collectable )
	{
		m_uiHelpFlags &= ~HELPFLAG_Unk1;
		return;
	}

	if ( !( m_uiHelpFlags & HELPFLAG_Unk1 ) || ( m_uiHelpFlags & HELPFLAG_NoTriggerType ) || bTriggeredOnce )
	{
		if ( a_rPacket.m_eMessage == s_eMessageEnable || a_rPacket.m_eMessage == s_eMessageEnable2 )
		{
			m_uiHelpFlags |= HELPFLAG_Unk1;
			return;
		}

		if ( a_rPacket.m_eMessage == s_eMessageDisable2 || a_rPacket.m_eMessage == s_eMessageDisable1 || a_rPacket.m_eMessage == s_eMessageDisable3 )
		{
			m_uiHelpFlags = ( m_uiHelpFlags & ~HELPFLAG_Unk1 ) | HELPFLAG_Unk2;
			return;
		}
	}
	else
	{
		// The blob in the trigger can read the help (its read button)
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

		if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TOUCH && pBlob != TNULL && pBlob->GetHelpTrigger() != this )
		{
			pBlob->SetHelpTrigger( this );
			APropTrigger::HandleEntityIO( a_pCaller, a_rPacket );
			return;
		}

		if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_UNTOUCH && pBlob != TNULL && pBlob->GetHelpTrigger() == this )
		{
			pBlob->SetHelpTrigger( TNULL );
			APropTrigger::HandleEntityIO( a_pCaller, a_rPacket );
			return;
		}
	}

	APropTrigger::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 00761d30
void APropTriggerHelp::Activate()
{
	Trigger();
	m_uiTriggerFlags |= TRIGGERFLAG_Triggered;

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

	if ( pBlob->GetHelpTrigger() == this )
		pBlob->SetHelpTrigger( TNULL );
}

// $deBlob: FUNCTION 00761be0
void APropTriggerHelp::Reset( ABaseEntity* a_pEntity )
{
	ResetHelpState();

	if ( m_iHelpType == HELPTYPE_Collectable || ( m_uiParentTrigger == 0 && !( m_uiHelpFlags & HELPFLAG_Unk2 ) ) )
		m_uiHelpFlags |= HELPFLAG_Unk1;
	else
		m_uiHelpFlags &= ~HELPFLAG_Unk1;

	m_uiHelpFlags &= ~HELPFLAG_Counted;
	APropTrigger::Reset( a_pEntity );
}

// $deBlob: FUNCTION 00761c60
TBOOL APropTriggerHelp::CanObjectTrigger( TObject* a_pObject )
{
	if ( !( m_uiHelpFlags & HELPFLAG_Unk1 ) || !( m_uiHelpFlags & HELPFLAG_NoTriggerType ) )
		return TFALSE;

	if ( ms_bHelpShown && !ms_bHelpBlocking )
		return TFALSE;

	TXUI* pXUI = TXUI::GetSingleton();

	if ( AChallengeManager::GetSingleton()->IsPromptShown() || pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked() )
		return TFALSE;

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();

	if ( pPlayerManager->IsPlayerUnk60( 0 ) )
		return TFALSE;

	// The letter box helps only trigger while the blob's state is set, the others only while it isn't
	if ( m_iHelpType == HELPTYPE_LetterBox )
	{
		if ( !pPlayerManager->GetBlobData( 0 )->IsKeyFrameCameraActive() )
			return TFALSE;

		if ( m_iHelpType == HELPTYPE_LetterBox )
			return APropTrigger::CanObjectTrigger( a_pObject );
	}

	if ( pPlayerManager->GetBlobData( 0 )->IsKeyFrameCameraActive() )
		return TFALSE;

	return APropTrigger::CanObjectTrigger( a_pObject );
}

// $deBlob: FUNCTION 00762240
void APropTriggerHelp::TriggerAllTargets( ABaseEntity* a_pCaller )
{
	AEntityIOPacket oPacket;
	oPacket.m_eMessage = GetMessageToSend();
	oPacket.m_iData    = 0;

	for ( TINT i = 0; i < GetNumEntityTargets(); i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( pTarget != TNULL )
		{
			// NOTE: the message isn't reset, the targets after a help trigger also get s_eMessageEnable
			if ( pTarget->IsA( &TGetClass( APropTriggerHelp ) ) )
				oPacket.m_eMessage = s_eMessageEnable;

			pTarget->HandleEntityIO( a_pCaller, oPacket );
		}
	}
}

// $deBlob: FUNCTION 007622c0
// $deBlob: FUNCTION 00762754
void APropTriggerHelp::PostCreate()
{
	APropTrigger::PostCreate();

	if ( m_iHelpType == HELPTYPE_Collectable || ( m_uiParentTrigger == 0 && !( m_uiHelpFlags & HELPFLAG_Unk2 ) ) )
		m_uiHelpFlags |= HELPFLAG_Unk1;
	else
		m_uiHelpFlags &= ~HELPFLAG_Unk1;
}

// $deBlob: FUNCTION 00762600
void APropTriggerHelp::SetBBox( TVector4 a_vBBox )
{
	if ( m_iHelpType == HELPTYPE_LetterBox )
		AInvisibleEntity::SetBBox( a_vBBox );
	else
		APropTrigger::SetBBox( a_vBBox );
}

// $deBlob: FUNCTION 00762630
void APropTriggerHelp::Trigger()
{
	if ( !( m_uiTriggerFlags & TRIGGERFLAG_TriggerOnce ) )
		m_uiHelpFlags &= ~HELPFLAG_NoTriggerType;

	ShowHelp( this, m_iHelpType, m_iLocaleID, m_iTopic, TFALSE, m_fDisplayTime, m_iSpeaker, TNULL, TFALSE );
}

// $deBlob: FUNCTION 00761d70
void APropTriggerHelp::ShowContextHelp( TUINT32 a_uiHelp )
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	if ( pGameStatus->GetLevelProperties()->m_sprintLevel != 0 || pGameStatus->GetLevelData().m_bMultiplayer || pGameStatus->GetSPLevel() != 0 )
		return;

	if ( ( ms_uiShownContextHelps & a_uiHelp ) || ms_bHelpShown || ms_bHelpBlocking || APlayerManager::GetSingleton()->IsPlayerUnk60( 0 ) )
		return;

	ms_uiShownContextHelps |= a_uiHelp;

	TINT iStringId;

	switch ( a_uiHelp )
	{
		case 0x1: iStringId = 0x377; break;
		case 0x2: iStringId = 0x36F; break;
		case 0x4: iStringId = 0x370; break;
		case 0x8: iStringId = 0x371; break;
		case 0x10: iStringId = 0x372; break;
		case 0x20: iStringId = 0x373; break;
		case 0x40: iStringId = 0x374; break;
		case 0x80: iStringId = 0x375; break;
		case 0x100: iStringId = 0x376; break;
		case 0x200: iStringId = 0x379; break;
		case 0x400: iStringId = 0x378; break;
		case 0x800: iStringId = 0x37A; break;
		case 0x4000: iStringId = 0x382; break;
		case 0x8000: iStringId = 0x383; break;
		default: return;
	}

	ShowHelp( TNULL, HELPTYPE_Hud, iStringId, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );
}

// $deBlob: FUNCTION 00761f20
void APropTriggerHelp::ShowHelp( APropTriggerHelp* a_pTrigger, TINT8 a_iHelpType, TINT a_iStringId, TINT8 a_iFullScreenTopic, TBOOL a_bBlocking, TFLOAT a_fDisplayTime, TINT8 a_iSpeaker, const TWCHAR* a_wszText, TBOOL a_bForce )
{
	// The help messages are on (the options), or the in-game menu is shown
	TXUIScene* pActiveScene = TXUI::GetSingleton()->GetActiveScene();
	TBOOL      bHelpEnabled = AOptions::GetSingleton()->IsHelpEnabled() || ( pActiveScene != TNULL && pActiveScene->IsA( &TGetClass( ABlobInGame ) ) );

	if ( ms_bHelpDisabled || AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
		return;

	if ( !a_bForce && ms_bHelpShown && a_bBlocking )
		return;

	if ( a_iHelpType == HELPTYPE_Collectable || ( a_iHelpType == HELPTYPE_Scene && bHelpEnabled ) )
	{
		TXUIScene* pScene = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "helphud.xur" ), 0 );
		TASSERT( pScene->IsA( &TGetClass( ABlobHelpScene ) ) );

		ABlobHelpScene* pHelpScene = TSTATICCAST( ABlobHelpScene, pScene );
		pHelpScene->SetHelpText( a_iStringId, a_wszText );
		pHelpScene->SetTrigger( a_pTrigger );

		if ( a_iHelpType == HELPTYPE_Collectable )
		{
			a_pTrigger->SetCollected();

			pHelpScene->ShowCollectableCount();
		}
		else
		{
			pHelpScene->SetSpeaker( a_iSpeaker );
		}

		ms_bHelpBlocking = a_bBlocking;
		ms_bHelpShown    = TTRUE;
		return;
	}

	ABlobHelpHud* pHelpHud = TNULL;

	if ( a_iHelpType == HELPTYPE_Hud )
	{
		if ( ms_pHelpHud != TNULL && a_iStringId != -1 && bHelpEnabled )
			pHelpHud = ms_pHelpHud;
	}
	else if ( a_iHelpType == HELPTYPE_LetterBox )
	{
		if ( ms_pLetterBoxHelpHud != TNULL && a_iStringId != -1 )
			pHelpHud = ms_pLetterBoxHelpHud;
	}
	else if ( a_iHelpType == HELPTYPE_FullScreen && bHelpEnabled )
	{
		TASSERT( a_iFullScreenTopic != -1 );
		sm_iCurrFullscreenTopic     = a_iFullScreenTopic;
		sm_iCurrFullscreenMessageID = a_iStringId;

		// The topic's TRB is streamed by AInGameState, then OpenFullscreenHelp opens the help
		TCHAR* szFileName = TStringManager::GetTempString8();
		TStringManager::String8Format( szFileName, "data/xui/%s/%s.trb", TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() )->GetCurLocaleCode(), sm_aszFullscreenTopics[ a_iFullScreenTopic ] );

		AGameState* pGameState = AGameStateController::GetSingleton()->GetCurrentGameState();
		TASSERT( pGameState->IsA( &TGetClass( AInGameState ) ) );

		if ( ms_bHelpHudShown )
		{
			ms_pHelpHud->HideNow();
			ms_bHelpHudShown = TFALSE;
		}

		TSTATICCAST( AInGameState, pGameState )->RequestStreamedTRB( &sm_oTRB, szFileName, a_pTrigger );
	}

	if ( pHelpHud != TNULL )
	{
		pHelpHud->ShowMessage( a_iStringId, a_fDisplayTime, a_iSpeaker, a_wszText );
		ms_bHelpBlocking = a_bBlocking;
		ms_bHelpShown    = TTRUE;
		ms_bHelpHudShown = TTRUE;
		return;
	}

	if ( ms_bHelpShown )
		ms_bHelpBlocking = a_bBlocking;
}

// $deBlob: FUNCTION 00762680
void APropTriggerHelp::ResetHelpState()
{
	ms_bHelpShown    = TFALSE;
	ms_bHelpBlocking = TFALSE;
	ms_bHelpHudShown = TFALSE;
	ms_bUnk9B720C    = TFALSE;
}

// $deBlob: FUNCTION 00762390
void APropTriggerHelp::CloseFullscreenHelp()
{
	TAssetInit::DeinitAssets( sm_oTRB );
	sm_oTRB.Close();

	sm_iCurrFullscreenTopic     = -1;
	sm_iCurrFullscreenMessageID = -1;
	ms_pHelpMenu                = TNULL;
	ms_bUnk9B720D               = TFALSE;
}

// $deBlob: FUNCTION 007623d0
void APropTriggerHelp::SetCollected()
{
	if ( !( m_uiHelpFlags & HELPFLAG_Counted ) )
	{
		m_uiHelpFlags |= HELPFLAG_Counted;
		AGameStatus::GetSingleton()->GetLevelData().AddHelpCollected();
	}
}

// $deBlob: FUNCTION 00762400
void APropTriggerHelp::OpenFullscreenHelp( APropTriggerHelp* a_pTrigger )
{
	TMemoryHeap* pOldHeap = AMemory::SetTRBHeap( TXUI::ms_pXUIMemoryBlock );
	TAssetInit::InitAssets( sm_oTRB, TTRUE, TFALSE );
	AMemory::SetTRBHeap( pOldHeap );

	TASSERT( sm_iCurrFullscreenTopic != -1 );

	TString8 strSceneName;
	strSceneName.Format( "%s.xur", sm_aszFullscreenTopics[ sm_iCurrFullscreenTopic ] );

	TXUIScene* pScene = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( strSceneName.GetString( 0 ) ), TUINT32( 0 ) );
	TASSERT( pScene->IsA( &TGetClass( ABlobHelpFullScreen ) ) );
	ABlobHelpFullScreen* pHelp = TSTATICCAST( ABlobHelpFullScreen, pScene );

	if ( sm_iCurrFullscreenTopic != 0 )
	{
		pHelp->SetTopic( sm_iCurrFullscreenTopic );
	}
	else
	{
		TASSERT( sm_iCurrFullscreenMessageID != -1 );
		pHelp->SetMessage( T2Locale::GetSingleton()->GetString( sm_iCurrFullscreenMessageID ) );
	}

	pHelp->SetTrigger( a_pTrigger );
	ms_bHelpShown = TTRUE;
	TXUI::GetSingleton()->TransitionTo( ms_pHelpMenu, pHelp );
}

// $deBlob: FUNCTION 00762300
void APropTriggerHelp::LoadFullscreenTRB( const TCHAR* a_szFileName )
{
	sm_oTRB.SetMemoryFunctions( AMemory::TemporaryTRBAllocator, AMemory::TemporaryTRBDeallocator, TNULL );
	TMemoryHeap* pOldTRBHeap = AMemory::SetTRBHeap( TXUI::ms_pXUIMemoryBlock );

	TTRB::ERROR res = sm_oTRB.Load( a_szFileName );
	TASSERT( res == TTRB::ERROR_OK );

	TAssetInit::InitAssets( sm_oTRB, TTRUE, TFALSE );
	AMemory::SetTRBHeap( pOldTRBHeap );
}

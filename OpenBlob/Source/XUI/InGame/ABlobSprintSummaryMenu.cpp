#include "pch.h"
#include "ABlobSprintSummaryMenu.h"
#include "ABlobRewardScreen.h"
#include "Sound/AMusicManager.h"
#include "GUI/AGUIManager.h"
#include "GUI/AGUICursor.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerProfileManager.h"
#include "Challenges/AChallengeManager.h"
#include "MemoryCard/SaveLoad.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIInput.h>
#include <XUI/TXUIVisual.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <Math/TMathInline.h>

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0064c1d0
// $deBlob: FUNCTION 0064c1e0
// $deBlob: FUNCTION 0064c250
// $deBlob: FUNCTION 0064c2c0
// $deBlob: FUNCTION 0064c2d0
// $deBlob: FUNCTION 0053df10
// $deBlob: FUNCTION 00858b40
TDEFINE_CLASS( ABlobSprintSummaryMenu );

// NOTE: retail's order (136 and 137 swapped)
const TINT ABlobSprintSummaryMenu::sm_aiSprintLevelNames[ 20 ] = {
	134, 135, 137, 136, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153
};

// $deBlob: FUNCTION 0064c2e0
ABlobSprintSummaryMenu::ABlobSprintSummaryMenu()
    : ABlobMenu( -1 )
{
	m_pContinue      = TNULL;
	m_pRewardScreen  = TNULL;
	m_pNewMedal      = TNULL;
	m_bRecordPending = TTRUE;
	m_bUnk20D        = TFALSE;
	m_pTitle         = TNULL;
	m_bWaitForFade   = TFALSE;
}

// $deBlob: FUNCTION 0064cc70
// $deBlob: FUNCTION 0064cc90
// $deBlob: FUNCTION 0064cc82
ABlobSprintSummaryMenu::~ABlobSprintSummaryMenu()
{
}

// $deBlob: FUNCTION 0064c340
void ABlobSprintSummaryMenu::OnCreated()
{
	ABlobMenu::OnCreated();

	// The game is paused
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pState )->SetPaused( TTRUE );

	AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.0f );
	AGUIManager::GetSingleton()->GetCursor()->SetVisible( TTRUE, TTRUE );
	AGUIManager::GetSingleton()->SetCursorVisibleCount( 1 );

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	TASSERT( TFALSE == AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );

	// The time
	TXUIControl* pTime = FindChildXUIControl( 12 );
	TFLOAT       fTime = pGameStatus->GetLevelData().m_fLevelTime;
	TMath::Clip( fTime, 0.0f, pGameStatus->GetLevelData().m_fTimeLimit );

	TFLOAT fSeconds = TFLOAT( fmod( TDOUBLE( fTime ), 60.0 ) );
	TFLOAT fTenths  = TFLOAT( fmod( TDOUBLE( fSeconds ), 1.0 ) );
	FormatTime( TINT( fTime / 60.0f ), TINT( fSeconds ), TINT( fTenths * 10.0f ), m_wszTime, 32 );
	pTime->SetText( m_wszTime, -1 );

	// The counter of the level's type
	TXUIElement*       pType      = FindChildXUIElement( 7 );
	TINT               iSlot      = pGameStatus->GetSelectedSlot();
	TXUIControl*       pCounter   = FindChildXUIControl( 6 );
	AChallengeManager* pChallenge = AChallengeManager::GetSingleton();
	TINT               iTotal     = pChallenge->GetCounterTotal();
	TINT               iCount     = pChallenge->GetCounter();

	if ( AGameStatus::IsLandmarkSlot( iSlot ) )
	{
		pType->PlayAnimation( L"Landmark", L"Landmark" );
		TStringManager::String16Format( m_wszCounter, 32, L"%d/%d", iCount, iTotal );
	}
	else if ( AGameStatus::IsPaintSlot( iSlot ) )
	{
		pType->PlayAnimation( L"Paint", L"Paint" );
		TStringManager::String16Format( m_wszCounter, 32, L"%d/%d", iCount, iTotal );
	}
	else if ( AGameStatus::IsCombatSlot( iSlot ) )
	{
		pType->PlayAnimation( L"Combat", L"Combat" );
		TStringManager::String16Format( m_wszCounter, 32, L"%d/%d", iCount, iTotal );
	}
	else if ( AGameStatus::IsRaceSlot( iSlot ) )
	{
		pType->PlayAnimation( L"Race", L"Race" );
		TStringManager::String16Format( m_wszCounter, 32, T2Locale::GetSingleton()->GetString( 210 ) );
	}

	pCounter->SetText( m_wszCounter, -1 );

	// The medal
	TXUIElement* pMedals  = FindChildXUIControl( 5 )->GetVisual()->FindChildXUIElement( L"Medals" );
	TINT         iMedal   = -1;
	TUINT32      uiMedals = pGameStatus->GetLevelData().m_aPlayerData[ 0 ].m_auiUnk3C[ 0 ];

	if ( uiMedals & EnSaveData::AWARD_Bronze )
	{
		iMedal = 57;
		pMedals->PlayAnimation( L"Bronze", L"Bronze" );
	}
	else if ( uiMedals & EnSaveData::AWARD_Silver )
	{
		iMedal = 58;
		pMedals->PlayAnimation( L"Silver", L"Silver" );
	}
	else if ( uiMedals & EnSaveData::AWARD_Gold )
	{
		iMedal = 59;
		pMedals->PlayAnimation( L"Gold", L"Gold" );
	}
	else
	{
		pMedals->PlayAnimation( L"NoMedal", L"NoMedal" );
	}

	TXUIControl* pMedalName = FindChildXUIControl( 4 );

	if ( iMedal == -1 )
		pMedalName->SetText( L"", -1 );
	else
		pMedalName->SetText( T2Locale::GetSingleton()->GetString( iMedal ), -1 );

	// The reward screen follows the results
	TXUI*      pXUI          = TXUI::GetSingleton();
	TXUIScene* pRewardScreen = pXUI->CreateScene( pXUI->FindResource( "rewardscreen.xur" ), TUINT32( 0 ) );
	TASSERT( pRewardScreen->IsA( &TGetClass( ABlobRewardScreen ) ) );
	m_pRewardScreen = TSTATICCAST( ABlobRewardScreen, pRewardScreen );
	m_pRewardScreen->Unlink();

	m_pContinue = FindChildXUIControl( 3 );
	m_pNewMedal = FindChildXUIControl( 14 );

	// A new medal (or a better time of the medal the profile has)
	APlayerProfile* pProfile    = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
	TFLOAT          fMedalTime  = pProfile->GetData()->GetSprintMedalTime( iSlot );
	TUINT32         uiNewMedals = pGameStatus->GetLevelData().m_aPlayerData[ 0 ].m_auiUnk3C[ 0 ];
	TBOOL           bNewMedal;

	if ( ( ( uiNewMedals & EnSaveData::AWARD_Bronze ) && !pProfile->HasSprintBronzeMedal( iSlot ) ) ||
	     ( ( uiNewMedals & EnSaveData::AWARD_Silver ) && !pProfile->HasSprintSilverMedal( iSlot ) ) ||
	     ( ( uiNewMedals & EnSaveData::AWARD_Gold ) && !pProfile->HasSprintGoldMedal( iSlot ) ) )
	{
		bNewMedal = TTRUE;
	}
	else if ( ( ( uiNewMedals & EnSaveData::AWARD_Bronze ) && pProfile->HasSprintSilverMedal( iSlot ) ) || ( ( uiNewMedals & EnSaveData::AWARD_Silver ) && pProfile->HasSprintGoldMedal( iSlot ) ) )
	{
		bNewMedal = TFALSE;
	}
	else if ( fMedalTime == 0.0f )
	{
		bNewMedal = TTRUE;
	}
	else
	{
		bNewMedal = !( fMedalTime <= fTime );
	}

	m_pNewMedal->SetVisible( bNewMedal );

	// The title: the sprint level's name
	m_pTitle = FindChildXUIControl( 16 );
	m_pTitle->SetText( T2Locale::GetSingleton()->GetString( sm_aiSprintLevelNames[ pGameStatus->GetSelectedSlot() ] ), -1 );

	m_pContinue->SetEnabled( TFALSE );

	AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TTRUE );
	AGameStateController::GetSingleton()->FadeFromBlack( 0.5f, 0 );
	m_bWaitForFade = TTRUE;
	pXUI->GetInput()->SetEnabled( TFALSE );
}

// $deBlob: FUNCTION 0064c8a0
void __stdcall ABlobSprintSummaryMenu::FormatTime( TINT a_iMinutes, TINT a_iSeconds, TINT a_iTenths, TWCHAR* a_wszBuffer, TINT a_iSize )
{
	if ( a_iMinutes == 0 )
	{
		const TWCHAR* wszFormat = L"0:%d";

		if ( a_iSeconds < 10 )
			wszFormat = L"0:0%d";

		TStringManager::String16Format( a_wszBuffer, a_iSize, wszFormat, a_iSeconds );
	}
	else
	{
		const TWCHAR* wszFormat;

		if ( a_iSeconds < 10 )
			wszFormat = L"%d:0%d";
		else
			wszFormat = L"%d:%d";

		TStringManager::String16Format( a_wszBuffer, a_iSize, wszFormat, a_iMinutes, a_iSeconds );
	}

	TWCHAR wszTenths[ 32 ];
	TStringManager::String16Format( wszTenths, 32, L"%d", a_iTenths );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail formats the buffer into itself (overlapping buffers)
	TWCHAR wszTime[ 32 ];
	TStringManager::String16Copy( wszTime, a_wszBuffer, 32 );
	TStringManager::String16Format( a_wszBuffer, a_iSize, L"%ls.%ls", wszTime, wszTenths );
#else
	TStringManager::String16Format( a_wszBuffer, a_iSize, L"%ls.%ls", a_wszBuffer, wszTenths );
#endif
}

// $deBlob: FUNCTION 0064c940
TBOOL ABlobSprintSummaryMenu::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( m_bRecordPending || m_bUnk20D )
		return TFALSE;

	if ( a_pSource != m_pContinue )
		return ABlobMenu::OnNotifyPress( a_pSource, a_iController );

	if ( m_pRewardScreen->GetNumRewards() > 0 )
	{
		// The reward screen shows the unlocks
		FindParentControl()->AddChildTail( m_pRewardScreen );
		TXUI::GetSingleton()->TransitionTo( this, m_pRewardScreen );
		return TTRUE;
	}

	if ( m_pRewardScreen != TNULL )
		delete m_pRewardScreen;

	TXUI*        pXUI      = TXUI::GetSingleton();
	TXUIScene*   pGameOver = pXUI->CreateScene( pXUI->FindResource( "sprintgameover.xur" ), TUINT32( 0 ) );
	TXUIElement* pParent   = FindParentControl();

	if ( pParent != TNULL )
		pParent->AddChildTail( pGameOver );

	pXUI->TransitionTo( this, pGameOver );
	return TTRUE;
}

// $deBlob: FUNCTION 0064ca20
void ABlobSprintSummaryMenu::RecordResults()
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	if ( pGameStatus->GetLevelMode() != -1 )
	{
		TFLOAT             fTime      = pGameStatus->GetLevelData().m_fLevelTime;
		AChallengeManager* pChallenge = AChallengeManager::GetSingleton();

		APlayerProfileManager::GetSingleton()->GetCurrentProfile()->RecordSprintResult(
		    pGameStatus->GetSelectedSlot(),
		    pGameStatus->GetLevelData().m_aPlayerData[ 0 ].m_auiUnk3C[ 0 ],
		    fTime,
		    pChallenge->GetCounter(),
		    pChallenge->GetCounterTotal(),
		    m_pRewardScreen
		);

		return;
	}

	m_pContinue->SetEnabled( TTRUE );
}

// $deBlob: FUNCTION 0064cac0
void ABlobSprintSummaryMenu::Tick( TFLOAT a_fDeltaTime )
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( m_bWaitForFade )
	{
		// The fade in is over: the cursor is back
		if ( !pController->IsFading( 0 ) && pController->GetFade( 0 ) == 0.0f )
		{
			AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TFALSE );
			m_bWaitForFade = TFALSE;
			AGUIManager::GetSingleton()->SetCursorActive( TTRUE );
			AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );
			TXUIScene::Tick( a_fDeltaTime );
			return;
		}

		if ( m_bWaitForFade )
		{
			TXUIScene::Tick( a_fDeltaTime );
			return;
		}
	}

	if ( m_bRecordPending == TTRUE && TXUI::GetSingleton()->GetTransToScene() == TNULL && TXUI::GetSingleton()->GetTransBackFromScene() == TNULL && !TXUI::GetSingleton()->IsInputLocked() )
	{
		m_bRecordPending = TFALSE;
		RecordResults();
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0064cb90
TBOOL ABlobSprintSummaryMenu::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	// The results' animation is over: the profile is saved
	if ( a_eMessage == XUIMessage_NamedFrame && TStringManager::String16Compare( a_wszParam, L"EndSprintsSum", -1 ) == 0 && AGameStatus::GetSingleton()->GetLevelMode() != -1 )
	{
		SaveLoad::GetInstance()->SetCaller( this );

		if ( SaveLoad::GetInstance()->IsEnabled() )
		{
			SaveLoad::GetInstance()->SaveProfile( 0, 0 );
			SaveLoad::GetInstance()->Update( 0.0f );
		}

		m_pContinue->SetEnabled( TTRUE );
		return TTRUE;
	}

	return TXUIControl::SendMessage( a_eMessage, a_wszParam );
}

// $deBlob: FUNCTION 0064cc30
void ABlobSprintSummaryMenu::EndTransBackTo()
{
	if ( m_bUnk20D )
	{
		m_bUnk20D = TFALSE;
		m_pContinue->SetEnabled( TTRUE );
	}

	ABlobMenu::EndTransBackTo();
}

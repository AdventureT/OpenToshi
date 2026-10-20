#include "pch.h"
#include "ABlobScoreMenu.h"
#include "GameInterface/ABossEndSeqState.h"
#include "ABlobRewardScreen.h"
#include "XUI/TXUIBackButton.h"
#include "Sound/ASoundInterface.h"
#include "Sound/AMusicManager.h"
#include "GUI/AGUIManager.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerProfileManager.h"
#include "MemoryCard/SaveLoad.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIGroup.h>
#include <XUI/TXUIVisual.h>
#include <XUI/TXUICanvas.h>
#include <XUI/TXUIInput.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0064ac50
// $deBlob: FUNCTION 0064ac60
// $deBlob: FUNCTION 0064acd0
// $deBlob: FUNCTION 0064ad40
// $deBlob: FUNCTION 0064ad50
// $deBlob: FUNCTION 0053de90
// $deBlob: FUNCTION 00858b20
TDEFINE_CLASS( ABlobScoreMenu );

ABlobScoreMenu* ABlobScoreMenu::sm_pInstance = TNULL;

// The names of the worlds (the same table as ABlobLevelSelection's)
static const TINT s_aiWorldNames[ 10 ] = { 124, 125, 126, 127, 128, 129, 130, 131, 132, 133 };

// $deBlob: FUNCTION 0064ad60
ABlobScoreMenu::ABlobScoreMenu()
    : ABlobMenu( -1 )
{
	m_pTransitionControl = TNULL;
	m_pNoMedal           = TNULL;
	m_pNewRecord         = TNULL;
	m_bTransOutRequested = TFALSE;
	m_bResultsBusy       = TFALSE;
	m_bResultsShown      = TFALSE;
	m_pTransElement      = TNULL;
	m_pRewardScreen      = TNULL;
	m_bLeaving           = TFALSE;
	m_bWaitForFade       = TFALSE;
}

// $deBlob: FUNCTION 0064bd20
// $deBlob: FUNCTION 0064bd09
ABlobScoreMenu::~ABlobScoreMenu()
{
	sm_pInstance = TNULL;
}

// $deBlob: FUNCTION 0064b3b0
void ABlobScoreMenu::Tick( TFLOAT a_fDeltaTime )
{

	if ( !m_bLeaving )
	{
		// The fade in is over: the input is back
		if ( m_bWaitForFade && !AGameStateController::GetSingleton()->IsFading( 0 ) && AGameStateController::GetSingleton()->GetFade( 0 ) == 0.0f )
		{
			AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TFALSE );
			m_bWaitForFade = TFALSE;
			TXUI::GetSingleton()->GetInput()->SetEnabled( TTRUE );
			TXUIScene::Tick( a_fDeltaTime );
			return;
		}
	}
	else if ( !AGameStateController::GetSingleton()->IsFading( 0 ) && AGameStateController::GetSingleton()->GetFade( 0 ) == 1.0f )
	{
		// The fade out is over: the level ends
		if ( m_bWaitForFade )
		{
			m_bWaitForFade = TFALSE;

			AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();

			if ( pState->IsA( &TGetClass( ABossEndSeqState ) ) )
				TSTATICCAST( ABossEndSeqState, pState )->Finish();
			else
			{
				TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
				TSTATICCAST( AInGameState, pState )->EndLevelFromResults();
			}
		}

		TXUIScene::Tick( a_fDeltaTime );
		return;
	}

	if ( !m_bWaitForFade && !TSTATICCAST( TXUIGroup, m_pTransElement )->GetTimelineControl()->IsPlaying() )
	{
		if ( !m_bResultsShown )
		{
			if ( !m_bResultsBusy )
			{
				// The results are recorded in the profile (and saved), the player may go on once that is done
				m_bResultsBusy = TTRUE;

				if ( m_pBackButton != TNULL )
					m_pBackButton->SetEnabled( TFALSE );

				APlayerProfile* pProfile   = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
				TUINT32         uiFlags    = AGameStatus::GetSingleton()->GetLevelData().m_aPlayerData[ 0 ].m_auiUnk3C[ 0 ];
				TINT            eLevelMode = AGameStatus::GetSingleton()->GetLevelMode();
				TINT            iLevel     = AGameStatus::GetSingleton()->GetSPLevel();

				TASSERT( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE );

				if ( eLevelMode == -1 )
				{
					m_bResultsShown = TTRUE;
				}
				else
				{
					TFLOAT fTime = AGameStatus::GetSingleton()->GetLevelData().m_fLevelTime;
					pProfile->RecordLevelResult( iLevel, eLevelMode, uiFlags, fTime, AGameStatus::GetSingleton()->GetLevelData().GetPoints( 0 ), m_pRewardScreen );

					// The profile is saved at once
					SaveLoad::GetInstance()->SetCaller( this );

					if ( SaveLoad::GetInstance()->IsEnabled() )
					{
						SaveLoad::GetInstance()->SaveProfile( 0, 0 );
					}
					else
					{
						m_bResultsBusy  = TFALSE;
						m_bResultsShown = TTRUE;

						if ( m_pBackButton != TNULL )
							m_pBackButton->SetEnabled( TTRUE );
					}

					SaveLoad::GetInstance()->Update( 0.0f );

					m_bResultsBusy  = TFALSE;
					m_bResultsShown = TTRUE;

					if ( m_pBackButton != TNULL )
						m_pBackButton->SetEnabled( TTRUE );
				}
			}
		}
		else if ( m_bTransOutRequested )
		{
			m_bTransOutRequested = TFALSE;

			if ( m_pRewardScreen->GetNumRewards() < 1 )
			{
				// Nothing was unlocked: fades out and leaves
				AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TTRUE );
				AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
				m_bLeaving     = TTRUE;
				m_bWaitForFade = TTRUE;

				if ( m_pRewardScreen != TNULL )
					delete m_pRewardScreen;
			}
			else
			{
				// The reward screen shows the unlocks
				TXUIElement* pParent = FindParentControl();

				if ( pParent != TNULL )
					pParent->AddChildTail( m_pRewardScreen );
				else
					TXUI::GetSingleton()->GetCanvas()->AddScene( m_pRewardScreen );

				TXUI::GetSingleton()->TransitionTo( this, m_pRewardScreen );
			}
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0064adf0
void ABlobScoreMenu::OnCreated()
{
	ABlobMenu::OnCreated();

	// The award controls and their awarded flags
	for ( TINT i = 0; i < MAXAWARDS; i++ )
	{
		m_abAwarded[ i ] = TFALSE;

		TWCHAR wszName[ 0x40 ];
		T2String16::Format( wszName, 0x40, L"Award%d", i + 1 );
		m_apAwards[ i ] = FindChildXUIControl( wszName );
	}

	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );

	// In a level the game is paused, the music goes on and the vista camera's snapshot plays
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();

	if ( pState->IsA( &TGetClass( AInGameState ) ) )
	{
		TASSERT( TFALSE == AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer );
		TSTATICCAST( AInGameState, pState )->SetPaused( TTRUE );
		AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.75f );
		AMusicManager::GetSingleton()->PlaySnapshot( "vistacam" );
	}
	else
	{
		// NOTE: retail checks IsA( ABossEndSeqState ) here and ignores the result
		pState->IsA( &TGetClass( ABossEndSeqState ) );
	}

	m_pTransitionControl     = FindChildXUIControl( 0x27 );
	TXUIElement*    pResults = FindChildXUIElement( 2 );
	TINT            iLevel   = AGameStatus::GetSingleton()->GetSPLevel();
	APlayerProfile* pProfile = APlayerProfileManager::GetSingleton()->GetCurrentProfile();
	m_pTransElement          = FindChildXUIElement( 2 );

	// The title: the world's name
	TXUIControl* pTitle = FindChildXUIControl( 6 );

	if ( TUINT( iLevel ) < 10 )
		pTitle->SetText( T2Locale::GetSingleton()->GetString( s_aiWorldNames[ iLevel ] ), -1 );
	else
		pTitle->SetText( L"", -1 );

	// The score and whether it's a new record
	TINT         iPoints = AGameStatus::GetSingleton()->GetLevelData().GetPoints( 0 );
	TXUIControl* pScore  = TSTATICCAST( TXUIControl, pResults->FindChildXUIElement( 0xC ) );
	TStringManager::String16Format( m_wszScore, 0x10, T2Locale::GetSingleton()->GetString( 0xBE ), iPoints );
	pScore->SetText( m_wszScore, -1 );

	m_pNewRecord = FindChildXUIControl( 0xE );
	m_pNewRecord->SetVisible( pProfile->GetData()->m_auiBestPoints[ iLevel ] < TUINT32( iPoints ) );
	FindChildXUIElement( 0xD )->SetVisible( m_pNewRecord->IsVisible() );

	// The time of the level
	TXUIControl* pTime = TSTATICCAST( TXUIControl, pResults->FindChildXUIElement( 0xA ) );
	TFLOAT       fTime = AGameStatus::GetSingleton()->GetLevelData().m_fLevelTime;
	FormatTime( TINT( fTime / 60.0f ), TINT( TFLOAT( fmod( TDOUBLE( fTime ), 60.0 ) ) ), m_wszTime, 0x10, -1 );
	pTime->SetText( m_wszTime, -1 );

	// A new medal of the world (hidden when the level gave none, or no better one than the profile has)
	m_pNoMedal         = FindChildXUIControl( 0x10 );
	TFLOAT  fMedalTime = pProfile->GetData()->GetMedalTime( iLevel );
	TUINT32 uiFlags    = AGameStatus::GetSingleton()->GetLevelData().m_aPlayerData[ 0 ].m_auiUnk3C[ 0 ];
	TBOOL   bNoNewMedal;

	if ( ( ( uiFlags & EnSaveData::AWARD_Bronze ) && !pProfile->HasBronzeMedal( iLevel ) ) ||
	     ( ( uiFlags & EnSaveData::AWARD_Silver ) && !pProfile->HasSilverMedal( iLevel ) ) ||
	     ( ( uiFlags & EnSaveData::AWARD_Gold ) && !pProfile->HasGoldMedal( iLevel ) ) )
	{
		bNoNewMedal = TTRUE;
	}
	else if ( ( ( uiFlags & EnSaveData::AWARD_Bronze ) && pProfile->HasSilverMedal( iLevel ) ) || ( ( uiFlags & EnSaveData::AWARD_Silver ) && pProfile->HasGoldMedal( iLevel ) ) )
	{
		bNoNewMedal = TFALSE;
	}
	else if ( fMedalTime == 0.0f )
	{
		bNoNewMedal = TTRUE;
	}
	else
	{
		bNoNewMedal = !( fMedalTime <= AGameStatus::GetSingleton()->GetLevelData().m_fLevelTime );
	}

	m_pNoMedal->SetVisible( bNoNewMedal );
	FindChildXUIElement( 0xF )->SetVisible( m_pNoMedal->IsVisible() );

	TStringManager::GetTempString16();

	// The achievement controls (ids 0x31 down to 0x28)
	for ( TINT i = 0; i < MAXACHIEVEMENTS; i++ )
		m_apAchievements[ i ] = FindChildXUIControl( 0x31 - i );

	SetupMedals();

	// The reward screen follows the results
	sm_pInstance = this;

	TXUIScene* pRewardScreen = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "rewardscreen.xur" ), 0 );
	TASSERT( pRewardScreen->IsA( &TGetClass( ABlobRewardScreen ) ) );
	m_pRewardScreen = TSTATICCAST( ABlobRewardScreen, pRewardScreen );
	m_pRewardScreen->Unlink();

	AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TTRUE );
	AGameStateController::GetSingleton()->FadeFromBlack( 0.5f, 0 );
	m_bWaitForFade = TTRUE;
	TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
}

// $deBlob: FUNCTION 0064b820
void ABlobScoreMenu::SetupMedals()
{
	TWCHAR*                 wszAnim    = TStringManager::GetTempString16();
	AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();
	TUINT32                 uiMedals   = rLevelData.m_aPlayerData[ 0 ].m_auiUnk3C[ 0 ];

	// The medal of the level
	TXUIElement* pMedals = FindChildXUIControl( 0x11 )->GetVisual()->FindChildXUIElement( L"Medals" );

	if ( uiMedals & EnSaveData::AWARD_Bronze )
		pMedals->PlayAnimation( L"Bronze", L"Bronze" );
	else if ( uiMedals & EnSaveData::AWARD_Silver )
		pMedals->PlayAnimation( L"Silver", L"Silver" );
	else if ( uiMedals & EnSaveData::AWARD_Gold )
		pMedals->PlayAnimation( L"Gold", L"Gold" );
	else
		pMedals->PlayAnimation( L"NoMedal", L"NoMedal" );

	// The paint award (the best of Paint25..Paint100) and the paint percentage
	TINT i;

	for ( i = 0; i < 4; i++ )
	{
		if ( AGameStatus::s_aAwards[ i ].m_uiFlag & uiMedals )
		{
			TStringManager::String16Format( wszAnim, L"Medal%lsOn", AGameStatus::s_aAwards[ i ].m_wszName );
			m_abAwarded[ 0 ] = TTRUE;
			break;
		}
	}

	if ( i == 4 )
		TStringManager::String16Format( wszAnim, L"%ls", L"MedalPaintOff" );

	TXUIVisual* pPaintVisual = m_apAchievements[ 0 ]->GetVisual();
	pPaintVisual->FindChildXUIControl( L"Title" )->SetText( T2Locale::GetSingleton()->GetString( 0x47 ), -1 );

	TXUIControl*  pPercent = pPaintVisual->FindChildXUIControl( L"100percent" );
	const TWCHAR* wszPercent;

	if ( TINT( rLevelData.m_auiLevelCounts[ 0 ] ) < 1 )
	{
		wszPercent = L"";
	}
	else
	{
		TStringManager::String16Format( m_awszAwardTexts[ 1 ], L"%d%%", TINT( rLevelData.GetPaintPercentage( 0 ) ) );
		wszPercent = m_awszAwardTexts[ 1 ];
	}

	pPercent->SetText( wszPercent, -1 );
	pPaintVisual->FindChildXUIElement( L"gp" )->PlayAnimation( wszAnim, wszAnim );

	// The other awards: on or off, with their counters (found / in the level)
	for ( i = 4; i < AGameStatus::s_iNumAwards; i++ )
	{
		const AGameStatus::Award& rAward = AGameStatus::s_aAwards[ i ];

		if ( TStringManager::String16Length( rAward.m_wszName ) < 1 )
			continue;

		TBOOL bAwarded = ( rAward.m_uiFlag & uiMedals ) != 0;

		if ( bAwarded )
			TStringManager::String16Format( wszAnim, L"Medal%lsOn", rAward.m_wszName );
		else
			TStringManager::String16Format( wszAnim, L"Medal%lsOff", rAward.m_wszName );

		TINT    iTotal  = -1;
		TINT    iCount  = -1;
		TWCHAR* wszText = TNULL;

		switch ( rAward.m_uiFlag )
		{
			case EnSaveData::AWARD_Challenge:
				iCount           = rLevelData.m_iNumCompletedChallenges;
				iTotal           = rLevelData.m_iNumChallenges;
				wszText          = m_awszAwardTexts[ 0 ];
				m_abAwarded[ 1 ] = bAwarded;
				break;
			case EnSaveData::AWARD_Styles:
				iCount           = TINT( rLevelData.m_aPlayerData[ 0 ].m_auiUnk3C[ 2 ] );
				iTotal           = TINT( rLevelData.m_auiUnk160[ 0 ] );
				wszText          = m_awszAwardTexts[ 7 ];
				m_abAwarded[ 3 ] = bAwarded;
				break;
			case EnSaveData::AWARD_Trees:
				iCount           = TINT( rLevelData.m_aPlayerData[ 0 ].m_auiProgress[ 0 ] );
				iTotal           = TINT( rLevelData.m_auiLevelCounts[ 1 ] );
				wszText          = m_awszAwardTexts[ 2 ];
				m_abAwarded[ 4 ] = bAwarded;
				break;
			case EnSaveData::AWARD_Landmarks:
				iCount           = TINT( rLevelData.m_aPlayerData[ 0 ].m_auiProgress[ 4 ] );
				iTotal           = TINT( rLevelData.m_auiLevelCounts[ 2 ] );
				wszText          = m_awszAwardTexts[ 6 ];
				m_abAwarded[ 5 ] = bAwarded;
				break;
			case EnSaveData::AWARD_Bill:
				iCount           = TINT( rLevelData.m_aPlayerData[ 0 ].m_auiProgress[ 5 ] );
				iTotal           = TINT( rLevelData.m_auiLevelCounts[ 3 ] );
				wszText          = m_awszAwardTexts[ 5 ];
				m_abAwarded[ 6 ] = bAwarded;
				break;
			case EnSaveData::AWARD_Transform:
				iTotal           = TINT( rLevelData.m_auiLevelCounts[ 4 ] );
				iCount           = TINT( rLevelData.m_aPlayerData[ 0 ].m_auiProgress[ 6 ] );
				wszText          = m_awszAwardTexts[ 4 ];
				m_abAwarded[ 7 ] = bAwarded;
				break;
			case EnSaveData::AWARD_Raydian:
				iCount           = TINT( rLevelData.m_aPlayerData[ 0 ].m_auiProgress[ 2 ] );
				iTotal           = TINT( rLevelData.m_auiLevelCounts[ 6 ] );
				wszText          = m_awszAwardTexts[ 3 ];
				m_abAwarded[ 8 ] = bAwarded;
				break;
			case EnSaveData::AWARD_Vista:
				iCount           = ( uiMedals >> 16 ) & 1;
				iTotal           = 1;
				wszText          = m_awszAwardTexts[ 8 ];
				m_abAwarded[ 9 ] = bAwarded;
				break;
		}

		TINT iControl = i - 3;
		TASSERT( iControl < MAXACHIEVEMENTS );

		TXUIVisual* pVisual = m_apAchievements[ iControl ]->GetVisual();
		pVisual->FindChildXUIControl( L"Title" )->SetText( T2Locale::GetSingleton()->GetString( rAward.m_iString ), -1 );

		TXUIControl* pCounter = pVisual->FindChildXUIControl( L"100percent" );

		if ( ( rAward.m_uiFlag & EnSaveData::AWARD_Time ) == 0 )
		{
			if ( iCount == -1 )
			{
				pCounter->SetText( L"", -1 );
			}
			else
			{
				TStringManager::String16Format( wszText, L"%d/%d", iCount, iTotal );
				pCounter->SetText( wszText, -1 );
			}
		}
		else if ( ( uiMedals & EnSaveData::AWARD_Time ) == 0 )
		{
			// The time award not won: the time to beat
			TFLOAT fTargetTime = rLevelData.m_fTargetTime;
			FormatTime( TINT( fTargetTime / 60.0f ), TINT( TFLOAT( fmod( TDOUBLE( fTargetTime ), 60.0 ) ) ), m_awszAwardTexts[ 9 ], 0x10, -1 );
			pCounter->SetText( m_awszAwardTexts[ 9 ], -1 );
		}
		else
		{
			// Won: the level's time
			pCounter->SetText( m_wszTime, -1 );
			m_abAwarded[ 2 ] = TTRUE;
		}

		pVisual->FindChildXUIElement( L"gp" )->PlayAnimation( wszAnim, wszAnim );
	}
}

// $deBlob: FUNCTION 0065b4f0
void __stdcall ABlobScoreMenu::FormatTime( TINT a_iMinutes, TINT a_iSeconds, TWCHAR* a_wszBuffer, TINT a_iSize, TINT a_iUnused )
{
	if ( a_iMinutes == 0 )
	{
		const TWCHAR* wszFormat = L"0:%d";

		if ( a_iSeconds < 10 )
			wszFormat = L"0:0%d";

		TStringManager::String16Format( a_wszBuffer, a_iSize, wszFormat, a_iSeconds );
		return;
	}

	const TWCHAR* wszFormat;

	if ( a_iSeconds < 10 )
		wszFormat = L"%d:0%d";
	else
		wszFormat = L"%d:%d";

	TStringManager::String16Format( a_wszBuffer, a_iSize, wszFormat, a_iMinutes, a_iSeconds );
}

// $deBlob: FUNCTION 0064b7c0
TBOOL ABlobScoreMenu::OnNotifySetFocus( Toshi::TXUIElement* a_pSource )
{
	for ( TINT i = 0; i < MAXAWARDS; i++ )
	{
		if ( a_pSource == m_apAwards[ i ] )
		{
			// The award's sound: ASoundInterface::PlayEvent( group 4, event 0x4D or 0x4F )
			// NOTE: retail returns the masked result of the event call, always 0 as TBOOL
			if ( m_abAwarded[ i ] )
			{
				ASoundInterface::PlayEvent( ASoundMap::GROUPID( 4 ), ASoundMap::EVENTID( 0x4D ), FMOD_EVENT_DEFAULT, TTRUE );
			}
			else
			{
				ASoundInterface::PlayEvent( ASoundMap::GROUPID( 4 ), ASoundMap::EVENTID( 0x4F ), FMOD_EVENT_DEFAULT, TTRUE );
			}
			return TFALSE;
		}
	}

	return ABlobMenu::OnNotifySetFocus( a_pSource );
}

// $deBlob: FUNCTION 0064b330
TBOOL ABlobScoreMenu::HandleButton( TINT a_eButton, TINT a_iController )
{
	// The player may leave once the results are shown and the transition control has fully faded in (opacity 127)
	if ( !m_bTransOutRequested && m_bResultsShown && a_eButton == XUIButton_Accept )
	{
		if ( m_pTransitionControl->GetFlags_Opacity() == 1.0f )
		{
			m_bTransOutRequested = TTRUE;
			m_pTransElement->PlayAnimation( L"TransFrom", L"EndTransFrom" );
		}
	}

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 0064b770
void ABlobScoreMenu::EndTransBackTo()
{
	if ( m_bResultsBusy )
	{
		m_bResultsBusy  = TFALSE;
		m_bResultsShown = TTRUE;

		// The back button is enabled (and its visual played) if it was not enabled yet
		if ( m_pBackButton != TNULL )
		{
			m_pBackButton->SetEnabled( TTRUE );
		}
	}

	ABlobMenu::EndTransBackTo();
}

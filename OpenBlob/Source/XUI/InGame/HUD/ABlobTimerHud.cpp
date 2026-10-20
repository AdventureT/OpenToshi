#include "pch.h"
#include "ABlobTimerHud.h"
#include "Challenges/AChallengeManager.h"
#include "Challenges/AChallenge.h"
#include "GameController/AGameStatus.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUIVisual.h>

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Below this many seconds the time is low (challenge timer, level timer)
static TINT s_iChallengeLowTime = 15;
static TINT s_iLevelLowTime     = 60;

// $deBlob: FUNCTION 0065adc0
// $deBlob: FUNCTION 0065add0
// $deBlob: FUNCTION 0065ae40
// $deBlob: FUNCTION 0065aeb0
// $deBlob: FUNCTION 0065aec0
// $deBlob: FUNCTION 0053e5c0
// $deBlob: FUNCTION 00858ce0
TDEFINE_CLASS( ABlobTimerHud );

// $deBlob: FUNCTION 0065b550
ABlobTimerHud::ABlobTimerHud()
    : m_bDirty( TFALSE )
    , m_iMinutes( 0 )
    , m_iSeconds( 0 )
    , m_iTenths( -1 )
    , m_bChallengeTimer( TFALSE )
    , m_pBar( TNULL )
    , m_pTimeup( TNULL )
    , m_fBarStart( 0.0f )
    , m_fSprintStart( 0.0f )
    , m_fBarEnd( 0.0f )
    , m_eWarning( Warning_None )
{
}

// $deBlob: FUNCTION 0065b5f0
// $deBlob: FUNCTION 0065b5e5
ABlobTimerHud::~ABlobTimerHud()
{
}

// $deBlob: FUNCTION 0065aed0
void ABlobTimerHud::OnCreated()
{
	m_pFill           = TSTATICCAST( TXUIGroup, FindChildXUIElement( 6 ) );
	m_bFill           = TTRUE;
	m_bChallengeTimer = TStringManager::String16Compare( GetID(), L"ChallengeTimerHud", -1 ) == 0;

	if ( !m_bChallengeTimer && !AGameStatus::GetSingleton()->GetLevelData().m_bMultiplayer )
	{
		m_pBar    = TSTATICCAST( TXUIGroup, FindChildXUIElement( 7 ) );
		m_pTimeup = TSTATICCAST( TXUIGroup, FindChildXUIElement( 15 ) );
	}

	if ( m_pBar != TNULL )
	{
		m_pBar->StopAnimation();
		m_fBarStart    = 0.0f;
		m_fSprintStart = m_pBar->GetTimelineControl()->GetNamedFrameTime( L"SprintStart" );
		m_fBarEnd      = m_pBar->GetTimelineControl()->GetNamedFrameTime( L"End" );
	}
}

// $deBlob: FUNCTION 0065af80
void ABlobTimerHud::Tick( TFLOAT a_fDeltaTime )
{
	if ( IsVisible() && m_bDirty == TTRUE )
	{
		m_bDirty = TFALSE;

		if ( m_iMinutes != 0 )
		{
			if ( m_eWarning != Warning_None )
			{
				PlayAnimation( L"Normal", L"EndNormal" );
				m_pVisual->PlayAnimation( 0, 1 );

				if ( m_pTimeup != TNULL )
					m_pTimeup->GetTimelineControl()->ResetAnimation();

				m_bDirty = TTRUE;
				SetTime( m_iMinutes, m_iSeconds, m_iTenths );
			}

			m_eWarning = Warning_None;
			TXUIScene::Tick( a_fDeltaTime );
			return;
		}

		TINT iSeconds = m_iSeconds;

		if ( iSeconds != 0 )
		{
			if ( iSeconds < 5 )
			{
				m_pVisual->PlayAnimation( L"TimeVeryLow", L"EndTimeVeryLow" );
				m_eWarning = Warning_VeryLow;
			}
			else
			{
				// NOTE: retail reads the level's m_sprintLevel here, but both branches end up the same
				if ( iSeconds < s_iChallengeLowTime || ( !m_bChallengeTimer && iSeconds < s_iLevelLowTime ) )
				{
					m_pVisual->PlayAnimation( L"TimeLow", L"EndTimeLow" );
					m_eWarning = Warning_Low;
				}
				else
				{
					m_pVisual->PlayAnimation( 0, 1 );
					m_eWarning = Warning_None;
				}
			}
		}
		else if ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel == 0 || m_iTenths == 0 )
		{
			m_pVisual->PlayAnimation( L"Timeup", L"EndTimeup" );
			m_eWarning = Warning_Timeup;

			if ( m_pTimeup != TNULL )
			{
				m_oTimelineControl.PlayAnimation( L"Timeup", L"EndTimeup" );
				m_pTimeup->StopAnimation();
				m_pTimeup->PlayAnimation( -1, -1 );
			}
		}

		if ( m_eWarning == Warning_Low || m_eWarning == Warning_VeryLow )
			PlayAnimation( L"TimeLow", L"EndTimeLow" );
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0065b1c0
void ABlobTimerHud::SetTime( TINT a_iMinutes, TINT a_iSeconds, TINT a_iTenths )
{
	if ( a_iMinutes != m_iMinutes )
	{
		m_iMinutes = a_iMinutes;
		m_bDirty   = TTRUE;
	}

	if ( a_iSeconds != m_iSeconds )
	{
		m_iSeconds = a_iSeconds;
		m_bDirty   = TTRUE;
	}

	if ( a_iTenths != -1 && a_iTenths != m_iTenths )
	{
		m_iTenths = a_iTenths;
		m_bDirty  = TTRUE;
	}

	if ( m_bDirty != TTRUE )
		return;

	if ( m_pFill != TNULL && !m_pFill->GetTimelineControl()->IsPlaying() )
	{
		if ( m_bFill )
			m_pFill->PlayAnimation( L"Fill", L"EndFill" );
		else
			m_pFill->PlayAnimation( L"Empty", L"EndEmpty" );

		m_bFill = !m_bFill;
	}

	if ( m_pBar != TNULL )
	{
		AGameStatus*                        pGameStatus = AGameStatus::GetSingleton();
		ALevelInformation::LevelProperties* pProps      = pGameStatus->GetLevelProperties();
		TFLOAT                              fTotal;
		TFLOAT                              fLeft;
		TBOOL                               bSprintBar = TTRUE;

		if ( pProps->m_sprintLevel != 0 )
		{
			fTotal = pProps->m_afTimeLimits[ 0 ] * 60.0f;
			fLeft  = pGameStatus->GetTimeLeft();
		}
		else if ( !m_bChallengeTimer )
		{
			m_pBar->GetTimelineControl()->SetTimeLineTime( ( 1.0f - TFLOAT( a_iSeconds ) / 60.0f ) * ( m_fBarEnd - m_fBarStart ) + m_fBarStart, TTRUE );
			bSprintBar = TFALSE;
		}
		else
		{
			AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();

			if ( pChallenge != TNULL )
			{
				fLeft  = pChallenge->GetTimeLeft();
				fTotal = TFLOAT( pChallenge->GetData()->m_iTimeLimit );
			}
			else
				bSprintBar = TFALSE;
		}

		if ( bSprintBar )
			m_pBar->GetTimelineControl()->SetTimeLineTime( ( 1.0f - fLeft / fTotal ) * ( m_fBarEnd - m_fSprintStart ) + m_fSprintStart, TTRUE );
	}

	if ( m_iMinutes != 0 )
		TStringManager::String16Format( m_wszTime, 32, ( m_iSeconds < 10 ) ? L"%d:0%d" : L"%d:%d", m_iMinutes, m_iSeconds );
	else
		TStringManager::String16Format( m_wszTime, 32, ( m_iSeconds < 10 ) ? L"0:0%d" : L"0:%d", m_iSeconds );

	SetPresenterText( m_wszTime, 0, -1 );
}

// $deBlob: FUNCTION 0065b3b0
void ABlobTimerHud::ResetWarning()
{
	TFLOAT fTimeLeft;
	TBOOL  bTimeup = TFALSE;

	if ( m_bChallengeTimer )
	{
		AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();

		if ( pChallenge == TNULL )
			bTimeup = TTRUE;
		else
			fTimeLeft = pChallenge->GetTimeLeft();
	}
	else
		fTimeLeft = AGameStatus::GetSingleton()->GetTimeLeft();

	if ( !bTimeup )
	{
		TINT iMinutes = TINT( fTimeLeft / 60.0f );
		TINT iSeconds = TINT( TFLOAT( fmod( fTimeLeft, 60.0 ) ) );

		if ( iMinutes == 0 && iSeconds == 0 )
			bTimeup = TTRUE;
		else if ( iMinutes == 0 && iSeconds < 5 )
		{
			m_pVisual->PlayAnimation( L"TimeVeryLow", L"EndTimeVeryLow" );
			m_eWarning = Warning_VeryLow;
		}
		else if ( iMinutes == 0 && iSeconds < 15 )
		{
			m_pVisual->PlayAnimation( L"TimeLow", L"EndTimeLow" );
			m_eWarning = Warning_Low;
		}
		else
		{
			m_pVisual->PlayAnimation( 0, 1 );
			m_eWarning = Warning_None;
		}
	}

	if ( bTimeup )
	{
		m_pVisual->PlayAnimation( L"Timeup", L"EndTimeup" );
		m_eWarning = Warning_Timeup;
	}

	PlayAnimation( L"Normal", L"EndNormal" );

	if ( m_pTimeup != TNULL )
		m_pTimeup->GetTimelineControl()->ResetAnimation();
}

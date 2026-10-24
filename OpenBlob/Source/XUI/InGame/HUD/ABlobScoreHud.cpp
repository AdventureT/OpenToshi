#include "pch.h"
#include "ABlobScoreHud.h"
#include "ABlobGoalHud.h"
#include "ABlobHud.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerManager.h"
#include "Cheats/ACheatManager.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The time between two steps of the counter
static TCONSTEXPR_VAR TFLOAT s_fCountInterval = 0.01f;
// The speed the queued bonuses scroll up at
static TCONSTEXPR_VAR TFLOAT s_fBonusScrollSpeed = 250.0f;
// The counter steps down by difference (<= 1000, <= 5000, more)
static TCONSTEXPR_VAR TINT s_aiCountDownSteps[ 3 ] = { 10, 100, 500 };
// The counter steps up by difference
static TCONSTEXPR_VAR TINT s_aiCountUpSteps[ 3 ] = { 10, 100, 500 };

// $deBlob: FUNCTION 00658a80
// $deBlob: FUNCTION 00658a90
// $deBlob: FUNCTION 00658b00
// $deBlob: FUNCTION 00658b70
// $deBlob: FUNCTION 00658b80
// $deBlob: FUNCTION 0053e500
// $deBlob: FUNCTION 00858cb0
TDEFINE_CLASS( ABlobScoreHud );

// $deBlob: FUNCTION 006597a0
ABlobScoreHud::ABlobScoreHud()
    : m_uiPlayer( 0 )
    , m_iDisplayedScore( -1 )
    , m_iScore( -1 )
    , m_fCountTime( 0.0f )
    , m_fBonusDelay( -1.0f )
    , m_bCountingDown( TFALSE )
    , m_pColourEnergy( TNULL )
    , m_fSpinTime( 0.0f )
    , m_bSpinning( TFALSE )
{
}

// $deBlob: FUNCTION 00658b90
// $deBlob: FUNCTION 00659a20
// $deBlob: FUNCTION 00659a11
ABlobScoreHud::~ABlobScoreHud()
{
	m_QueuedBonuses.Clear();
	m_FreeBonuses.Clear();
}

// $deBlob: FUNCTION 00658c40
void ABlobScoreHud::OnCreated()
{
	if ( AGameStatus::GetSingleton()->GetLevelMode() == 2 )
		SetVisible( TFALSE );

	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
	{
		for ( TINT i = 0; i < MAX_BONUSES; i++ )
		{
			m_aBonuses[ i ].m_pControl = FindChildXUIControl( i + 161 );

			if ( i == 0 )
				m_fBonusY = m_aBonuses[ 0 ].m_pControl->GetPositionY();

			m_aBonuses[ i ].m_iPoints = -1;
			m_FreeBonuses.PushBack( &m_aBonuses[ i ] );
		}
	}

	m_pColourEnergy = FindChildXUIControl( L"ColourEnergy" );
}

// $deBlob: FUNCTION 00659240
void ABlobScoreHud::Reset()
{
	m_iScore          = 0;
	m_iDisplayedScore = 0;
	m_fCountTime      = 0.0f;

	m_pVisual->StopAnimation();
	m_pVisual->PlayAnimation( L"Normal", L"EndNormal" );

	TStringManager::String16Format( m_wszScore, 0x10, L"%d", m_iDisplayedScore );
	SetPresenterText( m_wszScore, 0, -1 );

	m_QueuedBonuses.Clear();
	m_FreeBonuses.Clear();

	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
	{
		for ( TINT i = 0; i < MAX_BONUSES; i++ )
		{
			m_FreeBonuses.PushBack( &m_aBonuses[ i ] );
			m_aBonuses[ i ].m_pControl->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );
		}
	}

	m_fBonusDelay = -1.0f;
	m_eState      = State_Idle;
}

// $deBlob: FUNCTION 00659340
void ABlobScoreHud::AddScore( TINT a_iPoints, TINT a_iBonus, TBOOL a_bCombo, TFLOAT a_fDelay )
{
	m_fSpinTime = 1.0f;

	if ( !m_bSpinning && !m_pColourEnergy->GetVisual()->GetTimelineControl()->IsPlaying() )
	{
		m_bSpinning = TTRUE;
		m_pColourEnergy->GetVisual()->PlayAnimation( L"SpinUp", L"EndSpinUp" );
	}

	AGameStatus::GetSingleton()->GetLevelData().AddPoints( m_uiPlayer, a_iPoints );

	if ( ABlobHud::GetSingleton()->GetGoalHud() != TNULL )
		TSTATICCAST( ABlobGoalHud, ABlobHud::GetSingleton()->GetGoalHud() )->PlayPulse();

	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 && ABlobHud::GetSingleton() != TNULL )
	{
		ABlobHud::GetSingleton()->SetPendingShowcase( ABlobHud::SHOWCASE_Energy );
		ABlobHud::GetSingleton()->SetPendingShowcase( ABlobHud::SHOWCASE_Gate );
	}

	if ( a_iBonus <= 0 )
		return;

	if ( m_FreeBonuses.IsEmpty() )
		CollectBonus();

	Bonus* pBonus = m_FreeBonuses.Head();
	pBonus->Remove();

	if ( m_QueuedBonuses.IsEmpty() )
	{
		pBonus->m_pControl->SetPosition( pBonus->m_pControl->GetPositionX(), m_fBonusY );
	}
	else
	{
		TXUIControl* pLast = m_QueuedBonuses.Tail()->m_pControl;
		pBonus->m_pControl->SetPosition( pLast->GetPositionX(), pLast->GetHeight() * 1.25f + pLast->GetPositionY() );
	}

	pBonus->m_iPoints = a_iBonus;
	TStringManager::String16Format( pBonus->m_wszText, 8, L"%d", a_iBonus );
	pBonus->m_pControl->SetText( pBonus->m_wszText, -1 );
	pBonus->m_pControl->GetVisual()->PlayAnimation( L"Show", L"EndShow" );
	pBonus->m_bUnk10 = TFALSE;

	if ( a_bCombo )
	{
		for ( auto it = m_QueuedBonuses.Begin(); it != m_QueuedBonuses.End(); it++ )
			it->m_bUnk10 = TFALSE;
	}

	m_QueuedBonuses.PushBack( pBonus );

	if ( m_eState != State_Collecting )
		m_fBonusDelay = a_fDelay;
}

// $deBlob: FUNCTION 00659660
void ABlobScoreHud::CollectBonus()
{
	if ( m_QueuedBonuses.IsEmpty() )
	{
		m_fBonusDelay = -1.0f;
		return;
	}

	Bonus* pBonus = m_QueuedBonuses.Head();
	pBonus->Remove();

	AGameStatus::GetSingleton()->GetLevelData().AddPoints( m_uiPlayer, pBonus->m_iPoints );
	pBonus->m_pControl->GetVisual()->PlayAnimation( L"Accumulate", L"EndAccumulate" );
	m_FreeBonuses.PushBack( pBonus );

	TINT iNumQueued = m_QueuedBonuses.Size();
	m_fBonusDelay   = -1.0f;

	if ( iNumQueued > 0 )
		m_eState = State_Collecting;
}

// $deBlob: FUNCTION 00658cf0
void ABlobScoreHud::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bSpinning )
	{
		m_fSpinTime -= a_fDeltaTime;

		if ( m_fSpinTime < 0.0f )
			m_fSpinTime = 0.0f;
	}

	if ( m_fBonusDelay > 0.0f && m_eState != State_Collecting )
	{
		m_fBonusDelay -= a_fDeltaTime;

		if ( m_fBonusDelay < 0.0f )
			CollectBonus();
	}

	if ( m_eState == State_Collecting && !m_QueuedBonuses.IsEmpty() )
	{
		TFLOAT fScroll = s_fBonusScrollSpeed * a_fDeltaTime;

		for ( auto it = m_QueuedBonuses.Begin(); it != m_QueuedBonuses.End(); it++ )
			it->m_pControl->SetPosition( it->m_pControl->GetPositionX(), it->m_pControl->GetPositionY() - fScroll );

		Bonus* pFirst  = m_QueuedBonuses.Begin();
		TFLOAT fFirstY = pFirst->m_pControl->GetPositionY();

		if ( m_fBonusY > fFirstY )
		{
			TFLOAT fOffset = m_fBonusY - fFirstY;

			for ( auto it = m_QueuedBonuses.Begin(); it != m_QueuedBonuses.End(); it++ )
				it->m_pControl->SetPosition( it->m_pControl->GetPositionX(), it->m_pControl->GetPositionY() + fOffset );

			AGameStatus::GetSingleton()->GetLevelData().AddPoints( m_uiPlayer, pFirst->m_iPoints );
			pFirst->m_pControl->GetVisual()->PlayAnimation( L"Accumulate", L"EndAccumulate" );

			TASSERT( !m_QueuedBonuses.IsEmpty() );
			m_QueuedBonuses.Begin()->Remove();
			m_FreeBonuses.PushBack( pFirst );

			if ( m_QueuedBonuses.IsEmpty() || pFirst->m_bUnk10 )
				m_eState = State_Idle;
		}
	}

	if ( IsVisible() && AGameStateController::GetSingleton()->GetCurrentGameState()->IsA( &TGetClass( AInGameState ) ) )
	{
		TINT iScore = AGameStatus::GetSingleton()->GetLevelData().GetPoints( m_uiPlayer );

		if ( iScore != m_iScore )
		{
			TINT iOldScore = m_iScore;
			m_iScore       = iScore;

			// The score cheat: the counter shows the score at once
			if ( ACheatManager::GetSingleton()->IsScoreJumpPending() )
			{
				m_iDisplayedScore = iScore;
				TStringManager::String16Format( m_wszScore, 0x10, L"%d", iScore );
				SetPresenterText( m_wszScore, 0, -1 );
				ACheatManager::GetSingleton()->ClearScoreJumpPending();
				m_bCountingDown = TFALSE;
			}
			else
			{
				m_fCountTime = 0.0f;
				m_pVisual->StopAnimation();

				if ( iScore < iOldScore )
				{
					m_pVisual->PlayAnimation( L"PulseUpBad", L"EndPulseUpBad" );
					m_bCountingDown = TTRUE;
				}
				else
				{
					m_pVisual->PlayAnimation( L"PulseUp", L"EndPulseUp" );
					m_bCountingDown = TFALSE;
				}
			}
		}

		if ( m_bCountingDown )
		{
			if ( m_iScore < m_iDisplayedScore )
			{
				m_fCountTime += a_fDeltaTime;

				if ( m_fCountTime >= s_fCountInterval )
				{
					m_fCountTime = 0.0f;

					TINT iDiff = m_iDisplayedScore - m_iScore;
					TINT iStep = ( iDiff > 5000 ) ? s_aiCountDownSteps[ 2 ] : ( iDiff > 1000 ) ? s_aiCountDownSteps[ 1 ] :
					                                                                             s_aiCountDownSteps[ 0 ];

					m_iDisplayedScore -= iStep;

					if ( m_iDisplayedScore < m_iScore )
						m_iDisplayedScore = m_iScore;

					TStringManager::String16Format( m_wszScore, 0x10, L"%d", m_iDisplayedScore );
					SetPresenterText( m_wszScore, 0, -1 );

					if ( m_iDisplayedScore == m_iScore )
					{
						m_pVisual->PlayAnimation( L"PulseDownBad", L"EndPulseDownBad" );
						m_bCountingDown = TFALSE;
					}
				}
			}
		}
		else if ( m_iScore > m_iDisplayedScore )
		{
			m_fCountTime += a_fDeltaTime;

			if ( m_fCountTime >= s_fCountInterval )
			{
				m_fCountTime = 0.0f;

				TINT iDiff = m_iScore - m_iDisplayedScore;
				TINT iStep = ( iDiff > 5000 ) ? s_aiCountUpSteps[ 2 ] : ( iDiff > 1000 ) ? s_aiCountUpSteps[ 1 ] :
				                                                                           s_aiCountUpSteps[ 0 ];

				m_iDisplayedScore += iStep;

				if ( m_iDisplayedScore > m_iScore )
					m_iDisplayedScore = m_iScore;

				TStringManager::String16Format( m_wszScore, 0x10, L"%d", m_iDisplayedScore );
				SetPresenterText( m_wszScore, 0, -1 );

				if ( m_iDisplayedScore == m_iScore )
					m_pVisual->PlayAnimation( L"PulseDown", L"EndPulseDown" );
			}
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00659720
TBOOL ABlobScoreHud::SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam )
{
	if ( !m_bSpinning || m_fSpinTime > 0.0f || a_eMessage != XUIMessage_NamedFrame || TStringManager::String16Compare( a_wszParam, L"EndSpinUp" ) != 0 )
		return TXUIControl::SendMessage( a_eMessage, a_wszParam );

	m_bSpinning = TFALSE;
	m_pColourEnergy->GetVisual()->PlayAnimation( L"SpinDown", L"EndSpinDown" );
	return TTRUE;
}

#include "pch.h"
#include "ABlobGoalHud.h"
#include "ABlobHud.h"
#include "GameController/AGameStatus.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00650660
// $deBlob: FUNCTION 00650670
// $deBlob: FUNCTION 006506e0
// $deBlob: FUNCTION 00650750
// $deBlob: FUNCTION 00650760
// $deBlob: FUNCTION 0053e0d0
// $deBlob: FUNCTION 00858bb0
TDEFINE_CLASS( ABlobGoalHud );

// $deBlob: FUNCTION 00650770
ABlobGoalHud::ABlobGoalHud()
    : m_pBarA( TNULL )
    , m_pBarB( TNULL )
    , m_pBonusA( TNULL )
    , m_pBonusB( TNULL )
    , m_pUnlock( TNULL )
    , m_pPulse( TNULL )
    , m_pDisplay( TNULL )
    , m_bDisplayed( TFALSE ) // NOTE: retail's ctor doesn't set it; the memory is initialised here
    , m_iState( STATE_Progress )
    , m_fStartA( 0.0f )
    , m_fEndA( 0.0f )
    , m_fStartB( 0.0f )
    , m_fEndB( 0.0f )
    , m_fProgressA( 0.0f )
    , m_fProgressB( 0.0f )
    , m_fMaxA( 0.0f )
    , m_fMaxB( 0.0f )
    , m_iBonusLevel( 0 )
    , m_iNumGoals( 0 )
    , m_pafGoals( TNULL )
    , m_bUnk280( TFALSE )
{
}

// $deBlob: FUNCTION 00651390
// $deBlob: FUNCTION 00650850
// $deBlob: FUNCTION 00651386
ABlobGoalHud::~ABlobGoalHud()
{
	if ( m_pafGoals != TNULL )
		TFree( m_pafGoals );
}

// $deBlob: FUNCTION 00650880
void ABlobGoalHud::OnCreated()
{
	m_pBarA = TSTATICCAST( TXUIScene, FindChildXUIControl( 0x1B ) );
	m_pBarA->StopAnimation();

	m_pBarB = TSTATICCAST( TXUIScene, FindChildXUIControl( 0x23 ) );
	m_pBarB->StopAnimation();

	m_pBonusA  = FindChildXUIElement( 0x1F );
	m_pBonusB  = FindChildXUIElement( 0x24 );
	m_pUnlock  = FindChildXUIElement( 0x42 );
	m_pPulse   = FindChildXUIElement( 0x1C );
	m_pDisplay = FindChildXUIElement( 0x28 );

	m_fStartA = m_pBarA->GetTimelineControl().GetNamedFrameTime( L"0" );
	m_fEndA   = m_pBarA->GetTimelineControl().GetNamedFrameTime( L"100" );
	m_fStartB = m_pBarB->GetTimelineControl().GetNamedFrameTime( L"0" );
	m_fEndB   = m_pBarB->GetTimelineControl().GetNamedFrameTime( L"100" );

	// The goals are the 0-terminated list at LevelProperties+0x5C (up to 10), a pointer to the TRB's list
	const TINT* piGoals = *(const TINT**)&AGameStatus::GetSingleton()->GetLevelProperties()->m_auiGoals[ 0 ];

	m_iNumGoals = 0;
	while ( m_iNumGoals < 10 && piGoals[ m_iNumGoals ] != 0 )
		m_iNumGoals++;

	if ( m_iNumGoals > 0 )
	{
		m_pafGoals = (TFLOAT*)TMemoryHeap_Malloc( Toshi::TXUI::ms_pXUIMemoryBlock, m_iNumGoals * sizeof( TFLOAT ) );

		for ( TINT i = 0; i < m_iNumGoals; i++ )
			m_pafGoals[ i ] = TFLOAT( piGoals[ i ] );
	}
}

// Start + ( end - start ) * a_fProgress
static TFLOAT GoalHudLerp( TFLOAT a_fStart, TFLOAT a_fEnd, TFLOAT a_fProgress )
{
	return ( a_fEnd - a_fStart ) * a_fProgress + a_fStart;
}

// $deBlob: FUNCTION 00650a60
void ABlobGoalHud::Tick( TFLOAT a_fDeltaTime )
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	// The goals are shown while the level has goals to show (the list 0x78C of the level data)
	if ( !m_bDisplayed )
	{
		if ( pGameStatus->GetLevelData().m_oList78C.Size() > 0 )
		{
			m_bDisplayed = TTRUE;
			m_pDisplay->PlayAnimation( L"Display", L"EndDisplay" );
		}
	}
	else if ( pGameStatus->GetLevelData().m_oList78C.Size() == 0 )
	{
		m_bDisplayed = TFALSE;
		m_pDisplay->PlayAnimation( L"Hide", L"EndHide" );
	}

	// The single player level: visible while ABlobHud shows the gate (SHOWCASE_Gate)
	if ( pGameStatus->GetSPLevel() == 0 )
	{
		TFLOAT fOpacity = 0.0f;

		if ( ABlobHud::GetSingleton() != TNULL && ( ABlobHud::GetSingleton()->GetShownShowcases() & ABlobHud::SHOWCASE_Gate ) != 0 )
			fOpacity = 1.0f;

		SetOpacity( fOpacity );
	}

	if ( m_pafGoals == TNULL || m_iState == STATE_Idle )
	{
		TXUIScene::Tick( a_fDeltaTime );
		return;
	}

	TINT   iPoints = pGameStatus->GetLevelData().GetPoints( 0 );
	TINT   iGoal   = TINT( pGameStatus->GetLevelData().m_uiUnk774 );
	TFLOAT fPoints = TFLOAT( iPoints );

	if ( m_iState == STATE_Decay )
	{
		// The bars go back to their start
		TFLOAT fStep = 2.0f * a_fDeltaTime;

		m_fProgressA -= fStep;
		if ( m_fProgressA < 0.0f )
			m_fProgressA = 0.0f;

		m_fProgressB -= fStep;
		if ( m_fProgressB < 0.0f )
			m_fProgressB = 0.0f;

		m_pBarA->GetTimelineControl().SetTimeLineTime( GoalHudLerp( m_fStartA, m_fEndA, m_fProgressA ), TTRUE );
		m_pBarB->GetTimelineControl().SetTimeLineTime( GoalHudLerp( m_fStartB, m_fEndB, m_fProgressB ), TTRUE );

		if ( m_fProgressA != 0.0f || m_fProgressB != 0.0f )
		{
			TXUIScene::Tick( a_fDeltaTime );
			return;
		}

		if ( m_bUnk280 )
		{
			m_iState = STATE_Idle;
			TXUIScene::Tick( a_fDeltaTime );
			return;
		}

		// The score is back above the goal: the bars rise again to the next goal (STATE_Bonus)
		// NOTE: retail reads m_pafGoals[ iGoal - 1 ] and [ iGoal + 1 ], [ iGoal + 2 ] without a bounds check: at the
		// first goal or the last goals it reads outside the array
		m_iState = STATE_Bonus;

		TFLOAT fMaxA = ( fPoints - m_pafGoals[ iGoal - 1 ] ) / ( m_pafGoals[ iGoal ] - m_pafGoals[ iGoal - 1 ] );
		m_fMaxA      = fMaxA;

		if ( fMaxA < 1.0f )
		{
			m_fMaxB       = 0.0f;
			m_iBonusLevel = 0;
		}
		else
		{
			if ( fMaxA > 1.0f )
				m_fMaxA = 1.0f;

			m_fMaxB       = 0.0f;
			m_iBonusLevel = 1;

			if ( iGoal < m_iNumGoals )
			{
				TFLOAT fMaxB = ( fPoints - m_pafGoals[ iGoal ] ) / ( m_pafGoals[ iGoal + 1 ] - m_pafGoals[ iGoal ] );
				m_fMaxB      = fMaxB;

				if ( fMaxB <= 1.0f )
				{
					m_iBonusLevel = 1;
				}
				else
				{
					m_fMaxB       = 1.0f;
					m_iBonusLevel = 2;

					if ( iGoal < m_iNumGoals - 2 )
					{
						fMaxB   = ( fPoints - m_pafGoals[ iGoal + 1 ] ) / ( m_pafGoals[ iGoal + 2 ] - m_pafGoals[ iGoal + 1 ] );
						m_fMaxB = fMaxB;

						if ( fMaxB > 1.0f )
						{
							m_fMaxB       = 1.0f;
							m_iBonusLevel = 3;
						}
					}
				}
			}
		}
	}
	else if ( m_iState == STATE_Bonus )
	{
		// The bonus rises to its targets
		TFLOAT fStep = 2.0f * a_fDeltaTime;

		m_fProgressA += fStep;
		if ( m_fMaxA < m_fProgressA )
			m_fProgressA = m_fMaxA;

		m_fProgressB += fStep;
		if ( m_fMaxB < m_fProgressB )
			m_fProgressB = m_fMaxB;

		m_pBarA->GetTimelineControl().SetTimeLineTime( GoalHudLerp( m_fStartA, m_fEndA, m_fProgressA ), TTRUE );
		m_pBarB->GetTimelineControl().SetTimeLineTime( GoalHudLerp( m_fStartB, m_fEndB, m_fProgressB ), TTRUE );

		if ( m_fProgressA == m_fMaxA && m_fProgressB == m_fMaxB )
			m_iState = STATE_Progress;

		TXUIScene::Tick( a_fDeltaTime );
		return;
	}
	else if ( m_iState == STATE_Progress )
	{
		// The score fills the bars up to the goals; each goal reached plays its bonus level
		TFLOAT fProgress;

		if ( iGoal == 0 )
			fProgress = fPoints / m_pafGoals[ 0 ];
		else
			fProgress = ( fPoints - m_pafGoals[ iGoal - 1 ] ) / ( m_pafGoals[ iGoal ] - m_pafGoals[ iGoal - 1 ] );

		m_fProgressA = fProgress;

		if ( fProgress < 1.0f )
		{
			m_pBonusA->PlayAnimation( L"Normal", L"EndNormal" );
			m_fProgressB = 0.0f;
		}
		else
		{
			if ( fProgress > 1.0f )
				m_fProgressA = 1.0f;

			m_fProgressB = 0.0f;
			m_pBonusA->PlayAnimation( L"Normal", L"EndNormal" );

			if ( m_iBonusLevel < 1 )
			{
				PlayAnimation( L"GateUnlocked", L"EndGateUnlocked" );
				m_pUnlock->PlayAnimation( L"Unlocked", L"Unlocked" );
				m_iBonusLevel = 1;
			}

			m_pBonusB->PlayAnimation( L"Normal", L"EndNormal" );

			if ( iGoal < m_iNumGoals - 1 )
			{
				fProgress    = ( fPoints - m_pafGoals[ iGoal ] ) / ( m_pafGoals[ iGoal + 1 ] - m_pafGoals[ iGoal ] );
				m_fProgressB = fProgress;

				if ( fProgress > 1.0f )
				{
					m_fProgressB = 1.0f;

					if ( m_iBonusLevel < 2 )
						m_iBonusLevel = 2;

					m_pBonusA->PlayAnimation( L"BonusLevel1", L"BonusLevel1" );
					m_pBonusB->PlayAnimation( L"BonusLevel1", L"BonusLevel1" );

					if ( iGoal < m_iNumGoals - 2 )
					{
						fProgress    = ( fPoints - m_pafGoals[ iGoal + 1 ] ) / ( m_pafGoals[ iGoal + 2 ] - m_pafGoals[ iGoal + 1 ] );
						m_fProgressB = fProgress;

						if ( fProgress > 1.0f )
						{
							m_fProgressB = 1.0f;

							if ( iGoal < m_iNumGoals - 3 )
							{
								fProgress    = ( fPoints - m_pafGoals[ iGoal + 2 ] ) / ( m_pafGoals[ iGoal + 3 ] - m_pafGoals[ iGoal + 2 ] );
								m_fProgressB = fProgress;

								if ( fProgress > 1.0f )
									m_fProgressB = 1.0f;
							}

							if ( m_iBonusLevel < 3 )
								m_iBonusLevel = 3;

							m_pBonusA->PlayAnimation( L"BonusLevel2", L"BonusLevel2" );
							m_pBonusB->PlayAnimation( L"BonusLevel2", L"BonusLevel2" );
						}
					}
				}
			}
		}

		m_pBarA->GetTimelineControl().SetTimeLineTime( GoalHudLerp( m_fStartA, m_fEndA, m_fProgressA ), TTRUE );
		m_pBarB->GetTimelineControl().SetTimeLineTime( GoalHudLerp( m_fStartB, m_fEndB, m_fProgressB ), TTRUE );

		TXUIScene::Tick( a_fDeltaTime );
		return;
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006509f0
void ABlobGoalHud::SetGoalCount( TINT a_iCount )
{
	if ( a_iCount == -1 )
	{
		SetVisible( TFALSE );
		return;
	}

	SetVisible( TTRUE );

	// NOTE: retail compares the level mode with 2 here (the level mode enum has 0 and 1 only)
	const TWCHAR* wszFormat = AGameStatus::GetSingleton()->GetLevelMode() == 2 ? L"%d%%" : L"%d";
	TStringManager::String16Format( m_wszCount, TARRAYSIZE( m_wszCount ), wszFormat, a_iCount );

	SetPresenterText( m_wszCount, 0, -1 );
}

// $deBlob: FUNCTION 00651240
void ABlobGoalHud::StartDecay()
{
	m_iState = STATE_Decay;

	PlayAnimation( L"Normal", L"EndNormal" );

	if ( m_iBonusLevel < 2 )
		m_pUnlock->PlayAnimation( L"Normal", L"EndNormal" );
}

// $deBlob: FUNCTION 00651280
void ABlobGoalHud::Reset()
{
	m_iState      = STATE_Progress;
	m_iBonusLevel = 0;
	m_fProgressA  = 0.0f;
	m_fProgressB  = 0.0f;
	m_fMaxA       = 0.0f;
	m_fMaxB       = 0.0f;
	m_bUnk280     = TFALSE;

	PlayAnimation( L"Normal", L"EndNormal" );
	m_pUnlock->PlayAnimation( L"Normal", L"EndNormal" );

	m_pBarA->GetTimelineControl().SetTimeLineTime( m_fStartA, TTRUE );
	m_pBarB->GetTimelineControl().SetTimeLineTime( m_fStartB, TTRUE );

	m_pBonusA->PlayAnimation( L"Normal", L"EndNormal" );
	m_pBonusB->PlayAnimation( L"Normal", L"EndNormal" );
	m_pDisplay->PlayAnimation( L"Normal", L"EndNormal" );
}

// $deBlob: FUNCTION 00651370
void ABlobGoalHud::PlayPulse()
{
	m_pPulse->PlayAnimation( L"Pulse", L"EndPulse" );
}

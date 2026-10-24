#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the goals counter of the single player HUD (ABlobHud's element 0x19, AGameStatus::LevelData+0x7B0;
// scene "GoalHud"). It shows the goal count, runs two bars (controls 0x1B and 0x23, each one
// a timeline from frame "0" to "100") that follow the score through the level's goal thresholds, and plays the
// Normal/BonusLevel1/BonusLevel2/Unlocked/Display/Hide animations. The thresholds are the level's list at
// LevelProperties+0x5C (a 0-terminated list of up to 10 ints), copied as floats into a heap array.
//-----------------------------------------------------------------------------
class ABlobGoalHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobGoalHud, Toshi::TXUIScene );

	// m_iState
	enum STATE : TINT
	{
		STATE_Idle     = 0,
		STATE_Progress = 1, // the score fills the bars
		STATE_Decay    = 2, // the bars go back to their start
		STATE_Bonus    = 3, // the bonus rises to its targets
	};

public:
	ABlobGoalHud();
	~ABlobGoalHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void OnCreated() override;                 // +0x94

	// Shows the goal count (a_iCount -1: hides the HUD); the level mode 2 shows it as a percentage (retail's compare)
	void SetGoalCount( TINT a_iCount );

	void SetState( TINT a_iState ) { m_iState = a_iState; }
	void SetUnk280( TBOOL a_bUnk280 ) { m_bUnk280 = a_bUnk280; }
	// $deBlob: FUNCTION 00646d20
	TBOOL GetUnk280() const { return m_bUnk280; }

	// The score is going down: the bars go back to their start (STATE_Decay)
	void StartDecay();

	// Back to the start of the level (STATE_Progress)
	void Reset();

	// Plays the "Pulse" animation (the goal was reached, ABlobScoreHud calls it on each score change)
	void PlayPulse();

private:
	TWCHAR              m_wszCount[ 128 ]; // 0x130 the goal count text (SetGoalCount)
	Toshi::TXUIScene*   m_pBarA;           // 0x230 control 0x1B (its timeline 0x98 is bar A)
	Toshi::TXUIScene*   m_pBarB;           // 0x234 control 0x23 (its timeline 0x98 is bar B)
	Toshi::TXUIElement* m_pBonusA;         // 0x238 element 0x1F ("Normal", "BonusLevel1/2")
	Toshi::TXUIElement* m_pBonusB;         // 0x23C element 0x24 ("Normal", "BonusLevel1/2")
	Toshi::TXUIElement* m_pUnlock;         // 0x240 element 0x42 ("Normal", "Unlocked")
	Toshi::TXUIElement* m_pPulse;          // 0x244 element 0x1C ("Pulse", "EndPulse")
	Toshi::TXUIElement* m_pDisplay;        // 0x248 element 0x28 ("Display", "Hide", "Normal")
	TBOOL               m_bDisplayed;      // 0x24C shown while the goal list isn't empty
	TINT                m_iState;          // 0x250 STATE
	TFLOAT              m_fStartA;         // 0x254 bar A's timeline at "0"
	TFLOAT              m_fEndA;           // 0x258 bar A's timeline at "100"
	TFLOAT              m_fStartB;         // 0x25C bar B's timeline at "0"
	TFLOAT              m_fEndB;           // 0x260 bar B's timeline at "100"
	TFLOAT              m_fProgressA;      // 0x264 0..1
	TFLOAT              m_fProgressB;      // 0x268 0..1
	TFLOAT              m_fMaxA;           // 0x26C the bonus rise's target of bar A
	TFLOAT              m_fMaxB;           // 0x270 the bonus rise's target of bar B
	TINT                m_iBonusLevel;     // 0x274 0..3: the bonus levels reached
	TINT                m_iNumGoals;       // 0x278 the goals of the level (up to 10)
	TFLOAT*             m_pafGoals;        // 0x27C the goals as floats (TMemoryHeap, ms_pXUIMemoryBlock)
	TBOOL               m_bUnk280;         // 0x280 set by the game (the rise ends at STATE_Idle)
};

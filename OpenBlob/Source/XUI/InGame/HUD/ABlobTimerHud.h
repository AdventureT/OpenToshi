#pragma once
#include <XUI/TXUIScene.h>
#include <XUI/TXUIGroup.h>

//-----------------------------------------------------------------------------
// Purpose: the countdown of the single player HUD (ABlobHud's control 0x9E, GetChallengeTimerControl) and of the challenge
// timer (scene "ChallengeTimerHud"): shows m:ss, flashes its "Fill"/"Empty" element on each change, moves the
// sprint bar timeline and plays the TimeLow/TimeVeryLow/Timeup animations.
//-----------------------------------------------------------------------------
class ABlobTimerHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobTimerHud, Toshi::TXUIScene );

	enum Warning
	{
		Warning_None    = 0,
		Warning_Low     = 1,
		Warning_VeryLow = 2,
		Warning_Timeup  = 3,
	};

public:
	ABlobTimerHud();
	~ABlobTimerHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void OnCreated() override;                 // +0x94

	// Sets the displayed time (a_iTenths -1: unchanged)
	void SetTime( TINT a_iMinutes, TINT a_iSeconds, TINT a_iTenths );

	// Picks the warning animation from the current time left and restarts the normal animation
	void ResetWarning();

private:
	TBOOL             m_bDirty;          // 0x130
	TINT              m_iMinutes;        // 0x134
	TINT              m_iSeconds;        // 0x138
	TINT              m_iTenths;         // 0x13C
	TWCHAR            m_wszTime[ 32 ];   // 0x140
	Toshi::TXUIGroup* m_pFill;           // 0x180 element 6
	TBOOL             m_bFill;           // 0x184
	TBOOL             m_bChallengeTimer; // 0x185 the scene is "ChallengeTimerHud"
	Toshi::TXUIGroup* m_pBar;            // 0x188 element 7
	Toshi::TXUIGroup* m_pTimeup;         // 0x18C element 15
	TFLOAT            m_fBarStart;       // 0x190
	TFLOAT            m_fSprintStart;    // 0x194 "SprintStart"
	TFLOAT            m_fBarEnd;         // 0x198 "End"
	TINT              m_eWarning;        // 0x19C Warning
};

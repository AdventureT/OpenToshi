#pragma once
#include "XUI/ABlobMenu.h"

class ABlobRewardScreen;

//-----------------------------------------------------------------------------
// Purpose: the results of a sprint level (sprintsummarymenu.xur): the time, the counter of the level's type, the
// medal; the results are recorded in the profile in the first
// Tick and saved when the "EndSprintsSum" frame is reached; continue shows the reward screen (if anything was
// unlocked) or the sprint game over screen (sprintgameover.xur).
//-----------------------------------------------------------------------------
class ABlobSprintSummaryMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobSprintSummaryMenu, ABlobMenu );

	// The names of the sprint levels
	static const TINT sm_aiSprintLevelNames[ 20 ];

public:
	ABlobSprintSummaryMenu();
	~ABlobSprintSummaryMenu();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam ) override;    // +0x78
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void EndTransBackTo() override; // +0x12C

	// Formats a time as "m:ss.t"
	static void __stdcall FormatTime( TINT a_iMinutes, TINT a_iSeconds, TINT a_iTenths, TWCHAR* a_wszBuffer, TINT a_iSize );

private:
	// Records the results in the current profile
	void RecordResults();

private:
	TWCHAR              m_wszTime[ 32 ];    // 0x140
	TWCHAR              m_wszCounter[ 32 ]; // 0x180
	TWCHAR              m_wszUnk1C0[ 32 ];  // 0x1C0 not used
	Toshi::TXUIControl* m_pContinue;        // 0x200 control 3
	ABlobRewardScreen*  m_pRewardScreen;    // 0x204
	Toshi::TXUIControl* m_pNewMedal;        // 0x208 control 14
	TBOOL               m_bRecordPending;   // 0x20C the results are recorded by the next Tick
	TBOOL               m_bUnk20D;          // 0x20D never set
	Toshi::TXUIControl* m_pTitle;           // 0x210 control 16
	TBOOL               m_bWaitForFade;     // 0x214
};

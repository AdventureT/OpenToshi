#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the help menu of the pause menu: each of its eight buttons
// opens the full screen help (ABlobHelpFullScreen) of a topic, streamed by AInGameState.
//-----------------------------------------------------------------------------
class ABlobHelp : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobHelp, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_TOPIC_BUTTONS = 8;

public:
	ABlobHelp();
	~ABlobHelp();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void EndTransBackTo() override; // +0x12C

private:
	// The full screen help topic of each button
	static const TINT ms_aiButtonTopics[ NUM_TOPIC_BUTTONS ];

private:
	Toshi::TXUIControl* m_apTopicButtons[ NUM_TOPIC_BUTTONS ]; // 0x140 controls 12..5
};

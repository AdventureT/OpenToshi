#pragma once
#include <XUI/TXUIScene.h>

class APropTriggerHelp;
class ABlobTextReveal;

TOSHI_NAMESPACE_START
class TXUITabScene;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: the full screen help: a scene loaded from its
// own TRB (data/xui/<locale>/<topic>.trb, "<topic>.xur") by APropTriggerHelp (help type 1) or by the pause menu's
// help menu (ABlobHelp). Topic 0 (helpfullscreen) shows one message (SetMessage), the other topics show the pages of
// their tab scene (SetTopic) with the Next/Prev buttons. The game is paused while it's shown (Pause/Unpause).
//-----------------------------------------------------------------------------
class ABlobHelpFullScreen : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobHelpFullScreen, Toshi::TXUIScene );

	// The topics up to this one end on a confirm page: Next closes the help, the button only turns the pages
	static TCONSTEXPR_VAR TINT NUM_CONFIRM_TOPICS = 11;

	// The pages of a topic (the strings of the tabs)
	struct PageStrings
	{
		const TINT* m_piStrings;
		TINT        m_iNumPages;
	};

	static TCONSTEXPR_VAR TINT NUM_TOPICS = 18;

public:
	ABlobHelpFullScreen();
	~ABlobHelpFullScreen();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                 // +0x04
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0
	virtual TBOOL OnNotifySetFocus( TXUIElement* a_pSource ) override;                  // +0xB4
	virtual TBOOL OnNotifyValueChanged( TXUIElement* a_pSource ) override;              // +0xBC

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;          // +0x114
	virtual void TransBackFrom() override;    // +0x120
	virtual void EndTransTo() override;       // +0x124
	virtual void EndTransBackFrom() override; // +0x130

	//-----------------------------------------------------------------------------
	// ABlobHelpFullScreen
	//-----------------------------------------------------------------------------
	// The trigger that showed the help (its targets are triggered once the help is closed), TNULL from the help menu
	virtual void SetTrigger( APropTriggerHelp* a_pTrigger ); // +0x134

	// Shows a message (the scene "HelpFullscreen", control 4)
	void SetMessage( const TWCHAR* a_wszMessage );

	// Shows the pages of a_iTopic and marks the topic as seen in the profile
	void SetTopic( TINT a_iTopic );

	// Pauses the game for the help (AInGameState::OnUpdate, before the TRB is streamed)
	static void Pause();

	// Unpauses the game (the help is deleted over the HUD)
	static void Unpause();

private:
	// Closes the help with a fade to black over the letter box HUD (Tick transitions back once it's black)
	void CloseWithFade();

private:
	static const PageStrings paPageStrings[ NUM_TOPICS ];

private:
	APropTriggerHelp*    m_pTrigger;       // 0x130
	ABlobTextReveal*     m_pMessage;       // 0x134 "Message"
	Toshi::TXUIControl*  m_pContinue;      // 0x138 "Continue"
	Toshi::TXUIControl*  m_pNext;          // 0x13C "Next"
	Toshi::TXUIControl*  m_pPrev;          // 0x140 "Prev"
	Toshi::TXUITabScene* m_pTabScene;      // 0x144 "TabScene"
	Toshi::TXUIElement*  m_pTabBackground; // 0x148 the tab scene's "XuiNineGridColored"
	TBOOL                m_bMessage;       // 0x14C SetMessage was called
	TINT8                m_iTopic;         // 0x14D -1
	TBOOL                m_bClosing;       // 0x14E fading to black before the transition back
};

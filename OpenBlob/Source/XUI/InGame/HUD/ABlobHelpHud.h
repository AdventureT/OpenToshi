#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the in-level help popup of the single player HUD (ABlobHud's control 0x153):
// shows a help text with the face of its speaker ("RevHead1"-"RevHead4") for a while.
// APropTriggerHelp shows it (HelpType 2) and keeps it.
//-----------------------------------------------------------------------------
class ABlobHelpHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobHelpHud, Toshi::TXUIScene );

	static TCONSTEXPR_VAR TINT iMAX = 4;

	// The help text telling the player that the blob is ink (hides once the blob isn't black any more)
	static TCONSTEXPR_VAR TINT INK_HELP_STRING = 0x37C;

public:
	ABlobHelpHud();
	~ABlobHelpHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void OnCreated() override;                 // +0x94

	// Shows the string a_iStringId (or a_wszText) for a_fDisplayTime seconds with the face of a_iSpeaker
	void ShowMessage( TINT a_iStringId, TFLOAT a_fDisplayTime, TINT a_iSpeaker, const TWCHAR* a_wszText );

	// Hides the popup with its "Hide" animation
	void Hide();

	// Hides the popup at once ("EndNormal")
	void HideNow();

private:
	TFLOAT              m_fDisplayTime;       // 0x130 -1: not shown
	TBOOL               m_bInkHelp;           // 0x134 the text is INK_HELP_STRING
	Toshi::TXUIElement* m_apRevHeads[ iMAX ]; // 0x138 "RevHead1"-"RevHead4"
};

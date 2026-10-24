#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the letter box scene (LetterBox.xur) of the cutscenes: the black bars over the screen, the
// help popup of APropTriggerHelp's letter box type (ABlobHelpHud child 4) and the players' bodies, which are taken out
// of the physics while it's up (ABlobEntity::SetLevelStartAttached).
//-----------------------------------------------------------------------------
class ABlobLetterBoxHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobLetterBoxHud, Toshi::TXUIScene );

public:
	ABlobLetterBoxHud();
	~ABlobLetterBoxHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransBackFrom() override;    // +0x120
	virtual void EndTransBackFrom() override; // +0x130

	// Shows or hides the bars (control 5)
	void SetBarsVisible( TBOOL a_bVisible );

	// Plays the "Display" animation of control 10 and of element 35
	void PlayTopDisplay();
	void PlayBottomDisplay();

	static ABlobLetterBoxHud* GetInstance() { return ms_pInstance; }

private:
	static ABlobLetterBoxHud* ms_pInstance;

private:
	Toshi::TXUIControl* m_pBars;   // 0x130 control 5, the bars
	Toshi::TXUIControl* m_pTop;    // 0x134 control 10, played by PlayTopDisplay
	Toshi::TXUIElement* m_pBottom; // 0x138 element 35, played by PlayBottomDisplay
};

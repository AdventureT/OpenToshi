#pragma once
#include <XUI/TXUIScene.h>
#include <XUI/TXUIProgressBar.h>

//-----------------------------------------------------------------------------
// Purpose: the "SHAKE!!" prompt of the single player HUD (ABlobHud's control 0xD9):
// shown with a progress bar while the blob shakes off grapplers (extra spheres),
// a hotplate or a landmark (STATE_HOTPLATE/STATE_LMARK).
//-----------------------------------------------------------------------------
class ABlobShakeHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobShakeHud, Toshi::TXUIScene );

	enum MODE
	{
		MODE_NONE      = -1,
		MODE_GRAPPLERS = 0,
		MODE_HOTPLATE  = 1,
		MODE_LANDMARK  = 2,
	};

public:
	ABlobShakeHud();
	~ABlobShakeHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void OnCreated() override;                 // +0x94

	// Hides the prompt (ABlobHud::Reset)
	void Reset();

	// The hotplate's shake progress (0-100)
	void SetHotplateProgress( TINT a_iProgress );

private:
	// Shows the prompt (the platform's "Shake" glyph and text)
	void ShowPrompt();

private:
	Toshi::TXUIProgressBar* m_pProgress; // 0x130 control 0xDA
	Toshi::TXUIControl*     m_pPrompt;   // 0x134 control 0xDB
	TINT                    m_eMode;     // 0x138 MODE
};

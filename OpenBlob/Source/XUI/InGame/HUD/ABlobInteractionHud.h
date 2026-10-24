#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the interaction prompt of the single player HUD (scenes "InteractionHud2" and "InteractZJumpAttack").
// Its Tick moves it to the point of interaction and plays the Show/Hide animations with the "AButton"/"ZButton"
// prompts (or the swing message of the HUD's scene 0xCD); OnCreated finds its "ScaleGroup" element.
//-----------------------------------------------------------------------------
class ABlobInteractionHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobInteractionHud, Toshi::TXUIScene );

	// The prompt shown (m_iState)
	enum STATE : TINT
	{
		STATE_None       = -1,
		STATE_ZButton    = 1, // "ZButton"
		STATE_WindCannon = 2, // "AButton" of a wind cannon
		STATE_Swing      = 3, // the swing message 0x206
		STATE_SwingLock  = 4, // the swing message 0x1EC (a Z-jump priority lock)
		STATE_Challenge  = 6, // "AButton" of a nearby challenge
	};

public:
	ABlobInteractionHud();
	~ABlobInteractionHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void OnCreated() override;                 // +0x94

private:
	Toshi::TXUIScene* GetSwingScene();
	void              ShowZPrompt();
	void              ShowSwingMessage( TINT a_iNameString, TWCHAR* a_wszBuffer );

private:
	TINT                m_iState;      // 0x130 STATE
	TBOOL               m_bUnk134;     // 0x134 1: "InteractZJumpAttack", 0: "InteractionHud2"
	Toshi::TXUIElement* m_pScaleGroup; // 0x138 element "ScaleGroup" (OnCreated)
	Toshi::TVector4     m_vPosition;   // 0x13C the position the prompt follows (default 0,0,0,1)
	Toshi::TXUIScene*   m_pSwingScene; // 0x14C the ABlobHud child scene 0xCD ("Drum", "raydianswing", "swing_pc", "Message"), found on first use
};

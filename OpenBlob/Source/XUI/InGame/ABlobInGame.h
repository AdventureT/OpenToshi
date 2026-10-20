#pragma once
#include <XUI/TXUIScene.h>

class ABlobCursorTrail;

//-----------------------------------------------------------------------------
// Purpose: the scene that holds the game while a level runs (ingame.xur, "InGame").
// Its child "PauseMenu" (ABlobPauseMenu) is paused and shown when the scene transitions to with Usage_Pause, and
// resumed when the scene transitions back. The scene is created by AInGameState and AInGameStateHelper.
//-----------------------------------------------------------------------------
class ABlobInGame : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobInGame, Toshi::TXUIScene );

	// What the scene is shown for
	enum Usage : TINT
	{
		Usage_None    = -1,
		Usage_Pause   = 0,
		Usage_Results = 1, // the level's results menu (AInGameState::OnUpdate)
	};

public:
	ABlobInGame();
	~ABlobInGame();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	// Hides the world while the scene is visible, shows the cursor trails with it
	virtual void SetVisible( TBOOL a_bVisible ) override; // +0x48

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override;        // +0x94
	virtual void TransTo() override;          // +0x114
	virtual void TransBackFrom() override;    // +0x120
	virtual void EndTransTo() override;       // +0x124
	virtual void EndTransBackFrom() override; // +0x130

	// Would play the looping background movie a_wszMovie behind the post-game menus (ABlobVistasMenu,
	// ABlobRewardScreen); retail's PC build only returns TTRUE.
	TBOOL PlayBackgroundMovie( const TWCHAR* a_wszMovie, TBOOL a_bLoop );

	// The usage of the scene (set by the owner before the transition)
	// $deBlob: FUNCTION 005c8ab0
	void SetUsage( Usage a_eUsage ) { m_eUsage = a_eUsage; }

	// Set by a closed confirmation of the pause menu (its parent is this scene): the pause menu is shown again
	void SetReopenPause( TBOOL a_bReopen ) { m_bReopenPause = a_bReopen; }

	// Deletes the scenes stacked on this one down to the HUD (ABlobHud), the level restart
	void CloseScenesToHud();

	// The cursor trails are put back into this scene (after child 5)
	void AttachCursorTrails();

	// The cursor trails are moved into a_pParent after a_pAfter (the full screen help)
	void AttachCursorTrailsTo( Toshi::TXUIElement* a_pParent, Toshi::TXUIElement* a_pAfter );

	// Shows or hides the cursor trails
	void SetCursorTrailsVisible( TBOOL a_bVisible );

	// The next Tick moves the cursor trails into a_pParent after a_pAfter (the full screen help opened over the help
	// menu)
	static void SetCursorTrailsParentLater( Toshi::TXUIElement* a_pParent, Toshi::TXUIElement* a_pAfter )
	{
		ms_bMoveCursorTrails   = TTRUE;
		ms_pCursorTrailsParent = a_pParent;
		ms_pCursorTrailsAfter  = a_pAfter;
	}

	// The next Tick puts the cursor trails back into this scene
	static void SetRestoreCursorTrails() { ms_bRestoreCursorTrails = TTRUE; }

private:
	static Toshi::TXUIElement* ms_pCursorTrailsAfter;
	static Toshi::TXUIElement* ms_pCursorTrailsParent;
	static TBOOL               ms_bMoveCursorTrails;
	static TBOOL               ms_bRestoreCursorTrails;

	TINT                m_eUsage;              // 0x130 Usage
	Toshi::TXUIElement* m_pUnk134;             // 0x134 child element 5
	ABlobCursorTrail*   m_apCursorTrails[ 4 ]; // 0x138 children 9..6 (the cursor trails of the pause menus)
	TINT                m_iUnk148;             // 0x148
	TUINT8              m_aUnk14C[ 0x100 ];    // 0x14C..0x24B not ported yet
	TINT                m_iUnk24C;             // 0x24C
	TBOOL               m_bReopenPause;        // 0x250 set by a closed confirmation: EndTransBackFrom shows the pause menu again
};

#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the "connect the controller of player N" popup (controllerscreen.xur "ControllerPopup"): the game
// states check every frame that each player still has
// a device (UpdateFrontEnd in the menus, UpdateInGame in a level) and show it while one is missing; while it's shown
// the movie, the game's GUI and the in-game sounds are paused.
//-----------------------------------------------------------------------------
class ABlobControllerPopup : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobControllerPopup, Toshi::TXUIScene );

	enum STATE
	{
		STATE_Hidden,  // EndTransBackFrom
		STATE_TransTo, // TransTo
		STATE_Shown,   // EndTransTo
		STATE_TransBackFrom,
	};

	struct PlayerInfo
	{
		TBOOL m_bHasDevice; // 0x0
		TINT  m_iUnk4;      // 0x4
	};

public:
	ABlobControllerPopup();
	~ABlobControllerPopup();

	// $deBlob: FUNCTION 005c8920
	STATE GetState() const { return m_eState; }

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;          // +0x114
	virtual void TransBackFrom() override;    // +0x120
	virtual void EndTransTo() override;       // +0x124
	virtual void EndTransBackFrom() override; // +0x130

	// Shows the popup while a player of the level has no device (AInGameState only); TFALSE while it's needed
	// NOTE: a_fDeltaTime isn't used
	TBOOL UpdateInGame( TFLOAT a_fDeltaTime );

	// Shows the popup while a player of the menus has no device (not in AInGameState); TFALSE while it's needed
	TBOOL UpdateFrontEnd( TFLOAT a_fDeltaTime );

	void SetDisabled( TBOOL a_bDisabled );
	void SetIgnoreInputLock( TBOOL a_bIgnore );

private:
	// The popup can come and go (no transition running, input not locked unless m_bIgnoreInputLock)
	TBOOL CanTransition() const;

private:
	STATE      m_eState;            // 0x130
	TINT       m_iPlayer;           // 0x134 the player without device, -1
	TINT       m_iUnk138;           // 0x138 -1
	PlayerInfo m_aPlayers[ 4 ];     // 0x13C
	TWCHAR     m_wszMessage[ 128 ]; // 0x15C
	TBOOL      m_bWasInGamePaused;  // 0x25C the in-game state was paused before the popup
	TBOOL      m_bDisabled;         // 0x25D
	TINT       m_iNumHiddenGUIs;    // 0x260 the game's GUI viewports the popup hid
	TINT       m_iUnk264;           // 0x264
	TBOOL      m_bFadeWasEnabled;   // 0x268 viewport 0's fade before the popup
	TFLOAT     m_fFade;             // 0x26C
	TBOOL      m_bPausedMovie;      // 0x270
	TBOOL      m_bCanvasWasVisible; // 0x271
	TBOOL      m_bIgnoreInputLock;  // 0x272
};

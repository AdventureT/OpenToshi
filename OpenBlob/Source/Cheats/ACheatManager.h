#pragma once
#include "Input/AInputHelperSimple.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2Array.h>

namespace Toshi
{
class T2GUIElement;
class T2GUITextBox;
class TInputDevice;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: the cheat codes: while enabled it reads the keyboard and the gamepads (the shoulder buttons held, then the
// directions), keeps the last 10 presses per player 1/2 and runs the cheat whose sequence ends the queue. The in-game
// cheats work in the story levels, the menu ones in ABlobMainMenu.
// Retail's five-cheat table (m_aCheats) is never filled
//-----------------------------------------------------------------------------
class ACheatManager
    : public Toshi::TSingleton<ACheatManager>
{
public:
	static TCONSTEXPR_VAR TINT CHEATS_NUMOF        = 5;
	static TCONSTEXPR_VAR TINT QUEUE_SIZE          = 10;
	static TCONSTEXPR_VAR TINT NUM_DOODADS         = 16; // 8 keyboard keys, then the 8 gamepad buttons
	static TCONSTEXPR_VAR TINT NUM_PADS            = 4;
	static TCONSTEXPR_VAR TINT NUM_SHOULDERDOODADS = 2;

	typedef void ( *t_CheatCallback )();

	// A cheat of the (never filled) table; the two functions' calling conventions are unknown
	struct Cheat
	{
		TINT   m_iCode;                                        // 0x00
		TINT   m_iLocaleId;                                    // 0x04
		TUINT8 m_bEnabled;                                     // 0x08
		void ( *m_pfnSet )( TBOOL a_bEnabled, TINT a_iCheat ); // 0x0C
		TBOOL( *m_pfnGet )
		( TINT a_iCheat ); // 0x10
	};

public:
	// Set by Cheat_AddScore: ABlobScoreHud shows the new score at once (and clears it)
	TBOOL IsScoreJumpPending() const { return m_bScoreJumpPending != 0; }
	void  ClearScoreJumpPending() { m_bScoreJumpPending = TFALSE; }

public:
	ACheatManager();
	~ACheatManager();

	// Enables the cheat codes; a_bInGame: the in-game table, else the menu ones
	void SetEnabled( TBOOL a_bEnabled, TBOOL a_bInGame );

	// Reads the input and checks the cheat sequences
	void Update( TFLOAT a_fDeltaTime );

	// The shoulder buttons are held (keyboard: the cheat key, gamepads: both shoulder buttons)
	TBOOL IsAllShoulderButtonsDown();

	// Shows or hides the message of the last cheat; returns whether it was shown
	TBOOL SetMessageVisible( TBOOL a_bVisible );

	// The time left of the message (0: hidden)
	// $deBlob: FUNCTION 0064a220
	TFLOAT GetMessageTime() const { return m_fMessageTime; }

	// Shows the message for a_fTime seconds
	void ShowMessage( TFLOAT a_fTime );

	void  SetCheatsEnabled( TUINT a_uiMask );
	void  SetCheatEnabledFlag( TINT a_iCheat, TBOOL a_bEnabled );
	void  SetCheatEnabled( TINT a_iCheat, TBOOL a_bEnabled );
	TINT  GetCode( TINT a_iCheat );
	TINT  GetLocaleId( TINT a_iCheat );
	TBOOL IsCheatEnabled( TINT a_iCheat );
	void  UpdateCheat( TINT a_iCheat );

	// The cheats (the callbacks of the sequence tables)
	static void Cheat_ToggleControlsLock();
	static void Cheat_AddTime();
	static void Cheat_AddLife();
	static void Cheat_SkipGoals();
	static void Cheat_UnlockAll();
	static void Cheat_UnlockBlobParty();
	static void Cheat_UnlockExtras();
	static void Cheat_UnlockTracks();
	static void Cheat_ToggleSafeZone();
	static void Cheat_AddScore();
	static void Cheat_ShowCivBlockPosition();
	static void DummyCheat();

private:
	void SetupInputHelper();
	void SetupInputHelperShoulderButtons();
	void ClearInputQueue();
	void ClearCheatsEnabled();

	// Runs the cheat whose sequence ends a_rQueue (a_iCount: its next index) of a_iController
	void CheckSequences( Toshi::T2Array<TINT, QUEUE_SIZE>& a_rQueue, TINT a_iCount, TINT a_iController );


private:
	Toshi::T2Array<TINT, QUEUE_SIZE>          m_aQueue0;                         // 0x00 the last doodads of player 1 (keyboard and first gamepad)
	Toshi::T2Array<TINT, QUEUE_SIZE>          m_aQueue1;                         // 0x28 the last doodads of player 2
	TINT                                      m_iQueue0Index;                    // 0x50 the next index of m_aQueue0
	TINT                                      m_iQueue1Index;                    // 0x54
	TUINT8                                    m_bEnabled;                        // 0x58
	TUINT8                                    m_bInGame;                         // 0x59 the in-game table
	TUINT8                                    m_bSequence;                       // 0x5A the shoulder buttons were held at the last update
	AInputHelperSimple                        m_oKeyboardHelper;                 // 0x5C 16 doodads
	AInputHelperSimple                        m_oKeyboardShoulderHelper;         // 0x6C 2 doodads
	AInputHelperSimple                        m_aPadHelpers[ NUM_PADS ];         // 0x7C 16 doodads each
	AInputHelperSimple                        m_aPadShoulderHelpers[ NUM_PADS ]; // 0xBC 2 doodads each
	Toshi::T2Array<TINT, NUM_DOODADS>         m_aDoodads;                        // 0xFC keyboard 0-7, gamepad 8-15
	Toshi::T2Array<TINT, NUM_SHOULDERDOODADS> m_aShoulderDoodads;                // 0x13C gamepad shoulder, keyboard shoulder (0x140)
	Toshi::T2Array<TUINT8, NUM_PADS>          m_abShoulderHeld;                  // 0x144
	Toshi::TInputDevice*                      m_pCheatDevice;                    // 0x148 the device of the last cheat
	TINT                                      m_iCheatDeviceId;                  // 0x14C
	Toshi::T2Array<Cheat, CHEATS_NUMOF>       m_aCheats;                         // 0x150
	TUINT8                                    m_bScoreJumpPending;               // 0x1B4
	Toshi::T2GUITextBox*                      m_pMessageBox;                     // 0x1B8 the message
	TFLOAT                                    m_fMessageTime;                    // 0x1BC
	TWCHAR                                    m_wszMessage[ 256 ];               // 0x1C0
};

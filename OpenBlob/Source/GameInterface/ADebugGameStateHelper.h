#pragma once
#include "Input/AInputHelper.h"

#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Purpose: a debug input helper of the game states: two buttons of the in-game context toggle the console variable
// "pickups_debug". Nothing creates it in retail.
//-----------------------------------------------------------------------------
class ADebugGameStateHelper
    : public Toshi::TSingleton<ADebugGameStateHelper>
{
public:
	ADebugGameStateHelper();
	virtual ~ADebugGameStateHelper();

	// Adds the buttons 21, 26 and 27 of the in-game context to the input helper
	void Init();

	// TRUE when button 21 was just pressed on the keyboard while button 22 is down on the first controller
	TBOOL Update( TFLOAT a_fDeltaTime );

	// Toggles m_bDebugPickups and "pickups_debug" with it
	void ToggleDebugPickups();

	// Toggles m_bDebugPickups when "pickups_debug" was changed elsewhere
	void SyncDebugPickups();

private:
	AInputHelper m_oInputHelper;  // 0x04
	TBOOL        m_bDebugPickups; // 0x28
	TBOOL        m_bUnk29;        // 0x29 TRUE
	TINT         m_iUnk2C;        // 0x2C -1
};

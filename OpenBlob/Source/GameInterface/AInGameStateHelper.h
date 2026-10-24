#pragma once
#include "Input/AInputHelper.h"

class AInGameState;

//-----------------------------------------------------------------------------
// Purpose: the pause input of AInGameState (a member of the state at 0x34).
//-----------------------------------------------------------------------------
class AInGameStateHelper
{
public:
	AInGameStateHelper( AInGameState* a_pState );
	virtual ~AInGameStateHelper();

	// Returns 1 when the pause menu was opened
	TINT OnUpdate( TFLOAT a_fDeltaTime );
	void OnInsertion();
	// Removes the buttons of the input helper (AInGameState::OnRemoval)
	void OnRemoval();
	// Forgets the buttons that were just pressed (AInGameState::OnActivate)
	// $deBlob: FUNCTION 005c8e40
	void ResetJustDown() { m_oInputHelper.ResetJustDown(); }

private:
	AInputHelper  m_oInputHelper; // 0x04
	AInGameState* m_pState;       // 0x28
};

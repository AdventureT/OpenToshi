#pragma once
#include "GameInterface/AGameState.h"

//-----------------------------------------------------------------------------
// Purpose: a test game state (no members of its own): shows a green square and "Blob07" in the game's GUI.
//-----------------------------------------------------------------------------
class ATestState : public AGameState
{
public:
	TDECLARE_CLASS( ATestState, AGameState );

public:
	ATestState();
	~ATestState();

	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x18
	virtual void         OnInsertion() override;                   // +0x1C
	virtual void         OnRemoval() override;                     // +0x20
	virtual void         OnActivate() override;                    // +0x2C
	virtual void         OnDeactivate() override;                  // +0x30
};

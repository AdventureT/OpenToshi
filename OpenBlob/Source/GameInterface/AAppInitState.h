#pragma once
#include "AGameState.h"

//-----------------------------------------------------------------------------
// Purpose: the first state: waits for the streamed assets, creates the global systems of the game, then gives way to
// the frontend (or AModelViewState).
//-----------------------------------------------------------------------------
class AAppInitState : public AGameState
{
public:
	TDECLARE_CLASS( AAppInitState, AGameState );

public:
	AAppInitState();
	~AAppInitState();

	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void         OnInsertion() override;
	virtual void         OnRemoval() override;
};

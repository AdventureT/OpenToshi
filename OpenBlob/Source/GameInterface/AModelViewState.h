#pragma once
#include "AGameState.h"

class AModelInstance2;

//-----------------------------------------------------------------------------
// Purpose: the model viewer (no TClass of its own): shows "data/WorldModel.trb" at the origin and cycles through its
// skeleton's animations (button 1 skips to the next one, button 0 leaves). Pushed by AAppInitState::OnRemoval instead
// of the frontend when the app asks for it (ShouldLoadModelViewState).
//-----------------------------------------------------------------------------
class AModelViewState : public AGameState
{
public:
	AModelViewState();
	~AModelViewState();

	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void         OnInsertion() override;
	virtual void         OnRemoval() override;

private:
	// Plays the next animation of the model (the first after the last)
	void PlayNextAnim();

private:
	TUINT32          m_uiUnk2C;   // 0x2C
	TINT             m_iState;    // 0x30 0: waiting for the streaming, 1: the model is created
	AModelInstance2* m_pInstance; // 0x34
	TINT16           m_iAnim;     // 0x38 the sequence played, -1: none yet
};                                // 0x3C

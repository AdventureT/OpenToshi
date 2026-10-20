#pragma once
#include "AGameState.h"

class ABlobControllerPopup;

//-----------------------------------------------------------------------------
// Purpose: the end of the game after the boss: the boss music and the frontend sounds load behind the
// "letterboxbossend.xur" letterbox, then the movie "11_Triumph" plays (skippable), the music starts and the score menu
// (scoremenu.xur) is shown; Finish (called by the reward screen and the score menu) goes back to the frontend.
//-----------------------------------------------------------------------------
class ABossEndSeqState
    : public AGameState
{
public:
	TDECLARE_CLASS( ABossEndSeqState, AGameState );

	enum STATE
	{
		STATE_Loading,   // the sounds are loading
		STATE_LetterBox, // waits for the letterbox animation
		STATE_Movie,     // "11_Triumph"
		STATE_ScoreMenu, // shows the score menu
		STATE_InScoreMenu,
		STATE_Finished, // back to the frontend
	};

public:
	ABossEndSeqState();
	~ABossEndSeqState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x18
	virtual void         OnInsertion() override;                   // +0x1C
	virtual void         OnRemoval() override;                     // +0x20
	// $deBlob: FUNCTION 005c2400
	virtual void OnActivate() override { AGameState::OnActivate(); } // +0x2C
	// $deBlob: FUNCTION 005c2410
	virtual void OnDeactivate() override { AGameState::OnDeactivate(); } // +0x30
	// $deBlob: FUNCTION 005c24e0
	virtual TBOOL CheckForControllerRemoval() override { return TTRUE; } // +0x38

	// Goes back to the frontend (the reward screen and the score menu are done)
	void Finish();

private:
	STATE                 m_eState;           // 0x2C
	ABlobControllerPopup* m_pControllerPopup; // 0x30
};

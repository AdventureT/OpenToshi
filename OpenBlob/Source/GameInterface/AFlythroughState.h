#pragma once
#include "GameInterface/AGameState.h"
#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Purpose: a level fly-through: plays the scripted camera animation m_strCamera with the enemies paused and the
// letterbox, button 0x23 skips it; at its end the screen fades to black and back.
//-----------------------------------------------------------------------------
class AFlythroughState : public AGameState
{
public:
	TDECLARE_CLASS( AFlythroughState, AGameState );

	enum STATE : TINT
	{
		STATE_Playing   = 0,
		STATE_FadingOut = 1, // the camera finished or was skipped
		STATE_FadingIn  = 2,
	};

public:
	AFlythroughState( const TCHAR* a_szCamera = "" );
	~AFlythroughState();

	// $deBlob: FUNCTION 005c2f00
	virtual TBOOL        WasControllerRemoved() override { return TTRUE; } // +0x10
	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override;         // +0x18
	virtual void         OnInsertion() override;                           // +0x1C
	virtual void         OnRemoval() override;                             // +0x20
	virtual void         OnActivate() override;                            // +0x2C
	virtual void         OnDeactivate() override;                          // +0x30
	// $deBlob: FUNCTION 005c2ec0
	virtual TBOOL CheckForControllerRemoval() override { return TTRUE; } // +0x38

private:
	Toshi::TString8 m_strCamera;      // 0x2C the scripted camera animation
	STATE           m_eState;         // 0x38
	TBOOL           m_bSkipRequested; // 0x3C
};

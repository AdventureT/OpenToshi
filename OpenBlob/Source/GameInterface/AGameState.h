#pragma once
#include <Toshi/TObject.h>
#include <Toshi/T2DList.h>
#include "Input/AInputHelperSimple.h"

//-----------------------------------------------------------------------------
// Purpose: a state of the game. The states are a stack in AGameStateController; each state can have child states that
// get the same events.
//-----------------------------------------------------------------------------
class AGameState
    : public Toshi::TObject
    , public Toshi::T2DList<AGameState>::Node
{
public:
	TDECLARE_CLASS( AGameState, Toshi::TObject );

public:
	typedef TUINT8 State;
	enum State_ : State
	{
		State_Null      = 0,
		State_Activated = BITFLAG( 0 ),
		State_Inserted  = BITFLAG( 1 ),
		State_Suspended = BITFLAG( 2 ),
	};

	typedef TUINT32 UpdateResult;
	enum UpdateResult_ : UpdateResult
	{
		UpdateResult_OK,
		UpdateResult_Remove,
	};

public:
	AGameState();
	~AGameState();

	// $deBlob: FUNCTION 005b0b40
	virtual void HandleMenuRequestToCancel() {} // +0x0C
	// $deBlob: FUNCTION 005c1ce0
	virtual TBOOL WasControllerRemoved() { return TFALSE; } // +0x10
	// $deBlob: FUNCTION 005b0a10
	virtual void         OnFadeFinished( TINT a_iViewport, TBOOL a_bFromBlack ) {} // +0x14 AGameStateController::UpdateFade
	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime );                          // +0x18
	virtual void         OnInsertion();                                            // +0x1C
	virtual void         OnRemoval();                                              // +0x20
	virtual void         OnSuspend();                                              // +0x24
	virtual void         OnResume( AGameState* a_pPreviousState );                 // +0x28
	virtual void         OnActivate();                                             // +0x2C
	virtual void         OnDeactivate();                                           // +0x30
	// $deBlob: FUNCTION 005c1c90
	virtual TBOOL CanActivateConsoleState() { return TTRUE; } // +0x34
	// $deBlob: FUNCTION 005b0400
	virtual TBOOL CheckForControllerRemoval() { return TFALSE; } // +0x38
	// $deBlob: FUNCTION 005b0a20
	virtual TBOOL PauseOnControllerReinserted() { return TFALSE; } // +0x3C

	// Pops this state (it has to be the current one) from AGameStateController
	void RemoveSelf();

	void AddChildState( AGameState* a_pState );

	static void SetupLoadIcon();
	static void DestroyLoadIcon();

	// $deBlob: FUNCTION 005c1ca0
	TUINT32 GetParent() const { return m_uiParent; }
	State   GetState() const { return m_State; }
	void    AddState( State a_eState ) { m_State |= a_eState; }
	void    RemoveState( State a_eState ) { m_State &= ~a_eState; }

public:
	static void* sm_pLoadIconRect; // T2GUIRectangle
	static void* sm_pLoadIconMat;  // T2GUIMaterial

private:
	Toshi::T2DList<AGameState> m_GameStates;  // 0x0C child states
	AInputHelperSimple         m_InputHelper; // 0x14 the console key
	TUINT32                    m_uiParent;    // 0x24
	State                      m_State;       // 0x28
};

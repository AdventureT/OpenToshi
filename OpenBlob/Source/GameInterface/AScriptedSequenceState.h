#pragma once
#include "GameInterface/AGameState.h"
#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Purpose: a game state for a scripted sequence: only keeps a name, its updates do nothing of their own.
// NOTE: retail's TClass has TObject as parent (not AGameState)
//-----------------------------------------------------------------------------
class AScriptedSequenceState : public AGameState
{
public:
	TDECLARE_CLASS( AScriptedSequenceState, Toshi::TObject );

public:
	AScriptedSequenceState( const TCHAR* a_szName = "" );
	~AScriptedSequenceState();

	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x18
	// $deBlob: FUNCTION 005d2c50
	virtual void OnInsertion() override {} // +0x1C
	// $deBlob: FUNCTION 005d2c60
	virtual void OnRemoval() override {} // +0x20
	// $deBlob: FUNCTION 005d2c70
	virtual void OnActivate() override {} // +0x2C
	// $deBlob: FUNCTION 005d2c80
	virtual void OnDeactivate() override {} // +0x30
	// $deBlob: FUNCTION 005d2d00
	virtual TBOOL CheckForControllerRemoval() override { return TTRUE; } // +0x38

private:
	Toshi::TString8 m_strName; // 0x2C
	TUINT32         m_uiUnk38; // 0x38
};

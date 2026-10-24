#pragma once
#include "AGameState.h"
#include "MemoryCard/EnSaveData.h"

//-----------------------------------------------------------------------------
// Purpose: loads the save at startup: shows black.xur while SaveLoad loads the save into the application's save data (a
// copy of the defaults is restored if it fails), then pushes AFrontEndMovieState.
//-----------------------------------------------------------------------------
class ASaveLoadStartAppState : public AGameState
{
public:
	TDECLARE_CLASS( ASaveLoadStartAppState, AGameState );

public:
	ASaveLoadStartAppState();
	~ASaveLoadStartAppState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x18
	virtual void         OnInsertion() override;                   // +0x1C
	virtual void         OnRemoval() override;                     // +0x20

private:
	EnSaveData m_oSaveData; // 0x2C the application's save data before loading
};

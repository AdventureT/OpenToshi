#pragma once
#include "AChallenge.h"


//-----------------------------------------------------------------------------
// Purpose: a challenge without a goal of its own, only its marker and cameras
//-----------------------------------------------------------------------------
class ANullChallenge
    : public AChallenge
{
public:
	// The data in the TRB (0x3C bytes)
	struct TypeData : public Data
	{
		TINT m_eMarkerType; // 0x38 0: race, 1: normal, 2: landmark, 3: combat
	};

public:
	ANullChallenge();
	// The complete-object destructor (the last marker) is used by AChallengeManager::LoadChallenges' eh vector constructor and the vector deleting destructor
	// $deBlob: FUNCTION 0059cc80
	// $deBlob: FUNCTION 0059ca20
	virtual ~ANullChallenge() override = default; // +0x00

	virtual void Complete() override; // +0x08

private:
};

TSTATICASSERT( sizeof( ANullChallenge ) == 0x174 );

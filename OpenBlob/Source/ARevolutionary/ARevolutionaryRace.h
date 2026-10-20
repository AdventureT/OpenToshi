#pragma once
#include "ARevolutionary/ARevolutionary.h"

//-----------------------------------------------------------------------------
// Purpose: the marker of a race challenge: an ARevolutionary showing mesh 1 of the model (m_uiMesh) and mesh 0 instead
// of 0 (m_uiMeshA)
//-----------------------------------------------------------------------------
class ARevolutionaryRace
    : public ARevolutionary
{
public:
	TDECLARE_CLASS( ARevolutionaryRace, ARevolutionary );

public:
	ARevolutionaryRace();
	virtual ~ARevolutionaryRace() override; // +0x08
};

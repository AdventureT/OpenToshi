#pragma once
#include "ARevolutionary/ARevolutionary.h"

//-----------------------------------------------------------------------------
// Purpose: the marker of a landmark challenge: an ARevolutionary showing mesh 0 of the model (m_uiMesh) and mesh 1
// instead of 0 (m_uiMeshA)
//-----------------------------------------------------------------------------
class ARevolutionaryLandmark
    : public ARevolutionary
{
public:
	TDECLARE_CLASS( ARevolutionaryLandmark, ARevolutionary );

public:
	ARevolutionaryLandmark();
	virtual ~ARevolutionaryLandmark() override; // +0x08
};

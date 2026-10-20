#pragma once
#include "ARevolutionary/ARevolutionary.h"

//-----------------------------------------------------------------------------
// Purpose: the marker of a combat challenge: an ARevolutionary showing mesh 0 of the model (m_uiMesh) and mesh 1
// instead of 0 (m_uiMeshA)
//-----------------------------------------------------------------------------
class ARevolutionaryCombat
    : public ARevolutionary
{
public:
	TDECLARE_CLASS( ARevolutionaryCombat, ARevolutionary );

public:
	ARevolutionaryCombat();
	virtual ~ARevolutionaryCombat() override; // +0x08
};

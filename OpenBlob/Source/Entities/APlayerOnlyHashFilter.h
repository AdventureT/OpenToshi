#pragma once
#include "Entities/AEntityHashFilter.h"
#include "Entities/ABaseEntity.h"

//-----------------------------------------------------------------------------
// Purpose: the AEntityHash query filter that only accepts the players' entities (ABaseEntity::FLAG_Player, used by
// APlayerMoverComponent::Tick).
//-----------------------------------------------------------------------------
class APlayerOnlyHashFilter
    : public AEntityHashFilter
{
public:
	// $deBlob: FUNCTION 007e5dd0
	virtual TBOOL operator()( ABaseEntity* a_pEntity ) override { return ( a_pEntity->GetFlags() & ABaseEntity::FLAG_Player ) != 0; }
};

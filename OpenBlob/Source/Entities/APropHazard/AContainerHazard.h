#pragma once
#include "Entities/ABaseEntity.h"
#include "Entities/ATriggerUtil.h"

class ASpawnPointPool;

//-----------------------------------------------------------------------------
// Purpose: a group of hazards (plus ATriggerUtil's 6 slots at 0x80, its code sits in APropHazard.cpp): an invisible,
// non-ticking entity whose targets are APropHazards. It forwards the trigger and on/off messages to them and shares its
// "HazardPointList" with them (APropHazard::GetSpawnPoint).
// NOTE: names from the TClass ("AContainerHazard") and the entity params
//-----------------------------------------------------------------------------
class AContainerHazard
    : public ABaseEntity
    , public ATriggerUtil
{
public:
	TDECLARE_CLASS( AContainerHazard, ABaseEntity );

public:
	AContainerHazard();
	virtual ~AContainerHazard() override; // +0x08

	virtual void PostCreate() override;                                                               // +0x20
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8

	ASpawnPointPool* GetSpawnPoints() const { return m_pSpawnPoints; }

private:
	ASpawnPointPool* m_pSpawnPoints; // 0x90 "HazardPointList"
};

#pragma once
#include "Entities/AVisibleEntity.h"

class ACar;

//-----------------------------------------------------------------------------
// Purpose: the spawner of a tank (ATankSpawner\ATankSpawner.cpp, parent AVisibleEntity): it finds its ACar (a tank) by
// the ID of its parameters (ProcessParams, FindIDEntity) and wakes it up again once it has been destroyed (states 0 to
// 4: wait for the tank, 5 s, a HandleEntityIO to the tank, 3 s, back to 0), at most "NumTanksAvailable" times when a
// combat challenge counts the tanks (m_bChallengeLinked).
// The layout is checked by TSTATICASSERT (0x9C is AVisibleEntity's size).
//-----------------------------------------------------------------------------
class ATankSpawner
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( ATankSpawner, AVisibleEntity );

public:
	ATankSpawner();
	virtual ~ATankSpawner() override; // +0x08

	virtual void         Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void         Create() override;                                                                   // +0x1C
	virtual void         ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void         HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void         Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual const TCHAR* GetModelName() override;                                                             // +0xEC

	// The tank was destroyed (ACar::HandleEntityIO); the last one disables the spawner and tells the challenge
	void OnTankDestroyed();

	// All the tanks of the challenge were destroyed (ACombatChallenge)
	TBOOL IsComplete() const;

	ACar* GetTank() const { return m_pTank; }

protected:
	ACar* m_pTank;            // 0x9C
	TINT8 m_iTanksAvailable;  // 0xA0 "NumTanksAvailable" (-1: no limit)
	TINT8 m_iTanksDestroyed;  // 0xA1
	TBOOL m_bDisabled;        // 0xA2 the spawner doesn't tick
	TBOOL m_bChallengeLinked; // 0xA3 a combat challenge counts the tanks (MESSAGE_CHALLENGEINIT)
};

#pragma once
#include "Entities/ABaseEntity.h"

#include <Toshi/T2DList.h>

class AEnemy;

//-----------------------------------------------------------------------------
// Purpose: a squad of enemies: the entity targets are the AEnemyWrapper and AStreetSpawner entities whose enemies
// belong to it. The active enemies share their targets (AEnemyBehaviour::UpdateEnemyList reports a seen enemy with
// AddTarget, every active member is told with AAIBehaviour::AddToEnemyList), a target is forgotten once its time runs
// out (Tick). Once every member is dead (AreAllMembersDead), OnMemberKilled spawns a pickup near the player (unless a
// combat challenge owns the enemies).
// The wrappers' x range (m_fMinX, m_fRangeX) is used by AEnemyWrapper::SpawnEnemy to offset the spawn height.
//-----------------------------------------------------------------------------
class ASquad
    : public ABaseEntity
{
public:
	TDECLARE_CLASS( ASquad, ABaseEntity );

	static TCONSTEXPR_VAR TINT MAX_TARGETS = 4;

	// A member of the squad (0xC bytes)
	using SquadNode = Toshi::T2DListNodeWrapper<AEnemy>;
	using SquadList = Toshi::T2DList<SquadNode>;

public:
	ASquad();
	virtual ~ASquad() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                     // +0x0C
	virtual void PostCreate() override;                                    // +0x20
	virtual void Destroy() override;                                       // +0x24
	virtual void ProcessParams( const AEntityParams& a_rParams ) override; // +0x2C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                 // +0xC8

	// Tells a_pEnemy's behaviour about every target of the squad
	void TellTargets( ABaseEntity* a_pEnemy );

	// The enemy joins the active members (and learns the squad's targets)
	void AddMember( AEnemy* a_pEnemy );

	// The enemy leaves the active members
	void RemoveMember( AEnemy* a_pEnemy );

	// Adds the target a_pTarget for a_fTime seconds (or raises its time)
	void AddTarget( ABaseEntity* a_pTarget, TFLOAT a_fTime );

	// Is a_pTarget a target of the squad whose time isn't over?
	TBOOL IsTarget( ABaseEntity* a_pTarget ) const;

	// A member was killed: spawns a pickup once every member is dead
	void OnMemberKilled();

	// Are every wrapper released and every street spawner deactivated with no spawn left?
	TBOOL AreAllMembersDead() const;

	// $deBlob: FUNCTION 007811d0
	TFLOAT GetMinX() const { return m_fMinX; }
	// $deBlob: FUNCTION 00781200
	TFLOAT GetRangeX() const { return m_fRangeX; }

private:
	TINT         m_iNumSquadIDs;                // 0x80 the entity targets
	TINT*        m_pSquadIDs;                   // 0x84
	SquadNode*   m_pSquadPool;                  // 0x88 one node per possible member
	SquadList    m_oFreeSquadList;              // 0x8C
	SquadList    m_oActiveSquadList;            // 0x94
	ABaseEntity* m_apTargets[ MAX_TARGETS ];    // 0x9C
	TFLOAT       m_afTargetTime[ MAX_TARGETS ]; // 0xAC
	TINT         m_iNumTargets;                 // 0xBC
	TFLOAT       m_fMinX;                       // 0xC0 the lowest x of the wrappers
	TFLOAT       m_fRangeX;                     // 0xC4 -1: no wrapper
};

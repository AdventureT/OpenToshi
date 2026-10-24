#pragma once
#include "AChallenge.h"


//-----------------------------------------------------------------------------
// Purpose: a challenge against enemies
//-----------------------------------------------------------------------------
class ACombatChallenge
    : public AChallenge
{
public:
	// One linked entity (0x10 bytes)
	struct Entry
	{
		union
		{
			TINT m_iID; // 0x00 in the TRB...
		};
		ABaseEntity* m_pEntity; // 0x04 ...and the entity (Create)
		TINT         m_eType;   // 0x08 0: APropTriggerEnemyGroup, 1: AEnemyWrapper, 2: ACar, 3: APropTurret, 4: ATankSpawner
		TINT         m_iDone;   // 0x0C killed (or not counted)
	};

	// The data in the TRB (0x40 bytes)
	struct TypeData : public Data
	{
		Entry*  m_pEntries;     // 0x38
		TUINT32 m_uiNumEntries; // 0x3C
	};

public:
	ACombatChallenge();
	// The complete-object destructor (the last marker) is used by AChallengeManager::LoadChallenges' eh vector constructor and the vector deleting destructor
	// $deBlob: FUNCTION 0059cbc0
	// $deBlob: FUNCTION 0059ca00
	virtual ~ACombatChallenge() override = default; // +0x00

	virtual void  Start() override;                            // +0x04
	virtual void  OnCombatEvent( void* a_pEntityID ) override; // +0x18
	virtual void  Update( TFLOAT a_fDeltaTime ) override;      // +0x30
	virtual void  Reset( TBOOL a_bUnk ) override;              // +0x3C
	virtual TINT  GetScore() override;                         // +0x40
	virtual TBOOL IsAlreadyCompleted() override;               // +0x44
	virtual TBOOL Create( TypeData* a_pData );                 // +0x4C

	// Whether a_pEntity is one of the linked AEnemyWrapper entities (ASquad: no pickup for them)
	TBOOL IsLinkedEnemy( ABaseEntity* a_pEntity ) const;

private:
	TINT m_iNumEnemies;  // 0x174 the enemies to kill (Start)
	TINT m_iEnemiesLeft; // 0x178
};

TSTATICASSERT( sizeof( ACombatChallenge ) == 0x17C );

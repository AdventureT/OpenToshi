#pragma once
#include "Entities/APropTrigger.h"

class AEnemyWrapper;

//-----------------------------------------------------------------------------
// Purpose: a trigger that spawns a group of enemies: its entity targets that are enemy wrappers (FLAG_EnemyWrapper) are
// the enemies of the group. Once triggered it activates every wrapper (SpawnEnemyGroup), waits until they're all gone,
// then spawns the next wave after "Delay" ("EnemyWavesCount" waves, 0: forever) and triggers its targets once the last
// wave is dead. ACombatChallenge counts the enemies of the groups it links.
//-----------------------------------------------------------------------------
class APropTriggerEnemyGroup
    : public APropTrigger
{
public:
	TDECLARE_CLASS( APropTriggerEnemyGroup, APropTrigger );

	// The state of the group (the low 3 bits of m_uiGroupFlags)
	enum TRIGGERSTATE
	{
		TRIGGERSTATE_Idle     = 0, // waits for a trigger
		TRIGGERSTATE_Spawn    = 1, // spawns the group (Tick)
		TRIGGERSTATE_Delay    = 2, // waits "Delay" before the next wave
		TRIGGERSTATE_Spawned  = 3, // waits until every enemy is gone
		TRIGGERSTATE_Finished = 4, // the last wave is dead
		TRIGGERSTATE_Max      = 5,
	};

	// m_uiGroupFlags (above the state)
	enum GROUPFLAG : TUINT8
	{
		GROUPFLAG_StateMask                      = 0x07,
		GROUPFLAG_Disabled                       = 0x08, // MESSAGE_CHALLENGEINIT: can't be triggered, Trigger restarts the waves
		GROUPFLAG_SpawnOnTriggerOnly             = 0x10, // "SpawnOnTriggerOnly": the next wave waits for a trigger
		GROUPFLAG_NoSpawn                        = 0x20, // MESSAGE_STOP: never spawns (and ignores triggers)
		GROUPFLAG_KeepEnemies                    = 0x40, // MESSAGE_CHALLENGERESET: the next Reset keeps the spawned enemies
		GROUPFLAG_DespawnEnemiesOnChallengeReset = 0x80, // "DespawnEnemiesOnChallengeReset"
	};

	// One enemy of the group (0x1C bytes)
	struct EnemyInfo
	{
		enum STATE
		{
			STATE_None     = 0,
			STATE_Spawned  = 1,
			STATE_Dead     = 2,
			STATE_Suicider = 3, // its enemy left the group (MESSAGE_ENEMYKILLED of the wrapper), spawns again
		};

		// $deBlob: FUNCTION 00780300
		EnemyInfo()
		    : m_iTarget( -1 ), m_eEnemyType( -1 ), m_vPosition( 0.0f, 0.0f, 0.0f, 1.0f ), m_eState( STATE_None )
		{}

		// $deBlob: FUNCTION 007803b0
		TBOOL IsSuicider() const { return m_eState == STATE_Suicider; }

		TINT            m_iTarget;    // 0x00 the index of the entity target (the wrapper)
		TINT            m_eEnemyType; // 0x04 AEnemyWrapper::GetEnemyType
		Toshi::TVector4 m_vPosition;  // 0x08 the wrapper's position
		TINT            m_eState;     // 0x18 STATE
	};

	// The wrapper spawned for an enemy (8 bytes)
	struct SpawnedEnemy
	{
		AEnemyWrapper* pEnemyWrapper; // 0x00
		EnemyInfo*     pEnemyInfo;    // 0x04
	};

public:
	APropTriggerEnemyGroup();
	virtual ~APropTriggerEnemyGroup() override; // +0x08

	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void  PostCreate() override;                                                               // +0x20 (also ATriggerUtil +0x08)
	virtual void  Destroy() override;                                                                  // +0x24
	virtual void  ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C (also ATriggerUtil +0x04)
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void  Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual TBOOL CanObjectTrigger( Toshi::TObject* a_pObject ) override;                              // +0xE0
	virtual void  Trigger() override;                                                                  // +0xE4

	// The enemy of a_pWrapper is gone (killed or released): it doesn't belong to the group anymore
	void ReleaseEnemy( AEnemyWrapper* a_pWrapper );

	// The enemy of a_pWrapper left the group: it spawns again with the next wave
	void FlagEnemyAsSuicider( AEnemyWrapper* a_pWrapper );

	// Sets the enemy a_iIndex
	void AddEnemyInfo( TINT a_iTarget, TINT a_eEnemyType, const Toshi::TVector4& a_rPosition, TINT a_iIndex );

	// Counts the delay down: is the next wave due?
	TBOOL AbleToSpawn( TFLOAT a_fDeltaTime );

	// $deBlob: FUNCTION 007803f0
	void SetTriggerState( TINT a_state )
	{
		m_uiGroupFlags &= ~GROUPFLAG_StateMask;
		TASSERT( a_state >= 0 && a_state < TRIGGERSTATE_Max );
		m_uiGroupFlags |= TUINT8( a_state );
	}

	// $deBlob: FUNCTION 00780390
	TINT GetTriggerState() const { return m_uiGroupFlags & GROUPFLAG_StateMask; }

	TINT GetNumEnemies() const { return m_iNumEnemies; }
	TINT GetNumSpawnedEnemies() const;
	// $deBlob: FUNCTION 007811b0
	TINT16 GetDropHeight() const { return m_iDropHeight; }
	// $deBlob: FUNCTION 007811c0
	TINT16 GetDropRange() const { return m_iDropRange; }

private:
	void SpawnEnemyGroup( TFLOAT a_fDeltaTime );
	void SpawnGroupAllDeadUpdate( TFLOAT a_fDeltaTime );
	void SpawnEnemy( SpawnedEnemy& a_rSpawned, EnemyInfo* a_pInfo );
	void GetPickupAttribs( TINT& a_reType, TINT& a_riCount );

private:
	TINT          m_iNumEnemies;     // 0xC0 the entity targets that are enemy wrappers
	TINT          m_iEnemyToSpawn;   // 0xC4
	EnemyInfo*    m_pEnemyInfo;      // 0xC8 m_iNumEnemies
	SpawnedEnemy* m_pSpawnedEnemies; // 0xCC m_iNumEnemies
	TUINT8        m_uiNumWaves;      // 0xD0 "EnemyWavesCount" (0: forever)
	TUINT8        m_uiCurrentWave;   // 0xD1
	TFLOAT        m_fDelay;          // 0xD4 "Delay" between the waves
	TFLOAT        m_fTimer;          // 0xD8
	TFLOAT        m_fPickupChance;   // 0xDC "PickupChance" (< 0: the type data's)
	TFLOAT        m_fHealthChance;   // 0xE0 "HealthChance" (< 0: the type data's)
	TINT16        m_iPickupCount;    // 0xE4 "PickupCount"
	TINT16        m_iDropHeight;     // 0xE6 "DropHeight" (AEnemyWrapper::SpawnEnemy)
	TINT16        m_iDropRange;      // 0xE8 "DropRange"
	TUINT8        m_uiGroupFlags;    // 0xEA TRIGGERSTATE + GROUPFLAG
};

#pragma once
#include "Entities/AVisibleEntity.h"
#include <Render/TModel.h>

class AEnemy;
class ACollisionObject;
namespace Toshi
{
class TMaterial;
class TParticleEffect;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: a street spawner (AStreetSpawner\AStreetSpawner.cpp, parent AVisibleEntity): a manhole that spawns Inkies
// (AEnemyManager::CreateEnemy, type 0 or 3) every "Spawn Delay" seconds while a player's camera is within 175 units, at
// most "MaxSpawns" alive (and ms_iNumSpawned < 10 for all the spawners). Deactivated (message 0x28) it renders with the
// spawner colour material (RenderCallback).
// States: 0 waits, 1 opens, 2 spawns once at half a second, 3 closes (the last spawn follows it).
// The layout is checked by TSTATICASSERT (0x9C is AVisibleEntity's size).
//-----------------------------------------------------------------------------
class AStreetSpawner
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( AStreetSpawner, AVisibleEntity );

	using SSFLAG = TUINT8;
	enum SSFLAG_ : SSFLAG
	{
		SSFLAG_HasSpawnDelay = 0x01, // "Spawn Delay" was set
		SSFLAG_StartEnabled  = 0x02, // "Start Enabled" (with a spawn delay)
		SSFLAG_Enabled       = 0x04,
		SSFLAG_Reset         = 0x08, // reset by a blob this frame (IsDead), cleared by Tick
		SSFLAG_Deactivated   = 0x10, // MESSAGE_TOGGLE: never spawns again, rendered with sm_pMaterial
	};

public:
	AStreetSpawner();
	virtual ~AStreetSpawner() override; // +0x08

	virtual void              Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void              OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void              Create() override;                                                                   // +0x1C
	virtual void              Destroy() override;                                                                  // +0x24
	virtual void              ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual TBOOL             IsDead() const override;                                                             // +0x3C
	virtual void              HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual ACollisionObject* GetCollisionObject() override;                                                       // +0x70
	virtual TBOOL             IsTargetable() override;                                                             // +0xA0
	virtual void              Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual void              Despawn( TBOOL a_bPlayEffect ) override;                                             // +0xE4
	virtual void*             GetDespawnEffect() override;                                                         // +0xE8
	virtual const TCHAR*      GetModelName() override;                                                             // +0xEC

	// $deBlob: FUNCTION 007a4950
	void   SetSquad( ABaseEntity* a_pSquad ) { m_pSquad = a_pSquad; }
	TINT16 GetMaxSpawns() const { return m_iMaxSpawns; }
	TINT16 GetNumSpawns() const { return m_iNumSpawns; }
	TBOOL  IsDeactivated() const { return ( m_uiSSFlags & SSFLAG_Deactivated ) != 0; }

	// Starts spawning (messages 1, 0x25, 0x26)
	void Enable();

	// Stops spawning (ABossTrigger::DespawnTargets)
	void Stop();

	// Calls Despawn( TTRUE ) on the spawned enemies (ABossTrigger::DespawnTargets)
	void DeactivateSpawns();

	// Restarts the spawn timer
	void RestartTimer();

	// The collision object and the entity flags of Create
	void CreateCollisionObject();

	// The total of the enemies spawned by the street spawners (reset by AInGameState)
	static TINT ms_iNumSpawned;

private:
	// Can an enemy spawn now (the counts and the linked instance's collision)?
	TBOOL CanSpawn() const;

	// The model's render callback: the deactivated spawner renders with sm_pMaterial
	static void __stdcall RenderCallback( Toshi::TModelInstance* a_pModelInstance, void* a_pSpawner );

private:
	static Toshi::TMaterial*       sm_pMaterial; // the spawner colour material
	static Toshi::TParticleEffect* sm_despawnEffect;
	static TINT                    ms_iMaxNumSpawned;
	static TFLOAT                  ms_fSpawnRange; // the distance to a camera

	Toshi::TModelInstance::t_CustomRenderCB m_fnOldRenderCallback; // 0x9C the model's render callback before Create
	void*                                   m_pOldRenderUserData;  // 0xA0
	AEnemy**                                m_ppSpawns;            // 0xA4 m_iMaxSpawns enemies
	ABaseEntity*                            m_pSquad;              // 0xA8 an ASquad (set by it), passed to the spawned enemies
	ACollisionObject*                       m_pCollisionObject;    // 0xAC
	TFLOAT                                  m_fTimer;              // 0xB0
	TFLOAT                                  m_fSpawnDelay;         // 0xB4 "Spawn Delay"
	TINT16                                  m_iMaxSpawns;          // 0xB8 "MaxSpawns"
	TINT16                                  m_iNumSpawns;          // 0xBA
	TINT16                                  m_iLastSpawn;          // 0xBC the index of the last spawn (-1: none)
	SSFLAG                                  m_uiSSFlags;           // 0xBE SSFLAG
	TUINT8                                  m_uiInkyType;          // 0xBF "Inky Type" (0: enemy type 0, else 3)
	TINT16                                  m_iLinkedInstance;     // 0xC0 "LinkedInstance": a terrain instance whose collision must be ready
};

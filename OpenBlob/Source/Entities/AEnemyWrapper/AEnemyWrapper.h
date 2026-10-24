#pragma once
#include "Entities/ABaseEntity.h"

class AEnemy;

//-----------------------------------------------------------------------------
// Purpose: the placeholder of an enemy in the level data: once its LOD rises above low it asks AEnemyManager for a
// pooled enemy of its type (AEnemyManager::CreateEnemy) and gives it back when the enemy dies or leaves.
//-----------------------------------------------------------------------------
class AEnemyWrapper
    : public ABaseEntity
{
public:
	TDECLARE_CLASS( AEnemyWrapper, ABaseEntity );

	// AEnemyTypes::EnemyType (de Blob has 4)
	static TCONSTEXPR_VAR TINT ENEMYTYPE_INVALID   = -1;
	static TCONSTEXPR_VAR TINT ENEMYTYPE_COUNT     = 4;
	static TCONSTEXPR_VAR TINT ENEMYTYPE_LeechBot2 = 2; // uses the ALeechBot2 custom data check

	enum WFLAG : TUINT8
	{
		WFLAG_Released      = 0x1,  // released (killed, gone or deactivated)
		WFLAG_SPAWNED       = 0x2,  // m_pSpawnedEnemy is valid
		WFLAG_HasOwner      = 0x4,  // SetOwnerTrigger
		WFLAG_Activated     = 0x8,  // Activate
		WFLAG_Persistent    = 0x10, // "Persistent": stays released after leaving the medium LOD
		WFLAG_WaitForLinked = 0x20, // waits (Tick) until the linked terrain instance is ready
	};

public:
	AEnemyWrapper();
	virtual ~AEnemyWrapper() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8

	// Spawns the enemy (unless at low LOD or already spawned) and marks the wrapper activated
	void Activate( TINT a_iPickupType, TINT a_iNumPickups );
	// Releases the enemy (if spawned)
	void Deactivate();
	void SetOwnerTrigger( ABaseEntity* a_pTrigger );

	// The position of the spawned enemy, or the wrapper's
	const Toshi::TVector4& GetPosition() const;

	TINT GetEnemyType() const { return m_eEnemyType; }
	// $deBlob: FUNCTION 0077ce20
	TINT16 GetLinkedInstance() const { return m_iLinkedInstance; }
	// $deBlob: FUNCTION 00781210
	AEnemy* GetSpawnedEnemy() const { return m_pSpawnedEnemy; }
	// $deBlob: FUNCTION 007811e0
	ABaseEntity* GetOwnerTrigger() const { return ( m_iOwnerTrigger < 0 ) ? TNULL : FindIDEntity( m_iOwnerTrigger ); }
	// $deBlob: FUNCTION 00781220
	ABaseEntity* GetSpawner() const { return ( m_iSpawner < 0 ) ? TNULL : FindIDEntity( m_iSpawner ); }
	TBOOL        IsSpawned() const { return ( m_uiFlags & WFLAG_SPAWNED ) != 0; }
	TUINT8       GetFlags() const { return m_uiFlags; }
	// $deBlob: FUNCTION 0059dcd0
	TUINT8 GetCustomData() const { return m_uiCustomData; }

	// Is the enemy still there (not released, and activated if it has an owner)?
	// $deBlob: FUNCTION 0059dd00
	TBOOL IsAlive() const { return !( m_uiFlags & WFLAG_Released ) && ( !( m_uiFlags & WFLAG_HasOwner ) || ( m_uiFlags & WFLAG_Activated ) ); }

	// The squad of the wrapper (ASquad::PostCreate); m_iSpawner is the squad's ID
	// $deBlob: FUNCTION 007a4880
	void SetSquad( ABaseEntity* a_pSquad )
	{
		TASSERT( !a_pSquad->IsPooled() );
		m_iSpawner = a_pSquad->GetID();
		TASSERT( m_iSpawner > -1 && FindIDEntity( m_iSpawner ) == a_pSquad );
	}

private:
	void SpawnEnemy( TINT a_iPickupType, TINT a_iNumPickups, TBOOL a_bOffsetHeight );
	void ReleaseEnemy();

	void OnEnemyGone();
	void OnEnemyKilled();
	void OnEnemyRemoved();

private:
	AEnemy* m_pSpawnedEnemy;   // 0x80
	TINT16  m_iSpawner;        // 0x84 its ASquad (SetSquad), whose x range offsets the spawn height
	TINT16  m_iOwnerTrigger;   // 0x86 APropTriggerEnemyGroup
	TINT16  m_iLinkedInstance; // 0x88 "LinkedInstance": a terrain instance (ATerrain2::GetInstance)
	TINT8   m_eEnemyType;      // 0x8A "EnemyType"
	TUINT8  m_uiFlags;         // 0x8B WFLAG
	TUINT8  m_uiCustomData;    // 0x8C "CustomData" + "CustomData2"
};

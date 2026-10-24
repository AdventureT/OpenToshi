#pragma once
#include "Entities/ACharEntity.h"
#include "Entities/AEntityFactory.h"

namespace Toshi
{
class TPhysicsObjectContainer;
class TPhysicsPhantomContainer;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: the base of the INKT enemies: pooled by AEnemyManager (FLAG_Pooled), handed out to the AEnemyWrapper of the
// level data (AEnemyManager::CreateEnemy) and given back when killed (ReleaseToWrapper).
//-----------------------------------------------------------------------------
class AEnemy
    : public ACharEntity
{
public:
	TDECLARE_CLASS( AEnemy, ACharEntity );

	// ABaseEntity states of the enemies (COMMONSTATE)
	enum COMMONSTATE
	{
		COMMONSTATE_Idle       = 0,
		COMMONSTATE_IdleMoving = 1,
		COMMONSTATE_Fidget     = 2,
		COMMONSTATE_Walk       = 3,
		COMMONSTATE_Run        = 4,
		COMMONSTATE_Fall       = 5,
		COMMONSTATE_Land       = 6,
		COMMONSTATE_Alarm      = 7,
		COMMONSTATE_Angry      = 9,
		COMMONSTATE_Recover    = 10,
	};

	// m_uiEnemyFlags
	enum EFLAG : TUINT16
	{
		EFLAG_Melee              = 0x1,  // AEnemyMelee
		EFLAG_OneProjectilePool  = 0x2,  // one projectile pool (AEnemyManager::InitProjectilePool)
		EFLAG_TwoProjectilePools = 0x4,  // two projectile pools
		EFLAG_Grappler           = 0x20, // AEnemyGrappler
		EFLAG_Active             = 0x40,
		EFLAG_InactiveTimedOut   = 0x80,
		EFLAG_Unk100             = 0x100,
		EFLAG_Suicider           = EFLAG_InactiveTimedOut, // AEnemyManager::FindSuicider takes such an enemy when its pool is empty
	};

	// The type data of an enemy ("<type>_data_EntityData", AEntityTypeData::m_pTRBData)
	struct EnemyData
	    : public AEntityTypeData::TRBData
	{
		TUINT8           m_aUnk54[ 0x08 ];        // 0x54
		Toshi::TVector4* m_pBBox;                 // 0x5C
		TINT             m_iEnemyType;            // 0x60 AEnemyManager::EnemyType
		TFLOAT           m_fVisualRange;          // 0x64
		TFLOAT           m_fWalkSpeed;            // 0x68
		TFLOAT           m_fRunSpeed;             // 0x6C
		TFLOAT           m_fRecoilFactor;         // 0x70
		TFLOAT           m_fUnk74;                // 0x74
		TFLOAT           m_fUnk78;                // 0x78 AEnemyBehaviour slot +0x60
		TFLOAT           m_fPathTaskTime;         // 0x7C AAISchedule::OverrideTaskTime of a task that moves along a path
		TFLOAT           m_fRepathTime;           // 0x80 AEnemySchedule: the time between the surround point checks
		TUINT8           m_aUnk84[ 0x28 ];        // 0x84
		TFLOAT           m_fChooseAttackRangeMin; // 0xAC
		TFLOAT           m_fChooseAttackRangeMax; // 0xB0
		TFLOAT           m_fAttackRangeMin;       // 0xB4
		TFLOAT           m_fAttackRangeMax;       // 0xB8
		TFLOAT           m_fAttackRangeOptimal;   // 0xBC
		TFLOAT           m_fAttackRecoverTime;    // 0xC0
		TFLOAT           m_fTaskTime;             // 0xC4 AAISchedule::OverrideTaskTime
		TFLOAT           m_fCharAttackRangeOptimal;                // 0xC8
		TFLOAT           m_fCharAttackRangeMax;                // 0xCC
		TFLOAT           m_fFidgetFreq;           // 0xD0
		TFLOAT           m_fSpawnRange;           // 0xD4
		TFLOAT           m_fPatrolRadius;         // 0xD8
	};

	// CreateComponent / GetComponent
	enum ComponentType
	{
		ComponentType_Pose         = 0,
		ComponentType_Model        = 1,
		ComponentType_FXBreakpoint = 2,
		ComponentType_State        = 3,
		ComponentType_Motor        = 4,
		ComponentType_Pathfinding  = 5,
		ComponentType_Behaviour    = 6,
		ComponentType_Schedule     = 7,
		ComponentType_Count        = 8,
	};

public:
	AEnemy();
	virtual ~AEnemy() override; // +0x08

	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void  OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void  PostCreate() override;                                                               // +0x20
	virtual void  SetVisible( TBOOL a_bVisible ) override;                                             // +0x38
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual TBOOL HandleFallenThroughWorld() override;                                                 // +0x78
	virtual void  SetBBox( Toshi::TVector4 a_vBBox ) override;                                         // +0x7C
	// $deBlob: FUNCTION 007a8b60
	virtual TBOOL IsTargetable() override { return TTRUE; } // +0xA0
	// NOTE: CreateEnemy passes the wrapper through the TINT of ABaseEntity's slot
	// $deBlob: FUNCTION 0077d1e0
	virtual void SetPoolOwner( TINT a_iOwner ) override { m_pWrapper = TREINTERPRETCAST( ABaseEntity*, a_iOwner ); } // +0xC0
	// $deBlob: FUNCTION 0077cef0
	virtual TINT GetPoolOwner() override { return TREINTERPRETCAST( TINT, m_pWrapper ); } // +0xC4
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                // +0xC8
	virtual void OnLeaveState( TINT a_iState ) override;                                  // +0xD4
	virtual void OnEnterState( TINT a_iState ) override;                                  // +0xD8
	virtual void Despawn( TBOOL a_bPlayEffect ) override;                                 // +0xE4
	// $deBlob: FUNCTION 0077ced0
	virtual const TCHAR* GetModelName() override { return GetEnemyData()->m_szModelName; } // +0xEC
	// $deBlob: FUNCTION 007a7860
	virtual void InitRecoverState() override { SetState( COMMONSTATE_Recover ); } // +0xF0
	// $deBlob: FUNCTION 0077cfb0
	virtual TFLOAT GetVisualRange() override { return GetEnemyData()->m_fVisualRange; } // +0x108
	// $deBlob: FUNCTION 0077cfc0
	virtual TFLOAT GetWalkSpeed() override { return GetEnemyData()->m_fWalkSpeed; } // +0x10C
	// $deBlob: FUNCTION 0077cf20
	virtual TFLOAT GetRunSpeed() override { return GetEnemyData()->m_fRunSpeed; } // +0x110
	virtual TFLOAT GetFleeRange() override;                                       // +0x114
	// $deBlob: FUNCTION 0077ce80
	virtual TFLOAT GetAttackRangeOptimal() override { return GetEnemyData()->m_fCharAttackRangeOptimal; } // +0x118
	// $deBlob: FUNCTION 0077ce30
	virtual TFLOAT GetAttackRangeMax() override { return GetEnemyData()->m_fCharAttackRangeMax; } // +0x11C
	virtual void   CreateBehaviourComponent() override;                              // +0x130
	virtual void   CreateScheduleComponent() override;                               // +0x134
	// $deBlob: FUNCTION 007a8200
	virtual TBOOL Unk13C( ABaseEntity* a_pEntity ) override { return TUINT8( ~( a_pEntity->GetFlags() >> 5 ) ) & 1; } // +0x13C
	virtual TBOOL UpdateKnock( TFLOAT a_fDeltaTime ) override;                   // +0x140
	// $deBlob: FUNCTION 0077cf10
	virtual TFLOAT GetRecoilFactor() override { return GetEnemyData()->m_fRecoilFactor; } // +0x144
	virtual void   UpdateFacing( TFLOAT a_fDeltaTime ) override;                          // +0x148
	virtual void   StartCurrentStateTask() override;                                      // +0x154
	virtual TBOOL  UpdateCurrentStateTask( TFLOAT a_fDeltaTime ) override;                // +0x158
	virtual void   InitIdleState() override;                                              // +0x15C
	virtual TBOOL  UpdateIdleState( TFLOAT a_fDeltaTime ) override;                       // +0x160
	// $deBlob: FUNCTION 007a83d0
	virtual void  InitFidgetState() override { SetState( COMMONSTATE_Fidget ); } // +0x164
	virtual TBOOL UpdateFidgetState( TFLOAT a_fDeltaTime ) override;             // +0x168
	virtual void  InitMoveState() override;                                      // +0x16C
	// $deBlob: FUNCTION 0077d2a0
	virtual TBOOL UpdateMoveState( TFLOAT a_fDeltaTime ) override { return TFALSE; } // +0x170
	virtual void  InitDeathState() override;                                         // +0x18C
	virtual void  InitAlarmState() override;                                         // +0x194
	virtual TBOOL UpdateAlarmState( TFLOAT a_fDeltaTime ) override;                  // +0x198

	virtual void SetActive( TBOOL a_bActive, TBOOL a_bUnk ); // +0x19C
	// $deBlob: FUNCTION 007a8bb0
	virtual void SetCustomData( TINT a_iCustomData ) {} // +0x1A0
	virtual void UpdateMovement( TFLOAT a_fDeltaTime ); // +0x1A4
	// $deBlob: FUNCTION 0077d240
	virtual TBOOL ShouldFaceEnemy() { return TFALSE; }                                 // +0x1A8
	virtual void  TakeDamage( TINT16 a_iDamage );                                      // +0x1AC
	virtual void  OnDeath( const AEntityIOPacket& a_rPacket, ABaseEntity* a_pKiller ); // +0x1B0
	// $deBlob: FUNCTION 0077d130
	virtual TBOOL IsInvulnerable() { return TFALSE; } // +0x1B4
	// $deBlob: FUNCTION 0077d250
	virtual TBOOL ShouldFlee( TINT a_iEnemy ) { return TFALSE; } // +0x1B8
	virtual TBOOL ShouldIgnoreBlob( void* a_pEntity );           // +0x1BC
	// $deBlob: FUNCTION 0077cd90
	virtual EnemyData* GetEnemyData() { return TSTATICCAST( EnemyData, m_pTypeData->m_pTRBData ); } // +0x1C0
	// $deBlob: FUNCTION 0077cfa0
	virtual TINT GetEnemyType() { return GetEnemyData()->m_iEnemyType; } // +0x1C4 AEnemyManager::EnemyType
	// $deBlob: FUNCTION 0077cdb0
	virtual TFLOAT GetFidgetFreq() { return GetEnemyData()->m_fFidgetFreq; } // +0x1C8
	// $deBlob: FUNCTION 0077cda0
	virtual TFLOAT GetAttackedTimeOut( TINT a_iSchedule, TINT a_iTask ) { return -1.0f; } // +0x1CC
	// $deBlob: FUNCTION 0077cd20
	virtual TINT GetAttackType( TINT a_iAttack ) { return 0; } // +0x1D0
	// $deBlob: FUNCTION 0077cd30
	virtual TINT GetAttackSchedule() { return 5; } // +0x1D4
	// $deBlob: FUNCTION 007a8ad0
	virtual TINT GetKillScore() { return -1; } // +0x1D8
	// $deBlob: FUNCTION 0077ce00
	virtual TFLOAT GetSpawnRange() { return GetEnemyData()->m_fSpawnRange; } // +0x1DC
	// $deBlob: FUNCTION 0077cf30
	virtual TFLOAT GetPatrolRadius() { return GetEnemyData()->m_fPatrolRadius; } // +0x1E0
	virtual void   UpdateLowLOD( TFLOAT a_fDeltaTime );                          // +0x1E4
	virtual void   UpdateMediumLOD( TFLOAT a_fDeltaTime );                       // +0x1E8
	virtual TBOOL  CanPlayFallSchedule();                                        // +0x1EC
	virtual void   SetAlpha( TFLOAT a_fUnk );                                    // +0x1F0
	// $deBlob: FUNCTION 0077d260
	virtual void OnInactiveTimeOut() { m_uiEnemyFlags |= EFLAG_InactiveTimedOut; } // +0x1F4
	// $deBlob: FUNCTION 007a83a0
	virtual void Unk1F8() { InitIdleState(); } // +0x1F8
	// $deBlob: FUNCTION 007a83b0
	virtual TBOOL Unk1FC( TFLOAT a_fDeltaTime )
	{
		TBOOL bResult = TFALSE;

		if ( 0 != m_iState )
			bResult = TTRUE;

		return bResult;
	} // +0x1FC
	virtual void  InitAngryState();                                       // +0x200
	virtual TBOOL UpdateAngryState( TFLOAT a_fDeltaTime );                // +0x204
	// $deBlob: FUNCTION 0077cff0
	virtual void Unk208() {} // +0x208
	// $deBlob: FUNCTION 0077d2b0
	virtual TBOOL Unk20C( TFLOAT a_fDeltaTime ) { return TTRUE; } // +0x20C
	// $deBlob: FUNCTION 0077d000
	virtual void Unk210() {} // +0x210
	// $deBlob: FUNCTION 0077d2c0
	virtual TBOOL Unk214( TFLOAT a_fDeltaTime ) { return TTRUE; } // +0x214
	// $deBlob: FUNCTION 0077d2d0
	virtual void Unk218( TFLOAT a_fDeltaTime ) {} // +0x218
	// $deBlob: FUNCTION 007a8550
	virtual void  InitFallState() { SetState( COMMONSTATE_Fall ); } // +0x21C
	virtual TBOOL UpdateFallState( TFLOAT a_fDeltaTime );           // +0x220
	// $deBlob: FUNCTION 0077cf50
	virtual void* GetSplatEffect() { return TNULL; } // +0x224

	// Creates the component a_eType (ComponentType), returns it
	void* CreateComponent( ComponentType a_eType );
	void* GetComponent( ComponentType a_eType );

	// Gives the enemy back to its wrapper (MESSAGE_ENEMYRELEASED) or to AEnemyManager
	void  ReleaseToWrapper();
	TBOOL UpdateMusicMode( TINT a_iLOD, TBOOL a_bHasEnemy );
	void  EndAtomState();
	void  UpdateFallTime( TFLOAT a_fDeltaTime );
	void  UpdateActiveTime( TFLOAT a_fDeltaTime );
	void  SetTimeRatio( TFLOAT a_fCurrentTime, TFLOAT a_fMaxTime );
	TBOOL IsInSpawnRange( const Toshi::TVector4& a_rPosition, TBOOL a_bReduced );

	void SpawnPickups();
	void SpawnDecal();

	TBOOL IsActive() const { return ( m_uiEnemyFlags & EFLAG_Active ) != 0; }

	// $deBlob: FUNCTION 007d0ca0
	ABaseEntity* GetSquad() const { return m_pSquad; }

	// The physics body of a blob
	// NOTE: ABlobEntity::m_pBody (0x3E8) is private and ABlobEntity.h belongs to the blob's port; retail reads the member
	static Toshi::TPhysicsObjectContainer* GetBlobBody( ABaseEntity* a_pBlob )
	{
		return *TREINTERPRETCAST( Toshi::TPhysicsObjectContainer**, TREINTERPRETCAST( TUINT8*, a_pBlob ) + 0x3E8 );
	}
	// $deBlob: FUNCTION 007d0bf0
	TFLOAT GetClosestEnemyDist() const { return m_fClosestEnemyDist; }
	void   SetClosestEnemyDist( TFLOAT a_fDist ) { m_fClosestEnemyDist = a_fDist; }

	static TBOOL IsRecoverState( TINT a_iState );

	const Toshi::TVector4& GetSpawnPosition() const { return m_vSpawnPosition; }

	friend class AEnemyManager;

protected:
	TUINT8                           m_uiPickupType;              // 0xBC (CreateEnemy) 0xFF
	TUINT8                           m_uiMusicMode;               // 0xBD AEnemyManager::UpdateEnemyMusicMode
	TFLOAT                           m_fFallTime;                 // 0xC0
	Toshi::TVector4                  m_vSpawnPosition;            // 0xC4
	TINT16                           m_iNumPickups;               // 0xD4 1
	TUINT16                          m_uiEnemyFlags;              // 0xD6 EFLAG
	ABaseEntity*                     m_pSquad;                    // 0xD8 its ASquad (the spawner of CreateEnemy)
	TFLOAT                           m_fClosestEnemyDist;         // 0xDC TMath::MAXFLOAT, the closest enemy added (AEnemyBehaviour::AddedNewEnemy)
	TFLOAT                           m_fActiveTime;               // 0xE0
	ABaseEntity*                     m_pWrapper;                  // 0xE4 AEnemyWrapper
	Toshi::TPhysicsPhantomContainer* m_pEnemyLiquidRayCastHelper; // 0xE8 the phantom of the liquid ray cast (COMMONSTATE_Fall)
};

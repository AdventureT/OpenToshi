#pragma once
#include "Console/AConsoleVar.h"
#include "Entities/AI/Behaviour/AAIBehaviour.h"
#include "Collision/ACollisionFunctor.h"

class AEnemy;
class AEnemyTarget;
struct AEntityIOPacket;

//-----------------------------------------------------------------------------
// Purpose: the line of sight functor of the enemies (4 bytes): the visible entities block the sight.
//-----------------------------------------------------------------------------
class AEnemyLOSCollisionFunctor
    : public ACollisionFunctor
{
public:
	// NOTE: the base's HitNonSolidObject returns TFALSE
	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override { return TFALSE; } // +0x10
	virtual TBOOL HitIgnorableObject( ACollisionObject* a_pObject ) override;                  // +0x18
	// $deBlob: FUNCTION 007d0b10
	// $deBlob: FUNCTION 007d09e0
	virtual ~AEnemyLOSCollisionFunctor() override = default; // +0x20
};


//-----------------------------------------------------------------------------
// Purpose: the behaviour of the INKT enemies (Enemies\AEnemyBehaviour.cpp): their enemies (the blobs, AEnemyTarget),
// threat ratings, attack ranges and the schedule they choose next.
//-----------------------------------------------------------------------------
class AEnemyBehaviour
    : public AAIBehaviour
{
public:
	AEnemyBehaviour( ABaseEntity* a_pEntity );
	virtual ~AEnemyBehaviour() override; // +0x00

	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x04
	virtual void  ResetComponent() override;                                                           // +0x08
	virtual TBOOL IsEnabled() override;                                                                // +0x0C
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x10
	virtual void  OnScheduleFinished( TINT a_iSchedule, TINT a_iResult, TINT a_iTask ) override;       // +0x18
	virtual void  SelectSchedule() override;                                                           // +0x1C
	virtual void  UpdateEnvironment() override;                                                        // +0x20
	// $deBlob: FUNCTION 007cf6e0
	virtual void   OnDeath() override { SetSelectedEnemy( TNULL ); }                                    // +0x24
	virtual void   SetSelectedEnemy( ABaseEntity* a_pEnemy ) override;                                  // +0x28
	virtual TBOOL  IsEnemy( ABaseEntity* a_pEntity ) override;                                          // +0x2C
	virtual TBOOL  GetChooseAttackRange( TFLOAT* a_pMin, TFLOAT* a_pMax, TINT a_iAttackType ) override; // +0x34
	virtual TBOOL  GetAttackRange( TFLOAT* a_pMin, TFLOAT* a_pMax, TINT a_iAttackType ) override;       // +0x38
	virtual TFLOAT GetAttackRangeOptimal() override;                                                    // +0x3C
	virtual void   JustBeenAttacked( ABaseEntity* a_pAttacker ) override;                               // +0x4C
	// $deBlob: FUNCTION 007d0c30
	virtual void*  GetLOSCollisionFunctor() override { return sm_pLOSCollisionFunctor; }                                   // +0x54
	virtual TFLOAT GetAttackedTimeOut( TINT a_iSchedule, TINT a_iTask ) override;                                          // +0x5C
	virtual TFLOAT GetBeenAttackedTime() override;                                                                         // +0x60
	virtual TFLOAT CalculateEnemyThreat( Enemy* a_pEnemy, TFLOAT a_fMaxDistance, TFLOAT a_fMaxFrustration ) override;      // +0x64
	virtual void   AddedNewEnemy( Enemy* a_pEnemy ) override;                                                              // +0x68
	virtual TBOOL  GetAttackPoint( ABaseEntity* a_pEntity, Toshi::TVector4* a_pPoint, TFLOAT a_fGroundDistance ) override; // +0x6C

	virtual TBOOL ShouldSelectNewEnemy( TBOOL a_bSelect ); // +0x70
	// $deBlob: FUNCTION 007d0c60
	virtual TINT         GetSelectEnemyType() { return 0; }                                                                                        // +0x74
	virtual ABaseEntity* SelectEnemy();                                                                                                            // +0x78
	virtual TINT         SelectIdleSchedule();                                                                                                     // +0x7C
	virtual TINT         SelectAttackSchedule();                                                                                                   // +0x80
	virtual void         GetThreatFactors( TFLOAT* a_pUnk1, TFLOAT* a_pUnk2, TFLOAT* a_pUnk3, TFLOAT* a_pUnk4, TFLOAT* a_pUnk5, TFLOAT* a_pUnk6 ); // +0x84
	// $deBlob: FUNCTION 007d0c50
	virtual TINT GetMaxMobSquadSize() { return 5; } // +0x88
	// $deBlob: FUNCTION 007d0c90
	virtual TINT GetMobBehaviourType() { return 2; } // +0x8C

	// Caches the owner's schedule and navigator
	void CacheComponents();

	// Updates the threat ratings of the known enemies (AEnemyManager spreads it over the frames)
	void UpdateEnemyList( TFLOAT a_fDeltaTime );

	AEnemy* GetEnemyOwner();

	// $deBlob: FUNCTION 007ced00
	AEnemyTarget* GetTargetChar() const { return m_pTargetChar; }

	// Where the enemy (default: the selected one) will be in a_fTime seconds, from a_pFrom (default: its position);
	// without an enemy, ahead of the owner at its attack range
	Toshi::TVector4 GetEnemyPredictedPosition( ABaseEntity* a_pEnemy, TFLOAT a_fTime, const Toshi::TVector4* a_pFrom );
	TBOOL           FaceEnemyPredictedPosition( TFLOAT a_fTime, TBOOL a_bImmediate, ABaseEntity* a_pEnemy );

	// A point to surround the enemy (default: the selected one): 0 got one, 1 not this frame, 4 no enemy, 5 not a
	// player (m_iSurroundResult keeps the code)
	TINT GetSurroundPoint( Toshi::TVector4& a_rvOutPoint, ABaseEntity* a_pEnemy, TFLOAT a_fUnk3, TBOOL a_bUnk4, TBOOL a_bUnk5, TBOOL a_bUnk6, TBOOL a_bUnk7, const Toshi::TVector4& a_rcUnk8 );

	void SetSurroundResult( TINT a_iResult ) { m_iSurroundResult = a_iResult; }

	// Is the line from the owner's eyes to a_pEnemy (a blob) free?
	TBOOL CanSeeEnemy( ABaseEntity* a_pEnemy );

	// The frustration of the selected enemy grows (by Enemies_data's step, up to its maximum) or is reset
	void UpdateFrustration( TBOOL a_bReset );

	// Killed: no enemy, the death schedule
	void  BeenHit( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket );
	TBOOL CanChooseSchedule();

	static void InitialiseStatic();
	static void DeinitialiseStatic();
	static void PauseForCutscene( TBOOL a_bPause );

private:
	static void* sm_pLOSCollisionFunctor; // AEnemyLOSCollisionFunctor (4 bytes, a vtable)
	static TBOOL sm_bPausedForCutscene;

	void*         m_pSchedule;       // 0x34 the owner's schedule (CacheComponents)
	void*         m_pNavigator;      // 0x38 the owner's AAINavigator (CacheComponents)
	TINT          m_iSurroundResult; // 0x3C -1
	AEnemyTarget* m_pTargetChar;     // 0x40 the target of the selected enemy (AEnemyManager::FindTarget)
};

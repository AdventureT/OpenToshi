#pragma once
#include "Entities/AEntityComponent2.h"

#include <Toshi/T2DList.h>
#include <Toshi/T2ObjectPool.h>
#include <Toshi/T2SortedList.h>
#include <Math/TVector4.h>

class ACharEntity;
class ACollisionObject;

//-----------------------------------------------------------------------------
// Purpose: the base of the AI behaviours (abstract: +0x1C and +0x2C are pure): the known enemies of the character
// (m_Enemies), the selected one and the timers of the last attack; picks the next schedule (AAISchedule).
//-----------------------------------------------------------------------------
class AAIBehaviour
    : public AEntityComponent2
{
public:
	// A known enemy (0x20 bytes, from m_pEnemyPool)
	struct Enemy
	    : public Toshi::T2DList<Enemy>::Node
	{
		// m_Enemies is sorted by threat, the highest first
		struct SortResults
		{
			TINT operator()( const Enemy& a_rcVal1, const Enemy& a_rcVal2 ) const
			{
				return ( a_rcVal2.m_fThreatRating < a_rcVal1.m_fThreatRating ) ? -1 : 1;
			}
		};

		ABaseEntity* m_pEnemy        = TNULL;  // 0x08
		TBOOL        m_bNew          = TFALSE; // 0x0C TTRUE once added (the alert schedule is chosen for it)
		TFLOAT       m_fLostTime     = 0.0f;   // 0x10 out of sight or range for that long (GetLOSMemoryTime)
		TFLOAT       m_fThreatRating = 0.0f;   // 0x14
		TFLOAT       m_fDistanceSq   = 0.0f;   // 0x18 squared from the owner when it was added, then the distance
		TFLOAT       m_fFrustration  = 0.0f;   // 0x1C counted down (AEnemyBehaviour::UpdateFrustration)
	};

	using EnemyPool = Toshi::T2ObjectPool<Enemy, 4, 32, 4>;
	using EnemyList = Toshi::T2SortedList<Enemy, Toshi::T2DList<Enemy>, Enemy::SortResults>;

	static TCONSTEXPR_VAR TINT MAX_ENEMIES = 4;

	// m_uiFlags
	enum FLAG : TUINT32
	{
		FLAG_EnemyInSight = 0x1, // a known enemy is in range and sight (AEnemyBehaviour::UpdateEnemyList)
		FLAG_HasEnemies   = 0x2, // m_Enemies isn't empty
		FLAG_BeenAttacked = 0x4, // just been attacked (JustBeenAttacked)
		FLAG_Taunted      = 0x8,
		FLAG_PathFailed   = 0x10,
	};

public:
	AAIBehaviour( ABaseEntity* a_pEntity );
	virtual ~AAIBehaviour() override; // +0x00

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void ResetComponent() override;            // +0x08
	// $deBlob: FUNCTION 007e2880
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override {} // +0x10
	// $deBlob: FUNCTION 007d0dc0
	virtual void OnScheduleStarted( TINT a_eSchedule ) {}                              // +0x14
	virtual void OnScheduleFinished( TINT a_iSchedule, TINT a_iResult, TINT a_iTask ); // +0x18
	virtual void SelectSchedule() = 0;                                                 // +0x1C
	// $deBlob: FUNCTION 007e29c0
	virtual void UpdateEnvironment() {} // +0x20
	// $deBlob: FUNCTION 007e2920
	virtual void OnDeath() {} // +0x24
	// $deBlob: FUNCTION 007d0ef0
	virtual void  SetSelectedEnemy( ABaseEntity* a_pEnemy ) { m_pSelectedEnemy = a_pEnemy; } // +0x28
	virtual TBOOL IsEnemy( ABaseEntity* a_pEntity ) = 0;                                     // +0x2C
	// $deBlob: FUNCTION 007e1790
	// Chooses the attack (m_iAttackType): always 0
	virtual TINT ChooseAttack( TBOOL a_bUnk1, TINT a_iUnk2 ) // +0x30
	{
		if ( a_bUnk1 )
			m_iAttackType = 0;

		return 0;
	}
	// $deBlob: FUNCTION 007e19b0
	virtual TBOOL GetChooseAttackRange( TFLOAT* a_pMin, TFLOAT* a_pMax, TINT a_iAttackType ) // +0x34
	{
		*a_pMax = -1.0f;
		*a_pMin = -1.0f;
		return TFALSE;
	}
	// $deBlob: FUNCTION 007e19d0
	virtual TBOOL GetAttackRange( TFLOAT* a_pMin, TFLOAT* a_pMax, TINT a_iAttackType ) // +0x38
	{
		*a_pMax = -1.0f;
		*a_pMin = -1.0f;
		return TFALSE;
	}
	// $deBlob: FUNCTION 007e19f0
	virtual TFLOAT GetAttackRangeOptimal() { return 2.0f; } // +0x3C
	// Moves a_pPosition onto the ground below it (a_fDistance at most)
	virtual TBOOL EntityGroundPosition( Toshi::TVector4* a_pPosition, TFLOAT a_fDistance, ACollisionObject* a_pIgnore, TINT a_iUnk4 ); // +0x40
	virtual void  UpdateMemory( TFLOAT a_fDeltaTime );                                                                                 // +0x44
	// $deBlob: FUNCTION 007d0b90
	virtual TBOOL CustomizedUpdateEnvironment( TINT a_iUnk ) { return TFALSE; } // +0x48
	virtual void  JustBeenAttacked( ABaseEntity* a_pAttacker );                 // +0x4C
	// $deBlob: FUNCTION 007e1d00
	virtual void BeenAttackedTimedout() { m_uiFlags &= ~FLAG_BeenAttacked; } // +0x50
	// $deBlob: FUNCTION 007e2870
	virtual void* GetLOSCollisionFunctor() { return TNULL; } // +0x54
	// $deBlob: FUNCTION 007d0c40
	virtual TFLOAT GetLOSMemoryTime() { return 2.0f; } // +0x58
	// $deBlob: FUNCTION 007d0c20
	virtual TFLOAT GetAttackedTimeOut( TINT a_iSchedule, TINT a_iTask ) { return 0.3f; } // +0x5C
	// $deBlob: FUNCTION 007e2860
	virtual TFLOAT GetBeenAttackedTime() { return 3.5f; }                                                    // +0x60
	virtual TFLOAT CalculateEnemyThreat( Enemy* a_pEnemy, TFLOAT a_fMaxDistance, TFLOAT a_fMaxFrustration ); // +0x64
	// $deBlob: FUNCTION 007e2780
	virtual void  AddedNewEnemy( Enemy* a_pEnemy ) {}                                                            // +0x68
	virtual TBOOL GetAttackPoint( ABaseEntity* a_pEntity, Toshi::TVector4* a_pPoint, TFLOAT a_fGroundDistance ); // +0x6C

	TBOOL  IsKnownEnemy( ABaseEntity* a_pEntity );
	void   AddToEnemyList( ABaseEntity* a_pEntity );
	void   ClearEnemyList();
	Enemy* FindInEnemyList( ABaseEntity* a_pEnemy );
	void   RemoveFromEnemyList( Enemy* a_pEnemy );

	// The closest known enemy: by its current distance (a_bCurrent) or by the stored one
	Enemy* GetClosestEnemyNode( TBOOL a_bCurrent );
	// Its entity
	ABaseEntity* GetClosestEnemy( TBOOL a_bCurrent );
	// The squared distance of the closest known enemy, -1.0f without one
	TFLOAT GetDistanceSqToClosestEnemy( TBOOL a_bCurrent );

	// Is a_rcPosition in the attack range (the optimal distance as the maximum with a_bOptimal) of a_iAttackType
	// (-1: m_iAttackType), a_fRadius and the owner's size taken off?
	TBOOL WithinAttackRange( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TBOOL a_bOptimal, TINT a_iAttackType );

	// The same check with a given range (no range: both -1)
	TBOOL WithinRange( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fMin, TFLOAT a_fMax );

	// The point at the optimal attack range from the selected enemy, on the owner's side
	TBOOL GetOptimalAttackPosition( Toshi::TVector4& a_rvOutPosition );

	// Turns the owner to the selected enemy (when its navigator isn't moving it)
	// Turns the owner to a_pEntity: at once (the motor) or through the navigator
	TBOOL FaceEntity( ABaseEntity* a_pEntity, TBOOL a_bInstant );
	TBOOL FaceSelectedEnemy( TBOOL a_bUnk );

	// The selected enemy was just added (Enemy::m_bNew)
	TBOOL IsSelectedEnemyNew();
	void  ClearSelectedEnemyNew();
	void SetTaunted();
	// $deBlob: FUNCTION 007e1c90
	void ClearTaunted() { m_uiFlags &= ~FLAG_Taunted; }

	ACharEntity* GetCharOwner();

	ABaseEntity* GetSelectedEnemy() const { return m_pSelectedEnemy; }

	// $deBlob: FUNCTION 007e1ca0
	TBOOL HasPathFailed() const
	{
		TBOOL bFailed = ( m_uiFlags >> 4 ) & 1;
		return bFailed;
	}
	void  SetScheduleTimer( TFLOAT a_fTime ) { m_fScheduleTimer = a_fTime; }

protected:
	TFLOAT       m_fScheduleTimer; // 0x14
	TUINT32      m_uiFlags;        // 0x18 FLAG
	EnemyList    m_Enemies;        // 0x1C
	TINT         m_iAttackType;    // 0x24 -1
	TFLOAT       m_fAttackedTimer; // 0x28 BeenAttackedTimedout once it runs out
	ABaseEntity* m_pSelectedEnemy; // 0x2C
	EnemyPool*   m_pEnemyPool;     // 0x30
};

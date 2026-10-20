#pragma once
#include "Entities/AVisibleEntity.h"

class ACollisionObject;

//-----------------------------------------------------------------------------
// Purpose: a character driven by the AI (abstract: slots +0x108-+0x11C, +0x130, +0x134 and +0x13C are pure): owns a
// ground motor, a pathfinding, a behaviour and a schedule component and runs the states of its current AI task
// (StartStatesForTask, the Init/Update*State slots).
//-----------------------------------------------------------------------------
class ACharEntity
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( ACharEntity, AVisibleEntity );

	// AAITasks::AITASKS
	static TCONSTEXPR_VAR TINT8 AITASK_INVALID = -1;

	// The turning settings of TurnToFace (0x14 bytes)
	struct TurnSettings
	{
		TFLOAT m_fMinAngle;     // 0x00 below it the entity doesn't turn
		TFLOAT m_fMaxTurnSpeed; // 0x04
		TFLOAT m_fMinTurnSpeed; // 0x08
		TFLOAT m_fAcceleration; // 0x0C
		TFLOAT m_fDeceleration; // 0x10
	};

	enum TURNFLAG : TUINT8
	{
		TURNFLAG_Left         = 0x1,
		TURNFLAG_Right        = 0x2,
		TURNFLAG_Mask         = 0x3,
		TURNFLAG_AttackFailed = 0x4, // the attack failed (AAISchedule task 10)
	};

public:
	ACharEntity();
	virtual ~ACharEntity() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                       // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override;                      // +0x10
	virtual void Create() override;                                          // +0x1C
	virtual void SetPosition( const Toshi::TVector4& a_rPosition ) override; // +0x40
	// $deBlob: FUNCTION 0077cee0
	virtual void*           GetPathfinding() override { return m_pPathfinding; }                                 // +0x54
	virtual void            HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual Toshi::TVector4 GetReactionForce( void* a_pVelocity, void* a_pEntity ) override;                     // +0x60
	// $deBlob: FUNCTION 0077cd50
	virtual ACollisionObject* GetCollisionObject() override { return m_pCollisionObject; } // +0x70
	// $deBlob: FUNCTION 0077cf90
	virtual TFLOAT GetStepUpHeight() const override { return m_vBBox.y; } // +0x74

	// $deBlob: FUNCTION 007c7df0
	virtual void InitRecoverState() {} // +0xF0
	// Sends a_eMessage to the blobs inside the attack cone, returns how many were hit
	virtual TINT CheckMeleeCollision( TFLOAT a_fAngle, TFLOAT a_fMinDot, TFLOAT a_fRadius, TINT a_iUnk4, AEntityIOPacket::MESSAGE a_eMessage, TINT a_iAttackType, TINT a_iDamage, TINT a_iUnk8, TFLOAT a_fHeight ); // +0xF4
	// $deBlob: FUNCTION 0077cd00
	virtual void AdjustAttackMessage( void* a_pTarget, void* a_pMessage ) {} // +0xF8
	// $deBlob: FUNCTION 0077cf40
	virtual void* GetScheduleComponent() { return m_pScheduleComponent; } // +0xFC
	// $deBlob: FUNCTION 0077cd40
	virtual void*  GetBehaviourComponent() { return m_pBehaviourComponent; } // +0x100
	virtual void*  GetMotorComponent();                                      // +0x104
	virtual TFLOAT GetVisualRange()        = 0;                              // +0x108
	virtual TFLOAT GetWalkSpeed()          = 0;                              // +0x10C
	virtual TFLOAT GetRunSpeed()           = 0;                              // +0x110
	virtual TFLOAT GetFleeRange()          = 0;                              // +0x114
	virtual TFLOAT GetAttackRangeOptimal() = 0;                              // +0x118
	virtual TFLOAT GetAttackRangeMax()     = 0;                              // +0x11C
	// $deBlob: FUNCTION 0077cd10
	virtual TBOOL Unk120( TINT a_iUnk ) { return TTRUE; }                 // +0x120
	virtual void  HandleTaskFailed( TINT a_iTask );                       // +0x124
	virtual void  HandleTaskFinished( TINT a_iTask, TBOOL a_bNoAdvance ); // +0x128
	virtual void  InitialiseComponents();                                 // +0x12C
	virtual void  CreateBehaviourComponent() = 0;                         // +0x130
	virtual void  CreateScheduleComponent()  = 0;                         // +0x134
	virtual void  UpdateComponents( TFLOAT a_fDeltaTime );                // +0x138
	virtual TBOOL Unk13C( ABaseEntity* a_pEntity ) = 0;                           // +0x13C
	// $deBlob: FUNCTION 007c7e60
	virtual TBOOL UpdateKnock( TFLOAT a_fDeltaTime ) { return TFALSE; } // +0x140
	// $deBlob: FUNCTION 007c7d80
	virtual TFLOAT              GetRecoilFactor() { return 1.0f; }             // +0x144
	virtual void                UpdateFacing( TFLOAT a_fDeltaTime );           // +0x148
	virtual Toshi::TVector4     GetDesiredFacing();                            // +0x14C
	virtual const TurnSettings* GetTurnSettings();                             // +0x150
	virtual void                StartCurrentStateTask();                       // +0x154
	virtual TBOOL               UpdateCurrentStateTask( TFLOAT a_fDeltaTime ); // +0x158
	// $deBlob: FUNCTION 007c7dd0
	virtual void InitIdleState() {} // +0x15C
	// $deBlob: FUNCTION 007c7e50
	virtual TBOOL UpdateIdleState( TFLOAT a_fDeltaTime ) { return TFALSE; } // +0x160
	// $deBlob: FUNCTION 007c7dc0
	virtual void InitFidgetState() {} // +0x164
	// $deBlob: FUNCTION 007c7e40
	virtual TBOOL UpdateFidgetState( TFLOAT a_fDeltaTime ) { return TFALSE; } // +0x168
	// $deBlob: FUNCTION 007c7de0
	virtual void InitMoveState() {} // +0x16C
	// $deBlob: FUNCTION 007c7e70
	virtual TBOOL UpdateMoveState( TFLOAT a_fDeltaTime ) { return TFALSE; } // +0x170
	// $deBlob: FUNCTION 0077cfd0
	virtual void InitAttackState() {} // +0x174
	// $deBlob: FUNCTION 0077d270
	virtual TBOOL UpdateAttackState( TFLOAT a_fDeltaTime ) { return TTRUE; } // +0x178
	// $deBlob: FUNCTION 007c7d10
	virtual void InitRecoilState() { InitRecoverState(); } // +0x17C
	// $deBlob: FUNCTION 007c7d20
	virtual TBOOL UpdateRecoilState( TFLOAT a_fDeltaTime ) { return UpdateKnock( a_fDeltaTime ); } // +0x180
	// $deBlob: FUNCTION 0077cfe0
	virtual void InitBlockState() {} // +0x184
	// $deBlob: FUNCTION 0077d280
	virtual TBOOL UpdateBlockState( TFLOAT a_fDeltaTime ) { return TTRUE; } // +0x188
	// $deBlob: FUNCTION 007c7db0
	virtual void InitDeathState() {} // +0x18C
	// $deBlob: FUNCTION 0077d290
	virtual TBOOL UpdateDeathState( TFLOAT a_fDeltaTime ) { return TTRUE; } // +0x190
	// $deBlob: FUNCTION 007c7da0
	virtual void InitAlarmState() {} // +0x194
	// $deBlob: FUNCTION 007c7e30
	virtual TBOOL UpdateAlarmState( TFLOAT a_fDeltaTime ) { return TTRUE; } // +0x198

	// Starts a_iTask (AAITasks::AITASKS) and its states
	void StartStatesForTask( TINT a_iTask );
	void StartInvalidTask();
	void ForceEndStatesForTask();
	void UpdateStatesForTask( TFLOAT a_fDeltaTime );

	// Turns the entity towards a_rvTarget (normalised XZ) as a_rSettings allow
	void TurnToFace( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rvTarget, const TurnSettings& a_rSettings );

	Toshi::TVector4 GetEyePosition() const;
	TBOOL           CanOccupyPosition( const Toshi::TVector4& a_rPosition );

	// $deBlob: FUNCTION 007c7e00
	void SetLife( TINT16 a_iLife ) { m_iLife = a_iLife; }
	// $deBlob: FUNCTION 007c7e10
	void SetTurningDirection( TUINT8 a_uiDirection ) { m_uiTurnFlags = ( m_uiTurnFlags & ~TURNFLAG_Mask ) | a_uiDirection; }

	TINT   GetCurrentTask() const { return m_iCurrentTask; }
	TUINT8 GetTurnFlags() const { return m_uiTurnFlags; }
	void   ClearTurnFlags( TUINT8 a_uiFlags ) { m_uiTurnFlags &= ~a_uiFlags; }

	static void InitialiseStatic();
	static void DeinitialiseStatic();

protected:
	void AddComponents();
	void CreateMotorComponent();
	void CreatePathfindingComponent();

protected:
	static void* ms_pEntityData; // "GlobalData_ACharEntityData_EntityData"

	TFLOAT             m_fTurnSpeed;          // 0x9C
	AEntityComponent2* m_pMotorComponent;     // 0xA0 AGndMotorComponent (0x284 bytes)
	ACollisionObject*  m_pCollisionObject;    // 0xA4
	AEntityComponent2* m_pBehaviourComponent; // 0xA8 (AEnemyBehaviour for enemies)
	AEntityComponent2* m_pScheduleComponent;  // 0xAC AAISchedule
	AEntityComponent2* m_pPathfinding;        // 0xB0 APathfindingComponent (0x48 bytes)
	ABaseEntity*       m_pUnkB4;              // 0xB4 the sender of the last MESSAGE_UNK31
	TUINT8             m_uiTurnFlags;         // 0xB8 TURNFLAG
	TINT8              m_iCurrentTask;        // 0xB9 AAITasks::AITASKS
};

#pragma once
#include "Entities/AProp.h"
#include "Collision/ACollisionInstance2.h"

#include <Sound/TSound.h>
#include <Physics/TPhysicsResponder.h>
#include <Toshi/T2DList.h>

TOSHI_NAMESPACE_START
class TParticleEffect;
class TPhysicsMotionBase;
TOSHI_NAMESPACE_END

class AEntityModelComponent;

//-----------------------------------------------------------------------------
// Purpose: a pressure switch, pressed by the blob (APropSwitchPressure\APropSwitchPressure.cpp, plus ATriggerUtil's 6
// at 0x9C); a sector switch is locked behind the sector's unlock score.
//-----------------------------------------------------------------------------
class APropSwitchPressure
    : public AProp
{
public:
	TDECLARE_CLASS( APropSwitchPressure, AProp );

	enum SWITCHFLAG : TUINT16
	{
		SWITCHFLAG_TriggerOnce          = 0x1,
		SWITCHFLAG_TriggerOnRelease     = 0x2,
		SWITCHFLAG_StateDependent       = 0x4,
		SWITCHFLAG_Slam                 = 0x8, // "Slam"
		SWITCHFLAG_Targetable           = 0x10,
		SWITCHFLAG_TargetableFromBelow  = 0x20,
		SWITCHFLAG_Triggered            = 0x40, // the press animation started
		SWITCHFLAG_Pressed              = 0x80,
		SWITCHFLAG_SingleUse            = 0x100,
		SWITCHFLAG_SoundPlaying         = 0x200,
		SWITCHFLAG_Locked               = 0x400, // INITIALLY_LOCKED
		SWITCHFLAG_SectorSwitch         = 0x800,
		SWITCHFLAG_ChallengeEndPending  = 0x1000, // the challenge end scene is pending (a gate opened)
		SWITCHFLAG_UnlockWaitsForCamera = 0x2000, // the unlock waits for its scripted camera
	};

	enum STATE : TINT
	{
		STATE_Pressing    = 0,
		STATE_Pressed     = 1,
		STATE_Releasing   = 2,
		STATE_Idle        = 3,
		STATE_WaitRelease = 4,
		STATE_Locked      = 5,
	};

	// A sector switch in the level's list of sector switches (AGameStatus::LevelData+0x784), sorted by unlock score
	// (0x10 bytes)
	class SectorSwitchNode
	    : public Toshi::T2GenericDList::Node
	{
	public:
		SectorSwitchNode( APropSwitchPressure* a_pSwitch, ABaseEntity* a_pDoor );
		~SectorSwitchNode();

	public:
		APropSwitchPressure* m_pSwitch; // 0x08
		ABaseEntity*         m_pDoor;   // 0x0C the APropDoor target
	};

	// Receives the physics body's trigger events and sends them to the switch
	class APropSwitchPressureCollisionEventHandler
	    : public Toshi::TPhysicsResponder
	{
	public:
		APropSwitchPressureCollisionEventHandler( APropSwitchPressure* a_pSwitch )
		    : m_pSwitch( a_pSwitch ) {}

		// $deBlob: FUNCTION 0079f350
		// $deBlob: FUNCTION 0079f300
		virtual ~APropSwitchPressureCollisionEventHandler() override = default;

		virtual void OnTriggerEnter( Toshi::TPhysicsObjectContainer* a_pBody, Toshi::TPhysicsObjectContainer* a_pOther ) override; // +0x20
		virtual void OnTriggerLeave( Toshi::TPhysicsObjectContainer* a_pBody, Toshi::TPhysicsObjectContainer* a_pOther ) override; // +0x24

	public:
		APropSwitchPressure* m_pSwitch; // 0x04
	};

	// The fixed body of the switch (0x94 bytes, the same as APropTrigger's)
	struct PhysicsData
	{
		Toshi::TMatrix44                          m_oMatrix;        // 0x00 the body's matrix (the bottom of the box)
		TUINT8                                    m_aUnk40[ 0x40 ]; // 0x40
		ACollisionInstance2::OwnerInfo            m_oOwnerInfo;     // 0x80 the body's user data { 0, this }
		Toshi::TPhysicsObjectContainer*           m_pBody;          // 0x88
		APropSwitchPressureCollisionEventHandler* m_pHandler;       // 0x8C
		Toshi::TPhysicsMotionBase*                m_pMover;         // 0x90 (never set)
	};

public:
	APropSwitchPressure();
	virtual ~APropSwitchPressure() override; // +0x08

	// The linked door is touched while it closes (APropDoor::HandleEntityIO): the switch plays its release animation
	void ReleaseByDoor();

	virtual void            Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void            OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void            PostCreate() override;                                                               // +0x20
	virtual void            ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void            SetVisible( TBOOL a_bVisible ) override;                                             // +0x38
	virtual void            SetPosition( const Toshi::TVector4& a_rPosition ) override;                          // +0x40
	virtual void            SetMatrix( const Toshi::TMatrix44& a_rMatrix ) override;                             // +0x48
	virtual void            HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void            AddToPhysics() override;                                                             // +0x88
	virtual void            RemoveFromPhysics() override;                                                        // +0x8C
	virtual TBOOL           IsTargetable() override;                                                             // +0xA0
	virtual TBOOL           CanBeTargetedFromBelow() override;                                                   // +0xA4
	virtual Toshi::TVector4 GetTargetOffset() override;                                                          // +0xAC
	virtual void            Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual void            Despawn( TBOOL a_bPlayEffect ) override;                                             // +0xE4
	virtual void*           GetDespawnEffect() override;                                                         // +0xE8

	// $deBlob: FUNCTION 0079f420
	TBOOL IsLocked() const { return m_eState == STATE_Locked; }
	// $deBlob: FUNCTION 0079f4b0
	void SetSound( FMOD::Event* a_pSound ) { m_pSound = a_pSound; }
	// ATriggerUtil
	virtual void TriggerAllTargets( ABaseEntity* a_pCaller ) override; // +0x0C

private:
	// Creates the fixed body from the entity data's matrix and box
	void CreatePhysics( const AEntityParams& a_rParams );
	void DestroyPhysics();
	// Moves the body (and the shield) to the entity's matrix
	void UpdatePhysics();
	// Opens the locked switch: the shield disappears, the explosion plays
	void Unlock();
	// The sector switch is pressed while locked: the message with the missing points
	void UpdateUnlockMessage();
	// The countdown sound while the switch waits to release (event group 8, 0x92)
	void StartCountdownSound();
	void UpdateCountdownSound();
	void StopCountdownSound();

private:
	static Toshi::TParticleEffect* sm_explosionEffect; // "Blob_Explosion"
	static Toshi::TParticleEffect* sm_despawnEffect;
	static TINT                    sm_iSoundMode; // how the press sound's parameter grows

private:
	STATE                  m_eState;                    // 0xB4
	TINT16                 m_iPressAnim;                // 0xB8 "anim_press"
	TINT16                 m_iReleaseAnim;              // 0xBA "anim_release"
	TFLOAT                 m_fDeactivationDelay;        // 0xBC "DeactivationDelay"
	TFLOAT                 m_fDeactivationTimer;        // 0xC0 the delay's countdown
	TFLOAT                 m_fPressTimer;               // 0xC4 the time to wait before the next state
	FMOD::Event*           m_pSound;                    // 0xC8
	PhysicsData*           m_pPhysicsData;              // 0xCC
	TINT                   m_iUnlockScore;              // 0xD0
	SectorSwitchNode*      m_pSectorNode;               // 0xD4 0x10 bytes
	AEntityModelComponent* m_pShield;                   // 0xD8 "Landmark_target_shield.trb"
	TWCHAR                 m_wszUnlockMessage[ 0x100 ]; // 0xDC
	ABaseEntity*           m_pPresser;                  // 0x2DC
	TUINT16                m_uiSwitchFlags;             // 0x2E0 SWITCHFLAG
	TINT8                  m_iSectorIndex;              // 0x2E2 "SectorIndex"
	TUINT8                 m_uiUnlockType;              // 0x2E3 "Unlock Type"
	TINT                   m_iScriptedSequence;         // 0x2E4 "Sector Unlock Scripted Sequence"
	TFLOAT                 m_fUnlockDelay;              // 0x2E8 "Unlock delay"
	TFLOAT                 m_fUnlockTimer;              // 0x2EC the unlock delay's countdown
};

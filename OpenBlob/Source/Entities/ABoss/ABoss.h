#pragma once
#include "Entities/AVisibleEntity.h"

#include <Math/TQuaternion.h>

class ACollisionObject;
class ABlobEntity;
class APropWater;
class AContainerHazard;

namespace Toshi
{
class TAnimation;
class TXUIScene;
class TXUIElement;
class TXUIFigure;
} // namespace Toshi

// Is the blob unable to be shot (dead, about to respawn or without control)?
TBOOL IsBlobUnavailable( ABlobEntity* a_pBlob );

//-----------------------------------------------------------------------------
// Purpose: the boss of the last level (Spaceship, SPLevel 9): Comrade Black's mech. It turns to face Blob and fires its
// gun hands at him (ink projectiles) while the boss triggers of the round (ABossTrigger, raised platforms whose
// buildings Blob must paint) are up; once every trigger of the round is complete it's stunned and Blob can Z-jump into
// it (its life, m_iLife, is shown on the "LandmarkTarget" HUD scene over its hat). Four rounds (BOSSFIGHT_NUM_ROUNDS),
// each with its own shot counts and timings from the type data. Its instance is kept in sm_pBoss.
//-----------------------------------------------------------------------------
class ABoss
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( ABoss, AVisibleEntity );

	static TCONSTEXPR_VAR TINT BOSSFIGHT_NUM_ROUNDS = 4;

	enum STATE
	{
		STATE_Inactive     = 0, // hidden and not ticking until it's triggered (HandleEntityIO MESSAGE_TRIGGER)
		STATE_Intro        = 1, // the intro (APlayerManager's scripted camera "Intro")
		STATE_RoundStart   = 2, // raises the boss triggers of the round when done
		STATE_RoundStarted = 3,
		STATE_Attack       = 4, // aims the eye, the hands and the body at Blob and fires
		STATE_ChargeStart  = 5, // the round's shots are fired
		STATE_Charge       = 6, // m_fChargeTime
		STATE_ChargeEnd    = 7,
		STATE_TurnStart    = 8, // Blob is behind it for too long: turns towards him
		STATE_Turn         = 9,
		STATE_TurnEnd      = 10,
		STATE_StunStart    = 11, // every boss trigger of the round is complete (OnBossTriggerCompleted)
		STATE_Stunned      = 12, // targetable by Blob's Z-jump, the life HUD is shown
		STATE_Recover      = 13, // the stun time is over: the triggers of the round go up again
		STATE_Hit          = 14, // Blob's Z-jump hit it (Blob is in ABlobEntity::STATE_BOSS)
		STATE_HitHold      = 15,
		STATE_HitRecover   = 16, // it survived the hit (OnHit)
		STATE_RoundEnd     = 17, // its life for the round is over (OnHit): the next round
		STATE_NextRound    = 18,
		STATE_Defeated     = 19, // the last round is over (APlayerManager's scripted camera "Death")
		STATE_DefeatedEnd  = 20,
	};

	// m_uiBossFlags
	enum BOSSFLAG : TUINT8
	{
		BOSSFLAG_TurnRight    = 0x1, // the side of the last turn towards Blob
		BOSSFLAG_BlobBehind   = 0x2, // Blob is behind the boss (more than 90 degrees from its facing)
		BOSSFLAG_MusicStarted = 0x4, // set once the last grass bomb sequence is started (STATE_Defeated)
		BOSSFLAG_ShotFired    = 0x8, // a hand just fired (m_iLastShotBone)
	};

	// The derived part of the type data (after AEntityTypeData::TRBData)
	struct BossData
	    : public AEntityTypeData::TRBData
	{
		struct Round
		{
			TFLOAT m_fTurnSpeed;  // 0x00 the body's turn towards Blob (radians per second)
			TINT8  m_iNumShots;   // 0x04 the shots of a series
			TFLOAT m_fShotDelay;  // 0x08 the time between two shots
			TFLOAT m_fChargeTime; // 0x0C the charge after a series (m_fChargeTime)
		};

		TFLOAT m_fAimSpeedH;                      // 0x54 the speed of the hands' horizontal aim (per second)
		TFLOAT m_fAimSpeedV;                      // 0x58 the speed of the hands' vertical aim (per second)
		TFLOAT m_fAimLimitH;                      // 0x5C the hands' horizontal aim limit (0-1)
		TFLOAT m_fStunTime;                       // 0x60 STATE_Stunned's duration
		Round  m_aRounds[ BOSSFIGHT_NUM_ROUNDS ]; // 0x64
		TBOOL  m_bTurnToBlob;                     // 0xA4 turns when Blob is behind it for m_fTurnDelay
		TFLOAT m_fTurnDelay;                      // 0xA8
		TFLOAT m_fTurnAngle;                      // 0xAC degrees
		TFLOAT m_fTurnSpeed;                      // 0xB0 degrees per second
	};

public:
	ABoss();
	virtual ~ABoss() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void Create() override;                                                                   // +0x1C
	virtual void PostCreate() override;                                                               // +0x20
	virtual void Destroy() override;                                                                  // +0x24
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	// $deBlob: FUNCTION 00772ea0
	virtual ACollisionObject* GetCollisionObject() override { return m_pCollision; } // +0x70
	virtual TBOOL             IsTargetable() override;                               // +0xA0
	virtual Toshi::TVector4   GetTargetOffset() override;                            // +0xAC
	// $deBlob: FUNCTION 00770c00
	virtual TINT GetTargetColour() override { return 16; } // +0xB4
	// The life of a round (m_iLife is the life left)
	// $deBlob: FUNCTION 00772ed0
	virtual TINT         GetMaxLife() override { return 50; }      // +0xB8
	virtual void         Reset( ABaseEntity* a_pEntity ) override; // +0xC8
	virtual void         OnLeaveState( TINT a_iState ) override;   // +0xD4
	virtual void         OnEnterState( TINT a_iState ) override;   // +0xD8
	virtual const TCHAR* GetModelName() override;                  // +0xEC

	// The third person camera of the stun (ABlobFollowCamera::Update's position handler 9): in front of the boss,
	// looking at it; returns TFALSE outside STATE_StunStart/STATE_Stunned (while flashing)
	TBOOL GetStunCamera( Toshi::TVector4* a_pOutPosition, Toshi::TQuaternion* a_pOutOrientation, TFLOAT* a_pOutFOV, TFLOAT* a_pOutFOVSpeed, TBOOL* a_pOutSnap );

	// Blob hit the stunned boss: a_bKilled the round's life is over (STATE_RoundEnd), else STATE_HitRecover
	void OnHit( TBOOL a_bKilled );

	// A boss trigger of the round is complete: the boss is stunned once all of them are
	void OnBossTriggerCompleted();

	// The eye bone's position, a_fDistance + 5 in front of it (along the facing)
	void GetEyeFrontPosition( Toshi::TVector4& a_rOutPosition, TFLOAT a_fDistance );

	// The eye bone's position
	void GetEyePosition( Toshi::TVector4& a_rOutPosition );

	// The life left (m_iLife)
	TINT GetLife() const { return m_iLife; }
	void SetLife( TINT a_iLife ) { m_iLife = TINT16( a_iLife ); }

	static ABoss* GetSingleton() { return sm_pBoss; }

private:
	BossData* GetBossData() const { return TSTATICCAST( BossData, GetTypeData()->m_pTRBData ); }

	void CreateCollision();
	void SetupRound();
	void RaiseBossTriggers();

	// Turns the body animation (and the boss when it's off) towards Blob
	void UpdateBodyTurn( TFLOAT a_fDeltaTime, ABlobEntity* a_pBlob );
	// Aims a hand at Blob and fires it when it's on target
	void UpdateHand( TFLOAT a_fDeltaTime, TINT16* a_pBone, ABlobEntity* a_pBlob, Toshi::TAnimation* a_pVAnim, TFLOAT* a_pfVTarget, TFLOAT* a_pfV, TFLOAT* a_pfVLength, Toshi::TAnimation* a_pHAnim, TFLOAT* a_pfHTarget, TFLOAT* a_pfH, TFLOAT* a_pfHLength, TINT16* a_pShootAnimID, TBOOL a_bLeft );
	// Aims the eye at Blob
	void UpdateEye( TFLOAT a_fDeltaTime, ABlobEntity* a_pBlob );
	// The life HUD (m_pScene) over the hat, visible in STATE_Stunned
	void UpdateHUD();

	// The timer of the next shot
	// $deBlob: FUNCTION 00772680
	TFLOAT GetShotTimer() const { return ( 0 < m_iShotCount ) ? m_fShotTimer : m_fChargeTime; }

	void ResetAimTargets();

private:
	static ABoss* sm_pBoss;
	// The boss material's white pass is set up once
	static TBOOL ms_bMaterialSetup;

private:
	ACollisionObject*   m_pCollision;            // 0x9C
	Toshi::TAnimation*  m_pVEyeAnim;             // 0xA0
	Toshi::TAnimation*  m_pHEyeAnim;             // 0xA4
	Toshi::TAnimation*  m_pHLeftHandAnim;        // 0xA8
	Toshi::TAnimation*  m_pVLeftHandAnim;        // 0xAC
	Toshi::TAnimation*  m_pHRightHandAnim;       // 0xB0
	Toshi::TAnimation*  m_pVRightHandAnim;       // 0xB4
	Toshi::TAnimation*  m_pBodyTurnAnim;         // 0xB8
	TFLOAT              m_fHEyeAnimLength;       // 0xBC
	TFLOAT              m_fVEyeAnimLength;       // 0xC0
	TFLOAT              m_fHLeftHandAnimLength;  // 0xC4
	TFLOAT              m_fVLeftHandAnimLength;  // 0xC8
	TFLOAT              m_fHRightHandAnimLength; // 0xCC
	TFLOAT              m_fVRightHandAnimLength; // 0xD0
	TFLOAT              m_fBodyTurnAnimLength;   // 0xD4
	TFLOAT              m_fStunTimer;            // 0xD8
	TFLOAT              m_fHEye;                 // 0xDC the animations' positions (0-1)
	TFLOAT              m_fVEye;                 // 0xE0
	TFLOAT              m_fHLeftHand;            // 0xE4
	TFLOAT              m_fHLeftHandTarget;      // 0xE8
	TFLOAT              m_fVLeftHand;            // 0xEC
	TFLOAT              m_fVLeftHandTarget;      // 0xF0
	TFLOAT              m_fHRightHand;           // 0xF4
	TFLOAT              m_fHRightHandTarget;     // 0xF8
	TFLOAT              m_fVRightHand;           // 0xFC
	TFLOAT              m_fVRightHandTarget;     // 0x100
	TFLOAT              m_fBodyTurn;             // 0x104
	TFLOAT              m_fChargeTime;           // 0x108 STATE_Charge's time, also the first shot's timer
	TFLOAT              m_fShotTimer;            // 0x10C
	TFLOAT              m_fTurnSpeed;            // 0x110 BossData::Round of m_uiRound
	TFLOAT              m_fShotDelay;            // 0x114
	TFLOAT              m_fRoundChargeTime;      // 0x118
	TFLOAT              m_fFlashTime;            // 0x11C the hit flash (STATE_StunStart), Blob is attached meanwhile
	TFLOAT              m_fBlobBehindTime;       // 0x120
	TFLOAT              m_fTurnAngle;            // 0x124 radians left to turn
	APropWater*         m_pWater;                // 0x128
	AContainerHazard*   m_pContainerHazard;      // 0x12C its spawn points spawn the pickups of the rounds
	TUINT16*            m_aiBossTriggers;        // 0x130 entity IDs
	Toshi::TXUIScene*   m_pScene;                // 0x134 "LandmarkTarget" of hud.xur
	Toshi::TXUIFigure*  m_pDropletLow;           // 0x138
	Toshi::TXUIFigure*  m_pDropletHigh;          // 0x13C
	Toshi::TXUIElement* m_pTextPresenter1;       // 0x140
	Toshi::TXUIElement* m_pTextPresenter2;       // 0x144
	TWCHAR              m_wszLife[ 4 ];          // 0x148 the life text ("%d")
	TINT16              m_iHatBone;              // 0x150
	TINT16              m_iEyeBone;              // 0x152
	TINT16              m_iLHandBone;            // 0x154
	TINT16              m_iRHandBone;            // 0x156
	TINT16              m_iLastShotBone;         // 0x158
	TINT16              m_iHEyeAnimID;           // 0x15A
	TINT16              m_iVEyeAnimID;           // 0x15C
	TINT16              m_iHLeftHandAnimID;      // 0x15E
	TINT16              m_iVLeftHandAnimID;      // 0x160
	TINT16              m_iShootLeftHandAnimID;  // 0x162
	TINT16              m_iHRightHandAnimID;     // 0x164
	TINT16              m_iVRightHandAnimID;     // 0x166
	TINT16              m_iShootRightHandAnimID; // 0x168
	TINT16              m_iBodyTurnAnimID;       // 0x16A
	TINT8               m_iShotCount;            // 0x16C
	TINT8               m_iNumShots;             // 0x16D BossData::Round
	TUINT8              m_uiNumBossTriggers;     // 0x16E
	TUINT8              m_uiRound;               // 0x16F
	TUINT8              m_uiBossFlags;           // 0x170 BOSSFLAG
};

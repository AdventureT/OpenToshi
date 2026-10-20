#pragma once
#include "Entities/APropPlatform.h"
#include "Entities/AEntityFactory.h"
#include "Entities/APropTurret/ATurretHelper.h"

TOSHI_NAMESPACE_START
class TParticleEffect;
class TParticleEffectInstance;
class TAnimation;
TOSHI_NAMESPACE_END

class AModelsAttachedComponent;
class AEntityModelComponent;
class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: a vehicle driving a spline (cars, tanks with a turret, paint tankers...).
//-----------------------------------------------------------------------------
class ACar
    : public APropPlatform
{
	friend class ATankSpawner; // the spawner wakes its tank up (m_iLife, m_Flags)

public:
	TDECLARE_CLASS( ACar, APropPlatform );

	enum VEHICLETYPE : TUINT8
	{
		VEHICLETYPE_CAR             = 0,
		VEHICLETYPE_TANK            = 4,
		VEHICLETYPE_PAINTTANKER     = 7,
		VEHICLETYPE_BLIMP           = 8,
		VEHICLETYPE_TRAIN_PRISONCAR = 9,
	};

	enum CARFLAG : TUINT16
	{
		CARFLAG_TurretOff           = 0x4,
		CARFLAG_BlimpColoured       = 0x8, // the car, the train or the blimp is coloured (painted)
		CARFLAG_Waiting             = 0x10,
		CARFLAG_Targetable          = 0x100,
		CARFLAG_TargetableFromBelow = 0x200,
	};

	// The car's "<type>_data_EntityData" (0x7C bytes)
	struct CarData
	    : public AEntityTypeData::TRBData
	{
		// The turret of the tank (the same layout as APropTurret::TurretData; ATurretHelper::Initialise gets them)
		TFLOAT m_fRotateSpeed;     // 0x54
		TFLOAT m_fMaxVisualAngleA; // 0x58
		TFLOAT m_fMaxVisualAngleB; // 0x5C
		TFLOAT m_fVisualRangeA;    // 0x60 (a_fVisualRangeA)
		TFLOAT m_fVisualRangeB;    // 0x64 (a_fVisualRangeB)
		TINT   m_iMaxBurst;        // 0x68
		TFLOAT m_fBurstDelay;      // 0x6C (a_fBurstDelay)
		TFLOAT m_fFireDelay;       // 0x70 (a_fFireDelay)
		TFLOAT m_fResumeDelay;     // 0x74 how long the tank waits for a target before it drives on
		TFLOAT m_fSearchAngle;     // 0x78
	};

	// The tank's turret (0x58 bytes)
	struct TurretData
	{
		ATurretHelper*                  pHelper;         // 0x00 ATurretHelper (0xA8 bytes)
		TINT16                          iInkyBone;       // 0x04 m_iInkyBone "Tank_LB_Turret"
		TINT16                          iFireBone;       // 0x06 m_iFireBone "Tank_LB_Cannon_Tip"
		TINT16                          iDamage25Bone;   // 0x08 m_iDamage25Bone
		TINT16                          iDamage50Bone;   // 0x0A m_iDamage50Bone
		TINT16                          iDamage75Bone;   // 0x0C m_iDamage75Bone
		TFLOAT                          fResumeTimer;    // 0x10
		TFLOAT                          fSavedSpeed;     // 0x14
		TINT16                          iIdleAnim;       // 0x18 m_IdleAnim
		TINT16                          iFireAnimBehind; // 0x1A m_FireAnim_Behind
		TINT16                          iFireAnimFront;  // 0x1C m_FireAnim_Front
		TINT16                          iFireAnimLeft;   // 0x1E m_FireAnim_Left
		TINT16                          iFireAnimRight;  // 0x20 m_FireAnim_Right
		Toshi::TAnimation*              pHAnim;          // 0x24 m_pHAnim
		Toshi::TAnimation*              pVAnim;          // 0x28 m_pVAnim
		TFLOAT                          fHAnimDuration;  // 0x2C
		TFLOAT                          fVAnimDuration;  // 0x30
		TINT16                          iRecoilAnim;     // 0x34 m_iRecoilAnim
		TINT16                          iReloadAnim;     // 0x36 m_iReloadAnim
		TINT16                          iHitAnim;        // 0x38 m_iHitAnim
		Toshi::TParticleEffect*         pInkySplatFX;    // 0x3C "Blob_InkySplat"
		Toshi::TParticleEffect*         pDamageSparksFX; // 0x40 "Blob_VehicleDamageSparks"
		Toshi::TParticleEffect*         pDamageSmokeFX;  // 0x44 "Blob_VehicleDamageSmoke"
		Toshi::TParticleEffectInstance* apDamageFX[ 3 ]; // 0x48 at 25%, 50% and 75% damage
		ABaseEntity*                    pSpawner;        // 0x54 the ATankSpawner that made the tank (SetTankSpawner)
	};

public:
	ACar();
	virtual ~ACar() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void Create() override;                                                                   // +0x1C
	virtual void Destroy() override;                                                                  // +0x24
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void SetVisible( TBOOL a_bVisible ) override;                                             // +0x38
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	// $deBlob: FUNCTION 00779790
	virtual TBOOL IsTargetable() override
	{
		TBOOL bFlag = ( m_uiCarFlags >> 8 ) & 1;
		return bFlag;
	} // +0xA0
	// $deBlob: FUNCTION 007796b0
	virtual TBOOL CanBeTargetedFromBelow() override
	{
		TBOOL bFlag = ( m_uiCarFlags >> 9 ) & 1;
		return bFlag;
	} // +0xA4
	// $deBlob: FUNCTION 007797b0
	virtual TINT            GetTargettingPriority() override { return 0; } // +0xA8
	virtual Toshi::TVector4 GetTargetOffset() override;                    // +0xAC
	// $deBlob: FUNCTION 00779730
	virtual TINT GetMaxLife() override { return ( m_eCarType == 4 ) ? 100 : 0; } // +0xB8
	virtual void Reset( ABaseEntity* a_pEntity ) override;                       // +0xC8
	// $deBlob: FUNCTION 007796c0
	virtual TBOOL TicksAtLowLOD() override { return m_eCarType == 5 || APropPlatform::TicksAtLowLOD(); } // +0xF4
	virtual void  StartFromCurrentNode() override;                                                       // +0xFC
	virtual void  OnStartMoving() override;                                                              // +0x100
	virtual void  SetTurretActive( TBOOL a_bActive ) override;                                           // +0x108
	virtual void  StartNodeDelay( TINT16 a_iNode ) override;                                             // +0x10C

	TUINT8 GetCarType() const { return m_eCarType; }

	// The tank drives to a_vTarget (x, y, z, w), turning round when the target is behind it
	void FaceTarget( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW );

	// The tank stops turning.
	void StopTurning();

	// The tank was made by a_pSpawner (an ATankSpawner), TNULL: none.
	void SetTankSpawner( ABaseEntity* a_pSpawner );

private:
	// Plays the turret animation a_iAnim (the previous one stops)
	Toshi::TAnimation* PlayTurretAnim( TINT16 a_iAnim, TFLOAT a_fBlendInSpeed, TFLOAT a_fWeight, TFLOAT a_fSpeed );

	// Stops the sounds of the tank's turret helper
	void StopTurretSounds();

	// The paint tanker's tank a_uiTank was hit by a_pBlob: its colour is the blob's and it pops
	void OnTankerHit( ABlobEntity* a_pBlob, TUINT32 a_uiTank );

	// a_pBlob painted the car, the blimp or the vehicle in a_uiColour: the effect, the score
	void OnPainted( ABlobEntity* a_pBlob, TUINT32 a_uiColour );

	// The vehicle is destroyed (the explosion, the sound, the camera shake); a_bSpawn: the blob gets a new tank to
	// fight (the first player)
	void Explode( TBOOL a_bSpawn );

	// Painted or back to grey: swap the car's models (VEHICLETYPE_CAR)
	void ToggleCarColoured();
	// Swaps the train's models (VEHICLETYPE_TRAIN_PRISONCAR)
	void ToggleTrainColoured();
	// The blimp's texture (VEHICLETYPE_BLIMP)
	void SetBlimpColoured( TBOOL a_bColoured );

	void  SetTankSounds();
	TBOOL IsAnimationFinished( TINT16 a_iSequence );

public:
	// $deBlob: FUNCTION 0076aa40
	TUINT8 GetTankerStage() const
	{
		TASSERT( GetCarType() == VEHICLETYPE_PAINTTANKER );
		return m_uiTankerStage;
	}

protected:
	static Toshi::TParticleEffect* ms_pCollisionFX; // "Blob_CarCollision"
	static Toshi::TParticleEffect* ms_pExplosionFX; // "Blob_Explosion"

	TUINT16                   m_uiCarFlags;      // 0x1F0 CARFLAG
	TUINT8                    m_eCarType;        // 0x1F2 VEHICLETYPE ("Vehicle Type")
	TUINT8                    m_uiTankerStage;   // 0x1F3
	TINT16                    m_iTurretAnim;     // 0x1F4 -1 the animation of PlayTurretAnim
	TurretData*               m_pTurretData;     // 0x1F8 m_pTurretData (the tank)
	AModelsAttachedComponent* m_pModelsAttached; // 0x1FC the chopper's and the paint tanker's parts
	AEntityModelComponent*    m_pExtraModel;     // 0x200 the pimped car's and the train's second model
};

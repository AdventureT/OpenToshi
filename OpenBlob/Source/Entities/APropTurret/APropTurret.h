#pragma once
#include "Entities/AProp.h"
#include "Entities/APropTurret/ATurretHelper.h"
#include "Entities/AEntityFactory.h"

TOSHI_NAMESPACE_START
class TAnimation;
class TTRB;
class TParticleEffect;
class TParticleEffectInstance;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: the turret prop (the tank's turret on its own): an AProp with an ATurretHelper aim at the blobs, a dias
// model that turns with the bones of the turret, damage bones and effects (APropTurret\APropTurret.cpp).
// Unnamed parts are Unk fields with their offset; vtable slots past the AProp overrides are not reconstructed yet.
//-----------------------------------------------------------------------------
class APropTurret
    : public AProp
{
public:
	TDECLARE_CLASS( APropTurret, AProp );

	enum TURRETFLAG : TUINT8
	{
		TURRETFLAG_SpawnsIn   = 0x1, // "Spawns In" (set in ProcessParams 781C00, bit 0 clear: 0x2)
		TURRETFLAG_NoSpawnsIn = 0x2,
		TURRETFLAG_Unk4       = 0x4,
		TURRETFLAG_Targetable = 0x8, // "Targetable"
	};

public:
	// The turret's type data (its TRB, the entity type's "<type>_EntityData"): the TRB base plus the aim values
	// read by ProcessParams (781C00); names from the Initialise arguments, the offsets are from the disassembly
	struct TurretData
	    : public AEntityTypeData::TRBData
	{
		TFLOAT m_fRotateSpeed;     // 0x54
		TFLOAT m_fMaxVisualAngleA; // 0x58
		TFLOAT m_fMaxVisualAngleB; // 0x5C
		TFLOAT m_fVisualRangeA;    // 0x60 (ATurretHelper::Initialise a_fVisualRangeA)
		TFLOAT m_fVisualRangeB;    // 0x64 (a_fVisualRangeB)
		TINT   m_iMaxBurst;        // 0x68
		TFLOAT m_fBurstDelay;      // 0x6C (a_fBurstDelay)
		TFLOAT m_fFireDelay;       // 0x70 (a_fFireDelay)
	};

public:
	APropTurret();
	virtual ~APropTurret() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void PostCreate() override;                                                               // +0x20
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8

	void PlayAnimation( TINT16 a_iAnim );
	void StopFiring();

	virtual void  SetVisible( TBOOL a_bVisible ) override; // +0x38
	virtual void  OnLODChanging( TINT a_iLOD ) override;   // +0x10
	virtual TBOOL IsTargetable() override;                 // +0xA0
	virtual TINT  GetMaxLife() override;                   // +0xB8
	virtual TINT  GetTargettingPriority() override;        // +0xA8
	void          Explode();

private:
	TUINT8                          m_uiTurretFlags;   // 0xB4 TURRETFLAG
	TINT16                          m_iIdleAnim;       // 0xB6 "idle_loop" (m_IdleAnim)
	TINT16                          m_iFireAnim;       // 0xB8 (m_FireAnim)
	TINT16                          m_iFireBone;       // 0xBA "turret_gun_tip" (m_iFireBone)
	TINT16                          m_iDamage25Bone;   // 0xBC "Tank_LB_Damage25"
	TINT16                          m_iDamage50Bone;   // 0xBE "Tank_LB_Damage50"
	TINT16                          m_iDamage75Bone;   // 0xC0 "Tank_LB_Damage75"
	TINT16                          m_iHitAnim;        // 0xC2 "Hit" (m_iHitAnimID)
	TINT16                          m_iSpawnAnim;      // 0xC4 "Spawn" (m_iSpawnAnimID)
	TINT16                          m_iDiasSpawnAnim;  // 0xC6 "Spawn" of the dias model
	TINT16                          m_iCurrentAnim;    // 0xC8
	TUINT8                          m_Unk0CA[ 0x2 ];   // 0xCA
	ATurretHelper                   m_Helper;          // 0xCC the aim helper, owner = this
	Toshi::TAnimation*              m_pHAnim;          // 0x174 "H_Rotate" overlay (m_pHAnim)
	Toshi::TAnimation*              m_pVAnim;          // 0x178 "P_Rotate" overlay (m_pVAnim)
	TFLOAT                          m_fHAnimLength;    // 0x17C
	TFLOAT                          m_fVAnimLength;    // 0x180
	Toshi::TParticleEffect*         m_pFXExplosion;    // 0x184 "Blob_Explosion"
	Toshi::TParticleEffect*         m_pFXInkySplat;    // 0x188 "Blob_InkySplat"
	Toshi::TParticleEffect*         m_pFXDamageSparks; // 0x18C "Blob_VehicleDamageSparks"
	Toshi::TParticleEffect*         m_pFXDamageSmoke;  // 0x190 "Blob_VehicleDamageSmoke"
	Toshi::TParticleEffectInstance* m_pFXDamage25;     // 0x194 sparks above 25% damage
	Toshi::TParticleEffectInstance* m_pFXDamage50;     // 0x198 sparks above 50% damage
	Toshi::TParticleEffectInstance* m_pFXDamage75;     // 0x19C smoke above 75% damage
	AModelInstance2*                m_pDiasModel;      // 0x1A0 "turret_dias_animated.trb"
};

TSTATICASSERT( sizeof( APropTurret ) == 0x1A4 );

#pragma once

//-----------------------------------------------------------------------------
// Purpose: the data of a projectile type ("ProjectileData_..._EntityData" in data/entities/ProjectileData.trb, read by
// AEntityData::GetProjectileData). The TRB stores the names as pointers, the loader fixes them up.
// NOTE: the offsets are from the retail reads in AProjectile and ATurretHelper::Initialise callers.
// The size of the record is not known; the fields we read are at the offsets below.
//-----------------------------------------------------------------------------
class ADataProjectile
{
public:
	TUINT8       m_aUnk00[ 0x04 ];   // 0x00
	const TCHAR* m_szModelName;      // 0x04 the model of the projectile (AModelFactory), or ""
	const TCHAR* m_szFXTrail;        // 0x08 the composition of the trail (AFXCompositionInstance), or ""
	const TCHAR* m_szFXHit;          // 0x0C the composition of the hit (AFXCompositionInstance), or ""
	const TCHAR* m_szParticleEffect; // 0x10 the particle effect that the pool preloads (AProjectileManager::InitPool), or ""
	const TCHAR* m_szFXUnk;          // 0x14 the third composition (AFXCompositionInstance), or ""
	TFLOAT       m_fRadius;          // 0x18 the radius of the swept sphere (AProjectile::UpdateLifeTime)
	TFLOAT       m_fAreaRadius;      // 0x1C the radius of the area damage (AProjectile::AreaDamage, <= 0: none)
	TUINT        m_uiDamage;         // 0x20 AAttackMessage a_uiDamage (HitEntity)
	TUINT        m_uiUnk24;          // 0x24 AAttackMessage a_uiUnk18 (HitEntity)
	TUINT8       m_bUnk28;           // 0x28 AAttackMessage a_bUnk2C (HitEntity)
	TUINT8       m_aUnk29[ 0x03 ];   // 0x29
	TFLOAT       m_fSpeed;           // 0x2C the speed (Fire, Activate; ATurretHelper a_fProjectileSpeed)
	TFLOAT       m_fArcAngle;        // 0x30 the arc angle of the ballistic shot (Fire, Activate; negative = flat)
	TFLOAT       m_fGravity;         // 0x34 the gravity (Fire, Activate; 0 = straight; ATurretHelper a_fGravity)
	TFLOAT       m_fMaxLifeTime;     // 0x38 the maximum life time (UpdateLifeTime)
};

#pragma once
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>
#include "Entities/Projectiles/ADataProjectile.h"

class ABaseEntity;
class ACollisionObject;
class ADataProjectile;
class AProjectile;
class AFXCompositionInstance;
class AModelInstance2;
namespace Toshi
{
class TParticleEffect;
class TParticleEffectInstance;
} // namespace Toshi
namespace TerrainTypes
{
struct CellInstance;
}

//-----------------------------------------------------------------------------
// Purpose: the projectile types (Projectiles\AProjectileManager.cpp, PROJECTILETYPES; the other values are not named yet).
//-----------------------------------------------------------------------------
class AProjectileTypes
{
public:
	enum PROJECTILETYPES : TINT
	{
		PROJECTILE_INVALID = -1,
		// NOTE: PROJECTILE_Count = 2 is inferred from the manager's two pool slots and the asserts (a_eType < PROJECTILE_Count);
		// the other values are not named yet
		PROJECTILE_Count = 2,
	};
};

//-----------------------------------------------------------------------------
// Purpose: the per-shot data that replaces the ADataProjectile values of one projectile (AProjectile +0x10).
// NOTE: the owner of this data is not named yet, so only the offsets are.
//-----------------------------------------------------------------------------
struct AProjectileDataOverride
{
	TUINT8 m_aUnk00[ 0x08 ]; // 0x00
	TFLOAT m_fSpeed;         // 0x08 (ADataProjectile +0x2C)
	TFLOAT m_fGravity;       // 0x0C (ADataProjectile +0x34)
	TFLOAT m_fRadius;        // 0x10 (ADataProjectile +0x18)
	TUINT  m_uiDamage;       // 0x14 (ADataProjectile +0x20)
	TFLOAT m_fAreaRadius;    // 0x18 (ADataProjectile +0x1C)
	TFLOAT m_fLifeTime;      // 0x1C (ADataProjectile +0x38)
	TUINT8 m_aUnk20[ 0x04 ]; // 0x20
	TUINT  m_uiUnk24;        // 0x24 (ADataProjectile +0x24)
};

//-----------------------------------------------------------------------------
// Purpose: a projectile of the enemies and props (a plain class: no base, no TClass yet). Its virtuals: the dtor, Tick, Activate, HitEntity (attacks an entity), HitWorld (an empty slot) and
// HitInstance (a terrain instance).
//-----------------------------------------------------------------------------
class AProjectile
{
	friend class APropTurret; // the turret fires its projectiles (m_uiAttackType)
	friend class ABoss;       // the boss too (m_uiAttackType)
	friend class ACar;        // so does the tank
	friend class IOCallbacks; // the IO action 0x41 (m_pOverride, m_uiAttackType)

public:
	AProjectile( AProjectileTypes::PROJECTILETYPES a_eType, ADataProjectile* a_pData );
	virtual ~AProjectile();                   // +0x00
	virtual void Tick( TFLOAT a_fDeltaTime ); // +0x04

	void         Kill( TBOOL a_bPlayEffects );
	virtual void Activate( ABaseEntity* a_pOwner, const Toshi::TVector4& a_rvPosition, const Toshi::TVector4& a_rvDirection, TUINT a_uiFlags ); // +0x08
	void         Place( ABaseEntity* a_pOwner, const Toshi::TVector4& a_rvPosition, const Toshi::TVector4& a_rvDirection, TUINT a_uiFlags );
	void         Fire( ABaseEntity* a_pOwner, const Toshi::TVector4& a_rvPosition, const Toshi::TVector4& a_rvTarget, TUINT a_uiFlags );
	virtual void HitEntity( ABaseEntity* a_pEntity ); // +0x0C
	// $deBlob: FUNCTION 007d5ca0
	virtual void HitWorld() {}                                           // +0x10
	virtual void HitInstance( TerrainTypes::CellInstance* a_pInstance ); // +0x14
	void         UpdateLifeTime( TFLOAT a_fDeltaTime );
	void         UpdateEffects( TFLOAT a_fDeltaTime );
	void         SetVelocity( const Toshi::TVector4& a_rvVelocity );
	void         StopEffects(); // (orients the model along the velocity)
	void         InitEffects( const Toshi::TMatrix44& a_rTransform );
	TBOOL        StartParticleTrail();
	void         AreaDamage( ABaseEntity* a_pIgnore );
	TFLOAT       GetRadius() const;

	static void OnParticleTrailFinished( Toshi::TParticleEffectInstance* a_pEffectInstance, void* a_pUserData );
	static void HitInstanceCallback( void* a_pItem, void* a_pUserData ); // (TBoxTree render function)

	// The values of the shot: the override's if there is one, else the data's
	// $deBlob: FUNCTION 007d5b80
	TFLOAT GetAreaRadius() const { return ( m_pOverride == TNULL ) ? m_pData->m_fAreaRadius : m_pOverride->m_fAreaRadius; }
	// $deBlob: FUNCTION 007d5be0
	TUINT GetDamage() const { return ( m_pOverride == TNULL ) ? m_pData->m_uiDamage : m_pOverride->m_uiDamage; }
	// $deBlob: FUNCTION 007d5c00
	TFLOAT GetGravity() const { return ( m_pOverride == TNULL ) ? m_pData->m_fGravity : m_pOverride->m_fGravity; }
	// $deBlob: FUNCTION 007d5c20
	TUINT GetUnk24() const { return ( m_pOverride == TNULL ) ? m_pData->m_uiUnk24 : m_pOverride->m_uiUnk24; }
	// $deBlob: FUNCTION 007d5c30
	TFLOAT GetMaxLifeTime() const { return ( m_pOverride == TNULL ) ? m_pData->m_fMaxLifeTime : m_pOverride->m_fLifeTime; }
	// $deBlob: FUNCTION 007d5c60
	TFLOAT GetSpeed() const { return ( m_pOverride == TNULL ) ? m_pData->m_fSpeed : m_pOverride->m_fSpeed; }
	// $deBlob: FUNCTION 007d5c90
	TFLOAT GetLifeTime() const { return m_fLifeTime; }
	// $deBlob: FUNCTION 007d5ce0
	void SetLifeTime( TFLOAT a_fLifeTime ) { m_fLifeTime = a_fLifeTime; }

	// $deBlob: FUNCTION 007d5cb0
	TBOOL IsActive() const { return ( m_uiFlags & 1 ) != 0; }

private:
	ABaseEntity*                      m_pOwner;               // 0x04
	AProjectileTypes::PROJECTILETYPES m_eType;                // 0x08 m_eType (assert 0 <= m_eType < PROJECTILE_Count)
	ADataProjectile*                  m_pData;                // 0x0C
	AProjectileDataOverride*          m_pOverride;            // 0x10 per-shot data (TNULL: the values of m_pData)
	TFLOAT                            m_fLifeTime;            // 0x14 (-1.0f at the ctor)
	Toshi::TVector4                   m_vPosition;            // 0x18
	Toshi::TVector4                   m_vVelocity;            // 0x28
	Toshi::TVector4                   m_vAccel;               // 0x38 (X0Y0Z0W1 at the ctor; Activate sets Y = the gravity of the shot)
	TUINT                             m_uiAttackType;         // 0x48 Unk (zeroed by the ctor)
	TUINT                             m_uiFlags;              // 0x4C bit 0 = active (IsActive), bit 1 = moving, bit 3 = effects running
	AModelInstance2*                  m_pModel;               // 0x50 the model of m_pData's model name (or TNULL)
	AFXCompositionInstance*           m_pFXTrail;             // 0x54 the composition of m_pData's trail name
	AFXCompositionInstance*           m_pFXHit;               // 0x58 the composition of m_pData's hit name
	AFXCompositionInstance*           m_pFXUnk;               // 0x5C the composition of m_pData's third name
	Toshi::TParticleEffect*           m_pParticleEffect;      // 0x60 the effect of the trail (zeroed by the ctor, never set in retail)
	Toshi::TParticleEffectInstance*   m_pParticleEffectTrail; // 0x64 the particle effect (assert m_pParticleEffectTrail)
};

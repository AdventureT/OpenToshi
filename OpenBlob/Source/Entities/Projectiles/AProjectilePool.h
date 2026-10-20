#pragma once
#include "Entities/Projectiles/AProjectile.h"

//-----------------------------------------------------------------------------
// Purpose: a pool of the projectiles of one type (Projectiles\AProjectilePool.cpp).
//-----------------------------------------------------------------------------
class AProjectilePool
{
public:
	AProjectilePool( AProjectile** a_ppProjectiles, TUINT a_uiSize );
	virtual ~AProjectilePool(); // +0x00

	AProjectile* GetFreeProjectile();
	void         KillAll();
	void         KillAllActive( TBOOL a_bPlayEffects );
	void         Tick( TFLOAT a_fDeltaTime );

	TUINT GetSize() const { return m_uiSize; }

private:
	AProjectile** m_ppProjectiles; // 0x04
	TUINT         m_uiSize;        // 0x08
};

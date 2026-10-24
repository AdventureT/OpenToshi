#pragma once
#include <Toshi/TSingleton.h>
#include "Entities/Projectiles/AProjectile.h"

class AProjectilePool;

//-----------------------------------------------------------------------------
// Purpose: the singleton that owns the projectile pools of all types (Projectiles\AProjectileManager.cpp).
// the layout is +0x04 the pool of type 0, +0x08 the pool of type 1, +0x0C a byte enable flag (the ctor sets it to 1).
//-----------------------------------------------------------------------------
class AProjectileManager
    : public Toshi::TSingleton<AProjectileManager>
{
public:
	AProjectileManager();
	virtual ~AProjectileManager();

	void         InitPool( AProjectileTypes::PROJECTILETYPES a_eType, TUINT a_uiSize );
	AProjectile* CreateProjectile( AProjectileTypes::PROJECTILETYPES a_eType );
	void         Tick( TFLOAT a_fDeltaTime );
	void         KillAllActive( AProjectileTypes::PROJECTILETYPES a_eType );
	void         KillAllActive();
	void         DeletePools();
	TUINT        GetPoolSize( AProjectileTypes::PROJECTILETYPES a_eType );

private:
	AProjectilePool* m_apPools[ AProjectileTypes::PROJECTILE_Count ]; // 0x04
	TUINT8           m_bEnabled;                                      // 0x0C
};

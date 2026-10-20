#include "pch.h"
#include "AProjectileManager.h"
#include "AProjectilePool.h"
#include "ADataProjectile.h"
#include "Entities/AEntityData.h"
#include "Entities/APropTurret/ATurretHelper.h"
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0078c820
AProjectileManager::AProjectileManager()
{
	for ( TINT i = 0; i < AProjectileTypes::PROJECTILE_Count; i++ )
	{
		m_apPools[ i ] = TNULL;
	}

	m_bEnabled = TTRUE;
}

// $deBlob: FUNCTION 0078cda0
// $deBlob: FUNCTION 0078c880
AProjectileManager::~AProjectileManager()
{
	DeletePools();
}

// $deBlob: FUNCTION 0078c930
void AProjectileManager::InitPool( AProjectileTypes::PROJECTILETYPES a_eType, TUINT a_uiSize )
{
	TASSERT( a_uiSize > 0 );

	if ( m_apPools[ a_eType ] == TNULL && a_eType != AProjectileTypes::PROJECTILE_INVALID )
	{
		ADataProjectile* pData = TSTATICCAST( ADataProjectile, AEntityData::GetSingleton()->GetProjectileData( ATurretHelper::GetProjectileDataName( a_eType ) ) );

		TString8 sMessage = TString8::VarArgs( "Data required for projectile %d %s", a_eType, ATurretHelper::GetProjectileDataName( a_eType ) );
		TASSERT( pData, sMessage );

		// The particle effect of the data is loaded once for the pool
		if ( TStringManager::String8Compare( "", pData->m_szParticleEffect, -1 ) != 0 )
			TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( pData->m_szParticleEffect );

		AProjectile** ppProjectiles = TSTATICCAST( AProjectile*, TMemalign( 16, a_uiSize * sizeof( AProjectile* ) ) );

		for ( TUINT i = 0; i < a_uiSize; i++ )
			ppProjectiles[ i ] = new AProjectile( a_eType, pData );

		m_apPools[ a_eType ] = new AProjectilePool( ppProjectiles, a_uiSize );
	}
}

// $deBlob: FUNCTION 0078cb20
void AProjectileManager::DeletePools()
{
	for ( TINT i = 0; i < AProjectileTypes::PROJECTILE_Count; i++ )
	{
		if ( m_apPools[ i ] )
		{
			delete m_apPools[ i ];
			m_apPools[ i ] = TNULL;
		}
	}
}

// $deBlob: FUNCTION 0078cb50
TUINT AProjectileManager::GetPoolSize( AProjectileTypes::PROJECTILETYPES a_eType )
{
	TASSERT( a_eType < AProjectileTypes::PROJECTILE_Count );

	// NOTE: a_eType == PROJECTILE_INVALID (-1) is not asserted by retail; it reads the vtable slot before m_apPools
	if ( m_apPools[ a_eType ] )
	{
		return m_apPools[ a_eType ]->GetSize();
	}

	return 0;
}

// $deBlob: FUNCTION 0078cbb0
AProjectile* AProjectileManager::CreateProjectile( AProjectileTypes::PROJECTILETYPES a_eType )
{
	TASSERT( a_eType < AProjectileTypes::PROJECTILE_Count );

	if ( m_bEnabled && m_apPools[ a_eType ] )
	{
		return m_apPools[ a_eType ]->GetFreeProjectile();
	}

	return TNULL;
}

// $deBlob: FUNCTION 0078cc10
void AProjectileManager::Tick( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < AProjectileTypes::PROJECTILE_Count; i++ )
	{
		if ( m_apPools[ i ] )
		{
			m_apPools[ i ]->Tick( a_fDeltaTime );
		}
	}
}

// $deBlob: FUNCTION 0078cc50
void AProjectileManager::KillAllActive( AProjectileTypes::PROJECTILETYPES a_eType )
{
	TASSERT( AProjectileTypes::PROJECTILE_INVALID < a_eType && AProjectileTypes::PROJECTILE_Count > a_eType );

	// NOTE: retail doesn't check that the pool of a_eType exists
	m_apPools[ a_eType ]->KillAllActive( TTRUE );
}

// $deBlob: FUNCTION 0078ccd0
void AProjectileManager::KillAllActive()
{
	for ( TINT i = 0; i < AProjectileTypes::PROJECTILE_Count; i++ )
	{
		if ( m_apPools[ i ] )
		{
			m_apPools[ i ]->KillAllActive( TFALSE );
		}
	}
}

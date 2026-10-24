#include "pch.h"
#include "Entities/APropFXSurface.h"
#include "Entities/Projectiles/AProjectileManager.h"
#include "AGndMotorManager.h"
#include "AEntityFramework.h"
#include "APickupData.h"
#include "AEntityFactory.h"
#include "APooledEntityManager.h"
#include "ASpawnedCivManager.h"
#include "AEntityHash.h"
#include "Enemies/AEnemyManager.h"
#include "Terrain2/ATerrain2.h"
#include "PathFinding/ABuildPathJobQueue.h"
#include "PathFinding/AAINetwork.h"
#include "PathFinding/AAIMoveProbe.h"
#include "PathFinding/AAIJumpTrajectory.h"
#include "AWaypointData.h"
#include "APlatformManager.h"
#include "ACharEntity.h"
#include "JobSystem/AJobQueue.h"
#include "AGndMotorComponent.h"
#include "APickupableComponent.h"
#include "PathFinding/AAIWaypoint.h"
#include "Animation/AAnimationPoseFactory.h"
#include "ATRBResourceFactory.h"
#include "AShakerComponent.h"

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0078d460
void AEntityFramework::Startup()
{
	new ATRBResourceFactory();

	new AAnimationPoseFactory();

	new AAINetwork();

	new ABuildPathJobQueue();

	new AEntityFactory();

	AGndMotorComponent::InitialiseData();

	APickupableComponent::LoadData();

	AShakerComponent::LoadData();

	ACharEntity::InitialiseStatic();

	new AProjectileManager();

	AAIMoveProbe::Init();
	AAIJumpTrajectory::Init();

	new APooledEntityManager();

	new AEnemyManager();

	APropFXSurface::InitialisePool();

	new APickupData();

	new AGndMotorManager();

	new APlatformManager();

	new ASpawnedCivManager();
}

// $deBlob: FUNCTION 0078d810
void AEntityFramework::Shutdown()
{
	// The civilians' manager goes first
	if ( ASpawnedCivManager::GetSingleton() != TNULL )
		delete ASpawnedCivManager::GetSingleton();

	if ( AProjectileManager::GetSingleton() )
		delete AProjectileManager::GetSingleton();

	if ( APooledEntityManager::GetSingleton() )
		delete APooledEntityManager::GetSingleton();

	if ( AEnemyManager::GetSingleton() )
		delete AEnemyManager::GetSingleton();

	AAIJumpTrajectory::Destroy();
	AAIMoveProbe::Destroy();

	AAIWaypoint::sm_FreeList.DeleteAll();

	APropFXSurface::DeinitialisePool();

	AGndMotorComponent::DeinitialiseData();

	APickupableComponent::UnloadData();

	AShakerComponent::UnloadData();

	ACharEntity::DeinitialiseStatic();

	if ( AAINetwork::GetSingleton() )
		delete AAINetwork::GetSingleton();

	if ( AJobQueue::GetInstance() )
		delete AJobQueue::GetInstance();

	if ( AEntityFactory::GetSingleton() )
		delete AEntityFactory::GetSingleton();

	if ( AAnimationPoseFactory::GetSingleton() )
		delete AAnimationPoseFactory::GetSingleton();

	if ( AEntityHash::GetSingleton() )
		delete AEntityHash::GetSingleton();

	if ( ATRBResourceFactory::GetSingleton() )
		delete ATRBResourceFactory::GetSingleton();

	if ( APickupData::GetSingleton() )
		delete APickupData::GetSingleton();

	if ( AWaypointData::GetSingleton() )
		delete AWaypointData::GetSingleton();

	if ( APlatformManager::GetSingleton() )
		delete APlatformManager::GetSingleton();
}

// $deBlob: FUNCTION 0078d980
void AEntityFramework::PreTick( TFLOAT a_fDeltaTime )
{
	APlatformManager::GetSingleton()->Tick();
}

// $deBlob: FUNCTION 0078d690
void AEntityFramework::InitialiseEntityHash()
{
	ATerrain2::RegionInfo* pInfo = ATerrain2::GetInstance()->GetRegion( 0 )->m_pInfo;

	TFLOAT fXMin = pInfo->m_fX - 300.0f;
	TFLOAT fYMin = pInfo->m_fZ - 300.0f;
	TFLOAT fXMax = TFLOAT( pInfo->m_uiSizeX + 4 ) * 150.0f + fXMin;
	TFLOAT fYMax = TFLOAT( pInfo->m_uiSizeZ + 4 ) * 150.0f + fYMin;

	AEntityFactory* pFactory = AEntityFactory::GetSingleton();
	new AEntityHash();

	// NOTE: retail converts with fistp (round to nearest)
	AEntityHash::GetSingleton()->Init( pFactory, fXMin, fYMin, fXMax, fYMax, TINT( std::lrint( ( fXMax - fXMin ) * 0.0625f ) ), TINT( std::lrint( ( fYMax - fYMin ) * 0.0625f ) ) );
}

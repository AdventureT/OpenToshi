#include "pch.h"
#include "AEntityFactory.h"
#include "AEntityFramework.h"
#include "APooledEntityManager.h"
#include "ASpawnedCivManager.h"
#include "Enemies/AEnemyManager.h"
#include "AEntityData.h"
#include "AEntityHash.h"
#include "PathFinding/APathfindingComponent.h"
#include "AGndMotorComponent.h"
#include "Terrain2/ATerrain2.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Terrain2/ACellBucket.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include "Cameras/ACamera.h"

#include <Render/TRenderContext.h>

#include <Physics/Havok/TPhysicsManager.h>

#include <Toshi/T2String8.h>

#include <stdio.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AEntityFactory ) == 0x23C );

// $deBlob: FUNCTION 00758af0
AEntityFactory::AEntityFactory()
{
	m_EntityFlagsIndex = 0;
	m_itAlwaysTick     = m_oAlwaysTickEntities.End();
}

// $deBlob: FUNCTION 00758bf0
// $deBlob: FUNCTION 0078da30
AEntityFactory::~AEntityFactory()
{
	DestroyAllEntities();

	for ( TINT i = 0; i < m_oTypeDatas.Size(); i++ )
	{
		AEntityTypeData* pTypeData = m_oTypeDatas[ i ];

		if ( pTypeData )
			delete pTypeData;
	}

	m_oTypeDatas.Reset();
	m_oCallbacks.DeleteAll();
}

// $deBlob: FUNCTION 00758fa0
void AEntityFactory::Tick( TFLOAT a_fDeltaTime, TUINT a_uiFlags )
{
	UpdateEntitiesLOD();

	AEntityFramework::PreTick( a_fDeltaTime );

	AEntityHash* pHash = AEntityHash::GetSingleton();

	for ( auto it = m_oAlwaysTickEntities.Begin(); it != m_oAlwaysTickEntities.End(); )
	{
		ABaseEntity* pEntity = ( it++ )->m_pEntity;

		if ( !pEntity->IsFlagged( ABaseEntity::FLAG_DontTick ) )
		{
			if ( !( a_uiFlags & 1 ) || pEntity->IsFlagged( ABaseEntity::FLAG_Pickup ) )
			{
				pEntity->Tick( a_fDeltaTime );

				if ( pEntity->IsFlagged( ABaseEntity::FLAG_Hashed ) && pEntity->IsFlagged( ABaseEntity::FLAG_UpdateMatrix ) )
					pHash->Add( *pEntity, TFALSE );
			}
		}
		else
		{
			// The names of the message: the entity type (TString8::GetString asserts IsIndexValid(0)) and the class
			TMAYBE_UNUSED const TCHAR* szTypeName  = pEntity->GetTypeData()->m_strName.GetString();
			TMAYBE_UNUSED const TCHAR* szClassName = pEntity->GetClass()->GetName();

			// NOTE: retail's message also prints the class and type names (TASSERT has no format arguments)
			TASSERT( m_lStaleEntities.Exists( &pEntity->GetCellPair().m_oStaleNode ), "Why is there an entity (%s,%s) that is flagged as \"DON'T TICK\" in the always tick list?\n" );
		}
	}

	// The entities of the loaded cells
	const T2DList<ACellBucket>& rActiveCells = ATerrain2::GetInstance()->GetActiveCells();

	for ( auto itBucket = rActiveCells.Begin(); itBucket != rActiveCells.End(); itBucket++ )
	{
		TerrainTypes::Cell* pCell = itBucket->GetCell();

		if ( !( pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) )
			continue;

		T2DList<ACellPair>* pEntities = TREINTERPRETCAST( T2DList<ACellPair>*, pCell->m_pUnk38 );

		for ( auto it = pEntities->Begin(); it != pEntities->End(); )
		{
			ABaseEntity* pEntity = ( it++ )->m_pEntity;

			if ( !( a_uiFlags & 1 ) || pEntity->IsFlagged( ABaseEntity::FLAG_Pickup ) )
			{
				if ( !pEntity->IsFlagged( ABaseEntity::FLAG_DontTick ) && !pEntity->IsFlagged( ABaseEntity::FLAG_LowLOD ) )
				{
					pEntity->Tick( a_fDeltaTime );
				}

				if ( pEntity->IsFlagged( ABaseEntity::FLAG_Hashed ) && pEntity->IsFlagged( ABaseEntity::FLAG_UpdateMatrix ) )
					pHash->Add( *pEntity, TFALSE );
			}
		}
	}

	for ( auto it = m_lStaleEntities.Begin(); it != m_lStaleEntities.End(); )
	{
		ACellPair* pCellPair = ( it++ )->GetNodeValue();
		ProcessStaleEntity( *pCellPair->m_pEntity );
	}

	TASSERT( m_lStaleEntities.IsEmpty() );
}

// $deBlob: FUNCTION 00759350
void AEntityFactory::PreRenderUpdate()
{
	for ( auto it = m_oAlwaysTickEntities.Begin(); it != m_oAlwaysTickEntities.End(); )
	{
		ABaseEntity* pEntity = ( it++ )->m_pEntity;

		if ( pEntity->IsFlagged( 0x800 ) )
			pEntity->PreRenderUpdate();

		pEntity->SetFlags( pEntity->GetFlags() & ~ABaseEntity::FLAG_UpdateMatrix );
	}

	// The entities of the loaded cells
	const T2DList<ACellBucket>& rActiveCells = ATerrain2::GetInstance()->GetActiveCells();

	for ( auto itBucket = rActiveCells.Begin(); itBucket != rActiveCells.End(); itBucket++ )
	{
		TerrainTypes::Cell* pCell = itBucket->GetCell();

		if ( !( pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) )
			continue;

		T2DList<ACellPair>* pEntities = TREINTERPRETCAST( T2DList<ACellPair>*, pCell->m_pUnk38 );

		for ( auto it = pEntities->Begin(); it != pEntities->End(); )
		{
			ABaseEntity* pEntity = ( it++ )->m_pEntity;

			if ( pEntity->IsFlagged( 0x800 ) )
				pEntity->PreRenderUpdate();

			pEntity->SetFlags( pEntity->GetFlags() & ~ABaseEntity::FLAG_UpdateMatrix );
		}
	}
}

// $deBlob: FUNCTION 007594f0
void AEntityFactory::AddCallback( void* a_pUserData, Callback_t a_fnCallback )
{
	CallbackNode* pNode = new CallbackNode;
	pNode->m_pUserData  = a_pUserData;
	pNode->m_fnCallback = a_fnCallback;
	m_oCallbacks.PushBack( pNode );
}

// $deBlob: FUNCTION 00759530
void AEntityFactory::RemoveCallback( void* a_pUserData )
{
	for ( auto it = m_oCallbacks.Begin(); it != m_oCallbacks.End(); it++ )
	{
		if ( it->m_pUserData == a_pUserData )
		{
			CallbackNode* pNode = it;
			pNode->Remove();
			delete pNode;
			return;
		}
	}
}

// $deBlob: FUNCTION 00759610
void AEntityFactory::CallCallbacks()
{
	for ( auto it = m_oCallbacks.Begin(); it != m_oCallbacks.End(); it++ )
		it->m_fnCallback( it->m_pUserData );
}

// $deBlob: FUNCTION 00759690
ABaseEntity* AEntityFactory::CreateEntity( const TCHAR* a_szEntityName )
{
	TString8 strPath( "data/entities/" );
	strPath += a_szEntityName;

	AEntityTypeData* pTypeData = AddPrecachedTypeData( strPath );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: temporary scaffolding: retail calls CreateObject on the type's TClass even when
	// no class of that name is registered (it crashes); while the concrete entity classes aren't ported, unknown
	// types aren't created
	if ( pTypeData->m_pClass == TNULL )
		return TNULL;
#endif

	ABaseEntity* pEntity = TDYNAMICCAST( ABaseEntity, pTypeData->m_pClass->CreateObject() );
	pEntity->SetTypeData( pTypeData );
	pEntity->Create();
	pEntity->OnDataLoad();

	m_oEntities.PushBack( pEntity );
	AddEntity( *pEntity );
	AEntityHash::GetSingleton()->Add( *pEntity, TFALSE );

	return pEntity;
}

// $deBlob: FUNCTION 007597f0
void AEntityFactory::ReactivateAllEntities()
{
	while ( !m_oInactiveEntities.IsEmpty() )
	{
		ABaseEntity* pEntity = m_oInactiveEntities.Tail();
		RemoveEntity( *pEntity );
		m_oEntities.PushBack( pEntity );
		AddEntity( *pEntity );
	}

	for ( auto it = m_oEntities.Begin(); it != m_oEntities.End(); )
		( it++ )->Reset( TNULL );
}

// $deBlob: FUNCTION 00759960
void AEntityFactory::RemoveEntity( ABaseEntity& a_rEntity )
{
	a_rEntity.Remove();

	ACellPair& rCellPair = a_rEntity.GetCellPair();

	if ( rCellPair.IsLinked() )
	{
		TASSERT( m_itAlwaysTick != TNULL );

		if ( m_itAlwaysTick->m_pEntity == &a_rEntity )
			m_itAlwaysTick++;

		a_rEntity.SetFlags( ( a_rEntity.GetFlags() & ~0xE00 ) | ABaseEntity::FLAG_LOD0 );
		rCellPair.Remove();
		rCellPair.m_pCell = TNULL;
		a_rEntity.SetFlags( a_rEntity.GetFlags() & ~ABaseEntity::FLAG_InCell );
	}

	if ( rCellPair.m_oStaleNode.IsLinked() )
		rCellPair.m_oStaleNode.Remove();
}

// $deBlob: FUNCTION 00759a40
void AEntityFactory::ActivateEntity( ABaseEntity& a_rEntity )
{
	m_oEntities.PushBack( &a_rEntity );
	AddEntity( a_rEntity );
}

// $deBlob: FUNCTION 00759aa0
void AEntityFactory::DeactivateEntity( ABaseEntity& a_rEntity )
{
	RemoveEntity( a_rEntity );
	m_oInactiveEntities.PushBack( &a_rEntity );
}

// $deBlob: FUNCTION 00759af0
void AEntityFactory::ReactivateEntity( ABaseEntity& a_rEntity )
{
	RemoveEntity( a_rEntity );
	m_oEntities.PushBack( &a_rEntity );
	AddEntity( a_rEntity );
}

// $deBlob: FUNCTION 00759b50
void AEntityFactory::DestroyEntity( ABaseEntity* a_pEntity )
{
	RemoveEntity( *a_pEntity );
	a_pEntity->Destroy();
	delete a_pEntity;
}

// $deBlob: FUNCTION 00759b80
void AEntityFactory::DestroyAllEntities()
{
	while ( !m_oEntities.IsEmpty() )
	{
		ABaseEntity* pEntity = m_oEntities.Tail();
		RemoveEntity( *pEntity );
		pEntity->Destroy();
		delete pEntity;
	}

	while ( !m_oInactiveEntities.IsEmpty() )
	{
		ABaseEntity* pEntity = m_oInactiveEntities.Tail();
		RemoveEntity( *pEntity );
		pEntity->Destroy();
		delete pEntity;
	}

	TASSERT( m_oUnlinkedEntities.IsEmpty() );
}

// $deBlob: FUNCTION 00759c90
void AEntityFactory::SetAllUnitTimeStamps( TUINT32 a_uiTimeStamp )
{
	for ( auto it = m_oEntities.Begin(); it != m_oEntities.End(); it++ )
		it->SetHashTimeStamp( a_uiTimeStamp );
}

// $deBlob: FUNCTION 00759d50
void AEntityFactory::CreatePools()
{
	// The enemy pools of the current region
	ATerrain2*            pTerrain    = ATerrain2::GetInstance();
	TerrainTypes::Region* pRegion     = pTerrain->GetRegion( ATerrain2Interface::GetInstance()->GetCurrentRegion() );
	TINT                  iNumEnemies = 0;

	for ( TINT i = 0; i < pRegion->m_iNumEnemyPools; i++ )
	{
		TINT iCount = pRegion->m_pEnemyPools[ i ].m_iCount;
		iNumEnemies += iCount;
		AEnemyManager::GetSingleton()->InitEnemyPool( AEnemyManager::EnemyType( pRegion->m_pEnemyPools[ i ].m_eType ), iCount );
	}

	AEnemyManager::GetSingleton()->InitEnemyUpdatePool( iNumEnemies );
	AEnemyManager::GetSingleton()->InitEnemyComponents();

	// The civilians' pool comes first: up to 64 ACivs (the pool's type 0), then the spawned civilians' wrappers
	ASpawnedCivManager* pCivManager = ASpawnedCivManager::GetSingleton();
	TINT                iNumCivs    = pCivManager->m_iNumCivs;

	if ( iNumCivs > 0 )
	{
		if ( iNumCivs > 0x40 )
			iNumCivs = 0x40;

		APooledEntityManager::GetSingleton()->AddPoolCount( 0, iNumCivs );
	}

	pCivManager->m_iNumCivs += 0x32;
	pCivManager->Create();

	// The pools of the types 1 to 4 (retail order: sizes first, then the pools are created)
	APooledEntityManager::GetSingleton()->EnsurePoolCount( 1, 8 );
	APooledEntityManager::GetSingleton()->EnsurePoolCount( 2, 8 );
	APooledEntityManager::GetSingleton()->EnsurePoolCount( 3, 8 );
	APooledEntityManager::GetSingleton()->EnsurePoolCount( 4, 8 );
	APooledEntityManager::GetSingleton()->CreatePools();
}

// $deBlob: FUNCTION 00759e60
void AEntityFactory::InitPooledEntities()
{
	AEnemyManager::GetSingleton()->InitEnemies();

	APooledEntityManager::GetSingleton()->DeactivateFreeEntities();
}

// $deBlob: FUNCTION 00759e80
void AEntityFactory::UpdateEntitiesLOD()
{
	TPhysicsManager::GetSingleton()->BeginBatchOperation();

	// The unlinked and the always ticked entities find their cell
	for ( m_itAlwaysTick = m_oUnlinkedEntities.Begin(); m_itAlwaysTick != m_oUnlinkedEntities.End(); )
	{
		ABaseEntity* pEntity = m_itAlwaysTick->m_pEntity;
		m_itAlwaysTick++;
		ProcessStaleEntity( *pEntity );
		UpdateEntityLOD( *pEntity );
	}

	for ( m_itAlwaysTick = m_oAlwaysTickEntities.Begin(); m_itAlwaysTick != m_oAlwaysTickEntities.End(); )
	{
		ABaseEntity* pEntity = m_itAlwaysTick->m_pEntity;
		m_itAlwaysTick++;
		ProcessStaleEntity( *pEntity );
		UpdateEntityLOD( *pEntity );
	}

	// The entities of the loaded cells
	const T2DList<ACellBucket>& rActiveCells = ATerrain2::GetInstance()->GetActiveCells();

	for ( auto itBucket = rActiveCells.Begin(); itBucket != rActiveCells.End(); itBucket++ )
	{
		TerrainTypes::Cell* pCell = itBucket->GetCell();

		if ( !( pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) )
			continue;

		T2DList<ACellPair>* pEntities = TREINTERPRETCAST( T2DList<ACellPair>*, pCell->m_pUnk38 );

		for ( m_itAlwaysTick = pEntities->Begin(); m_itAlwaysTick != pEntities->End(); )
		{
			ABaseEntity* pEntity = m_itAlwaysTick->m_pEntity;
			m_itAlwaysTick++;

			if ( !( pEntity->GetFlags() & 0x1000 ) || ( pEntity->GetFlags() & ABaseEntity::FLAG_UpdateMatrix ) )
				ProcessStaleEntity( *pEntity );

			UpdateEntityLOD( *pEntity );
		}
	}

	TPhysicsManager::GetSingleton()->EndBatchOperation();
}

// $deBlob: FUNCTION 0075a1b0
void AEntityFactory::ProcessStaleEntity( ABaseEntity& a_rEntity )
{
	ACellPair& rCellPair = a_rEntity.GetCellPair();

	if ( rCellPair.m_oStaleNode.IsLinked() )
		rCellPair.m_oStaleNode.Remove();

	if ( !a_rEntity.IsLinked() )
	{
		if ( rCellPair.IsLinked() )
		{
			a_rEntity.OnLODChanging( ABaseEntity::FLAG_LOD0 );
			a_rEntity.SetFlags( ( a_rEntity.GetFlags() & ~0xE00 ) | ABaseEntity::FLAG_LOD0 );
			rCellPair.Remove();
			rCellPair.m_pCell = TNULL;
		}

		return;
	}

	TerrainTypes::Cell* pCell = TNULL;
	ATerrain2::GetInstance()->GetCellIndex( ATerrain2Interface::GetInstance()->GetCurrentRegion(), &a_rEntity.GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), &pCell );

	if ( a_rEntity.GetFlags() & ABaseEntity::FLAG_Ticking )
	{
		// Ticking entities are always ticked: they go to m_oAlwaysTickEntities instead of their cell's list
		if ( !( a_rEntity.GetFlags() & ABaseEntity::FLAG_InCell ) )
		{
			if ( rCellPair.IsLinked() )
				rCellPair.Remove();

			rCellPair.InsertAfter( m_oAlwaysTickEntities.End() );
			a_rEntity.SetFlags( a_rEntity.GetFlags() | ABaseEntity::FLAG_InCell );
		}

		rCellPair.m_pCell = pCell;

		if ( pCell != TNULL )
		{
			const TPhysicsManagerInitData& rWorld = TPhysicsManager::GetSingleton()->GetInitData();
			TFLOAT                         fY     = a_rEntity.GetMatrix().m_f42;

			if ( rWorld.m_vBroadPhaseMin.y <= fY - 25.0f && fY + 25.0f <= rWorld.m_vBroadPhaseMax.y )
				return;
		}

		a_rEntity.OnOutOfWorld();
		return;
	}

	// Not ticking: in its cell's list, or in m_oUnlinkedEntities outside the terrain
	if ( pCell != rCellPair.m_pCell || ( a_rEntity.GetFlags() & ABaseEntity::FLAG_InCell ) )
	{
		if ( rCellPair.IsLinked() )
		{
			TASSERT( !a_rEntity.GetCellPair().m_pCell || TSTATICCAST( TerrainTypes::Cell, a_rEntity.GetCellPair().m_pCell )->m_pUnk38->IsInList( &a_rEntity.GetCellPair() ) || m_oAlwaysTickEntities.IsInList( &a_rEntity.GetCellPair() ) );
			rCellPair.Remove();
			a_rEntity.SetFlags( a_rEntity.GetFlags() & ~ABaseEntity::FLAG_InCell );
		}

		rCellPair.m_pCell = pCell;

		if ( pCell != TNULL )
		{
			const TPhysicsManagerInitData& rWorld = TPhysicsManager::GetSingleton()->GetInitData();
			TFLOAT                         fY     = a_rEntity.GetMatrix().m_f42;

			if ( fY - 25.0f < rWorld.m_vBroadPhaseMin.y || rWorld.m_vBroadPhaseMax.y < fY + 25.0f )
				a_rEntity.OnOutOfWorld();

			rCellPair.InsertAfter( TREINTERPRETCAST( T2DList<ACellPair>*, pCell->m_pUnk38 )->End() );
			return;
		}

		a_rEntity.OnOutOfWorld();
		rCellPair.InsertAfter( m_oUnlinkedEntities.End() );
	}
}

// The LOD distances (SetLODDistances sets them from the level's distances, AEnvironmentFX)
// NOTE: retail starts with a base and slope of 0 (only the large and max distances get 175 from their static
// initialisers); they are initialised here as SetLODDistances( 175.0f, 175.0f ) would set them, so an entity updated
// before the first SetLODDistances doesn't get a LOD distance below 1 (it would be released as soon as it spawned)
// $deBlob: FUNCTION 00540760
static TFLOAT s_fLargeLODDistance = 175.0f; // DAT_009b5ffc FLAG_UseLargeLODDistance
// $deBlob: FUNCTION 00540780
static TFLOAT s_fMaxLODDistance   = 175.0f;                                 // DAT_009b6000 bounding radius > 30
static TFLOAT s_fLODDistanceSlope = ( 175.0f - 175.0f ) / ( 30.0f - 2.0f ); // DAT_009b5ff0
static TFLOAT s_fLODDistanceBase  = 175.0f;                                 // DAT_009b5ff4

// $deBlob: FUNCTION 0075a3d0
void AEntityFactory::UpdateEntityLOD( ABaseEntity& a_rEntity )
{
	if ( !a_rEntity.IsLinked() )
		return;

	TUINT32 uiFlags       = a_rEntity.GetFlags();
	TFLOAT  fLODDistSq    = 0.0f;
	TBOOL   bForceHigh    = ( uiFlags & 0x2000000 ) != 0;
	TFLOAT  fCameraDistSq = TMath::MAXFLOAT;

	if ( !bForceHigh )
	{
		AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();

		for ( TINT i = 0; i < AXYZViewportManager::VIEWPORT_Count; i++ )
		{
			if ( pViewportManager->IsViewportUsed( i ) )
			{
				ACamera* pCamera = pViewportManager->GetCamera( i );

				if ( pCamera != TNULL )
				{
					TFLOAT fDistSq = TVector4::DistanceSq( pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), a_rEntity.GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) );

					if ( fDistSq < fCameraDistSq )
						fCameraDistSq = fDistSq;
				}
			}
		}

		uiFlags        = a_rEntity.GetFlags();
		TFLOAT fRadius = a_rEntity.GetBoundingSphereRadius();
		TFLOAT fLODDist;

		if ( uiFlags & ABaseEntity::FLAG_UseLargeLODDistance )
			fLODDist = s_fLargeLODDistance;
		else if ( fRadius < 2.0f )
			fLODDist = 175.0f;
		else if ( fRadius > 30.0f )
			fLODDist = s_fMaxLODDistance;
		else
			// NOTE: retail adds ( fRadius - 2 ) once more after the interpolation
			fLODDist = s_fLODDistanceSlope * ( fRadius - 2.0f ) + s_fLODDistanceBase + ( fRadius - 2.0f );

		fLODDistSq = fLODDist * fLODDist;
	}

	if ( !bForceHigh )
	{
		TerrainTypes::Cell* pCell = TSTATICCAST( TerrainTypes::Cell, a_rEntity.GetCellPair().m_pCell );

		if ( pCell != TNULL )
		{
			TUINT32 uiCellFlags = pCell->m_uiFlags;

			if ( ( ( uiCellFlags & TerrainTypes::CellRuntimeFlag_Loaded ) || ( uiFlags & ABaseEntity::FLAG_IgnoreCellForLOD ) ) && fCameraDistSq <= fLODDistSq )
			{
				if ( ( !( uiCellFlags & TerrainTypes::CellRuntimeFlag_Loaded ) || ( uiCellFlags & TerrainTypes::CellRuntimeFlag_Unloading ) ) && !( uiFlags & ABaseEntity::FLAG_IgnoreCellForLOD ) )
					return;

				if ( !IsInAnyFrustum( a_rEntity, ( uiFlags & ABaseEntity::FLAG_IgnoreCellForLOD ) != 0 ) )
				{
					if ( a_rEntity.GetLOD() == ABaseEntity::FLAG_MediumLOD && ( TUINT8( a_rEntity.GetFlags() >> 12 ) & 1 ) )
						return;

					a_rEntity.OnLODChanging( ABaseEntity::FLAG_MediumLOD );
					a_rEntity.SetFlags( ( a_rEntity.GetFlags() & ~0xA00 ) | 0x1400 );
					return;
				}

				goto HighLOD;
			}
		}

		TASSERT( ( ABaseEntity::FLAG_LODMask_Level & uiFlags ) == ABaseEntity::FLAG_LowLOD || ( ABaseEntity::FLAG_LODMask_Level & uiFlags ) == ABaseEntity::FLAG_MediumLOD || ( ABaseEntity::FLAG_LODMask_Level & uiFlags ) == ABaseEntity::FLAG_HighLOD );

		if ( ( a_rEntity.GetFlags() & ABaseEntity::FLAG_LODMask_Level ) != ABaseEntity::FLAG_LowLOD || !( TUINT8( a_rEntity.GetFlags() >> 12 ) & 1 ) )
		{
			a_rEntity.OnLODChanging( ABaseEntity::FLAG_LowLOD );
			TUINT32 uiOldFlags = a_rEntity.GetFlags();
			a_rEntity.SetFlags( ( uiOldFlags & ~0xC00 ) | 0x1200 );

			if ( uiOldFlags & ABaseEntity::FLAG_Pooled )
			{
				// NOTE: GetPoolOwner returns an entity here
				ABaseEntity* pOther = TREINTERPRETCAST( ABaseEntity*, a_rEntity.GetPoolOwner() );

				if ( pOther != TNULL )
				{
					AEntityIOPacket oPacket = { AEntityIOPacket::MESSAGE_CULLED, 0 };
					pOther->HandleEntityIO( &a_rEntity, oPacket );
				}
			}
		}

		return;
	}

HighLOD:
	if ( a_rEntity.GetLOD() != ABaseEntity::FLAG_HighLOD || !( TUINT8( a_rEntity.GetFlags() >> 12 ) & 1 ) )
	{
		a_rEntity.OnLODChanging( ABaseEntity::FLAG_HighLOD );
		a_rEntity.SetFlags( ( a_rEntity.GetFlags() & ~0x600 ) | 0x1800 );
	}
}

// $deBlob: FUNCTION 0075a6f0
void AEntityFactory::AddEntity( ABaseEntity& a_rEntity )
{
	TASSERT( !a_rEntity.GetCellPair().IsLinked() );

	a_rEntity.GetCellPair().m_pCell = TNULL;
	a_rEntity.SetFlags( ( a_rEntity.GetFlags() & ~ABaseEntity::FLAG_LODMask ) | ABaseEntity::FLAG_LOD0 );
	a_rEntity.GetCellPair().InsertAfter( m_oUnlinkedEntities.End() );
}

// $deBlob: FUNCTION 0075a760
void AEntityFactory::MarkStale( ABaseEntity& a_rEntity )
{
	if ( a_rEntity.IsLinked() && !a_rEntity.GetCellPair().m_oStaleNode.IsLinked() )
		m_lStaleEntities.PushBack( &a_rEntity.GetCellPair().m_oStaleNode );
}

// $deBlob: FUNCTION 0075a790
TBOOL AEntityFactory::IsInAnyFrustum( ABaseEntity& a_rEntity, TBOOL a_bAnyLOD )
{
	if ( !a_bAnyLOD && a_rEntity.GetLOD() == ABaseEntity::FLAG_LowLOD )
		return TFALSE;

	TFLOAT   fRadius   = a_rEntity.GetBoundingSphereRadius();
	TVector4 vPosition = a_rEntity.GetBasePosition();
	TSphere  oSphere( vPosition.x, vPosition.y, vPosition.z, fRadius );

	AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();

	for ( TINT i = 0; i < AXYZViewportManager::VIEWPORT_Count; i++ )
	{
		if ( pViewportManager->IsViewportUsed( i ) )
		{
			ACamera* pCamera = pViewportManager->GetCamera( i );

			if ( pCamera != TNULL && TRenderContext::CullSphereToFrustumSimple( oSphere, pCamera->GetViewPlanes(), 0x3F ) )
				return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0075a8b0
// Called by ATerrain2Interface::OnCellLoaded for the entities of the cell (cell->m_pUnk38)
void AEntityFactory::OnCellLoaded( TerrainTypes::Cell* a_pCell )
{
	T2DList<ACellPair>* pEntities = TREINTERPRETCAST( T2DList<ACellPair>*, a_pCell->m_pUnk38 );

	for ( m_itAlwaysTick = pEntities->Begin(); m_itAlwaysTick != pEntities->End(); )
	{
		ABaseEntity* pEntity = m_itAlwaysTick->m_pEntity;
		m_itAlwaysTick++;

		UpdateEntityLOD( *pEntity );

		if ( pEntity->GetPathfinding() != TNULL )
		{
			AGndMotorComponent* pMotor = TSTATICCAST( APathfindingComponent, pEntity->GetPathfinding() )->GetMotor();

			if ( pMotor != TNULL )
				pMotor->ResetComponent();
		}
	}
}

// $deBlob: FUNCTION 0075a9b0
// Called by ATerrain2Interface::OnCellUnloaded for the entities of the cell (cell->m_pUnk38)
void AEntityFactory::OnCellUnloaded( TerrainTypes::Cell* a_pCell )
{
	T2DList<ACellPair>* pEntities = TREINTERPRETCAST( T2DList<ACellPair>*, a_pCell->m_pUnk38 );

	for ( m_itAlwaysTick = pEntities->Begin(); m_itAlwaysTick != pEntities->End(); )
	{
		ABaseEntity* pEntity = m_itAlwaysTick->m_pEntity;
		m_itAlwaysTick++;

		if ( pEntity->GetPathfinding() != TNULL )
		{
			AGndMotorComponent* pMotor = TSTATICCAST( APathfindingComponent, pEntity->GetPathfinding() )->GetMotor();

			if ( pMotor != TNULL )
				pMotor->ResetComponent();
		}

		// Asserts the entity's LOD, the result is not needed here
		pEntity->GetLOD();

		TUINT32 uiFlags = pEntity->GetFlags();

		// An entity on FLAG_LOD0 that is already flagged 0x1000 is left alone
		if ( ( uiFlags & ABaseEntity::FLAG_LODMask_Level ) == ABaseEntity::FLAG_LOD0 && ( uiFlags & 0x1000 ) != 0 )
			continue;

		if ( ( uiFlags & ABaseEntity::FLAG_IgnoreCellForLOD ) == 0 )
		{
			pEntity->OnLODChanging( ABaseEntity::FLAG_LOD0 );

			// Sets FLAG_LOD0 and 0x1000, clears 0x0C00 (re-read: OnLODChanging may change the flags)
			pEntity->SetFlags( ( pEntity->GetFlags() & 0xFFFFF3FF ) | 0x1200 );
		}
	}
}

// Sets the LOD distances of the entities: a_fMaxDistance for the entities bigger than 30, interpolated from 175 for the
// ones between 2 and 30, a_fLargeDistance for the FLAG_UseLargeLODDistance ones
// $deBlob: FUNCTION 0075ab40
void AEntityFactory::SetLODDistances( TFLOAT a_fMaxDistance, TFLOAT a_fLargeDistance )
{
	s_fLargeLODDistance = a_fLargeDistance;
	s_fMaxLODDistance   = a_fMaxDistance;
	s_fLODDistanceBase  = 175.0f;
	s_fLODDistanceSlope = ( a_fMaxDistance - 175.0f ) / ( 30.0f - 2.0f );
}

// $deBlob: FUNCTION 0075ad20
AEntityTypeData* AEntityFactory::AddTypeData( const TCHAR* a_szEntityName, AEntityTypeData::TRBData* a_pTRBData )
{
	AEntityTypeData* pTypeData = new AEntityTypeData;
	pTypeData->m_strName       = a_szEntityName;
	pTypeData->m_pTRBData      = a_pTRBData;
	pTypeData->m_pClass        = TClass::Find( a_pTRBData->m_szClassName, &TGetClass( ABaseEntity ) );

	m_oTypeDatas.Push( pTypeData );
	return pTypeData;
}

// $deBlob: FUNCTION 0075aba0
AEntityTypeData* AEntityFactory::AddPrecachedTypeData( const TCHAR* a_szEntityName )
{
	for ( TINT i = 0; i < m_oTypeDatas.Size(); i++ )
	{
		if ( m_oTypeDatas[ i ]->m_strName.CompareNoCase( a_szEntityName, -1 ) == 0 )
			return m_oTypeDatas[ i ];
	}

	TCHAR szSymbol[ 1024 ];
	sprintf( szSymbol, "%s_data_EntityData", AEntityData::GetDirectoryName( a_szEntityName ) );

	AEntityTypeData::TRBData* pTRBData = TSTATICCAST( AEntityTypeData::TRBData, AEntityData::GetSingleton()->GetEntityData( szSymbol ) );

	AEntityTypeData* pTypeData = new AEntityTypeData;
	pTypeData->m_strName       = a_szEntityName;
	pTypeData->m_pTRBData      = pTRBData;
	pTypeData->m_pClass        = TClass::Find( pTRBData->m_szClassName, &TGetClass( ABaseEntity ) );

	m_oTypeDatas.Push( pTypeData );
	return pTypeData;
}

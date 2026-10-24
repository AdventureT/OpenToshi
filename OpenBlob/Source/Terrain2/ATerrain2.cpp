#include "pch.h"
#include "ATerrain2.h"
#include "AAssetStreaming.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ATerrain2* ATerrain2::sm_Instance;
TBOOL      ATerrain2::ms_bDebugDrawCellMeshes = TFALSE;
TBOOL      ATerrain2::ms_bRenderCellMeshes    = TTRUE;

TSTATICASSERT( sizeof( ATerrain2 ) == 0x70 );
TSTATICASSERT( sizeof( ATerrain2::CellLoadQueueObject ) == 0x18 );
TSTATICASSERT( sizeof( ATerrain2::MaterialEntry ) == 0x0C );

// $deBlob: FUNCTION 0061db30
ATerrain2::ATerrain2()
{
	m_bStreaming          = TTRUE;
	m_fnFindBucketToReuse = TNULL;

	TASSERT( !sm_Instance );
	sm_Instance = this;

	m_MP_TRB_Header = TNULL;

	for ( TINT i = 0; i < MAX_CELL_CALLBACKS; i++ )
	{
		m_afnCellLoaded[ i ]   = TNULL;
		m_afnCellUnloaded[ i ] = TNULL;
	}
}

// $deBlob: FUNCTION 0061dc50
// $deBlob: FUNCTION 00625320
ATerrain2::~ATerrain2()
{
	TASSERT( m_oFreeList.Size() == MAX_MATERIALS );
	m_oFreeList.Clear();

	for ( TINT i = 0; i < MAX_MATERIALS; i++ )
	{
		Toshi::TWorldMaterial* pMaterial = m_pMaterials[ i ].m_pMaterial;
		m_pMaterials[ i ].m_pMaterial    = TNULL;

		if ( pMaterial )
			delete pMaterial;
	}

	delete[] m_pMaterials;
	m_pMaterials    = TNULL;
	m_MP_TRB_Header = TNULL;

	// NOTE: retail clears the two free lists before the arrays they point into are destroyed
	m_activeCellFreeList.Clear();
	m_queueFreeList.Clear();

	delete[] m_pBuckets;
	m_pBuckets = TNULL;

	delete[] m_pQueueObjects;
	m_pQueueObjects = TNULL;

	TASSERT( m_activeCells.IsEmpty() );
	TASSERT( m_loadQueue.IsEmpty() );
	TASSERT( m_unloadQueue.IsEmpty() );

	TASSERT( sm_Instance == this );
	sm_Instance = TNULL;

	// NOTE: the list members (m_oRoot checks of retail) are destroyed by the compiler in reverse declaration order
}

// $deBlob: FUNCTION 0061e0a0
void ATerrain2::Load( TTRB* a_pTRB, TBOOL a_bStreaming, TINT a_iNumCells, TINT a_iNumQueueObjects )
{
	m_Header = TSTATICCAST( Header, a_pTRB->GetSymbolAddress( "Terrain_Main" ) );
	TASSERT( 0 < m_Header->regionCount );

	if ( !a_bStreaming )
	{
		a_iNumCells  = m_Header->regions[ 0 ].cellCount;
		m_bStreaming = TFALSE;
	}

	TMemoryHeap* pOldWorldHeap = TWorldShader::GetWorldMemHeap();
	TWorldShader::SetWorldMemHeap( TMemory_GetGlobalHeap() );
	m_pBuckets = new ACellBucket[ a_iNumCells ];
	TWorldShader::SetWorldMemHeap( pOldWorldHeap );

	for ( TINT i = 0; i < a_iNumCells; i++ )
	{
		m_activeCellFreeList.PushFront( &m_pBuckets[ i ] );
		m_pBuckets[ i ].AllocateMemory( i );
	}

	m_pQueueObjects = new CellLoadQueueObject[ a_iNumQueueObjects ];

	for ( TINT i = 0; i < a_iNumQueueObjects; i++ )
		m_queueFreeList.PushFront( &m_pQueueObjects[ i ] );

	m_pMaterials = new MaterialEntry[ MAX_MATERIALS ];

	TWorldShader* pWorldShader = TWorldShader::GetSingleton();

	for ( TINT i = 0; i < MAX_MATERIALS; i++ )
	{
		TWorldMaterialHAL* pMaterial  = TSTATICCAST( TWorldMaterialHAL, pWorldShader->CreateMaterial( "" ) );
		m_pMaterials[ i ].m_pMaterial = pMaterial;

		pMaterial->SetUVStream( 0, 0 );
		pMaterial->SetUVStream( 1, 1 );
		pMaterial->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_OVERRIDE );
		pMaterial->SetMultiPassBlendType( 1, TWorldMaterialHAL::MULTIPASSBLENDTYPE_MODULATE );
		pMaterial->SetTextureNum( 2 );
		pMaterial->SetTexture( 0, TNULL );
		pMaterial->SetTexture( 1, TNULL );

		m_oFreeList.PushFront( &m_pMaterials[ i ] );
	}
}

// $deBlob: FUNCTION 0061e360
void ATerrain2::RequestCell( TINT a_iRegion, TINT a_iCell, TBOOL a_bLoad, TBOOL a_bPriority )
{
	QueueCell( a_iRegion, a_iCell, a_bLoad, a_bPriority );
}

// $deBlob: FUNCTION 0061e370
ACellBucket* ATerrain2::LoadCell( TINT a_iRegion, TINT a_iCell )
{
	TASSERT( !m_activeCellFreeList.IsEmpty() );

	TerrainTypes::Cell* pCell = GetCell( a_iRegion, a_iCell );

	if ( pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Hidden )
		return TNULL;

	ACellBucket* pBucket = m_activeCellFreeList.Tail();
	pBucket->Remove();
	pBucket->Reset();

	TerrainTypes::Cell* c = StartLoad( a_iRegion, a_iCell, pBucket );
	TASSERT( c == pCell );

	m_activeCells.PushFront( pBucket );
	return pBucket;
}

// $deBlob: FUNCTION 0061e470
void ATerrain2::UpdateQueues()
{
	if ( !m_unloadQueue.IsEmpty() )
	{
		CellLoadQueueObject* pObject = m_unloadQueue.Head();
		pObject->Remove();
		UnloadQueued( pObject );
		m_queueFreeList.PushFront( pObject );
	}

	if ( !m_loadQueue.IsEmpty() )
	{
		TBOOL bNoFreeBucket = TFALSE;

		if ( m_activeCellFreeList.IsEmpty() )
		{
			CellLoadQueueObject* pHead     = m_loadQueue.Head();
			TBOOL                bPriority = pHead->m_uiFlags & CellLoadQueueObject::Flag_Priority;
			ACellBucket*         pBucket   = m_fnFindBucketToReuse( bPriority );

			if ( pBucket )
			{
				CellLoadQueueObject* queueObj = QueueCell( pBucket->GetRegionIndex(), pBucket->GetCellIndex(), TFALSE, bPriority );
				TASSERT( pBucket == queueObj->m_bucket );
			}

			bNoFreeBucket = m_activeCellFreeList.IsEmpty();
		}

		if ( !bNoFreeBucket )
		{
			CellLoadQueueObject* pObject = m_loadQueue.Head();
			pObject->Remove();

			ACellBucket* pBucket = LoadCell( pObject->m_regionIndex, pObject->m_cellIndex );
			pBucket->GetCell()->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_QueuedLoad;

			m_queueFreeList.PushFront( pObject );
		}
	}
}

// $deBlob: FUNCTION 0061e5f0
TINT* ATerrain2::GetCellVis( TINT region, TINT cell )
{
	if ( cell == TerrainTypes::CellVis::NONE )
		return TNULL;

	TASSERT( region >= 0 && region < m_Header->regionCount );
	TASSERT( cell >= 0 && cell < m_Header->regions[ region ].cellCount );

	return m_Header->regions[ region ].cellVis[ cell ].m_aNeighbours;
}

// $deBlob: FUNCTION 0061e6a0
TerrainTypes::Cell* ATerrain2::GetCell( TINT region, TINT cell )
{
	if ( cell == TerrainTypes::CellVis::NONE )
		return TNULL;

	TASSERT( region >= 0 && region < m_Header->regionCount );
	TASSERT( cell >= 0 && cell < m_Header->regions[ region ].cellCount );

	return &m_Header->regions[ region ].cells[ cell ];
}

// $deBlob: FUNCTION 0061e760
TINT ATerrain2::GetNumCells( TINT region )
{
	TASSERT( region >= 0 && region < m_Header->regionCount );
	return m_Header->regions[ region ].cellCount;
}

// $deBlob: FUNCTION 0061e7c0
TBOOL ATerrain2::IsStreaming()
{
	for ( auto it = m_activeCells.Begin(); it != m_activeCells.End(); it++ )
	{
		if ( it->GetCell()->m_uiFlags & ( TerrainTypes::CellRuntimeFlag_Loading | TerrainTypes::CellRuntimeFlag_Unloading ) )
			return TTRUE;
	}

	return TPhysicsManager::GetSingleton()->HasAddedBodies();
}

// $deBlob: FUNCTION 0061e850
TINT ATerrain2::GetNumLoadingCells()
{
	TINT iNumCells = 0;

	for ( auto it = m_activeCells.Begin(); it != m_activeCells.End(); it++ )
	{
		if ( it->GetCell()->m_uiFlags & ( TerrainTypes::CellRuntimeFlag_Loading | TerrainTypes::CellRuntimeFlag_QueuedLoad ) )
			iNumCells++;
	}

	return iNumCells;
}

// $deBlob: FUNCTION 0061e8d0
ACellBucket* ATerrain2::FindActiveBucket( TerrainTypes::Cell* a_pCell )
{
	for ( auto it = m_activeCells.Begin(); it != m_activeCells.End(); it++ )
	{
		if ( it->GetCell() == a_pCell )
			return it;
	}

	return TNULL;
}

// $deBlob: FUNCTION 0061e960
void ATerrain2::UnloadAll( TBOOL a_bCallCallbacks )
{
	// The cells that are being loaded are cancelled
	for ( auto it = m_activeCells.Begin(); it != m_activeCells.End(); it++ )
	{
		TerrainTypes::Cell* pCell = it->GetCell();

		if ( pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Loading )
			pCell->m_uiFlags |= TerrainTypes::CellRuntimeFlag_CancelLoad;
	}

	while ( AAssetStreaming::GetSingleton()->HasActiveJobs() )
		AAssetStreaming::GetSingleton()->Update( 0.0f );

	// The queued loads go back to the free queue objects (the tail first)
	while ( !m_loadQueue.IsEmpty() )
	{
		CellLoadQueueObject* pObject = m_loadQueue.Tail();
		pObject->Remove();

		GetCell( pObject->m_regionIndex, pObject->m_cellIndex )->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_QueuedLoad;

		pObject->m_regionIndex = -1;
		pObject->m_cellIndex   = -1;
		pObject->m_uiFlags     = 0;
		pObject->m_bucket      = TNULL;
		m_queueFreeList.PushFront( pObject );
	}

	// The active cells are unloaded (the loaded ones tell the unloaded callbacks 4 to 1 first)
	while ( !m_activeCells.IsEmpty() )
	{
		ACellBucket* pBucket = m_activeCells.Head();

		if ( a_bCallCallbacks && ( pBucket->GetCell()->m_uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) )
		{
			for ( TINT i = MAX_CELL_CALLBACKS - 1; i > 0; i-- )
			{
				if ( m_afnCellUnloaded[ i ] )
					m_afnCellUnloaded[ i ]( pBucket );
			}
		}

		pBucket->Unload();
		pBucket->Remove();
		m_activeCellFreeList.PushBack( pBucket );
	}

	while ( !m_unloadQueue.IsEmpty() )
	{
		CellLoadQueueObject* pObject = m_unloadQueue.Tail();
		pObject->Remove();

		pObject->m_regionIndex = -1;
		pObject->m_cellIndex   = -1;
		pObject->m_uiFlags     = 0;
		pObject->m_bucket      = TNULL;
		m_queueFreeList.PushBack( pObject );
	}
}

// $deBlob: FUNCTION 0061e300
void ATerrain2::Release()
{
	UnloadAll( TFALSE );

	m_Header        = TNULL;
	m_MP_TRB_Header = TNULL;
}

// $deBlob: FUNCTION 0061ebb0
void ATerrain2::CancelQueuedLoads( TINT a_region, TINT a_iCell, TINT* a_pNeighbours )
{
	if ( m_loadQueue.IsEmpty() )
		return;

	auto itBegin = m_loadQueue.Begin();
	auto it      = m_loadQueue.End();

	do
	{
		it = it.Prev();

		CellLoadQueueObject& obj = *it;
		TASSERT( obj.m_regionIndex == a_region );

		if ( obj.m_cellIndex != a_iCell && ( obj.m_uiFlags & CellLoadQueueObject::Flag_Priority ) )
		{
			TINT i = 0;

			for ( ; i < 8; i++ )
			{
				if ( a_pNeighbours[ i ] == obj.m_cellIndex )
					break;
			}

			if ( i == 8 )
			{
				GetCell( obj.m_regionIndex, obj.m_cellIndex )->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_QueuedLoad;
				obj.Remove();
				m_queueFreeList.PushBack( &obj );
				return;
			}
		}
	} while ( it != itBegin );
}

// $deBlob: FUNCTION 0061ecf0
ATerrain2::MaterialEntry* ATerrain2::AllocMaterial()
{
	TASSERT( !m_oFreeList.IsEmpty() && "ATerrain2: No more materials in freelist!" );

	MaterialEntry*     pEntry    = TSTATICCAST( MaterialEntry, m_oFreeList.PopFront() );
	TWorldMaterialHAL* pMaterial = TSTATICCAST( TWorldMaterialHAL, pEntry->m_pMaterial );
	pEntry->m_uiFlags            = 0;

	for ( TINT i = 0; i < TWorldMaterialHAL::s_iMaxPasses; i++ )
		pMaterial->SetUVMode( i, TWorldMaterialHAL::UVMODE_STREAM );

	pMaterial->SetUserDataIndex( TWorldMaterialHAL::UData_0, -1 );
	pMaterial->SetUserDataIndex( TWorldMaterialHAL::UData_1, -1 );
	pMaterial->SetUserDataIndex( TWorldMaterialHAL::UData_2, -1 );

	return pEntry;
}

// $deBlob: FUNCTION 0061ed70
void ATerrain2::ReleaseMaterial( MaterialEntry* a_pEntry )
{
	TWorldMaterial* pMaterial = a_pEntry->m_pMaterial;
	pMaterial->SetUVAnimation( TNULL );
	TSTATICCAST( TWorldMaterialHAL, pMaterial )->OnDestroy();

	for ( TUINT i = 0; i < 6; i++ )
		pMaterial->SetTexture( i, TNULL );

	m_oFreeList.PushFront( a_pEntry );
}

// $deBlob: FUNCTION 0061edf0
ATerrain2::CellLoadQueueObject* ATerrain2::QueueCell( TINT a_iRegion, TINT a_iCell, TBOOL a_bLoad, TBOOL a_bPriority )
{
	T2DList<CellLoadQueueObject>* pQueue = &m_loadQueue;
	TerrainTypes::Cell*           pCell  = GetCell( a_iRegion, a_iCell );

	if ( !a_bLoad )
	{
		pQueue          = &m_unloadQueue;
		TUINT32 uiFlags = pCell->m_uiFlags;

		if ( uiFlags & ( TerrainTypes::CellRuntimeFlag_CancelLoad | TerrainTypes::CellRuntimeFlag_QueuedUnload ) )
			return TNULL;

		if ( uiFlags & TerrainTypes::CellRuntimeFlag_QueuedLoad )
		{
			pCell->m_uiFlags = uiFlags & ~TerrainTypes::CellRuntimeFlag_QueuedLoad;

			for ( auto it = m_loadQueue.Begin(); it != m_loadQueue.End(); it++ )
			{
				if ( it->m_regionIndex == a_iRegion && it->m_cellIndex == a_iCell )
				{
					it->Remove();
					m_queueFreeList.PushBack( it );
					return TNULL;
				}
			}
		}
	}
	else
	{
		TUINT32 uiFlags = pCell->m_uiFlags;

		if ( !( uiFlags & TerrainTypes::CellRuntimeFlag_QueuedUnload ) )
		{
			if ( uiFlags & TerrainTypes::CellRuntimeFlag_CancelLoad )
			{
				pCell->m_uiFlags = uiFlags & ~TerrainTypes::CellRuntimeFlag_CancelLoad;
				return TNULL;
			}

			if ( uiFlags & ( TerrainTypes::CellRuntimeFlag_Loaded | TerrainTypes::CellRuntimeFlag_Loading | TerrainTypes::CellRuntimeFlag_QueuedLoad | TerrainTypes::CellRuntimeFlag_Hidden ) )
			{
				if ( uiFlags & TerrainTypes::CellRuntimeFlag_QueuedLoad )
				{
					for ( auto it = m_loadQueue.Begin(); it != m_loadQueue.End(); it++ )
					{
						if ( it->m_regionIndex == a_iRegion && it->m_cellIndex == a_iCell )
						{
							if ( a_bPriority )
							{
								it->Remove();
								m_loadQueue.PushFront( it );
								it->m_uiFlags |= CellLoadQueueObject::Flag_Priority;
								return TNULL;
							}

							it->m_uiFlags &= ~CellLoadQueueObject::Flag_Priority;
							break;
						}
					}
				}

				return TNULL;
			}
		}
		else
		{
			for ( auto it = m_unloadQueue.Begin(); it != m_unloadQueue.End(); it++ )
			{
				if ( it->m_regionIndex == a_iRegion && it->m_cellIndex == a_iCell )
				{
					if ( ( it->m_uiFlags & CellLoadQueueObject::Flag_Priority ) && !a_bPriority )
						return TNULL;

					pCell->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_QueuedUnload;
					it->Remove();
					m_queueFreeList.PushBack( it );
					return TNULL;
				}
			}
		}
	}

	TASSERT( !m_queueFreeList.IsEmpty() );

	CellLoadQueueObject* pObject = m_queueFreeList.Tail();
	pObject->Remove();
	pObject->m_regionIndex = a_iRegion;
	pObject->m_bucket      = TNULL;
	pObject->m_uiFlags     = 0;
	pObject->m_cellIndex   = a_iCell;

	pCell->m_uiFlags |= a_bLoad ? TerrainTypes::CellRuntimeFlag_QueuedLoad : TerrainTypes::CellRuntimeFlag_QueuedUnload;

	if ( !a_bPriority )
	{
		pQueue->PushBack( pObject );
	}
	else
	{
		pObject->m_uiFlags |= CellLoadQueueObject::Flag_Priority;
		pQueue->PushFront( pObject );
	}

	if ( !a_bLoad )
		pObject->m_bucket = FindActiveBucket( pCell );

	return pObject;
}

// $deBlob: FUNCTION 0061f160
TerrainTypes::Cell* ATerrain2::StartLoad( TINT a_region, TINT a_cell, ACellBucket* a_pBucket )
{
	TASSERT( a_region < m_Header->regionCount );
	TerrainTypes::Region* region = &m_Header->regions[ a_region ];

	TASSERT( a_cell < region->cellCount );
	TerrainTypes::Cell* cell = &region->cells[ a_cell ];

	TASSERT( 0 == ( cell->m_uiFlags & ( TerrainTypes::CellRuntimeFlag_Loaded | TerrainTypes::CellRuntimeFlag_Loading ) ) );

	a_pBucket->Set( cell, a_region, a_cell );

	if ( !( cell->m_uiFlags & TerrainTypes::CellFlag_Unk1 ) && !( cell->m_uiFlags & TerrainTypes::CellFlag_HasModel ) )
	{
		// Nothing to stream
		cell->m_uiFlags |= TerrainTypes::CellRuntimeFlag_Loaded;

		for ( TINT i = MAX_CELL_CALLBACKS - 1; i != 0; i-- )
		{
			if ( m_afnCellLoaded[ i ] )
				m_afnCellLoaded[ i ]( a_pBucket );
		}

		return cell;
	}

	cell->m_uiFlags |= TerrainTypes::CellRuntimeFlag_Loading;

	const TCHAR*   szFileName = cell->m_pData->m_szTRB;
	ATRBLoaderJob* pJob       = AAssetStreaming::GetSingleton()->GetFreeJob();
	TTRB*          pTRB       = new ( a_pBucket->GetHeap() ) TTRB();

	pTRB->SetMemoryFunctions( CellTRBAlloc, CellTRBFree, a_pBucket );
	pJob->Init( pTRB, szFileName );
	pJob->m_fnOnFinished        = OnCellTRBLoaded;
	pJob->m_pOnFinishedUserData = a_pBucket;
	AAssetStreaming::GetSingleton()->AddMainThreadJob2( pJob );

	a_pBucket->SetTRB( pTRB );
	return cell;
}

// $deBlob: FUNCTION 0061f450
void ATerrain2::UnloadQueued( CellLoadQueueObject* a_pObject )
{
	if ( !a_pObject->m_bucket )
		return;

	a_pObject->m_bucket->GetCell()->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_QueuedUnload;
	ACellBucket* pBucket = a_pObject->m_bucket;

	for ( TINT i = MAX_CELL_CALLBACKS - 1; i != 0; i-- )
	{
		if ( m_afnCellUnloaded[ i ] )
			m_afnCellUnloaded[ i ]( pBucket );
	}

	pBucket = a_pObject->m_bucket;
	pBucket->Unload();
	pBucket->Remove();
	m_activeCellFreeList.PushBack( pBucket );
}

// $deBlob: FUNCTION 0061f4b0
void ATerrain2::OnCellTRBLoaded( ATRBLoaderJob* a_pJob, void* a_pBucket )
{
	ACellBucket*        pBucket = TSTATICCAST( ACellBucket, a_pBucket );
	TerrainTypes::Cell* pCell   = pBucket->GetCell();

	AAssetStreaming::GetSingleton()->ReleaseJob( a_pJob );
	ATerrain2* pTerrain = sm_Instance;

	if ( pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_CancelLoad )
	{
		pBucket->Unload();
		pBucket->Remove();
		pTerrain->m_activeCellFreeList.PushBack( pBucket );
		return;
	}

	pBucket->OnLoaded();

	if ( ( pCell->m_uiFlags & TerrainTypes::CellFlag_HasModel ) && pCell->m_pData->m_pWorldVis )
		pCell->m_pData->m_pWorldVis->SetRenderCallbackUser( OnRenderCellMesh, pCell );

	pTerrain = sm_Instance;

	for ( TINT i = MAX_CELL_CALLBACKS - 1; i != 0; i-- )
	{
		if ( pTerrain->m_afnCellLoaded[ i ] )
			pTerrain->m_afnCellLoaded[ i ]( pBucket );
	}
}

// $deBlob: FUNCTION 0061f540
void* __stdcall ATerrain2::CellTRBAlloc( TTRB::AllocType a_eType, TUINT32 a_uiSize, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData )
{
	return TMemoryHeap_Memalign( TSTATICCAST( ACellBucket, a_pUserData )->GetHeap(), a_iAlignment, a_uiSize );
}

// $deBlob: FUNCTION 0061f560
void __stdcall ATerrain2::CellTRBFree( TTRB::AllocType a_eType, void* a_pMem, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData )
{
	TFree( a_pMem );
}

// $deBlob: FUNCTION 0061f570
TINT ATerrain2::AddCellLoadedCallback( t_CellCallback a_fnCallback )
{
	TINT iFreeSlot = -1;

	for ( TINT i = MAX_CELL_CALLBACKS - 1; i != 0; i-- )
	{
		if ( m_afnCellLoaded[ i ] == a_fnCallback )
			return i;

		if ( m_afnCellLoaded[ i ] == TNULL )
			iFreeSlot = i;
	}

	if ( iFreeSlot >= 0 )
		m_afnCellLoaded[ iFreeSlot ] = a_fnCallback;

	return iFreeSlot;
}

// $deBlob: FUNCTION 0061f5b0
TINT ATerrain2::AddCellUnloadedCallback( t_CellCallback a_fnCallback )
{
	TINT iFreeSlot = -1;

	for ( TINT i = MAX_CELL_CALLBACKS - 1; i != 0; i-- )
	{
		if ( m_afnCellUnloaded[ i ] == a_fnCallback )
			return i;

		if ( m_afnCellUnloaded[ i ] == TNULL )
			iFreeSlot = i;
	}

	if ( iFreeSlot >= 0 )
		m_afnCellUnloaded[ iFreeSlot ] = a_fnCallback;

	TASSERT( iFreeSlot >= 0, "Out of cell call backs" );
	return iFreeSlot;
}

// $deBlob: FUNCTION 0061f780
TINT ATerrain2::GetCellIndex( TINT a_region, const TVector4* a_pPos, TerrainTypes::Cell** a_ppCell )
{
	TASSERT( a_region >= 0 && a_region < m_Header->regionCount );

	RegionInfo* pInfo = m_Header->regions[ a_region ].m_pInfo;

	TINT iZ     = TMath::FloorToInt( a_pPos->z );
	TINT iX     = TMath::FloorToInt( a_pPos->x );
	TINT iCellX = TMath::FloorToInt( ( TFLOAT( iX ) - pInfo->m_fX ) / 150.0f );
	TINT iCellZ = TMath::FloorToInt( ( TFLOAT( iZ ) - pInfo->m_fZ ) / 150.0f );

	if ( iCellX > -1 && iCellX < pInfo->m_uiSizeX && iCellZ > -1 && iCellZ < pInfo->m_uiSizeZ )
	{
		TINT cellIndex = pInfo->m_auiCells[ pInfo->m_uiSizeX * iCellZ + iCellX ];
		TASSERT( ( cellIndex >= 0 && cellIndex < GetNumCells( a_region ) ) || ( cellIndex == 0xff ) );

		TerrainTypes::Cell* pCell = TNULL;

		if ( cellIndex == 0xff )
			cellIndex = TerrainTypes::CellVis::NONE;
		else
			pCell = GetCell( a_region, cellIndex );

		if ( a_ppCell )
			*a_ppCell = pCell;

		return cellIndex;
	}

	if ( a_ppCell )
		*a_ppCell = TNULL;

	return TerrainTypes::CellVis::NONE;
}

// $deBlob: FUNCTION 0061f910
TBOOL ATerrain2::IsPosInsideCell( TerrainTypes::Cell* a_pCell, const TVector4* a_pPos )
{
	TerrainTypes::Cell* pCell = TNULL;
	GetCellIndex( 0, a_pPos, &pCell );

	return pCell == a_pCell;
}

// The multiplayer data of the level (its "MP_Data_Header" symbol)
// $deBlob: FUNCTION 0061f940
void ATerrain2::LoadMPData( TTRB* a_pTRB )
{
	m_MP_TRB_Header = a_pTRB->GetSymbolAddress( "MP_Data_Header" );
}

// Debug sphere draw (empty)
static inline void DebugDrawSphereStub( const TMatrix44* a_pMatrix, TFLOAT a_fRadius, TUINT32 a_uiColour )
{
	// DOES_NOTHING
}

// $deBlob: FUNCTION 0061f960
void __stdcall ATerrain2::OnRenderCellMesh( TTMDWin::TRBWorldCellMesh* a_pCellMesh, TWorldVis::RenderData* a_pRenderData, void* a_pCell )
{
	TWorldMeshHAL* pCellMesh = TSTATICCAST( TTRBWorldMesh, a_pCellMesh->m_pWorldMesh )->m_pMesh;
	TASSERT( pCellMesh->IsA( &TGetClass( TWorldMeshHAL ) ) );

	if ( ms_bRenderCellMeshes )
	{
		TBOOL bRes = pCellMesh->Render();
		TASSERT( bRes );
	}

	if ( ms_bDebugDrawCellMeshes )
	{
		// The mesh's bounding sphere moved by the cell's matrix: the translation of a copy of the matrix becomes the
		// centre in world space (TMatrix44::TransformVector writes the translation row of the copy, as retail does)
		TerrainTypes::Cell* pCell = TSTATICCAST( TerrainTypes::Cell, a_pCell );
		TMatrix44           mCell = *pCell->m_pMatrix;
		TVector3            vCentre( a_pCellMesh->m_Sphere.GetOrigin().x, a_pCellMesh->m_Sphere.GetOrigin().y, a_pCellMesh->m_Sphere.GetOrigin().z );
		TVector3            vWorldCentre;

		TMatrix44::TransformVector( vWorldCentre, mCell, vCentre );
		mCell.m_f41 = vWorldCentre.x;
		mCell.m_f42 = vWorldCentre.y;
		mCell.m_f43 = vWorldCentre.z;

		DebugDrawSphereStub( &mCell, a_pCellMesh->m_Sphere.GetRadius(), 0xffff0000 );
	}
}

// $deBlob: FUNCTION 0061f6c0
ATerrain2::t_CellCallback ATerrain2::GetCellLoadedCallback( TINT a_iSlot ) const
{
	TASSERT( a_iSlot >= 0 && a_iSlot < MAX_CELL_CALLBACKS );
	return m_afnCellLoaded[ a_iSlot ];
}

// $deBlob: FUNCTION 0061f720
ATerrain2::t_CellCallback ATerrain2::GetCellUnloadedCallback( TINT a_iSlot ) const
{
	TASSERT( a_iSlot >= 0 && a_iSlot < MAX_CELL_CALLBACKS );
	return m_afnCellUnloaded[ a_iSlot ];
}

// NOTE: with a_maxCallbacks 0 the loop runs down from -1 through the negative indices until it wraps (retail's
// do/while); dead code
// $deBlob: FUNCTION 0061f3f0
void ATerrain2::CallCellCallbacks( t_CellCallback* a_pCallbacks, TINT a_maxCallbacks, ACellBucket* a_pBucket )
{
	TASSERT( a_maxCallbacks >= 0 );

	TINT i = a_maxCallbacks - 1;

	if ( a_maxCallbacks != 1 )
	{
		do
		{
			if ( a_pCallbacks[ i ] != TNULL )
				a_pCallbacks[ i ]( a_pBucket );

			i--;
		} while ( i != 0 );
	}
}

// $deBlob: FUNCTION 0061e320
void* ATerrain2::LoadCell( const TCHAR* a_szName )
{
	TINT                iRegion;
	TINT                iCell;
	TerrainTypes::Cell* pCell = FindCell( a_szName, &iRegion, &iCell );

	if ( pCell != TNULL && !( pCell->m_uiFlags & 0x10000 ) )
		return LoadCell( iRegion, iCell );

	return pCell;
}

// $deBlob: FUNCTION 0061f340
TerrainTypes::Cell* ATerrain2::FindCell( const TCHAR* a_szName )
{
	TINT iRegion;
	TINT iCell;
	return FindCell( a_szName, &iRegion, &iCell );
}

// $deBlob: FUNCTION 0061f360
TerrainTypes::Cell* ATerrain2::FindCell( const TCHAR* a_szName, TINT* a_pRegion, TINT* a_pCell )
{
	for ( TINT i = 0; i < m_Header->regionCount; i++ )
	{
		TerrainTypes::Region* pRegion = &m_Header->regions[ i ];

		for ( TINT j = 0; j < pRegion->cellCount; j++ )
		{
			TerrainTypes::Cell* pCell = &pRegion->cells[ j ];

			if ( T2String8::Compare( pCell->m_szName, a_szName ) == 0 )
			{
				*a_pRegion = i;
				*a_pCell   = j;
				return pCell;
			}
		}
	}

	return TNULL;
}

// The multiplayer data of a_pInstance
// $deBlob: FUNCTION 00578550
ATerrain2::MPInstanceData* ATerrain2::GetMPInstanceData( const TerrainTypes::CellInstance* a_pInstance ) const
{
	TINT iIndex = TINT( a_pInstance - m_Header->instances );
	TASSERT( iIndex >= 0 && iIndex < TSTATICCAST( MPHeader, m_MP_TRB_Header )->instancecount );
	return &TSTATICCAST( MPHeader, m_MP_TRB_Header )->m_pInstances[ iIndex ];
}

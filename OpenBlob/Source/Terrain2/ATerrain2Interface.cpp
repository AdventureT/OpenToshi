#include "pch.h"
#include "Sound/APositionalSoundManager.h"
#include "Sound/ASoundBreakpointComponent.h"
#include "Buildings/ABuildingManager.h"
#include "Buildings/ALandmarkInstance.h"
#include "Buildings/ATreeInstance.h"
#include "Buildings/AGrassInstance.h"
#include "Buildings/AToggleVisInstance.h"
#include "Buildings/ATextureSwapInstance.h"
#include "ATerrain2Interface.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/AEntityFactory.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Collision/ACollisionInstance2.h"
#include "Collision/ACollisionInstanceMover.h"
#include "Collision/ACollisionObject.h"
#include "ATerrain2.h"
#include "ACellBucket.h"
#include "Cameras/ACameraManager.h"
#include "GameController/AGameStatus.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"
#include "ARandom.h"
#include "AAssetStreaming.h"
#include "ACurrentLevelInformation.h"
#include "Render/ARenderer.h"
#include "Render/Material/AMaterialManager.h"
#include "Render/RenderLightmappedInstance_DX11.h"
#include "Platform/DX11/AJellyShader_DX11.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Animation/AModel2.h"
#include "Render/Lighting/ALightingData.h"
#include "Entities/AZJumpData.h"
#include <Physics/Havok/TPhysicsManager.h>

#include <Render/TRender.h>
#include <Render/TModelManager.h>
#include <Render/TRenderContext.h>
#include <Render/TCameraObject.h>
#include <Toshi/THPTimer.h>

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ATerrain2Interface* ATerrain2Interface::sm_Instance;
TFLOAT              ATerrain2Interface::ms_afInstanceLODDistancesSq[ 4 ];
TBOOL               ATerrain2Interface::ms_bCancelFarLoads     = TFALSE;
TBOOL               ATerrain2Interface::ms_bCullInstances        = TTRUE;
TBOOL               ATerrain2Interface::ms_bRenderLowLODMeshes = TTRUE;
TFLOAT              ATerrain2Interface::ms_fInstanceShadeCoeffHigh;
TFLOAT              ATerrain2Interface::ms_fInstanceShadeCoeffFade;
TFLOAT              ATerrain2Interface::ms_fInstanceShadeCoeffNear;
TFLOAT              ATerrain2Interface::ms_fInstanceShadeCoeffLow;

TSTATICASSERT( sizeof( ATerrain2Interface ) == 300 );
TSTATICASSERT( sizeof( ATerrain2Interface::CellLowLOD ) == 0x90 );

// $deBlob: FUNCTION 0068b380
// NOTE: a method of a box (centre at 0x00, extents at 0x10) of the engine's math that isn't ported: -1 if the box is
// behind the plane, 1 if it's in front of it, 0 if it intersects it
static TINT ClassifyBoxPlane( const TVector4* a_pBox, const TPlane& a_rPlane )
{
	TVector4 vNormal( a_rPlane.GetNormal().x, a_rPlane.GetNormal().y, a_rPlane.GetNormal().z, 1.0f );
	TVector4 vAbsNormal;
	vAbsNormal.Abs4( vNormal );

	const TVector4& rCentre  = a_pBox[ 0 ];
	const TVector4& rExtents = a_pBox[ 1 ];

	TFLOAT fDistance = ( rCentre.y * vNormal.y + rCentre.x * vNormal.x + rCentre.z * vNormal.z ) - a_rPlane.GetD();
	TFLOAT fRadius   = rExtents.y * vAbsNormal.y + rExtents.x * vAbsNormal.x + rExtents.z * vAbsNormal.z;

	if ( fRadius + fDistance < 0.0f )
		return -1;

	return 0.0f < fDistance - fRadius;
}

// $deBlob: FUNCTION 00620270
ATerrain2Interface::ATerrain2Interface( TINT a_iRegion )
{
	m_currentRegion            = a_iRegion;
	m_iCurrentCell             = TerrainTypes::CellVis::NONE;
	m_fnGetPosition            = TNULL;
	m_bStreaming               = TTRUE;
	m_numInstAnims             = 0;
	m_pPerCellInstancesVisTree = TNULL;
	m_uiFlags                  = 0;
	m_uiCreateCell             = 0;
	m_uiCreateInstance         = 0;
	m_fTime                    = 0.0f;
	m_pWorldVisPool            = 0;
	m_ppCellInstanceLists      = TNULL;
	m_piNumCellInstances       = TNULL;
	m_pRenderNodes             = TNULL;

	TASSERT( !sm_Instance );
	sm_Instance = this;

	m_pRenderNodes = TSTATICCAST( RenderNode, TMemalign( 16, sizeof( RenderNode ) * MAX_RENDERING_CELLS ) );

	ATerrain2* pTerrain = ATerrain2::GetInstance();

	ms_afInstanceLODDistancesSq[ 0 ] = 40000.0f;
	ms_afInstanceLODDistancesSq[ 1 ] = 40000.0f;
	ms_afInstanceLODDistancesSq[ 2 ] = 62500.0f;
	ms_afInstanceLODDistancesSq[ 3 ] = 90000.0f;

	pTerrain->SetFindBucketToReuseCallback( FindBucketToReuse );
	pTerrain->AddCellLoadedCallback( OnCellLoadedCallback );
	pTerrain->AddCellUnloadedCallback( OnCellUnloadedCallback );
	pTerrain->AddCellUnloadedCallback( APositionalSoundManager::OnCellUnloadedCallback );

	TUINT uiNumInstAnims = pTerrain->GetRegion( m_currentRegion )->numInstAnimModel;
	m_pInstAnims         = new InstanceAnim[ uiNumInstAnims ];

	for ( TUINT i = 0; i < pTerrain->GetRegion( m_currentRegion )->numInstAnimModel; i++ )
	{
		m_pInstAnims[ i ].m_pModelInstance = TNULL;
		m_pInstAnims[ i ].m_pInstance      = TNULL;

		if ( m_pInstAnims[ i ].m_pBreakpoint )
		{
			delete TSTATICCAST( ASoundBreakpointComponent, m_pInstAnims[ i ].m_pBreakpoint );
			m_pInstAnims[ i ].m_pBreakpoint = TNULL;
		}
	}

	m_worldVisPoolSize = 0;

	for ( TINT k = 0; k < 4; k++ )
	{
		TINT iNumInstances     = pTerrain->GetNumInstances();
		m_apInstanceFades[ k ] = TSTATICCAST( TUINT32, TMemalign( 16, iNumInstances * 8 ) );

		for ( TINT i = 0; i < iNumInstances; i++ )
		{
			m_apInstanceFades[ k ][ i * 2 + 1 ] = 0xC0000000; // -2.0f
			m_apInstanceFades[ k ][ i * 2 ]     = 0;
		}
	}

	TUINT uiNumCells      = pTerrain->GetNumCells( m_currentRegion );
	m_ppCellInstanceLists = TSTATICCAST( void*, TMemalign( 16, uiNumCells * sizeof( void* ) ) );
	m_piNumCellInstances  = TSTATICCAST( TINT, TMemalign( 16, uiNumCells * sizeof( TINT ) ) );

	for ( TUINT i = 0; i < uiNumCells; i++ )
	{
		TUINT32* pList             = TSTATICCAST( TUINT32, TMemalign( 16, 0x20 ) );
		m_ppCellInstanceLists[ i ] = pList;

		for ( TINT j = 0; j < 8; j++ )
			pList[ j ] = 0;

		m_piNumCellInstances[ i ] = 0;
	}

	m_piNearCells = TSTATICCAST( TINT, TMemalign( 16, 4 * sizeof( TINT ) ) );

	for ( TINT i = 0; i < 4; i++ )
		m_piNearCells[ i ] = -1;

	m_bUseBlobPosition       = TTRUE;
	m_bSinglePlayer = AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE;
}

// $deBlob: FUNCTION 006206b0
// $deBlob: FUNCTION 005c87e0
ATerrain2Interface::~ATerrain2Interface()
{
	TFree( m_piNearCells );
	m_piNearCells = TNULL;

	TASSERT( m_usedRenderNodes == 0 );
	TASSERT( m_renderingCellsRoot == TNULL );
	TFree( m_pRenderNodes );

	ATerrain2* pTerrain   = ATerrain2::GetInstance();
	TUINT      uiNumCells = pTerrain->GetNumCells( m_currentRegion );

	for ( TUINT i = 0; i < uiNumCells; i++ )
		TFree( m_ppCellInstanceLists[ i ] );

	TFree( m_ppCellInstanceLists );
	TFree( m_piNumCellInstances );

	UnloadAllCells();
	DestroyInstanceAnims();
	DestroyCellInstances();
	DestroyCellLowLODs();

	// The world vis pool, its entries are destroyed last to first
	if ( m_pWorldVisPool )
	{
		for ( TUINT32 i = m_worldVisPoolSize; i-- > 0; )
			m_pWorldVisPool[ i ].~WorldVisEntry();

		TFree( m_pWorldVisPool );
		m_pWorldVisPool = TNULL;
	}

	delete[] m_pInstAnims;

	TASSERT( sm_Instance == this );
	sm_Instance = TNULL;

	pTerrain->SetFindBucketToReuseCallback( TNULL );
	pTerrain->RemoveCellLoadedCallback( OnCellLoadedCallback );
	pTerrain->RemoveCellUnloadedCallback( OnCellUnloadedCallback );
	pTerrain->Release();

	if ( ATerrain2::GetInstance() )
		delete ATerrain2::GetInstance();

	for ( TINT i = 0; i < 4; i++ )
		TFree( m_apInstanceFades[ i ] );
}

// Unloads every cell (the unloaded callbacks are called) and clears the runtime flags of the cells that are left
// $deBlob: FUNCTION 00621c50
void ATerrain2Interface::UnloadAllCells()
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();
	pTerrain->UnloadAll( TTRUE );

	for ( TINT i = 0; i < pTerrain->GetNumCells( m_currentRegion ); i++ )
		pTerrain->GetCell( m_currentRegion, i )->m_uiFlags &= ~( TerrainTypes::CellRuntimeFlag_Visible | TerrainTypes::CellRuntimeFlag_Hidden );

	m_iCurrentCell = TerrainTypes::CellVis::NONE;
}

// $deBlob: FUNCTION 00621af0
void ATerrain2Interface::ResetAllCells()
{
	UnloadAllCells();

	ATerrain2* pTerrain = ATerrain2::GetInstance();

	if ( !m_bStreaming )
	{
		for ( TINT i = 0; i < pTerrain->GetNumCells( m_currentRegion ); i++ )
			pTerrain->RequestCell( m_currentRegion, i, TTRUE, TFALSE );
	}

	for ( TINT i = 0; i < pTerrain->GetNumCells( m_currentRegion ); i++ )
	{
		TerrainTypes::Cell* pCell = pTerrain->GetCell( m_currentRegion, i );

		for ( TUINT j = 0; j < pCell->m_uiNumInstances; j++ )
		{
			TerrainTypes::CellInstance* pInstance = &pCell->m_pInstances[ j ];

			// Visible again unless it starts hidden (0x200), no toggle running
			if ( pInstance->m_uiFlags & 0x200 )
				pInstance->m_uiFlags &= ~TerrainTypes::InstanceFlags_Visible;
			else
				pInstance->m_uiFlags |= TerrainTypes::InstanceFlags_Visible;

			pInstance->m_uiFlags &= 0xE1FFFFFF;

			// The moved dynamic bodies go back to the instance
			AModelInstance2* pModelInstance = TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 );

			if ( pModelInstance != TNULL )
			{
				ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, pModelInstance->GetCollisionInstance() );

				if ( pCollision != TNULL )
				{
					for ( TINT k = 0; k < pCollision->GetNumPhysicsObjects(); k++ )
					{
						ACollisionInstanceMover* pMover = pCollision->GetPhysicsInfo( k ).m_pInstanceMover;

						if ( pMover != TNULL )
							pMover->ResetToInstance();
					}

					pCollision->ClearFlags( ACollisionInstance2::Flags_NotInWorld );
				}
			}

			// The reset callback of the instance's type (index type & 7)
			switch ( pInstance->m_uiFlags & 7 )
			{
				case TerrainTypes::InstanceType_Building: ABuildingInstance::OnTable97b958( pInstance ); break;
				case TerrainTypes::InstanceType_Tree: ATreeInstance::OnReset( pInstance ); break;
				case TerrainTypes::InstanceType_TextureSwap: ATextureSwapInstance::OnReset( pInstance ); break;
				case TerrainTypes::InstanceType_ToggleVis: AToggleVisInstance::OnReset( pInstance ); break;
			}
		}
	}
}

// Deletes the model instances and the sound breakpoints of the animated instances
// $deBlob: FUNCTION 00622ae0
void ATerrain2Interface::DestroyInstanceAnims()
{
	for ( TINT i = 0; i < m_numInstAnims; i++ )
	{
		InstanceAnim& rAnim = m_pInstAnims[ i ];

		if ( rAnim.m_pModelInstance )
			delete rAnim.m_pModelInstance;

		rAnim.m_pModelInstance = TNULL;
		rAnim.m_pInstance      = TNULL;

		if ( rAnim.m_pBreakpoint )
		{
			delete TSTATICCAST( ASoundBreakpointComponent, rAnim.m_pBreakpoint );
			rAnim.m_pBreakpoint = TNULL;
		}
	}
}

// Destroys the model instances, the collisions, the texture overrides and the object of every instance of the cells,
// then the instance lists of the cells and the cells' box trees
// $deBlob: FUNCTION 006228e0
void ATerrain2Interface::DestroyCellInstances()
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();

	for ( TINT c = 0; c < pTerrain->GetNumCells( m_currentRegion ); c++ )
	{
		TerrainTypes::Cell* pCell = pTerrain->GetCell( m_currentRegion, c );

		for ( TUINT32 i = 0; i < pCell->m_uiNumInstances; i++ )
		{
			TerrainTypes::CellInstance* pInstance = &pCell->m_pInstances[ i ];

			if ( pInstance->m_pModelInstance2 )
				AModelFactory::GetSingleton()->DestroyInstance( TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 ) );

			if ( pInstance->m_pLowLODInstance )
			{
				delete pInstance->m_pLowLODInstance;
				pInstance->m_pLowLODInstance = TNULL;

				// An instance without a streamed model owns the collision made for its low LOD model
				if ( pInstance->m_szModel == TNULL && pInstance->symbol )
				{
					delete TREINTERPRETCAST( ACollisionInstance2*, TREINTERPRETCAST( TUINTPTR, pInstance->symbol ) );
					pInstance->symbol = TNULL;
				}

				if ( ( pInstance->m_uiFlags & 7 ) == 5 && pInstance->m_pTypeData )
				{
					// The texture overrides of the instance: each material is destroyed (OnDestroy, then deleted)
					TerrainTypes::TextureOverride* pOverride = TSTATICCAST( TerrainTypes::TextureOverride, pInstance->m_pTypeData );

					do
					{
						TMaterial* pMaterial = pOverride->m_pMaterial;

						if ( pMaterial != TNULL )
						{
							// Retail clears the material's UV animation (+0x4C) before OnDestroy (vtable +0x0C)
							pMaterial->SetUVAnimation( TNULL );
							pMaterial->Destroy(); // OnDestroy, then the deleting destructor (vtable +0x08)
						}

						pOverride = pOverride->m_pNext;
					} while ( pOverride != TNULL );
				}
			}

			// The ACollisionObject of the instance (flag 0x800)
			if ( pInstance->m_uiUnk1C )
			{
				ACollisionObject* pObject = TREINTERPRETCAST( ACollisionObject*, pInstance->m_uiUnk1C );

				// NOTE: retail removes it only when it is linked
				if ( pObject->IsLinked() )
					pObject->Remove();

				delete pObject;

				pInstance->m_uiUnk1C = 0;
			}
		}

		if ( pCell->m_pUnk38 )
		{
			// NOTE: retail asserts that the list is empty (m_oRoot.m_pNext == &m_oRoot), T2GenericDList has no root here
			delete pCell->m_pUnk38;
			pCell->m_pUnk38 = TNULL;
		}
	}

	if ( m_pPerCellInstancesVisTree )
	{
		delete[] m_pPerCellInstancesVisTree;
		m_pPerCellInstancesVisTree = TNULL;
	}
}

// Destroys the model instances of the low LOD cells that aren't in the pool and the pool itself
// $deBlob: FUNCTION 00623400
void ATerrain2Interface::DestroyCellLowLODs()
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();

	for ( TINT i = 0; i < pTerrain->GetNumCells( m_currentRegion ); i++ )
	{
		TerrainTypes::Cell* pCell = pTerrain->GetCell( m_currentRegion, i );

		// The entries of the pool are destroyed with it
		if ( !( pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_LowLODPool ) && pCell->m_pLowLOD )
			delete TSTATICCAST( TModelInstance, pCell->m_pLowLOD );

		pCell->m_pLowLOD = TNULL;
	}

	if ( m_pCellLowLODPool )
	{
		for ( TINT i = m_numCellLowLODPool; i-- > 0; )
			m_pCellLowLODPool[ i ].~CellLowLOD();

		TFree( m_pCellLowLODPool );
		m_pCellLowLODPool = TNULL;
	}
}

// NOTE: the low LOD pool of the region and the low LOD models of the cells, shared by Init and InitAllCells
// $deBlob: FUNCTION 006230f0
void ATerrain2Interface::InitCellLowLODs()
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();

	m_numCellLowLODPool = pTerrain->GetRegion( m_currentRegion )->m_iNumCellLowLODs;
	m_pCellLowLODPool   = TSTATICCAST( CellLowLOD, TMemalign( 16, m_numCellLowLODPool * sizeof( CellLowLOD ) ) );

	for ( TINT i = 0; i < m_numCellLowLODPool; i++ )
		new ( &m_pCellLowLODPool[ i ] ) CellLowLOD();

	// The entries of the pool in use
	TINT iNumLowLODs = 0;

	for ( TINT iCell = 0; iCell < pTerrain->GetNumCells( m_currentRegion ); iCell++ )
	{
		TerrainTypes::Cell* pCell = pTerrain->GetCell( m_currentRegion, iCell );

		if ( !( pCell->m_uiFlags & TerrainTypes::CellFlag_HasLowLOD ) )
			continue;

		T2ModelPtr pModel = TModelManager::GetSingleton()->FindModel( pCell->m_szLowLODModel );
		TASSERT( pModel && pModel->IsCreated() && "model.IsValid() && model->IsCreated()" );

		// The low LOD shares the pool's world mesh when the cell has no model of its own and the model is a world
		// mesh with cells; otherwise it's a TModelInstance of the cell
		TTMDWin::TRBWorldCellGroup* pWorldGroup = TNULL;

		if ( !( pCell->m_uiFlags & TerrainTypes::CellFlag_HasModel ) && AMaterialInfo::IsModelFirstMeshClass( pModel, &TGetClass( TWorldMeshHAL ) ) )
		{
			TTMDWin::TRBWorldData* pWorldData = TSTATICCAST( TTMDWin::TTRBWorldHeader, pModel->GetAssetData() )->m_pWorldData;

			if ( pWorldData->m_iNumGroups > 0 && pWorldData->m_ppGroups[ 0 ]->m_iNumCells > 0 )
				pWorldGroup = pWorldData->m_ppGroups[ 0 ];
		}

		if ( !pWorldGroup )
		{
			TModelInstance* pInstance = pModel->CreateInstance();
			pCell->m_pLowLOD          = pInstance;
			pInstance->GetTransform().SetMatrix( *pCell->m_pMatrix );
			continue;
		}

		// Looks for the entry of this model, the pool gets a new one otherwise
		CellLowLOD* pEntry = TNULL;

		for ( TINT j = 0; j < iNumLowLODs; j++ )
		{
			if ( m_pCellLowLODPool[ j ].m_szModel == pCell->m_szLowLODModel )
			{
				pEntry = &m_pCellLowLODPool[ j ];
				break;
			}
		}

		if ( !pEntry )
		{
			TASSERT( iNumLowLODs < m_numCellLowLODPool && "currLowLODIndex < m_numCellLowLODPool" );

			pEntry            = &m_pCellLowLODPool[ iNumLowLODs++ ];
			pEntry->m_szModel = pCell->m_szLowLODModel;
			pEntry->m_pModel  = pModel;
			pEntry->m_oWorldVis.Create( pWorldGroup, TNULL );
		}

		pCell->m_uiFlags |= TerrainTypes::CellRuntimeFlag_LowLODPool;
		pCell->m_pLowLOD = pEntry;
	}
}

// $deBlob: FUNCTION 00620910
void ATerrain2Interface::Init( const TVector4* a_pPosition, const TVector4* a_pDirection )
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();

	TINT initCell = pTerrain->GetCellIndex( 0, a_pPosition, TNULL );
	TASSERT( initCell != TerrainTypes::CellVis::NONE, "Start position is not inside any known cell!" );

	TINT* pNeighbours = pTerrain->GetCellVis( m_currentRegion, initCell );
	pTerrain->LoadCell( m_currentRegion, initCell );

	for ( TINT i = 0; i < 8; i++ )
	{
		if ( pNeighbours[ i ] != TerrainTypes::CellVis::NONE )
			pTerrain->LoadCell( m_currentRegion, pNeighbours[ i ] );
	}

	m_iCurrentCell   = initCell;
	m_vStartPosition = *a_pPosition;
	m_vPosition      = *a_pPosition;

	InitCellLowLODs();
}

// $deBlob: FUNCTION 00620cc0
void ATerrain2Interface::InitAllCells()
{
	m_bStreaming = TFALSE;

	ATerrain2* pTerrain = ATerrain2::GetInstance();

	// NOTE: retail loads the cells of region 0 whatever the region (the count is the one of m_currentRegion)
	for ( TINT i = 0; i < pTerrain->GetNumCells( m_currentRegion ); i++ )
		pTerrain->LoadCell( 0, i );

	InitCellLowLODs();
}

// $deBlob: FUNCTION 00621000
void ATerrain2Interface::Render()
{
	TRender*        pRender  = TRender::GetSingleton();
	ATerrain2*      pTerrain = ATerrain2::GetInstance();
	TRenderContext* pContext = pRender->GetCurrentContext();
	const TPlane*   pPlanes  = pContext->GetWorldPlanes();

	TVector4  vAmbientColour  = pContext->GetAmbientColour();
	TMatrix44 mLightColour    = pRender->GetLightColour();
	TMatrix44 mLightDirection = pRender->GetLightDirection();

	TINT  iNumCells = pTerrain->GetNumCells( m_currentRegion );
	TBOOL bOldCull  = ms_bCullInstances;

	AGameStatus::s_uiNumVisiblePaintedInstances   = 0;
	AGameStatus::s_uiNumVisiblePaintableInstances = 0;

	TVector4 vCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();

	m_renderingCellsRoot = TNULL;
	m_usedRenderNodes    = 0;

	TMatrix44 mProjection;
	TRenderContext::ComputePerspectiveProjection( mProjection, pContext->GetViewportParameters(), pContext->GetProjectionParameters() );

	// NOTE: the third argument of RenderCell is the address of an uninitialised local of retail's
	TMatrix44 mUnused;

	for ( TINT i = 0; i < iNumCells; i++ )
	{
		TerrainTypes::Cell* pCell = pTerrain->GetCell( m_currentRegion, i );

		TVector4 aBox[ 2 ];
		aBox[ 0 ] = *pCell->m_pCentre;
		aBox[ 1 ] = *pCell->m_pExtents;

		TBOOL bCulled = TFALSE;

		if ( i != m_iCurrentCell )
		{
			for ( TINT j = 0; j < 6; j++ )
			{
				if ( ClassifyBoxPlane( aBox, pPlanes[ j ] ) == 1 )
				{
					bCulled = TTRUE;
					break;
				}
			}
		}

		if ( !bCulled && !( pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Hidden ) )
		{
			pCell->m_uiFlags |= TerrainTypes::CellRuntimeFlag_Visible;
			AddRenderNode( pCell, TVector4::DistanceSq( vCameraPos, *pCell->m_pCentre ) );
		}
		else
		{
			pCell->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_Visible;
		}
	}

	for ( RenderNode* pNode = m_renderingCellsRoot; pNode != TNULL; pNode = pNode->m_pNext )
	{
		TINT iCell = TINT( pNode->m_pCell - pTerrain->GetRegion( m_currentRegion )->cells );
		RenderCell( iCell, pNode->m_pCell, &mUnused, mProjection, pPlanes );
	}

	m_renderingCellsRoot = TNULL;
	m_usedRenderNodes    = 0;
	ms_bCullInstances      = bOldCull;

	pContext->SetAmbientColour( vAmbientColour );
	pRender->SetLightColourMatrix( mLightColour );
	pRender->SetLightDirectionMatrix( mLightDirection );
}

// $deBlob: FUNCTION 00623900
void ATerrain2Interface::UpdateNearCellCollision()
{
	if ( !m_bSinglePlayer )
		return;

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetCurrentPlayerBlob() );

	if ( pBlob == TNULL )
		return;

	TPhysicsManager::GetSingleton()->BeginBatchOperation();

	TVector4 vPosition = m_bUseBlobPosition ? pBlob->GetBodyPosition() : pBlob->GetViewCamera()->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

	ATerrain2*          pTerrain = ATerrain2::GetInstance();
	TerrainTypes::Cell* pCell    = TNULL;
	TINT                iCell    = pTerrain->GetCellIndex( m_currentRegion, &vPosition, &pCell );
	TINT*               pCellVis = pTerrain->GetCellVis( m_currentRegion, iCell );

	// The cell and its neighbours with their distances
	TINT   aiCells[ 9 ];
	TFLOAT afDistancesSq[ 9 ];

	if ( pCellVis == TNULL )
	{
		for ( TINT i = 0; i < 9; i++ )
		{
			aiCells[ i ]       = TerrainTypes::CellVis::NONE;
			afDistancesSq[ i ] = TMath::MAXFLOAT;
		}
	}
	else
	{
		aiCells[ 0 ]       = iCell;
		afDistancesSq[ 0 ] = 0.0f;

		for ( TINT i = 0; i < 8; i++ )
		{
			aiCells[ i + 1 ] = pCellVis[ i ];

			if ( pCellVis[ i ] == TerrainTypes::CellVis::NONE )
			{
				afDistancesSq[ i + 1 ] = TMath::MAXFLOAT;
			}
			else
			{
				TerrainTypes::Cell* pNeighbour = pTerrain->GetCell( m_currentRegion, pCellVis[ i ] );
				TVector4            vDiff( vPosition.x - pNeighbour->m_pCentre->x, vPosition.y - pNeighbour->m_pCentre->y, vPosition.z - pNeighbour->m_pCentre->z, vPosition.w );
				afDistancesSq[ i + 1 ] = vDiff.MagnitudeSq();
			}
		}
	}

	// Nearest first
	for ( TINT i = 0; i < 9; i++ )
	{
		for ( TINT j = i + 1; j < 9; j++ )
		{
			if ( afDistancesSq[ j ] < afDistancesSq[ i ] )
			{
				TINT   iTemp       = aiCells[ i ];
				TFLOAT fTemp       = afDistancesSq[ i ];
				aiCells[ i ]       = aiCells[ j ];
				afDistancesSq[ i ] = afDistancesSq[ j ];
				aiCells[ j ]       = iTemp;
				afDistancesSq[ j ] = fTemp;
			}
		}
	}

	// The cells that aren't among the 4 nearest any more
	for ( TINT i = 0; i < 4; i++ )
	{
		TBOOL bKept = TFALSE;

		for ( TINT j = 0; j < 4; j++ )
		{
			if ( aiCells[ j ] == m_piNearCells[ i ] )
			{
				bKept = TTRUE;
				break;
			}
		}

		if ( bKept )
			continue;

		TerrainTypes::Cell* pOldCell = pTerrain->GetCell( m_currentRegion, m_piNearCells[ i ] );

		if ( pOldCell != TNULL && ( ( pOldCell->m_uiFlags >> 25 ) & 1 ) )
		{
			RemoveCellInstanceCollision( pOldCell );
			pOldCell->m_uiFlags &= ~0x2000000;
		}
	}

	for ( TINT i = 0; i < 4; i++ )
		m_piNearCells[ i ] = aiCells[ i ];

	for ( TINT i = 0; i < 4; i++ )
	{
		TerrainTypes::Cell* pNearCell = pTerrain->GetCell( m_currentRegion, m_piNearCells[ i ] );

		if ( pNearCell != TNULL && !( ( pNearCell->m_uiFlags >> 25 ) & 1 ) )
		{
			AddCellInstanceCollision( pNearCell );
			pNearCell->m_uiFlags |= 0x2000000;
		}
	}

	TPhysicsManager::GetSingleton()->EndBatchOperation();
}

// $deBlob: FUNCTION 00623bc0
void ATerrain2Interface::AddCellInstanceCollision( TerrainTypes::Cell* a_pCell )
{
	for ( TUINT i = 0; i < a_pCell->m_uiNumInstances; i++ )
	{
		TerrainTypes::CellInstance* pInstance = &a_pCell->m_pInstances[ i ];

		if ( !( ( pInstance->m_uiFlags >> 11 ) & 1 ) || !( ( a_pCell->m_uiFlags >> 3 ) & 1 ) )
			continue;

		AModelInstance2* pModelInstance = TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 );

		if ( pModelInstance != TNULL && pModelInstance->GetCollisionInstance() != TNULL )
		{
			if ( ( pInstance->m_uiFlags >> 23 ) & 1 )
				TSTATICCAST( ACollisionInstance2, pModelInstance->GetCollisionInstance() )->AddToWorld( 0x40 );
		}
		else if ( pInstance->m_szModel == TNULL && pInstance->m_szLowLOD != TNULL && pInstance->symbol != TNULL )
		{
			// NOTE: an instance without a streamed model keeps its ACollisionInstance2 at 0x20
			( (ACollisionInstance2*)pInstance->symbol )->AddToWorld( 0x40 );
		}
	}
}

// $deBlob: FUNCTION 00623c30
void ATerrain2Interface::RemoveCellInstanceCollision( TerrainTypes::Cell* a_pCell )
{
	for ( TUINT i = 0; i < a_pCell->m_uiNumInstances; i++ )
	{
		TerrainTypes::CellInstance* pInstance = &a_pCell->m_pInstances[ i ];

		if ( !( ( pInstance->m_uiFlags >> 11 ) & 1 ) || !( ( a_pCell->m_uiFlags >> 3 ) & 1 ) )
			continue;

		AModelInstance2* pModelInstance = TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 );

		if ( pModelInstance != TNULL && pModelInstance->GetCollisionInstance() != TNULL )
			TSTATICCAST( ACollisionInstance2, pModelInstance->GetCollisionInstance() )->RemoveFromWorld( 0x40 );
		else if ( pInstance->m_szModel == TNULL && pInstance->m_szLowLOD != TNULL && pInstance->symbol != TNULL )
			( (ACollisionInstance2*)pInstance->symbol )->RemoveFromWorld( 0x40 );
	}
}

// $deBlob: FUNCTION 00623db0
// Updates the animated instances (flag 0x20) of the loaded, visible cells: their model, the instance's matrix from its
// bone (and the low LOD's and the model's transforms) and the breakpoint sounds
void ATerrain2Interface::UpdateInstanceAnims( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_numInstAnims; i++ )
	{
		InstanceAnim*               pAnim     = &m_pInstAnims[ i ];
		TerrainTypes::CellInstance* pInstance = pAnim->m_pInstance;

		// Only the instances that are loaded (0x1000000) and in a visible cell
		if ( ( pInstance->m_pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Visible ) == 0 || ( pInstance->m_uiFlags & 0x1000000 ) == 0 )
			continue;

		pAnim->m_pModelInstance->Update( a_fDeltaTime );

		InstanceAnimRecord* pRecord   = TSTATICCAST( InstanceAnimRecord, pInstance->m_pAnimRecord );
		TSkeletonInstance*  pSkeleton = pAnim->m_pModelInstance->GetSkeletonInstance();
		const TMatrix44&    rBone     = pSkeleton->GetBoneTransformCurrent( pRecord->m_iBoneID );

		TMatrix44 mWorld;
		mWorld.Multiply( *pInstance->m_pMatrix, rBone );

		if ( pInstance->m_pLowLODInstance != TNULL )
			pInstance->m_pLowLODInstance->GetTransform().SetMatrix( mWorld );

		TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 )->GetModelInstance()->GetTransform().SetMatrix( mWorld );

		// The breakpoint sounds follow the instance (TickImp, when the instance has entries)
		ASoundBreakpointComponent* pBreakpoint = TSTATICCAST( ASoundBreakpointComponent, pAnim->m_pBreakpoint );

		if ( pBreakpoint != TNULL && pBreakpoint->HasEntry() )
			pBreakpoint->TickImp();
	}
}

// $deBlob: FUNCTION 006212e0
void ATerrain2Interface::Update( TFLOAT a_fDeltaTime )
{
	ATerrain2::GetInstance()->UpdateQueues();
	UpdateInstanceAnims( a_fDeltaTime );
	UpdateStreaming();
	UpdateCellsReady();
	UpdateNearCellCollision();
	m_fTime += a_fDeltaTime;
}

// $deBlob: FUNCTION 00621340
TBOOL ATerrain2Interface::UpdateInstanceCreation( TBOOL a_bTimed )
{
	if ( m_uiFlags & Flag_CreatingInstances )
	{
		THPTimer oTimer;
		TFLOAT   fElapsed = 0.0f;

		if ( a_bTimed )
		{
			oTimer.Reset();
			oTimer.Update();
		}

		do
		{
			CreateInstances();

			TerrainTypes::Cell* pCell = ATerrain2::GetInstance()->GetCell( m_currentRegion, m_uiCreateCell );

			if ( m_uiCreateInstance == pCell->m_uiNumInstances )
			{
				m_uiCreateInstance = 0;
				InitCellInstances( pCell );
				m_uiCreateCell++;

				if ( m_uiCreateCell == ATerrain2::GetInstance()->GetNumCells( m_currentRegion ) )
				{
					m_uiFlags &= ~Flag_CreatingInstances;
					CreatePerCellInstancesVisTree();
					return TFALSE;
				}
			}

			if ( !a_bTimed )
				break;

			oTimer.Update();
			fElapsed += oTimer.GetDelta();
		} while ( fElapsed <= 0.015833333f );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00621490
void ATerrain2Interface::UpdateStreaming()
{
	if ( !m_fnGetPosition )
		return;

	m_fnGetPosition( &m_vPosition );

	ATerrain2*          pTerrain = ATerrain2::GetInstance();
	TerrainTypes::Cell* pCell    = TNULL;
	TINT                iCell    = pTerrain->GetCellIndex( m_currentRegion, &m_vPosition, &pCell );

	if ( !m_bStreaming )
	{
		m_iCurrentCell = iCell;
		return;
	}

	if ( !pCell )
		return;

	if ( iCell != TerrainTypes::CellVis::NONE && !( pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) )
		pTerrain->RequestCell( m_currentRegion, iCell, TTRUE, TTRUE );

	if ( iCell != m_iCurrentCell )
	{
		m_iCurrentCell    = iCell;
		TINT* pNeighbours = pTerrain->GetCellVis( m_currentRegion, iCell );

		for ( TUINT i = 0; i < 8; i++ )
		{
			if ( pNeighbours[ i ] != TerrainTypes::CellVis::NONE )
			{
				pTerrain->GetCell( m_currentRegion, pNeighbours[ i ] );
				pTerrain->RequestCell( m_currentRegion, pNeighbours[ i ], TTRUE, TTRUE );
			}
		}

		if ( ms_bCancelFarLoads )
			pTerrain->CancelQueuedLoads( m_currentRegion, m_iCurrentCell, pNeighbours );
	}

	TINT* pNeighbours = pTerrain->GetCellVis( m_currentRegion, m_iCurrentCell );

	for ( TINT i = 0; i < 8; i++ )
	{
		if ( pNeighbours[ i ] == TerrainTypes::CellVis::NONE )
			continue;

		TINT* pFarNeighbours = pTerrain->GetCellVis( m_currentRegion, pNeighbours[ i ] );

		for ( TINT j = 0; j < 8; j++ )
		{
			TINT iFarCell = pFarNeighbours[ j ];

			if ( iFarCell == TerrainTypes::CellVis::NONE )
				continue;

			TINT k = 0;

			for ( ; k < 8; k++ )
			{
				if ( pNeighbours[ k ] == iFarCell )
					break;
			}

			if ( k < 8 )
				continue;

			if ( pTerrain->GetCell( m_currentRegion, iFarCell )->m_uiFlags & TerrainTypes::CellRuntimeFlag_Visible )
				pTerrain->RequestCell( m_currentRegion, iFarCell, TTRUE, TFALSE );
		}
	}
}

// $deBlob: FUNCTION 00573390
// The "Main" symbol of the level's TRB: the instance animation sheet of ATerrain2Interface::m_pAnimSheet
static void* GetAnimSheet()
{
	TTRB* pTRB = ACurrentLevelInformation::GetSingleton()->Stream( 4 );
	return pTRB->GetSymbolAddress( "Main" );
}

// $deBlob: FUNCTION 006234a0
void ATerrain2Interface::InitWorldVisPool()
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();

	TUINT32 uiNumWorldVis = pTerrain->GetRegion( m_currentRegion )->m_uiNumWorldVis;
	m_worldVisPoolSize    = uiNumWorldVis;
	m_pWorldVisPool       = TSTATICCAST( WorldVisEntry, TMemalign( 16, uiNumWorldVis * sizeof( WorldVisEntry ) ) );

	for ( TUINT32 i = 0; i < uiNumWorldVis; i++ )
		new ( &m_pWorldVisPool[ i ] ) WorldVisEntry();

	// The entries in use (the pool's first free entry)
	TUINT32 uiNumUsed = 0;

	for ( TINT c = 0; c < pTerrain->GetNumCells( m_currentRegion ); c++ )
	{
		TerrainTypes::Cell* pCell = pTerrain->GetCell( m_currentRegion, c );

		for ( TUINT32 i = 0; i < pCell->m_uiNumInstances; i++ )
		{
			TerrainTypes::CellInstance* pInstance = &pCell->m_pInstances[ i ];

			// Only the flag 0x40 instances (the meshes of a symbol of the cell's TRB) have a world vis
			if ( !( pInstance->m_uiFlags & 0x40 ) )
				continue;

			TASSERT( pInstance->m_szModel && pInstance->m_szModel[ 0 ] );

			TCHAR* szModelName = TStringManager::GetTempString8();
			T2String8::Format( szModelName, "%s.txs", pInstance->m_szModel );
			T2String8::ToLowerCase( szModelName );

			T2ModelPtr pModel = TModelManager::GetSingleton()->FindModel( szModelName );
			TASSERT( pModel && pModel->IsCreated() && "model.IsValid() && model->IsCreated()" );

			// The world vis is only made for a model whose world has cells with more than one mesh
			TTMDWin::TRBWorldData* pWorldData = TSTATICCAST( TTMDWin::TTRBWorldHeader, pModel->GetAssetData() )->m_pWorldData;

			if ( pWorldData->m_iNumGroups <= 0 )
				continue;

			TTMDWin::TRBWorldCellGroup* pGroup = pWorldData->m_ppGroups[ 0 ];

			if ( pGroup->m_iNumCells <= 0 )
				continue;

			TTMDWin::TRBWorldCell* pWorldCell = pGroup->m_ppCells[ 0 ];

			if ( pWorldCell->m_iMeshCount <= 1 || pWorldCell->m_uiUnk00 <= 1 )
				continue;

			// The model's entry is shared by the instances of the model
			TUINT32 uiIndex = 0;

			for ( ; uiIndex < uiNumWorldVis; uiIndex++ )
			{
				if ( m_pWorldVisPool[ uiIndex ].m_pModel.GetID() != T2ResourcePtr::IDINVALID && m_pWorldVisPool[ uiIndex ].m_pModel.GetID() == pModel.GetID() )
					break;
			}

			if ( uiIndex >= uiNumWorldVis )
			{
				TASSERT( uiNumUsed < uiNumWorldVis && "current < size" );

				WorldVisEntry& rEntry = m_pWorldVisPool[ uiNumUsed ];
				rEntry.m_pModel       = pModel;
				rEntry.m_oWorldVis.Create( pGroup, TNULL );

				uiIndex = uiNumUsed++;
			}

			pInstance->worldVisIndex = TUINT8( uiIndex );
		}
	}
}

// Destroys the instance anims, the cell instances and the low LODs
// $deBlob: FUNCTION 00621600
void ATerrain2Interface::DestroyInstances()
{
	DestroyInstanceAnims();
	DestroyCellInstances();
	DestroyCellLowLODs();
}

// $deBlob: FUNCTION 00621620
void ATerrain2Interface::StartInstanceCreation()
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();

	TINT iNumModels         = 0;
	TINT iNumType2          = 0;
	TINT iNumType2Unk15     = 0;
	TINT iNumType2Flagged   = 0;
	TINT iNumType3          = 0;
	TINT iNumType1Unflagged = 0;

	for ( TUINT c = 0; c < TUINT( pTerrain->GetNumCells( m_currentRegion ) ); c++ )
	{
		TerrainTypes::Cell* pCell = pTerrain->GetCell( m_currentRegion, c );

		for ( TUINT i = 0; i < pCell->m_uiNumInstances; i++ )
		{
			TerrainTypes::CellInstance* pInstance = &pCell->m_pInstances[ i ];
			TUINT32                     uiFlags   = pInstance->m_uiFlags;
			TUINT32                     uiType    = uiFlags & 7;

			pInstance->worldVisIndex = 0xFF;

			if ( uiType == 2 )
			{
				TUINT8* pTypeData = TSTATICCAST( TUINT8, pInstance->m_pTypeData );

				if ( ( pTypeData[ 0x14 ] & 1 ) == 0 )
				{
					iNumType2++;

					if ( pTypeData[ 0x15 ] != 0 )
						iNumType2Unk15++;
				}
				else
				{
					iNumType2Flagged++;
					pInstance->m_uiFlags = uiFlags | 0x80000000;
				}
			}
			else if ( uiType == 3 )
			{
				iNumType3++;
			}
			else if ( uiType == 4 )
			{
				iNumModels++;
			}
			else if ( uiType == 1 && pInstance->m_szModel != TNULL && pInstance->m_szModel[ 0 ] != '\0' )
			{
				iNumModels++;

				if ( ( uiFlags & 0x40000 ) == 0 )
					iNumType1Unflagged++;
			}
		}
	}

	InitWorldVisPool();

	m_pAnimSheet = GetAnimSheet();
	ABuildingManager::GetSingleton()->Init( iNumModels, iNumType2Flagged + iNumType2 );

	m_uiFlags |= Flag_CreatingInstances;
	m_uiCreateCell     = 0;
	m_uiCreateInstance = 0;

	AGameStatus* pGameStatus                        = AGameStatus::GetSingleton();
	pGameStatus->m_oLevelData.m_auiLevelCounts[ 2 ] = iNumType2Unk15;
	pGameStatus->m_oLevelData.m_auiLevelCounts[ 0 ] = iNumType1Unflagged;
	pGameStatus->m_oLevelData.m_auiLevelCounts[ 1 ] = iNumType3;
	pGameStatus->m_oLevelData.m_auiLevelCounts[ 3 ] = iNumType2Flagged;
}

// $deBlob: FUNCTION 00621780
TBOOL ATerrain2Interface::IsCurrentCellReady()
{
	if ( m_iCurrentCell == TerrainTypes::CellVis::NONE )
		return TTRUE;

	TerrainTypes::Cell* pCell = ATerrain2::GetInstance()->GetCell( m_currentRegion, m_iCurrentCell );

	if ( !pCell )
		return TFALSE;

	TUINT32 uiFlags = pCell->m_uiFlags;

	return ( ( uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) && !( uiFlags & TerrainTypes::CellRuntimeFlag_Unloading ) && ( uiFlags & TerrainTypes::CellRuntimeFlag_Ready ) ) ||
	    ( uiFlags & TerrainTypes::CellRuntimeFlag_Hidden );
}

// $deBlob: FUNCTION 006217d0
TBOOL ATerrain2Interface::IsCurrentCellAreaReady()
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();

	if ( m_iCurrentCell != TerrainTypes::CellVis::NONE )
	{
		TerrainTypes::Cell* pCell = pTerrain->GetCell( m_currentRegion, m_iCurrentCell );

		if ( pCell == TNULL )
			return TFALSE;

		TUINT32 uiFlags = pCell->m_uiFlags;

		if ( !( ( ( uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) && !( uiFlags & TerrainTypes::CellRuntimeFlag_Unloading ) && ( uiFlags & TerrainTypes::CellRuntimeFlag_Ready ) ) ||
		        ( uiFlags & TerrainTypes::CellRuntimeFlag_Hidden ) ) )
			return TFALSE;
	}

	if ( m_iCurrentCell != TerrainTypes::CellVis::NONE )
	{
		TINT* piNeighbours = pTerrain->GetCellVis( m_currentRegion, m_iCurrentCell );

		for ( TUINT i = 0; i < 8; i++ )
		{
			if ( piNeighbours[ i ] == TerrainTypes::CellVis::NONE )
				continue;

			TUINT32 uiFlags = pTerrain->GetCell( m_currentRegion, piNeighbours[ i ] )->m_uiFlags;

			// A visible neighbour that isn't loaded yet
			if ( ( uiFlags & TerrainTypes::CellRuntimeFlag_Visible ) && ( !( uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) || ( uiFlags & TerrainTypes::CellRuntimeFlag_Unloading ) ) )
				return TFALSE;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006218a0
TBOOL ATerrain2Interface::IsStreaming()
{
	return ATerrain2::GetInstance()->IsStreaming();
}

// $deBlob: FUNCTION 006218c0
void ATerrain2Interface::OnCellLoaded( ACellBucket* a_pBucket )
{
	TPhysicsManager::GetSingleton()->BeginBatchOperation();

	TerrainTypes::Cell* pCell = a_pBucket->GetCell();

	for ( TUINT i = 0; i < pCell->m_uiNumInstances; i++ )
	{
		TerrainTypes::CellInstance* pInstance = &pCell->m_pInstances[ i ];
		TUINT32                     uiMask    = m_bSinglePlayer ? 0x20 : 0x60;

		if ( ( pInstance->m_uiFlags & 0x800 ) && ( a_pBucket->GetCell()->m_uiFlags & TerrainTypes::CellFlag_Collision ) )
		{
			AModelInstance2* pModelInstance = TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 );

			if ( pModelInstance != TNULL && pModelInstance->GetCollisionInstance() != TNULL )
			{
				if ( ( pInstance->m_uiFlags >> 23 ) & 1 )
					TSTATICCAST( ACollisionInstance2, pModelInstance->GetCollisionInstance() )->AddToWorld( uiMask );
			}
			else if ( pInstance->m_szModel == TNULL && pInstance->m_szLowLOD != TNULL && pInstance->symbol != TNULL )
			{
				// NOTE: an instance without a streamed model keeps its ACollisionInstance2 at 0x20
				( (ACollisionInstance2*)pInstance->symbol )->AddToWorld( uiMask );
			}
		}

		if ( ( pInstance->m_uiFlags & 0x40 ) && pInstance->symbol != TNULL )
		{
			pInstance->m_pSymbolData = TSTATICCAST( TerrainTypes::InstanceMeshes, a_pBucket->GetTRB()->GetSymbolAddress( pInstance->symbol ) );
			a_pBucket->CreateInstanceMaterials( pInstance );
		}

		pInstance->m_uiFlags |= 0x1000000;

		// NOTE: retail calls the callback of the type from a table
		switch ( pInstance->m_uiFlags & 7 )
		{
			case TerrainTypes::InstanceType_Landmark:
				ALandmarkInstance::OnAddedToWorld( pInstance );
				break;
			case TerrainTypes::InstanceType_ToggleVis:
				AToggleVisInstance::OnAddedToWorld( pInstance );
				break;
		}
	}

	AEntityFactory::GetSingleton()->OnCellLoaded( a_pBucket->GetCell() );

	APositionalSoundManager::GetSingleton()->OnCellLoaded( a_pBucket->GetCell() );

	TPhysicsManager::GetSingleton()->EndBatchOperation();
}

// $deBlob: FUNCTION 006219e0
void ATerrain2Interface::OnCellUnloaded( ACellBucket* a_pBucket )
{
	TPhysicsManager::GetSingleton()->BeginBatchOperation();

	TerrainTypes::Cell* pCell = a_pBucket->GetCell();

	for ( TUINT i = 0; i < pCell->m_uiNumInstances; i++ )
	{
		TerrainTypes::CellInstance* pInstance = &pCell->m_pInstances[ i ];

		if ( !( pInstance->m_uiFlags & 0x1000000 ) )
			continue;

		pInstance->m_uiFlags &= ~0x1000000;
		TUINT32 uiMask = m_bSinglePlayer ? 0x20 : 0x60;

		if ( ( pInstance->m_uiFlags & 0x800 ) && ( pCell->m_uiFlags & TerrainTypes::CellFlag_Collision ) )
		{
			AModelInstance2* pModelInstance = TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 );

			if ( pModelInstance != TNULL && pModelInstance->GetCollisionInstance() != TNULL )
				TSTATICCAST( ACollisionInstance2, pModelInstance->GetCollisionInstance() )->RemoveFromWorld( uiMask );
			else if ( pInstance->m_szModel == TNULL && pInstance->m_szLowLOD != TNULL && pInstance->symbol != TNULL )
				( (ACollisionInstance2*)pInstance->symbol )->RemoveFromWorld( uiMask );
		}

		// NOTE: retail calls the callback of the type from a table
		switch ( pInstance->m_uiFlags & 7 )
		{
			case TerrainTypes::InstanceType_Landmark:
				ALandmarkInstance::OnRemovedFromWorld( pInstance );
				break;
			case TerrainTypes::InstanceType_TextureSwap:
				ATextureSwapInstance::OnRemovedFromWorld( pInstance );
				break;
		}

		if ( ( pInstance->m_uiFlags & 0x40 ) && pInstance->symbol != TNULL )
		{
			AJellyShader_DX11::GetSingleton()->OnInstanceRemoved( pInstance );
			pInstance->m_pSymbolData = TNULL;
		}
	}

	a_pBucket->ReleaseMaterials();

	AEntityFactory::GetSingleton()->OnCellUnloaded( pCell );

	TPhysicsManager::GetSingleton()->EndBatchOperation();
}

// $deBlob: FUNCTION 00621cb0
TBOOL __stdcall ATerrain2Interface::IsInstanceInView( TerrainTypes::CellInstance* a_pInstance )
{
	TModelInstance* pModelInstance = TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 )->GetModelInstance();

	T2ModelPtr pModel  = pModelInstance->GetModel();
	TSphere    oSphere = pModel->GetLOD( 0 ).GetRenderVolume();

	const TVector3& rScale = pModelInstance->GetTransform().GetScale();
	TVector3        vCentre( oSphere.GetOrigin().x * rScale.x, oSphere.GetOrigin().y * rScale.y, oSphere.GetOrigin().z * rScale.z );

	TFLOAT fMaxScale = rScale.x;
	if ( fMaxScale < rScale.y ) fMaxScale = rScale.y;
	if ( fMaxScale < rScale.z ) fMaxScale = rScale.z;

	TFLOAT fRadius = fMaxScale * oSphere.GetRadius();

	if ( fRadius <= 0.0f )
		return TTRUE;

	TMatrix44::TransformVector( vCentre, *a_pInstance->m_pMatrix, vCentre );

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
	return TRenderContext::CullSphereToFrustumSimple( TSphere( vCentre, fRadius ), pCamera->GetViewPlanes(), 6 );
}

// $deBlob: FUNCTION 00624f40
// $deBlob: FUNCTION 00625150
ATerrain2Interface::InstanceAnim::~InstanceAnim()
{
	m_pModelInstance = TNULL;
	m_pInstance      = TNULL;

	if ( m_pBreakpoint != TNULL )
	{
		delete TSTATICCAST( ASoundBreakpointComponent, m_pBreakpoint );
		m_pBreakpoint = TNULL;
	}
}

// $deBlob: FUNCTION 00621e10
void ATerrain2Interface::SetInstanceInCellList( TerrainTypes::CellInstance* a_pInstance, TBOOL a_bAdd )
{
	TerrainTypes::Cell* pCell   = a_pInstance->m_pCell;
	TBOOL               bInList = ( a_pInstance->m_uiFlags >> 29 ) & 1;

	if ( !a_bAdd )
	{
		if ( !bInList )
			return;
	}
	else if ( bInList )
	{
		return;
	}

	ATerrain2* pTerrain   = ATerrain2::GetInstance();
	TUINT      uiNumCells = pTerrain->GetNumCells( m_currentRegion );
	TUINT      uiCell     = 0;

	if ( uiNumCells == 0 )
		return;

	while ( pTerrain->GetCell( m_currentRegion, uiCell ) != pCell )
	{
		uiCell++;

		if ( uiNumCells <= uiCell )
			return;
	}

	if ( uiCell == 0xffffffff )
		return;

	void** ppInstances = TSTATICCAST( void*, m_ppCellInstanceLists[ uiCell ] );

	if ( !a_bAdd )
	{
		TINT  iNumInstances = m_piNumCellInstances[ uiCell ];
		TINT  iDest         = 0;
		TBOOL bFound        = TFALSE;

		for ( TINT i = 0; i < iNumInstances; i++ )
		{
			if ( ppInstances[ i ] == a_pInstance )
				bFound = TTRUE;
			else
				ppInstances[ iDest++ ] = ppInstances[ i ];
		}

		if ( bFound )
			m_piNumCellInstances[ uiCell ]--;

		a_pInstance->m_uiFlags &= ~0x20000000;
	}
	else
	{
		TUINT uiNumInstances = m_piNumCellInstances[ uiCell ];

		if ( uiNumInstances < 8 )
		{
			for ( TUINT i = 0; i < uiNumInstances; i++ )
			{
				if ( ppInstances[ i ] == a_pInstance )
					return;
			}

			a_pInstance->m_uiFlags |= 0x20000000;
			ppInstances[ uiNumInstances ] = a_pInstance;
			m_piNumCellInstances[ uiCell ]++;
		}
	}
}

// $deBlob: FUNCTION 00621f40
TBOOL ATerrain2Interface::IsPositionInReadyCell( const TVector4& a_rPosition, TBOOL a_bCheckNeighbours )
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();

	TerrainTypes::Cell* pCell      = TNULL;
	TINT                iCellIndex = pTerrain->GetCellIndex( m_currentRegion, &a_rPosition, &pCell );

	if ( pCell != TNULL && !HASANYFLAG( pCell->m_uiFlags, TerrainTypes::CellRuntimeFlag_Ready ) )
		return TFALSE;

	if ( a_bCheckNeighbours )
	{
		TINT* pCellVis = pTerrain->GetCellVis( m_currentRegion, iCellIndex );

		if ( pCellVis != TNULL )
		{
			for ( TINT i = 0; i < 8; i++ )
			{
				TerrainTypes::Cell* pNeighbour = pTerrain->GetCell( m_currentRegion, pCellVis[ i ] );

				if ( pNeighbour != TNULL && !HASANYFLAG( pNeighbour->m_uiFlags, TerrainTypes::CellRuntimeFlag_Ready ) )
				{
					const TVector4& rCentre  = *pNeighbour->m_pCentre;
					const TVector4& rExtents = *pNeighbour->m_pExtents;

					if ( a_rPosition.x >= rCentre.x - rExtents.x && a_rPosition.y >= rCentre.y - rExtents.y &&
					     a_rPosition.z >= rCentre.z - rExtents.z && rExtents.x + rCentre.x >= a_rPosition.x &&
					     rExtents.y + rCentre.y >= a_rPosition.y && rExtents.z + rCentre.z >= a_rPosition.z )
					{
						return TFALSE;
					}
				}
			}
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00622060
void ATerrain2Interface::RenderCellLowLOD( TerrainTypes::Cell* a_pCell, TMatrix44* a_pUnused, const TMatrix44& a_rProjection )
{
	if ( !( a_pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_LowLODPool ) )
	{
		RenderLowLODModel( TSTATICCAST( TModelInstance, a_pCell->m_pLowLOD ) );
		return;
	}

	// The low LOD is drawn with an ambient colour of (1, 1, 1); the context's own is restored at the end
	TRenderContext* pContext       = TRender::GetSingleton()->GetCurrentContext();
	TVector4        vAmbientColour = pContext->GetAmbientColour();
	pContext->SetAmbientColour( 1.0f, 1.0f, 1.0f );

	m_oTransform.SetMatrix( *a_pCell->m_pMatrix );
	m_oTransform.Push();

	TMatrix44 mModelView = TRender::GetSingleton()->GetCurrentContext()->GetModelViewMatrix();

	CellLowLOD* pLowLOD = TSTATICCAST( CellLowLOD, a_pCell->m_pLowLOD );
	pLowLOD->m_oWorldVis.SetRenderCallbackUser( OnRenderCellLowLODMesh, a_pCell );
	pLowLOD->m_oWorldVis.Reset();
	pLowLOD->m_oWorldVis.Build( mModelView, a_rProjection, 0, TFALSE );
	pLowLOD->m_oWorldVis.Render( mModelView, a_rProjection, TFALSE );

	m_oTransform.Pop();
	pContext->SetAmbientColour( vAmbientColour );
}

// The world vis callback of a low LOD cell: draws the cell's world mesh
// $deBlob: FUNCTION 00624c60
void __stdcall ATerrain2Interface::OnRenderCellLowLODMesh( TTMDWin::TRBWorldCellMesh* a_pCellMesh, TWorldVis::RenderData* a_pRenderData, void* a_pCell )
{
	TWorldMeshHAL* pCellMesh = TSTATICCAST( TTRBWorldMesh, a_pCellMesh->m_pWorldMesh )->m_pMesh;
	TASSERT( pCellMesh->IsA( &TGetClass( TWorldMeshHAL ) ) );

	if ( ms_bRenderLowLODMeshes )
	{
		TBOOL bRes = pCellMesh->Render();
		TASSERT( bRes );
	}
}

// $deBlob: FUNCTION 006221b0
void ATerrain2Interface::RenderCellModel( TerrainTypes::Cell* a_pCell, TMatrix44* a_pUnused, const TMatrix44& a_rProjection )
{
	if ( a_pCell && ( a_pCell->m_uiFlags & TerrainTypes::CellFlag_HasModel ) )
	{
		TerrainTypes::CellData* pData = a_pCell->m_pData;

		{
			// The UV animations of the materials of the cell's model (AMaterialManager::UpdateUVs)
			T2ModelPtr pModel = pData->m_pModelInstance->GetModel();
			AMaterialManager::GetSingleton()->UpdateUVs( pModel.GetData() );
		}

		TWorldVis* pWorldVis = pData->m_pWorldVis;

		if ( pWorldVis )
		{
			TTransformObject& rTransform = pData->m_pModelInstance->GetTransform();
			rTransform.Push();

			TMatrix44 mModelView = TRender::GetSingleton()->GetCurrentContext()->GetModelViewMatrix();
			pWorldVis->Reset();
			pWorldVis->Build( mModelView, a_rProjection, 0, TFALSE );
			pWorldVis->Render( mModelView, a_rProjection, TFALSE );

			pData->m_pModelInstance->GetTransform().Pop();
		}
	}
}

// $deBlob: FUNCTION 006222d0
void ATerrain2Interface::RenderCell( TINT a_iCell, TerrainTypes::Cell* a_pCell, TMatrix44* a_pUnused, const TMatrix44& a_rProjection, const TPlane* a_pPlanes )
{
	TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();
	TUINT32         uiFlags  = a_pCell->m_uiFlags;

	if ( !( uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) || ( uiFlags & TerrainTypes::CellRuntimeFlag_QueuedUnload ) || !( uiFlags & TerrainTypes::CellFlag_HasModel ) )
	{
		if ( uiFlags & TerrainTypes::CellFlag_HasLowLOD )
			RenderCellLowLOD( a_pCell, a_pUnused, a_rProjection );
	}
	else
	{
		RenderCellModel( a_pCell, a_pUnused, a_rProjection );
	}

	if ( a_pCell->m_pUnk34 != TNULL )
	{
		ms_bCullInstances = TFALSE;

		TBoxTree& rTree = m_pPerCellInstancesVisTree[ a_iCell ];
		rTree.SetUserData( const_cast<TMatrix44*>( &a_rProjection ) );
		rTree.SetPlanes( a_pPlanes );
		rTree.SetViewPosition( pContext->GetWorldViewMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) );
		rTree.SetFlag( TBoxTree::Flags_SortByDistance );
		rTree.Render();
		rTree.ClearFlag( TBoxTree::Flags_SortByDistance );
		rTree.SetUserData( TNULL );
	}

	ms_bCullInstances = TTRUE;

	for ( TUINT i = 0; i < a_pCell->m_uiNumUnk2C; i++ )
		RenderInstance( a_pCell->m_ppUnk2C[ i ], const_cast<TMatrix44*>( &a_rProjection ) );

	TINT   iNumInstances = m_piNumCellInstances[ a_iCell ];
	void** ppInstances   = TSTATICCAST( void*, m_ppCellInstanceLists[ a_iCell ] );

	for ( TINT i = 0; i < iNumInstances; i++ )
		RenderInstance( ppInstances[ i ], const_cast<TMatrix44*>( &a_rProjection ) );
}

// $deBlob: FUNCTION 006223e0
void ATerrain2Interface::AddRenderNode( TerrainTypes::Cell* a_pCell, TFLOAT a_fDistanceSq )
{
	if ( m_usedRenderNodes == MAX_RENDERING_CELLS )
		return;

	TASSERT( m_usedRenderNodes < MAX_RENDERING_CELLS );

	RenderNode* pNode    = &m_pRenderNodes[ m_usedRenderNodes++ ];
	pNode->m_pNext       = TNULL;
	pNode->m_pCell       = a_pCell;
	pNode->m_fDistanceSq = a_fDistanceSq;

	RenderNode* pPrev = TNULL;
	RenderNode* pIt   = m_renderingCellsRoot;

	for ( ; pIt != TNULL && a_fDistanceSq <= pIt->m_fDistanceSq; pIt = pIt->m_pNext )
		pPrev = pIt;

	pNode->m_pNext = pIt;

	if ( pPrev )
		pPrev->m_pNext = pNode;
	else
		m_renderingCellsRoot = pNode;
}

// $deBlob: FUNCTION 00622490
ACollisionInstance2* ATerrain2Interface::CreateInstanceCollision( TerrainTypes::CellInstance* a_pInstance, TModelInstance* a_pModelInstance )
{

	T2ModelPtr           pModel     = a_pModelInstance->GetModel();
	ACollisionInstance2* pCollision = TNULL;

	ACollisionInstance2::OwnerInfo oOwner;
	oOwner.m_eType  = 3;
	oOwner.m_pOwner = a_pInstance;

	if ( a_pInstance->m_pUnk28 != TNULL )
	{
		// The collision primitives of the instance
		pCollision = new ACollisionInstance2;
		pCollision->SetModelInstance( a_pModelInstance, TREINTERPRETCAST( ACollisionInstance2::OwnerObject*, a_pInstance ) );
		pCollision->CreatePhysicsObjects( a_pModelInstance, &oOwner, TREINTERPRETCAST( ACollisionInstance2::PhysicsRecord*, a_pInstance->m_pUnk28 ) );
	}
	else if ( pModel.GetData()->GetCollisionDataCount() > 0 )
	{
		// The collision meshes of the model
		pCollision = new ACollisionInstance2;
		pCollision->SetModelInstance( a_pModelInstance, TREINTERPRETCAST( ACollisionInstance2::OwnerObject*, a_pInstance ) );
		pCollision->Create( a_pModelInstance, &oOwner, TFALSE, 0, TNULL );
	}

	return pCollision;
}

// $deBlob: FUNCTION 006225e0
// Sets up an animated instance (flag 0x20): the instance's TRB record gets its model instance and its bone and sequence
// IDs, the sequence is played on the model's skeleton and the instance is added to m_pInstAnims.
void ATerrain2Interface::CreateInstanceAnim( TerrainTypes::CellInstance* a_pInstance )
{
	InstanceAnimRecord* pRecord = TSTATICCAST( InstanceAnimRecord, a_pInstance->m_pAnimRecord );

	TASSERT( m_numInstAnims < TINT( ATerrain2::GetInstance()->GetRegion( m_currentRegion )->numInstAnimModel ) );

	InstanceAnim* pAnim = &m_pInstAnims[ m_numInstAnims ];
	m_numInstAnims++;
	pAnim->m_pInstance = a_pInstance;

	// The sheet lookup below needs the name, the record's first word is replaced by the model instance
	const TCHAR* szName = pRecord->m_szModel;

	TCHAR* szFileName = TStringManager::GetTempString8();
	T2String8::Format( szFileName, "%s.txs", szName );
	T2String8::ToLowerCase( szFileName );

	// The T2ModelPtr holds the model's reference (retail's DecRefCount at the end)
	T2ModelPtr      pModel         = TModelManager::GetSingleton()->FindModel( szFileName );
	TModelInstance* pModelInstance = pModel.GetData()->CreateInstance();

	pRecord->m_pModelInstance = pModelInstance;
	pAnim->m_pModelInstance   = pModelInstance;

	TSkeletonInstance* pSkeletonInstance = pModelInstance->GetSkeletonInstance();
	TSkeleton*         pSkeleton         = pSkeletonInstance->GetSkeleton();

	TINT iBoneID     = pSkeleton->GetBoneID( pRecord->m_szBone, -1 );
	TINT iSequenceID = pSkeleton->GetSequenceID( pRecord->m_szSequence, -1 );

	pRecord->m_iBoneID     = iBoneID;
	pRecord->m_iSequenceID = iSequenceID;

	// The bodies of the instance follow their bones (keyframed movers)
	ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 )->GetCollisionInstance() );

	if ( pCollision != TNULL )
		pCollision->CreateKeyframeMovers();

	TAnimation* pAnimation = pSkeletonInstance->AddAnimation( TINT16( iSequenceID ), 1.0f, 0.0f );

	TASSERT( iSequenceID < pSkeleton->GetSequenceCount() );

	TFLOAT fDuration  = pSkeleton->GetSequence( iSequenceID )->GetDuration();
	TFLOAT fStartTime = 0.0f;

	if ( pRecord->m_uiFlags & 1 )
	{
		// A random start time from the shared random pool (ARandom's TRandom, the top bit is dropped)
		TUINT32 uiRandom = ARandom::GetSingleton()->GetRandom().RandRaw();

		fStartTime = TFLOAT( uiRandom >> 1 ) * 4.6566129e-10f * fDuration;
	}
	else if ( pRecord->m_uiFlags & 2 )
	{
		fStartTime = pRecord->m_fStartTime * fDuration;
	}

	pAnimation->SetSpeed( *TREINTERPRETCAST( TFLOAT*, &pRecord->m_iSpeed ) );
	pAnimation->SetTime( fStartTime );

	if ( ASoundBreakpointComponent::FindEntry( TSTATICCAST( ASoundBreakpointComponent::BreakpointTable, m_pAnimSheet ), szName ) != TNULL )
	{
		// The breakpoint sounds of the instance (ASoundBreakpointComponent, 0x70 bytes); the position is the matrix's row 3
		ASoundBreakpointComponent* pBreakpoint = new ASoundBreakpointComponent( pModelInstance, szName );

		pAnim->m_pBreakpoint = pBreakpoint;
		pBreakpoint->PatchAnimIDs( pModelInstance );
		pBreakpoint->m_pPosition = TREINTERPRETCAST( TUINT8*, a_pInstance->m_pMatrix ) + 0x30;
	}
}

// The entity factory's and the positional sounds' part of OnCellLoaded
// $deBlob: FUNCTION 00622bc0
void ATerrain2Interface::NotifyCellLoaded( ACellBucket* a_pBucket )
{
	AEntityFactory::GetSingleton()->OnCellLoaded( a_pBucket->GetCell() );
	APositionalSoundManager::GetSingleton()->OnCellLoaded( a_pBucket->GetCell() );
}

// $deBlob: FUNCTION 00622bf0
void ATerrain2Interface::CreateInstances()
{
	TerrainTypes::Cell* pCell = ATerrain2::GetInstance()->GetCell( m_currentRegion, m_uiCreateCell );

	for ( TINT i = 0; i < 5; i++ )
	{
		if ( pCell->m_uiNumInstances <= m_uiCreateInstance )
			break;

		TerrainTypes::CellInstance* pInstance = &pCell->m_pInstances[ m_uiCreateInstance ];
		pInstance->m_pCell                    = pCell;

		if ( !( pInstance->m_uiFlags & 0x200 ) )
			pInstance->m_uiFlags |= 0x800000;

		// The streamed model
		AModelInstance2* pModelInstance = TNULL;

		if ( pInstance->m_szModel != TNULL )
		{
			TCHAR* szFileName = TStringManager::GetTempString8();
			T2String8::Format( szFileName, "%s.trb", pInstance->m_szModel );
			pModelInstance = AModelFactory::GetSingleton()->CreateUnmanagedInstance( szFileName, TFALSE, pInstance->m_uiFlags, TTRUE );

			TUINT32 uiType = pInstance->m_uiFlags & 7;

			if ( uiType != 1 && uiType != 2 && uiType != 3 && uiType != 4 )
			{
				// The skinned meshes of LOD 0 (ASkinMeshHAL) take the instance's ambient scale (ALightingData)
				T2ModelPtr pModelPtr = pModelInstance->GetModel()->GetModelPtr();
				TModelLOD& rLOD      = pModelPtr.GetData()->GetLOD( 0 );

				for ( TINT iMesh = 0; iMesh < rLOD.GetMeshCount(); iMesh++ )
				{
					if ( rLOD.GetMesh( iMesh )->GetClass() == &TGetClass( ASkinMeshHAL ) )
					{
						pModelInstance->SetAmbientScale( ALightingData::GetSingleton()->GetInstanceAmbientScale( pInstance->m_iAmbientScaleIndex ) );
						break;
					}
				}
			}

			if ( !( ( pInstance->m_uiFlags >> 11 ) & 1 ) || !( ( pCell->m_uiFlags >> 3 ) & 1 ) )
			{
				pInstance->m_uiUnk1C = 0;
			}
			else
			{
				// The collision of the instance
				ACollisionInstance2* pCollision = CreateInstanceCollision( pInstance, pModelInstance->GetModelInstance() );

				if ( pCollision != TNULL )
				{
					pModelInstance->SetCollisionInstance( pCollision );
					pInstance->m_uiUnk1C = TREINTERPRETCAST( TUINT32, new ACollisionObject( TNULL, 0, 0xff ) );
				}
			}

			pModelInstance->SetTransform( *pInstance->m_pMatrix );
			pModelInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

			pInstance->m_pModelInstance2 = pModelInstance;
		}

		// The low LOD model
		if ( pInstance->m_szLowLOD == TNULL || *pInstance->m_szLowLOD == '\0' )
		{
			pInstance->m_pLowLODInstance = TNULL;
		}
		else
		{
			TCHAR*     szTempString = TStringManager::GetTempString8();
			TINT       iNameOffset  = AAssetStreaming::GetTXSNameFromTRB( pInstance->m_szLowLOD, szTempString, 0x400 );
			T2ModelPtr pModel       = TModelManager::GetSingleton()->FindModel( szTempString + iNameOffset );

			if ( pModel.GetID() == T2ResourcePtr::IDINVALID )
			{
				pInstance->m_pLowLODInstance = TNULL;
			}
			else
			{
				TModelInstance* pLODInstance = pModel.GetData()->CreateInstance();
				pInstance->m_pLowLODInstance = pLODInstance;
				pLODInstance->GetTransform().SetMatrix( *pInstance->m_pMatrix );
				TASSERT( !pInstance->m_pLowLODInstance->GetTransform().GetMatrix().AsBasisVector3( 1 ).Equals( TVector3::VEC_ZERO ) );

				if ( pInstance->m_szModel == TNULL )
				{
					TASSERT( pInstance->symbol == TNULL, "Low LOD only models should not have any streamed data!" );

					// NOTE: an instance without a streamed model keeps its ACollisionInstance2 at 0x20 (symbol)
					ACollisionInstance2* pCollision = CreateInstanceCollision( pInstance, pInstance->m_pLowLODInstance );

					if ( pCollision != TNULL )
					{
						pInstance->symbol = TREINTERPRETCAST( const TCHAR*, pCollision );
						pCollision->SetMatrix( *pInstance->m_pMatrix );
					}
				}
			}
		}

		// NOTE: retail calls the creation callback of the type from a table
		switch ( pInstance->m_uiFlags & 7 )
		{
			case TerrainTypes::InstanceType_Building:
				ABuildingInstance::OnCreated( pInstance );
				break;
			case TerrainTypes::InstanceType_Landmark:
				ALandmarkInstance::OnCreated( pInstance );
				break;
			case TerrainTypes::InstanceType_Tree:
				ATreeInstance::OnCreated( pInstance );
				break;
			case TerrainTypes::InstanceType_Grass:
				AGrassInstance::OnCreated( pInstance );
				break;
			case TerrainTypes::InstanceType_TextureSwap:
				ATextureSwapInstance::OnCreated( pInstance );
				break;
			case TerrainTypes::InstanceType_ToggleVis:
				AToggleVisInstance::OnCreated( pInstance );
				break;
		}

		if ( pModelInstance != TNULL && ( pInstance->m_uiFlags & 0x20 ) )
			CreateInstanceAnim( pInstance );

		// A Z-jump instance: its node of the Z-jump set (zjump.m_iSet, zjump.m_iNode) points back to the instance
		if ( ( pInstance->m_uiFlags & 7 ) == 7 )
		{
			TASSERT( pInstance->zjump.m_iSet >= 0 && pInstance->zjump.m_iNode >= 0 );

			AZJumpData::Set*  pSet  = AZJumpData::GetSingleton()->GetSet( pInstance->zjump.m_iSet );
			AZJumpData::Node& rNode = pSet->m_pNodes[ pInstance->zjump.m_iNode ];

			TASSERT( rNode.m_eType == AZJumpData::NodeType_Instance );

			rNode.m_pInstance = pInstance;
		}

		m_uiCreateInstance++;
	}
}

// CreatePerCellInstancesVisTree
// $deBlob: FUNCTION 00623030
void ATerrain2Interface::CreateVisTrees()
{
	CreatePerCellInstancesVisTree();
}

// $deBlob: FUNCTION 00623040
void ATerrain2Interface::InitCellInstances( TerrainTypes::Cell* a_pCell )
{
	for ( TUINT i = 0; i < a_pCell->m_uiNumInstances; i++ )
	{
		TerrainTypes::CellInstance* pInstance = &a_pCell->m_pInstances[ i ];

		if ( pInstance->m_pUnk28 != TNULL && pInstance->m_pModelInstance2 != TNULL )
		{
			ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 )->GetCollisionInstance() );
			pCollision->CreateConstraint( TREINTERPRETCAST( ACollisionInstance2::PhysicsRecord*, pInstance->m_pUnk28 ), pInstance->m_pMatrix );
		}
	}

	a_pCell->m_pUnk38 = new T2GenericDList();
}

// $deBlob: FUNCTION 00623810
void ATerrain2Interface::UpdateCellsReady()
{
	ATerrain2* pTerrain  = ATerrain2::GetInstance();
	TINT       iNumCells = pTerrain->GetNumCells( m_currentRegion );

	for ( TINT c = 0; c < iNumCells; c++ )
	{
		TerrainTypes::Cell* pCell   = pTerrain->GetCell( m_currentRegion, c );
		TUINT32             uiFlags = pCell->m_uiFlags;

		if ( !( uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) )
		{
			pCell->m_uiFlags = uiFlags & ~TerrainTypes::CellRuntimeFlag_Ready;
		}
		else if ( !( uiFlags & TerrainTypes::CellRuntimeFlag_Ready ) )
		{
			if ( pCell->m_pData != TNULL && pCell->m_pData->m_pCollision != TNULL )
			{
				if ( TSTATICCAST( ACollisionInstance2, pCell->m_pData->m_pCollision )->IsBeingAdded( ACollisionInstance2::PhysicsObjectType_Standard ) )
					return;
			}

			// The cell is ready once none of its instances' collisions is still waiting in the physics manager's batch
			TBOOL bBusy = TFALSE;

			for ( TUINT i = 0; i < pCell->m_uiNumInstances && !bBusy; i++ )
			{
				TerrainTypes::CellInstance* pInstance  = &pCell->m_pInstances[ i ];
				ACollisionInstance2*        pCollision = TNULL;

				if ( pInstance->m_pModelInstance2 != TNULL )
					pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 )->GetCollisionInstance() );
				else if ( pInstance->m_szModel == TNULL && pInstance->m_szLowLOD != TNULL && pInstance->symbol != TNULL )
					pCollision = (ACollisionInstance2*)( pInstance->symbol ); // NOTE: see CreateInstances

				if ( pCollision != TNULL && pCollision->IsBeingAdded( ACollisionInstance2::PhysicsObjectType_Standard ) )
					bBusy = TTRUE;
			}

			if ( !bBusy )
				pCell->m_uiFlags |= TerrainTypes::CellRuntimeFlag_Ready;
		}
	}
}

// The low LOD model grows out of the ground between the LOD distances 2 and 3 (scale y: sin of pi/2.. 0.01)
static void UpdateLowLODScale( TerrainTypes::CellInstance* a_pInstance, TFLOAT a_fDistanceSq, const TFLOAT* a_pLODDistancesSq )
{
	TTransformObject& rTransform = a_pInstance->m_pLowLODInstance->GetTransform();

	if ( a_fDistanceSq <= a_pLODDistancesSq[ 2 ] )
	{
		if ( !( a_pInstance->m_uiFlags & 0x40000000 ) )
		{
			rTransform.SetScale( 1.0f, 1.0f, 1.0f );
			a_pInstance->m_uiFlags |= 0x40000000;
		}
	}
	else
	{
		TFLOAT fAngle = TMath::LerpTowards( TMath::HALF_PI, 0.01f, ( a_fDistanceSq - a_pLODDistancesSq[ 2 ] ) / ( a_pLODDistancesSq[ 3 ] - a_pLODDistancesSq[ 2 ] ) );

		rTransform.SetScale( 1.0f, TFLOAT( sin( fAngle ) ), 1.0f );
		a_pInstance->m_uiFlags &= ~0x40000000;
	}
}

// $deBlob: FUNCTION 00623ee0
void ATerrain2Interface::RenderInstance( void* a_pInstance, void* a_pUserData )
{
	TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, a_pInstance );
	TRenderContext*             pContext  = TRender::GetSingleton()->GetCurrentContext();

	TVector4 vCameraPos = pContext->GetCameraObject()->GetTransformObject().GetTranslation();
	vCameraPos.w        = 1.0f;

	TerrainTypes::Cell* pCell       = pInstance->m_pCell;
	ATerrain2Interface* pInterface  = sm_Instance;
	const TFLOAT*       pfLODDistSq = ms_afInstanceLODDistancesSq;

	if ( !( pInstance->m_uiFlags & 0x800000 ) ) // TerrainTypes::InstanceFlags_Visible
		return;

	if ( !ms_bCullInstances && ( pInstance->m_uiFlags & 0x20000000 ) )
		return;

	ATerrain2* pTerrain       = ATerrain2::GetInstance();
	TFLOAT     fOldShadeCoeff = pContext->GetShadeCoeff();
	TFLOAT     fDistanceSq    = 0.0f;
	TBOOL      bRenderLowLOD  = TFALSE;

	if ( !( pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Loaded ) || ( pCell->m_uiFlags & ( TerrainTypes::CellRuntimeFlag_Unloading | TerrainTypes::CellRuntimeFlag_QueuedUnload ) ) )
	{
		// The model of the cell isn't loaded: only the low LOD
		if ( pInstance->m_pLowLODInstance != TNULL )
		{
			fDistanceSq = TVector4::DistanceSq( **pInstance->m_ppCentre, vCameraPos );

			if ( pInstance->m_pModelInstance2 != TNULL )
			{
				if ( fDistanceSq < pfLODDistSq[ 3 ] || ( pInstance->m_uiFlags & 8 ) )
				{
					if ( !( pInstance->m_uiFlags & 8 ) )
					{
						UpdateLowLODScale( pInstance, fDistanceSq, pfLODDistSq );

						pInstance->m_pLowLODInstance->SetForceFade( TTRUE );
						pInstance->m_pLowLODInstance->SetFade( 0.99f );

						TINT iInstanceIndex = TINT( pInstance - pTerrain->GetInstances() );
						TASSERT( iInstanceIndex < pTerrain->GetNumInstances() && iInstanceIndex >= 0 );

						TFLOAT* pFades                   = TREINTERPRETCAST( TFLOAT*, pInterface->m_apInstanceFades[ ARenderer::GetCurrentCameraIndex() ] );
						pFades[ iInstanceIndex * 2 + 1 ] = 2.0f;
						pContext->SetShadeCoeff( ms_fInstanceShadeCoeffLow );
					}

					bRenderLowLOD = TTRUE;
				}
			}
			else if ( fDistanceSq < pfLODDistSq[ 1 ] || ( pInstance->m_uiFlags & 8 ) )
			{
				pInstance->m_pLowLODInstance->SetForceFade( TFALSE );
				bRenderLowLOD = TTRUE;
			}
		}
	}
	else
	{
		fDistanceSq = TVector4::DistanceSq( **pInstance->m_ppCentre, vCameraPos );

		if ( pInstance->m_pModelInstance2 != TNULL )
		{
			if ( pInstance->m_pLowLODInstance != TNULL && !( pInstance->m_uiFlags & 8 ) )
			{
				// Cross fade between the model and the low LOD
				TBOOL  bFading     = !( pInstance->m_uiFlags & 0x80000 );
				TFLOAT fFadeRef    = -2.0f;
				TBOOL  bInFade     = TFALSE;
				TBOOL  bSkipUpdate = TFALSE;

				if ( bFading )
				{
					TINT iInstanceIndex = TINT( pInstance - pTerrain->GetInstances() );
					TASSERT( iInstanceIndex < pTerrain->GetNumInstances() && iInstanceIndex >= 0 );

					TFLOAT* pFades = TREINTERPRETCAST( TFLOAT*, pInterface->m_apInstanceFades[ ARenderer::GetCurrentCameraIndex() ] );
					fFadeRef       = pFades[ iInstanceIndex * 2 + 1 ];

					TFLOAT fExtra = -19600.0f;

					if ( TINT( pInstance->m_uiFlags ) >= 0 )
					{
						fExtra = 0.0f;

						if ( pInstance->m_uiFlags & 0x200000 )
							fExtra = 52900.0f;
					}

					TBOOL bStartFade = TFALSE;

					if ( fFadeRef == -2.0f && fDistanceSq > 400.0f + pfLODDistSq[ 0 ] + fExtra )
					{
						if ( pfLODDistSq[ 2 ] <= fDistanceSq )
						{
							fFadeRef    = 2.0f;
							bSkipUpdate = TTRUE;
						}
						else
						{
							fFadeRef                         = 1.0f;
							pFades[ iInstanceIndex * 2 + 1 ] = 1.0f;
							bStartFade                       = TTRUE;
						}
					}
					else if ( fFadeRef == 2.0f && fDistanceSq < 400.0f * 0.5f + pfLODDistSq[ 0 ] + fExtra )
					{
						fFadeRef                         = -1.0f;
						pFades[ iInstanceIndex * 2 + 1 ] = -1.0f;
						bStartFade                       = TTRUE;
					}

					if ( bStartFade )
						pFades[ iInstanceIndex * 2 ] = pInterface->m_fTime;

					if ( !bSkipUpdate && fFadeRef != -2.0f && fFadeRef != 2.0f )
					{
						bInFade = TTRUE;

						TFLOAT fTimeDelta = pInterface->m_fTime - pFades[ iInstanceIndex * 2 ];
						TASSERT( fTimeDelta >= 0.f );
						pFades[ iInstanceIndex * 2 ] = pInterface->m_fTime;

						TFLOAT fMin;
						TFLOAT fMax;

						if ( 0.0f <= fFadeRef )
						{
							TASSERT( fFadeRef > 0.f );
							TFLOAT fFade = TMath::LerpTowards( 0.0f, 0.99f, fFadeRef - 1.0f );
							pInstance->m_pLowLODInstance->SetForceFade( TTRUE );
							pInstance->m_pLowLODInstance->SetFade( fFade );
							fMax     = 2.0f;
							fMin     = 1.0f;
							fFadeRef = 2.0f * fTimeDelta + fFadeRef;
						}
						else
						{
							TFLOAT fFade = TMath::LerpTowards( 0.99f, 0.0f, TMath::Abs( ( fFadeRef - -1.0f ) * -1.0f ) );
							pInstance->m_pLowLODInstance->SetForceFade( TTRUE );
							pInstance->m_pLowLODInstance->SetFade( fFade );
							fMax     = -1.0f;
							fFadeRef = fFadeRef - 2.0f * fTimeDelta;
							fMin     = -2.0f;
						}

						TMath::Clip( fFadeRef, fMin, fMax );
						pFades[ iInstanceIndex * 2 + 1 ] = fFadeRef;
					}
				}

				if ( fDistanceSq > pfLODDistSq[ 2 ] )
				{
					pInstance->m_pLowLODInstance->SetForceFade( TTRUE );
					pInstance->m_pLowLODInstance->SetFade( 0.99f );
				}

				UpdateLowLODScale( pInstance, fDistanceSq, pfLODDistSq );

				TBOOL bRenderModel = TFALSE;
				TBOOL bFadeModel   = TFALSE;

				if ( !bFading )
				{
					if ( fDistanceSq < pfLODDistSq[ 0 ] )
					{
						bRenderModel = TTRUE;
					}
					else if ( fDistanceSq < pfLODDistSq[ 3 ] )
					{
						pInstance->m_pLowLODInstance->SetForceFade( TTRUE );
						pInstance->m_pLowLODInstance->SetFade( 0.99f );
						bRenderLowLOD = TTRUE;
					}
				}
				else if ( fFadeRef == -2.0f )
				{
					bRenderModel = TTRUE;
				}
				else if ( fFadeRef == 2.0f && fDistanceSq < pfLODDistSq[ 3 ] )
				{
					pInstance->m_pLowLODInstance->SetForceFade( TTRUE );
					pInstance->m_pLowLODInstance->SetFade( 0.99f );
					bRenderLowLOD = TTRUE;
				}
				else if ( bInFade )
				{
					bFadeModel    = TTRUE;
					bRenderLowLOD = TTRUE;
				}

				if ( bRenderModel || bFadeModel )
				{
					pContext->SetShadeCoeff( bRenderModel ? ms_fInstanceShadeCoeffHigh : ms_fInstanceShadeCoeffFade );
					RenderLightmappedInstance::RenderInstance( pCell, pInstance, ms_bCullInstances, 0, fDistanceSq, a_pUserData );
				}
			}
			else if ( fDistanceSq < pfLODDistSq[ 1 ] || ( pInstance->m_uiFlags & 8 ) )
			{
				pContext->SetShadeCoeff( ms_fInstanceShadeCoeffNear );
				RenderLightmappedInstance::RenderInstance( pCell, pInstance, ms_bCullInstances, 0, fDistanceSq, a_pUserData );
			}
		}
		else if ( fDistanceSq < pfLODDistSq[ 1 ] || ( pInstance->m_uiFlags & 8 ) )
		{
			pInstance->m_pLowLODInstance->SetForceFade( TFALSE );
			bRenderLowLOD = TTRUE;
		}
	}

	if ( bRenderLowLOD )
	{
		RenderLightmappedInstance::RenderLowLODInstance( pCell, pInstance, ms_bCullInstances, fDistanceSq );
	}

	pContext->SetShadeCoeff( fOldShadeCoeff );
}

// $deBlob: FUNCTION 00624a70
void ATerrain2Interface::CreatePerCellInstancesVisTree()
{
	TASSERT( m_pPerCellInstancesVisTree == TNULL );

	ATerrain2* pTerrain = ATerrain2::GetInstance();
	TINT       iCount   = pTerrain->GetNumCells( m_currentRegion );

	// NOTE: retail allocates the array with TMemalign16 (the count before it, like new[])
	m_pPerCellInstancesVisTree = new TBoxTree[ iCount ];

	for ( TINT i = 0; i < iCount; i++ )
	{
		TerrainTypes::Cell* pCell = pTerrain->GetCell( m_currentRegion, i );

		if ( pCell->m_pUnk34 != TNULL )
			m_pPerCellInstancesVisTree[ i ].SetRoot( TSTATICCAST( TBoxTree::Node, pCell->m_pUnk34 ) );

		m_pPerCellInstancesVisTree[ i ].SetRenderFunction( RenderInstance );
	}
}

// $deBlob: FUNCTION 00624800
ACellBucket* ATerrain2Interface::FindBucketToReuse( TBOOL a_bPriority )
{
	ATerrain2*                  pTerrain     = ATerrain2::GetInstance();
	const T2DList<ACellBucket>& rActiveCells = pTerrain->GetActiveCells();
	ATerrain2Interface*         pInterface   = sm_Instance;

	if ( rActiveCells.IsEmpty() )
		return TNULL;

	TINT* pNeighbours = pTerrain->GetCellVis( pInterface->m_currentRegion, pInterface->m_iCurrentCell );

	// Cancels the load of a cell that isn't seen
	for ( auto it = rActiveCells.Begin(); it != rActiveCells.End(); it++ )
	{
		TerrainTypes::Cell* cell    = it->GetCell();
		TUINT32             uiFlags = cell->m_uiFlags;

		if ( ( uiFlags & ( TerrainTypes::CellRuntimeFlag_CancelLoad | TerrainTypes::CellRuntimeFlag_QueuedUnload ) ) == 0 &&
		     ( uiFlags & TerrainTypes::CellRuntimeFlag_Loading ) && it->GetCellIndex() != pInterface->m_iCurrentCell )
		{
			TBOOL bNeighbour = TFALSE;

			if ( pNeighbours )
			{
				for ( TUINT i = 0; i < 8; i++ )
				{
					if ( it->GetCellIndex() == pNeighbours[ i ] )
					{
						bNeighbour = TTRUE;
						break;
					}
				}
			}

			if ( !bNeighbour && !( uiFlags & TerrainTypes::CellRuntimeFlag_Visible ) )
			{
				TASSERT( ( cell->m_uiFlags & TerrainTypes::CellRuntimeFlag_CancelLoad ) == 0 );
				TASSERT( ( cell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Loading ) != 0 );
				cell->m_uiFlags |= TerrainTypes::CellRuntimeFlag_CancelLoad;
				return TNULL;
			}
		}
	}

	const TUINT32 uiBusyFlags = TerrainTypes::CellRuntimeFlag_Loading | TerrainTypes::CellRuntimeFlag_CancelLoad | TerrainTypes::CellRuntimeFlag_QueuedUnload;

	// The least recently loaded cell that isn't the current one, next to it or seen
	for ( T2DList<ACellBucket>::Iterator it = rActiveCells.Back();; it = it.Prev() )
	{
		TBOOL bSkip = ( it->GetCell()->m_uiFlags & uiBusyFlags ) != 0 || it->GetCellIndex() == pInterface->m_iCurrentCell;

		if ( !bSkip && pNeighbours )
		{
			for ( TUINT i = 0; i < 8; i++ )
			{
				if ( it->GetCellIndex() == pNeighbours[ i ] )
				{
					bSkip = TTRUE;
					break;
				}
			}
		}

		if ( !bSkip )
		{
			if ( !( it->GetCell()->m_uiFlags & TerrainTypes::CellRuntimeFlag_Visible ) )
				return it;

			// Seen: only if it isn't next to a neighbour
			// NOTE: retail doesn't check pNeighbours here
			TBOOL bFar = TTRUE;

			for ( TINT i = 0; i < 8 && bFar; i++ )
			{
				TINT iNeighbour = pNeighbours[ i ];

				if ( iNeighbour != TerrainTypes::CellVis::NONE )
				{
					TINT* pFarNeighbours = pTerrain->GetCellVis( pInterface->m_currentRegion, iNeighbour );

					for ( TINT j = 0; j < 8; j++ )
					{
						if ( pFarNeighbours[ j ] == it->GetCellIndex() )
						{
							bFar = TFALSE;
							break;
						}
					}
				}
			}

			if ( bFar )
				return it;
		}

		if ( it == rActiveCells.Begin() )
			break;
	}

	if ( a_bPriority )
	{
		// Any cell that isn't busy, the current one or next to it
		for ( T2DList<ACellBucket>::Iterator it = rActiveCells.Back();; it = it.Prev() )
		{
			if ( ( it->GetCell()->m_uiFlags & uiBusyFlags ) == 0 && it->GetCellIndex() != pInterface->m_iCurrentCell )
			{
				if ( !pNeighbours )
					return it;

				TUINT i = 0;

				while ( it->GetCellIndex() != pNeighbours[ i ] )
				{
					if ( ++i > 7 )
						return it;
				}
			}

			if ( it == rActiveCells.Begin() )
				break;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 00624c00
void ATerrain2Interface::OnCellLoadedCallback( ACellBucket* a_pBucket )
{
	sm_Instance->OnCellLoaded( a_pBucket );
}

// $deBlob: FUNCTION 00624c10
void ATerrain2Interface::OnCellUnloadedCallback( ACellBucket* a_pBucket )
{
	sm_Instance->OnCellUnloaded( a_pBucket );
}

// $deBlob: FUNCTION 00622b50
void ATerrain2Interface::AddInstanceToWorld( ACellBucket* a_pBucket, TerrainTypes::CellInstance* a_pInstance )
{
	AddInstanceCollision( a_pBucket->GetCell(), a_pInstance, m_bSinglePlayer ? 0x20 : 0x60 );

	if ( ( a_pInstance->m_uiFlags & 0x40 ) && a_pInstance->symbol != TNULL )
	{
		a_pInstance->m_pSymbolData = TSTATICCAST( TerrainTypes::InstanceMeshes, a_pBucket->GetTRB()->GetSymbolAddress( a_pInstance->symbol ) );
		a_pBucket->CreateInstanceMaterials( a_pInstance );
	}

	a_pInstance->m_uiFlags |= 0x1000000;

	// NOTE: retail calls the callback of the type from a table
	switch ( a_pInstance->m_uiFlags & 7 )
	{
		case TerrainTypes::InstanceType_Landmark:
			ALandmarkInstance::OnAddedToWorld( a_pInstance );
			break;
		case TerrainTypes::InstanceType_ToggleVis:
			AToggleVisInstance::OnAddedToWorld( a_pInstance );
			break;
	}
}

// $deBlob: FUNCTION 00623ca0
void ATerrain2Interface::AddInstanceCollision( TerrainTypes::Cell* a_pCell, TerrainTypes::CellInstance* a_pInstance, TUINT32 a_uiMask )
{
	if ( ( a_pInstance->m_uiFlags & 0x800 ) && ( a_pCell->m_uiFlags & TerrainTypes::CellFlag_Collision ) )
	{
		AModelInstance2* pModelInstance = TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 );

		if ( pModelInstance != TNULL && pModelInstance->GetCollisionInstance() != TNULL )
		{
			if ( ( a_pInstance->m_uiFlags >> 23 ) & 1 )
				TSTATICCAST( ACollisionInstance2, pModelInstance->GetCollisionInstance() )->AddToWorld( a_uiMask );
		}
		else if ( a_pInstance->m_szModel == TNULL && a_pInstance->m_szLowLOD != TNULL && a_pInstance->symbol != TNULL )
		{
			( (ACollisionInstance2*)a_pInstance->symbol )->AddToWorld( a_uiMask );
		}
	}
}

// $deBlob: FUNCTION 00623d10
void ATerrain2Interface::RemoveInstanceCollision( TerrainTypes::Cell* a_pCell, TerrainTypes::CellInstance* a_pInstance, TUINT32 a_uiMask )
{
	if ( ( a_pInstance->m_uiFlags & 0x800 ) && ( a_pCell->m_uiFlags & TerrainTypes::CellFlag_Collision ) )
	{
		AModelInstance2* pModelInstance = TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 );

		if ( pModelInstance != TNULL && pModelInstance->GetCollisionInstance() != TNULL )
			TSTATICCAST( ACollisionInstance2, pModelInstance->GetCollisionInstance() )->RemoveFromWorld( a_uiMask );
		else if ( a_pInstance->m_szModel == TNULL && a_pInstance->m_szLowLOD != TNULL && a_pInstance->symbol != TNULL )
			( (ACollisionInstance2*)a_pInstance->symbol )->RemoveFromWorld( a_uiMask );
	}
}

#include "ToshiPCH.h"
#include "TPhysicsManager.h"
#include "TMemoryListener.h"
#include "TPhysicsCollisionFilter.h"
#include "TPhysicsBrokenConstraintListener.h"
#include "TPhysicsTriggerShape.h"
#include "TPhysicsConstraintContainer.h"
#include "Physics/TPhysicsResponder.h"
#include "Physics/TPhysicsMotionBase.h"
#include "Physics/TPhysicsPluginBase.h"
#include "Physics/TBoxData.h"
#include "Physics/TPlaneData.h"
#include "Physics/TSphereData.h"
#include "Physics/TCapsuleData.h"
#include "Physics/TCylinderData.h"
#include "Physics/TCSGNode.h"
#include "TPhysicsMeshContainer.h"
#include "TPhysicsStandardMeshShape.h"
#include "TPhysicsCustomMeshShape.h"
#include "TPhysicsHFContainer.h"
#include "TPhysicsHeightFieldShape.h"
#include "TPhysicsManagerCollisionListener.h"

#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Memory/Memory/FreeList/hkFreeListMemory.h>
#include <Common/Base/Memory/Memory/FreeList/SystemMemoryBlockServer/hkSystemMemoryBlockServer.h>
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Collide/Dispatch/hkpAgentRegisterUtil.h>
#include <Physics/Collide/Dispatch/hkpCollisionDispatcher.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Collide/Shape/Convex/Capsule/hkpCapsuleShape.h>
#include <Physics/Collide/Shape/Convex/Cylinder/hkpCylinderShape.h>
#include <Physics/Collide/Shape/HeightField/Plane/hkpPlaneShape.h>
#include <Physics/Collide/Shape/Compound/Collection/List/hkpListShape.h>
#include <Physics/Collide/Shape/Misc/Transform/hkpTransformShape.h>
#include <Physics/Collide/Shape/Misc/Bv/hkpBvShape.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppBvTreeShape.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppUtility.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppCompilerInput.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Dynamics/Constraint/hkpConstraintInstance.h>
#include <Physics/Dynamics/Constraint/Bilateral/BallAndSocket/hkpBallAndSocketConstraintData.h>
#include <Physics/Dynamics/Constraint/Bilateral/StiffSpring/hkpStiffSpringConstraintData.h>
#include <Physics/Dynamics/Constraint/Bilateral/Hinge/hkpHingeConstraintData.h>
#include <Physics/Dynamics/Constraint/Bilateral/LimitedHinge/hkpLimitedHingeConstraintData.h>
#include <Physics/Dynamics/Constraint/Bilateral/Prismatic/hkpPrismaticConstraintData.h>
#include <Physics/Dynamics/Constraint/Bilateral/Wheel/hkpWheelConstraintData.h>
#include <Physics/Dynamics/Constraint/Breakable/hkpBreakableConstraintData.h>
#include <Physics/Dynamics/Constraint/Malleable/hkpMalleableConstraintData.h>
#include <Physics/ConstraintSolver/Solve/hkpSolverResults.h>
#include <Physics/Collide/Query/Collector/BodyPairCollector/hkpFirstCdBodyPairCollector.h>
#include <Physics/Collide/Query/Collector/BodyPairCollector/hkpAllCdBodyPairCollector.h>
#include <Physics/Collide/Query/Collector/PointCollector/hkpAllCdPointCollector.h>
#include <Physics/Collide/Query/Collector/PointCollector/hkpClosestCdPointCollector.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>
#include <Physics/Collide/Query/Collector/RayCollector/hkpAllRayHitCollector.h>
#include <Physics/Collide/Shape/Convex/Triangle/hkpTriangleShape.h>
#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>
#include <Physics/Dynamics/Phantom/hkpSimpleShapePhantom.h>
#include <Physics/Dynamics/Phantom/hkpCachingShapePhantom.h>
#include <Physics/Collide/Query/CastUtil/hkpLinearCastInput.h>
#include <Physics/Collide/Agent/hkpProcessCollisionInput.h>
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>
#include <Physics/Utilities/Collide/ContactModifiers/SurfaceVelocity/hkpSurfaceVelocityUtil.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>
#include <Common/Internal/ConvexHull/hkGeometryUtility.h>
#include <Common/Base/Types/Geometry/hkGeometry.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>
#include <Common/Base/Thread/Job/ThreadPool/Cpu/hkCpuJobThreadPool.h>

// The Havok keycode: the one shipped with the SDK (Common/Base/KeyCode.h)
#include <Common/Base/keycode.cxx>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TBOOL TPhysicsManager::ms_bCachingPhantoms = TFALSE;
TBOOL TPhysicsManager::ms_bRayPhantoms     = TTRUE;
TBOOL TPhysicsManager::ms_bShapePhantoms   = TTRUE;

TMemoryHeap* TPhysicsManager::ms_pHeap = TNULL;

// The hkBaseSystem error report function
// $deBlob: FUNCTION 0072b430
static void HK_CALL ErrorReport( const char* a_szMessage, void* a_pObject )
{
	// NOTE: retail compares the message with these and does nothing else (its output is compiled out)
	(void)strncmp( a_szMessage, "Err", 3 );
	(void)strncmp( a_szMessage, ".\\shape\\mopp\\hkpMoppUtility.cpp(18): [0x6e8d163b] Warning : 'Building Mopp code at runtime", 0x5a );
}

// $deBlob: FUNCTION 0072b400
void* __cdecl TPhysicsManager::Alloc( int a_iSize, int a_iAlign )
{
	return TMemalign( a_iAlign, a_iSize, GetHeap() );
}

// $deBlob: FUNCTION 0072b420
void __cdecl TPhysicsManager::Free( void* a_pMem )
{
	TFree( a_pMem );
}

TMemoryHeap* TPhysicsManager::GetHeap()
{
	return ms_pHeap ? ms_pHeap : TMemory_GetGlobalHeap();
}

// $deBlob: FUNCTION 007203c0
TPhysicsManager::TPhysicsManager()
{
	m_iNumStepListeners = 0;
	m_initialised       = TFALSE;

	m_initData.m_fTimeStep      = 1.0f / 60.0f;
	m_initData.m_vBroadPhaseMin = TVector4( -500.0f, -500.0f, -500.0f, 1.0f );
	m_initData.m_vBroadPhaseMax = TVector4( 500.0f, 500.0f, 500.0f, 1.0f );
	m_initData.m_vGravity.Multiply( TVector4::VEC_POSY, 9.8f );

	m_fTimeStep = m_initData.m_fTimeStep;

#ifdef DEBLOB_COMMUNITY_PATCH
	m_uiTotalSteps = 0;
#endif

	m_initData.m_iUnk34                        = 3000;
	m_initData.m_iMaxCustomMovers              = 2000;
	m_initData.m_iMaxConstraints               = 200;
	m_initData.m_iMaxCollisionCallbacks        = 800;
	m_initData.m_iMaxTriggerCallbacks          = 100;
	m_initData.m_iMaxConstraintBrokenCallbacks = 100;
}

// $deBlob: FUNCTION 00720510
// $deBlob: FUNCTION 0056c310
TPhysicsManager::~TPhysicsManager()
{
}


// $deBlob: FUNCTION 007205d0
TPhysicsManager* TPhysicsManager::Create()
{
	return new ( TMalloc( sizeof( TPhysicsManager ), GetHeap() ) ) TPhysicsManager();
}

hkpWorld* TPhysicsManager::GetWorld( TINT a_world ) const
{
	TASSERT( a_world >= 0 );
	TASSERT( a_world < PHYSICS_WORLDS );
	return m_apWorlds[ a_world ];
}

// $deBlob: FUNCTION 00720620
void TPhysicsManager::Initialise( const TPhysicsManagerInitData* a_pInitData )
{
	TASSERT( !m_initialised );

	if ( a_pInitData )
		m_initData = *a_pInitData;

	m_MemoryAllocator.SetHeap( GetHeap() );

	for ( TINT i = 0; i < m_iNumStepListeners; i++ )
		m_apStepListeners[ i ] = TNULL;

	m_iNumStepListeners         = 0;
	m_initialised               = TTRUE;
	m_fAliveTime                = 0.0f;
	m_fTimeStep                 = m_initData.m_fTimeStep;
	m_bInSimulation             = TFALSE;
	m_bBatchOperationInProgress = TFALSE;
	m_bBatchCommitDisabled      = TFALSE;
	m_bSavedBatchOperation      = TFALSE;
	m_iBatchCommitLimit         = 5;
	m_iNumSteps                 = 0;
	m_pUnk60                    = TNULL;
	m_pMemoryBlockServer        = TNULL;
	m_pMemoryListener           = TNULL;

	for ( TINT i = 0; i < TARRAYSIZE( m_auiBatchStats ); i++ )
		m_auiBatchStats[ i ] = 0;

	// Havok's system allocations go to the physics heap
	hkSystemMalloc = Alloc;
	hkSystemFree   = Free;

	m_pMemoryBlockServer = new hkSystemMemoryBlockServer( 0x40000 );
	m_pFreeListMemory    = new hkFreeListMemory( m_pMemoryBlockServer );

	// NOTE: Havok 5.5's hkThreadMemory ctor also took the max number of elements on its free lists (retail: 16);
	// 6.5's has no such parameter
	m_pThreadMemory = new hkThreadMemory( m_pFreeListMemory );

	hkBaseSystem::init( m_pFreeListMemory, m_pThreadMemory, ErrorReport, HK_NULL );
	m_pFreeListMemory->removeReference();

	m_pMemoryListener = new TMemoryListener();
	m_pFreeListMemory->setLimitedMemoryListener( m_pMemoryListener );

	m_pStackBuffer = TNULL;
	m_pStackBuffer = hkAllocate<char>( 0x60000, HK_MEMORY_CLASS_BASE );
	m_pThreadMemory->setStackArea( m_pStackBuffer, 0x60000 );

	m_iDefaultWorld = 0;

	for ( TINT i = 0; i < PHYSICS_WORLDS; i++ )
	{
		hkpWorldCinfo oInfo;
		oInfo.m_gravity.set( m_initData.m_vGravity.x, m_initData.m_vGravity.y, m_initData.m_vGravity.z, 0.0f );
		oInfo.m_expectedMinPsiDeltaTime = m_initData.m_fTimeStep;
		oInfo.m_broadPhaseWorldAabb.m_min.set( m_initData.m_vBroadPhaseMin.x, m_initData.m_vBroadPhaseMin.y, m_initData.m_vBroadPhaseMin.z, 0.0f );
		oInfo.m_broadPhaseWorldAabb.m_max.set( m_initData.m_vBroadPhaseMax.x, m_initData.m_vBroadPhaseMax.y, m_initData.m_vBroadPhaseMax.z, 0.0f );
		oInfo.m_simulationType               = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
		oInfo.m_processActionsInSingleThread = true;
		oInfo.m_expectedMaxLinearVelocity    = 330.0f;
		oInfo.setupSolverInfo( hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM );

		// NOTE: retail passes its SDK version (50500, Havok 5.5.0) to hkpWorld's ctor; we pass 6.5's (the default)
		m_apWorlds[ i ] = new hkpWorld( oInfo );

		// NOTE: Havok 5.5 returned a multithreaded world marked for write by the creating thread; 6.5's is
		// unmarked, so it's marked here
		m_apWorlds[ i ]->markForWrite();

		hkpAgentRegisterUtil::registerAllAgents( m_apWorlds[ i ]->getCollisionDispatcher() );

		// NOTE: retail sets the dispatcher's byte at +0x1e22 (5.5 layout) after registering the agents; taken as
		// m_checkEnabled (the flag after the registration flags)
		m_apWorlds[ i ]->getCollisionDispatcher()->m_checkEnabled = true;

		TPhysicsManagerCollisionFilter* pFilter = new TPhysicsManagerCollisionFilter();
		m_apWorlds[ i ]->setCollisionFilter( pFilter, true, HK_UPDATE_FILTER_ON_WORLD_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
		pFilter->removeReference();

		m_iLastSubSystemId = 0;

		// NOTE: retail creates a Havok 5.5 hkpMultithreadingUtil for the world (hkpMultithreadingUtilCinfo: the
		// world and 1 thread), which runs the step on its own worker thread between StartStepWorld and
		// FinishStepWorld. Havok 6.5 has no hkpMultithreadingUtil: the same is done with a job queue and a
		// one-thread hkCpuJobThreadPool per world (hkpWorld::initMtStep/finishMtStep)
		m_apMultithreadingUtils[ i ] = TNULL;
		{
			hkJobQueueCinfo oQueueInfo;
			m_apJobQueues[ i ] = new hkJobQueue( oQueueInfo );
			m_apWorlds[ i ]->registerWithJobQueue( m_apJobQueues[ i ] );

			hkCpuJobThreadPoolCinfo oPoolInfo;
			oPoolInfo.m_numThreads                     = 1;
			oPoolInfo.m_timerBufferPerThreadAllocation = 0;
			m_apThreadPools[ i ]                       = new hkCpuJobThreadPool( oPoolInfo );
		}

		m_abStepping[ i ] = TFALSE;
	}

	m_pConstraints              = new ( TMalloc( sizeof( T2DynamicVector<TPhysicsConstraintContainer*> ), GetHeap() ) ) T2DynamicVector<TPhysicsConstraintContainer*>( &m_MemoryAllocator, m_initData.m_iMaxConstraints, 0 );
	m_pBrokenConstraintListener = new TPhysicsBrokenConstraintListener( this );

	m_pCollisionCallbacks           = TSTATICCAST( TPhysicsCollisionCallback, TMalloc( m_initData.m_iMaxCollisionCallbacks * sizeof( TPhysicsCollisionCallback ), GetHeap() ) );
	m_iNumCollisionCallbacks        = 0;
	m_iMaxNumCollisionCallbacksUsed = 0;

	m_iNumContactData       = m_initData.m_iMaxCollisionCallbacks / 2;
	m_pContactData          = TSTATICCAST( TPhysicsContactData, TMalloc( m_iNumContactData * sizeof( TPhysicsContactData ), GetHeap() ) );
	m_pFreeContactData      = m_pContactData;
	m_iNumContactDataAllocs = 0;
	m_iNumContactDataFrees  = 0;

	for ( TINT i = 0; i < m_iNumContactData; i++ )
	{
		TPhysicsContactData* pEntry = &m_pContactData[ i ];
		pEntry->m_uiMaterialA       = 0;
		pEntry->m_uiMaterialB       = 0;
		pEntry->m_pBodyA            = TNULL;
		pEntry->m_pBodyB            = TNULL;
		pEntry->m_bUsed             = TFALSE;
		pEntry->m_bToi              = TFALSE;
	}

	m_pTriggerCallbacks           = TSTATICCAST( TPhysicsTriggerCallback, TMalloc( m_initData.m_iMaxTriggerCallbacks * sizeof( TPhysicsTriggerCallback ), GetHeap() ) );
	m_iNumTriggerCallbacks        = 0;
	m_iMaxNumTriggerCallbacksUsed = 0;

	m_pCustomMovers    = new ( TMalloc( sizeof( T2DynamicVector<TPhysicsMotionBase*> ), GetHeap() ) ) T2DynamicVector<TPhysicsMotionBase*>( &m_MemoryAllocator, m_initData.m_iMaxCustomMovers, 0 );
	m_iNumCustomMovers = 0;

	m_pConstraintBrokenCallbacks           = TMalloc( m_initData.m_iMaxConstraintBrokenCallbacks * 8, GetHeap() );
	m_iNumConstraintBrokenCallbacks        = 0;
	m_iMaxNumConstraintBrokenCallbacksUsed = 0;
}

// $deBlob: FUNCTION 00720d90
void TPhysicsManager::Unitialise()
{
	TASSERT( m_initialised );

	while ( m_pConstraints->Size() != 0 )
		DestroyConstraint( ( *m_pConstraints )[ 0 ] );

	m_pConstraints->Clear();
	m_pConstraints->FreeMemory();
	TFree( m_pConstraints );
	m_pConstraints = TNULL;

	if ( m_pBrokenConstraintListener )
		delete m_pBrokenConstraintListener;

	m_pBrokenConstraintListener = TNULL;

	TASSERT( m_addedBodies.IsEmpty() );
	TASSERT( m_removedBodies.IsEmpty() );
	TASSERT( m_bBatchOperationInProgress == TFALSE );

	for ( auto it = m_activeBodiesStatic.Begin(); it != m_activeBodiesStatic.End(); )
	{
		TPhysicsObjectContainer* pBody = it;
		it++;
		DestroyBody( pBody );
	}

	TASSERT( m_activeBodiesStatic.IsEmpty() );

	for ( auto it = m_activeBodiesKeyframe.Begin(); it != m_activeBodiesKeyframe.End(); )
	{
		TPhysicsObjectContainer* pBody = it;
		it++;
		DestroyBody( pBody );
	}

	TASSERT( m_activeBodiesKeyframe.IsEmpty() );

	for ( auto it = m_activeBodiesDynamic.Begin(); it != m_activeBodiesDynamic.End(); )
	{
		TPhysicsObjectContainer* pBody = it;
		it++;
		DestroyBody( pBody );
	}

	TASSERT( m_activeBodiesDynamic.IsEmpty() );

	DestroyPendingBodies();
	TASSERT( m_destroyBodies.IsEmpty() );

	for ( auto it = m_bodies.Begin(); it != m_bodies.End(); )
	{
		TPhysicsObjectContainer* pBody = it;
		it++;
		DestroyBody( pBody );
	}

	TASSERT( m_bodies.IsEmpty() );
	TASSERT( m_addedBodies.IsEmpty() );
	TASSERT( m_removedBodies.IsEmpty() );

	while ( m_pCustomMovers->Size() != 0 )
	{
		TPhysicsMotionBase* pMover = ( *m_pCustomMovers )[ 0 ];
		RemoveCustomMover( pMover );
		pMover->OnRemoved();
	}
	m_pCustomMovers->Clear();
	m_pCustomMovers->FreeMemory();
	TFree( m_pCustomMovers );
	m_pCustomMovers    = TNULL;
	m_iNumCustomMovers = 0;

	TFree( m_pCollisionCallbacks );
	m_pCollisionCallbacks           = TNULL;
	m_iNumCollisionCallbacks        = 0;
	m_iMaxNumCollisionCallbacksUsed = 0;

	TFree( m_pTriggerCallbacks );
	m_pTriggerCallbacks           = TNULL;
	m_iNumTriggerCallbacks        = 0;
	m_iMaxNumTriggerCallbacksUsed = 0;

	TFree( m_pConstraintBrokenCallbacks );
	m_pConstraintBrokenCallbacks           = TNULL;
	m_iNumConstraintBrokenCallbacks        = 0;
	m_iMaxNumConstraintBrokenCallbacksUsed = 0;

	TFree( m_pContactData );
	m_pContactData = TNULL;

	for ( TINT i = 0; i < m_iNumStepListeners; i++ )
		m_apStepListeners[ i ] = TNULL;

	m_iNumStepListeners = 0;

	for ( TINT i = 0; i < PHYSICS_WORLDS; i++ )
	{
		// NOTE: retail deletes the world's hkpMultithreadingUtil (Havok 5.5) here; with 6.5: the thread pool and the
		// job queue that replace it
		if ( m_apThreadPools[ i ] )
			delete m_apThreadPools[ i ];

		if ( m_apJobQueues[ i ] )
			delete m_apJobQueues[ i ];

		m_apThreadPools[ i ] = TNULL;
		m_apJobQueues[ i ]   = TNULL;

		// NOTE: retail calls the world's deleting destructor directly (not removeReference)
		if ( m_apWorlds[ i ] )
			delete m_apWorlds[ i ];
	}

	m_pThreadMemory->setStackArea( TNULL, 0 );
	hkDeallocate( (char*)m_pStackBuffer );
	m_pThreadMemory->removeReference();

	// The hkDefaultFreeListMemoryServer singleton gets the memory size 1 before hkBaseSystem::quit
	// NOTE: retail writes the 16-bit m_memSizeAndFlags at +4 (5.5 layout, the same in 6.5)
	if ( hkSingleton<hkDefaultFreeListMemoryServer>::s_instance != HK_NULL )
		hkSingleton<hkDefaultFreeListMemoryServer>::s_instance->m_memSizeAndFlags = 1;

	hkBaseSystem::quit();

	if ( m_pMemoryBlockServer )
		delete m_pMemoryBlockServer;

	if ( m_pMemoryListener )
		delete m_pMemoryListener;

	TFree( m_pUnk60 );
	m_initialised = TFALSE;
}

// $deBlob: FUNCTION 00721560
void TPhysicsManager::Update( TFLOAT a_fDeltaTime )
{
	TASSERT( m_initialised );

	if ( !( 1e-05f > a_fDeltaTime ) )
	{
		m_bInSimulation = TTRUE;
		m_iNumSteps     = 0;
		m_fAliveTime += a_fDeltaTime;
		m_fTimeStep = m_fAliveTime;

		while ( m_fAliveTime >= m_initData.m_fTimeStep )
		{
			StartStepWorld( -1 );
			FinishStepWorld( -1 );
			m_iNumSteps++;
#ifdef DEBLOB_COMMUNITY_PATCH
			m_uiTotalSteps++;
#endif
			m_fAliveTime -= m_initData.m_fTimeStep;
		}

		m_bInSimulation = TFALSE;
		DestroyPendingBodies();
	}
}

// Which batch UpdateBatches commits first (toggled every frame)
static TBOOL s_bCommitAddFirst = TFALSE;

// $deBlob: FUNCTION 00721620
void TPhysicsManager::UpdateBatches( TFLOAT a_fDeltaTime )
{
	m_auiBatchStats[ 0 ]  = m_auiBatchStats[ 6 ];
	m_auiBatchStats[ 1 ]  = m_auiBatchStats[ 7 ];
	m_auiBatchStats[ 2 ]  = m_auiBatchStats[ 8 ];
	m_auiBatchStats[ 3 ]  = m_auiBatchStats[ 9 ];
	m_auiBatchStats[ 4 ]  = m_auiBatchStats[ 10 ];
	m_auiBatchStats[ 5 ]  = m_auiBatchStats[ 11 ];
	m_auiBatchStats[ 12 ] = m_auiBatchStats[ 16 ];
	m_auiBatchStats[ 13 ] = m_auiBatchStats[ 17 ];
	m_auiBatchStats[ 14 ] = m_auiBatchStats[ 18 ];
	m_auiBatchStats[ 6 ]  = 0;
	m_auiBatchStats[ 7 ]  = 0;
	m_auiBatchStats[ 8 ]  = 0;
	m_auiBatchStats[ 9 ]  = 0;
	m_auiBatchStats[ 10 ] = 0;
	m_auiBatchStats[ 11 ] = 0;
	m_auiBatchStats[ 15 ] = m_auiBatchStats[ 19 ];
	m_auiBatchStats[ 16 ] = 0;
	m_auiBatchStats[ 17 ] = 0;
	m_auiBatchStats[ 18 ] = 0;
	m_auiBatchStats[ 19 ] = 0;

	if ( !m_bBatchCommitDisabled )
		return;

	TINT iNumAdded   = m_addedBodies.Size();
	TINT iNumRemoved = m_removedBodies.Size();

	if ( s_bCommitAddFirst )
	{
		if ( iNumAdded > 0 )
			CommitAddBatch();
		else if ( iNumRemoved > 0 )
			CommitRemoveBatch();
	}
	else
	{
		if ( iNumRemoved > 0 )
			CommitRemoveBatch();
		else if ( iNumAdded > 0 )
			CommitAddBatch();
	}

	CommitPhantomAddBatch();
	CommitPhantomRemoveBatch();

	s_bCommitAddFirst = !s_bCommitAddFirst;
}

// $deBlob: FUNCTION 00721770
void TPhysicsManager::StartStepWorld( TINT a_iWorld )
{
	TASSERT( m_initialised );

	m_iNumCollisionCallbacks        = 0;
	m_iNumTriggerCallbacks          = 0;
	m_iNumConstraintBrokenCallbacks = 0;

	for ( TINT i = 0; i < PHYSICS_WORLDS; i++ )
	{
		if ( ( a_iWorld == -1 || a_iWorld == i ) && m_abStepping[ i ] )
			return;
	}

	if ( a_iWorld == -1 )
	{
		for ( TINT i = 0; i < PHYSICS_WORLDS; i++ )
			m_abStepping[ i ] = TTRUE;
	}
	else
	{
		m_abStepping[ a_iWorld ] = TTRUE;
	}

	UpdateCustomMovers( a_iWorld );
	StartStepPlugins( a_iWorld );

	if ( a_iWorld != -1 )
	{
		StartStepWorldThread( a_iWorld );
		return;
	}

	for ( TINT i = 0; i < PHYSICS_WORLDS; i++ )
		StartStepWorldThread( i );
}

// NOTE: retail: m_apWorlds[ i ]->unmarkForWrite() and m_apMultithreadingUtils[ i ]->startStepWorld(
// m_initData.m_fTimeStep ). Havok 6.5: the step's jobs are put on the world's job queue and its one-thread pool
// processes them
void TPhysicsManager::StartStepWorldThread( TINT a_iWorld )
{
	hkpWorld* pWorld = m_apWorlds[ a_iWorld ];

	pWorld->unmarkForWrite();
	pWorld->initMtStep( m_apJobQueues[ a_iWorld ], m_initData.m_fTimeStep );
	m_apThreadPools[ a_iWorld ]->processAllJobs( m_apJobQueues[ a_iWorld ] );
}

// NOTE: retail: m_apMultithreadingUtils[ i ]->waitForStepWorld(). Havok 6.5: wait for the pool, then finish the
// step (continuous simulation, single threaded) and take the world back for writing
void TPhysicsManager::FinishStepWorldThread( TINT a_iWorld )
{
	hkpWorld* pWorld = m_apWorlds[ a_iWorld ];

	m_apThreadPools[ a_iWorld ]->waitForCompletion();
	pWorld->finishMtStep( m_apJobQueues[ a_iWorld ], m_apThreadPools[ a_iWorld ] );
	pWorld->markForWrite();
}

// $deBlob: FUNCTION 00721890
void TPhysicsManager::FinishStepWorld( TINT a_iWorld )
{
	TASSERT( m_initialised );

	for ( TINT i = 0; i < PHYSICS_WORLDS; i++ )
	{
		if ( ( a_iWorld == -1 || a_iWorld == i ) && !m_abStepping[ i ] )
			return;
	}

	if ( a_iWorld == -1 )
	{
		for ( TINT i = 0; i < PHYSICS_WORLDS; i++ )
		{
			m_abStepping[ i ] = TFALSE;
			FinishStepWorldThread( i );
		}
	}
	else
	{
		m_abStepping[ a_iWorld ] = TFALSE;
		FinishStepWorldThread( a_iWorld );
	}

	UpdateActiveBodies( a_iWorld );
	DispatchCollisionCallbacks( a_iWorld );
	DispatchConstraintBrokenCallbacks( a_iWorld );
	DispatchTriggerCallbacks( a_iWorld );

	for ( TINT i = 0; i < m_iNumStepListeners; i++ )
	{
		if ( a_iWorld == -1 || a_iWorld == m_apStepListeners[ i ]->GetWorld() )
			m_apStepListeners[ i ]->OnFinishStep( this );
	}
}

// $deBlob: FUNCTION 00726500
TPhysicsObjectContainer* TPhysicsManager::CreateFixedBody( TMatrix44* a_pMatrix, void* a_pUserData )
{
	TPhysicsObjectContainer* pBody = CreateBody( a_pMatrix, a_pUserData, 0, 1 );
	pBody->m_flags |= TPhysicsObjectContainer::PMF_FIXED;
	return pBody;
}

// $deBlob: FUNCTION 007219a0
TPhysicsObjectContainer* TPhysicsManager::CreateBody( TMatrix44* a_pMatrix, void* a_pUserData, TINT a_eMotionType, TINT a_eSubType )
{
	TASSERT( m_initialised );

	TPhysicsObjectContainer* pBody = TPhysicsObjectContainer::Create();
	pBody->InsertBefore( m_bodies.End() );

#ifdef TOSHI_DEBUG
	TVector4 vScale;
	a_pMatrix->GetScale( vScale );
	TASSERT( TMath::Abs( vScale.Magnitude() - TMath::Sqrt( 3.0f ) ) < PHYSICS_MANAGER_ERROR );
#endif

	pBody->m_pMatrix             = a_pMatrix;
	pBody->m_pUserData           = a_pUserData;
	pBody->m_eCreationMotionType = TUINT16( a_eMotionType );
	pBody->m_eMotionType         = TUINT16( a_eMotionType );
	pBody->m_flags               = 0;

	if ( a_eMotionType == MotionType_Keyframed )
		pBody->m_flags = TPhysicsObjectContainer::PMF_KEYFRAMED;
	else if ( a_eMotionType == MotionType_KeyframedReporting )
		pBody->m_flags = TPhysicsObjectContainer::PMF_KEYFRAMED | TPhysicsObjectContainer::PMF_UNK800;
	else if ( a_eMotionType == MotionType_Static )
		pBody->m_flags = TPhysicsObjectContainer::PMF_KEYFRAMED | TPhysicsObjectContainer::PMF_STATIC;
	else if ( a_eSubType == SubType_SphereInertia )
		pBody->m_flags = TPhysicsObjectContainer::PMF_SPHERE_INERTIA;
	else if ( a_eSubType == SubType_Unk2 )
		pBody->m_flags = TPhysicsObjectContainer::PMF_UNK2000;

	pBody->m_iWorld = m_iDefaultWorld;
	return pBody;
}

// $deBlob: FUNCTION 00721b40
void TPhysicsManager::DestroyBody( TPhysicsObjectContainer* a_pBody )
{
	TASSERT( m_initialised );

	RemoveFromSimulation( a_pBody, TFALSE );

	if ( m_bBatchOperationInProgress )
	{
		a_pBody->m_flags |= TPhysicsObjectContainer::PMF_DELETE_PENDING;
		return;
	}

	if ( m_pConstraints )
	{
		for ( TINT i = 0; i < m_pConstraints->Size(); i++ )
		{
			if ( ( *m_pConstraints )[ i ]->m_pBodyA == a_pBody )
				( *m_pConstraints )[ i ]->m_pBodyA = TNULL;

			if ( ( *m_pConstraints )[ i ]->m_pBodyB == a_pBody )
				( *m_pConstraints )[ i ]->m_pBodyB = TNULL;
		}
	}

	if ( m_bInSimulation )
	{
		a_pBody->Remove();
		a_pBody->InsertBefore( m_destroyBodies.End() );
		return;
	}

	a_pBody->Remove();
	TPhysicsObjectContainer::Destroy( a_pBody );
}

// $deBlob: FUNCTION 00727670
void TPhysicsManager::DestroyPendingBodies()
{
	while ( !m_destroyBodies.IsEmpty() )
	{
		TPhysicsObjectContainer* pBody = m_destroyBodies.Begin();
		pBody->Remove();
		TPhysicsObjectContainer::Destroy( pBody );
	}
}

// $deBlob: FUNCTION 007276d0
TBOOL TPhysicsManager::IsBodyInList( TPhysicsObjectContainer* a_pBody, T2DList<TPhysicsObjectContainer>& a_rList )
{
	return a_rList.Exists( a_pBody );
}

// $deBlob: FUNCTION 00721c30
void TPhysicsManager::SetMotionType( TPhysicsObjectContainer* a_pBody, TUINT a_eMotionType )
{
	if ( a_eMotionType == a_pBody->m_eMotionType || ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_FIXED ) )
		return;

	TUINT16 eCreationMotionType = a_pBody->m_eCreationMotionType;

	if ( eCreationMotionType == MotionType_Keyframed || eCreationMotionType == MotionType_KeyframedReporting )
	{
		// NOTE: retail only asserts on bodies created keyframed and returns, they never change their motion type
		TASSERT( a_eMotionType != TPhysicsManager::MotionType_Dynamic, "TPhysicsManager: Incompatible motion type\n" );
		return;
	}

	if ( eCreationMotionType == MotionType_Static )
	{
		TASSERT( 0, "TPhysicsManager: Incompatible motion type\n" );
		return;
	}

	TUINT16 uiFlags        = a_pBody->m_flags;
	a_pBody->m_eMotionType = TUINT16( a_eMotionType );

	if ( a_eMotionType == MotionType_Static )
		uiFlags = ( uiFlags & ~TPhysicsObjectContainer::PMF_UNK800 ) | TPhysicsObjectContainer::PMF_STATIC | TPhysicsObjectContainer::PMF_KEYFRAMED;
	else if ( a_eMotionType == MotionType_Keyframed )
		uiFlags = ( uiFlags & ~( TPhysicsObjectContainer::PMF_UNK800 | TPhysicsObjectContainer::PMF_STATIC ) ) | TPhysicsObjectContainer::PMF_KEYFRAMED;
	else if ( a_eMotionType == MotionType_KeyframedReporting )
		uiFlags = ( uiFlags & ~TPhysicsObjectContainer::PMF_STATIC ) | TPhysicsObjectContainer::PMF_UNK800 | TPhysicsObjectContainer::PMF_KEYFRAMED;
	else
		uiFlags = uiFlags & ~( TPhysicsObjectContainer::PMF_UNK800 | TPhysicsObjectContainer::PMF_STATIC | TPhysicsObjectContainer::PMF_KEYFRAMED );

	a_pBody->m_flags = uiFlags;

	if ( a_pBody->m_body )
		a_pBody->m_body->setMotionType( GetHavokMotionType( a_pBody, a_eMotionType ), HK_ENTITY_ACTIVATION_DO_ACTIVATE, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK );

	if ( TPhysicsObjectContainer::IsInSimulation( a_pBody ) && !( a_pBody->m_flags & ( TPhysicsObjectContainer::PMF_IN_ADD_BATCH | TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH | TPhysicsObjectContainer::PMF_DELETE_PENDING ) ) )
	{
		a_pBody->Remove();

		if ( a_eMotionType == MotionType_Static )
			a_pBody->InsertBefore( m_activeBodiesStatic.End() );
		else if ( a_eMotionType == MotionType_Dynamic )
			a_pBody->InsertBefore( m_activeBodiesDynamic.End() );
		else
			a_pBody->InsertBefore( m_activeBodiesKeyframe.End() );
	}
}

// $deBlob: FUNCTION 00721db0
void TPhysicsManager::AddToSimulation( TPhysicsObjectContainer* a_pBody, TBOOL a_bForce )
{
	TASSERT( a_pBody );

	if ( a_pBody && a_pBody->IsInSimulation() )
	{
		if ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH )
		{
			TASSERT( a_pBody->m_body->getWorld() );
			ActivateBody( a_pBody );
			return;
		}

		if ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_ADD_BATCH )
		{
			TASSERT( a_pBody->m_body );
		}
		else
		{
			TASSERT( a_pBody->m_body->getWorld() );
		}

		TASSERT( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH ) == 0 );
		return;
	}

	if ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_ADD_BATCH )
	{
		TASSERT( a_pBody->m_body );
		TASSERT( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH ) == 0 );
		return;
	}

	if ( !m_bBatchOperationInProgress || a_bForce )
	{
		TASSERT( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH ) == 0 );
		a_pBody->m_flags |= TPhysicsObjectContainer::PMF_IN_SIMULATION;
		ActivateBody( a_pBody );
	}
	else
	{
		AddBody( a_pBody );

		if ( a_pBody->m_body )
		{
			TASSERT( a_pBody->m_body->getWorld() == HK_NULL );
			return;
		}
	}

	TASSERT( a_pBody->m_body == HK_NULL );
	TASSERT( a_pBody->m_geom == HK_NULL );

	hkpRigidBodyCinfo oInfo;
	hkpMassProperties oMassProperties;
	TFLOAT            fAllowedPenetrationDepth = 0.0f;

	switch ( a_pBody->m_eShapeType )
	{
		case TShapeBaseClass::Type_Plane:
		{
			TPlaneData*     pPlaneData = TSTATICCAST( TPlaneData, a_pBody->m_pShapeData );
			const TVector4& rcNormal   = pPlaneData->GetNormal();
			const TVector4& rcPoint    = pPlaneData->GetPoint();

			// NOTE: retail's plane equation has +dot( normal, point ) as w (Havok's is n.x + w = 0) and its AABB has
			// min.z > max.z; kept as retail
			hkVector4 vPlane( rcNormal.x, rcNormal.y, rcNormal.z, rcNormal.y * rcPoint.y + rcPoint.x * rcNormal.x + rcNormal.z * rcPoint.z );
			hkAabb    oAabb;
			oAabb.m_min.set( 100.0f, 100.0f, 0.1f, 0.0f );
			oAabb.m_max.set( 100.0f, 100.0f, -0.1f, 0.0f );
			hkVector4 vMassHalfExtents( 100.0f, 100.0f, 0.1f, 0.0f );

			a_pBody->m_geom = new hkpPlaneShape( vPlane, oAabb );
			hkpInertiaTensorComputer::computeBoxVolumeMassProperties( vMassHalfExtents, a_pBody->m_fMass, oMassProperties );
			fAllowedPenetrationDepth = 0.2f;
			break;
		}
		case TShapeBaseClass::Type_Sphere:
		{
			TSphereData* pSphereData = TSTATICCAST( TSphereData, a_pBody->m_pShapeData );

			a_pBody->m_geom = new hkpSphereShape( pSphereData->GetRadius() );
			hkpInertiaTensorComputer::computeSphereVolumeMassProperties( pSphereData->GetRadius(), a_pBody->m_fMass, oMassProperties );
			fAllowedPenetrationDepth = pSphereData->GetRadius() * 0.2f;
			break;
		}
		case TShapeBaseClass::Type_Box:
		{
			TBoxData*       pBoxData = TSTATICCAST( TBoxData, a_pBody->m_pShapeData );
			const TVector4& rcHalf   = pBoxData->GetHalfExtents();
			hkVector4       vHalfExtents( rcHalf.x, rcHalf.y, rcHalf.z, 0.0f );

			// NOTE: retail allocates the shapes with hkThreadMemory (its 5.5 operator new); the same through 6.5's
			a_pBody->m_geom = new hkpBoxShape( vHalfExtents, 0.05f );
			hkpInertiaTensorComputer::computeBoxVolumeMassProperties( vHalfExtents, a_pBody->m_fMass, oMassProperties );

			fAllowedPenetrationDepth = TMath::Min( TMath::Min( rcHalf.x, rcHalf.y ), rcHalf.z ) * 0.2f;
			break;
		}
		case TShapeBaseClass::Type_Capsule:
		{
			TCapsuleData* pCapsuleData = TSTATICCAST( TCapsuleData, a_pBody->m_pShapeData );
			hkVector4     vA( 0.0f, 0.0f, pCapsuleData->GetHeight() * 0.5f, 0.0f );
			hkVector4     vB( 0.0f, 0.0f, pCapsuleData->GetHeight() * -0.5f, 0.0f );

			a_pBody->m_geom = new hkpCapsuleShape( vA, vB, pCapsuleData->GetRadius() );
			hkpInertiaTensorComputer::computeCapsuleVolumeMassProperties( vA, vB, pCapsuleData->GetRadius(), a_pBody->m_fMass, oMassProperties );
			fAllowedPenetrationDepth = pCapsuleData->GetRadius() * 0.2f;
			break;
		}
		case TShapeBaseClass::Type_Cylinder:
		{
			TCylinderData* pCylinderData = TSTATICCAST( TCylinderData, a_pBody->m_pShapeData );
			hkVector4      vA( 0.0f, 0.0f, pCylinderData->GetHeight() * 0.5f, 0.0f );
			hkVector4      vB( 0.0f, 0.0f, pCylinderData->GetHeight() * -0.5f, 0.0f );

			a_pBody->m_geom = new hkpCylinderShape( vA, vB, pCylinderData->GetRadius(), 0.05f );
			hkpInertiaTensorComputer::computeCylinderVolumeMassProperties( vA, vB, pCylinderData->GetRadius(), a_pBody->m_fMass, oMassProperties );
			fAllowedPenetrationDepth = pCylinderData->GetRadius() * 0.2f;
			break;
		}
		case TShapeBaseClass::Type_Mesh:
		{
			TPhysicsMeshContainer* pContainer = a_pBody->m_pMeshContainer;

			if ( !pContainer->IsConvex() )
			{
				hkpMeshShape* pMeshShape;

				if ( pContainer->IsCustomMesh() == TTRUE || pContainer->GetIndexSize() == sizeof( TUINT8 ) )
				{
					TPhysicsCustomMeshShape* pCustomShape = new TPhysicsCustomMeshShape( pContainer );
					pCustomShape->SetRadius( 0.0f );
					pMeshShape = pCustomShape;
				}
				else
				{
					TPhysicsStandardMeshShape* pStandardShape = new TPhysicsStandardMeshShape( pContainer );
					pStandardShape->SetRadius( 0.0f );
					pMeshShape = pStandardShape;
				}

				hkpMeshShape::Subpart oPart;
				oPart.m_materialIndexStriding = 0;
				oPart.m_numMaterials          = 1;
				oPart.m_materialStriding      = 0;
				oPart.m_vertexBase            = pContainer->GetVertices();
				oPart.m_vertexStriding        = pContainer->GetVertexStride();
				oPart.m_numVertices           = pContainer->GetNumVertices();
				oPart.m_indexBase             = pContainer->GetIndices();

				if ( pContainer->GetPrimitiveType() == TPhysicsPrimitiveType_List )
					oPart.m_indexStriding = pContainer->GetIndexStride() * 3;
				else
					oPart.m_indexStriding = pContainer->GetIndexStride();

				oPart.m_numTriangles           = pContainer->GetNumTriangles();
				oPart.m_stridingType           = ( pContainer->GetIndexSize() == sizeof( TUINT32 ) ) ? hkpMeshShape::INDICES_INT32 : hkpMeshShape::INDICES_INT16;
				oPart.m_flipAlternateTriangles = 0;

				if ( pContainer->GetMaterialIndices() == TNULL || pContainer->GetMaterial( 0 ) == TNULL )
				{
					oPart.m_materialIndexBase = HK_NULL;
					oPart.m_materialBase      = HK_NULL;
					oPart.m_numMaterials      = 0;
				}
				else
				{
					pContainer->GetMaterial( 0 );
					oPart.m_materialIndexStridingType = hkpMeshShape::MATERIAL_INDICES_INT8;
					oPart.m_materialIndexBase         = pContainer->GetMaterialIndices();
					oPart.m_materialIndexStriding     = 1;
					// NOTE: TPhysicsMeshMaterial starts with hkpMeshMaterial's m_filterInfo
					oPart.m_materialBase     = TREINTERPRETCAST( const hkpMeshMaterial*, pContainer->GetMaterial( 0 ) );
					oPart.m_materialStriding = sizeof( TPhysicsMeshMaterial );
					oPart.m_numMaterials     = pContainer->GetNumMaterials();
				}

				pMeshShape->addSubpart( oPart );

				hkpMoppCode*         pMoppCode = HK_NULL;
				TModelCollisionMopp* pMopp     = pContainer->GetMoppCode();

				if ( pMopp == TNULL )
				{
					hkpMoppCompilerInput oMoppInput;
					pMoppCode = hkpMoppUtility::buildCode( pMeshShape, oMoppInput );
				}
				else
				{
					hkpMoppCode::CodeInfo codeInfo;
					codeInfo.m_offset.set( pMopp->m_aOffset[ 0 ], pMopp->m_aOffset[ 1 ], pMopp->m_aOffset[ 2 ], pMopp->m_aOffset[ 3 ] );
					TINT byteCodeSize = pMopp->m_iByteCodeSize;

					TASSERT( ( codeInfo.m_offset( 0 ) > -1e9f ) && ( codeInfo.m_offset( 0 ) < 1e9f ), "TPhysicsManager: Header of MoppCode Archive is corrupt!" );
					TASSERT( ( codeInfo.m_offset( 1 ) > -1e9f ) && ( codeInfo.m_offset( 1 ) < 1e9f ), "TPhysicsManager: Header of MoppCode Archive is corrupt!" );
					TASSERT( ( codeInfo.m_offset( 2 ) > -1e9f ) && ( codeInfo.m_offset( 2 ) < 1e9f ), "TPhysicsManager: Header of MoppCode Archive is corrupt!" );
					TASSERT( byteCodeSize < 10000000, "TPhysicsManager: Mopp tree data corrupt, byte code size is huge!" );

					pMoppCode = new hkpMoppCode( codeInfo, pMopp->m_pByteCode, byteCodeSize );
					// NOTE: Havok 5.5's hkpMoppCode has no build type; 6.5's external data ctor leaves it unset
					pMoppCode->m_buildType = hkpMoppCode::BUILT_WITHOUT_CHUNK_SUBDIVISION;
				}

				pMeshShape->setUserData( hkUlong( a_pBody ) );

				hkpMoppBvTreeShape* pTreeShape = new hkpMoppBvTreeShape( pMeshShape, pMoppCode );
				pMoppCode->removeReference();

				a_pBody->m_geom = pTreeShape;
				hkpInertiaTensorComputer::computeSphereVolumeMassProperties( a_pBody->m_fRadius, a_pBody->m_fMass, oMassProperties );
				pMeshShape->removeReference();
			}
			else
			{
				// The convex hull of the vertices
				hkStridedVertices oStridedIn;
				oStridedIn.m_vertices    = TREINTERPRETCAST( const float*, pContainer->GetVertices() );
				oStridedIn.m_numVertices = pContainer->GetNumVertices();
				oStridedIn.m_striding    = 12;

				hkArray<hkVector4> aPlaneEquations;
				hkGeometry         oGeometry;
				hkGeometryUtility::createConvexGeometry( oStridedIn, oGeometry, aPlaneEquations, HK_GEOM_CONVEXHULL_MODE_FAST );

				hkStridedVertices oStridedHull;
				oStridedHull.m_vertices    = TREINTERPRETCAST( const float*, oGeometry.m_vertices.begin() );
				oStridedHull.m_numVertices = oGeometry.m_vertices.getSize();
				oStridedHull.m_striding    = sizeof( hkVector4 );

				// NOTE: retail creates the shape with the default convex radius and then zeroes it (+0x10, 5.5 layout)
				hkpConvexVerticesShape* pConvexShape = new hkpConvexVerticesShape( oStridedHull, aPlaneEquations, 0.05f );
				pConvexShape->setRadius( 0.0f );
				a_pBody->m_geom = pConvexShape;

				hkpInertiaTensorComputer::computeGeometryVolumeMassProperties( &oGeometry, a_pBody->m_fMass, oMassProperties );
			}

			fAllowedPenetrationDepth = 0.1f;
			break;
		}
		case TShapeBaseClass::Type_List:
		{
			if ( a_pBody->GetShapeList()->Size() == 0 )
			{
				TASSERT( 0 );
				break;
			}

			fAllowedPenetrationDepth = 0.1f;

			hkArray<hkpShape*>           aShapes;
			hkArray<hkpMassElement>      aMassElements;
			T2Vector<hkpMassElement, 50> vecElements;
			T2Vector<hkpShape*, 50>      vecReleaseShapes;

			TFLOAT fNodeMass = a_pBody->m_fMass / TFLOAT( a_pBody->GetShapeList()->Size() );

			for ( TINT i = 0; i < a_pBody->GetShapeList()->Size(); i++ )
			{
				TCSGNode* pNode  = ( *a_pBody->GetShapeList() )[ i ];
				hkpShape* pShape = HK_NULL;
				vecElements.PushBack();
				hkpMassElement*  pElement = &vecElements.Back();
				TShapeBaseClass* pData    = pNode->GetShapeData();

				switch ( pData->GetType() )
				{
					case TShapeBaseClass::Type_Sphere:
					{
						TFLOAT fRadius = TSTATICCAST( TSphereData, pData )->GetRadius();
						pShape         = new hkpSphereShape( fRadius );
						hkpInertiaTensorComputer::computeSphereVolumeMassProperties( fRadius, fNodeMass, pElement->m_properties );
						break;
					}
					case TShapeBaseClass::Type_Box:
					{
						const TVector4& rcHalf = TSTATICCAST( TBoxData, pData )->GetHalfExtents();
						hkVector4       vHalfExtents( rcHalf.x, rcHalf.y, rcHalf.z, 0.0f );
						pShape = new hkpBoxShape( vHalfExtents, 0.05f );
						hkpInertiaTensorComputer::computeBoxVolumeMassProperties( vHalfExtents, fNodeMass, pElement->m_properties );
						break;
					}
					case TShapeBaseClass::Type_Capsule:
					{
						TCapsuleData* pCapsuleData = TSTATICCAST( TCapsuleData, pData );
						hkVector4     vA( 0.0f, 0.0f, pCapsuleData->GetHeight() * 0.5f, 0.0f );
						hkVector4     vB( 0.0f, 0.0f, pCapsuleData->GetHeight() * -0.5f, 0.0f );
						pShape = new hkpCapsuleShape( vA, vB, pCapsuleData->GetRadius() );
						hkpInertiaTensorComputer::computeCapsuleVolumeMassProperties( vA, vB, pCapsuleData->GetRadius(), fNodeMass, pElement->m_properties );
						break;
					}
					case TShapeBaseClass::Type_Cylinder:
					{
						TCylinderData* pCylinderData = TSTATICCAST( TCylinderData, pData );
						hkVector4      vA( 0.0f, 0.0f, pCylinderData->GetHeight() * 0.5f, 0.0f );
						hkVector4      vB( 0.0f, 0.0f, pCylinderData->GetHeight() * -0.5f, 0.0f );
						pShape = new hkpCylinderShape( vA, vB, pCylinderData->GetRadius(), 0.05f );
						hkpInertiaTensorComputer::computeCylinderVolumeMassProperties( vA, vB, pCylinderData->GetRadius(), fNodeMass, pElement->m_properties );
						break;
					}
					default:
						TASSERT( TFALSE );
						break;
				}

				if ( pShape )
				{
					pShape->setUserData( hkUlong( a_pBody ) );

					hkTransform oTransform;
					oTransform.set4x4ColumnMajor( &pNode->GetTransform().m_f11 );

					hkpShape* pTransformShape = new hkpTransformShape( pShape, oTransform );
					aShapes.pushBack( pTransformShape );

					pElement->m_transform.set4x4ColumnMajor( &pNode->GetTransform().m_f11 );
					vecReleaseShapes.PushBack( pShape );
					vecReleaseShapes.PushBack( pTransformShape );
				}

				hkpMassElement& rNewElement = aMassElements.expandOne();
				rNewElement.m_properties    = pElement->m_properties;
				rNewElement.m_transform     = pElement->m_transform;
			}

			if ( aShapes.getSize() > 0 )
			{
				a_pBody->m_geom = new hkpListShape( aShapes.begin(), aShapes.getSize() );
				hkpInertiaTensorComputer::combineMassProperties( aMassElements, oMassProperties );
			}

			for ( TINT i = 0; i < vecReleaseShapes.Size(); i++ )
				vecReleaseShapes[ i ]->removeReference();

			break;
		}
		case TShapeBaseClass::Type_HeightField:
		{
			TPhysicsHFContainer* pContainer = a_pBody->m_pHFContainer;

			hkpSampledHeightFieldBaseCinfo oCinfo;
			oCinfo.m_xRes      = pContainer->m_x;
			oCinfo.m_zRes      = pContainer->m_y;
			oCinfo.m_minHeight = pContainer->m_fMinHeight;
			oCinfo.m_maxHeight = pContainer->m_fMaxHeight;
			oCinfo.m_scale.set( pContainer->m_fSizeX / TFLOAT( pContainer->m_x ), 1.0f, pContainer->m_fSizeY / TFLOAT( pContainer->m_y ), 0.0f );

			a_pBody->m_geom = new TPhysicsHeightFieldShape( oCinfo, pContainer );
			break;
		}
		default:
			TASSERT( TFALSE );
			break;
	}

	if ( a_pBody->m_eShapeType == TShapeBaseClass::Type_HeightField )
	{
		TPhysicsHFContainer* pContainer = a_pBody->m_pHFContainer;

		// Rotate the Havok height field (x, z, heights along y) into the container's plane
		TMatrix44 oRotation;
		oRotation.Identity();
		oRotation.RotateX( 1.5707964f );

		TMatrix44 oLocal;
		oLocal.AsBasisVector4( 0 ) = oRotation.AsBasisVector4( 0 );
		oLocal.AsBasisVector4( 1 ) = oRotation.AsBasisVector4( 1 );
		oLocal.AsBasisVector4( 2 ) = oRotation.AsBasisVector4( 2 );
		oLocal.AsBasisVector4( 3 ).Set( 0.0f, pContainer->m_fSizeY, 0.0f, 1.0f );

		TMatrix44 oMatrix;
		oMatrix.Multiply( oLocal, *a_pBody->m_pMatrix );
		*a_pBody->m_pMatrix = oMatrix;
		pContainer->m_oInvTransform.Invert( oMatrix );
	}

	if ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_FIXED )
	{
		// A trigger volume: the shape becomes the bounding volume of a TPhysicsTriggerShape
		hkpShape* pTriggerShape = new TPhysicsTriggerShape( this );
		hkpShape* pBvShape      = new hkpBvShape( a_pBody->m_geom, pTriggerShape );

		a_pBody->m_geom->removeReference();
		pTriggerShape->removeReference();
		a_pBody->m_geom = pBvShape;
	}

	const TMatrix44& rcMatrix = *a_pBody->m_pMatrix;
	oInfo.m_position.set( rcMatrix.m_f41, rcMatrix.m_f42, rcMatrix.m_f43, 0.0f );

	TQuaternion quat;
	TMatrix44::MatToQuat( quat, rcMatrix );
	TFLOAT fQuatLength = TMath::Sqrt( quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w );

	if ( fQuatLength != 0.0f )
	{
		TFLOAT fInvLength = 1.0f / fQuatLength;
		quat.x *= fInvLength;
		quat.y *= fInvLength;
		quat.z *= fInvLength;
		quat.w *= fInvLength;
	}

	TUINT16 eMotionType = a_pBody->m_eCreationMotionType;

	oInfo.m_collisionFilterInfo         = a_pBody->m_uiCollisionFilterInfo;
	oInfo.m_shape                       = a_pBody->m_geom;
	oInfo.m_collisionResponse           = hkpMaterial::RESPONSE_SIMPLE_CONTACT;
	oInfo.m_processContactCallbackDelay = 0xffff;
	oInfo.m_rotation.m_vec.set( quat.x, quat.y, quat.z, quat.w );
	oInfo.m_linearVelocity.setZero4();
	oInfo.m_angularVelocity.setZero4();
	oInfo.m_inertiaTensor = oMassProperties.m_inertiaTensor;
	oInfo.m_centerOfMass.set( a_pBody->m_vCenterOfMass.x, a_pBody->m_vCenterOfMass.y, a_pBody->m_vCenterOfMass.z, 0.0f );
	oInfo.m_mass                     = a_pBody->m_fMass;
	oInfo.m_linearDamping            = a_pBody->m_fLinearDamping;
	oInfo.m_angularDamping           = a_pBody->m_fAngularDamping;
	oInfo.m_friction                 = a_pBody->m_fFriction;
	oInfo.m_restitution              = a_pBody->m_fRestitution;
	oInfo.m_maxLinearVelocity        = a_pBody->m_fMaxLinearVelocity;
	oInfo.m_maxAngularVelocity       = a_pBody->m_fMaxAngularVelocity;
	oInfo.m_allowedPenetrationDepth  = fAllowedPenetrationDepth;
	oInfo.m_motionType               = GetHavokMotionType( a_pBody, eMotionType );
	oInfo.m_rigidBodyDeactivatorType = hkpRigidBodyDeactivator::DEACTIVATOR_SPATIAL;
	oInfo.m_solverDeactivation       = hkpRigidBodyCinfo::SOLVER_DEACTIVATION_LOW;
	oInfo.m_qualityType              = GetHavokQualityType( a_pBody, eMotionType );
	oInfo.m_autoRemoveLevel          = 0;

	a_pBody->m_geom->setUserData( hkUlong( a_pBody ) );
	a_pBody->m_body = new hkpRigidBody( oInfo );

	if ( !( a_pBody->m_flags & TPhysicsObjectContainer::PMF_FIXED ) && a_pBody->m_eCreationMotionType != a_pBody->m_eMotionType )
	{
		a_pBody->m_body->setMotionType( GetHavokMotionType( a_pBody, a_pBody->m_eMotionType ), HK_ENTITY_ACTIVATION_DO_ACTIVATE, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK );
		a_pBody->m_body->getCollidableRw()->setQualityType( GetHavokQualityType( a_pBody, a_pBody->m_eMotionType ) );
	}

	if ( a_pBody->m_pListener && !( a_pBody->m_flags & TPhysicsObjectContainer::PMF_FIXED ) )
	{
		a_pBody->m_pCollisionListener = new TPhysicsManagerCollisionListener( a_pBody->m_body, this );
	}

	if ( a_pBody->m_eMotionType == MotionType_Dynamic )
	{
		// NOTE: retail writes hkpEntity::m_processContactCallbackDelay (+0x96 in Havok 5.5: hkpRigidBody's ctor copies
		// it from hkpRigidBodyCinfo +0xa there)
		a_pBody->m_body->setProcessContactCallbackDelay( 0 );
	}

	if ( !m_bBatchOperationInProgress || a_bForce )
	{
		m_auiBatchStats[ 10 ]++;
		m_apWorlds[ a_pBody->m_iWorld ]->addEntity( a_pBody->m_body, HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE );
	}
}

// $deBlob: FUNCTION 00723600
void TPhysicsManager::RemoveFromSimulation( TPhysicsObjectContainer* a_pBody, TBOOL a_bForce )
{
	TASSERT( a_pBody );

	if ( !a_pBody || !a_pBody->IsInSimulation() )
	{
		if ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_ADD_BATCH )
		{
			TASSERT( a_pBody->m_body );
			RemoveBody( a_pBody );
			return;
		}

		if ( a_pBody->m_body )
		{
			TASSERT( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH ) );
		}

		return;
	}

	TASSERT( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_ADD_BATCH ) == 0 );

	if ( !m_bBatchOperationInProgress || a_bForce )
	{
		TASSERT( a_pBody->m_body->getWorld() );
		a_pBody->m_flags &= ~TPhysicsObjectContainer::PMF_IN_SIMULATION;
		DeactivateBody( a_pBody );
	}
	else if ( !( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH ) )
	{
		TASSERT( a_pBody->m_body->getWorld() );
		RemoveBody( a_pBody );
		return;
	}

	if ( m_bBatchOperationInProgress && !a_bForce )
		return;

	hkpWorld* pWorld = a_pBody->m_body->getWorld();

	if ( m_pConstraints )
	{
		for ( TINT i = 0; i < m_pConstraints->Size(); i++ )
		{
			TPhysicsConstraintContainer* pConstraint = ( *m_pConstraints )[ i ];

			if ( pConstraint->m_pBodyA == a_pBody || pConstraint->m_pBodyB == a_pBody )
				RemoveConstraint( pConstraint );
		}
	}

	m_auiBatchStats[ 11 ]++;
	TASSERT( !TPhysicsObjectContainer::IsInSimulation( a_pBody ) );

	pWorld->removeEntity( a_pBody->m_body );

	TASSERT( a_pBody->m_geom->getReferenceCount() == 2 );
	TASSERT( a_pBody->m_body->getReferenceCount() == 1 );

	a_pBody->m_geom->removeReference();
	a_pBody->m_body->removeReference();

	a_pBody->m_geom                 = TNULL;
	a_pBody->m_body                 = TNULL;
	a_pBody->m_pCollisionListener   = TNULL;
	a_pBody->m_pSurfaceVelocityUtil = TNULL;
}

// $deBlob: FUNCTION 00723ad0
void TPhysicsManager::SetBox( TPhysicsObjectContainer* a_pBody, const TVector4& a_rcHalfExtents )
{
	TVector4 vHalfExtents = a_rcHalfExtents;

	if ( vHalfExtents.x > 0.05f )
		vHalfExtents.x -= 0.05f;

	if ( vHalfExtents.y > 0.05f )
		vHalfExtents.y -= 0.05f;

	if ( vHalfExtents.z > 0.05f )
		vHalfExtents.z -= 0.05f;

	a_pBody->m_eShapeType = TShapeBaseClass::Type_Box;
	a_pBody->m_pShapeData = TBoxData::Get( vHalfExtents );
	a_pBody->m_fRadius    = vHalfExtents.Magnitude();
}

// $deBlob: FUNCTION 00723a90
void TPhysicsManager::SetSphere( TPhysicsObjectContainer* a_pBody, TFLOAT a_fRadius )
{
	if ( a_fRadius > 0.025f )
		a_fRadius -= 0.025f;

	a_pBody->m_fRadius    = a_fRadius;
	a_pBody->m_eShapeType = TShapeBaseClass::Type_Sphere;
	a_pBody->m_pShapeData = TSphereData::Get( a_fRadius );
}

// $deBlob: FUNCTION 00723b50
void TPhysicsManager::SetCapsule( TPhysicsObjectContainer* a_pBody, TFLOAT a_fHeight, TFLOAT a_fRadius )
{
	// NOTE: retail checks 0.05 but subtracts 0.025 (the cylinder subtracts 0.05)
	if ( a_fRadius > 0.05f )
		a_fRadius -= 0.025f;

	a_pBody->m_eShapeType = TShapeBaseClass::Type_Capsule;
	a_pBody->m_fRadius    = a_fHeight * 0.5f + a_fRadius;
	a_pBody->m_pShapeData = TCapsuleData::Get( a_fHeight, a_fRadius );
}

// $deBlob: FUNCTION 00723bb0
void TPhysicsManager::SetCylinder( TPhysicsObjectContainer* a_pBody, TFLOAT a_fHeight, TFLOAT a_fRadius )
{
	if ( a_fRadius > 0.05f )
		a_fRadius -= 0.05f;

	if ( a_fHeight > 0.05f )
		a_fHeight -= 0.05f;

	a_pBody->m_eShapeType = TShapeBaseClass::Type_Cylinder;
	a_pBody->m_fRadius    = a_fHeight * 0.5f + a_fRadius;
	a_pBody->m_pShapeData = TCylinderData::Get( a_fHeight, a_fRadius );
}

// $deBlob: FUNCTION 00723c20
void TPhysicsManager::SetPlane( TPhysicsObjectContainer* a_pBody, const TVector4& a_rcNormal, const TVector4& a_rcPoint )
{
	a_pBody->m_eShapeType = TShapeBaseClass::Type_Plane;
	a_pBody->m_pShapeData = TPlaneData::Get( a_rcNormal, a_rcPoint );
	a_pBody->m_fRadius    = 10000.0f;
}

// $deBlob: FUNCTION 00723c50
void TPhysicsManager::SetMesh( TPhysicsObjectContainer* a_pBody, TPhysicsMeshContainer* a_pContainer )
{
	a_pBody->m_eShapeType     = TShapeBaseClass::Type_Mesh;
	a_pBody->m_pMeshContainer = a_pContainer;
	a_pBody->m_fRadius        = a_pContainer->GetBoundingRadius();
}

// $deBlob: FUNCTION 00723cd0
void TPhysicsManager::SetHeightField( TPhysicsObjectContainer* a_pBody, TPhysicsHFContainer* a_pContainer, TBOOL a_bUnk )
{
	a_pBody->m_eShapeType   = TShapeBaseClass::Type_HeightField;
	a_pBody->m_pHFContainer = a_pContainer;
	a_pBody->m_fRadius      = 1.0f;

	if ( a_bUnk )
		a_pBody->m_flags |= TPhysicsObjectContainer::PMF_UNK40;
}

// $deBlob: FUNCTION 00723a10
void TPhysicsManager::SetListener( TPhysicsObjectContainer* a_pBody, TPhysicsResponder* a_pListener )
{
	a_pBody->m_pListener = a_pListener;

	if ( !a_pBody->m_body )
		return;

	TPhysicsManagerCollisionListener* pCollisionListener = TSTATICCAST( TPhysicsManagerCollisionListener, a_pBody->m_pCollisionListener );

	if ( a_pListener == TNULL )
	{
		if ( pCollisionListener )
		{
			a_pBody->m_body->removeCollisionListener( pCollisionListener );
#ifdef DEBLOB_COMMUNITY_PATCH
			// NOTE: retail leaves the deleted listener registered as the entity listener of the body
			a_pBody->m_body->removeEntityListener( pCollisionListener );
#endif
			delete pCollisionListener;
			a_pBody->m_pCollisionListener = TNULL;
		}
	}
	else if ( !pCollisionListener )
	{
		a_pBody->m_pCollisionListener = new TPhysicsManagerCollisionListener( a_pBody->m_body, this );
	}
}

// $deBlob: FUNCTION 00724340
void TPhysicsManager::SetCollisionLayer( TPhysicsObjectContainer* a_pBody, TUINT32 a_uiLayer )
{
	TUINT32 uiOldInfo                = a_pBody->m_uiCollisionFilterInfo;
	TUINT32 uiNewInfo                = ( ( uiOldInfo ^ a_uiLayer ) & 0xfff ) ^ uiOldInfo;
	a_pBody->m_uiCollisionFilterInfo = uiNewInfo;

	if ( a_pBody->m_body && uiOldInfo != uiNewInfo )
	{
		a_pBody->m_body->getCollidableRw()->setCollisionFilterInfo( uiNewInfo );

		if ( a_pBody->IsInSimulation() )
			m_apWorlds[ a_pBody->m_iWorld ]->updateCollisionFilterOnEntity( a_pBody->m_body, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
	}
}

// $deBlob: FUNCTION 00724390
void TPhysicsManager::SetCollisionGroup( TPhysicsObjectContainer* a_pBody, TUINT32 a_uiGroup )
{
	TUINT32 uiOldInfo                = a_pBody->m_uiCollisionFilterInfo;
	TUINT32 uiNewInfo                = ( ( a_uiGroup << 12 ^ uiOldInfo ) & 0xfff000 ) ^ uiOldInfo;
	a_pBody->m_uiCollisionFilterInfo = uiNewInfo;

	if ( a_pBody->m_body && uiOldInfo != uiNewInfo )
	{
		a_pBody->m_body->getCollidableRw()->setCollisionFilterInfo( uiNewInfo );

		if ( a_pBody->IsInSimulation() )
			m_apWorlds[ a_pBody->m_iWorld ]->updateCollisionFilterOnEntity( a_pBody->m_body, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
	}
}

// Adds a node to the shape list of a_pBody and grows its radius to a_fExtent
static void AddListNode( TPhysicsObjectContainer* a_pBody, TCSGNode* a_pNode, TFLOAT a_fExtent )
{
	a_pBody->GetShapeList()->PushBack( a_pNode );

	if ( a_pBody->m_fRadius < a_fExtent )
		a_pBody->m_fRadius = a_fExtent;
}

// $deBlob: FUNCTION 00723d00
void TPhysicsManager::AddListSphere( TPhysicsObjectContainer* a_pBody, const TMatrix44& a_rcTransform, TFLOAT a_fRadius )
{
	if ( a_fRadius > 0.025f )
		a_fRadius -= 0.025f;

	a_pBody->m_eShapeType = TShapeBaseClass::Type_List;
#ifdef TOSHI_DEBUG
	TVector4 vScale;
	a_rcTransform.GetScale( vScale );
	TASSERT( TMath::Abs( vScale.Magnitude() - TMath::Sqrt( 3.0f ) ) < PHYSICS_MANAGER_ERROR );
#endif

	TCSGNode* pNode = TCSGNode::Get( TShapeBaseClass::Type_Sphere, TSphereData::Get( a_fRadius ), a_rcTransform );
	AddListNode( a_pBody, pNode, a_rcTransform.GetTranslation().Magnitude() + a_fRadius );
}

// $deBlob: FUNCTION 00723e60
void TPhysicsManager::AddListBox( TPhysicsObjectContainer* a_pBody, const TMatrix44& a_rcTransform, const TVector4& a_rcHalfExtents )
{
	TVector4 vHalfExtents = a_rcHalfExtents;

	if ( vHalfExtents.x > 0.05f )
		vHalfExtents.x -= 0.05f;

	if ( vHalfExtents.y > 0.05f )
		vHalfExtents.y -= 0.05f;

	if ( vHalfExtents.z > 0.05f )
		vHalfExtents.z -= 0.05f;

	a_pBody->m_eShapeType = TShapeBaseClass::Type_List;
#ifdef TOSHI_DEBUG
	TVector4 vScale;
	a_rcTransform.GetScale( vScale );
	TASSERT( TMath::Abs( vScale.Magnitude() - TMath::Sqrt( 3.0f ) ) < PHYSICS_MANAGER_ERROR );
#endif

	TCSGNode* pNode = TCSGNode::Get( TShapeBaseClass::Type_Box, TBoxData::Get( vHalfExtents ), a_rcTransform );
	AddListNode( a_pBody, pNode, a_rcTransform.GetTranslation().Magnitude() + vHalfExtents.Magnitude() );
}

// $deBlob: FUNCTION 00724030
void TPhysicsManager::AddListCapsule( TPhysicsObjectContainer* a_pBody, const TMatrix44& a_rcTransform, TFLOAT a_fHeight, TFLOAT a_fRadius )
{
	if ( a_fRadius > 0.05f )
		a_fRadius -= 0.025f;

	a_pBody->m_eShapeType = TShapeBaseClass::Type_List;
#ifdef TOSHI_DEBUG
	TVector4 vScale;
	a_rcTransform.GetScale( vScale );
	TASSERT( TMath::Abs( vScale.Magnitude() - TMath::Sqrt( 3.0f ) ) < PHYSICS_MANAGER_ERROR );
#endif

	TCSGNode* pNode = TCSGNode::Get( TShapeBaseClass::Type_Capsule, TCapsuleData::Get( a_fHeight, a_fRadius ), a_rcTransform );
	AddListNode( a_pBody, pNode, a_rcTransform.GetTranslation().Magnitude() + a_fHeight * 0.5f + a_fRadius );
}

// $deBlob: FUNCTION 007241b0
void TPhysicsManager::AddListCylinder( TPhysicsObjectContainer* a_pBody, const TMatrix44& a_rcTransform, TFLOAT a_fHeight, TFLOAT a_fRadius )
{
	if ( a_fRadius > 0.05f )
		a_fRadius -= 0.05f;

	if ( a_fHeight > 0.05f )
		a_fHeight -= 0.05f;

	a_pBody->m_eShapeType = TShapeBaseClass::Type_List;
#ifdef TOSHI_DEBUG
	TVector4 vScale;
	a_rcTransform.GetScale( vScale );
	TASSERT( TMath::Abs( vScale.Magnitude() - TMath::Sqrt( 3.0f ) ) < PHYSICS_MANAGER_ERROR );
#endif

	TCSGNode* pNode = TCSGNode::Get( TShapeBaseClass::Type_Cylinder, TCylinderData::Get( a_fHeight, a_fRadius ), a_rcTransform );
	AddListNode( a_pBody, pNode, a_rcTransform.GetTranslation().Magnitude() + a_fHeight * 0.5f + a_fRadius );
}

// Set while a batch operation may start
// NOTE: in retail this flag is in .bss and nothing writes it, so it's always FALSE: BeginBatchOperation never starts a
// batch, AddToSimulation adds every body at once and the add/remove batches stay empty outside SetBatchCommitDisabled
static TBOOL s_bBatchOperationsEnabled = TFALSE;

// $deBlob: FUNCTION 00723940
void TPhysicsManager::BeginBatchOperation()
{
	if ( s_bBatchOperationsEnabled )
	{
		TASSERT( m_bBatchOperationInProgress == TFALSE );
		m_bBatchOperationInProgress = TTRUE;
	}
}

// $deBlob: FUNCTION 00723990
void TPhysicsManager::EndBatchOperation()
{
	m_bBatchOperationInProgress = TFALSE;

	if ( !m_bBatchCommitDisabled )
	{
		CommitRemoveBatch();
		CommitAddBatch();
		CommitPhantomAddBatch();
		CommitPhantomRemoveBatch();
	}
}

// The batch size of hkpWorld::addEntityBatch/removeEntityBatch
static TCONSTEXPR_VAR TINT MAX_BATCH_ENTITIES = 200;

// The world with the most bodies of a_rList (the first one on a tie)
static TINT GetBatchWorld( T2DList<TPhysicsObjectContainer>& a_rList )
{
	TINT aCounts[ PHYSICS_WORLDS ] = { 0, 0 };

	for ( auto it = a_rList.Begin(); it != a_rList.End(); it++ )
		aCounts[ it->m_iWorld ]++;

	TINT iCount = -1;
	TINT iWorld = -1;

	if ( aCounts[ 0 ] >= 0 )
	{
		iWorld = 0;
		iCount = aCounts[ 0 ];
	}

	if ( iCount < aCounts[ 1 ] )
		iWorld = 1;

	return iWorld;
}

// $deBlob: FUNCTION 00727b60
void TPhysicsManager::CommitRemoveBatch()
{
	T2DList<TPhysicsObjectContainer> oRemoved;
	hkpEntity*                       apEntities[ MAX_BATCH_ENTITIES ];
	TINT                             iNumEntities = 0;
	TBOOL                            bBatch       = m_removedBodies.Size() >= 2;
	TINT                             iWorld       = GetBatchWorld( m_removedBodies );

	for ( auto it = m_removedBodies.Begin(); it != m_removedBodies.End(); )
	{
		TPhysicsObjectContainer* pBody = it;
		it++;

		if ( pBody->m_body->getWorld() == HK_NULL )
		{
			pBody->Remove();
			pBody->InsertBefore( oRemoved.End() );
		}
		else if ( !bBatch )
		{
			pBody->Remove();
			pBody->InsertBefore( oRemoved.End() );
			pBody->m_flags &= ~TPhysicsObjectContainer::PMF_IN_SIMULATION;
			m_auiBatchStats[ 11 ]++;
			pBody->m_body->getWorld()->removeEntity( pBody->m_body );
		}
		else if ( pBody->m_iWorld == iWorld )
		{
			pBody->Remove();
			pBody->InsertBefore( oRemoved.End() );
			apEntities[ iNumEntities ] = pBody->m_body;
			iWorld                     = pBody->m_iWorld;
			pBody->m_flags &= ~TPhysicsObjectContainer::PMF_IN_SIMULATION;

			TPhysicsObjectContainer* POC = pBody;
			TASSERT( POC->m_body->getWorld() );

			// NOTE: retail frees an internal array of the Havok 5.5 entity here (+0x50/+0x54, 32-byte elements);
			// 6.5's removeEntityBatch handles its own data

			iNumEntities++;

			if ( iNumEntities >= MAX_BATCH_ENTITIES || ( iNumEntities >= m_iBatchCommitLimit && m_bBatchCommitDisabled == TTRUE ) )
			{
				m_auiBatchStats[ 7 ]++;
				m_auiBatchStats[ 9 ] += iNumEntities;
				m_apWorlds[ iWorld ]->removeEntityBatch( apEntities, iNumEntities );

				TBOOL bStop  = iNumEntities >= m_iBatchCommitLimit && m_bBatchCommitDisabled == TTRUE;
				iNumEntities = 0;

				if ( bStop )
					break;
			}
		}
	}

	if ( iNumEntities > 0 )
	{
		m_auiBatchStats[ 7 ]++;
		m_auiBatchStats[ 9 ] += iNumEntities;
		m_apWorlds[ iWorld ]->removeEntityBatch( apEntities, iNumEntities );
	}

	while ( !oRemoved.IsEmpty() )
	{
		TPhysicsObjectContainer* POC = oRemoved.Begin();
		DeactivateBody( POC );

		// NOTE: retail inlines RemoveConstraint for the constraints of the body
		if ( m_pConstraints )
		{
			for ( TINT i = 0; i < m_pConstraints->Size(); i++ )
			{
				TPhysicsConstraintContainer* pConstraint = ( *m_pConstraints )[ i ];

				if ( pConstraint->m_pBodyA == POC || pConstraint->m_pBodyB == POC )
					RemoveConstraint( pConstraint );
			}
		}

		TASSERT( POC->m_geom->getReferenceCount() == 2 );
		TASSERT( POC->m_body->getReferenceCount() == 1 );

		POC->m_geom->removeReference();
		POC->m_body->removeReference();

		POC->m_geom                 = TNULL;
		POC->m_body                 = TNULL;
		POC->m_pCollisionListener   = TNULL;
		POC->m_pSurfaceVelocityUtil = TNULL;

		if ( POC->m_flags & TPhysicsObjectContainer::PMF_DELETE_PENDING )
		{
			POC->m_flags &= ~TPhysicsObjectContainer::PMF_DELETE_PENDING;
			DestroyBody( POC );
		}
	}
}

// $deBlob: FUNCTION 00727760
void TPhysicsManager::CommitAddBatch()
{
	T2DList<TPhysicsObjectContainer> oAdded;
	hkpEntity*                       apEntities[ MAX_BATCH_ENTITIES ];
	TINT                             iNumEntities = 0;
	TBOOL                            bBatch       = m_addedBodies.Size() >= 2;
	TINT                             iWorld       = GetBatchWorld( m_addedBodies );

	for ( auto it = m_addedBodies.Begin(); it != m_addedBodies.End(); )
	{
		TPhysicsObjectContainer* pBody = it;
		it++;

		if ( !bBatch )
		{
			pBody->Remove();
			pBody->InsertBefore( oAdded.End() );
			pBody->m_flags |= TPhysicsObjectContainer::PMF_IN_SIMULATION;
			m_auiBatchStats[ 10 ]++;
			m_apWorlds[ pBody->m_iWorld ]->addEntity( pBody->m_body, HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE );
		}
		else if ( pBody->m_iWorld == iWorld )
		{
			pBody->Remove();
			pBody->InsertBefore( oAdded.End() );
			apEntities[ iNumEntities++ ] = pBody->m_body;
			iWorld                       = pBody->m_iWorld;
			pBody->m_flags |= TPhysicsObjectContainer::PMF_IN_SIMULATION;

			if ( iNumEntities >= MAX_BATCH_ENTITIES || ( iNumEntities >= m_iBatchCommitLimit && m_bBatchCommitDisabled == TTRUE ) )
			{
				m_auiBatchStats[ 6 ]++;
				m_auiBatchStats[ 8 ] += iNumEntities;
				m_apWorlds[ iWorld ]->addEntityBatch( apEntities, iNumEntities, HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE );

				TBOOL bStop  = iNumEntities >= m_iBatchCommitLimit && m_bBatchCommitDisabled == TTRUE;
				iNumEntities = 0;

				if ( bStop )
					break;
			}
		}
	}

	if ( iNumEntities > 0 )
	{
		m_auiBatchStats[ 6 ]++;
		m_auiBatchStats[ 8 ] += iNumEntities;
		m_apWorlds[ iWorld ]->addEntityBatch( apEntities, iNumEntities, HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE );
	}

	while ( !oAdded.IsEmpty() )
	{
		TPhysicsObjectContainer* POC = oAdded.Begin();
		TASSERT( POC->m_body->getWorld() );
		ActivateBody( POC );
	}
}

// $deBlob: FUNCTION 00728170
void TPhysicsManager::CommitPhantomAddBatch()
{
	if ( m_PhantomAddBatch.IsEmpty() )
		return;

	T2DList<TPhysicsPhantomContainer> oBatch;

	// The batch is for the world with the most pending phantoms (the first one on a tie), the others wait
	TINT aiCounts[ 2 ] = { 0, 0 };

	for ( auto it = m_PhantomAddBatch.Begin(); it != m_PhantomAddBatch.End(); it++ )
		aiCounts[ it->m_uiWorld ]++;

	TINT iMax = -1;

	if ( aiCounts[ 0 ] > -1 )
		iMax = aiCounts[ 0 ];

	TINT        iWorld = ( iMax < aiCounts[ 1 ] ) ? 1 : 0;
	hkpPhantom* apPhantoms[ 200 ];
	TINT        iNumPhantoms = 0;

	for ( auto it = m_PhantomAddBatch.Begin(); it != m_PhantomAddBatch.End(); it++ )
	{
		TPhysicsPhantomContainer* pContainer = it;

		if ( pContainer->m_uiWorld == iWorld )
		{
			it = pContainer->Prev();
			pContainer->Remove();
			pContainer->InsertBefore( oBatch.End() );
			apPhantoms[ iNumPhantoms++ ] = pContainer->m_phantom;
			pContainer->m_uiFlags |= TPhysicsPhantomContainer::PPF_IN_SIMULATION;

			if ( iNumPhantoms >= 200 )
			{
				m_auiBatchStats[ 16 ] += iNumPhantoms;
				m_apWorlds[ iWorld ]->addPhantomBatch( apPhantoms, iNumPhantoms );
				iNumPhantoms = 0;
			}
		}
	}

	if ( iNumPhantoms > 0 )
	{
		m_auiBatchStats[ 16 ] += iNumPhantoms;
		m_apWorlds[ iWorld ]->addPhantomBatch( apPhantoms, iNumPhantoms );
	}

	while ( !oBatch.IsEmpty() )
	{
		TPhysicsPhantomContainer* pContainer = oBatch.Begin();
		TASSERT( pContainer->m_phantom->getWorld(), "phantomContainer->m_phantom->getWorld()" );
		pContainer->Remove();
		pContainer->InsertBefore( m_phantoms.End() );
	}
}

// $deBlob: FUNCTION 00728480
void TPhysicsManager::CommitPhantomRemoveBatch()
{
	if ( m_PhantomRemoveBatch.IsEmpty() )
		return;

	T2DList<TPhysicsPhantomContainer> oBatch;

	// The batch is for the world with the most pending phantoms (the first one on a tie), the others wait
	TINT aiCounts[ 2 ] = { 0, 0 };

	for ( auto it = m_PhantomRemoveBatch.Begin(); it != m_PhantomRemoveBatch.End(); it++ )
		aiCounts[ it->m_uiWorld ]++;

	TINT iMax = -1;

	if ( aiCounts[ 0 ] > -1 )
		iMax = aiCounts[ 0 ];

	TINT        iWorld = ( iMax < aiCounts[ 1 ] ) ? 1 : 0;
	hkpPhantom* apPhantoms[ 200 ];
	TINT        iNumPhantoms = 0;

	for ( auto it = m_PhantomRemoveBatch.Begin(); it != m_PhantomRemoveBatch.End(); it++ )
	{
		TPhysicsPhantomContainer* pContainer = it;

		if ( pContainer->m_uiWorld == iWorld )
		{
			it = pContainer->Prev();
			pContainer->Remove();
			pContainer->InsertBefore( oBatch.End() );
			apPhantoms[ iNumPhantoms++ ] = pContainer->m_phantom;
			pContainer->m_uiFlags &= ~TPhysicsPhantomContainer::PPF_IN_SIMULATION;

			if ( iNumPhantoms >= 200 )
			{
				m_auiBatchStats[ 17 ] += iNumPhantoms;
				m_apWorlds[ iWorld ]->removePhantomBatch( apPhantoms, iNumPhantoms );
				iNumPhantoms = 0;
			}
		}
	}

	if ( iNumPhantoms > 0 )
	{
		m_auiBatchStats[ 17 ] += iNumPhantoms;
		m_apWorlds[ iWorld ]->removePhantomBatch( apPhantoms, iNumPhantoms );
	}

	while ( !oBatch.IsEmpty() )
	{
		TPhysicsPhantomContainer* pContainer = oBatch.Begin();
		pContainer->Remove();
		pContainer->InsertBefore( m_phantoms.End() );
	}
}


// $deBlob: FUNCTION 007239c0
void TPhysicsManager::SetBatchCommitDisabled( TBOOL a_bBatchCommitDisabled, TINT a_iBatchCommitLimit )
{
	m_iBatchCommitLimit = a_iBatchCommitLimit;

	if ( m_bBatchCommitDisabled != a_bBatchCommitDisabled )
	{
		m_bBatchCommitDisabled = a_bBatchCommitDisabled;

		if ( !a_bBatchCommitDisabled )
		{
			CommitRemoveBatch();
			CommitAddBatch();
			CommitPhantomAddBatch();
			CommitPhantomRemoveBatch();
		}
	}
}

// $deBlob: FUNCTION 00726290
void TPhysicsManager::AddCustomMover( TPhysicsMotionBase* a_pMover )
{
	RemoveCustomMover( a_pMover );
	TASSERT( m_iNumCustomMovers < m_initData.m_iMaxCustomMovers );

	if ( m_iNumCustomMovers < m_initData.m_iMaxCustomMovers )
	{
		a_pMover->m_iWorld = m_iDefaultWorld;
		m_pCustomMovers->PushBack( a_pMover );
		m_iNumCustomMovers++;
	}
}

// $deBlob: FUNCTION 00726320
void TPhysicsManager::RemoveCustomMover( TPhysicsMotionBase* a_pMover )
{
	for ( TINT i = 0; i < m_iNumCustomMovers; i++ )
	{
		if ( ( *m_pCustomMovers )[ i ] == a_pMover )
		{
			m_pCustomMovers->Erase( m_pCustomMovers->Find( a_pMover ) );
			m_iNumCustomMovers--;
			return;
		}
	}
}

// $deBlob: FUNCTION 0072aaf0
void TPhysicsManager::UpdateCustomMovers( TINT a_iWorld )
{
	for ( TINT i = 0; i < m_iNumCustomMovers; i++ )
	{
		TPhysicsMotionBase* pMover = ( *m_pCustomMovers )[ i ];

		if ( a_iWorld == -1 || a_iWorld == pMover->m_iWorld )
		{
			// The mover's world is the default world while it updates (TPhysicsManager_i_Inline.h SetDefaultWorld)
			TASSERT( pMover->m_iWorld >= 0 );
			TASSERT( pMover->m_iWorld < PHYSICS_WORLDS );
			TINT iOldWorld  = m_iDefaultWorld;
			m_iDefaultWorld = pMover->m_iWorld;

			pMover->Update( m_initData.m_fTimeStep, this );

			TASSERT( iOldWorld >= 0 );
			TASSERT( iOldWorld < PHYSICS_WORLDS );
			m_iDefaultWorld = iOldWorld;
		}
	}
}

// $deBlob: FUNCTION 00726720
void TPhysicsManager::GetMemoryStatistics( hkMemoryStatistics& a_rOutStats )
{
	hkMemoryStatistics oStats;
	m_pFreeListMemory->calculateStatistics( oStats );
	a_rOutStats = oStats;
}

// $deBlob: FUNCTION 00726770
void TPhysicsManager::RecordCollisionCallback( TPhysicsObjectContainer* a_pBodyA, TPhysicsObjectContainer* a_pBodyB, TUINT32 a_uiMaterialA, TUINT32 a_uiMaterialB, const TVector4& a_rPosition, const TVector4& a_rNormal, TUINT16 a_uiType, TUINT16 a_uiContactPointId )
{
	// Skip the point if a callback of the same type was already recorded for the pair at about the same place
	for ( TINT i = 0; i < m_iNumCollisionCallbacks; i++ )
	{
		TPhysicsCollisionCallback* pCallback = &m_pCollisionCallbacks[ i ];

		if ( ( pCallback->m_uiType & a_uiType ) != 0 &&
		     ( pCallback->m_uiType & TPhysicsCollisionCallback::Type_Disabled ) == 0 &&
		     ( ( pCallback->m_aInfo[ 0 ].m_self.m_pBody == a_pBodyA && pCallback->m_aInfo[ 0 ].m_other.m_pBody == a_pBodyB ) ||
		       ( pCallback->m_aInfo[ 0 ].m_self.m_pBody == a_pBodyB && pCallback->m_aInfo[ 0 ].m_other.m_pBody == a_pBodyA ) ) )
		{
			TVector4 vDiff;
			vDiff.x = pCallback->m_aInfo[ 0 ].m_vPosition.x - a_rPosition.x;
			vDiff.y = pCallback->m_aInfo[ 0 ].m_vPosition.y - a_rPosition.y;
			vDiff.z = pCallback->m_aInfo[ 0 ].m_vPosition.z - a_rPosition.z;
			vDiff.w = pCallback->m_aInfo[ 0 ].m_vPosition.w;

			if ( vDiff.MagnitudeSq() < 0.001f )
				return;
		}
	}

	TASSERT( m_iNumCollisionCallbacks < m_initData.m_iMaxCollisionCallbacks, "TPhysicsManager: Physics manager has run out of collision callbacks.\n" );

	if ( m_iNumCollisionCallbacks >= m_initData.m_iMaxCollisionCallbacks )
		return;

	TFLOAT   fRelativeSpeed = 0.0f;
	TVector4 vVelocityA( 0.0f, 0.0f, 0.0f, 1.0f );
	TVector4 vVelocityB( 0.0f, 0.0f, 0.0f, 1.0f );

	if ( a_pBodyA->m_eMotionType == MotionType_Dynamic || a_pBodyB->m_eMotionType == MotionType_Dynamic )
	{
		TVector4 vRelative;

		if ( a_pBodyA->m_eMotionType == MotionType_Dynamic )
		{
			const hkVector4& rVelA = a_pBodyA->m_body->getLinearVelocity();
			vVelocityA.Set( rVelA( 0 ), rVelA( 1 ), rVelA( 2 ), 1.0f );

			if ( a_pBodyB->m_eMotionType == MotionType_Dynamic )
			{
				const hkVector4& rVelB = a_pBodyB->m_body->getLinearVelocity();
				vVelocityB.Set( rVelB( 0 ), rVelB( 1 ), rVelB( 2 ), 1.0f );
				vRelative.Set( vVelocityA.x - vVelocityB.x, vVelocityA.y - vVelocityB.y, vVelocityA.z - vVelocityB.z, 1.0f );
			}
			else
			{
				vRelative = vVelocityA;
			}
		}
		else
		{
			const hkVector4& rVelB = a_pBodyB->m_body->getLinearVelocity();
			vVelocityB.Set( rVelB( 0 ), rVelB( 1 ), rVelB( 2 ), 1.0f );
			vRelative = vVelocityB;
		}

		fRelativeSpeed = vRelative.Magnitude();
	}

	TPhysicsCollisionCallback* pCallback = &m_pCollisionCallbacks[ m_iNumCollisionCallbacks ];
	pCallback->m_uiType                  = a_uiType;
	pCallback->m_uiContactPointId        = a_uiContactPointId;
	pCallback->m_aTransforms[ 0 ]        = *a_pBodyA->m_pMatrix;
	pCallback->m_aTransforms[ 1 ]        = *a_pBodyB->m_pMatrix;

	// Body A's view
	TPhysicsCollisionInfo& rInfoA = pCallback->m_aInfo[ 0 ];
	rInfoA.m_self.m_pBody         = a_pBodyA;
	rInfoA.m_other.m_pBody        = a_pBodyB;
	rInfoA.m_fRelativeSpeed       = fRelativeSpeed;
	rInfoA.m_vPosition            = a_rPosition;
	rInfoA.m_vNormal              = a_rNormal;
	rInfoA.m_self.m_vVelocity     = vVelocityA;
	rInfoA.m_self.m_pTransform    = &pCallback->m_aTransforms[ 0 ];
	rInfoA.m_other.m_pTransform   = &pCallback->m_aTransforms[ 1 ];
	rInfoA.m_other.m_vVelocity    = vVelocityB;
	rInfoA.m_self.m_uiMaterial    = a_uiMaterialA;
	rInfoA.m_other.m_uiMaterial   = a_uiMaterialB;

	// Body B's view
	TPhysicsCollisionInfo& rInfoB = pCallback->m_aInfo[ 1 ];
	rInfoB.m_self.m_pBody         = a_pBodyB;
	rInfoB.m_other.m_pBody        = a_pBodyA;
	rInfoB.m_fRelativeSpeed       = fRelativeSpeed;
	rInfoB.m_vPosition            = a_rPosition;
	rInfoB.m_vNormal.Multiply( a_rNormal, -1.0f );
	rInfoB.m_self.m_vVelocity   = vVelocityB;
	rInfoB.m_self.m_pTransform  = &pCallback->m_aTransforms[ 1 ];
	rInfoB.m_other.m_pTransform = &pCallback->m_aTransforms[ 0 ];
	rInfoB.m_other.m_vVelocity  = vVelocityA;
	rInfoB.m_self.m_uiMaterial  = a_uiMaterialB;
	rInfoB.m_other.m_uiMaterial = a_uiMaterialA;

	if ( a_uiType & TPhysicsCollisionCallback::Type_Added )
	{
		// Added points are dispatched at once and not kept
		DispatchCollisionCallback( pCallback, a_pBodyA->m_iWorld );
		return;
	}

	m_iNumCollisionCallbacks++;
	m_iMaxNumCollisionCallbacksUsed = TMath::Max( m_iMaxNumCollisionCallbacksUsed, m_iNumCollisionCallbacks );
}

// $deBlob: FUNCTION 00726d30
void TPhysicsManager::DisableCollisionCallbacks( TUINT16 a_uiContactPointId )
{
	for ( TINT i = 0; i < m_iNumCollisionCallbacks; i++ )
	{
		if ( m_pCollisionCallbacks[ i ].m_uiContactPointId == a_uiContactPointId )
			m_pCollisionCallbacks[ i ].m_uiType &= TPhysicsCollisionCallback::Type_Disabled;
	}
}

// Returns the mesh shape under a body's root shape (a hkpMoppBvTreeShape over a TPhysicsStandardMeshShape or
// TPhysicsCustomMeshShape), or TNULL
// NOTE: retail checks the root shape's Havok 5.5 type (0xb) and reads the mesh's subparts directly (picked by the key's
// sign bit); our meshes are Havok 6.5 hkpMeshShapes, so the subpart and material come from their inline getters
static const hkpMeshShape* GetRootMeshShape( const hkpCdBody* a_pCdBody )
{
	const hkpShape* pRootShape = a_pCdBody->getRootCollidable()->getShape();

	if ( pRootShape->getType() != HK_SHAPE_MOPP )
		return TNULL;

	const hkpShapeCollection* pCollection = TSTATICCAST( const hkpMoppBvTreeShape, pRootShape )->getShapeCollection();

	if ( pCollection->getType() != HK_SHAPE_TRIANGLE_COLLECTION )
		return TNULL;

	return TSTATICCAST( const hkpMeshShape, pCollection );
}

// $deBlob: FUNCTION 00726d80
TUINT32 TPhysicsManager::GetCollisionMaterial( const hkpCdBody* a_pCdBody )
{
	hkpShapeKey              uiKey     = a_pCdBody->getShapeKey();
	TPhysicsObjectContainer* pBody     = TREINTERPRETCAST( TPhysicsObjectContainer*, a_pCdBody->getRootCollidable()->getShape()->getUserData() );
	TUINT32                  uiDefault = pBody->m_uiSurface;

	if ( uiKey != HK_INVALID_SHAPE_KEY )
	{
		const hkpMeshShape* pMesh = GetRootMeshShape( a_pCdBody );

		if ( pMesh && pMesh->getSubPart( uiKey ).m_materialIndexStriding != 0 )
		{
			const TPhysicsMeshMaterial* pMaterial = TREINTERPRETCAST( const TPhysicsMeshMaterial*, pMesh->getMeshMaterial( uiKey ) );

			if ( pMaterial )
				return pMaterial->m_uiSurface;
		}
	}

	return uiDefault;
}

// $deBlob: FUNCTION 00726e10
TPhysicsContactData* TPhysicsManager::AllocContactData()
{
	TPhysicsContactData* pData = TNULL;
	TPhysicsContactData* pLast = m_pContactData + m_iNumContactData - 1;
	TPhysicsContactData* pFree = m_pFreeContactData;

	if ( pFree && pFree <= pLast )
	{
		pFree->m_uiMaterialA = 0;
		pFree->m_uiMaterialB = 0;
		pFree->m_pBodyA      = TNULL;
		pFree->m_pBodyB      = TNULL;
		pFree->m_bUsed       = TTRUE;

		// Move to the next free entry
		do
		{
			m_pFreeContactData++;
			pData = pFree;
		} while ( m_pFreeContactData <= pLast && m_pFreeContactData->m_bUsed );
	}

	m_iNumContactDataAllocs++;
	return pData;
}

// $deBlob: FUNCTION 00726e90
void TPhysicsManager::FreeContactData( TPhysicsContactData* a_pData )
{
	a_pData->m_uiMaterialA = 0;
	a_pData->m_uiMaterialB = 0;
	a_pData->m_pBodyA      = TNULL;
	a_pData->m_pBodyB      = TNULL;
	a_pData->m_bUsed       = TFALSE;
	a_pData->m_bToi        = TFALSE;
	m_iNumContactDataFrees++;

	if ( a_pData < m_pFreeContactData )
		m_pFreeContactData = a_pData;
}

// $deBlob: FUNCTION 00726ed0
const TPhysicsMeshMaterial* TPhysicsManager::GetMeshMaterial( const hkpCdBody* a_pCdBody )
{
	hkpShapeKey uiKey = a_pCdBody->getShapeKey();

	if ( uiKey != HK_INVALID_SHAPE_KEY )
	{
		const hkpMeshShape* pMesh = GetRootMeshShape( a_pCdBody );

		if ( pMesh && pMesh->getSubPart( uiKey ).m_materialIndexStriding != 0 )
			return TREINTERPRETCAST( const TPhysicsMeshMaterial*, pMesh->getMeshMaterial( uiKey ) );
	}

	return TNULL;
}


// $deBlob: FUNCTION 00726400
void TPhysicsManager::AddPlugin( TPhysicsPluginBase* a_pPlugin )
{
	RemovePlugin( a_pPlugin );
	a_pPlugin->m_iWorld = m_iDefaultWorld;
	TASSERT( m_iNumStepListeners < TARRAYSIZE( m_apStepListeners ), "m_iNumElements < CAPACITY" );
	m_apStepListeners[ m_iNumStepListeners++ ] = a_pPlugin;
}

// $deBlob: FUNCTION 00726470
void TPhysicsManager::RemovePlugin( TPhysicsPluginBase* a_pPlugin )
{
	for ( TINT i = 0; i < m_iNumStepListeners; i++ )
	{
		if ( m_apStepListeners[ i ] == a_pPlugin )
		{
			for ( TINT j = i; j < m_iNumStepListeners - 1; j++ )
				m_apStepListeners[ j ] = m_apStepListeners[ j + 1 ];
			m_apStepListeners[ --m_iNumStepListeners ] = TNULL;
			return;
		}
	}
}

// $deBlob: FUNCTION 0072ac40
void TPhysicsManager::StartStepPlugins( TINT a_iWorld )
{
	for ( TINT i = 0; i < m_iNumStepListeners; i++ )
	{
		TPhysicsPluginBase* pPlugin = m_apStepListeners[ i ];

		if ( a_iWorld == -1 || a_iWorld == pPlugin->GetWorld() )
		{
			TASSERT( pPlugin->GetWorld() >= 0 );
			TASSERT( pPlugin->GetWorld() < PHYSICS_WORLDS );
			TINT iOldWorld  = m_iDefaultWorld;
			m_iDefaultWorld = pPlugin->GetWorld();

			pPlugin->OnStartStep( m_initData.m_fTimeStep, this );

			TASSERT( iOldWorld >= 0 );
			TASSERT( iOldWorld < PHYSICS_WORLDS );
			m_iDefaultWorld = iOldWorld;
		}
	}
}

// $deBlob: FUNCTION 0072ad90
void TPhysicsManager::FinishStepPlugins( TINT a_iWorld )
{
	for ( TINT i = 0; i < m_iNumStepListeners; i++ )
	{
		TPhysicsPluginBase* pPlugin = m_apStepListeners[ i ];

		if ( a_iWorld == -1 || a_iWorld == pPlugin->GetWorld() )
			pPlugin->OnFinishStep( this );
	}
}

// $deBlob: FUNCTION 0072aa10
void TPhysicsManager::UpdateActiveBody( TPhysicsObjectContainer* a_pBody )
{
	TASSERT( TPhysicsObjectContainer::IsInSimulation( a_pBody ) );

	if ( a_pBody->m_body->isActive() )
	{
		a_pBody->m_flags |= TPhysicsObjectContainer::PMF_MOVED;

		if ( !( a_pBody->m_flags & TPhysicsObjectContainer::PMF_POSITION_ONLY ) )
		{
			a_pBody->m_body->getTransform().get4x4ColumnMajor( &a_pBody->m_pMatrix->m_f11 );
		}
		else
		{
			// NOTE: retail reads the body's translation at raw 5.5 offsets (0x110..0x118); the 6.5 getPosition() is the same value
			const hkVector4& rcPosition = a_pBody->m_body->getPosition();
			a_pBody->m_pMatrix->m_f41   = rcPosition( 0 );
			a_pBody->m_pMatrix->m_f42   = rcPosition( 1 );
			a_pBody->m_pMatrix->m_f43   = rcPosition( 2 );
			a_pBody->m_pMatrix->m_f44   = 1.0f;
		}

		// The move callback: (*(code *)**(undefined4 **)m_pMoveCallback)(m_pUserData). Its type isn't known yet, the call is
		// __thiscall
		if ( a_pBody->m_pMoveCallback )
		{
			typedef void( __thiscall * t_MoveCallback )( void* a_pThis, void* a_pUserData );

			void**         pVTable = *(void***)a_pBody->m_pMoveCallback;
			t_MoveCallback fnMove  = (t_MoveCallback)pVTable[ 0 ];
			fnMove( a_pBody->m_pMoveCallback, a_pBody->m_pUserData );
		}
	}
}

// $deBlob: FUNCTION 0072a8b0
void TPhysicsManager::UpdateActiveBodies( TINT a_iWorld )
{
	for ( auto it = m_activeBodiesDynamic.Begin(); it != m_activeBodiesDynamic.End(); it++ )
	{
		if ( a_iWorld == -1 || a_iWorld == it->m_iWorld )
			UpdateActiveBody( it );
	}

	for ( auto it = m_activeBodiesKeyframe.Begin(); it != m_activeBodiesKeyframe.End(); it++ )
	{
		if ( a_iWorld == -1 || a_iWorld == it->m_iWorld )
			UpdateActiveBody( it );
	}
}

// $deBlob: FUNCTION 0072ade0
void TPhysicsManager::DispatchCollisionCallbacks( TINT a_iWorld )
{
	// NOTE: retail inlines DispatchCollisionCallback here
	for ( TINT i = 0; i < m_iNumCollisionCallbacks; i++ )
		DispatchCollisionCallback( &m_pCollisionCallbacks[ i ], a_iWorld );
}

// Calls the collision handler of the callback's type on the responders of both bodies
// $deBlob: FUNCTION 0072aef0
void TPhysicsManager::DispatchCollisionCallback( TPhysicsCollisionCallback* a_pCallback, TINT a_iWorld )
{
	if ( a_pCallback->m_uiType & TPhysicsCollisionCallback::Type_Disabled )
		return;

	TPhysicsObjectContainer* pBodyA = a_pCallback->m_aInfo[ 0 ].m_self.m_pBody;
	TPhysicsObjectContainer* pBodyB = a_pCallback->m_aInfo[ 0 ].m_other.m_pBody;

	if ( ( a_iWorld == -1 || a_iWorld == pBodyA->m_iWorld ) && pBodyA && pBodyA->IsInSimulation() && pBodyB && pBodyB->IsInSimulation() )
	{
		for ( TINT i = 0; i < 2; i++ )
		{
			TPhysicsResponder* pResponder = a_pCallback->m_aInfo[ i ].m_self.m_pBody->m_pListener;

			if ( pResponder )
			{
				TPhysicsCollisionInfo* pInfo = &a_pCallback->m_aInfo[ i ];

				if ( a_pCallback->m_uiType & TPhysicsCollisionCallback::Type_Added )
					pResponder->OnCollisionAdded( pInfo );
				else if ( a_pCallback->m_uiType & TPhysicsCollisionCallback::Type_Confirmed )
					pResponder->OnCollisionConfirmed( pInfo );
				else if ( a_pCallback->m_uiType & TPhysicsCollisionCallback::Type_Process )
					pResponder->OnCollisionProcess( pInfo );
				else
					pResponder->OnCollisionUnk10( pInfo );
			}
		}
	}
}

// Restores the motion values of a body of a broken constraint (used twice by DispatchConstraintBrokenCallbacks):
// a_pfBroken are the values after the break, a_pfAdded the ones when the constraint was added; with a_bBlend, the
// values are blended by the share of the body's constraints that are still intact
static void TPhysicsManager_RestoreBrokenConstraintBody( T2DynamicVector<TPhysicsConstraintContainer*>* a_pConstraints, TPhysicsObjectContainer* a_pBody, const TFLOAT* a_pfBroken, const TFLOAT* a_pfAdded, TBOOL a_bBlend )
{
	if ( !a_bBlend )
	{
		a_pBody->m_fAngularDamping    = a_pfBroken[ 0 ];
		a_pBody->m_fLinearDamping     = a_pfBroken[ 1 ];
		a_pBody->m_fMaxLinearVelocity = a_pfBroken[ 2 ];
	}
	else
	{
		TINT iNumConstraints = 0;
		TINT iNumIntact      = 0;

		for ( TINT i = 0; i < a_pConstraints->Size(); i++ )
		{
			TPhysicsConstraintContainer* pConstraint = ( *a_pConstraints )[ i ];

			if ( pConstraint->m_pBodyA == a_pBody || pConstraint->m_pBodyB == a_pBody )
			{
				iNumConstraints++;

				if ( ( pConstraint->m_uiFlags & TPhysicsConstraintContainer::PCCF_BREAKABLE ) == 0 || !const_cast<hkpBreakableConstraintData*>( static_cast<const hkpBreakableConstraintData*>( pConstraint->m_pInstance->getData() ) )->getIsBroken( pConstraint->m_pInstance ) )
					iNumIntact++;
			}
		}

		TFLOAT fBroken = 1.0f - TFLOAT( iNumIntact ) / TFLOAT( iNumConstraints );
		TFLOAT fAdded  = 1.0f - fBroken;

		a_pBody->m_fAngularDamping    = a_pfBroken[ 0 ] * fBroken + a_pfAdded[ 0 ] * fAdded;
		a_pBody->m_fLinearDamping     = a_pfBroken[ 1 ] * fBroken + a_pfAdded[ 1 ] * fAdded;
		a_pBody->m_fMaxLinearVelocity = a_pfBroken[ 2 ] * fBroken + a_pfAdded[ 2 ] * fAdded;
	}

	// NOTE: retail writes Havok 5.5's hkpRigidBody fields
	a_pBody->m_body->setAngularDamping( a_pBody->m_fAngularDamping );
	a_pBody->m_body->setLinearDamping( a_pBody->m_fLinearDamping );
	a_pBody->m_body->setMaxLinearVelocity( a_pBody->m_fMaxLinearVelocity );
}

// $deBlob: FUNCTION 0072afd0
void TPhysicsManager::DispatchConstraintBrokenCallbacks( TINT a_iWorld )
{
	for ( TINT i = 0; i < m_iNumConstraintBrokenCallbacks; i++ )
	{
		TUINT32*                     pCallback   = TSTATICCAST( TUINT32, m_pConstraintBrokenCallbacks ) + i * 2;
		hkpConstraintInstance*       pInstance   = TREINTERPRETCAST( hkpConstraintInstance*, pCallback[ 0 ] );
		TPhysicsConstraintContainer* pConstraint = TREINTERPRETCAST( TPhysicsConstraintContainer*, pInstance->getUserData() );
		TPhysicsObjectContainer*     pBodyA      = pConstraint->m_pBodyA;
		TPhysicsObjectContainer*     pBodyB      = pConstraint->m_pBodyB;

		if ( a_iWorld != -1 && a_iWorld != pBodyA->m_iWorld )
			continue;

		TFLOAT fForce = *TREINTERPRETCAST( TFLOAT*, pCallback + 1 );

		if ( pBodyA->m_pListener )
			pBodyA->m_pListener->OnConstraintBroken( pConstraint, pBodyA, pBodyB, fForce );

		if ( pBodyB->m_pListener )
			pBodyB->m_pListener->OnConstraintBroken( pConstraint, pBodyB, pBodyA, fForce );

		TPhysicsConstraintContainer::BreakData* pBreakData = pConstraint->m_pBreakableData;

		if ( pBreakData->m_bBodyA && pBodyA->m_eMotionType == MotionType_Dynamic )
			TPhysicsManager_RestoreBrokenConstraintBody( m_pConstraints, pBodyA, &pBreakData->m_fAngularDampingBrokenA, &pBreakData->m_fAngularDampingA, pBreakData->m_uiBodyAFlag != 0 );

		if ( pBreakData->m_bBodyB && pBodyB->m_eMotionType == MotionType_Dynamic )
			TPhysicsManager_RestoreBrokenConstraintBody( m_pConstraints, pBodyB, &pBreakData->m_fAngularDampingBrokenB, &pBreakData->m_fAngularDampingB, pBreakData->m_uiBodyBFlag != 0 );
	}
}

// $deBlob: FUNCTION 0072b340
void TPhysicsManager::DispatchTriggerCallbacks( TINT a_iWorld )
{
	for ( TINT i = 0; i < m_iNumTriggerCallbacks; i++ )
	{
		TPhysicsTriggerCallback* pTriggerEvent = &m_pTriggerCallbacks[ i ];
		TASSERT( pTriggerEvent != TNULL );

		if ( a_iWorld == -1 || a_iWorld == pTriggerEvent->m_pTrigger->m_iWorld )
		{
			TPhysicsObjectContainer* pTrigger = pTriggerEvent->m_pTrigger;

			if ( pTrigger && pTrigger->IsInSimulation() && pTrigger->m_pListener )
			{
				if ( pTriggerEvent->m_bEnter )
					pTrigger->m_pListener->OnTriggerEnter( pTrigger, pTriggerEvent->m_pOther );
				else
					pTrigger->m_pListener->OnTriggerLeave( pTrigger, pTriggerEvent->m_pOther );
			}
		}
	}
}

// $deBlob: FUNCTION 007255a0
TINT TPhysicsManager::FindConstraint( TPhysicsConstraintContainer* a_pConstraint ) const
{
	for ( TINT i = 0; i < m_pConstraints->Size(); i++ )
	{
		if ( ( *m_pConstraints )[ i ] == a_pConstraint )
			return i;
	}

	return -1;
}

// $deBlob: FUNCTION 007255e0
TPhysicsConstraintContainer* TPhysicsManager::CreateConstraint( TPhysicsObjectContainer* a_pBodyA, TPhysicsObjectContainer* a_pBodyB, TBOOL a_bWantRuntime )
{
	TPhysicsConstraintContainer* pConstraint = new TPhysicsConstraintContainer();
	pConstraint->m_pBodyA                    = a_pBodyA;
	pConstraint->m_pBodyB                    = a_pBodyB;
	m_pConstraints->PushBack( pConstraint );
	pConstraint->m_bWantRuntime = a_bWantRuntime;
	pConstraint->m_iWorld = m_iDefaultWorld;
	return pConstraint;
}

// $deBlob: FUNCTION 007243f0
TINT8 TPhysicsManager::FindFreeSubSystemId()
{
	TINT8 iStart = TINT8( m_iLastSubSystemId );
	TINT8 iId    = iStart;

	do
	{
		iId++;
		*TREINTERPRETCAST( TINT8*, &m_iLastSubSystemId ) = iId;

		if ( iId == iStart )
			return iId;

		// Is a body using the id?
		TBOOL bUsed = TFALSE;

		for ( auto it = m_bodies.Begin(); it != m_bodies.End(); it++ )
		{
			if ( TINT8( it->m_uiCollisionFilterInfo >> 24 ) == iId )
			{
				bUsed = TTRUE;
				break;
			}
		}

		if ( !bUsed )
			return TINT8( m_iLastSubSystemId );
	} while ( TTRUE );
}

// $deBlob: FUNCTION 005a63d0
void TPhysicsManager::SetSubSystemId( TPhysicsObjectContainer* a_pBody, TINT8 a_iId )
{
	if ( a_pBody != TNULL )
	{
		// NOTE: retail clears the top byte (0x3b of the body) first
		a_pBody->m_uiCollisionFilterInfo &= 0x00ffffff;
		a_pBody->m_uiCollisionFilterInfo |= TUINT32( TUINT8( a_iId ) ) << 24;
	}
}

// $deBlob: FUNCTION 005a6460
void TPhysicsManager::SetConstraintBreakable( TPhysicsConstraintContainer* a_pConstraint, TFLOAT a_fThreshold )
{
	a_pConstraint->m_uiFlags = ( a_pConstraint->m_uiFlags & ~TPhysicsConstraintContainer::PCCF_MALLEABLE ) | TPhysicsConstraintContainer::PCCF_BREAKABLE;

	if ( a_pConstraint->m_pBreakableData != TNULL )
		TFree( a_pConstraint->m_pBreakableData );

	TPhysicsConstraintContainer::BreakData* pData = TSTATICCAST( TPhysicsConstraintContainer::BreakData, TMalloc( sizeof( TPhysicsConstraintContainer::BreakData ) ) );

	// NOTE: retail does not check the allocation
	pData->m_fValue = 0.0f;
	pData->m_bBodyA = TFALSE;
	pData->m_bBodyB = TFALSE;

	a_pConstraint->m_pBreakableData = pData;
	pData->m_fValue                 = a_fThreshold;
}

// $deBlob: FUNCTION 005a65b0
void TPhysicsManager::SetConstraintMalleable( TPhysicsConstraintContainer* a_pConstraint, TFLOAT a_fStrength, TFLOAT a_fUnk )
{
	a_pConstraint->m_uiFlags = ( a_pConstraint->m_uiFlags & ~TPhysicsConstraintContainer::PCCF_BREAKABLE ) | TPhysicsConstraintContainer::PCCF_MALLEABLE;

	if ( a_pConstraint->m_pBreakableData != TNULL )
		TFree( a_pConstraint->m_pBreakableData );

	// Only the first two floats of the data are used by a malleable constraint
	TPhysicsConstraintContainer::BreakData* pData = TSTATICCAST( TPhysicsConstraintContainer::BreakData, TMalloc( 8 ) );
	a_pConstraint->m_pBreakableData               = pData;
	// NOTE: the second float of a malleable constraint's data shares the slot of m_fAngularDampingBrokenA
	pData->m_fAngularDampingBrokenA               = a_fUnk;
	pData->m_fValue                               = a_fStrength;
}

// $deBlob: FUNCTION 00725fc0
void TPhysicsManager::SetConstraintBodyBreakInfo( TPhysicsConstraintContainer* a_pConstraint, TPhysicsObjectContainer* a_pBody, TFLOAT a_fUnk, TFLOAT a_fAngularDamping, TFLOAT a_fLinearDamping, TUINT8 a_uiFlag )
{
	TASSERT( a_pConstraint->m_uiFlags & TPhysicsConstraintContainer::PCCF_BREAKABLE );

	if ( a_pBody == a_pConstraint->m_pBodyA )
	{
		a_pConstraint->m_pBreakableData->m_bBodyA      = 0.0f < a_fUnk;
		a_pConstraint->m_pBreakableData->m_fAngularDampingBrokenA = a_fAngularDamping;
		a_pConstraint->m_pBreakableData->m_fLinearDampingBrokenA  = a_fLinearDamping;
		a_pConstraint->m_pBreakableData->m_uiBodyAFlag = a_uiFlag;
		return;
	}

	if ( a_pBody == a_pConstraint->m_pBodyB )
	{
		a_pConstraint->m_pBreakableData->m_bBodyB      = 0.0f < a_fUnk;
		a_pConstraint->m_pBreakableData->m_fAngularDampingBrokenB = a_fAngularDamping;
		a_pConstraint->m_pBreakableData->m_fLinearDampingBrokenB  = a_fLinearDamping;
		a_pConstraint->m_pBreakableData->m_uiBodyBFlag = a_uiFlag;
		return;
	}

	TASSERT( TFALSE );
}

// $deBlob: FUNCTION 007260c0
void TPhysicsManager::SetConstraintSteeringAngle( TPhysicsConstraintContainer* a_pConstraint, TFLOAT a_fAngle )
{
	if ( !a_pConstraint->m_bInWorld )
		return;

	TASSERT( a_pConstraint->m_eType == TPhysicsConstraintContainer::TYPE_WHEEL );

	hkpConstraintData* pData;

	if ( a_pConstraint->m_uiFlags & TPhysicsConstraintContainer::PCCF_BREAKABLE )
	{
		hkpBreakableConstraintData* pBreakable = TSTATICCAST( hkpBreakableConstraintData, a_pConstraint->m_pData );

		if ( pBreakable->getIsBroken( a_pConstraint->m_pInstance ) )
			return;

		pData = pBreakable->getWrappedConstraintData();
	}
	else if ( a_pConstraint->m_uiFlags & TPhysicsConstraintContainer::PCCF_MALLEABLE )
	{
		pData = TSTATICCAST( hkpMalleableConstraintData, a_pConstraint->m_pData )->getWrappedConstraintData();
	}
	else
	{
		pData = a_pConstraint->m_pData;
	}

	if ( pData != TNULL )
		TSTATICCAST( hkpWheelConstraintData, pData )->setSteeringAngle( a_fAngle );
}

// $deBlob: FUNCTION 00726150
TFLOAT TPhysicsManager::GetConstraintImpulse( TPhysicsConstraintContainer* a_pConstraint )
{
	if ( !a_pConstraint->m_bInWorld || !a_pConstraint->m_bWantRuntime )
		return 0.0f;

	// NOTE: the solver results (hkpSolverResults, 8 bytes each) at the start of the Havok 5.5 runtime of each
	// constraint type
	const hkpSolverResults* pResults = TREINTERPRETCAST( const hkpSolverResults*, a_pConstraint->m_pInstance->getRuntime() );

	switch ( a_pConstraint->m_eType )
	{
		case TPhysicsConstraintContainer::TYPE_BALLANDSOCKET:
			return pResults[ 0 ].getImpulseApplied() + 0.0f + pResults[ 1 ].getImpulseApplied() + pResults[ 2 ].getImpulseApplied();
		case TPhysicsConstraintContainer::TYPE_STIFFSPRING:
			return pResults[ 0 ].getImpulseApplied() + 0.0f;
		case TPhysicsConstraintContainer::TYPE_HINGE:
			return pResults[ 2 ].getImpulseApplied() + 0.0f + pResults[ 3 ].getImpulseApplied() + pResults[ 4 ].getImpulseApplied();
		case TPhysicsConstraintContainer::TYPE_LIMITEDHINGE:
			return pResults[ 5 ].getImpulseApplied() + 0.0f + pResults[ 6 ].getImpulseApplied() + pResults[ 7 ].getImpulseApplied() + pResults[ 2 ].getImpulseApplied();
		case TPhysicsConstraintContainer::TYPE_PRISMATIC:
			return pResults[ 5 ].getImpulseApplied() + 0.0f + pResults[ 6 ].getImpulseApplied() + pResults[ 7 ].getImpulseApplied();
		case TPhysicsConstraintContainer::TYPE_WHEEL:
			return pResults[ 5 ].getImpulseApplied() + 0.0f;
		default:
			TASSERT( TFALSE );
			return 0.0f;
	}
}

// $deBlob: FUNCTION 00726560
TPhysicsObjectContainer* TPhysicsManager::GetFirstActiveBody( TINT a_eMotionType )
{
	T2DList<TPhysicsObjectContainer>* pList;

	if ( a_eMotionType == MotionType_Static )
		pList = &m_activeBodiesStatic;
	else if ( a_eMotionType == MotionType_Dynamic )
		pList = &m_activeBodiesDynamic;
	else
		pList = &m_activeBodiesKeyframe;

	if ( pList->Begin() == pList->End() )
		return TNULL;

	return pList->Begin();
}

// $deBlob: FUNCTION 007265d0
TPhysicsObjectContainer* TPhysicsManager::GetNextActiveBody( TPhysicsObjectContainer* a_pBody, TINT a_eMotionType )
{
	T2DList<TPhysicsObjectContainer>* pList;

	if ( a_eMotionType == MotionType_Static )
		pList = &m_activeBodiesStatic;
	else if ( a_eMotionType == MotionType_Dynamic )
		pList = &m_activeBodiesDynamic;
	else
		pList = &m_activeBodiesKeyframe;

	auto itEnd = pList->End();

	if ( a_pBody == TNULL )
		return TNULL;

	T2DList<TPhysicsObjectContainer>::Iterator itNext = T2DList<TPhysicsObjectContainer>::Iterator( a_pBody ).Next();

	if ( itNext == itEnd )
		return TNULL;

	return itNext;
}

// $deBlob: FUNCTION 00726640
TINT TPhysicsManager::GetNumPhantomsInSimulation()
{
	TINT iCount = 0;

	for ( auto it = m_phantoms.Begin(); it != m_phantoms.End(); it++ )
	{
		if ( it->IsInSimulation() )
			iCount++;
	}

	return iCount;
}

// $deBlob: FUNCTION 005a64d0
void TPhysicsManager::SetConstraintHinge( TPhysicsConstraintContainer* a_pConstraint, const TVector4& a_rvPivot, const TVector4& a_rvAxisPoint )
{
	if ( !a_pConstraint->m_bInWorld )
	{
		a_pConstraint->m_eType   = TPhysicsConstraintContainer::TYPE_HINGE;
		a_pConstraint->m_vPivotA = a_rvPivot;

		// The axis points from the pivot to the second point
		a_pConstraint->m_vPivotB   = a_rvAxisPoint;
		a_pConstraint->m_vPivotB.x = a_pConstraint->m_vPivotB.x - a_pConstraint->m_vPivotA.x;
		a_pConstraint->m_vPivotB.y = a_pConstraint->m_vPivotB.y - a_pConstraint->m_vPivotA.y;
		a_pConstraint->m_vPivotB.z = a_pConstraint->m_vPivotB.z - a_pConstraint->m_vPivotA.z;
		a_pConstraint->m_vPivotB.Normalise();
	}
}

// $deBlob: FUNCTION 005a6530
void TPhysicsManager::SetConstraintLimitedHinge( TPhysicsConstraintContainer* a_pConstraint, const TVector4& a_rvPivot, const TVector4& a_rvAxisPoint, TFLOAT a_fMinAngle, TFLOAT a_fMaxAngle, TFLOAT a_fFriction )
{
	if ( !a_pConstraint->m_bInWorld )
	{
		a_pConstraint->m_eType   = TPhysicsConstraintContainer::TYPE_LIMITEDHINGE;
		a_pConstraint->m_vPivotA = a_rvPivot;

		// The axis points from the pivot to the second point
		a_pConstraint->m_vPivotB   = a_rvAxisPoint;
		a_pConstraint->m_vPivotB.x = a_pConstraint->m_vPivotB.x - a_pConstraint->m_vPivotA.x;
		a_pConstraint->m_vPivotB.y = a_pConstraint->m_vPivotB.y - a_pConstraint->m_vPivotA.y;
		a_pConstraint->m_vPivotB.z = a_pConstraint->m_vPivotB.z - a_pConstraint->m_vPivotA.z;
		a_pConstraint->m_vPivotB.Normalise();

		a_pConstraint->m_vLimits.y = a_fMinAngle;
		a_pConstraint->m_vLimits.z = a_fMaxAngle;
		a_pConstraint->m_vLimits.x = a_fFriction;
	}
}

// $deBlob: FUNCTION 005a6600
void TPhysicsManager::SetConstraintStiffSpring( TPhysicsConstraintContainer* a_pConstraint, const TVector4& a_rvPivotA, const TVector4& a_rvPivotB )
{
	if ( !a_pConstraint->m_bInWorld )
	{
		a_pConstraint->m_eType   = TPhysicsConstraintContainer::TYPE_STIFFSPRING;
		a_pConstraint->m_vPivotA = a_rvPivotA;
		a_pConstraint->m_vPivotB = a_rvPivotB;
	}
}

// $deBlob: FUNCTION 00725750
void TPhysicsManager::AddConstraint( TPhysicsConstraintContainer* a_pConstraint )
{
	if ( a_pConstraint->m_bInWorld )
		return;

	if ( a_pConstraint->m_pBodyA == TNULL || a_pConstraint->m_pBodyB == TNULL )
		return;

	if ( a_pConstraint->m_eType == TPhysicsConstraintContainer::TYPE_NONE )
		return;

	if ( !TPhysicsObjectContainer::IsInSimulation( a_pConstraint->m_pBodyA ) || !TPhysicsObjectContainer::IsInSimulation( a_pConstraint->m_pBodyB ) )
		return;

	const hkTransform& rTransformA = a_pConstraint->m_pBodyA->m_body->getTransform();
	const hkTransform& rTransformB = a_pConstraint->m_pBodyB->m_body->getTransform();

	const TVector4& rvPivotA = a_pConstraint->m_vPivotA;
	const TVector4& rvPivotB = a_pConstraint->m_vPivotB;

	switch ( a_pConstraint->m_eType )
	{
		case TPhysicsConstraintContainer::TYPE_BALLANDSOCKET:
		{
			hkpBallAndSocketConstraintData* pData = new hkpBallAndSocketConstraintData();
			pData->setInWorldSpace( rTransformA, rTransformB, hkVector4( rvPivotA.x, rvPivotA.y, rvPivotA.z, 0.0f ) );
			a_pConstraint->m_pData = pData;
			break;
		}
		case TPhysicsConstraintContainer::TYPE_STIFFSPRING:
		{
			hkpStiffSpringConstraintData* pData = new hkpStiffSpringConstraintData();
			pData->setInWorldSpace( rTransformA, rTransformB, hkVector4( rvPivotA.x, rvPivotA.y, rvPivotA.z, 0.0f ), hkVector4( rvPivotB.x, rvPivotB.y, rvPivotB.z, 0.0f ) );
			a_pConstraint->m_pData = pData;
			break;
		}
		case TPhysicsConstraintContainer::TYPE_HINGE:
		{
			hkpHingeConstraintData* pData = new hkpHingeConstraintData();
			pData->setInWorldSpace( rTransformA, rTransformB, hkVector4( rvPivotA.x, rvPivotA.y, rvPivotA.z, 0.0f ), hkVector4( rvPivotB.x, rvPivotB.y, rvPivotB.z, 0.0f ) );
			a_pConstraint->m_pData = pData;
			break;
		}
		case TPhysicsConstraintContainer::TYPE_LIMITEDHINGE:
		{
			hkpLimitedHingeConstraintData* pData = new hkpLimitedHingeConstraintData();
			pData->setInWorldSpace( rTransformA, rTransformB, hkVector4( rvPivotA.x, rvPivotA.y, rvPivotA.z, 0.0f ), hkVector4( rvPivotB.x, rvPivotB.y, rvPivotB.z, 0.0f ) );

			// NOTE: retail writes the values to the atoms of Havok 5.5's data (friction at 0xbc, angle limits at 0xc4
			// and 0xc8)
			pData->setMaxFrictionTorque( a_pConstraint->m_vLimits.x );
			pData->setMinAngularLimit( a_pConstraint->m_vLimits.y );
			pData->setMaxAngularLimit( a_pConstraint->m_vLimits.z );
			a_pConstraint->m_pData = pData;
			break;
		}
		case TPhysicsConstraintContainer::TYPE_PRISMATIC:
		{
			// NOTE: nothing in retail creates this type
			hkpPrismaticConstraintData* pData = new hkpPrismaticConstraintData();
			pData->setInWorldSpace( rTransformA, rTransformB, hkVector4( rvPivotA.x, rvPivotA.y, rvPivotA.z, 0.0f ), hkVector4( rvPivotB.x, rvPivotB.y, rvPivotB.z, 0.0f ) );

			// NOTE: retail writes the values to the atoms of Havok 5.5's data (friction at 0xb4, linear limits at 0xc8
			// and 0xcc)
			pData->setMaxFrictionForce( a_pConstraint->m_vLimits.x );
			pData->setMinLinearLimit( a_pConstraint->m_vLimits.y );
			pData->setMaxLinearLimit( a_pConstraint->m_vLimits.z );

			if ( ( a_pConstraint->m_uiFlags & TPhysicsConstraintContainer::PCCF_BREAKABLE ) == 0 )
			{
				// A dynamic body on a fixed one doesn't rotate (zero inverse inertia)
				// NOTE: retail calls the motion's setInertiaInvLocal (vtable +0x20) directly
				hkMatrix3 mZero;
				mZero.setZero();

				if ( a_pConstraint->m_pBodyA->m_eMotionType == MotionType_Static && a_pConstraint->m_pBodyB->m_eMotionType == MotionType_Dynamic )
					a_pConstraint->m_pBodyB->m_body->setInertiaInvLocal( mZero );
				else if ( a_pConstraint->m_pBodyB->m_eMotionType == MotionType_Static && a_pConstraint->m_pBodyA->m_eMotionType == MotionType_Dynamic )
					a_pConstraint->m_pBodyA->m_body->setInertiaInvLocal( mZero );
			}

			a_pConstraint->m_pData = pData;
			break;
		}
		case TPhysicsConstraintContainer::TYPE_WHEEL:
		{
			// NOTE: nothing in retail creates this type
			hkpWheelConstraintData* pData = new hkpWheelConstraintData();

			// The axle is m_vPivotB made perpendicular to the steering axis m_vSteeringAxis, the suspension axis is m_vSuspensionAxis
			TVector4 vSteering = a_pConstraint->m_vSteeringAxis;
			TVector4 vAxle     = a_pConstraint->m_vPivotB;
			TVector4 vCross;
			vCross.CrossProduct( vSteering, vAxle );
			vAxle.CrossProduct( vCross, vSteering );

			const TVector4& rvSuspension = a_pConstraint->m_vSuspensionAxis;

			pData->setInWorldSpace(
			    rTransformA,
			    rTransformB,
			    hkVector4( rvPivotA.x, rvPivotA.y, rvPivotA.z, 0.0f ),
			    hkVector4( vAxle.x, vAxle.y, vAxle.z, 0.0f ),
			    hkVector4( rvSuspension.x, rvSuspension.y, rvSuspension.z, 0.0f ),
			    hkVector4( vSteering.x, vSteering.y, vSteering.z, 0.0f )
			);

			// NOTE: retail writes the values to the atoms of Havok 5.5's data (the suspension limits at 0xa4 and 0xa8, its
			// strength and damping at 0xb0 and 0xb4), the same atoms as 6.5's setters
			pData->setSuspensionMinLimit( a_pConstraint->m_vLimits.y );
			pData->setSuspensionMaxLimit( a_pConstraint->m_vLimits.z );
			pData->setSuspensionStrength( a_pConstraint->m_vLimits.w );
			pData->setSuspensionDamping( a_pConstraint->m_vLimits.x );
			TASSERT( pData->isValid() );

			a_pConstraint->m_pData = pData;
			break;
		}
		default:
			TASSERT( TFALSE );
			break;
	}

	if ( a_pConstraint->m_uiFlags & TPhysicsConstraintContainer::PCCF_MALLEABLE )
	{
		hkpMalleableConstraintData* pMalleable = new hkpMalleableConstraintData( a_pConstraint->m_pData );
		pMalleable->setStrength( a_pConstraint->m_pBreakableData->m_fValue );
		a_pConstraint->m_pData->removeReference();
		a_pConstraint->m_pData = pMalleable;
	}
	else if ( a_pConstraint->m_uiFlags & TPhysicsConstraintContainer::PCCF_BREAKABLE )
	{
		hkpBreakableConstraintData* pBreakable = new hkpBreakableConstraintData( a_pConstraint->m_pData );
		pBreakable->setThreshold( a_pConstraint->m_pBreakableData->m_fValue );
		pBreakable->setRemoveWhenBroken( false );
		pBreakable->setRevertBackVelocityOnBreak( true );

		// NOTE: retail also stores the breakable listener (m_pBrokenConstraintListener) in the data (Havok 5.5's
		// hkpBreakableConstraintData::m_listener, +0x28); Havok 6.5 has no such member
		a_pConstraint->m_pData->removeReference();
		a_pConstraint->m_pData = pBreakable;

		// The Havok motion values of the bodies, restored by DispatchConstraintBrokenCallbacks when it breaks
		// NOTE: retail reads them from Havok 5.5's hkpRigidBody (+0x184, +0x188 and the hkUFloat8 at +0x18c)
		TPhysicsConstraintContainer::BreakData* pBreakData = a_pConstraint->m_pBreakableData;
		pBreakData->m_fLinearDampingA                      = a_pConstraint->m_pBodyA->m_body->getLinearDamping();
		pBreakData->m_fAngularDampingA                     = a_pConstraint->m_pBodyA->m_body->getAngularDamping();
		pBreakData->m_fMaxLinearVelocityA                  = a_pConstraint->m_pBodyA->m_body->getMaxLinearVelocity();
		pBreakData->m_fLinearDampingB                      = a_pConstraint->m_pBodyB->m_body->getLinearDamping();
		pBreakData->m_fAngularDampingB                     = a_pConstraint->m_pBodyB->m_body->getAngularDamping();
		pBreakData->m_fMaxLinearVelocityB                  = a_pConstraint->m_pBodyB->m_body->getMaxLinearVelocity();
	}

	hkpConstraintInstance* pInstance = new hkpConstraintInstance( a_pConstraint->m_pBodyA->m_body, a_pConstraint->m_pBodyB->m_body, a_pConstraint->m_pData, hkpConstraintInstance::PRIORITY_PSI );
	pInstance->setWantRuntime( a_pConstraint->m_bWantRuntime );
	pInstance->setUserData( hkUlong( a_pConstraint ) );
	a_pConstraint->m_pInstance = pInstance;

	m_apWorlds[ a_pConstraint->m_iWorld ]->addConstraint( pInstance );
	a_pConstraint->m_bInWorld = TTRUE;
}

// $deBlob: FUNCTION 00725650
void TPhysicsManager::DestroyConstraint( TPhysicsConstraintContainer* a_pConstraint )
{
	RemoveConstraint( a_pConstraint );
	m_pConstraints->FindAndErase( a_pConstraint );

	if ( a_pConstraint )
	{
		TFree( a_pConstraint->m_pBreakableData );
		delete a_pConstraint;
	}
}

// $deBlob: FUNCTION 00725f40
void TPhysicsManager::RemoveConstraint( TPhysicsConstraintContainer* a_pConstraint )
{
	if ( a_pConstraint->m_bInWorld )
	{
		a_pConstraint->m_bInWorld = TFALSE;
		m_apWorlds[ a_pConstraint->m_iWorld ]->removeConstraint( a_pConstraint->m_pInstance );
		a_pConstraint->m_pInstance->removeReference();
		a_pConstraint->m_pData->removeReference();
		a_pConstraint->m_pInstance = TNULL;
		a_pConstraint->m_pData     = TNULL;

		if ( a_pConstraint->m_pBreakableData )
		{
			TFree( a_pConstraint->m_pBreakableData );
			a_pConstraint->m_pBreakableData = TNULL;
		}
	}
}

// $deBlob: FUNCTION 00726c10
void TPhysicsManager::OnTriggerEnter( TPhysicsObjectContainer* a_pTrigger, TPhysicsObjectContainer* a_pOther )
{
	TASSERT( m_iNumTriggerCallbacks < m_initData.m_iMaxTriggerCallbacks );

	if ( m_iNumTriggerCallbacks < m_initData.m_iMaxTriggerCallbacks )
	{
		TPhysicsTriggerCallback& rCallback = m_pTriggerCallbacks[ m_iNumTriggerCallbacks ];
		rCallback.m_bEnter                 = TTRUE;
		rCallback.m_pTrigger               = a_pTrigger;
		rCallback.m_pOther                 = a_pOther;
		m_iNumTriggerCallbacks++;
		m_iMaxNumTriggerCallbacksUsed = TMath::Max( m_iMaxNumTriggerCallbacksUsed, m_iNumTriggerCallbacks );
	}
}

// $deBlob: FUNCTION 00726ca0
void TPhysicsManager::OnTriggerLeave( TPhysicsObjectContainer* a_pTrigger, TPhysicsObjectContainer* a_pOther )
{
	TASSERT( m_iNumTriggerCallbacks < m_initData.m_iMaxTriggerCallbacks );

	if ( m_iNumTriggerCallbacks < m_initData.m_iMaxTriggerCallbacks )
	{
		TPhysicsTriggerCallback& rCallback = m_pTriggerCallbacks[ m_iNumTriggerCallbacks ];
		rCallback.m_bEnter                 = TFALSE;
		rCallback.m_pTrigger               = a_pTrigger;
		rCallback.m_pOther                 = a_pOther;
		m_iNumTriggerCallbacks++;
		m_iMaxNumTriggerCallbacksUsed = TMath::Max( m_iMaxNumTriggerCallbacksUsed, m_iNumTriggerCallbacks );
	}
}

// $deBlob: FUNCTION 00726b80
void TPhysicsManager::OnConstraintBroken( hkpConstraintInstance* a_pConstraint, TFLOAT a_fForce )
{
	TASSERT( m_iNumConstraintBrokenCallbacks < m_initData.m_iMaxConstraintBrokenCallbacks && "TPhysicsManager: Physics manager has run out of constraint broken callbacks.\n" );

	if ( m_iNumConstraintBrokenCallbacks < m_initData.m_iMaxConstraintBrokenCallbacks )
	{
		TUINT32* pCallback          = TSTATICCAST( TUINT32, m_pConstraintBrokenCallbacks ) + m_iNumConstraintBrokenCallbacks * 2;
		pCallback[ 0 ]              = TUINT32( a_pConstraint );
		*(TFLOAT*)( pCallback + 1 ) = a_fForce;

		m_iNumConstraintBrokenCallbacks++;
		m_iMaxNumConstraintBrokenCallbacksUsed = TMath::Max( m_iMaxNumConstraintBrokenCallbacksUsed, m_iNumConstraintBrokenCallbacks );
	}
}

// $deBlob: FUNCTION 00730710
TVector4& TPhysicsManager::GetWorldCenterOfMass( TPhysicsObjectContainer* a_pPOC, TVector4& a_rvOutCenterOfMass )
{
	if ( a_pPOC )
	{
		TVector4 vCenterOfMass;
		TMatrix44::TransformVector( vCenterOfMass, *a_pPOC->m_pMatrix, a_pPOC->m_vCenterOfMass );
		a_rvOutCenterOfMass = vCenterOfMass;
	}
	else
	{
		a_rvOutCenterOfMass = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	}

	return a_rvOutCenterOfMass;
}

// $deBlob: FUNCTION 00730760
hkpMotion::MotionType TPhysicsManager::GetHavokMotionType( TPhysicsObjectContainer* a_pBody, TINT a_eMotionType )
{
	// NOTE: Havok 5.5's values (7 fixed, 6 keyframed, 2 sphere inertia, 1 dynamic) are the same in 6.5
	if ( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_FIXED ) || a_eMotionType == MotionType_Static )
		return hkpMotion::MOTION_FIXED;

	if ( a_eMotionType == MotionType_Keyframed || a_eMotionType == MotionType_KeyframedReporting )
		return hkpMotion::MOTION_KEYFRAMED;

	return ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_SPHERE_INERTIA ) ? hkpMotion::MOTION_SPHERE_INERTIA : hkpMotion::MOTION_DYNAMIC;
}

// $deBlob: FUNCTION 007307b0
hkpCollidableQualityType TPhysicsManager::GetHavokQualityType( TPhysicsObjectContainer* a_pBody, TINT a_eMotionType )
{
	// NOTE: retail returns Havok 5.5 numbers, whose enum starts with INVALID = 0 and has no DEBRIS_SIMPLE_TOI (5.5's
	// hkpRigidBody ctor treats 0 as unset and defaults to 1/2/3 for fixed/keyframed/other motions): mapped by name to
	// 6.5's enum (5.5: 1 FIXED, 2 KEYFRAMED, 3 DEBRIS, 4 MOVING, 5 CRITICAL, 9 KEYFRAMED_REPORTING)
	switch ( a_eMotionType )
	{
		case MotionType_Static: return HK_COLLIDABLE_QUALITY_FIXED;
		case MotionType_Keyframed: return HK_COLLIDABLE_QUALITY_KEYFRAMED;
		case MotionType_KeyframedReporting: return HK_COLLIDABLE_QUALITY_KEYFRAMED_REPORTING;
	}

	if ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_SPHERE_INERTIA )
		return HK_COLLIDABLE_QUALITY_DEBRIS;

	// retail: ( ( m_flags & PMF_UNK2000 ) | 0x8000 ) >> 13, i.e. 4 or 5
	return ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_UNK2000 ) ? HK_COLLIDABLE_QUALITY_CRITICAL : HK_COLLIDABLE_QUALITY_MOVING;
}

// $deBlob: FUNCTION 00731300
void TPhysicsManager::DeactivateBody( TPhysicsObjectContainer* a_pBody )
{
	TASSERT( a_pBody != TNULL );
	TASSERT( TPhysicsObjectContainer::IsInSimulation( a_pBody ) == TFALSE );

	a_pBody->Remove();
	a_pBody->InsertBefore( m_bodies.End() );
	a_pBody->m_flags &= ~( TPhysicsObjectContainer::PMF_IN_ADD_BATCH | TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH );
}

// $deBlob: FUNCTION 0072ff30
void TPhysicsManager::ActivateBody( TPhysicsObjectContainer* a_pBody )
{
	TASSERT( a_pBody != TNULL );
	TASSERT( TPhysicsObjectContainer::IsInSimulation( a_pBody ) == TTRUE );

	a_pBody->Remove();

	if ( a_pBody->m_eMotionType == MotionType_Static )
		a_pBody->InsertBefore( m_activeBodiesStatic.End() );
	else if ( a_pBody->m_eMotionType == MotionType_Dynamic )
		a_pBody->InsertBefore( m_activeBodiesDynamic.End() );
	else
		a_pBody->InsertBefore( m_activeBodiesKeyframe.End() );

	TASSERT( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_DELETE_PENDING ) == 0 );
	a_pBody->m_flags &= ~( TPhysicsObjectContainer::PMF_IN_ADD_BATCH | TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH | TPhysicsObjectContainer::PMF_DELETE_PENDING );
}

// $deBlob: FUNCTION 00730020
void TPhysicsManager::AddBody( TPhysicsObjectContainer* a_pBody )
{
	TASSERT( a_pBody != TNULL );
	TASSERT( TPhysicsObjectContainer::IsInSimulation( a_pBody ) == TFALSE );
	TASSERT( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_ADD_BATCH ) == 0 );

	a_pBody->Remove();
	a_pBody->InsertBefore( m_addedBodies.End() );

	TASSERT( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_DELETE_PENDING ) == 0 );
	a_pBody->m_flags = ( a_pBody->m_flags & ~( TPhysicsObjectContainer::PMF_IN_ADD_BATCH | TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH | TPhysicsObjectContainer::PMF_DELETE_PENDING ) ) | TPhysicsObjectContainer::PMF_IN_ADD_BATCH;
}

// $deBlob: FUNCTION 00730140
void TPhysicsManager::RemoveBody( TPhysicsObjectContainer* a_pBody )
{
	TASSERT( a_pBody != TNULL );
	TASSERT( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH ) == 0 );
	TASSERT( ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_DELETE_PENDING ) == 0 );

	a_pBody->Remove();
	a_pBody->InsertBefore( m_removedBodies.End() );
	a_pBody->m_flags = ( a_pBody->m_flags & ~( TPhysicsObjectContainer::PMF_IN_ADD_BATCH | TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH | TPhysicsObjectContainer::PMF_DELETE_PENDING ) ) | TPhysicsObjectContainer::PMF_IN_REMOVE_BATCH;
}

// $deBlob: FUNCTION 005808c0
void TPhysicsManager::StopBody( TPhysicsObjectContainer* a_pPOC )
{
	if ( a_pPOC == TNULL || !a_pPOC->IsInSimulation() )
		return;

	hkpRigidBody* pBody = a_pPOC->m_body;
	pBody->activate();
	pBody->setAngularVelocity( hkVector4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	pBody->activate();
	pBody->setLinearVelocity( hkVector4( 0.0f, 0.0f, 0.0f, 0.0f ) );
}

// $deBlob: FUNCTION 0058ca00
void TPhysicsManager::GetLinearVelocity( TPhysicsObjectContainer* a_pPOC, TVector4& a_rvOutVelocity ) const
{
	a_rvOutVelocity.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		// NOTE: retail reads the velocity from the body's motion (hkpRigidBody + 0x1A0 in Havok 5.5)
		const hkVector4& rvVelocity = a_pPOC->m_body->getLinearVelocity();
		a_rvOutVelocity.Set( rvVelocity( 0 ), rvVelocity( 1 ), rvVelocity( 2 ), 1.0f );
	}
}

// $deBlob: FUNCTION 0058cb40
TFLOAT TPhysicsManager::GetMaxLinearVelocity( TPhysicsObjectContainer* a_pPOC ) const
{
	TASSERT( a_pPOC );
	return a_pPOC->m_fMaxLinearVelocity;
}

// $deBlob: FUNCTION 005ac530
void TPhysicsManager::SetCenterOfMass( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcCenterOfMass )
{
	if ( a_pPOC == TNULL )
		return;

	a_pPOC->m_vCenterOfMass = a_rcCenterOfMass;

	if ( a_pPOC->m_body != TNULL )
		a_pPOC->m_body->setCenterOfMassLocal( hkVector4( a_rcCenterOfMass.x, a_rcCenterOfMass.y, a_rcCenterOfMass.z, 0.0f ) );
}

// $deBlob: FUNCTION 005ac5a0
void TPhysicsManager::SetLinearVelocity( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcVelocity )
{
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		// NOTE: retail compares with the motion's velocity (hkpRigidBody + 0x1A0 in Havok 5.5)
		hkpRigidBody*    pBody     = a_pPOC->m_body;
		const hkVector4& rvCurrent = pBody->getLinearVelocity();

		TBOOL bSameZ = TMath::Abs( a_rcVelocity.z - rvCurrent( 2 ) ) <= 0.001f;
		TBOOL bSameY = TMath::Abs( a_rcVelocity.y - rvCurrent( 1 ) ) <= 0.001f;
		TBOOL bSameX = TMath::Abs( a_rcVelocity.x - rvCurrent( 0 ) ) <= 0.001f;

		if ( !bSameZ || !bSameY || !bSameX )
		{
			pBody->activate();
			pBody->setLinearVelocity( hkVector4( a_rcVelocity.x, a_rcVelocity.y, a_rcVelocity.z, 0.0f ) );
		}
	}
}

// $deBlob: FUNCTION 005ac3d0
void TPhysicsManager::SetAngularVelocity( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcVelocity )
{
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		// NOTE: retail compares with the motion's velocity (hkpRigidBody + 0x1B0 in Havok 5.5)
		hkpRigidBody*    pBody     = a_pPOC->m_body;
		const hkVector4& rvCurrent = pBody->getAngularVelocity();

		TBOOL bSameZ = TMath::Abs( a_rcVelocity.z - rvCurrent( 2 ) ) <= 0.001f;
		TBOOL bSameY = TMath::Abs( a_rcVelocity.y - rvCurrent( 1 ) ) <= 0.001f;
		TBOOL bSameX = TMath::Abs( a_rcVelocity.x - rvCurrent( 0 ) ) <= 0.001f;

		if ( !bSameZ || !bSameY || !bSameX )
		{
			pBody->activate();
			pBody->setAngularVelocity( hkVector4( a_rcVelocity.x, a_rcVelocity.y, a_rcVelocity.z, 0.0f ) );
		}
	}
}

// $deBlob: FUNCTION 00726f50
hkpShape* TPhysicsManager::CreateShape( const TShapeBaseClass* a_pShapeData )
{
	// NOTE: retail allocates the shapes with hkThreadMemory (its 5.5 operator new); the same through 6.5's
	switch ( a_pShapeData->GetType() )
	{
		case TShapeBaseClass::Type_Sphere:
			return new hkpSphereShape( TSTATICCAST( const TSphereData, a_pShapeData )->GetRadius() );
		case TShapeBaseClass::Type_Box:
		{
			const TVector4& rcHalf = TSTATICCAST( const TBoxData, a_pShapeData )->GetHalfExtents();
			hkVector4       vHalfExtents( rcHalf.x, rcHalf.y, rcHalf.z, 0.0f );
			hkpBoxShape*    pShape  = new hkpBoxShape( vHalfExtents, 0.05f );
			TFLOAT          fRadius = pShape->getRadius();

			// Keep the outer size: shrink the box by its convex radius
			if ( 0.0f < fRadius )
			{
				vHalfExtents.set( rcHalf.x - fRadius, rcHalf.y - fRadius, rcHalf.z - fRadius, 0.0f );
				pShape->setHalfExtents( vHalfExtents );
			}

			return pShape;
		}
		case TShapeBaseClass::Type_Capsule:
		{
			const TCapsuleData* pCapsuleData = TSTATICCAST( const TCapsuleData, a_pShapeData );
			hkVector4           vA( 0.0f, 0.0f, pCapsuleData->GetHeight() * 0.5f, 0.0f );
			hkVector4           vB( 0.0f, 0.0f, pCapsuleData->GetHeight() * -0.5f, 0.0f );
			hkpCapsuleShape*    pShape  = new hkpCapsuleShape( vA, vB, pCapsuleData->GetRadius() );
			TFLOAT              fRadius = pShape->getRadius();

			if ( 0.0f < fRadius )
			{
				// NOTE: retail writes the shortened first vertex into both vertices (the capsule becomes a sphere)
				vA.set( 0.0f, 0.0f, pCapsuleData->GetHeight() * 0.5f - fRadius * 0.5f, fRadius );
				pShape->setVertex( 0, vA );
				pShape->setVertex( 1, vA );
			}

			return pShape;
		}
		case TShapeBaseClass::Type_Cylinder:
		{
			const TCylinderData* pCylinderData = TSTATICCAST( const TCylinderData, a_pShapeData );
			hkVector4            vA( 0.0f, 0.0f, pCylinderData->GetHeight() * 0.5f, 0.0f );
			hkVector4            vB( 0.0f, 0.0f, pCylinderData->GetHeight() * -0.5f, 0.0f );
			hkpCylinderShape*    pShape  = new hkpCylinderShape( vA, vB, pCylinderData->GetRadius(), 0.05f );
			TFLOAT               fRadius = pShape->getRadius();

			if ( 0.0f < fRadius )
			{
				// NOTE: retail writes the shortened first vertex into both vertices (as the capsule)
				vA.set( 0.0f, 0.0f, pCylinderData->GetHeight() * 0.5f - fRadius * 0.5f, 0.0f );
				pShape->setVertex( 0, vA );
				pShape->setVertex( 1, vA );
				pShape->setCylinderRadius( pCylinderData->GetRadius() - fRadius );
			}

			return pShape;
		}
		default:
			TASSERT( TFALSE, "TPhysicsManager::CreateShape(): This shape type is not supported!\n" );
			return TNULL;
	}
}

// $deBlob: FUNCTION 00725170
TPhysicsPhantomContainer* TPhysicsManager::CreateShapePhantom( const TShapeBaseClass* a_pShapeData )
{
	TPhysicsPhantomContainer* pContainer = TPhysicsPhantomContainer::Create();
	pContainer->InsertBefore( m_phantoms.End() );

	pContainer->m_uiWorld = TUINT16( m_iDefaultWorld );
	pContainer->SetType( ms_bCachingPhantoms ? TPhysicsPhantomContainer::TPhantomType_Caching : TPhysicsPhantomContainer::TPhantomType_Simple );
	pContainer->m_shapeType = TUINT16( a_pShapeData->GetType() );

	hkpShape*   pShape = CreateShape( a_pShapeData );
	hkTransform oTransform;
	oTransform.setIdentity();

	if ( ( pContainer->m_uiFlags & ( TPhysicsPhantomContainer::PPF_AABB | TPhysicsPhantomContainer::PPF_SIMPLE ) ) == 0 && ( pContainer->m_uiFlags & TPhysicsPhantomContainer::PPF_CACHING ) != 0 )
		pContainer->m_phantom = new hkpCachingShapePhantom( pShape, oTransform, 0 );
	else
		pContainer->m_phantom = new hkpSimpleShapePhantom( pShape, oTransform, 0 );

	pShape->removeReference();
	return pContainer;
}

// $deBlob: FUNCTION 007250d0
TPhysicsPhantomContainer* TPhysicsManager::CreateAabbPhantom()
{
	TPhysicsPhantomContainer* pContainer = TPhysicsPhantomContainer::Create();
	pContainer->InsertBefore( m_phantoms.End() );

	pContainer->m_uiWorld = TUINT16( m_iDefaultWorld );
	pContainer->SetType( TPhysicsPhantomContainer::TPhantomType_AABB );

	hkAabb oAabb;
	oAabb.m_min.set( 1.0f, 1.0f, 1.0f, 0.0f );
	oAabb.m_max.set( 1.0f, 1.0f, 1.0f, 0.0f );
	pContainer->m_phantom = new hkpAabbPhantom( oAabb, 0 );

	return pContainer;
}

// $deBlob: FUNCTION 00725310
void TPhysicsManager::DestroyPhantom( TPhysicsPhantomContainer* a_pContainer )
{
	RemovePhantomFromSimulation( a_pContainer, TNULL );

	a_pContainer->m_phantom->removeReference();
	a_pContainer->m_phantom = TNULL;
	a_pContainer->SetType( TPhysicsPhantomContainer::TPhantomType_AABB );
	a_pContainer->m_shapeType = 0;
	a_pContainer->Remove();

	TPhysicsPhantomContainer::Destroy( a_pContainer );
}

// $deBlob: FUNCTION 007253b0
void TPhysicsManager::AddPhantomToSimulation( TPhysicsPhantomContainer* a_pContainer, const TVector4& a_rcPosition, void* a_pUnused )
{
	TASSERT( a_pContainer, "phantomContainer" );

	TUINT32 uiFlags = a_pContainer->m_uiFlags;

	if ( uiFlags & TPhysicsPhantomContainer::PPF_IN_SIMULATION )
		return;

	if ( ( uiFlags & TPhysicsPhantomContainer::PPF_AABB ) == 0 && ( uiFlags & ( TPhysicsPhantomContainer::PPF_SIMPLE | TPhysicsPhantomContainer::PPF_CACHING ) ) != 0 )
	{
		hkTransform oTransform;
		oTransform.setIdentity();
		oTransform.getTranslation().set( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 0.0f );
		TSTATICCAST( hkpShapePhantom, a_pContainer->m_phantom )->setTransform( oTransform );
	}
	else
	{
		hkAabb oAabb;
		oAabb.m_min.set( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 0.0f );
		oAabb.m_max = oAabb.m_min;
		TSTATICCAST( hkpAabbPhantom, a_pContainer->m_phantom )->setAabb( oAabb );
	}

	m_apWorlds[ a_pContainer->m_uiWorld ]->addPhantom( a_pContainer->m_phantom );
	a_pContainer->m_uiFlags |= TPhysicsPhantomContainer::PPF_IN_SIMULATION;
	m_auiBatchStats[ 18 ]++;
}

// $deBlob: FUNCTION 00725530
void TPhysicsManager::RemovePhantomFromSimulation( TPhysicsPhantomContainer* a_pContainer, void* a_pUnused )
{
	TASSERT( a_pContainer, "phantomContainer" );

	if ( a_pContainer->m_uiFlags & TPhysicsPhantomContainer::PPF_IN_SIMULATION )
	{
		a_pContainer->m_uiFlags &= ~TPhysicsPhantomContainer::PPF_IN_SIMULATION;
		m_apWorlds[ a_pContainer->m_uiWorld ]->removePhantom( a_pContainer->m_phantom );
		m_auiBatchStats[ 19 ]++;
	}
}

void TPhysicsManager::SetPhantomCollisionFilterInfo( TPhysicsPhantomContainer* a_pContainer, TUINT32 a_uiFilterInfo )
{
	if ( a_pContainer == TNULL || a_pContainer->m_phantom == TNULL )
		return;

	hkpCollidable* pCollidable = a_pContainer->m_phantom->getCollidableRw();

	if ( pCollidable->getCollisionFilterInfo() == a_uiFilterInfo )
		return;

	pCollidable->setCollisionFilterInfo( a_uiFilterInfo );

	if ( a_pContainer->IsInSimulation() )
		m_apWorlds[ a_pContainer->m_uiWorld ]->updateCollisionFilterOnPhantom( a_pContainer->m_phantom, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
}

// $deBlob: FUNCTION 00561260
void TPhysicsManager::SetCollisionTestFlags( TPhysicsPhantomContainer* a_pContainer, TUINT32 a_uiTestFlags )
{
	GetSingleton()->SetPhantomCollisionFilterInfo( a_pContainer, ( a_uiTestFlags & 0xFFF ) * 0x1000 );
}

TUINT32 TPhysicsManager::GetCollisionTestFlags( TPhysicsPhantomContainer* a_pContainer ) const
{
	if ( a_pContainer == TNULL || a_pContainer->m_phantom == TNULL )
		return 0;

	return ( a_pContainer->m_phantom->getCollidable()->getCollisionFilterInfo() >> 12 ) & 0xFFF;
}

// $deBlob: FUNCTION 00561160
void TPhysicsManager::SetAabbPhantomBox( TPhysicsPhantomContainer* a_pContainer, const TVector4* a_pBox )
{
	if ( a_pContainer->GetType() != TPhysicsPhantomContainer::TPhantomType_AABB )
		return;

	TVector4 vHalfExtents;
	vHalfExtents.Multiply( a_pBox[ 1 ], 1.01f );

	hkAabb oAabb;
	oAabb.m_min.set( a_pBox[ 0 ].x - vHalfExtents.x, a_pBox[ 0 ].y - vHalfExtents.y, a_pBox[ 0 ].z - vHalfExtents.z, 0.0f );
	oAabb.m_max.set( a_pBox[ 0 ].x + vHalfExtents.x, a_pBox[ 0 ].y + vHalfExtents.y, a_pBox[ 0 ].z + vHalfExtents.z, 0.0f );
	TSTATICCAST( hkpAabbPhantom, a_pContainer->m_phantom )->setAabb( oAabb );
}

// $deBlob: FUNCTION 00727320
TBOOL TPhysicsManager::GetMeshContact( const hkpCollidable* a_pRootCollidable, TUINT32 a_uiKey, TPhysicsShapeContact& a_rContact )
{
	const hkpShape* pRootShape = a_pRootCollidable->getShape();

	// NOTE: retail checks Havok 5.5's MOPP type value (0xb)
	if ( a_uiKey == HK_INVALID_SHAPE_KEY || pRootShape->getType() != HK_SHAPE_MOPP )
		return TFALSE;

	const hkpMoppBvTreeShape* pTree = TSTATICCAST( const hkpMoppBvTreeShape, pRootShape );

	hkpShapeContainer::ShapeBuffer oBuffer;
	const hkpShape*                pChildShape = pTree->getContainer()->getChildShape( a_uiKey, oBuffer );

	if ( pChildShape == TNULL )
		return TFALSE;

	TASSERT( pChildShape->getType() == HK_SHAPE_TRIANGLE );
	const hkpTriangleShape* pTriangle = TSTATICCAST( const hkpTriangleShape, pChildShape );

	for ( TINT i = 0; i < 3; i++ )
	{
		const hkVector4& rcVertex = pTriangle->getVertex( i );
		a_rContact.m_avTriangle[ i ].Set( rcVertex( 0 ), rcVertex( 1 ), rcVertex( 2 ), 1.0f );
	}

	TPhysicsObjectContainer* pShapeBody = TREINTERPRETCAST( TPhysicsObjectContainer*, pRootShape->getUserData() );
	const TMatrix44*         pMatrix    = pShapeBody ? pShapeBody->m_pMatrix : TNULL;

	TMatrix44::TransformVector( a_rContact.m_avTriangle[ 0 ], *pMatrix, a_rContact.m_avTriangle[ 0 ] );
	TMatrix44::TransformVector( a_rContact.m_avTriangle[ 1 ], *pMatrix, a_rContact.m_avTriangle[ 1 ] );
	TMatrix44::TransformVector( a_rContact.m_avTriangle[ 2 ], *pMatrix, a_rContact.m_avTriangle[ 2 ] );

	const TVector4* pV = a_rContact.m_avTriangle;
	TVector4        vEdge12( pV[ 1 ].x - pV[ 2 ].x, pV[ 1 ].y - pV[ 2 ].y, pV[ 1 ].z - pV[ 2 ].z, pV[ 1 ].w );
	TVector4        vEdge01( pV[ 0 ].x - pV[ 1 ].x, pV[ 0 ].y - pV[ 1 ].y, pV[ 0 ].z - pV[ 1 ].z, pV[ 0 ].w );

	a_rContact.m_vSurfaceNormal.CrossProduct( vEdge01, vEdge12 );
	a_rContact.m_vSurfaceNormal.Normalise();

	// The winding of the triangle: a flag per triangle, or every other triangle of a strip is flipped
	TPhysicsMeshContainer* pMesh  = a_rContact.m_pBody ? a_rContact.m_pBody->m_pMeshContainer : TNULL;
	void*                  pStripInfo = pMesh->GetStripInfo();
	TBOOL                  bFlip  = TFALSE;

	if ( pStripInfo == TNULL )
	{
		if ( pMesh->GetPrimitiveType() == TPhysicsPrimitiveType_Strips )
			bFlip = ( a_uiKey & 1 ) != 0;
	}
	else if ( TUINT32( pMesh->GetNumStrips() ) <= 0x80 )
		bFlip = TSTATICCAST( TINT8, pStripInfo )[ a_uiKey ] >= 0;
	else
		bFlip = TSTATICCAST( TINT16, pStripInfo )[ a_uiKey ] >= 0;

	if ( bFlip )
	{
		a_rContact.m_vSurfaceNormal.x = -a_rContact.m_vSurfaceNormal.x;
		a_rContact.m_vSurfaceNormal.y = -a_rContact.m_vSurfaceNormal.y;
		a_rContact.m_vSurfaceNormal.z = -a_rContact.m_vSurfaceNormal.z;
	}

	// NOTE: retail reads Havok 5.5's subparts directly (see GetRootMeshShape)
	const hkpMeshShape* pMeshShape = TSTATICCAST( const hkpMeshShape, pTree->getShapeCollection() );

	if ( pMeshShape->getSubPart( a_uiKey ).m_materialIndexStriding != 0 )
	{
		const TPhysicsMeshMaterial* pMaterial = TREINTERPRETCAST( const TPhysicsMeshMaterial*, pMeshShape->getMeshMaterial( a_uiKey ) );

		if ( pMaterial )
		{
			a_rContact.m_uiMaterial = pMaterial->m_uiSurface;
			return TTRUE;
		}
	}

	return TFALSE;
}

static TBOOL IsInsideBox( const TVector4& a_rcPoint, const TVector4& a_rcMin, const TVector4& a_rcMax )
{
	return a_rcPoint.x <= a_rcMax.x && a_rcPoint.y <= a_rcMax.y && a_rcPoint.z <= a_rcMax.z &&
	    a_rcMin.x <= a_rcPoint.x && a_rcMin.y <= a_rcPoint.y && a_rcMin.z <= a_rcPoint.z;
}

// $deBlob: FUNCTION 0072a3f0
void TPhysicsManager::CheckRayPhantom( TPhysicsPhantomContainer* phantomContainer, const TPhysicsRay& a_rcRay )
{
	TASSERT( phantomContainer->GetType() == TPhysicsPhantomContainer::TPhantomType_AABB );
	TASSERT( phantomContainer->IsInSimulation() );

	hkpPhantom* pPhantom = phantomContainer->m_phantom;

	if ( pPhantom->getCollidable()->getCollisionFilterInfo() != ( ( a_rcRay.m_uiCollisionLayer & 0xFFF ) << 0xC ) )
		TASSERT( TFALSE, "TPhysicsManager - Collision filter of this query helper does not match the ray cast!\n" );

	const TVector4& rcFrom = a_rcRay.m_vFrom;
	const TVector4& rcTo   = a_rcRay.m_vTo;

	TFLOAT fMinX = TMath::Min( rcTo.x, rcFrom.x );
	TFLOAT fMinY = TMath::Min( rcTo.y, rcFrom.y );
	TFLOAT fMinZ = TMath::Min( rcTo.z, rcFrom.z );
	TFLOAT fMaxX = TMath::Max( rcTo.x, rcFrom.x );
	TFLOAT fMaxY = TMath::Max( rcTo.y, rcFrom.y );
	TFLOAT fMaxZ = TMath::Max( rcTo.z, rcFrom.z );

	hkAabb oAabb;
	pPhantom->calcAabb( oAabb );

	// The ray's box has to be inside the phantom's (x, y and z)
	TBOOL bInside = fMinX >= oAabb.m_min( 0 ) && fMinY >= oAabb.m_min( 1 ) && fMinZ >= oAabb.m_min( 2 ) &&
	    oAabb.m_max( 0 ) >= fMaxX && oAabb.m_max( 1 ) >= fMaxY && oAabb.m_max( 2 ) >= fMaxZ;

	if ( !bInside )
		TASSERT( TFALSE, "TPhysicsManager - The AABB of this query helper is too small for the ray cast!\n" );
}

// $deBlob: FUNCTION 0072a620
void TPhysicsManager::CheckShapePhantom( TPhysicsPhantomContainer* phantomContainer, const TShapeBaseClass& a_shape, TUINT32 a_uiCollisionLayer )
{
	TASSERT( phantomContainer->GetType() == TPhysicsPhantomContainer::TPhantomType_Simple || phantomContainer->GetType() == TPhysicsPhantomContainer::TPhantomType_Caching );
	TASSERT( phantomContainer->m_shapeType == a_shape.GetType() );
	TASSERT( phantomContainer->IsInSimulation() );

	if ( phantomContainer->m_phantom->getCollidable()->getCollisionFilterInfo() != ( ( a_uiCollisionLayer & 0xFFF ) << 0xC ) )
		TASSERT( TFALSE, "TPhysicsManager - Collision filter of this query helper does not match the shape cast!\n" );
}

// $deBlob: FUNCTION 0072a730
TBOOL TPhysicsManager::IsRayInWorld( const TPhysicsRay& a_rcRay )
{
	return IsInsideBox( a_rcRay.m_vFrom, m_initData.m_vBroadPhaseMin, m_initData.m_vBroadPhaseMax ) && IsInsideBox( a_rcRay.m_vTo, m_initData.m_vBroadPhaseMin, m_initData.m_vBroadPhaseMax );
}

// $deBlob: FUNCTION 0072a7c0
TBOOL TPhysicsManager::IsShapeInWorld( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix )
{
	return IsInsideBox( a_rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), m_initData.m_vBroadPhaseMin, m_initData.m_vBroadPhaseMax );
}

// $deBlob: FUNCTION 0072a820
TBOOL TPhysicsManager::IsSweptShapeInWorld( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, const TVector4& a_rcTo )
{
	return IsInsideBox( a_rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), m_initData.m_vBroadPhaseMin, m_initData.m_vBroadPhaseMax ) && IsInsideBox( a_rcTo, m_initData.m_vBroadPhaseMin, m_initData.m_vBroadPhaseMax );
}

// $deBlob: FUNCTION 00728740
TBOOL TPhysicsManager::CollideRay( const TPhysicsRay& a_rcRay, TPhysicsShapeContact& a_rOutContact, TPhysicsPhantomContainer* a_pPhantom )
{
	const TVector4& rcFrom = a_rcRay.m_vFrom;
	const TVector4& rcTo   = a_rcRay.m_vTo;

	if ( !IsInsideBox( rcFrom, m_initData.m_vBroadPhaseMin, m_initData.m_vBroadPhaseMax ) || !IsInsideBox( rcTo, m_initData.m_vBroadPhaseMin, m_initData.m_vBroadPhaseMax ) )
	{
		TASSERT( TFALSE, "TPhysicsManager::CollideRay - ray cast is either wholely or partially outside the world, returning no collision.\n" );
		return TFALSE;
	}

	TVector4 vRay( rcTo.x - rcFrom.x, rcTo.y - rcFrom.y, rcTo.z - rcFrom.z, rcTo.w );

	if ( vRay.Magnitude() < 0.0001f )
		return TFALSE;

	a_rOutContact.m_vPosition = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	a_rOutContact.m_vNormal   = TVector4::VEC_NEGY;
	a_rOutContact.m_pBody     = TNULL;

	hkpWorldRayCastInput oInput;
	oInput.m_from.set( rcFrom.x, rcFrom.y, rcFrom.z, 0.0f );
	oInput.m_to.set( rcTo.x, rcTo.y, rcTo.z, 0.0f );
	oInput.m_enableShapeCollectionFilter = TTRUE;
	oInput.m_filterInfo                  = ( a_rcRay.m_uiCollisionLayer & 0xFFF ) << 0xC;

	hkpWorldRayCastOutput oOutput;

	if ( a_pPhantom == TNULL || !ms_bRayPhantoms || !a_pPhantom->IsInSimulation() || a_pPhantom->GetType() != TPhysicsPhantomContainer::TPhantomType_AABB )
		m_apWorlds[ m_iDefaultWorld ]->castRay( oInput, oOutput );
	else
		TSTATICCAST( hkpAabbPhantom, a_pPhantom->m_phantom )->castRay( oInput, oOutput );

	TBOOL bHit = oOutput.m_rootCollidable != HK_NULL;

	if ( !bHit )
		return TFALSE;

	a_rOutContact.m_vPosition.Lerp4( rcFrom, rcTo, oOutput.m_hitFraction );
	a_rOutContact.m_vNormal.Set( oOutput.m_normal( 0 ), oOutput.m_normal( 1 ), oOutput.m_normal( 2 ), 1.0f );
	a_rOutContact.m_fDistance      = oOutput.m_hitFraction;
	a_rOutContact.m_pBody          = TREINTERPRETCAST( TPhysicsObjectContainer*, oOutput.m_rootCollidable->getShape()->getUserData() );
	a_rOutContact.m_vSurfaceNormal = a_rOutContact.m_vNormal;

	// The key of the leaf shape (NOTE: retail doesn't stop at MAX_HIERARCHY_DEPTH)
	hkpShapeKey uiKey = oOutput.m_shapeKeys[ 0 ];

	for ( TINT i = 1; i < hkpShapeRayCastOutput::MAX_HIERARCHY_DEPTH && uiKey != HK_INVALID_SHAPE_KEY && oOutput.m_shapeKeys[ i ] != HK_INVALID_SHAPE_KEY; i++ )
		uiKey = oOutput.m_shapeKeys[ i ];

	if ( GetMeshContact( oOutput.m_rootCollidable, uiKey, a_rOutContact ) )
		a_rOutContact.m_vNormal = a_rOutContact.m_vSurfaceNormal;
	else
		a_rOutContact.m_uiMaterial = GetDefaultMaterial( a_rOutContact.m_pBody );

	return bHit;
}

// $deBlob: FUNCTION 00728ae0
TINT TPhysicsManager::CollideRayAll( const TPhysicsRay& a_rcRay, TPhysicsShapeContact* a_pOutContacts, TINT a_iMaxContacts, TPhysicsPhantomContainer* a_pPhantom )
{
	const TVector4& rcFrom = a_rcRay.m_vFrom;
	const TVector4& rcTo   = a_rcRay.m_vTo;

	if ( !IsInsideBox( rcFrom, m_initData.m_vBroadPhaseMin, m_initData.m_vBroadPhaseMax ) || !IsInsideBox( rcTo, m_initData.m_vBroadPhaseMin, m_initData.m_vBroadPhaseMax ) )
	{
		TASSERT( TFALSE, "TPhysicsManager::CollideRay - ray cast is either wholely or partially outside the world, returning no collision.\n" );
		return 0;
	}

	TVector4 vRay( rcTo.x - rcFrom.x, rcTo.y - rcFrom.y, rcTo.z - rcFrom.z, rcTo.w );

	if ( vRay.NormaliseMag() < 0.0001f )
		return 0;

	for ( TINT i = 0; i < a_iMaxContacts; i++ )
	{
		a_pOutContacts[ i ].m_vPosition = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		a_pOutContacts[ i ].m_vNormal   = TVector4::VEC_NEGY;
		a_pOutContacts[ i ].m_pBody     = TNULL;
	}

	hkpWorldRayCastInput oInput;
	oInput.m_from.set( rcFrom.x, rcFrom.y, rcFrom.z, 0.0f );
	oInput.m_to.set( rcTo.x, rcTo.y, rcTo.z, 0.0f );
	oInput.m_enableShapeCollectionFilter = TTRUE;
	oInput.m_filterInfo                  = ( a_rcRay.m_uiCollisionLayer & 0xFFF ) << 0xC;

	hkpAllRayHitCollector oCollector;

	if ( a_pPhantom == TNULL || !ms_bRayPhantoms || !a_pPhantom->IsInSimulation() || a_pPhantom->GetType() != TPhysicsPhantomContainer::TPhantomType_AABB )
		m_apWorlds[ m_iDefaultWorld ]->castRay( oInput, oCollector );
	else
		TSTATICCAST( hkpAabbPhantom, a_pPhantom->m_phantom )->castRay( oInput, oCollector );

	TINT iNumHits = oCollector.getHits().getSize();

	if ( 0 < iNumHits )
	{
		oCollector.sortHits();

		for ( TINT i = 0; i < iNumHits; i++ )
		{
			if ( i >= a_iMaxContacts )
				continue;

			const hkpWorldRayCastOutput& rcHit    = oCollector.getHits()[ i ];
			TPhysicsShapeContact&        rContact = a_pOutContacts[ i ];

			rContact.m_vPosition.Lerp4( rcFrom, rcTo, rcHit.m_hitFraction );
			rContact.m_vNormal.Set( rcHit.m_normal( 0 ), rcHit.m_normal( 1 ), rcHit.m_normal( 2 ), 1.0f );
			rContact.m_pBody          = TREINTERPRETCAST( TPhysicsObjectContainer*, rcHit.m_rootCollidable->getShape()->getUserData() );
			rContact.m_fDistance      = rcHit.m_hitFraction;
			rContact.m_vSurfaceNormal = rContact.m_vNormal;

			// The key of the leaf shape
			hkpShapeKey uiKey = rcHit.m_shapeKeys[ 0 ];

			for ( TINT j = 1; uiKey != HK_INVALID_SHAPE_KEY && rcHit.m_shapeKeys[ j ] != HK_INVALID_SHAPE_KEY; j++ )
				uiKey = rcHit.m_shapeKeys[ j ];

			if ( GetMeshContact( rcHit.m_rootCollidable, uiKey, rContact ) )
				rContact.m_vNormal = rContact.m_vSurfaceNormal;
			else
				rContact.m_uiMaterial = GetDefaultMaterial( rContact.m_pBody );
		}
	}

	if ( iNumHits > a_iMaxContacts )
		iNumHits = a_iMaxContacts;

	return iNumHits;
}

// $deBlob: FUNCTION 00728f80
TBOOL TPhysicsManager::CollideShapeClosest( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, TUINT32 a_uiCollisionLayer, TPhysicsShapeContact& a_rOutContact, TPhysicsPhantomContainer* a_pPhantom )
{
	hkpShape* pShape = CreateShape( a_pShapeData );

	if ( pShape == TNULL )
		return TFALSE;

	const TVector4& rcPosition = a_rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	if ( m_initData.m_vBroadPhaseMax.x < rcPosition.x || m_initData.m_vBroadPhaseMax.y < rcPosition.y || m_initData.m_vBroadPhaseMax.z < rcPosition.z ||
	     rcPosition.x < m_initData.m_vBroadPhaseMin.x || rcPosition.y < m_initData.m_vBroadPhaseMin.y || rcPosition.z < m_initData.m_vBroadPhaseMin.z )
	{
		TASSERT( TFALSE, "TPhysicsManager::CollideShape - shape is either wholely or partially outside the world, returning no collision.\n" );

#ifdef DEBLOB_COMMUNITY_PATCH
		pShape->removeReference();
#else
		// NOTE: retail leaks the shape here
#endif
		return TFALSE;
	}

	hkTransform oTransform;
	oTransform.set4x4ColumnMajor( &a_rcMatrix.m_f11 );

	hkpClosestCdPointCollector oCollector;
	hkpWorld*                  pWorld = m_apWorlds[ m_iDefaultWorld ];

	hkpCollisionInput oInput = static_cast<const hkpCollisionInput&>( *pWorld->getCollisionInput() );
	oInput.setTolerance( 0.001f );

	if ( a_pPhantom == TNULL || !ms_bShapePhantoms || !a_pPhantom->IsInSimulation() || a_pPhantom->GetType() == TPhysicsPhantomContainer::TPhantomType_AABB )
	{
		// NOTE: retail builds the collidable on the stack (filter info at 0x1c, allowed penetration depth -1)
		hkpCollidable oCollidable( pShape, &oTransform );
		oCollidable.setCollisionFilterInfo( ( a_uiCollisionLayer & 0xFFF ) << 0xC );
		pWorld->getClosestPoints( &oCollidable, oInput, oCollector );
	}
	else
	{
		// The phantom's own shape, moved to the query
		hkpShapePhantom* pPhantom = TSTATICCAST( hkpShapePhantom, a_pPhantom->m_phantom );

		if ( !pPhantom->getTransform().isApproximatelyEqual( oTransform, 0.001f ) )
			pPhantom->setTransform( oTransform );

		pPhantom->getClosestPoints( oCollector, pWorld->getCollisionInput() );
	}

	TBOOL bHit = TFALSE;

	if ( oCollector.hasHit() )
	{
		const hkpRootCdPoint& rcHit    = oCollector.getHit();
		const hkVector4&      rvPos    = rcHit.m_contact.getPosition();
		const hkVector4&      rvNormal = rcHit.m_contact.getSeparatingNormal();

		a_rOutContact.m_vPosition.Set( rvPos( 0 ), rvPos( 1 ), rvPos( 2 ), 1.0f );
		a_rOutContact.m_vNormal.Set( rvNormal( 0 ), rvNormal( 1 ), rvNormal( 2 ), 1.0f );
		a_rOutContact.m_fDistance      = rcHit.m_contact.getDistance();
		a_rOutContact.m_pBody          = TREINTERPRETCAST( TPhysicsObjectContainer*, rcHit.m_rootCollidableB->getShape()->getUserData() );
		a_rOutContact.m_vSurfaceNormal = a_rOutContact.m_vNormal;

		if ( !GetMeshContact( rcHit.m_rootCollidableB, rcHit.m_shapeKeyB, a_rOutContact ) )
			a_rOutContact.m_uiMaterial = GetDefaultMaterial( a_rOutContact.m_pBody );

		bHit = TTRUE;
	}

	pShape->removeReference();
	return bHit;
}

// $deBlob: FUNCTION 00729290
TINT TPhysicsManager::CollideShape( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, TUINT32 a_uiCollisionLayer, TPhysicsShapeContact* a_pOutContacts, TINT a_iMaxContacts, TPhysicsPhantomContainer* a_pPhantom )
{
	hkpShape* pShape = CreateShape( a_pShapeData );

	if ( pShape == TNULL )
		return 0;

	const TVector4& rcPosition = a_rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	if ( m_initData.m_vBroadPhaseMax.x < rcPosition.x || m_initData.m_vBroadPhaseMax.y < rcPosition.y || m_initData.m_vBroadPhaseMax.z < rcPosition.z ||
	     rcPosition.x < m_initData.m_vBroadPhaseMin.x || rcPosition.y < m_initData.m_vBroadPhaseMin.y || rcPosition.z < m_initData.m_vBroadPhaseMin.z )
	{
		TASSERT( TFALSE, "TPhysicsManager::CollideShape - shape is either wholely or partially outside the world, returning no collision.\n" );

#ifdef DEBLOB_COMMUNITY_PATCH
		pShape->removeReference();
#else
		// NOTE: retail leaks the shape here
#endif
		return 0;
	}

	hkTransform oTransform;
	oTransform.set4x4ColumnMajor( &a_rcMatrix.m_f11 );

	hkpAllCdPointCollector oCollector;
	hkpWorld*              pWorld = m_apWorlds[ m_iDefaultWorld ];

	hkpCollisionInput oInput = static_cast<const hkpCollisionInput&>( *pWorld->getCollisionInput() );
	oInput.setTolerance( 0.001f );

	if ( a_pPhantom == TNULL || !ms_bShapePhantoms || !a_pPhantom->IsInSimulation() || a_pPhantom->GetType() == TPhysicsPhantomContainer::TPhantomType_AABB )
	{
		// NOTE: retail builds the collidable on the stack (filter info at 0x1c, allowed penetration depth -1)
		hkpCollidable oCollidable( pShape, &oTransform );
		oCollidable.setCollisionFilterInfo( ( a_uiCollisionLayer & 0xFFF ) << 0xC );
		pWorld->getClosestPoints( &oCollidable, oInput, oCollector );
	}
	else
	{
		// The phantom's own shape, moved to the query
		hkpShapePhantom* pPhantom = TSTATICCAST( hkpShapePhantom, a_pPhantom->m_phantom );

		if ( !pPhantom->getTransform().isApproximatelyEqual( oTransform, 0.001f ) )
			pPhantom->setTransform( oTransform );

		pPhantom->getClosestPoints( oCollector, pWorld->getCollisionInput() );
	}

	TINT iNumHits = oCollector.getHits().getSize();

	if ( 0 < iNumHits )
	{
		oCollector.sortHits();

		for ( TINT i = 0; i < iNumHits; i++ )
		{
			if ( i < a_iMaxContacts )
			{
				const hkpRootCdPoint& rcHit    = oCollector.getHits()[ i ];
				TPhysicsShapeContact& rContact = a_pOutContacts[ i ];
				const hkVector4&      rvPos    = rcHit.m_contact.getPosition();
				const hkVector4&      rvNormal = rcHit.m_contact.getSeparatingNormal();

				rContact.m_vPosition.Set( rvPos( 0 ), rvPos( 1 ), rvPos( 2 ), 1.0f );
				rContact.m_vNormal.Set( rvNormal( 0 ), rvNormal( 1 ), rvNormal( 2 ), 1.0f );
				rContact.m_pBody          = TREINTERPRETCAST( TPhysicsObjectContainer*, rcHit.m_rootCollidableB->getShape()->getUserData() );
				rContact.m_fDistance      = rcHit.m_contact.getDistance();
				rContact.m_vSurfaceNormal = rContact.m_vNormal;

				if ( !GetMeshContact( rcHit.m_rootCollidableB, rcHit.m_shapeKeyB, rContact ) )
					rContact.m_uiMaterial = GetDefaultMaterial( rContact.m_pBody );
			}
		}
	}

	pShape->removeReference();

	// The number of contacts written
	return TMath::Min( iNumHits, a_iMaxContacts );
}

// $deBlob: FUNCTION 007296a0
TBOOL TPhysicsManager::IntersectShape( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, TUINT32 a_uiCollisionLayer, TPhysicsObjectContainer** a_ppOutBody, TPhysicsPhantomContainer* a_pPhantom )
{
	TBOOL     bHit   = TFALSE;
	hkpShape* pShape = CreateShape( a_pShapeData );

	if ( pShape == TNULL )
		return TFALSE;

	const TVector4& rcPosition = a_rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	if ( m_initData.m_vBroadPhaseMax.x < rcPosition.x || m_initData.m_vBroadPhaseMax.y < rcPosition.y || m_initData.m_vBroadPhaseMax.z < rcPosition.z ||
	     rcPosition.x < m_initData.m_vBroadPhaseMin.x || rcPosition.y < m_initData.m_vBroadPhaseMin.y || rcPosition.z < m_initData.m_vBroadPhaseMin.z )
	{
		TASSERT( TFALSE, "TPhysicsManager::IntersectShape - shape is either wholely or partially outside the world, returning no collision.\n" );

#ifdef DEBLOB_COMMUNITY_PATCH
		pShape->removeReference();
#else
		// NOTE: retail leaks the shape here
#endif
		return TFALSE;
	}

	hkTransform oTransform;
	oTransform.set4x4ColumnMajor( &a_rcMatrix.m_f11 );

	hkpFirstCdBodyPairCollector oCollector;
	hkpWorld*                   pWorld = m_apWorlds[ m_iDefaultWorld ];

	if ( a_pPhantom == TNULL || !ms_bShapePhantoms || !a_pPhantom->IsInSimulation() || a_pPhantom->GetType() == TPhysicsPhantomContainer::TPhantomType_AABB )
	{
		// NOTE: retail builds the collidable on the stack (filter info at 0x1c, allowed penetration depth -1)
		hkpCollidable oCollidable( pShape, &oTransform );
		oCollidable.setCollisionFilterInfo( ( a_uiCollisionLayer & 0xFFF ) << 0xC );
		pWorld->getPenetrations( &oCollidable, *pWorld->getCollisionInput(), oCollector );
	}
	else
	{
		// The phantom's own shape, moved to the query
		hkpShapePhantom* pPhantom = TSTATICCAST( hkpShapePhantom, a_pPhantom->m_phantom );

		if ( !pPhantom->getTransform().isApproximatelyEqual( oTransform, 0.001f ) )
			pPhantom->setTransform( oTransform );

		pPhantom->getPenetrations( oCollector, pWorld->getCollisionInput() );
	}

	if ( oCollector.hasHit() )
	{
		bHit         = TTRUE;
		*a_ppOutBody = TREINTERPRETCAST( TPhysicsObjectContainer*, oCollector.getHit().m_rootCollidableB->getShape()->getUserData() );
	}

	pShape->removeReference();
	return bHit;
}

// $deBlob: FUNCTION 007298e0
TINT TPhysicsManager::IntersectShapeAll( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, TUINT32 a_uiCollisionLayer, TPhysicsObjectContainer** a_ppOutBodies, TINT a_iMaxBodies, TPhysicsPhantomContainer* a_pPhantom )
{
	hkpShape* pShape = CreateShape( a_pShapeData );

	if ( pShape == TNULL )
		return 0;

	const TVector4& rcPosition = a_rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	if ( m_initData.m_vBroadPhaseMax.x < rcPosition.x || m_initData.m_vBroadPhaseMax.y < rcPosition.y || m_initData.m_vBroadPhaseMax.z < rcPosition.z ||
	     rcPosition.x < m_initData.m_vBroadPhaseMin.x || rcPosition.y < m_initData.m_vBroadPhaseMin.y || rcPosition.z < m_initData.m_vBroadPhaseMin.z )
	{
		TASSERT( TFALSE, "TPhysicsManager::IntersectShape - shape is either wholely or partially outside the world, returning no collision.\n" );

#ifdef DEBLOB_COMMUNITY_PATCH
		pShape->removeReference();
#else
		// NOTE: retail leaks the shape here
#endif
		return 0;
	}

	hkTransform oTransform;
	oTransform.set4x4ColumnMajor( &a_rcMatrix.m_f11 );

	hkpAllCdBodyPairCollector oCollector;
	hkpWorld*                 pWorld = m_apWorlds[ m_iDefaultWorld ];

	if ( a_pPhantom == TNULL || !ms_bShapePhantoms || !a_pPhantom->IsInSimulation() || a_pPhantom->GetType() == TPhysicsPhantomContainer::TPhantomType_AABB )
	{
		// NOTE: retail builds the collidable on the stack (filter info at 0x1c, allowed penetration depth -1)
		hkpCollidable oCollidable( pShape, &oTransform );
		oCollidable.setCollisionFilterInfo( ( a_uiCollisionLayer & 0xFFF ) << 0xC );
		pWorld->getPenetrations( &oCollidable, *pWorld->getCollisionInput(), oCollector );
	}
	else
	{
		// The phantom's own shape, moved to the query
		hkpShapePhantom* pPhantom = TSTATICCAST( hkpShapePhantom, a_pPhantom->m_phantom );

		if ( !pPhantom->getTransform().isApproximatelyEqual( oTransform, 0.001f ) )
			pPhantom->setTransform( oTransform );

		pPhantom->getPenetrations( oCollector, pWorld->getCollisionInput() );
	}

	TINT iNumHits = oCollector.getHits().getSize();

	for ( TINT i = 0; i < iNumHits; i++ )
	{
		if ( i < a_iMaxBodies )
			a_ppOutBodies[ i ] = TREINTERPRETCAST( TPhysicsObjectContainer*, oCollector.getHits()[ i ].m_rootCollidableB->getShape()->getUserData() );
	}

	pShape->removeReference();

	if ( iNumHits > a_iMaxBodies )
		iNumHits = a_iMaxBodies;

	return iNumHits;
}

// $deBlob: FUNCTION 00729bd0
TBOOL TPhysicsManager::SweepShapeClosest( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, const TVector4& a_rcTo, TUINT32 a_uiCollisionLayer, TPhysicsShapeContact& a_rOutContact, TPhysicsPhantomContainer* a_pPhantom )
{
	hkpShape* pShape = CreateShape( a_pShapeData );

	if ( pShape == TNULL )
		return TFALSE;

	const TVector4& rcPosition = a_rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	if ( m_initData.m_vBroadPhaseMax.x < rcPosition.x || m_initData.m_vBroadPhaseMax.y < rcPosition.y || m_initData.m_vBroadPhaseMax.z < rcPosition.z ||
	     rcPosition.x < m_initData.m_vBroadPhaseMin.x || rcPosition.y < m_initData.m_vBroadPhaseMin.y || rcPosition.z < m_initData.m_vBroadPhaseMin.z ||
	     m_initData.m_vBroadPhaseMax.x < a_rcTo.x || m_initData.m_vBroadPhaseMax.y < a_rcTo.y || m_initData.m_vBroadPhaseMax.z < a_rcTo.z ||
	     a_rcTo.x < m_initData.m_vBroadPhaseMin.x || a_rcTo.y < m_initData.m_vBroadPhaseMin.y || a_rcTo.z < m_initData.m_vBroadPhaseMin.z )
	{
		TASSERT( TFALSE, "TPhysicsManager::SweepShape - swept shape is either wholely or partially outside the world, returning no collision.\n" );

#ifdef DEBLOB_COMMUNITY_PATCH
		pShape->removeReference();
#else
		// NOTE: retail leaks the shape here
#endif
		return TFALSE;
	}

	hkTransform oTransform;
	oTransform.set4x4ColumnMajor( &a_rcMatrix.m_f11 );

	hkpClosestCdPointCollector oCollector;
	hkpWorld*                  pWorld = m_apWorlds[ m_iDefaultWorld ];

	hkpLinearCastInput oInput;
	oInput.m_to.set( a_rcTo.x, a_rcTo.y, a_rcTo.z, 0.0f );
	oInput.m_maxExtraPenetration = HK_REAL_EPSILON;
	oInput.m_startPointTolerance = HK_REAL_EPSILON;

	if ( a_pPhantom == TNULL || !ms_bShapePhantoms || !a_pPhantom->IsInSimulation() || a_pPhantom->GetType() == TPhysicsPhantomContainer::TPhantomType_AABB )
	{
		// NOTE: retail builds the collidable on the stack (filter info at 0x1c, allowed penetration depth -1)
		hkpCollidable oCollidable( pShape, &oTransform );
		oCollidable.setCollisionFilterInfo( ( a_uiCollisionLayer & 0xFFF ) << 0xC );
		pWorld->linearCast( &oCollidable, oInput, oCollector, HK_NULL );
	}
	else
	{
		// NOTE: retail (Havok 5.5) moves the phantom when it isn't there yet (within 0.001), then casts it (vtable
		// +0x34); Havok 6.5's shape phantoms only cast with setTransformAndLinearCast
		hkpShapePhantom* pPhantom = TSTATICCAST( hkpShapePhantom, a_pPhantom->m_phantom );
		pPhantom->setTransformAndLinearCast( oTransform, oInput, oCollector, HK_NULL );
	}

	TBOOL bHit = TFALSE;

	if ( oCollector.hasHit() )
	{
		const hkpRootCdPoint& rcHit    = oCollector.getHit();
		const hkVector4&      rvPos    = rcHit.m_contact.getPosition();
		const hkVector4&      rvNormal = rcHit.m_contact.getSeparatingNormal();

		a_rOutContact.m_vPosition.Set( rvPos( 0 ), rvPos( 1 ), rvPos( 2 ), 1.0f );
		a_rOutContact.m_vNormal.Set( rvNormal( 0 ), rvNormal( 1 ), rvNormal( 2 ), 1.0f );
		a_rOutContact.m_fDistance      = rcHit.m_contact.getDistance();
		a_rOutContact.m_pBody          = TREINTERPRETCAST( TPhysicsObjectContainer*, rcHit.m_rootCollidableB->getShape()->getUserData() );
		a_rOutContact.m_vSurfaceNormal = a_rOutContact.m_vNormal;

		if ( !GetMeshContact( rcHit.m_rootCollidableB, rcHit.m_shapeKeyB, a_rOutContact ) )
			a_rOutContact.m_uiMaterial = GetDefaultMaterial( a_rOutContact.m_pBody );

		bHit = TTRUE;
	}

	pShape->removeReference();
	return bHit;
}

// $deBlob: FUNCTION 00729f70
TINT TPhysicsManager::SweepShape( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, const TVector4& a_rcTo, TUINT32 a_uiCollisionLayer, TPhysicsShapeContact* a_pOutContacts, TINT a_iMaxContacts, TPhysicsPhantomContainer* a_pPhantom )
{
	hkpShape* pShape = CreateShape( a_pShapeData );

	if ( pShape == TNULL )
		return 0;

	const TVector4& rcPosition = a_rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	if ( m_initData.m_vBroadPhaseMax.x < rcPosition.x || m_initData.m_vBroadPhaseMax.y < rcPosition.y || m_initData.m_vBroadPhaseMax.z < rcPosition.z ||
	     rcPosition.x < m_initData.m_vBroadPhaseMin.x || rcPosition.y < m_initData.m_vBroadPhaseMin.y || rcPosition.z < m_initData.m_vBroadPhaseMin.z ||
	     m_initData.m_vBroadPhaseMax.x < a_rcTo.x || m_initData.m_vBroadPhaseMax.y < a_rcTo.y || m_initData.m_vBroadPhaseMax.z < a_rcTo.z ||
	     a_rcTo.x < m_initData.m_vBroadPhaseMin.x || a_rcTo.y < m_initData.m_vBroadPhaseMin.y || a_rcTo.z < m_initData.m_vBroadPhaseMin.z )
	{
		TASSERT( TFALSE, "TPhysicsManager::SweepShape - swept shape is either wholely or partially outside the world, returning no collision.\n" );

#ifdef DEBLOB_COMMUNITY_PATCH
		pShape->removeReference();
#else
		// NOTE: retail leaks the shape here
#endif
		return 0;
	}

	hkTransform oTransform;
	oTransform.set4x4ColumnMajor( &a_rcMatrix.m_f11 );

	hkpAllCdPointCollector oCollector;
	hkpWorld*              pWorld = m_apWorlds[ m_iDefaultWorld ];

	hkpLinearCastInput oInput;
	oInput.m_to.set( a_rcTo.x, a_rcTo.y, a_rcTo.z, 0.0f );
	oInput.m_maxExtraPenetration = HK_REAL_EPSILON;
	oInput.m_startPointTolerance = HK_REAL_EPSILON;

	if ( a_pPhantom == TNULL || !ms_bShapePhantoms || !a_pPhantom->IsInSimulation() || a_pPhantom->GetType() == TPhysicsPhantomContainer::TPhantomType_AABB )
	{
		// NOTE: retail builds the collidable on the stack (filter info at 0x1c, allowed penetration depth -1)
		hkpCollidable oCollidable( pShape, &oTransform );
		oCollidable.setCollisionFilterInfo( ( a_uiCollisionLayer & 0xFFF ) << 0xC );
		pWorld->linearCast( &oCollidable, oInput, oCollector, HK_NULL );
	}
	else
	{
		// The phantom's own shape, moved to the start of the sweep
		hkpShapePhantom* pPhantom = TSTATICCAST( hkpShapePhantom, a_pPhantom->m_phantom );

		// NOTE: retail (Havok 5.5) moves the phantom when it isn't there yet, then casts it (vtable +0x34); Havok 6.5's
		// shape phantoms only cast with setTransformAndLinearCast
		pPhantom->setTransformAndLinearCast( oTransform, oInput, oCollector, HK_NULL );
	}

	TINT iNumHits = oCollector.getHits().getSize();

	if ( 0 < iNumHits )
	{
		oCollector.sortHits();

		for ( TINT i = 0; i < iNumHits; i++ )
		{
			if ( i < a_iMaxContacts )
			{
				const hkpRootCdPoint& rcHit    = oCollector.getHits()[ i ];
				TPhysicsShapeContact& rContact = a_pOutContacts[ i ];
				const hkVector4&      rvPos    = rcHit.m_contact.getPosition();
				const hkVector4&      rvNormal = rcHit.m_contact.getSeparatingNormal();

				rContact.m_vPosition.Set( rvPos( 0 ), rvPos( 1 ), rvPos( 2 ), 1.0f );
				rContact.m_vNormal.Set( rvNormal( 0 ), rvNormal( 1 ), rvNormal( 2 ), 1.0f );
				rContact.m_pBody          = TREINTERPRETCAST( TPhysicsObjectContainer*, rcHit.m_rootCollidableB->getShape()->getUserData() );
				rContact.m_fDistance      = rcHit.m_contact.getDistance();
				rContact.m_vSurfaceNormal = rContact.m_vNormal;

				if ( !GetMeshContact( rcHit.m_rootCollidableB, rcHit.m_shapeKeyB, rContact ) )
					rContact.m_uiMaterial = GetDefaultMaterial( rContact.m_pBody );
			}
		}
	}

	pShape->removeReference();

	return TMath::Min( iNumHits, a_iMaxContacts );
}

// $deBlob: FUNCTION 00751630
void TPhysicsManager::GetAngularVelocity( TPhysicsObjectContainer* a_pPOC, TVector4& a_rvOutVelocity ) const
{
	a_rvOutVelocity.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		// NOTE: retail reads the velocity from the body's motion (hkpRigidBody + 0x1B0 in Havok 5.5)
		const hkVector4& rvVelocity = a_pPOC->m_body->getAngularVelocity();
		a_rvOutVelocity.Set( rvVelocity( 0 ), rvVelocity( 1 ), rvVelocity( 2 ), 1.0f );
	}
}

// $deBlob: FUNCTION 007248f0
void TPhysicsManager::ApplyForce( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcForce )
{
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		hkpRigidBody* pBody = a_pPOC->m_body;
		pBody->activate();
		pBody->applyForce( m_initData.m_fTimeStep, hkVector4( a_rcForce.x, a_rcForce.y, a_rcForce.z, 0.0f ) );

		if ( a_pPOC->m_pListener != TNULL )
		{
			// NOTE: retail reads the centre of mass from the body's motion (hkpRigidBody + 0x130 in Havok 5.5)
			const hkVector4& rvCentre = pBody->getCenterOfMassInWorld();
			TVector4         vCentre( rvCentre( 0 ), rvCentre( 1 ), rvCentre( 2 ), 1.0f );
			a_pPOC->m_pListener->OnForceApplied( a_pPOC, const_cast<TVector4*>( &a_rcForce ), &vCentre );
		}
	}
}

// $deBlob: FUNCTION 00724a10
void TPhysicsManager::ApplyPointForce( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcPoint, const TVector4& a_rcForce )
{
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		hkpRigidBody* pBody = a_pPOC->m_body;
		pBody->activate();
		pBody->applyForce( m_initData.m_fTimeStep, hkVector4( a_rcForce.x, a_rcForce.y, a_rcForce.z, 0.0f ), hkVector4( a_rcPoint.x, a_rcPoint.y, a_rcPoint.z, 0.0f ) );

		if ( a_pPOC->m_pListener != TNULL )
			a_pPOC->m_pListener->OnForceApplied( a_pPOC, const_cast<TVector4*>( &a_rcForce ), const_cast<TVector4*>( &a_rcPoint ) );
	}
}

// $deBlob: FUNCTION 00724b30
void TPhysicsManager::ApplyImpulse( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcImpulse )
{
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		TVector4 vForce;
		vForce.Multiply( a_rcImpulse, 1.0f / m_initData.m_fTimeStep );
		ApplyForce( a_pPOC, vForce );
	}
}

// $deBlob: FUNCTION 00724bc0
void TPhysicsManager::ApplyPointImpulse( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcPoint, const TVector4& a_rcImpulse )
{
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		TVector4 vForce;
		vForce.Multiply( a_rcImpulse, 1.0f / m_initData.m_fTimeStep );

		// NOTE: retail computes the force but passes the impulse itself
		ApplyPointForce( a_pPOC, a_rcPoint, a_rcImpulse );
	}
}

// $deBlob: FUNCTION 00724c50
void TPhysicsManager::ApplyExplosion( TPhysicsObjectContainer* a_pIgnore, const TVector4& a_rcCentre, TFLOAT a_fStrength, TFLOAT a_fRadius )
{
	for ( auto it = m_activeBodiesDynamic.Begin(); it != m_activeBodiesDynamic.End(); it++ )
	{
		TPhysicsObjectContainer* pBody = it;

		if ( pBody != a_pIgnore && pBody->m_eMotionType == MotionType_Dynamic )
		{
			TVector4 vPosition = pBody->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );
			TVector4 vImpulse( vPosition.x - a_rcCentre.x, vPosition.y - a_rcCentre.y, vPosition.z - a_rcCentre.z, vPosition.w );
			TFLOAT   fDistance = vImpulse.Magnitude();

			if ( fDistance > 0.0001f && a_fRadius > fDistance )
			{
				vImpulse.Multiply( 1.0f / fDistance );
				vImpulse.Multiply( ( 1.0f - fDistance / a_fRadius ) * a_fStrength );
				ApplyPointImpulse( pBody, vPosition, vImpulse );
			}
		}
	}
}

// $deBlob: FUNCTION 00724dc0
void TPhysicsManager::ApplyForceAtPoint( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcPoint, const TVector4& a_rcAcceleration )
{
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		TVector4 vForce;
		vForce.Multiply( a_rcAcceleration, a_pPOC->m_fMass );

		hkpRigidBody* pBody = a_pPOC->m_body;
		pBody->activate();
		pBody->applyForce( m_initData.m_fTimeStep, hkVector4( vForce.x, vForce.y, vForce.z, 0.0f ), hkVector4( a_rcPoint.x, a_rcPoint.y, a_rcPoint.z, 0.0f ) );

		if ( a_pPOC->m_pListener != TNULL )
			a_pPOC->m_pListener->OnAccelerationApplied( a_pPOC, const_cast<TVector4*>( &a_rcAcceleration ), const_cast<TVector4*>( &a_rcPoint ) );
	}
}

// $deBlob: FUNCTION 00724ef0
void TPhysicsManager::ApplyTorque( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcTorque )
{
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		hkpRigidBody* pBody = a_pPOC->m_body;
		pBody->activate();
		pBody->applyTorque( m_initData.m_fTimeStep, hkVector4( a_rcTorque.x, a_rcTorque.y, a_rcTorque.z, 0.0f ) );
	}
}

// Moves what touches the body as if its surface moved at a_rcVelocity (conveyor belts): creates the body's
// hkpSurfaceVelocityUtil on the first call, then updates it
// $deBlob: FUNCTION 00724fc0
void TPhysicsManager::SetSurfaceVelocity( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcVelocity )
{
	TASSERT( a_pPOC );

	if ( a_pPOC == TNULL )
		return;

	if ( a_pPOC->IsInSimulation() )
	{
		hkVector4 vVelocity( a_rcVelocity.x, a_rcVelocity.y, a_rcVelocity.z, 0.0f );

		if ( a_pPOC->m_pSurfaceVelocityUtil != TNULL )
		{
			a_pPOC->m_pSurfaceVelocityUtil->setSurfaceVelocity( vVelocity );
			return;
		}

		a_pPOC->m_pSurfaceVelocityUtil = new hkpSurfaceVelocityUtil( a_pPOC->m_body, vVelocity );
		a_pPOC->m_pSurfaceVelocityUtil->removeReference();
	}
}

// $deBlob: FUNCTION 005a80b0
void TPhysicsManager::SetTransform( TPhysicsObjectContainer* a_pPOC, const TMatrix44& a_rcMatrix, TBOOL a_bNoActivate )
{
	*a_pPOC->m_pMatrix = a_rcMatrix;
	UpdateBodyTransform( a_pPOC, a_bNoActivate );
}

// $deBlob: FUNCTION 007244f0
void TPhysicsManager::ApplyHardKeyFrame( TPhysicsObjectContainer* a_pPOC, const TMatrix44& a_rcTarget, TFLOAT a_fTimeStep )
{
	// Only the keyframed bodies
	if ( a_pPOC && ( a_pPOC->m_eMotionType == MotionType_Dynamic || a_pPOC->m_eMotionType == MotionType_Static ) )
		return;

	TFLOAT fInvTimeStep = 1.0f / a_fTimeStep;

	// Linear velocity: from the current to the target position of the centre of mass
	TVector4 vTarget = a_pPOC ? a_pPOC->m_vCenterOfMass : TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	TMatrix44::TransformVector( vTarget, a_rcTarget, vTarget );

	TVector4 vCurrent( 0.0f, 0.0f, 0.0f, 1.0f );

	if ( a_pPOC )
		TMatrix44::TransformVector( vCurrent, *a_pPOC->m_pMatrix, a_pPOC->m_vCenterOfMass );

	TVector4 vVelocity( vTarget.x - vCurrent.x, vTarget.y - vCurrent.y, vTarget.z - vCurrent.z, vTarget.w );
	vVelocity.Multiply( fInvTimeStep );
	SetLinearVelocity( a_pPOC, vVelocity );

	// Angular velocity: the rotation from the current to the target orientation
	// NOTE: retail reads the matrix of a null a_pPOC here
	TMatrix44 mCurrentInv = *a_pPOC->m_pMatrix;
	mCurrentInv.SetTranslation( TVector4( 0.0f, 0.0f, 0.0f, 1.0f ) );
	mCurrentInv.InvertOrthonormal();

	TMatrix44 mTarget = a_rcTarget;
	mTarget.SetTranslation( TVector4( 0.0f, 0.0f, 0.0f, 1.0f ) );

	TMatrix44 mDiff;
	mDiff.Multiply( mTarget, mCurrentInv );

	TQuaternion qDiff;
	TMatrix44::MatToQuat( qDiff, mDiff );
	qDiff /= qDiff.Magnitude();

	TFLOAT fAngle = TMath::ACos( TMath::Abs( qDiff.w ) );
	fAngle        = fAngle + fAngle;

	TVector4 vAngular;

	if ( fAngle >= 0.001f )
	{
		TASSERT( qDiff.AsVector4().AsVector3().Magnitude() > TMath::FLOATEPSILON, "TPhysicsManager: Cannot extract axis from a Quaternion representing (within numerical tolerance) the Identity rotation (or Quaternion may not be normalized)." );

		TVector4 vAxis( qDiff.x, qDiff.y, qDiff.z, 1.0f );
		vAxis.Normalise();

		if ( qDiff.w < 0.0f )
		{
			vAxis.x = -vAxis.x;
			vAxis.y = -vAxis.y;
			vAxis.z = -vAxis.z;
		}

		vAngular.Multiply( vAxis, fAngle * fInvTimeStep );
	}
	else
	{
		vAngular.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	}

	SetAngularVelocity( a_pPOC, vAngular );
}

// $deBlob: FUNCTION 00724490
void TPhysicsManager::UpdateBodyTransform( TPhysicsObjectContainer* a_pBody, TBOOL a_bNoActivate )
{
	if ( a_pBody->m_body != TNULL )
	{
		hkTransform oTransform;
		oTransform.set4x4ColumnMajor( &a_pBody->m_pMatrix->m_f11 );
		oTransform.getColumn( 0 )( 3 ) = 0.0f;
		oTransform.getColumn( 1 )( 3 ) = 0.0f;
		oTransform.getColumn( 2 )( 3 ) = 0.0f;
		oTransform.getColumn( 3 )( 3 ) = 1.0f;

		a_pBody->m_body->setTransform( oTransform );

		if ( !a_bNoActivate && ( a_pBody->m_flags & TPhysicsObjectContainer::PMF_IN_SIMULATION ) )
			a_pBody->m_body->activate();
	}
}

TOSHI_NAMESPACE_END

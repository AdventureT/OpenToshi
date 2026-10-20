#include "pch.h"
#include "AEnemyManager.h"
#include "AEnemyGrappler.h"
#include "AEnemyBehaviour.h"
#include "AEnemySchedule.h"
#include "Entities/AEntityData.h"
#include "Entities/AEntityFactory.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AGndMotorComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/PathFinding/AAINavigator.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "Entities/AEnemyWrapper/AEnemyWrapper.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "AEnemyTarget.h"
#include "Entities/Projectiles/AProjectileManager.h"
#include "Console/AConsoleState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AEnemyManager ) == 0x100 );

// 0x009b7c78
// $deBlob: FUNCTION 00540c80
// $deBlob: FUNCTION 00859da0
static AConsoleVar s_cvEnemyShowUpdates( "enemy_show_updates", TFALSE );
// Creates an enemy of the named class near the player (CreateDebugEnemy), 0x009b7ca4
// $deBlob: FUNCTION 00540c60
// $deBlob: FUNCTION 00859d90
static AConsoleVar s_cvEnemyCreate( "enemy_create", "" );

// $deBlob: FUNCTION 007897f0
AEnemyManager::AEnemyManager()
{
	m_iResortTimer      = 0;
	m_bCanUpdate        = TFALSE;
	m_pEnvironmentEnemy = TNULL;
	m_iEnvironmentTimer = 0;
	m_iEnemyListTimer   = 0;
	m_bPaused           = TFALSE;
	m_pUpdateNodes      = TNULL;
	m_pEnemiesData      = TNULL;
	m_pEnemiesData      = AEntityData::GetSingleton()->GetEntityData( "Enemies_data_EntityData" );

	AEnemySchedule::InitialiseStatic();
	AEnemyBehaviour::InitialiseStatic();

	AEnemyTarget::CreateSlotPool( 200 );
	AConsoleState::RegisterCVar( &s_cvEnemyShowUpdates );
	AConsoleState::RegisterCVar( &s_cvEnemyCreate );
}

// $deBlob: FUNCTION 00789960
// $deBlob: FUNCTION 0078da00
AEnemyManager::~AEnemyManager()
{
	AConsoleState::UnregisterCVar( &s_cvEnemyShowUpdates );
	AConsoleState::UnregisterCVar( &s_cvEnemyCreate );
	AEnemyTarget::DestroySlotPool();

	AEnemyGrappler::ClearPlayers();

	AEnemyBehaviour::DeinitialiseStatic();

	m_pEnemiesData = TNULL;
}

// $deBlob: FUNCTION 00789a80
void AEnemyManager::InitEnemyPool( EnemyType a_eType, TINT a_iCount )
{
	// No Inky in the free paint levels
	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint && ( a_eType == ENEMYTYPE_InkyBaton || a_eType == ENEMYTYPE_InkyBatonElite || a_eType == ENEMYTYPE_InkyPen ) )
		return;

	TASSERT( m_FreeList[ a_eType ].IsEmpty() );

	AEntityFactory* pFactory    = AEntityFactory::GetSingleton();
	const TCHAR*    szClassName = GetEnemyClassName( a_eType );

	for ( TINT i = 0; i < a_iCount; i++ )
	{
		AEnemyListNode* pNode = new AEnemyListNode();

		ABaseEntity* pEntity = pFactory->CreateEntity( szClassName );
		TASSERT( ( pEntity == TNULL ) || pEntity->IsA( &TGetClass( AEnemy ) ) );
		pNode->m_pEnemy = TSTATICCAST( AEnemy, pEntity );

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: temporary scaffolding, the factory returns TNULL for the unported enemy classes (retail crashes in
		// CreateEntity first)
		if ( pNode->m_pEnemy == TNULL )
		{
			delete pNode;
			continue;
		}
#endif

		pNode->m_pEnemy->SetHash( TFALSE );
		pFactory->RemoveEntity( *pNode->m_pEnemy );
		m_FreeList[ a_eType ].PushBack( pNode );

		if ( i == 0 )
			InitProjectilePool( pNode->m_pEnemy, a_iCount );
	}
}

// The static data of the enemy schedules and behaviours
// $deBlob: FUNCTION 0078aba0
void AEnemyManager::InitialiseSchedules()
{
	AEnemySchedule::InitialiseStatic();
}

// $deBlob: FUNCTION 0078abb0
void AEnemyManager::InitialiseBehaviours()
{
	AEnemyBehaviour::InitialiseStatic();
}

// $deBlob: FUNCTION 0078abd0
void AEnemyManager::DeinitialiseBehaviours()
{
	AEnemyBehaviour::DeinitialiseStatic();
}

// $deBlob: FUNCTION 0078abe0
void AEnemyManager::InitProjectilePool( AEnemy* a_pEnemy, TINT a_iCount )
{
	// NOTE: the projectile types are in the data of the derived classes (0xDC and 0x110)
	const TUINT8* pData = TREINTERPRETCAST( const TUINT8*, a_pEnemy->GetTypeData()->m_pTRBData );

	if ( a_pEnemy->m_uiEnemyFlags & AEnemy::EFLAG_OneProjectilePool )
	{
		AProjectileManager::GetSingleton()->InitPool( *TREINTERPRETCAST( const AProjectileTypes::PROJECTILETYPES*, pData + 0xDC ), a_iCount );
		return;
	}

	if ( a_pEnemy->m_uiEnemyFlags & AEnemy::EFLAG_TwoProjectilePools )
	{
		AProjectileManager::GetSingleton()->InitPool( *TREINTERPRETCAST( const AProjectileTypes::PROJECTILETYPES*, pData + 0xDC ), a_iCount );
		AProjectileManager::GetSingleton()->InitPool( *TREINTERPRETCAST( const AProjectileTypes::PROJECTILETYPES*, pData + 0x110 ), a_iCount );
	}
}

// $deBlob: FUNCTION 00789c00
void AEnemyManager::InitEnemyUpdatePool( TINT a_iNumEnemies )
{
	TINT iCount    = a_iNumEnemies * UpdateType_Count;
	m_pUpdateNodes = new AEnemyUpdateNode[ iCount ];

	for ( TINT i = 0; i < iCount; i++ )
		m_FreeUpdateNodes.PushBack( &m_pUpdateNodes[ i ] );
}

// $deBlob: FUNCTION 00789cf0
void AEnemyManager::InitEnemyComponents()
{
	static TCONSTEXPR_VAR AEnemy::ComponentType s_aTypes[ AEnemy::ComponentType_Count ] = {
		AEnemy::ComponentType_Model,
		AEnemy::ComponentType_Pose,
		AEnemy::ComponentType_FXBreakpoint,
		AEnemy::ComponentType_State,
		AEnemy::ComponentType_Motor,
		AEnemy::ComponentType_Pathfinding,
		AEnemy::ComponentType_Behaviour,
		AEnemy::ComponentType_Schedule,
	};

	for ( TINT i = 0; i < AEnemy::ComponentType_Count; i++ )
	{
		AEnemy::ComponentType eType = s_aTypes[ i ];

		for ( TINT j = 0; j < ENEMYTYPE_Count; j++ )
		{
			for ( auto it = m_FreeList[ j ].Begin(); it != m_FreeList[ j ].End(); it++ )
			{
				AEntityComponent2* pComponent = TSTATICCAST( AEntityComponent2, it->m_pEnemy->CreateComponent( eType ) );

				if ( pComponent )
					m_aComponentManagers[ eType ].Activate( pComponent, TFALSE );
			}
		}
	}
}

// $deBlob: FUNCTION 00789f50
void AEnemyManager::DestroyPools()
{

	for ( TINT i = 0; i < ENEMYTYPE_Count; i++ )
		m_ActiveList[ i ].DeleteAll();

	for ( TINT i = 0; i < ENEMYTYPE_Count; i++ )
	{
		for ( auto it = m_FreeList[ i ].Begin(); it != m_FreeList[ i ].End(); it++ )
		{
			AEnemy* pEnemy = it->m_pEnemy;
			pEnemy->Destroy();
			delete pEnemy;
		}

		m_FreeList[ i ].DeleteAll();
	}

	for ( TINT i = 0; i < UpdateType_Count; i++ )
		m_aUpdateLists[ i ]->Clear();

	m_FreeUpdateNodes.Clear();

	if ( m_pUpdateNodes )
		delete[] m_pUpdateNodes;

	m_Targets.DeleteAll();

	for ( TINT i = 0; i < AEnemy::ComponentType_Count; i++ )
		m_aComponentManagers[ i ].Destroy();
}

// $deBlob: FUNCTION 0078a210
AEnemy* AEnemyManager::CreateEnemy( EnemyType a_eType, const TVector4& a_rvPosition, const TVector4& a_rvFacing, AEnemyWrapper* a_pWrapper, TINT8 a_iPickupType, TINT16 a_iNumPickups, TBOOL a_bUnk7, ABaseEntity* a_pSpawner, TINT a_iCustomData )
{
	AEnemy* pEnemy;

	if ( m_FreeList[ a_eType ].IsEmpty() )
	{
		pEnemy = FindSuicider( a_eType );
	}
	else
	{
		AEnemyListNode* pNode = m_FreeList[ a_eType ].PopBack();
		AEntityFactory::GetSingleton()->ActivateEntity( *pNode->m_pEnemy );
		m_ActiveList[ a_eType ].PushBack( pNode );
		pEnemy = pNode->m_pEnemy;
	}

	if ( pEnemy == TNULL )
		return TNULL;

	for ( TINT i = 0; i < UpdateType_Count; i++ )
	{
		if ( !m_FreeUpdateNodes.IsEmpty() )
		{
			AEnemyUpdateNode* pUpdateNode = m_FreeUpdateNodes.PopBack();
			pUpdateNode->m_pEnemy         = pEnemy;
			pUpdateNode->m_fTime          = 0.0f;
			m_aUpdateLists[ i ].Push( pUpdateNode );
		}
	}

	pEnemy->SetPooledID();
	pEnemy->SetCustomData( a_iCustomData );
	pEnemy->SetPosition( a_rvPosition );
	pEnemy->m_vSpawnPosition = a_rvPosition;
	pEnemy->SetFacing( a_rvFacing );

	for ( TINT i = 0; i < AEnemy::ComponentType_Count; i++ )
	{
		AEntityComponent2* pComponent = TSTATICCAST( AEntityComponent2, pEnemy->GetComponent( AEnemy::ComponentType( i ) ) );

		if ( pComponent )
			m_aComponentManagers[ i ].Activate( pComponent, TTRUE );
	}

	TSTATICCAST( AEntityComponent2, pEnemy->GetMotorComponent() )->ResetComponent();
	TSTATICCAST( AGndMotorComponent, pEnemy->GetMotorComponent() )->SetFacing( a_rvFacing );

	pEnemy->m_uiPickupType = TUINT8( a_iPickupType );
	pEnemy->m_iNumPickups  = a_iNumPickups;
	pEnemy->m_pSquad       = a_pSpawner;
	pEnemy->SetPoolOwner( TREINTERPRETCAST( TINT, a_pWrapper ) );
	pEnemy->SetActive( TTRUE, a_bUnk7 );
	pEnemy->SyncBodyTransform();

	return pEnemy;
}

// $deBlob: FUNCTION 00789e50
void AEnemyManager::InitEnemies()
{
	// The pooled enemies were removed from the factory before the loader's PostCreate pass
	for ( TINT i = 0; i < ENEMYTYPE_Count; i++ )
	{
		for ( auto it = m_FreeList[ i ].Begin(); it != m_FreeList[ i ].End(); it++ )
		{
			AEnemy* pEnemy = it->m_pEnemy;
			pEnemy->PostCreate();
			pEnemy->SetActive( TFALSE, TFALSE );
		}
	}
}

// $deBlob: FUNCTION 0078a480
void AEnemyManager::ReleaseEnemy( AEnemy* a_pEnemy )
{
	AEnemyListNode* pNode = FindEnemyNode( a_pEnemy );

	if ( pNode == TNULL )
		return;

	pNode->Remove();
	TSTATICCAST( AEnemyBehaviour, a_pEnemy->GetBehaviourComponent() )->SetSelectedEnemy( TNULL );
	a_pEnemy->SetActive( TFALSE, TFALSE );

	for ( TINT i = 0; i < UpdateType_Count; i++ )
		RemoveFromUpdateList( m_aUpdateLists[ i ], a_pEnemy );

	a_pEnemy->SetPoolOwner( 0 );
	a_pEnemy->ClearPooledID();
	AEntityFactory::GetSingleton()->RemoveEntity( *a_pEnemy );
	m_FreeList[ a_pEnemy->GetEnemyType() ].PushBack( pNode );

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	TINT            iNumPlayers    = pPlayerManager->GetNumPlayers();

	for ( TINT i = 0; i < iNumPlayers; i++ )
		TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( i ) ) )->GetZJumpComponent()->OnEntityRemoved( a_pEnemy );
}

// $deBlob: FUNCTION 0078a5b0
void AEnemyManager::ReleaseActiveEnemies( EnemyType a_eType )
{
	for ( auto it = m_ActiveList[ a_eType ].Begin(); it != m_ActiveList[ a_eType ].End(); )
	{
		AEnemy* pEnemy = ( it++ )->m_pEnemy;
		ReleaseEnemy( pEnemy );
	}
}

// $deBlob: FUNCTION 0078a0d0
void AEnemyManager::Tick( TFLOAT a_fDeltaTime )
{
	m_bCanUpdate = TTRUE;

	if ( --m_iResortTimer <= 0 )
	{
		m_iResortTimer = 60;

		for ( TINT i = 0; i < UpdateType_Count; i++ )
			ResortUpdateList( m_aUpdateLists[ i ] );
	}

	AAINavigator::sm_iNumOptimisedPaths = 0;

	for ( TINT i = 0; i < AEnemy::ComponentType_Count; i++ )
		m_aComponentManagers[ i ].Tick( a_fDeltaTime, i );

	UpdateTargets( a_fDeltaTime );
	UpdateEnvironments( a_fDeltaTime );
	UpdateEnemyList( a_fDeltaTime );
	UpdateMovement( a_fDeltaTime );

	// NOTE: retail reads the "enemy_show_updates" console variable without using it
	s_cvEnemyShowUpdates.GetBool();

	if ( TStringManager::String8CompareNoCase( "", s_cvEnemyCreate.GetString() ) != 0 )
		CreateDebugEnemy();
}

// Activates every component of a_pEnemy in its component manager
// $deBlob: FUNCTION 0078ac60
void AEnemyManager::ActivateComponents( AEnemy* a_pEnemy )
{
	for ( TINT i = 0; i < AEnemy::ComponentType_Count; i++ )
	{
		AEntityComponent2* pComponent = TSTATICCAST( AEntityComponent2, a_pEnemy->GetComponent( AEnemy::ComponentType( i ) ) );

		if ( pComponent != TNULL )
			m_aComponentManagers[ i ].Activate( pComponent, TTRUE );
	}
}

// $deBlob: FUNCTION 0078aca0
void AEnemyManager::TickComponents( TFLOAT a_fDeltaTime )
{
	AAINavigator::sm_iNumOptimisedPaths = 0;

	for ( TINT i = 0; i < AEnemy::ComponentType_Count; i++ )
		m_aComponentManagers[ i ].Tick( a_fDeltaTime, i );
}

// $deBlob: FUNCTION 0078b780
void AEnemyManager::CreateDebugEnemy()
{
	for ( TINT i = 0; i < ENEMYTYPE_Count; i++ )
	{
		if ( TStringManager::String8CompareNoCase( s_cvEnemyCreate.GetString(), GetEnemyClassName( EnemyType( i ) ) ) == 0 )
		{
			const TVector4& rcPlayerPos = APlayerManager::GetSingleton()->GetCurrentPlayerBlob()->GetMatrix().GetTranslation();

			TVector4 vPosition( rcPlayerPos.x + 3.0f, rcPlayerPos.y - 5.0f, rcPlayerPos.z + 3.0f, rcPlayerPos.w );
			CreateEnemy( EnemyType( i ), vPosition, TVector4( 1.0f, 0.0f, 0.0f, 1.0f ), TNULL, -1, 0, TFALSE, TNULL, 0 );
			break;
		}
	}

	s_cvEnemyCreate.SetString( "" );
}

// $deBlob: FUNCTION 0078a650
void AEnemyManager::AddTarget( ABaseEntity* a_pTarget )
{
	if ( a_pTarget != TNULL )
		m_Targets.PushBack( new AEnemyTarget( a_pTarget ) );

	TASSERT( ( a_pTarget == TNULL ) || a_pTarget->IsA( &TGetClass( ABlobEntity ) ) );
	AEnemyGrappler::AddPlayer( a_pTarget );
}

// $deBlob: FUNCTION 0078a750
AEnemyTarget* AEnemyManager::FindTarget( ABaseEntity* a_pTarget )
{
	for ( auto it = m_Targets.Begin(); it != m_Targets.End(); it++ )
	{
		if ( it->GetTarget() == a_pTarget )
			return it;
	}

	return TNULL;
}

// $deBlob: FUNCTION 0078add0
void AEnemyManager::UpdateTargets( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_Targets.Begin(); it != m_Targets.End(); it++ )
		it->Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0078aeb0
void AEnemyManager::UpdateEnvironments( TFLOAT a_fDeltaTime )
{
	UpdateList& rList = m_aUpdateLists[ UpdateType_Environment ];

	if ( rList.IsEmpty() )
	{
		m_iEnvironmentTimer = 0;
		return;
	}

	m_iEnvironmentTimer--;

	for ( auto it = rList.Begin(); it != rList.End(); it++ )
		it->m_fTime += a_fDeltaTime;

	if ( m_bCanUpdate && m_iEnvironmentTimer < 1 )
	{
		AEnemyUpdateNode* pNode = rList.PopFront();
		TSTATICCAST( AEnemyBehaviour, pNode->m_pEnemy->GetBehaviourComponent() )->UpdateEnvironment();
		pNode->m_fTime = 0.0f;
		rList.Push( pNode );

		m_pEnvironmentEnemy = pNode->m_pEnemy;
		m_iEnvironmentTimer = TSTATICCAST( EnemiesData, m_pEnemiesData )->m_iEnvironmentFrames;
		m_bCanUpdate        = TFALSE;
	}
}

// $deBlob: FUNCTION 0078b040
void AEnemyManager::UpdateEnemyList( TFLOAT a_fDeltaTime )
{
	UpdateList& rList = m_aUpdateLists[ UpdateType_EnemyList ];

	if ( rList.IsEmpty() )
	{
		m_iEnemyListTimer = 0;
		return;
	}

	AEnemyUpdateNode* pNode = TNULL;

	for ( auto it = rList.Begin(); it != rList.End(); )
	{
		AEnemyUpdateNode* pCurrent = ( it++ ).Get();
		pCurrent->m_fTime += a_fDeltaTime;

		// The enemy whose environment was just updated updates its enemy list too
		if ( m_bCanUpdate == TTRUE && m_pEnvironmentEnemy && m_pEnvironmentEnemy == pCurrent->m_pEnemy )
		{
			pNode = pCurrent;
			pCurrent->Remove();
			m_pEnvironmentEnemy = TNULL;
		}
	}

	m_iEnemyListTimer--;

	if ( m_bCanUpdate )
	{
		if ( pNode == TNULL )
		{
			if ( m_iEnemyListTimer > 0 )
				return;

			pNode = rList.PopFront();
		}

		TFLOAT fTime = pNode->m_fTime;
		TSTATICCAST( AEnemyBehaviour, pNode->m_pEnemy->GetBehaviourComponent() )->UpdateEnemyList( fTime );
		pNode->m_fTime = 0.0f;
		rList.Push( pNode );

		m_iEnemyListTimer = TSTATICCAST( EnemiesData, m_pEnemiesData )->m_iEnemyListFrames;
		m_bCanUpdate      = TFALSE;
	}
}

// $deBlob: FUNCTION 0078ace0
void AEnemyManager::UpdateMovement( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < ENEMYTYPE_Count; i++ )
	{
		for ( auto it = m_ActiveList[ i ].Begin(); it != m_ActiveList[ i ].End(); it++ )
			it->m_pEnemy->UpdateMovement( a_fDeltaTime );
	}
}

// $deBlob: FUNCTION 0078a860
void AEnemyManager::OnLODChanging( AEnemy* a_pEnemy )
{
	// NOTE: retail also constructs an unused local list here
	for ( TINT i = 0; i < UpdateType_Count; i++ )
	{
		UpdateList& rList = m_aUpdateLists[ i ];

		for ( auto it = rList.Begin(); it != rList.End(); )
		{
			AEnemyUpdateNode* pNode = ( it++ ).Get();

			if ( pNode->m_pEnemy == a_pEnemy )
			{
				pNode->Remove();
				rList.Push( pNode );
				break;
			}
		}
	}
}

// $deBlob: FUNCTION 0078aa20
void AEnemyManager::UpdateEnemyMusicMode( AEnemy* a_pEnemy, TINT a_iMusicMode )
{
	for ( TINT i = 0; i < AEnemy::ComponentType_Count; i++ )
	{
		TBOOL bNoUpdate;

		if ( a_iMusicMode == 0 )
			bNoUpdate = TFALSE;
		else if ( a_iMusicMode == 1 )
			bNoUpdate = i < AEnemy::ComponentType_Motor;
		else
			bNoUpdate = TTRUE;

		m_aComponentManagers[ i ].SetUpdateList( a_pEnemy, bNoUpdate ? AEnemyComponentManager::UpdateType_NoUpdate : AEnemyComponentManager::UpdateType_Update, i );
	}
}

// $deBlob: FUNCTION 0078aa70
void AEnemyManager::PauseForCutscene( TBOOL a_bPause )
{
	if ( m_bPaused == a_bPause )
		return;

	m_bPaused = a_bPause;
	AEnemyBehaviour::PauseForCutscene( a_bPause );

	if ( m_bPaused )
	{
		for ( TINT i = 0; i < ENEMYTYPE_Count; i++ )
		{
			for ( auto it = m_ActiveList[ i ].Begin(); it != m_ActiveList[ i ].End(); it++ )
				TSTATICCAST( AEnemyBehaviour, it->m_pEnemy->GetBehaviourComponent() )->SetSelectedEnemy( TNULL );
		}
	}
}

// $deBlob: FUNCTION 0078ab80
void AEnemyManager::LoadTRB()
{
	m_pEnemiesData = TNULL;
	m_pEnemiesData = AEntityData::GetSingleton()->GetEntityData( "Enemies_data_EntityData" );
}

// $deBlob: FUNCTION 007cda20
const TCHAR* AEnemyManager::GetEnemyClassName( EnemyType a_eType )
{
	switch ( a_eType )
	{
		case ENEMYTYPE_InkyBaton:
			return "AInky_Baton";
		case ENEMYTYPE_InkyPen:
			return "AInky_Pen";
		case ENEMYTYPE_LeechBot2:
			return "ALeechBot2";
		case ENEMYTYPE_InkyBatonElite:
			return "AInky_Baton_Elite";
		default:
			return TNULL;
	}
}

// $deBlob: FUNCTION 0078b4b0
AEnemyListNode* AEnemyManager::FindEnemyNode( AEnemy* a_pEnemy )
{
	TINT iEnemyType = a_pEnemy->GetEnemyType();
	TASSERT( ENEMYTYPE_INVALID != (EnemyType)iEnemyType );

	EnemyList& rList = m_ActiveList[ iEnemyType ];

	for ( auto it = rList.Begin(); it != rList.End(); it++ )
	{
		if ( it->m_pEnemy == a_pEnemy )
			return it.Get();
	}

	return TNULL;
}

// $deBlob: FUNCTION 0078b5e0
AEnemy* AEnemyManager::FindSuicider( EnemyType a_eType )
{
	EnemyList& rList = m_ActiveList[ a_eType ];

	for ( auto it = rList.Begin(); it != rList.End(); it++ )
	{
		AEnemyListNode* pNode  = it.Get();
		AEnemy*         pEnemy = pNode->m_pEnemy;

		if ( ( pEnemy->m_uiEnemyFlags & AEnemy::EFLAG_Suicider ) && pEnemy->GetPoolOwner() != 0 )
		{
			// The wrapper gives the enemy back (OnEnemyKilled), it's taken again at once
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_RECLAIM;
			oPacket.m_iData    = 0;
			TREINTERPRETCAST( ABaseEntity*, pEnemy->GetPoolOwner() )->HandleEntityIO( pEnemy, oPacket );

			pNode->Remove();
			AEntityFactory::GetSingleton()->ActivateEntity( *pEnemy );
			rList.PushBack( pNode );

			return pEnemy;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 0078b1a0
void AEnemyManager::RemoveFromUpdateList( UpdateList& a_rUpdateList, AEnemy* a_pEnemy )
{
	TASSERT( !a_rUpdateList.IsEmpty() );
	TINT iListSize = a_rUpdateList.Size();

	for ( auto it = a_rUpdateList.Begin(); it != a_rUpdateList.End(); )
	{
		AEnemyUpdateNode* pNode = ( it++ ).Get();

		if ( pNode->m_pEnemy == a_pEnemy )
		{
			pNode->Remove();
			m_FreeUpdateNodes.PushBack( pNode );
			TASSERT( iListSize == a_rUpdateList.Size() + 1 );
			return;
		}
	}
}

// $deBlob: FUNCTION 0078b2e0
void AEnemyManager::ResortUpdateList( UpdateList& a_rUpdateList )
{
	UpdateList oList;

	for ( auto it = a_rUpdateList.Begin(); it != a_rUpdateList.End(); )
	{
		AEnemyUpdateNode* pNode = ( it++ ).Get();
		pNode->Remove();
		oList.Push( pNode );
	}

	while ( !oList.IsEmpty() )
	{
		AEnemyUpdateNode* pNode = oList.PopBack();
		a_rUpdateList.Push( pNode );
	}
}

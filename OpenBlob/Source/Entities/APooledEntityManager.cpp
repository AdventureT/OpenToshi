#include "pch.h"
#include "APooledEntityManager.h"
#include "AEntityFactory.h"
#include "ABlobEntity/ABlobEntity.h"
#include "ABlobEntity/ABlobZJumpComponent.h"
#include "GameInterface/APlayerManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00790850
APooledEntityManager::APooledEntityManager()
{
	for ( TINT i = 0; i < NUM_POOL_TYPES; i++ )
	{
		m_EntityPoolAllocArray[ i ] = TNULL;
		m_aiEntityPoolCount[ i ]    = 0;
	}
}

// $deBlob: FUNCTION 0078dac0
// $deBlob: FUNCTION 00790950
APooledEntityManager::~APooledEntityManager()
{
	// The lists are destroyed after this (they assert that they're empty), then the singleton
}

// $deBlob: FUNCTION 00790a00
TINT APooledEntityManager::EnsurePoolCount( TINT a_eType, TINT a_iCount )
{
	TASSERT( m_EntityPoolAllocArray[ a_eType ] == TNULL );

	if ( m_aiEntityPoolCount[ a_eType ] < a_iCount )
		m_aiEntityPoolCount[ a_eType ] = a_iCount;

	return m_aiEntityPoolCount[ a_eType ];
}

// $deBlob: FUNCTION 00790a60
TINT APooledEntityManager::AddPoolCount( TINT a_eType, TINT a_iCount )
{
	TASSERT( m_EntityPoolAllocArray[ a_eType ] == TNULL );

	m_aiEntityPoolCount[ a_eType ] += a_iCount;
	return a_iCount;
}

// $deBlob: FUNCTION 00790ac0
void APooledEntityManager::CreatePools()
{
	for ( TINT i = 0; i < NUM_POOL_TYPES; i++ )
	{
		TINT iCount = m_aiEntityPoolCount[ i ];

		if ( iCount > 0 )
			CreatePool( i, iCount );
	}
}

// $deBlob: FUNCTION 00791110
void APooledEntityManager::CreatePool( TINT a_eType, TINT a_iCount )
{
	TASSERT( m_FreeList[ a_eType ].IsEmpty() );
	TASSERT( m_EntityPoolAllocArray[ a_eType ] == TNULL );

	APooledEntity* pPool              = new APooledEntity[ a_iCount ];
	m_EntityPoolAllocArray[ a_eType ] = pPool;

	const TCHAR* szName = GetEntityName( a_eType );

	for ( TINT i = 0; i < a_iCount; i++ )
	{
		APooledEntity* pPooled = &pPool[ i ];
		ABaseEntity*   pEntity = AEntityFactory::GetSingleton()->CreateEntity( szName );

		pPooled->m_pEntity = pEntity;

		// NOTE: temporary scaffolding, not in retail: the entity's class isn't ported yet
		// (CreateEntity returns TNULL); the entry stays out of the lists
		if ( pEntity == TNULL )
			continue;

		pEntity->SetFlags( pEntity->GetFlags() | ABaseEntity::FLAG_Pooled );
		pEntity->InitPooled( a_eType );
		pEntity->SetHash( TFALSE );

		pPooled->m_eType = a_eType;

		AEntityFactory::GetSingleton()->RemoveEntity( *pEntity );
		m_FreeList[ a_eType ].PushBack( pPooled );
	}
}

// $deBlob: FUNCTION 00790c90
void APooledEntityManager::DeactivateFreeEntities()
{
	for ( TINT i = 0; i < NUM_POOL_TYPES; i++ )
	{
		for ( auto it = m_FreeList[ i ].Begin(); it != m_FreeList[ i ].End(); it++ )
		{
			ABaseEntity* pEntity = it->m_pEntity;

			pEntity->PostCreate();         // +0x20
			pEntity->SetVisible( TFALSE ); // +0x38
			pEntity->SetActive( TFALSE );  // +0xCC
		}
	}
}

// $deBlob: FUNCTION 00790d90
void APooledEntityManager::DestroyPools()
{
	for ( TINT i = 0; i < NUM_POOL_TYPES; i++ )
	{
		m_UsedList[ i ].Clear();

		for ( auto it = m_FreeList[ i ].Begin(); it != m_FreeList[ i ].End(); it++ )
		{
			ABaseEntity* pEntity = it->m_pEntity;

			if ( pEntity != TNULL )
				delete pEntity;
		}

		m_FreeList[ i ].Clear();

		// NOTE: retail doesn't clear m_EntityPoolAllocArray here
		delete[] m_EntityPoolAllocArray[ i ];
	}
}

// $deBlob: FUNCTION 00790eb0
APooledEntity* APooledEntityManager::Acquire( TINT a_eType, const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rFacing, TINT a_iParam )
{
	if ( m_FreeList[ a_eType ].Begin() == m_FreeList[ a_eType ].End() )
		return TNULL;

	APooledEntity* pPooled = m_FreeList[ a_eType ].RBegin().Get();

	pPooled->Remove();
	m_UsedList[ a_eType ].PushBack( pPooled );

	ABaseEntity* pEntity = pPooled->m_pEntity;

	pEntity->SetPoolOwner( a_iParam );
	AEntityFactory::GetSingleton()->ActivateEntity( *pEntity );
	pEntity->SetPooledID();
	pEntity->SetPosition( a_rPosition );
	pEntity->SetFacing( a_rFacing );
	pEntity->SyncBodyTransform();
	pEntity->SetHash( TTRUE );

	return pPooled;
}

// $deBlob: FUNCTION 00790f60
void APooledEntityManager::Release( APooledEntity* a_pPooled )
{
	ABaseEntity* pEntity = a_pPooled->m_pEntity;

	pEntity->SetHash( TFALSE );
	pEntity->SetVisible( TFALSE ); // +0x38
	pEntity->SetActive( TFALSE );  // +0xCC
	pEntity->OnPoolRelease();      // +0xD0
	pEntity->SetPoolOwner( 0 );    // +0xC0

	pEntity->ClearPooledID();
	AEntityFactory::GetSingleton()->RemoveEntity( *pEntity );

	a_pPooled->Remove();
	m_FreeList[ a_pPooled->m_eType ].PushBack( a_pPooled );

	// The blobs' Z-jump components forget the entity
	TINT iNumPlayers = APlayerManager::GetSingleton()->GetNumPlayers();

	for ( TINT i = 0; i < iNumPlayers; i++ )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( i ) ) );
		pBlob->GetZJumpComponent()->OnEntityRemoved( pEntity );
	}
}

// $deBlob: FUNCTION 00791060
void APooledEntityManager::ReleaseAll()
{
	for ( TINT i = 0; i < NUM_POOL_TYPES; i++ )
	{
		auto it = m_UsedList[ i ].Begin();

		while ( it != m_UsedList[ i ].End() )
		{
			APooledEntity* pPooled = it.Get();
			it++;

			Release( pPooled );
		}
	}
}

// $deBlob: FUNCTION 007912b0
APooledEntity* APooledEntityManager::FindInUse( ABaseEntity* a_pEntity, TINT a_eType )
{
	for ( auto it = m_UsedList[ a_eType ].Begin(); it != m_UsedList[ a_eType ].End(); it++ )
	{
		if ( it->m_pEntity == a_pEntity )
			return it.Get();
	}

	return TNULL;
}

// $deBlob: FUNCTION 007e11c0
const TCHAR* APooledEntityManager::GetEntityName( TINT a_eType )
{
	switch ( a_eType )
	{
		case 0:
			return "ACiv";
		case 1:
		case 2:
		case 3:
			return "APropTriggerTimeBonus";
		case 4:
			return "APickupLife";
		default:
			return TNULL;
	}
}

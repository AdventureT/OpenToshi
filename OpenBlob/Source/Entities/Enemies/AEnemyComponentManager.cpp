#include "pch.h"
#include "AEnemyComponentManager.h"
#include "AEnemy.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007d0f40
AEnemyComponentManager::AEnemyComponentManager()
{
}

// $deBlob: FUNCTION 007d0f60
AEnemyComponentManager::~AEnemyComponentManager()
{
}

// $deBlob: FUNCTION 007d0f70
void AEnemyComponentManager::Activate( AEntityComponent2* a_pComponent, TBOOL a_bActive )
{
	if ( a_pComponent->IsLinked() )
		a_pComponent->Remove();

	m_aLists[ a_bActive ? UpdateType_Update : UpdateType_NoUpdate ].PushBack( a_pComponent );
}

// $deBlob: FUNCTION 007d0fd0
void AEnemyComponentManager::Destroy()
{
	for ( TINT i = 0; i < UpdateType_Count; i++ )
		m_aLists[ i ].DeleteAll();
}

// $deBlob: FUNCTION 007d1050
void AEnemyComponentManager::Tick( TFLOAT a_fDeltaTime, TINT a_eComponentType )
{
	UpdateList( a_fDeltaTime, m_aLists[ UpdateType_Update ], UpdateType_Update, a_eComponentType );
}

// $deBlob: FUNCTION 007d1100
void AEnemyComponentManager::UpdateList( TFLOAT a_fDeltaTime, ComponentList& a_rList, UpdateType a_eUpdateType, TINT a_eComponentType )
{
	for ( auto it = a_rList.Begin(); it != a_rList.End(); )
	{
		AEntityComponent2* pComponent = ( it++ ).Get();
		ABaseEntity*       pEntity    = pComponent->GetEntity();

		TASSERT( ( pEntity == TNULL ) || pEntity->IsA( &TGetClass( AEnemy ) ) );
		AEnemy* pEnemy = TSTATICCAST( AEnemy, pEntity );

		if ( pEnemy->IsActive() && pComponent->IsEnabled() )
			pComponent->Tick( a_fDeltaTime );
	}
}

// $deBlob: FUNCTION 007d1070
void AEnemyComponentManager::SetUpdateList( AEnemy* a_pEnemy, UpdateType a_eUpdateType, TINT a_eComponentType )
{
	AEntityComponent2* pComponent = TSTATICCAST( AEntityComponent2, a_pEnemy->GetComponent( AEnemy::ComponentType( a_eComponentType ) ) );

	if ( pComponent )
	{
		TASSERT( pComponent->IsLinked() );
		pComponent->Remove();
		m_aLists[ a_eUpdateType ].PushFront( pComponent );
	}
}

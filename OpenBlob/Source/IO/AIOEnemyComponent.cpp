#include "pch.h"
#include "AIOEnemyComponent.h"
#include "IO/AIOEntity.h"
#include "Entities/Enemies/AEnemyManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007cc4b0
AIOEnemyComponent::AIOEnemyComponent( AIOEntity* a_pOwner )
    : AIOEntityComponent( a_pOwner, TYPE )
{
}

// $deBlob: FUNCTION 007cc5d0
// $deBlob: FUNCTION 007cc4d0
AIOEnemyComponent::~AIOEnemyComponent()
{
}

// $deBlob: FUNCTION 007cc4e0
void AIOEnemyComponent::SetIOEntityData( void* a_pData, TTRB* a_pTRB )
{
	IOEntityData* pIOData = TSTATICCAST( IOEntityData, a_pData );

	TBOOL bCorrectType = pIOData->iType == TYPE;
	TASSERT( TTRUE == bCorrectType );

	Data* pData   = pIOData->pData;
	m_iEnemyType  = pData->iEnemyType;
	m_iUnk18      = pData->iUnk04;
	m_iPickupType = pData->iPickupType;
}

// $deBlob: FUNCTION 007cc540
TBOOL AIOEnemyComponent::ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther )
{
	if ( a_pMessage[ 0 ] != ACTION_Spawn )
		return TFALSE;

	// At the entity, facing its forward axis
	const TMatrix44& rMatrix = m_pOwner->GetMatrix();

	TVector4 vFacing = rMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vFacing.w        = 1.0f;

	TVector4 vPosition = rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	AEnemyManager::GetSingleton()->CreateEnemy( AEnemyManager::EnemyType( m_iEnemyType ), vPosition, vFacing, TNULL, TINT8( m_iPickupType ), TINT16( m_iUnk18 ), TFALSE, TNULL, 0 );
	return TTRUE;
}

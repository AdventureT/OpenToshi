#include "pch.h"
#include "ACollisionInstanceResponderMover.h"
#include "ACollisionInstanceMover.h"
#include "ACollisionInstance2.h"
#include "Entities/ABlobEntity/ABlobEntity.h"

#include <Physics/Havok/TPhysicsManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005a8220
void ACollisionInstanceResponderMover::OnCollisionAdded( TPhysicsCollisionInfo* a_pInfo )
{
	if ( m_pMover == TNULL )
		return;

	TPhysicsObjectContainer* pOther = a_pInfo->m_other.m_pBody;
	TUINT32                  uiGroup = pOther ? ( pOther->m_uiCollisionFilterInfo & 0xfff ) : 0;

	if ( ( uiGroup & 0x44 ) == 0 || pOther == TNULL )
		return;

	ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, pOther->m_pUserData );

	if ( pOwnerInfo == TNULL )
		return;

	if ( pOwnerInfo->m_eType == 1 )
	{
		// An entity: only if it's big enough
		if ( pOwnerInfo->m_pOwner != TNULL && TSTATICCAST( ABlobEntity, pOwnerInfo->m_pOwner )->GetSize() >= m_iUnk08 - 1 )
			m_pMover->OnHit();
	}
	else if ( pOwnerInfo->m_eType == 3 )
	{
		m_pMover->OnHit();
	}
}

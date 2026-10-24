#include "ToshiPCH.h"
#include "TCylinderData.h"
#include "Havok/TPhysicsManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TShapeDataCache TCylinderData::ms_oCache;

// $deBlob: FUNCTION 00733670
TCylinderData* TCylinderData::Get( TFLOAT a_fHeight, TFLOAT a_fRadius )
{
	TCylinderData* pData = TNULL;
	TINT   i     = 0;

	for ( ; i < ms_oCache.Size(); i++ )
	{
		TCylinderData* pCached = TSTATICCAST( TCylinderData, ms_oCache[ i ] );

		if ( pCached->m_fRadius == a_fRadius && pCached->m_fHeight == a_fHeight )
		{
			pData = pCached;
			break;
		}
	}

	if ( i == ms_oCache.Size() )
	{
		pData = new ( TMalloc( sizeof( TCylinderData ), TPhysicsManager::GetHeap() ) ) TCylinderData();
		pData->m_fHeight = a_fHeight;
		pData->m_fRadius = a_fRadius;
	}

	pData->m_eType = Type_Cylinder;
	pData->AddRef( &ms_oCache );
	return pData;
}

TOSHI_NAMESPACE_END

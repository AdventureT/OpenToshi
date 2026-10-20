#include "ToshiPCH.h"
#include "TSphereData.h"
#include "Havok/TPhysicsManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TShapeDataCache TSphereData::ms_oCache;

// $deBlob: FUNCTION 00733490
TSphereData* TSphereData::Get( TFLOAT a_fRadius )
{
	TSphereData* pData = TNULL;
	TINT   i     = 0;

	for ( ; i < ms_oCache.Size(); i++ )
	{
		TSphereData* pCached = TSTATICCAST( TSphereData, ms_oCache[ i ] );

		if ( pCached->m_fRadius == a_fRadius )
		{
			pData = pCached;
			break;
		}
	}

	if ( i == ms_oCache.Size() )
	{
		pData = new ( TMalloc( sizeof( TSphereData ), TPhysicsManager::GetHeap() ) ) TSphereData();
		pData->m_fRadius = a_fRadius;
	}

	pData->m_eType = Type_Sphere;
	pData->AddRef( &ms_oCache );
	return pData;
}

TOSHI_NAMESPACE_END

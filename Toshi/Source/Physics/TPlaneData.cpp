#include "ToshiPCH.h"
#include "TPlaneData.h"
#include "Havok/TPhysicsManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TShapeDataCache TPlaneData::ms_oCache;

// $deBlob: FUNCTION 00733370
TPlaneData* TPlaneData::Get( const TVector4& a_rcNormal, const TVector4& a_rcPoint )
{
	TPlaneData* pData = TNULL;
	TINT   i     = 0;

	for ( ; i < ms_oCache.Size(); i++ )
	{
		TPlaneData* pCached = TSTATICCAST( TPlaneData, ms_oCache[ i ] );

		if ( pCached->m_vNormal.IsEqual( a_rcNormal ) && pCached->m_vPoint.IsEqual( a_rcPoint ) )
		{
			pData = pCached;
			break;
		}
	}

	if ( i == ms_oCache.Size() )
	{
		pData = new ( TMalloc( sizeof( TPlaneData ), TPhysicsManager::GetHeap() ) ) TPlaneData();
		pData->m_vNormal = a_rcNormal;
		pData->m_vPoint  = a_rcPoint;
	}

	pData->m_eType = Type_Plane;
	pData->AddRef( &ms_oCache );
	return pData;
}

TOSHI_NAMESPACE_END

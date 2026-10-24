#include "ToshiPCH.h"
#include "TBoxData.h"
#include "Havok/TPhysicsManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TShapeDataCache TBoxData::ms_oCache;

// $deBlob: FUNCTION 00733580
TBoxData* TBoxData::Get( const TVector4& a_rcHalfExtents )
{
	TBoxData* pData = TNULL;
	TINT      i     = 0;

	for ( ; i < ms_oCache.Size(); i++ )
	{
		TBoxData*       pCached = TSTATICCAST( TBoxData, ms_oCache[ i ] );
		const TVector4& rcVec   = pCached->m_vHalfExtents;

		if ( a_rcHalfExtents.x == rcVec.x && a_rcHalfExtents.y == rcVec.y && a_rcHalfExtents.z == rcVec.z && a_rcHalfExtents.w == rcVec.w )
		{
			pData = pCached;
			break;
		}
	}

	if ( i == ms_oCache.Size() )
	{
		pData                 = new ( TMalloc( sizeof( TBoxData ), TPhysicsManager::GetHeap() ) ) TBoxData();
		pData->m_vHalfExtents = a_rcHalfExtents;
	}

	pData->m_eType = Type_Box;
	pData->AddRef( &ms_oCache );
	return pData;
}

TOSHI_NAMESPACE_END

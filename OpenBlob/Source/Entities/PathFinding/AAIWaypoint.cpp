#include "pch.h"
#include "AAIWaypoint.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005419d0
// $deBlob: FUNCTION 0085a110
Toshi::T2DList<AAIWaypoint> AAIWaypoint::sm_FreeList;

// $deBlob: FUNCTION 007dfe00
// $deBlob: FUNCTION 007dfce0
AAIWaypoint::~AAIWaypoint()
{
}

// $deBlob: FUNCTION 007dfd20
void AAIWaypoint::FillWaypointFreeList()
{
	for ( TINT i = 64; i != 0; i-- )
		sm_FreeList.PushBack( new AAIWaypoint() );
}

// $deBlob: FUNCTION 007e6af0
AAIWaypoint* AAIWaypoint::GetFreeWaypoint()
{
	if ( sm_FreeList.IsEmpty() )
		FillWaypointFreeList();

	AAIWaypoint* pWaypoint = sm_FreeList.PopBack();
	pWaypoint->m_uiFlags   = 0;

	return pWaypoint;
}

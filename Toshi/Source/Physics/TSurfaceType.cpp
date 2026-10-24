#include "ToshiPCH.h"
#include "TSurfaceType.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

const TCollisionSurface CollisionSurfaces::ms_aSurfaces[ NUM_SURFACES ] = {
	{ 0, 0, "surface_dust(default)" },
	{ 1, 1, "surface_rock" },
	{ 2, 2, "surface_sand" },
	{ 3, 3, "surface_goo" },
	{ 4, 4, "surface_water" },
	{ 5, 5, "surface_metal" },
	{ 6, 6, "surface_grass" },
	{ 7, 7, "surface_wood" },
};

// $deBlob: FUNCTION 005ac9b0
const TCollisionSurface* CollisionSurfaces::GetCollisionSurface( TUINT32 a_eGroupFlag )
{
	for ( TINT i = 0; i < NUM_SURFACES; i++ )
	{
		if ( ( 1 << ( i + 8 ) ) & a_eGroupFlag )
			return &ms_aSurfaces[ i ];
	}

	return &ms_aSurfaces[ 0 ];
}

TOSHI_NAMESPACE_END

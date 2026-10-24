#include "ToshiPCH.h"
#include "TPhysicsHeightFieldShape.h"
#include "TPhysicsHFContainer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

hkReal TPhysicsHeightFieldShape::getHeightAt( int a_x, int a_z ) const
{
	return m_pContainer->GetHeightAt( a_x, a_z );
}

// $deBlob: FUNCTION 00731700
void TPhysicsHeightFieldShape::collideSpheres( const CollideSpheresInput& a_rInput, SphereCollisionOutput* a_pOutputArray ) const
{
	// Havok's template (hkpSampledHeightFieldShape.inl)
	hkSampledHeightFieldShape_collideSpheres( *this, a_rInput, a_pOutputArray );
}

// $deBlob: FUNCTION 00731ff0
hkReal TPhysicsHeightFieldShape::getHeightAtImpl( int a_x, int a_z ) const
{
	return m_pContainer->GetHeightAt( a_x, a_z );
}

TOSHI_NAMESPACE_END

#include "pch.h"
#include "ACollisionObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005a9cd0
ACollisionObject::ACollisionObject( void* a_pOwner, TINT a_eOwnerType, TUINT8 a_uiGroupMask )
{
	m_pOwner      = a_pOwner;
	m_uiGroupMask = a_uiGroupMask;
	m_uiFlags     = FLAG_AABBDirty | FLAG_SphereDirty;

	if ( a_eOwnerType == 0 )
		m_uiFlags = FLAG_AABBDirty | FLAG_SphereDirty | FLAG_OwnerType0;
	else if ( a_eOwnerType == 1 )
		m_uiFlags = FLAG_AABBDirty | FLAG_SphereDirty | FLAG_OwnerType1;
	else if ( a_eOwnerType == 2 )
		m_uiFlags = FLAG_AABBDirty | FLAG_SphereDirty | FLAG_OwnerType2;
}

// $deBlob: FUNCTION 005a9dc0
TBOOL ACollisionObject::SphereIntersectsAABB( const TVector4& a_rvSphere, const TVector4& a_rvAABBCentre, const TVector4& a_rvAABBHalfExtents )
{
	TASSERT( a_rvAABBHalfExtents.x >= 0 );
	TASSERT( a_rvAABBHalfExtents.y >= 0 );
	TASSERT( a_rvAABBHalfExtents.z >= 0 );
	TASSERT( a_rvSphere.w >= 0 && a_rvSphere.w < 1000.0f );

	TFLOAT fDistX = TMath::Abs( a_rvSphere.x - a_rvAABBCentre.x );
	TFLOAT fDistY = TMath::Abs( a_rvSphere.y - a_rvAABBCentre.y );
	TFLOAT fDistZ = TMath::Abs( a_rvSphere.z - a_rvAABBCentre.z );

	TFLOAT fOutX = fDistX - a_rvAABBHalfExtents.x;
	TFLOAT fOutY = fDistY - a_rvAABBHalfExtents.y;
	TFLOAT fOutZ = fDistZ - a_rvAABBHalfExtents.z;

	TFLOAT fDist;
	TFLOAT fRadius;

	if ( a_rvAABBHalfExtents.x < fDistX )
	{
		fRadius = a_rvSphere.w;

		if ( fDistY <= a_rvAABBHalfExtents.y )
		{
			if ( fDistZ <= a_rvAABBHalfExtents.z )
				return fOutX <= fRadius;

			return fOutZ * fOutZ + fOutX * fOutX <= fRadius * fRadius;
		}

		fRadius = fRadius * fRadius;
		fDist   = fOutY * fOutY + fOutX * fOutX;

		if ( a_rvAABBHalfExtents.z < fDistZ )
			fDist += fOutZ * fOutZ;
	}
	else
	{
		if ( fDistY <= a_rvAABBHalfExtents.y )
		{
			if ( fDistZ <= a_rvAABBHalfExtents.z )
				return TTRUE;

			return fOutZ <= a_rvSphere.w;
		}

		fRadius = a_rvSphere.w;

		if ( a_rvAABBHalfExtents.z < fDistZ )
			return fOutZ * fOutZ + fOutY * fOutY <= fRadius * fRadius;

		fDist = fOutY;
	}

	if ( fDist <= fRadius )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 005aa040
// $deBlob: FUNCTION 005a9d20
ACollisionObject::~ACollisionObject()
{
}

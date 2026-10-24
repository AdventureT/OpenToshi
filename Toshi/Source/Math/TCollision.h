#pragma once
#include "TVector4.h"
#include "TPlane.h"
#include "TMatrix44.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: geometric collision tests
//-----------------------------------------------------------------------------
namespace TCollision
{

// The results of AABBSweptvsPlane
enum AABBSweptResult
{
	AABBSWEPT_NO_HIT   = 0,
	AABBSWEPT_HIT      = 1,
	AABBSWEPT_TOUCHING = 2, // the box already touches the plane
};

// The boxes a_rcCenter1 +- a_rcHalfExtents1 and a_rcCenter2 +- a_rcHalfExtents2 overlap (touching included)
TBOOL AABBvsAABB( const TVector4& a_rcCenter1, const TVector4& a_rcHalfExtents1, const TVector4& a_rcCenter2, const TVector4& a_rcHalfExtents2 );

// Sweeps the box a_rcCenter +- a_rcHalfExtents along a_rcDirection against the box a_rcBoxCenter +- a_rcBoxHalfExtents:
// TRUE if it's hit closer than a_rfDistance (a_rfDistance: the distance to the hit, a_rOutNormal: the face's normal)
TBOOL AABBSweptvsAABB( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, const TVector4& a_rcBoxCenter, const TVector4& a_rcBoxHalfExtents, TVector4& a_rOutNormal, TFLOAT& a_rfDistance );

// Sweeps the box a_rcCenter +- a_rcHalfExtents along a_rcDirection for a_rfDistance against the front of a_rcPlane:
// Returns an AABBSweptResult (a_rfDistance: the distance to the hit, 0 when already touching)
TINT AABBSweptvsPlane( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, const TPlane& a_rcPlane, TFLOAT& a_rfDistance );

// Sweeps the box a_rcCenter +- a_rcHalfExtents along a_rcDirection against the triangle a_ppVertices: TRUE if it's hit
// closer than a_rfDistance (a_rfDistance: the distance to the hit)
TBOOL AABBSweptvsTriangle( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance );

// The box that contains the box a_rcCenter +- a_rcHalfExtents swept along a_rcDirection for a_fDistance
void AABBSweptArea( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, TFLOAT a_fDistance, TVector4& a_rOutCenter, TVector4& a_rOutHalfExtents );

// Separating axis test of the box a_rcCenter +- a_vHalfExtents against the oriented box a_vOBBCenter, a_rcOBBRotation
// (the rows are its axes), a_rcOBBHalfExtents: TRUE if they overlap
TBOOL AABBvsOBB( const TVector4& a_rcCenter, TVector4 a_vHalfExtents, TVector4 a_vOBBCenter, const TMatrix44& a_rcOBBRotation, const TVector4& a_rcOBBHalfExtents );

} // namespace TCollision

TOSHI_NAMESPACE_END

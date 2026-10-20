#pragma once
#include "TMatrix44.h"
#include "TPlane.h"
#include "TSphere.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: Capsule (segment of fLength along vDirection centred on vCentre).
//-----------------------------------------------------------------------------
struct TCapsule
{
	TCapsule( const TVector4& a_rcCentre, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT a_fLength );

	TVector4 vCentre;         // 0x00
	TVector4 vDirection;      // 0x10
	TFLOAT   fRadius;         // 0x20
	TFLOAT   fLength;         // 0x24
	TFLOAT   fBoundingRadius; // 0x28
};

//-----------------------------------------------------------------------------
// Purpose: Ray (fLength along vDirection from vOrigin) used by the box queries.
//-----------------------------------------------------------------------------
struct TRay
{
	TVector4 vOrigin;    // 0x00
	TVector4 vDirection; // 0x10
	TFLOAT   fLength;    // 0x20
};

//-----------------------------------------------------------------------------
// Purpose: Triangle used by the box queries.
//-----------------------------------------------------------------------------
struct TCollisionTriangle
{
	TCollisionTriangle() = default;
	TCollisionTriangle( const TVector4& a_rcVertex1, const TVector4& a_rcVertex2, const TVector4& a_rcVertex3, const TVector4& a_rcNormal );

	TVector4 aVertices[ 3 ]; // 0x00
	TVector4 vNormal;        // 0x30
};

//-----------------------------------------------------------------------------
// Purpose: Contact found by the box queries.
//-----------------------------------------------------------------------------
struct TCollisionContact
{
	TVector4 vPosition; // 0x00
	TVector4 vNormal;   // 0x10
	TFLOAT   fDepth;    // 0x20
};

//-----------------------------------------------------------------------------
// Purpose: Oriented bounding box. The rows of the transform are the axes of the box and its centre,
// m_vExtents are the half sizes along the axes. The functions are in the order of de Blob's
// TOBBox.cpp (by address).
//-----------------------------------------------------------------------------
class TOBBox
{
public:
	TOBBox();
	TOBBox( const TMatrix44& a_rcTransform, const TVector4& a_rcExtents );
	TOBBox( const TOBBox& a_rcOther );

	TOBBox& operator=( const TOBBox& a_rcOther );

	// Returns 1 if the box is behind the plane, -1 if it's in front of it and 0 if the plane intersects it
	TINT ComparePlane( const TPlane& a_rcPlane ) const;

	TBOOL PointInside( const TVector3& a_rcPoint ) const;
	TBOOL PointInside( const TVector4& a_rcPoint ) const;

	// Returns the point of the box that is the closest to a_rcPoint
	void ClosestPoint( const TVector4& a_rcPoint, TVector4& a_rOutPoint ) const;

	void SetAxes( const TVector4& a_rcAxisX, const TVector4& a_rcAxisY, const TVector4& a_rcAxisZ );

	const TVector4& GetAxis( TINT a_iAxis ) const { return m_oTransform.AsBasisVector4( a_iAxis ); }
	const TVector4& GetCentre() const { return m_oTransform.GetTranslation(); }

	//-----------------------------------------------------------------------------
	// Box against a triangle. Return the time of the hit (0..1) or 2.0f if the box doesn't hit the triangle.
	//-----------------------------------------------------------------------------

	// Sweeps the box against the triangle from outside of the triangle's plane
	static TFLOAT IntersectTriangle( const TOBBox& a_rcBox, const TCollisionTriangle& a_rcTriangle, TVector4& a_rOutPoint, TVector4& a_rOutNormal );

	// Sweeps the box from a_rcPrevBox to a_rcBox
	static TFLOAT IntersectTriangle( const TOBBox& a_rcPrevBox, const TOBBox& a_rcBox, const TCollisionTriangle& a_rcTriangle, TVector4& a_rOutPoint, TVector4& a_rOutNormal );

	// Sweeps a_rcBox along a_rcMovement
	static TFLOAT IntersectTriangle( const TOBBox& a_rcBox, const TVector4& a_rcMovement, const TCollisionTriangle& a_rcTriangle, TVector4& a_rOutPoint, TVector4& a_rOutNormal );

	//-----------------------------------------------------------------------------
	// Overlap tests
	//-----------------------------------------------------------------------------
	static TBOOL Intersects( const TOBBox& a_rcBox, const TRay& a_rcRay );
	static TBOOL Intersects( const TOBBox& a_rcBox1, const TOBBox& a_rcBox2 );
	static TBOOL Intersects( const TOBBox& a_rcBox, const TSphere& a_rcSphere );
	static TBOOL Intersects( const TOBBox& a_rcBox, const TCapsule& a_rcCapsule );
	static TBOOL Intersects( const TOBBox& a_rcBox, const TCollisionTriangle& a_rcTriangle );

	//-----------------------------------------------------------------------------
	// Contacts (the normal points from the second shape to the first one)
	//-----------------------------------------------------------------------------
	static TBOOL Collide( const TOBBox& a_rcBox, const TRay& a_rcRay, TCollisionContact& a_rContact );
	static TBOOL Collide( const TOBBox& a_rcBox, const TCapsule& a_rcCapsule, TCollisionContact& a_rContact );
	static TBOOL Collide( const TOBBox& a_rcBox1, const TOBBox& a_rcBox2, TCollisionContact& a_rContact );
	static TBOOL Collide( const TOBBox& a_rcBox, const TSphere& a_rcSphere, TCollisionContact& a_rContact );

	// Spheres are ( x, y, z, radius )
	static TBOOL CollideSphereSphere( const TVector4& a_rcSphere1, const TVector4& a_rcSphere2, TCollisionContact& a_rContact );

	// Projects the box onto a_rcAxis
	static void ProjectBoxOntoAxis( const TVector4& a_rcAxis, const TMatrix44& a_rcTransform, const TVector4& a_rcExtents, TFLOAT& a_rfMin, TFLOAT& a_rfMax );

private:
	static void  ProjectTriangleOntoAxis( const TVector4& a_rcAxis, const TCollisionTriangle& a_rcTriangle, TFLOAT& a_rfMin, TFLOAT& a_rfMax );
	static TBOOL TestForNoIntersection( TFLOAT a_fVelocity, TFLOAT a_fMin1, TFLOAT a_fMax1, TFLOAT a_fMin2, TFLOAT a_fMax2, TFLOAT& a_rfEnter, TFLOAT& a_rfExit );

public:
	TMatrix44 m_oTransform; // 0x00
	TVector4  m_vExtents;   // 0x40
	TFLOAT    m_fRadius;    // 0x50, length of the extents
	TFLOAT    m_fMinExtent; // 0x54
};

TSTATICASSERT( sizeof( TOBBox ) == 0x58 );

// Closest points of the segment a_rcStart - a_rcEnd and the box
void ClosestLineBoxPoints( const TVector4& a_rcStart, const TVector4& a_rcEnd, const TOBBox& a_rcBox, TVector4& a_rOutSegmentPoint, TVector4& a_rOutBoxPoint );

TOSHI_NAMESPACE_END

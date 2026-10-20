/**
 * @file TMath.h
 * Core math utilities for the Toshi engine
 */

#pragma once
#include "TVector2.h"
#include "TVector4.h"
#include "TPlane.h"

TOSHI_NAMESPACE_START

class TSphere;

namespace TMath
{

// Returns the signed angle (radians) from a_vA to a_vB in the XZ plane; both must be normalised
TFLOAT AngleBetweenXZ( const TVector4& a_vA, const TVector4& a_vB );

// Shortest signed angle from a_fFrom to a_fTo, in [-PI, PI]
TFLOAT GetAngleDifference( TFLOAT a_fFrom, TFLOAT a_fTo );

// Moves a_vFrom towards a_vTo by a_fT (x, y and z, not past a_vTo; w is a_vFrom's)
TVector4 LERPVector4Clip( TVector4 a_vFrom, TVector4 a_vTo, TFLOAT a_fT );

// a_fTo - a_fFrom turning the given way round (a_bPositive: the result is >= 0, else <= 0)
TFLOAT GetDirectedAngleDifference( TFLOAT a_fFrom, TFLOAT a_fTo, TBOOL a_bPositive );

// Which side of the 2D plane (a_rcNormal through a_rcPlanePoint) a_rcPoint is: TTRUE / the distance along the normal
TBOOL  PointVsPlane2D( const TVector2& a_rcNormal, const TVector2& a_rcPlanePoint, const TVector2& a_rcPoint );
TFLOAT PointVsPlane2DF( const TVector2& a_rcNormal, const TVector2& a_rcPlanePoint, const TVector2& a_rcPoint );

// The tangent of a cardinal spline: (a_rcNext - a_rcPrevious) * a_fTension
void CalcCardinalSplineTangent( TVector3& a_rOutTangent, const TVector3& a_rcPrevious, const TVector3& a_rcNext, TFLOAT a_fTension );

// The point at a_fT of the Hermite curve from a_rvStart to a_rvEnd
void HermitePosition( TVector4& a_rvOut, const TVector4& a_rvStart, const TVector4& a_rvStartTangent, const TVector4& a_rvEnd, const TVector4& a_rvEndTangent, TFLOAT a_fT );

// The tangent at a_fT of the Hermite curve from a_rvStart to a_rvEnd
void HermiteTangent( TVector4& a_rvOut, const TVector4& a_rvStart, const TVector4& a_rvStartTangent, const TVector4& a_rvEnd, const TVector4& a_rvEndTangent, TFLOAT a_fT );

// Performs a cone-sphere intersection test and returns TTRUE if hit.
TBOOL ConeVsSphere( const TVector4& a_rvConePosition, const TVector4& a_rvConeDirection, float a_fConeFOV, float a_fSphereRadius, const TVector4& a_rvSpherePosition );

// Cone-sphere test with a cone length: the sphere is hit if it lies within a_fConeLength + a_fSphereRadius of the cone
// position inside the cone's FOV
TBOOL ConeVsSphere( const TVector4& a_rvConePosition, const TVector4& a_rvConeDirection, float a_fConeFOV, float a_fConeLength, const TVector4& a_rvSpherePosition, float a_fSphereRadius );

// Performs a ray-sphere intersection test and returns number of intersections. Always returns 0 or 2 intersections.
TINT RayVsCircle3D( const TVector4& a_rcRayOrigin, const TVector4& a_rcRayDir, const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfNearIntersection, TFLOAT& a_rfFarIntersection );

// Performs a sphere-line intersection test and returns TTRUE if hit anything.
TBOOL SphereVsLine( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TINT* a_pNumHits, TFLOAT* a_pNearIntersection, TFLOAT* a_pFarIntersection );

// Calculates squared distance from a point to line. Outputs position of the closest point in a_rClosestPoint.
TFLOAT DistancePointToLineSq( const TVector4& a_rcPoint, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TVector4& a_rClosestPoint );

// Calculates normal (counter clockwise) vector for a triangle.
void TriangleNormalCounterClockwise( TVector3& a_rNormal, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 );

// Calculates the plane of a triangle (counter clockwise normal).
// Normal of a counter clockwise triangle, not normalised
void TriangleUnscaledNormalCounterClockwise( TVector4& a_rNormal, const TVector3* const* a_ppVertices );

// The other variants of the triangle normals
void TriangleNormalCounterClockwise( TVector4& a_rNormal, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 );
void TriangleNormalCounterClockwise( TVector4& a_rNormal, const TVector3* const* a_ppVertices );
void TriangleNormalCounterClockwise( TVector3& a_rNormal, const TVector3* const* a_ppVertices );
void TriangleUnscaledNormalCounterClockwise( TVector4& a_rNormal, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 );
void TriangleUnscaledNormalCounterClockwise( TVector3& a_rNormal, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 );
void TriangleUnscaledNormalCounterClockwise( TVector4& a_rNormal, const TVector4* const* a_ppVertices );
void TriangleUnscaledNormalCounterClockwise( TVector3& a_rNormal, const TVector3* const* a_ppVertices );

// Whether the boxes overlap; when they don't, a_pCenter1Inside (optional) tells if the first box's center is inside
// the second box
TBOOL AABBVsAABB( const TVector4& a_rcCenter1, const TVector4& a_rcHalfExtents1, const TVector4& a_rcCenter2, const TVector4& a_rcHalfExtents2, TBOOL* a_pCenter1Inside );

void TrianglePlane( TPlane& a_rOutPlane, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 );

// Returns TTRUE if a_rcPoint (assumed to lie on the triangle's plane) is inside the triangle.
TBOOL IsPointInTriangle( const TVector4& a_rcVertex1, const TVector4& a_rcVertex2, const TVector4& a_rcVertex3, const TVector4& a_rcPoint );

// Returns TTRUE if the 2D point a_rcPoint is inside the triangle (barycentric test).
TBOOL IsPointInTriangle2D( const TVector2& a_rcVertex1, const TVector2& a_rcVertex2, const TVector2& a_rcVertex3, const TVector2& a_rcPoint );

// Returns TTRUE if a_rcPoint (assumed to lie on the polygon's plane) is inside the convex polygon (triangle fan).
TBOOL IsPointInPolygon( TINT a_iNumVertices, const TVector3* const* a_ppVertices, const TVector4& a_rcPoint );

// Squared distance from a point to a segment.
TFLOAT DistancePointToSegmentSq( const TVector4& a_rcPoint, const TVector3& a_rcStart, const TVector3& a_rcEnd );

// Squared distance from a point to a segment. Outputs the (clamped) segment parameter of the closest point.
TFLOAT DistancePointToSegmentSq( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcEnd, TFLOAT& a_rfT );

// Intersects two 2D lines, outputs the parameter on the first line. Returns TFALSE if the lines are parallel.
TBOOL LineVsLine2D( const TFLOAT* a_pLine1Start, const TFLOAT* a_pLine1End, const TFLOAT* a_pLine2Start, const TFLOAT* a_pLine2End, TFLOAT& a_rfT );

// Parameter (not clamped) of the point of the 2D line closest to a_rcPoint.
TFLOAT ClosestPointOnLine2D( const TVector2& a_rcLineStart, const TVector2& a_rcLineEnd, const TVector2& a_rcPoint );

// Returns TTRUE if the 2D segment is closer than a_fRadius to a_rcCenter. a_pOutPenetration (optional) gets how much closer.
TBOOL CircleVsSegment2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fRadius, const TVector2& a_rcCenter, TFLOAT* a_pOutPenetration );

// 2D ray (a_rcDirection normalised) against a circle: outputs the distances along the ray where it enters/leaves the
// circle, returns 2 (0 if the ray misses it or the circle is behind the ray)
TINT RayVsCircle2D( const TVector2& a_rcOrigin, const TVector2& a_rcDirection, const TVector2& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfNearIntersection, TFLOAT& a_rfFarIntersection );

// 2D ray (a_rcDirection normalised, a_rfDistance long) against the 2D capsule a_rcStart - a_rcEnd of a_fRadius. Returns
// TTRUE if it hits, a_rfDistance gets the distance to the hit
TBOOL RayVsCapsule2D( const TVector2& a_rcOrigin, const TVector2& a_rcDirection, TFLOAT& a_rfDistance, const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fRadius );

// Intersects two 2D lines, outputs the parameters of the intersection on both lines. Returns TFALSE if the lines are parallel.
TBOOL LineVsLine2D( const TVector2& a_rcLine1Start, const TVector2& a_rcLine1End, const TVector2& a_rcLine2Start, const TVector2& a_rcLine2End, TFLOAT& a_rfT1, TFLOAT& a_rfT2 );

// Intersects two 2D segments, outputs the intersection point. Returns TFALSE if they don't intersect.
TBOOL LineSegmentVsLineSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2, TVector2& a_rOutPoint );

// TRUE if the two 2D segments intersect.
TBOOL LineSegmentVsLineSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2 );

// Distance from a_rcStart to the projection of a_rcPoint on the segment, clamped to the segment.
TFLOAT DistanceAlongSegment( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcEnd );

// Squared distance between two segments.
TFLOAT DistanceSegmentToSegmentSq( const TVector3& a_rcStart1, const TVector3& a_rcEnd1, const TVector3& a_rcStart2, const TVector3& a_rcEnd2 );

// The square root of DistanceSegmentToSegmentSq
TFLOAT DistanceSegmentToSegment( const TVector3& a_rcStart1, const TVector3& a_rcEnd1, const TVector3& a_rcStart2, const TVector3& a_rcEnd2 );

// Squared distance between two 2D segments.
TFLOAT DistanceSegmentToSegmentSq2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2 );

// Distance between two 2D segments.
TFLOAT DistanceSegmentToSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2 );

// Returns TTRUE if two 2D segments are closer than a_fDistance to each other.
TBOOL IsSegmentNearSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2, TFLOAT a_fDistance );

// Returns TTRUE if the circle overlaps the 2D capsule (a_rcStart - a_rcEnd segment of a_fCapsuleRadius). a_pOutPenetration (optional) gets how deep.
TBOOL CircleVsCapsule2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fCapsuleRadius, const TVector2& a_rcCenter, TFLOAT a_fRadius, TFLOAT* a_pOutPenetration );

// Returns TTRUE if two spheres (xyz is the center, w is the radius) overlap.
TBOOL SphereVsSphere( const TVector4& a_rcSphere1, const TVector4& a_rcSphere2 );

// Returns TTRUE if two spheres overlap or touch.
TBOOL SphereVsSphere( const TVector4& a_rcCenter1, TFLOAT a_fRadius1, const TVector4& a_rcCenter2, TFLOAT a_fRadius2 );

// Sweeps a sphere from a_rcCenter along a_rcDirection (normalised) for a_rfDistance against a static sphere.
// On hit, a_rfDistance is the distance to the contact (minus a_fEpsilon) and a_rOutNormal points from a_rcOtherCenter to the sphere at the contact.
TBOOL SweptSphereVsSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector4& a_rcOtherCenter, TFLOAT a_fOtherRadius, TFLOAT& a_rfDistance, TVector4& a_rOutNormal, TFLOAT a_fEpsilon );

// Intersects the ray from a_rcOrigin along a_rcDirection (normalised) with the sphere. Returns the number of hits (0 or 2),
// a_rfNear and a_rfFar are the distances along the ray (a_rfNear can be negative if the ray starts inside the sphere).
TINT RayVsSphere( const TVector3& a_rcOrigin, const TVector3& a_rcDirection, const TVector3& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfNear, TFLOAT& a_rfFar );

// Sweeps a sphere from a_rcCenter along a_rcDirection (normalised) for a_rfDistance against the plane through a_vPlanePoint
// (the normal is flipped to face the sphere). On hit, a_rfDistance is the distance to the contact minus 0.1 (at least 0).
TBOOL SweptSphereVsPlane( const TVector4& a_rcCenter, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4 a_vPlanePoint, TVector4 a_vPlaneNormal );

// Returns TTRUE if a_rcPoint is inside the capsule going from a_rcStart along a_rcDirection for a_fLength.
TBOOL IsPointInCapsule( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT a_fLength );

// Tests a sphere against a convex polygon. a_rfDistance is the maximum allowed distance and outputs the distance to the polygon.
TBOOL SphereVsPolygon( const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4& a_rNormal, const TVector3* const* a_ppVertices, const TPlane& a_rcPlane, TINT a_iNumVertices, TFLOAT a_fEpsilon );

// Sweeps a sphere along a_rcDirection against a triangle. a_rfDistance is the maximum distance and outputs the distance to the hit.
// Returns a combination of hit types: 1 - face, 2 - vertex, 16 - edge (0 if nothing was hit).
TINT SweptSphereVsTriangle( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance, const TPlane& a_rcPlane, TVector4& a_rNormal );
TINT SweptSphereVsTriangle( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector3* const* a_ppVertices, TFLOAT& a_rfDistance, const TPlane& a_rcPlane, TVector4& a_rNormal );

// Deflects a_rcDirection off a plane with a_rcNormal and calculates how much of the remaining movement is left.
void SlideAlongPlane( const TVector4& a_rcDirection, const TVector4& a_rcNormal, TFLOAT a_fSpeedScale, TFLOAT a_fCompleteDist, TFLOAT a_fTravelledDistance, TFLOAT& a_rfOutDistance, TVector4& a_rOutDirection, TFLOAT a_fPushOut, TFLOAT a_fFriction, TFLOAT a_fMaxDot );

// Same as SlideAlongPlane, a_bLinearFriction makes the remaining distance fall off linearly with the angle.
void SlideAlongPlane( const TVector4& a_rcDirection, const TVector4& a_rcNormal, TFLOAT a_fSpeedScale, TFLOAT a_fCompleteDist, TFLOAT a_fTravelledDistance, TFLOAT& a_rfOutDistance, TVector4& a_rOutDirection, TBOOL a_bLinearFriction, TFLOAT a_fPushOut, TFLOAT a_fFriction, TFLOAT a_fMaxDot );

// Squared distance from a point to an infinite line. Outputs the parameter of the closest point.
TFLOAT DistancePointToLineSq( const TVector4& a_rcPoint, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TFLOAT& a_rfT );

// Squared distance between two segments. Outputs the parameters of the closest points.
TFLOAT DistanceSegmentToSegmentSq( const TVector4& a_rcStart1, const TVector4& a_rcEnd1, const TVector4& a_rcStart2, const TVector4& a_rcEnd2, TFLOAT& a_rfT1, TFLOAT& a_rfT2 );

// Ray (starting outside) against a sphere (a_fHeight is 0) or a capsule going from a_rcBase along a_rcUp for a_fHeight.
TBOOL RayVsCapsuleOutside( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, const TVector4& a_rcBase, TFLOAT a_fRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon );

// Same as RayVsCapsuleOutside but also handles rays starting inside of the capsule.
TBOOL RayVsCapsule( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, const TVector4& a_rcBase, TFLOAT a_fRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon );

// Sphere against a capsule. a_rfDistance is the maximum penetration and outputs the penetration depth.
TBOOL SphereVsCapsule( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcBase, TFLOAT a_fCapsuleRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon );

// Sweeps a sphere against a sphere (a_fHeight is 0) or a capsule. a_rfDistance is the maximum distance and outputs the distance to the hit.
TBOOL SweptSphereVsCapsule( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector4& a_rcBase, TFLOAT a_fCapsuleRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon );

// A ray from a_rcOrigin along a_rcDirection against the box a_rcCenter +- a_rcHalfExtents: TRUE (a_rfDistance: the
// entry parameter) if it's hit ahead of the origin
TBOOL RayVsAABB( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, TFLOAT& a_rfDistance );

// Intersection of the segment a_rcStart-a_rcEnd with a plane: TRUE (a_rOut: the point) if it crosses the plane
TBOOL LineIntersectsPlane3D( const TPlane& a_rcPlane, const TVector3& a_rcStart, const TVector3& a_rcEnd, TVector3& a_rOut );

// The closest points of the segments a_rcStart1-a_rcEnd1 and a_rcStart2-a_rcEnd2
void ClosestPointsBetweenLineSegments( const TVector4& a_rcStart1, const TVector4& a_rcEnd1, const TVector4& a_rcStart2, const TVector4& a_rcEnd2, TVector4& a_rOutPoint1, TVector4& a_rOutPoint2 );

// A ray against the triangle a_ppVertices (front side only): TRUE (a_rfDistance: the ray parameter) if it's hit
TBOOL RayVsTriangle( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance );

// The segment a_rcStart-a_rcEnd swept along a_rcDirection against the triangle: TRUE if it's hit closer than
// a_rfDistance (a_rfDistance: the distance to the hit)
TBOOL SweptSegmentVsTriangle( const TVector4& a_rcStart, const TVector4& a_rcEnd, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance );
TBOOL SweptSegmentVsTriangle( const TVector4& a_rcStart, const TVector4& a_rcEnd, TFLOAT a_fLength, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, const TVector4* const* a_ppEdgeDirs, TFLOAT& a_rfDistance );

// The normalised directions of the edges of the triangle (v0-v1, v1-v2, v2-v0)
void GetSweptSegmentTriangleEdgeDirs( const TVector4* const* a_ppVertices, TVector4* a_pOutEdgeDirs );

// The box a_rcCenter +- a_rcHalfExtents swept along a_rcDirection against the triangle: TRUE if it's hit closer than
// a_rfDistance (a_rfDistance: the distance to the hit)
TBOOL SweptAABBVsTriangle( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance );

// TRUE if a_rcPoint is on the side of the plane through a_rcPlanePoint that a_rcNormal points to (or on it)
TBOOL __stdcall IsPointInFrontOfPlane( const TVector3& a_rcNormal, const TVector3& a_rcPlanePoint, const TVector3& a_rcPoint );

// Distance along the segment a_rcStart-a_rcEnd of the point closest to a_rcPoint, clipped to the segment
TFLOAT ClippedClosestPointOnLine( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcEnd );

TBOOL    SphereIntersection( const TVector3& a_rcCentre1, TFLOAT a_fRadius1, const TVector3& a_rcCentre2, TFLOAT a_fRadius2, TFLOAT* a_pOutDistanceSq, TVector3* a_pOutPush, TFLOAT* a_pOutPenetration );
TBOOL    SphereIntersectionPlanar( const TVector3& a_rcCentre1, TFLOAT a_fRadius1, const TVector3& a_rcCentre2, TFLOAT a_fRadius2, TFLOAT* a_pOutDistanceSq, TVector3* a_pOutPush, TFLOAT* a_pOutPenetration );
TFLOAT   SphereIntersectionDynamic( const TVector3& a_rcPosA, TFLOAT a_fRadA, const TVector3& a_rcVelA, const TVector3& a_rcPosB, TFLOAT a_fRadB, const TVector3& a_rcVelB );
TBOOL    SphereInsideFrustum( const TPlane* a_pPlanes, TINT a_iNumPlanes, const TSphere& a_rcSphere, TBOOL& a_rbIntersects );
TBOOL    SweptSegmentVsSegment( const TVector3& a_rcStart1, const TVector3& a_rcEnd1, const TVector3& a_rcDirection, const TVector3& a_rcStart2, const TVector3& a_rcEnd2, TFLOAT& a_rfDistance, TBOOL a_bAllowNegative );
TBOOL    SweptAABBVsAABB( const TVector4& a_rcCenter1, const TVector4& a_rcHalfExtents1, const TVector4& a_rcDirection, const TVector4& a_rcCenter2, const TVector4& a_rcHalfExtents2, TFLOAT& a_rfDistance, TFLOAT a_fMinExit, TVector4* a_pOutNormal );
TINT     RayVsCircle3D( const TVector3& a_rcRayOrigin, const TVector3& a_rcRayDir, const TVector3& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfNearIntersection, TFLOAT& a_rfFarIntersection );
TINT     RayVsCircle3D( const TVector4& a_rcRayOrigin, const TVector4& a_rcRayDir, const TSphere& a_rcSphere, TFLOAT& a_rfNearIntersection, TFLOAT& a_rfFarIntersection );
TBOOL    RaySegmentVsLine2D( const TVector2& a_rcOrigin, const TVector2& a_rcDirection, const TVector2& a_rcLinePoint, const TVector2& a_rcLineDirection, TFLOAT& a_rfDistance );
void     CalculateCapsuleNormal2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, const TVector2& a_rcPoint, TVector2& a_rOutNormal );
TBOOL    PushPointOutOfCapsule2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fRadius, const TVector2& a_rcNormal, const TVector2& a_rcDirection, const TVector2& a_rcPoint, TFLOAT& a_rfDistance );
TBOOL    LineVsLine2D( const TVector2& a_rcLine1Start, const TVector2& a_rcLine1End, const TVector2& a_rcLine2Start, const TVector2& a_rcLine2End, TVector2& a_rOutPoint );
TBOOL    LineSegmentVsLine2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2, TVector2& a_rOutPoint );
TFLOAT   DistancePointToLineSegment( const TVector2& a_rcPoint, const TVector2& a_rcStart, const TVector2& a_rcEnd );
TFLOAT   DistancePointToLineSegmentSq( const TVector2& a_rcPoint, const TVector2& a_rcStart, const TVector2& a_rcEnd );
TFLOAT   DistancePointToLine( const TVector2& a_rcPoint, const TVector2& a_rcStart, const TVector2& a_rcEnd );
TFLOAT   DistancePointToLineSq( const TVector2& a_rcPoint, const TVector2& a_rcStart, const TVector2& a_rcEnd );
TFLOAT   DistancePointToLineSq( const TVector3& a_rcPoint, const TVector3& a_rcStart, const TVector3& a_rcEnd );
TFLOAT   ClosestPointOnLine( const TVector3& a_rcPoint, const TVector3& a_rcStart, const TVector3& a_rcEnd );
TFLOAT   ClosestPointOnLineOverTime2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, const TVector2& a_rcVelocity, const TVector2& a_rcPoint, TFLOAT a_fTime );
TBOOL    PointOnPlane3D( const TVector3& a_rcNormal, const TVector3& a_rcPlanePoint, const TVector3& a_rcPoint );
TBOOL    LineIntersectsPlane3D( const TPlane& a_rcPlane, const TVector3& a_rcStart, const TVector3& a_rcEnd );
void     GetAxisChangeLimit3D( const TVector3& a_rcFrom, const TVector3& a_rcTo, const TVector3& a_rcFallbackAxis, TFLOAT a_fMaxAngle, TVector3& a_rOut );
TFLOAT   AngleBetweenVectors2D( const TVector2& a_rcA, const TVector2& a_rcB );
TFLOAT   AngleBetweenXZ( const TVector3& a_vA, const TVector3& a_vB );
void     HermitePosition( TVector3& a_rvOut, const TVector3& a_rvStart, const TVector3& a_rvStartTangent, const TVector3& a_rvEnd, const TVector3& a_rvEndTangent, TFLOAT a_fT );
void     HermiteTangent( TVector3& a_rvOut, const TVector3& a_rvStart, const TVector3& a_rvStartTangent, const TVector3& a_rvEnd, const TVector3& a_rvEndTangent, TFLOAT a_fT );
TFLOAT   GetHermiteCurveLength( const TVector3& a_rvStart, const TVector3& a_rvStartTangent, const TVector3& a_rvEnd, const TVector3& a_rvEndTangent );
TFLOAT   GetHermiteCurveLength( const TVector4& a_rvStart, const TVector4& a_rvStartTangent, const TVector4& a_rvEnd, const TVector4& a_rvEndTangent );
TVector4 CalcCardinalSplineTangent( const TVector4& a_rcPrevious, const TVector4& a_rcNext, TFLOAT a_fTension );
void     NormaliseFloatArray( TFLOAT* a_pValues, TINT a_iCount, TFLOAT a_fTotal );
TBOOL    SphereVsPolygon( const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4& a_rNormal, const TVector3* const* a_ppVertices, TINT a_iNumVertices, TFLOAT a_fEpsilon );
TBOOL    SphereVsPolygon( const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4& a_rNormal, const TVector4* const* a_ppVertices, const TPlane& a_rcPlane, TINT a_iNumVertices, TFLOAT a_fEpsilon );
TBOOL    SphereVsPolygon( const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4& a_rNormal, const TVector4* const* a_ppVertices, TINT a_iNumVertices, TFLOAT a_fEpsilon );
void     SlideAlongPlane( const TVector4& a_rcDirection, const TVector4& a_rcNormal, TFLOAT a_fSpeedScale, TFLOAT a_fCompleteDist, TFLOAT a_fTravelledDistance, TVector4& a_rOutPosition, TVector4& a_rOutTravelled );
TBOOL    IsCollinearTriangle( const TVector4* const* a_ppVertices, TFLOAT a_fEpsilon );
TBOOL    IsCollinearTriangle( const TVector4& a_rcVertex1, const TVector4& a_rcVertex2, const TVector4& a_rcVertex3, TFLOAT a_fEpsilon );
void     Accelerate( TVector4& a_rPosition, TVector4& a_rVelocity, const TVector4& a_rcAcceleration, TFLOAT a_fDeltaTime );
TINT     GetCapsuleZone( const TVector2& a_rcPoint, const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fRadius );

} // namespace TMath

TOSHI_NAMESPACE_END

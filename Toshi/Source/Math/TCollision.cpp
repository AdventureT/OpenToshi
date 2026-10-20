#include "ToshiPCH.h"
#include "TCollision.h"
#include "TMath.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00828da0
TBOOL TCollision::AABBvsAABB( const TVector4& a_rcCenter1, const TVector4& a_rcHalfExtents1, const TVector4& a_rcCenter2, const TVector4& a_rcHalfExtents2 )
{
	return a_rcCenter1.x - a_rcHalfExtents1.x <= a_rcHalfExtents2.x + a_rcCenter2.x && a_rcCenter2.x - a_rcHalfExtents2.x <= a_rcHalfExtents1.x + a_rcCenter1.x &&
	    a_rcCenter1.y - a_rcHalfExtents1.y <= a_rcHalfExtents2.y + a_rcCenter2.y && a_rcCenter2.y - a_rcHalfExtents2.y <= a_rcHalfExtents1.y + a_rcCenter1.y &&
	    a_rcCenter1.z - a_rcHalfExtents1.z <= a_rcHalfExtents2.z + a_rcCenter2.z && a_rcCenter2.z - a_rcHalfExtents2.z <= a_rcHalfExtents1.z + a_rcCenter1.z;
}

// $deBlob: FUNCTION 00828e60
TBOOL TCollision::AABBSweptvsAABB( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, const TVector4& a_rcBoxCenter, const TVector4& a_rcBoxHalfExtents, TVector4& a_rOutNormal, TFLOAT& a_rfDistance )
{
	const TFLOAT fEpsilon = 1.1920929e-7f;

	TFLOAT fEnter = -TMath::MAXFLOAT;
	TFLOAT fExit  = TMath::MAXFLOAT;
	TINT   iAxis  = -1;

	for ( TINT i = 0; i < 3; i++ )
	{
		const TFLOAT fDir       = ( &a_rcDirection.x )[ i ];
		const TFLOAT fCenter    = ( &a_rcCenter.x )[ i ];
		const TFLOAT fExtent    = ( &a_rcHalfExtents.x )[ i ];
		const TFLOAT fBoxCenter = ( &a_rcBoxCenter.x )[ i ];
		const TFLOAT fBoxExtent = ( &a_rcBoxHalfExtents.x )[ i ];

		if ( fDir < -fEpsilon )
		{
			// Moving down the axis: the box has to be below the swept one's start
			if ( fBoxCenter - fBoxExtent > fExtent + fCenter )
				return TFALSE;

			TFLOAT fSum  = fBoxExtent + fExtent;
			TFLOAT fDiff = fBoxCenter - fCenter;
			TFLOAT fT    = ( fSum + fDiff ) * ( 1.0f / fDir );

			if ( fT > fEnter )
			{
				fEnter = fT;
				iAxis  = i;
			}

			fT = ( fDiff - fSum ) * ( 1.0f / fDir );

			if ( !( fExit <= fT ) )
				fExit = fT;
		}
		else if ( fDir > fEpsilon )
		{
			if ( fCenter - fExtent > fBoxExtent + fBoxCenter )
				return TFALSE;

			TFLOAT fDiff = fBoxCenter - fCenter;
			TFLOAT fSum  = fBoxExtent + fExtent;
			TFLOAT fT    = ( fDiff - fSum ) * ( 1.0f / fDir );

			if ( fT > fEnter )
			{
				fEnter = fT;
				iAxis  = i;
			}

			fT = ( fSum + fDiff ) * ( 1.0f / fDir );

			if ( !( fExit <= fT ) )
				fExit = fT;
		}
		else
		{
			// Not moving along the axis: they have to overlap on it
			TFLOAT fDiff = fBoxCenter - fCenter;
			TFLOAT fSum  = fBoxExtent + fExtent;

			if ( fDiff - fSum > -fEpsilon || fSum + fDiff < fEpsilon )
				return TFALSE;
		}
	}

	if ( fExit < fEnter )
		return TFALSE;

	if ( a_rfDistance < fEnter )
		return TFALSE;

	a_rOutNormal.Set( 0.0f, 0.0f, 0.0f, 1.0f );

	// NOTE: retail writes the normal's component iAxis even when it stayed -1 (the box doesn't move on any axis), one
	// float before a_rOutNormal
#ifdef DEBLOB_COMMUNITY_PATCH
	if ( iAxis >= 0 )
#endif
		( &a_rOutNormal.x )[ iAxis ] = -TMath::Sign( ( &a_rcDirection.x )[ iAxis ] );

	a_rfDistance = ( fEnter <= 0.0f ) ? 0.0f : fEnter;
	return TTRUE;
}

// $deBlob: FUNCTION 008295d0
TINT TCollision::AABBSweptvsPlane( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, const TPlane& a_rcPlane, TFLOAT& a_rfDistance )
{
	const TVector3& rcNormal = a_rcPlane.GetNormal();

	// Moving away from the plane
	if ( a_rcDirection.y * rcNormal.y + a_rcDirection.x * rcNormal.x + a_rcDirection.z * rcNormal.z >= 0.0f )
		return AABBSWEPT_NO_HIT;

	TFLOAT fPlaneDistance = a_rcPlane.GetD();

	// The extent of the box along the normal
	TVector4 vAbsNormal( rcNormal.x, rcNormal.y, rcNormal.z, fPlaneDistance );
	vAbsNormal.Abs4();

	TFLOAT fExtent   = a_rcHalfExtents.y * vAbsNormal.y + a_rcHalfExtents.x * vAbsNormal.x + a_rcHalfExtents.z * vAbsNormal.z;
	TFLOAT fDistance = a_rcCenter.y * rcNormal.y + a_rcCenter.x * rcNormal.x + rcNormal.z * a_rcCenter.z;

	if ( fExtent >= TMath::Abs( fDistance - fPlaneDistance ) )
	{
		a_rfDistance = 0.0f;
		return AABBSWEPT_TOUCHING;
	}

	TVector4 vMove;
	vMove.Multiply( a_rcDirection, a_rfDistance );

	TFLOAT fMoveDot = rcNormal.x * vMove.x + vMove.y * rcNormal.y + rcNormal.z * vMove.z;
	TFLOAT fT       = ( ( fExtent + fPlaneDistance ) - fDistance ) / fMoveDot;

	if ( fT < 0.0f || fT > 1.0f )
		return AABBSWEPT_NO_HIT;

	TVector4 vHitMove;
	vHitMove.Multiply( a_rcDirection, ( ( fExtent + fPlaneDistance ) - fDistance ) / fMoveDot );
	vHitMove.Multiply( a_rfDistance );

	TVector4 vHit( a_rcCenter.x + vHitMove.x, a_rcCenter.y + vHitMove.y, vHitMove.z + a_rcCenter.z, a_rcCenter.w );
	a_rfDistance = TVector4::Distance( vHit, a_rcCenter );

	return AABBSWEPT_HIT;
}

// The corners of the unit box, then its 12 edges (start, end); scaled by the half extents. NOTE: TMath.cpp has its
// own copy of both tables
static const TVector4 s_avCornerDims[ 8 ] = {
	{ -1.0f, -1.0f, 1.0f, 1.0f },
	{ -1.0f, 1.0f, 1.0f, 1.0f },
	{ 1.0f, -1.0f, 1.0f, 1.0f },
	{ 1.0f, 1.0f, 1.0f, 1.0f },
	{ -1.0f, -1.0f, -1.0f, 1.0f },
	{ -1.0f, 1.0f, -1.0f, 1.0f },
	{ 1.0f, -1.0f, -1.0f, 1.0f },
	{ 1.0f, 1.0f, -1.0f, 1.0f },
};

static const TVector4 s_avEdgeDims[ 12 ][ 2 ] = {
	{ { -1.0f, -1.0f, -1.0f, 1.0f }, { 1.0f, -1.0f, -1.0f, 1.0f } },
	{ { -1.0f, -1.0f, -1.0f, 1.0f }, { -1.0f, 1.0f, -1.0f, 1.0f } },
	{ { 1.0f, 1.0f, -1.0f, 1.0f }, { 1.0f, -1.0f, -1.0f, 1.0f } },
	{ { 1.0f, 1.0f, -1.0f, 1.0f }, { -1.0f, 1.0f, -1.0f, 1.0f } },
	{ { -1.0f, -1.0f, 1.0f, 1.0f }, { 1.0f, -1.0f, 1.0f, 1.0f } },
	{ { -1.0f, -1.0f, 1.0f, 1.0f }, { -1.0f, 1.0f, 1.0f, 1.0f } },
	{ { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, -1.0f, 1.0f, 1.0f } },
	{ { 1.0f, 1.0f, 1.0f, 1.0f }, { -1.0f, 1.0f, 1.0f, 1.0f } },
	{ { -1.0f, -1.0f, -1.0f, 1.0f }, { -1.0f, -1.0f, 1.0f, 1.0f } },
	{ { -1.0f, 1.0f, -1.0f, 1.0f }, { -1.0f, 1.0f, 1.0f, 1.0f } },
	{ { 1.0f, -1.0f, -1.0f, 1.0f }, { 1.0f, -1.0f, 1.0f, 1.0f } },
	{ { 1.0f, 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
};

// $deBlob: FUNCTION 00829100
TBOOL TCollision::AABBSweptvsTriangle( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance )
{
	const TVector4& rcVertex0 = *a_ppVertices[ 0 ];

	TVector4 vEdge1 = *a_ppVertices[ 1 ] - rcVertex0;
	vEdge1.w        = a_ppVertices[ 1 ]->w;
	TVector4 vEdge2 = *a_ppVertices[ 2 ] - rcVertex0;
	vEdge2.w        = a_ppVertices[ 2 ]->w;

	TVector4 vNormal;
	vNormal.CrossProduct( vEdge1, vEdge2 );
	vNormal.Normalise();

	// Moving away from the triangle
	if ( !( vNormal.y * a_rcDirection.y + a_rcDirection.x * vNormal.x + a_rcDirection.z * vNormal.z < 0.0f ) )
		return TFALSE;

	TBOOL  bHit = TFALSE;
	TFLOAT fT;

	// The corners of the box against the triangle
	for ( TINT i = 0; i < TARRAYSIZE( s_avCornerDims ); i++ )
	{
		const TVector4& rcCorner = s_avCornerDims[ i ];

		TVector4 vCorner( rcCorner.x * a_rcHalfExtents.x + a_rcCenter.x, rcCorner.y * a_rcHalfExtents.y + a_rcCenter.y, rcCorner.z * a_rcHalfExtents.z + a_rcCenter.z, a_rcCenter.w );

		if ( TMath::RayVsTriangle( vCorner, a_rcDirection, a_ppVertices, fT ) && fT > -TMath::FLOATEPSILON && a_rfDistance > fT )
		{
			a_rfDistance = fT;
			bHit         = TTRUE;

			if ( fT <= 0.0f )
				return TTRUE;
		}
	}

	// The vertices of the triangle moving the other way against the box
	TVector4 vBackDir( -a_rcDirection.x, -a_rcDirection.y, -a_rcDirection.z, a_rcDirection.w );

	for ( TINT i = 0; i < 3; i++ )
	{
		if ( TMath::RayVsAABB( *a_ppVertices[ i ], vBackDir, a_rcCenter, a_rcHalfExtents, fT ) && fT > -TMath::FLOATEPSILON && a_rfDistance > fT )
		{
			a_rfDistance = fT;
			bHit         = TTRUE;

			if ( fT <= 0.0f )
				return TTRUE;
		}
	}

	// The edges of the box against the triangle
	TVector4 aEdgeDirs[ 3 ];
	TMath::GetSweptSegmentTriangleEdgeDirs( a_ppVertices, aEdgeDirs );

	const TVector4* apEdgeDirs[ 3 ] = { &aEdgeDirs[ 0 ], &aEdgeDirs[ 1 ], &aEdgeDirs[ 2 ] };

	// NOTE: retail initialises this static and never reads it
	static TFLOAT s_fOneOverSqrt8 = 1.0f / TFLOAT( sqrt( 8.0 ) );
	(void)s_fOneOverSqrt8;

	for ( TINT i = 0; i < TARRAYSIZE( s_avEdgeDims ); i++ )
	{
		const TVector4& rcEdgeStart = s_avEdgeDims[ i ][ 0 ];
		const TVector4& rcEdgeEnd   = s_avEdgeDims[ i ][ 1 ];

		// Leading edges only
		if ( ( rcEdgeEnd.y + rcEdgeStart.y ) * a_rcDirection.y + a_rcDirection.x * ( rcEdgeEnd.x + rcEdgeStart.x ) + a_rcDirection.z * ( rcEdgeStart.z + rcEdgeEnd.z ) < 0.0f )
			continue;

		TVector4 vStart( a_rcHalfExtents.x * rcEdgeStart.x + a_rcCenter.x, a_rcHalfExtents.y * rcEdgeStart.y + a_rcCenter.y, a_rcHalfExtents.z * rcEdgeStart.z + a_rcCenter.z, a_rcCenter.w );
		TVector4 vEnd( a_rcHalfExtents.x * rcEdgeEnd.x + a_rcCenter.x, a_rcHalfExtents.y * rcEdgeEnd.y + a_rcCenter.y, a_rcHalfExtents.z * rcEdgeEnd.z + a_rcCenter.z, a_rcCenter.w );

		// The axis of the edge
		TVector4 vEdge( rcEdgeEnd.x - rcEdgeStart.x, rcEdgeEnd.y - rcEdgeStart.y, rcEdgeEnd.z - rcEdgeStart.z, rcEdgeEnd.w );
		vEdge.Abs4();

		TINT iAxis = 0;

		if ( vEdge.x <= vEdge.y )
			iAxis = ( vEdge.z > vEdge.y ) + 1;
		else if ( vEdge.z > vEdge.x )
			iAxis = 2;

		if ( TMath::SweptSegmentVsTriangle( vStart, vEnd, ( &a_rcHalfExtents.x )[ iAxis ] * 2.0f, a_rcDirection, a_ppVertices, apEdgeDirs, a_rfDistance ) )
		{
			bHit = TTRUE;

			if ( a_rfDistance <= 0.0f )
				return TTRUE;
		}
	}

	return bHit;
}

// $deBlob: FUNCTION 008297a0
void TCollision::AABBSweptArea( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, TFLOAT a_fDistance, TVector4& a_rOutCenter, TVector4& a_rOutHalfExtents )
{
	a_rOutCenter = a_rcDirection;
	a_rOutCenter.Multiply( a_fDistance * 0.5f );

	a_rOutHalfExtents = a_rOutCenter;
	a_rOutHalfExtents.Abs4();
	a_rOutHalfExtents.x = a_rcHalfExtents.x + a_rOutHalfExtents.x;
	a_rOutHalfExtents.y = a_rcHalfExtents.y + a_rOutHalfExtents.y;
	a_rOutHalfExtents.z = a_rcHalfExtents.z + a_rOutHalfExtents.z;

	a_rOutCenter.x = a_rcCenter.x + a_rOutCenter.x;
	a_rOutCenter.y = a_rcCenter.y + a_rOutCenter.y;
	a_rOutCenter.z = a_rcCenter.z + a_rOutCenter.z;
}

// $deBlob: FUNCTION 00829840
TBOOL TCollision::AABBvsOBB( const TVector4& a_rcCenter, TVector4 a_vHalfExtents, TVector4 a_vOBBCenter, const TMatrix44& a_rcOBBRotation, const TVector4& a_rcOBBHalfExtents )
{
	const TFLOAT* a = &a_vHalfExtents.x;
	const TFLOAT* b = &a_rcOBBHalfExtents.x;

	TVector4      vT( a_vOBBCenter.x - a_rcCenter.x, a_vOBBCenter.y - a_rcCenter.y, a_vOBBCenter.z - a_rcCenter.z, a_vOBBCenter.w );
	const TFLOAT* T = &vT.x;

	// R[ i ][ j ]: the i component of the OBB's axis j
	TFLOAT R[ 3 ][ 4 ];

	// The axes of the AABB
	for ( TINT i = 0; i < 3; i++ )
	{
		R[ i ][ 0 ] = ( &a_rcOBBRotation.AsBasisVector4( 0 ).x )[ i ];
		R[ i ][ 1 ] = ( &a_rcOBBRotation.AsBasisVector4( 1 ).x )[ i ];
		R[ i ][ 2 ] = ( &a_rcOBBRotation.AsBasisVector4( 2 ).x )[ i ];

		if ( TMath::Abs( T[ i ] ) > ( TMath::Abs( R[ i ][ 0 ] ) * b[ 0 ] + TMath::Abs( R[ i ][ 1 ] ) * b[ 1 ] ) + TMath::Abs( R[ i ][ 2 ] ) * b[ 2 ] + a[ i ] )
			return TFALSE;
	}

	// The axes of the OBB
	for ( TINT i = 0; i < 3; i++ )
	{
		if ( TMath::Abs( ( R[ 0 ][ i ] * T[ 0 ] + R[ 1 ][ i ] * T[ 1 ] ) + R[ 2 ][ i ] * T[ 2 ] ) > ( TMath::Abs( R[ 0 ][ i ] ) * a[ 0 ] + TMath::Abs( R[ 1 ][ i ] ) * a[ 1 ] ) + TMath::Abs( R[ 2 ][ i ] ) * a[ 2 ] + b[ i ] )
			return TFALSE;
	}

	// The cross products of the axes
	TFLOAT fAbsR02 = TMath::Abs( R[ 0 ][ 2 ] );
	TFLOAT fAbsR01 = TMath::Abs( R[ 0 ][ 1 ] );
	TFLOAT fAbsR10 = TMath::Abs( R[ 1 ][ 0 ] );
	TFLOAT fAbsR20 = TMath::Abs( R[ 2 ][ 0 ] );

	if ( TMath::Abs( R[ 1 ][ 0 ] * T[ 2 ] - R[ 2 ][ 0 ] * T[ 1 ] ) > ( b[ 2 ] * fAbsR01 + b[ 1 ] * fAbsR02 ) + ( fAbsR20 * a[ 1 ] + fAbsR10 * a[ 2 ] ) )
		return TFALSE;

	TFLOAT fAbsR00 = TMath::Abs( R[ 0 ][ 0 ] );
	TFLOAT fAbsR11 = TMath::Abs( R[ 1 ][ 1 ] );
	TFLOAT fAbsR21 = TMath::Abs( R[ 2 ][ 1 ] );

	if ( TMath::Abs( R[ 1 ][ 1 ] * T[ 2 ] - R[ 2 ][ 1 ] * T[ 1 ] ) > ( fAbsR00 * b[ 2 ] + b[ 0 ] * fAbsR02 ) + ( fAbsR21 * a[ 1 ] + fAbsR11 * a[ 2 ] ) )
		return TFALSE;

	TFLOAT fAbsR12 = TMath::Abs( R[ 1 ][ 2 ] );
	TFLOAT fAbsR22 = TMath::Abs( R[ 2 ][ 2 ] );

	if ( TMath::Abs( R[ 1 ][ 2 ] * T[ 2 ] - R[ 2 ][ 2 ] * T[ 1 ] ) > ( b[ 0 ] * fAbsR01 + fAbsR00 * b[ 1 ] ) + ( fAbsR22 * a[ 1 ] + fAbsR12 * a[ 2 ] ) )
		return TFALSE;

	if ( TMath::Abs( R[ 2 ][ 0 ] * T[ 0 ] - R[ 0 ][ 0 ] * T[ 2 ] ) > ( fAbsR12 * b[ 1 ] + fAbsR11 * b[ 2 ] ) + ( fAbsR00 * a[ 2 ] + fAbsR20 * a[ 0 ] ) )
		return TFALSE;

	if ( TMath::Abs( R[ 2 ][ 1 ] * T[ 0 ] - R[ 0 ][ 1 ] * T[ 2 ] ) > ( fAbsR12 * b[ 0 ] + fAbsR10 * b[ 2 ] ) + ( fAbsR21 * a[ 0 ] + fAbsR01 * a[ 2 ] ) )
		return TFALSE;

	if ( TMath::Abs( R[ 2 ][ 2 ] * T[ 0 ] - R[ 0 ][ 2 ] * T[ 2 ] ) > ( fAbsR11 * b[ 0 ] + fAbsR10 * b[ 1 ] ) + ( fAbsR22 * a[ 0 ] + fAbsR02 * a[ 2 ] ) )
		return TFALSE;

	if ( TMath::Abs( R[ 0 ][ 0 ] * T[ 1 ] - R[ 1 ][ 0 ] * T[ 0 ] ) > ( fAbsR22 * b[ 1 ] + fAbsR21 * b[ 2 ] ) + ( fAbsR00 * a[ 1 ] + fAbsR10 * a[ 0 ] ) )
		return TFALSE;

	if ( TMath::Abs( R[ 0 ][ 1 ] * T[ 1 ] - R[ 1 ][ 1 ] * T[ 0 ] ) > ( fAbsR22 * b[ 0 ] + fAbsR20 * b[ 2 ] ) + ( fAbsR11 * a[ 0 ] + fAbsR01 * a[ 1 ] ) )
		return TFALSE;

	return TMath::Abs( R[ 0 ][ 2 ] * T[ 1 ] - R[ 1 ][ 2 ] * T[ 0 ] ) <= ( fAbsR21 * b[ 0 ] + fAbsR20 * b[ 1 ] ) + ( fAbsR12 * a[ 0 ] + fAbsR02 * a[ 1 ] );
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TOBBox.h"
#include "TMath.h"

TOSHI_NAMESPACE_START

static TVector4 GetClosestPointOnLineToRay( const TVector4& a_rcSegmentStart, const TVector4& a_rcSegmentVector, const TVector4& a_rcLinePoint, const TVector4& a_rcLineDirection );
static void     LineClosestApproach( const TVector4& a_rcPointA, const TVector4& a_rcDirA, const TVector4& a_rcPointB, const TVector4& a_rcDirB, TFLOAT& a_rfAlpha, TFLOAT& a_rfBeta );
static TINT     IntersectRectQuad( const TFLOAT* a_pRectangle, TFLOAT* a_pQuad, TFLOAT* a_pOutPoints );

// NOTE: TVector4::Magnitude asserts on zero vectors, the original uses fsqrt
static TFLOAT VectorLength( const TVector4& a_rcVector )
{
	return sqrtf( a_rcVector.x * a_rcVector.x + a_rcVector.y * a_rcVector.y + a_rcVector.z * a_rcVector.z );
}

// $Barnyard: FUNCTION 006d86d0
TCapsule::TCapsule( const TVector4& a_rcCentre, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT a_fLength )
{
	vCentre         = a_rcCentre;
	vDirection      = a_rcDirection;
	fBoundingRadius = a_fLength * 0.5f + a_fRadius;
	fRadius         = a_fRadius;
	fLength         = a_fLength;
}

// $Barnyard: FUNCTION 006d8960
TCollisionTriangle::TCollisionTriangle( const TVector4& a_rcVertex1, const TVector4& a_rcVertex2, const TVector4& a_rcVertex3, const TVector4& a_rcNormal )
{
	aVertices[ 0 ] = a_rcVertex1;
	aVertices[ 1 ] = a_rcVertex2;
	aVertices[ 2 ] = a_rcVertex3;
	vNormal        = a_rcNormal;
}

// $deBlob: FUNCTION 00829e60
// $Barnyard: FUNCTION 006cf530
TOBBox::TOBBox()
{
	m_oTransform = TMatrix44::IDENTITY;
	m_vExtents   = TVector4::VEC_ZERO;
	m_fRadius    = 0.0f;
	m_fMinExtent = 0.0f;
}

// $deBlob: FUNCTION 00829ec0
// $Barnyard: FUNCTION 006d0db0
TOBBox::TOBBox( const TMatrix44& a_rcTransform, const TVector4& a_rcExtents )
{
	m_oTransform = a_rcTransform;
	m_vExtents   = a_rcExtents;
	m_fRadius    = m_vExtents.Magnitude();
	m_fMinExtent = m_vExtents.x;
	m_fMinExtent = ( m_vExtents.y < m_fMinExtent ) ? m_vExtents.y : m_fMinExtent;
	m_fMinExtent = ( m_vExtents.z < m_fMinExtent ) ? m_vExtents.z : m_fMinExtent;
}

// $deBlob: FUNCTION 00829f50
// $Barnyard: FUNCTION 006d0e30
TOBBox::TOBBox( const TOBBox& a_rcOther )
    : TOBBox( a_rcOther.m_oTransform, a_rcOther.m_vExtents )
{
}

// $deBlob: FUNCTION 00829fd0
// $Barnyard: FUNCTION 006cf580
TOBBox& TOBBox::operator=( const TOBBox& a_rcOther )
{
	m_oTransform = a_rcOther.m_oTransform;
	m_vExtents   = a_rcOther.m_vExtents;
	m_fRadius    = a_rcOther.m_fRadius;
	m_fMinExtent = a_rcOther.m_fMinExtent;

	return *this;
}

// $deBlob: FUNCTION 0082a020
TINT TOBBox::ComparePlane( const TPlane& a_rcPlane ) const
{
	const TVector3& rcNormal = a_rcPlane.GetNormal();

	TFLOAT fRadius = TMath::Abs( m_oTransform.m_f12 * rcNormal.y + m_oTransform.m_f11 * rcNormal.x + m_oTransform.m_f13 * rcNormal.z ) * m_vExtents.x;
	fRadius        = TMath::Abs( m_oTransform.m_f22 * rcNormal.y + m_oTransform.m_f21 * rcNormal.x + m_oTransform.m_f23 * rcNormal.z ) * m_vExtents.y + fRadius;
	fRadius        = fRadius + TMath::Abs( m_oTransform.m_f32 * rcNormal.y + m_oTransform.m_f31 * rcNormal.x + m_oTransform.m_f33 * rcNormal.z ) * m_vExtents.z;

	TFLOAT fDistance = ( m_oTransform.m_f42 * rcNormal.y + m_oTransform.m_f41 * rcNormal.x + m_oTransform.m_f43 * rcNormal.z ) - a_rcPlane.GetD();

	if ( -fRadius <= fDistance )
		return ( fDistance <= fRadius ) ? 0 : -1;

	return 1;
}

// $deBlob: FUNCTION 0082a130
TBOOL TOBBox::PointInside( const TVector3& a_rcPoint ) const
{
	TVector4 vPoint( a_rcPoint.x, a_rcPoint.y, a_rcPoint.z, 1.0f );

	TMatrix44 oInverse = m_oTransform;
	oInverse.Invert( oInverse );

	TVector4 vLocal;
	TMatrix44::TransformVector( vLocal, oInverse, vPoint );

	return TMath::Abs( vLocal.x ) <= m_vExtents.x && TMath::Abs( vLocal.y ) <= m_vExtents.y && TMath::Abs( vLocal.z ) <= m_vExtents.z;
}

// $deBlob: FUNCTION 0082a200
TBOOL TOBBox::PointInside( const TVector4& a_rcPoint ) const
{
	TMatrix44 oInverse = m_oTransform;
	oInverse.Invert( oInverse );

	TVector4 vLocal;
	TMatrix44::TransformVector( vLocal, oInverse, a_rcPoint );

	return TMath::Abs( vLocal.x ) <= m_vExtents.x && TMath::Abs( vLocal.y ) <= m_vExtents.y && TMath::Abs( vLocal.z ) <= m_vExtents.z;
}

// $deBlob: FUNCTION 0082a2b0
// $Barnyard: FUNCTION 006cf5c0
void TOBBox::ClosestPoint( const TVector4& a_rcPoint, TVector4& a_rOutPoint ) const
{
	TVector4 vToPoint;
	vToPoint.Substract( a_rcPoint, GetCentre() );

	a_rOutPoint = GetCentre();

	for ( TINT i = 0; i < 3; i++ )
	{
		const TVector4& rcAxis    = GetAxis( i );
		TFLOAT          fExtent   = m_vExtents.AsArray()[ i ];
		TFLOAT          fDistance = TVector4::DotProduct3( vToPoint, rcAxis );

		if ( fDistance > fExtent ) fDistance = fExtent;
		if ( fDistance < -fExtent ) fDistance = -fExtent;

		TVector4 vOffset;
		vOffset.Multiply( rcAxis, fDistance );
		a_rOutPoint.Add( vOffset );
	}
}

// $deBlob: FUNCTION 0082a480
void TOBBox::SetAxes( const TVector4& a_rcAxisX, const TVector4& a_rcAxisY, const TVector4& a_rcAxisZ )
{
	TASSERT( a_rcAxisX.w == 0.0f );
	m_oTransform.SetBasisVector4( BASISVECTOR_RIGHT, a_rcAxisX );

	TASSERT( a_rcAxisY.w == 0.0f );
	m_oTransform.SetBasisVector4( BASISVECTOR_UP, a_rcAxisY );

	TASSERT( a_rcAxisZ.w == 0.0f );
	m_oTransform.SetBasisVector4( BASISVECTOR_FORWARD, a_rcAxisZ );
}

// $deBlob: FUNCTION 0082a570
// $Barnyard: FUNCTION 006d3b20
TFLOAT TOBBox::IntersectTriangle( const TOBBox& a_rcBox, const TCollisionTriangle& a_rcTriangle, TVector4& a_rOutPoint, TVector4& a_rOutNormal )
{
	if ( !Intersects( a_rcBox, a_rcTriangle ) )
		return 2.0f;

	TVector4 vOffset;
	vOffset.Multiply( a_rcTriangle.vNormal, a_rcBox.m_fRadius );

	// Start outside of the triangle and sweep into it
	TOBBox oBox( a_rcBox );

	TVector4 vCentre(
	    a_rcBox.m_oTransform.m_f41 + vOffset.x,
	    a_rcBox.m_oTransform.m_f42 + vOffset.y,
	    a_rcBox.m_oTransform.m_f43 + vOffset.z,
	    a_rcBox.m_oTransform.m_f44
	);

	TASSERT( vCentre.w == 1.0f );
	oBox.m_oTransform.SetTranslation( vCentre );

	TVector4 vMovement;
	vMovement.Multiply( vOffset, -1.0f );

	return IntersectTriangle( oBox, vMovement, a_rcTriangle, a_rOutPoint, a_rOutNormal );
}

// $deBlob: FUNCTION 0082a670
// $Barnyard: FUNCTION 006d3ab0
TFLOAT TOBBox::IntersectTriangle( const TOBBox& a_rcPrevBox, const TOBBox& a_rcBox, const TCollisionTriangle& a_rcTriangle, TVector4& a_rOutPoint, TVector4& a_rOutNormal )
{
	TVector4 vMovement(
	    a_rcBox.m_oTransform.m_f41 - a_rcPrevBox.m_oTransform.m_f41,
	    a_rcBox.m_oTransform.m_f42 - a_rcPrevBox.m_oTransform.m_f42,
	    a_rcBox.m_oTransform.m_f43 - a_rcPrevBox.m_oTransform.m_f43,
	    a_rcBox.m_oTransform.m_f44
	);

	return IntersectTriangle( a_rcPrevBox, vMovement, a_rcTriangle, a_rOutPoint, a_rOutNormal );
}

// $deBlob: FUNCTION 0082a6d0
TBOOL TOBBox::Intersects( const TOBBox& a_rcBox, const TRay& a_rcRay )
{
	TMatrix44 oInverse = a_rcBox.m_oTransform;
	oInverse.Invert( oInverse );

	// The ray in the space of the box
	TVector4 vOffset(
	    a_rcRay.vOrigin.x - a_rcBox.m_oTransform.m_f41,
	    a_rcRay.vOrigin.y - a_rcBox.m_oTransform.m_f42,
	    a_rcRay.vOrigin.z - a_rcBox.m_oTransform.m_f43,
	    a_rcRay.vOrigin.w
	);

	TVector4 vOrigin;
	TMatrix44::RotateVector( vOrigin, oInverse, vOffset );

	TVector4 vDirection;
	TMatrix44::RotateVector( vDirection, oInverse, a_rcRay.vDirection );

	// Mirror the ray so that the direction is positive
	TFLOAT fOriginX = vOrigin.x;
	TFLOAT fDirX    = vDirection.x;

	if ( vDirection.x < 0.0f )
	{
		fOriginX = -vOrigin.x;
		fDirX    = -vDirection.x;
	}

	TFLOAT fDirY    = vDirection.y;
	TFLOAT fOriginY = vOrigin.y;

	if ( vDirection.y < 0.0f )
	{
		fOriginY = -vOrigin.y;
		fDirY    = -vDirection.y;
	}

	TFLOAT fDirZ    = vDirection.z;
	TFLOAT fOriginZ = vOrigin.z;

	if ( vDirection.z < 0.0f )
	{
		fOriginZ = -vOrigin.z;
		fDirZ    = -vDirection.z;
	}

	const TVector4 vExtents = a_rcBox.m_vExtents;

	if ( -vExtents.x > fOriginX && fDirX <= 0.0f )
		return TFALSE;

	if ( fOriginX > vExtents.x )
		return TFALSE;

	if ( -vExtents.y > fOriginY && fDirY <= 0.0f )
		return TFALSE;

	if ( fOriginY > vExtents.y ||
	     ( -vExtents.z > fOriginZ && fDirZ <= 0.0f ) ||
	     fOriginZ > vExtents.z ||
	     ( fDirX == 0.0f && fDirY == 0.0f && fDirZ == 0.0f ) )
	{
		return TFALSE;
	}

	TFLOAT fExit  = 3.402823466e+38f;
	TFLOAT fEnter = -3.402823466e+38f;

	if ( fDirX != 0.0f )
	{
		TFLOAT fT = ( -vExtents.x - fOriginX ) / fDirX;
		if ( fT > fEnter ) fEnter = fT;

		fT = ( vExtents.x - fOriginX ) / fDirX;
		if ( fT < 3.402823466e+38f ) fExit = fT;
	}

	if ( fDirY != 0.0f )
	{
		if ( ( -vExtents.y - fOriginY ) / fDirY > fEnter ) fEnter = ( -vExtents.y - fOriginY ) / fDirY;
		if ( fExit > ( vExtents.y - fOriginY ) / fDirY ) fExit = ( vExtents.y - fOriginY ) / fDirY;
	}

	if ( fDirZ != 0.0f )
	{
		if ( ( -vExtents.z - fOriginZ ) / fDirZ > fEnter ) fEnter = ( -vExtents.z - fOriginZ ) / fDirZ;
		if ( fExit > ( vExtents.z - fOriginZ ) / fDirZ ) fExit = ( vExtents.z - fOriginZ ) / fDirZ;
	}

	if ( fEnter > fExit )
		return TFALSE;

	if ( fEnter < 0.0f )
		fEnter = fExit;

	return fEnter >= 0.0f && fEnter <= a_rcRay.fLength;
}

// $deBlob: FUNCTION 0082a990
// $Barnyard: FUNCTION 006cf870
TBOOL TOBBox::Intersects( const TOBBox& a_rcBox1, const TOBBox& a_rcBox2 )
{
	TVector4 vDistance;
	vDistance.x = a_rcBox2.m_oTransform.m_f41 - a_rcBox1.m_oTransform.m_f41;
	vDistance.y = a_rcBox2.m_oTransform.m_f42 - a_rcBox1.m_oTransform.m_f42;
	vDistance.z = a_rcBox2.m_oTransform.m_f43 - a_rcBox1.m_oTransform.m_f43;
	vDistance.w = a_rcBox2.m_oTransform.m_f44;

	TMatrix44 oInverse = a_rcBox1.m_oTransform;
	oInverse.Invert( oInverse );

	// Distance in the space of the first box
	TVector4 vT;
	TMatrix44::RotateVector( vT, oInverse, vDistance );

	const TVector4& a0 = a_rcBox1.GetAxis( 0 );
	const TVector4& a1 = a_rcBox1.GetAxis( 1 );
	const TVector4& a2 = a_rcBox1.GetAxis( 2 );
	const TVector4& b0 = a_rcBox2.GetAxis( 0 );
	const TVector4& b1 = a_rcBox2.GetAxis( 1 );
	const TVector4& b2 = a_rcBox2.GetAxis( 2 );

	TFLOAT fA0 = a_rcBox1.m_vExtents.x;
	TFLOAT fA1 = a_rcBox1.m_vExtents.y;
	TFLOAT fA2 = a_rcBox1.m_vExtents.z;
	TFLOAT fB0 = a_rcBox2.m_vExtents.x;
	TFLOAT fB1 = a_rcBox2.m_vExtents.y;
	TFLOAT fB2 = a_rcBox2.m_vExtents.z;

	TFLOAT R00 = TVector4::DotProduct3( a0, b0 );
	TFLOAT R01 = TVector4::DotProduct3( a0, b1 );
	TFLOAT R02 = TVector4::DotProduct3( a0, b2 );
	TFLOAT Q00 = TMath::Abs( R00 );
	TFLOAT Q01 = TMath::Abs( R01 );
	TFLOAT Q02 = TMath::Abs( R02 );

	if ( Q02 * fB2 + Q01 * fB1 + Q00 * fB0 + fA0 < TMath::Abs( vT.x ) )
		return TFALSE;

	TFLOAT R10 = TVector4::DotProduct3( a1, b0 );
	TFLOAT R11 = TVector4::DotProduct3( a1, b1 );
	TFLOAT R12 = TVector4::DotProduct3( a1, b2 );
	TFLOAT Q10 = TMath::Abs( R10 );
	TFLOAT Q11 = TMath::Abs( R11 );
	TFLOAT Q12 = TMath::Abs( R12 );

	if ( Q12 * fB2 + Q11 * fB1 + Q10 * fB0 + fA1 < TMath::Abs( vT.y ) )
		return TFALSE;

	TFLOAT R20 = TVector4::DotProduct3( a2, b0 );
	TFLOAT R21 = TVector4::DotProduct3( a2, b1 );
	TFLOAT R22 = TVector4::DotProduct3( a2, b2 );
	TFLOAT Q20 = TMath::Abs( R20 );
	TFLOAT Q21 = TMath::Abs( R21 );
	TFLOAT Q22 = TMath::Abs( R22 );

	if ( Q22 * fB2 + Q21 * fB1 + Q20 * fB0 + fA2 < TMath::Abs( vT.z ) )
		return TFALSE;

	return Q20 * fA2 + Q10 * fA1 + Q00 * fA0 + fB0 >= TMath::Abs( TVector4::DotProduct3( b0, vDistance ) ) &&
	    Q21 * fA2 + Q11 * fA1 + Q01 * fA0 + fB1 >= TMath::Abs( TVector4::DotProduct3( b1, vDistance ) ) &&
	    Q22 * fA2 + Q12 * fA1 + Q02 * fA0 + fB2 >= TMath::Abs( TVector4::DotProduct3( b2, vDistance ) ) &&
	    Q20 * fA1 + Q10 * fA2 + Q01 * fB2 + Q02 * fB1 >= TMath::Abs( vT.z * R10 - R20 * vT.y ) &&
	    Q21 * fA1 + Q11 * fA2 + Q00 * fB2 + Q02 * fB0 >= TMath::Abs( vT.z * R11 - R21 * vT.y ) &&
	    Q22 * fA1 + Q12 * fA2 + Q00 * fB1 + Q01 * fB0 >= TMath::Abs( vT.z * R12 - R22 * vT.y ) &&
	    Q20 * fA0 + Q11 * fB2 + Q12 * fB1 + Q00 * fA2 >= TMath::Abs( R20 * vT.x - vT.z * R00 ) &&
	    Q21 * fA0 + Q10 * fB2 + Q12 * fB0 + Q01 * fA2 >= TMath::Abs( R21 * vT.x - vT.z * R01 ) &&
	    Q22 * fA0 + Q10 * fB1 + Q11 * fB0 + Q02 * fA2 >= TMath::Abs( R22 * vT.x - vT.z * R02 ) &&
	    Q21 * fB2 + Q22 * fB1 + Q10 * fA0 + Q00 * fA1 >= TMath::Abs( vT.y * R00 - R10 * vT.x ) &&
	    Q20 * fB2 + Q22 * fB0 + Q11 * fA0 + Q01 * fA1 >= TMath::Abs( vT.y * R01 - R11 * vT.x ) &&
	    Q20 * fB1 + Q21 * fB0 + Q12 * fA0 + Q02 * fA1 >= TMath::Abs( vT.y * R02 - R12 * vT.x );
}

// $deBlob: FUNCTION 0082b170
// $Barnyard: FUNCTION 006d0eb0
TBOOL TOBBox::Intersects( const TOBBox& a_rcBox, const TSphere& a_rcSphere )
{
	const TVector4& rcSphere = a_rcSphere.AsVector4();
	TVector4        vCentre( rcSphere.x, rcSphere.y, rcSphere.z, 1.0f );

	TVector4 vClosest;
	a_rcBox.ClosestPoint( vCentre, vClosest );

	vCentre.Set( rcSphere.x, rcSphere.y, rcSphere.z, 1.0f );

	return TVector4::DistanceSq( vClosest, vCentre ) <= rcSphere.w * rcSphere.w;
}

// $deBlob: FUNCTION 0082b200
// $Barnyard: FUNCTION 006d0640
TBOOL TOBBox::Intersects( const TOBBox& a_rcBox, const TCapsule& a_rcCapsule )
{
	TFLOAT fHalfLength = a_rcCapsule.fLength * 0.5f;

	TVector4 vHalf;
	vHalf.Multiply( a_rcCapsule.vDirection, fHalfLength );

	TVector4 vEnd( vHalf.x + a_rcCapsule.vCentre.x, vHalf.y + a_rcCapsule.vCentre.y, vHalf.z + a_rcCapsule.vCentre.z, a_rcCapsule.vCentre.w );

	vHalf.Multiply( a_rcCapsule.vDirection, fHalfLength );
	TVector4 vStart( a_rcCapsule.vCentre.x - vHalf.x, a_rcCapsule.vCentre.y - vHalf.y, a_rcCapsule.vCentre.z - vHalf.z, a_rcCapsule.vCentre.w );

	TVector4 vSegmentPoint;
	TVector4 vBoxPoint;
	ClosestLineBoxPoints( vEnd, vStart, a_rcBox, vSegmentPoint, vBoxPoint );

	TVector4 vSphere1( vSegmentPoint.x, vSegmentPoint.y, vSegmentPoint.z, a_rcCapsule.fRadius );
	TVector4 vSphere2( vBoxPoint.x, vBoxPoint.y, vBoxPoint.z, 0.0f );

	return TMath::SphereVsSphere( vSphere1, vSphere2 );
}

// Returns the larger one of the three values (in the order of the original's compares)
static TFLOAT Max3( TFLOAT a_fA, TFLOAT a_fB, TFLOAT a_fC )
{
	TFLOAT fMax = ( a_fA <= a_fB ) ? a_fB : a_fA;
	return ( a_fC <= fMax ) ? fMax : a_fC;
}

// Returns the smaller one of the three values (in the order of the original's compares)
static TFLOAT Min3( TFLOAT a_fA, TFLOAT a_fB, TFLOAT a_fC )
{
	TFLOAT fMin = ( a_fB <= a_fA ) ? a_fB : a_fA;
	return ( fMin <= a_fC ) ? fMin : a_fC;
}

// Returns max( min( p0, p1 ), -max( p0, p1 ) ), the triangle is separated from the box if it's larger than the radius
static TFLOAT GetEdgeAxisDistance( TFLOAT a_fP0, TFLOAT a_fP1 )
{
	TFLOAT fMax    = ( a_fP1 <= a_fP0 ) ? a_fP0 : a_fP1;
	TFLOAT fMin    = ( a_fP0 <= a_fP1 ) ? a_fP0 : a_fP1;
	TFLOAT fNegMax = -fMax;

	return ( fMin > fNegMax ) ? fMin : fNegMax;
}

// $deBlob: FUNCTION 0082b310
TBOOL TOBBox::Intersects( const TOBBox& a_rcBox, const TCollisionTriangle& a_rcTriangle )
{
	TMatrix44 oInverse = a_rcBox.m_oTransform;
	oInverse.Invert( oInverse );

	// The triangle in the space of the box
	TVector4 aVertices[ 3 ];

	for ( TINT i = 0; i < 3; i++ )
		TMatrix44::TransformVector( aVertices[ i ], oInverse, a_rcTriangle.aVertices[ i ] );

	TVector4 vNormal;
	TMatrix44::RotateVector( vNormal, oInverse, a_rcTriangle.vNormal );

	const TVector4& v0       = aVertices[ 0 ];
	const TVector4& v1       = aVertices[ 1 ];
	const TVector4& v2       = aVertices[ 2 ];
	const TVector4& vExtents = a_rcBox.m_vExtents;

	// The plane of the triangle
	TFLOAT fDistance = v0.x * vNormal.x + v0.y * vNormal.y + v0.z * vNormal.z;
	vNormal.Abs4();

	if ( TMath::Abs( fDistance ) > vNormal.x * vExtents.x + vNormal.y * vExtents.y + vNormal.z * vExtents.z )
		return TFALSE;

	// The axes of the box
	if ( -vExtents.x > Max3( v0.x, v1.x, v2.x ) ) return TFALSE;
	if ( Min3( v0.x, v1.x, v2.x ) > vExtents.x ) return TFALSE;
	if ( -vExtents.y > Max3( v0.y, v1.y, v2.y ) ) return TFALSE;
	if ( Min3( v0.y, v1.y, v2.y ) > vExtents.y ) return TFALSE;
	if ( -vExtents.z > Max3( v0.z, v1.z, v2.z ) ) return TFALSE;
	if ( Min3( v0.z, v1.z, v2.z ) > vExtents.z ) return TFALSE;

	// The edges of the triangle against the axes of the box
	TVector4 vEdge0( v1.x - v0.x, v1.y - v0.y, v1.z - v0.z, v1.w );
	TVector4 vEdge2( v0.x - v2.x, v0.y - v2.y, v0.z - v2.z, v0.w );
	TVector4 vEdge1( v2.x - v1.x, v2.y - v1.y, v2.z - v1.z, v2.w );

	TVector4 vAbsEdge0 = vEdge0;
	vAbsEdge0.Abs4();
	TVector4 vAbsEdge1 = vEdge1;
	vAbsEdge1.Abs4();
	TVector4 vAbsEdge2 = vEdge2;
	vAbsEdge2.Abs4();

	const TFLOAT fEpsilon = 0.01f;

	// X
	if ( GetEdgeAxisDistance( v0.z * vEdge0.y - v0.y * vEdge0.z, v2.z * vEdge0.y - v2.y * vEdge0.z ) > vAbsEdge0.y * vExtents.z + vAbsEdge0.z * vExtents.y + fEpsilon )
		return TFALSE;

	if ( GetEdgeAxisDistance( v0.z * vEdge1.y - v0.y * vEdge1.z, v1.z * vEdge1.y - v1.y * vEdge1.z ) > vAbsEdge1.y * vExtents.z + vAbsEdge1.z * vExtents.y + fEpsilon )
		return TFALSE;

	if ( GetEdgeAxisDistance( v0.z * vEdge2.y - v0.y * vEdge2.z, v1.z * vEdge2.y - v1.y * vEdge2.z ) > vAbsEdge2.y * vExtents.z + vAbsEdge2.z * vExtents.y + fEpsilon )
		return TFALSE;

	// Y
	if ( GetEdgeAxisDistance( v0.x * vEdge0.z - v0.z * vEdge0.x, v2.x * vEdge0.z - v2.z * vEdge0.x ) > vAbsEdge0.x * vExtents.z + vAbsEdge0.z * vExtents.x + fEpsilon )
		return TFALSE;

	if ( GetEdgeAxisDistance( v0.x * vEdge1.z - v0.z * vEdge1.x, v1.x * vEdge1.z - v1.z * vEdge1.x ) > vAbsEdge1.x * vExtents.z + vAbsEdge1.z * vExtents.x + fEpsilon )
		return TFALSE;

	if ( GetEdgeAxisDistance( v0.x * vEdge2.z - v0.z * vEdge2.x, v1.x * vEdge2.z - v1.z * vEdge2.x ) > vAbsEdge2.x * vExtents.z + vAbsEdge2.z * vExtents.x + fEpsilon )
		return TFALSE;

	// Z
	if ( GetEdgeAxisDistance( v0.y * vEdge0.x - v0.x * vEdge0.y, v2.y * vEdge0.x - v2.x * vEdge0.y ) > vAbsEdge0.x * vExtents.y + vAbsEdge0.y * vExtents.x + fEpsilon )
		return TFALSE;

	if ( GetEdgeAxisDistance( v0.y * vEdge1.x - v0.x * vEdge1.y, v1.y * vEdge1.x - v1.x * vEdge1.y ) > vAbsEdge1.x * vExtents.y + vAbsEdge1.y * vExtents.x + fEpsilon )
		return TFALSE;

	return GetEdgeAxisDistance( v0.y * vEdge2.x - v0.x * vEdge2.y, v1.y * vEdge2.x - v1.x * vEdge2.y ) <= vAbsEdge2.y * vExtents.x + vAbsEdge2.x * vExtents.y + fEpsilon;
}

// $deBlob: FUNCTION 0082bc50
TBOOL TOBBox::Collide( const TOBBox& a_rcBox, const TRay& a_rcRay, TCollisionContact& a_rContact )
{
	TMatrix44 oInverse = a_rcBox.m_oTransform;
	oInverse.Invert( oInverse );

	// The ray in the space of the box
	TVector4 vOffset(
	    a_rcRay.vOrigin.x - a_rcBox.m_oTransform.m_f41,
	    a_rcRay.vOrigin.y - a_rcBox.m_oTransform.m_f42,
	    a_rcRay.vOrigin.z - a_rcBox.m_oTransform.m_f43,
	    a_rcRay.vOrigin.w
	);

	TVector4 vOrigin;
	TMatrix44::RotateVector( vOrigin, oInverse, vOffset );

	TVector4 vDirection;
	TMatrix44::RotateVector( vDirection, oInverse, a_rcRay.vDirection );

	// Mirror the ray so that the direction is positive, afSign is the sign of the normal of the entered face
	TFLOAT afSign[ 3 ];

	for ( TINT i = 0; i < 3; i++ )
	{
		if ( vDirection.AsArray()[ i ] >= 0.0f )
		{
			afSign[ i ] = -1.0f;
		}
		else
		{
			vOrigin.AsArray()[ i ]    = -vOrigin.AsArray()[ i ];
			afSign[ i ]               = 1.0f;
			vDirection.AsArray()[ i ] = -vDirection.AsArray()[ i ];
		}
	}

	const TVector4 vExtents = a_rcBox.m_vExtents;

	if ( -vExtents.x > vOrigin.x && vDirection.x <= 0.0f )
		return TFALSE;

	if ( vOrigin.x > vExtents.x )
		return TFALSE;

	if ( -vExtents.y > vOrigin.y && vDirection.y <= 0.0f )
		return TFALSE;

	if ( vOrigin.y > vExtents.y )
		return TFALSE;

	if ( -vExtents.z > vOrigin.z && vDirection.z <= 0.0f )
		return TFALSE;

	if ( vOrigin.z > vExtents.z || ( vDirection.x == 0.0f && vDirection.y == 0.0f && vDirection.z == 0.0f ) )
		return TFALSE;

	TFLOAT fExit      = 3.402823466e+38f;
	TINT   iEnterAxis = 0;
	TINT   iExitAxis  = 0;
	TFLOAT fEnter     = -3.402823466e+38f;

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fDir    = vDirection.AsArray()[ i ];
		TFLOAT fOrigin = vOrigin.AsArray()[ i ];
		TFLOAT fExtent = vExtents.AsArray()[ i ];

		if ( fDir != 0.0f )
		{
			if ( ( -fExtent - fOrigin ) / fDir > fEnter )
			{
				fEnter     = ( -fExtent - fOrigin ) / fDir;
				iEnterAxis = i;
			}

			if ( fExit > ( fExtent - fOrigin ) / fDir )
			{
				fExit     = ( fExtent - fOrigin ) / fDir;
				iExitAxis = i;
			}
		}
	}

	if ( fEnter > fExit )
		return TFALSE;

	if ( fEnter < 0.0f )
	{
		fEnter     = fExit;
		iEnterAxis = iExitAxis;
	}

	if ( fEnter < 0.0f || fEnter > a_rcRay.fLength )
		return TFALSE;

	TVector4 vMovement;
	vMovement.Multiply( a_rcRay.vDirection, fEnter );

	a_rContact.vPosition.Set(
	    a_rcRay.vOrigin.x + vMovement.x,
	    vMovement.y + a_rcRay.vOrigin.y,
	    vMovement.z + a_rcRay.vOrigin.z,
	    a_rcRay.vOrigin.w
	);

	a_rContact.vNormal.Multiply( a_rcBox.GetAxis( iEnterAxis ), afSign[ iEnterAxis ] );
	a_rContact.fDepth = fEnter;

	return TTRUE;
}

// $deBlob: FUNCTION 0082bfe0
// $Barnyard: FUNCTION 006d04c0
TBOOL TOBBox::Collide( const TOBBox& a_rcBox, const TCapsule& a_rcCapsule, TCollisionContact& a_rContact )
{
	TFLOAT fHalfLength = a_rcCapsule.fLength * 0.5f;

	TVector4 vHalf;
	vHalf.Multiply( a_rcCapsule.vDirection, fHalfLength );

	TVector4 vEnd( vHalf.x + a_rcCapsule.vCentre.x, vHalf.y + a_rcCapsule.vCentre.y, vHalf.z + a_rcCapsule.vCentre.z, a_rcCapsule.vCentre.w );

	vHalf.Multiply( a_rcCapsule.vDirection, fHalfLength );
	TVector4 vStart( a_rcCapsule.vCentre.x - vHalf.x, a_rcCapsule.vCentre.y - vHalf.y, a_rcCapsule.vCentre.z - vHalf.z, a_rcCapsule.vCentre.w );

	TVector4 vSegmentPoint;
	TVector4 vBoxPoint;
	ClosestLineBoxPoints( vEnd, vStart, a_rcBox, vSegmentPoint, vBoxPoint );

	TVector4 vSphere1( vSegmentPoint.x, vSegmentPoint.y, vSegmentPoint.z, a_rcCapsule.fRadius );
	TVector4 vSphere2( vBoxPoint.x, vBoxPoint.y, vBoxPoint.z, 0.0f );

	return CollideSphereSphere( vSphere1, vSphere2, a_rContact );
}

// $deBlob: FUNCTION 0082c0f0
// $Barnyard: FUNCTION 006d1c50
TBOOL TOBBox::Collide( const TOBBox& a_rcBox1, const TOBBox& a_rcBox2, TCollisionContact& a_rContact )
{
	// NOTE: this is the box-box test of ODE (dBoxBox) returning only the deepest contact

	TVector4 vDistance(
	    a_rcBox2.m_oTransform.m_f41 - a_rcBox1.m_oTransform.m_f41,
	    a_rcBox2.m_oTransform.m_f42 - a_rcBox1.m_oTransform.m_f42,
	    a_rcBox2.m_oTransform.m_f43 - a_rcBox1.m_oTransform.m_f43,
	    a_rcBox2.m_oTransform.m_f44
	);

	TMatrix44 oInverse = a_rcBox1.m_oTransform;
	oInverse.Invert( oInverse );

	// Distance in the space of the first box
	TVector4 vPP;
	TMatrix44::RotateVector( vPP, oInverse, vDistance );

	const TVector4& a0 = a_rcBox1.GetAxis( 0 );
	const TVector4& a1 = a_rcBox1.GetAxis( 1 );
	const TVector4& a2 = a_rcBox1.GetAxis( 2 );
	const TVector4& b0 = a_rcBox2.GetAxis( 0 );
	const TVector4& b1 = a_rcBox2.GetAxis( 1 );
	const TVector4& b2 = a_rcBox2.GetAxis( 2 );

	TFLOAT A[ 3 ] = { a_rcBox1.m_vExtents.x, a_rcBox1.m_vExtents.y, a_rcBox1.m_vExtents.z };
	TFLOAT B[ 3 ] = { a_rcBox2.m_vExtents.x, a_rcBox2.m_vExtents.y, a_rcBox2.m_vExtents.z };

	TFLOAT R11 = TVector4::DotProduct3( a0, b0 );
	TFLOAT R12 = TVector4::DotProduct3( a0, b1 );
	TFLOAT R13 = TVector4::DotProduct3( a0, b2 );
	TFLOAT R21 = TVector4::DotProduct3( a1, b0 );
	TFLOAT R22 = TVector4::DotProduct3( a1, b1 );
	TFLOAT R23 = TVector4::DotProduct3( a1, b2 );
	TFLOAT R31 = TVector4::DotProduct3( a2, b0 );
	TFLOAT R32 = TVector4::DotProduct3( a2, b1 );
	TFLOAT R33 = TVector4::DotProduct3( a2, b2 );

	TBOOL    bInvertNormal = TFALSE;
	TBOOL    bWorldNormal  = TFALSE;
	TINT     iCode         = 0;
	TVector4 vNormalC;

	TFLOAT Q11 = TMath::Abs( R11 );
	TFLOAT Q12 = TMath::Abs( R12 );
	TFLOAT Q13 = TMath::Abs( R13 );
	TFLOAT Q21 = TMath::Abs( R21 );
	TFLOAT Q22 = TMath::Abs( R22 );
	TFLOAT Q23 = TMath::Abs( R23 );
	TFLOAT Q31 = TMath::Abs( R31 );
	TFLOAT Q32 = TMath::Abs( R32 );
	TFLOAT Q33 = TMath::Abs( R33 );

	TFLOAT fS = -3.402823466e+38f;
	TFLOAT fS2;

	// Faces of the first box
	fS2 = TMath::Abs( vPP.x ) - ( Q13 * B[ 2 ] + Q12 * B[ 1 ] + Q11 * B[ 0 ] + A[ 0 ] );
	if ( fS2 > 0.0f ) return TFALSE;
	if ( fS2 > fS )
	{
		fS            = fS2;
		vNormalC      = a0;
		bWorldNormal  = TTRUE;
		bInvertNormal = vPP.x < 0.0f;
		iCode         = 1;
	}

	fS2 = TMath::Abs( vPP.y ) - ( Q23 * B[ 2 ] + Q22 * B[ 1 ] + Q21 * B[ 0 ] + A[ 1 ] );
	if ( fS2 > 0.0f ) return TFALSE;
	if ( fS2 > fS )
	{
		fS            = fS2;
		vNormalC      = a1;
		bWorldNormal  = TTRUE;
		bInvertNormal = vPP.y < 0.0f;
		iCode         = 2;
	}

	fS2 = TMath::Abs( vPP.z ) - ( Q33 * B[ 2 ] + Q32 * B[ 1 ] + Q31 * B[ 0 ] + A[ 2 ] );
	if ( fS2 > 0.0f ) return TFALSE;
	if ( fS2 > fS )
	{
		fS            = fS2;
		vNormalC      = a2;
		bWorldNormal  = TTRUE;
		bInvertNormal = vPP.z < 0.0f;
		iCode         = 3;
	}

	// Faces of the second box
	fS2 = TMath::Abs( TVector4::DotProduct3( b0, vDistance ) ) - ( Q31 * A[ 2 ] + Q21 * A[ 1 ] + Q11 * A[ 0 ] + B[ 0 ] );
	if ( fS2 > 0.0f ) return TFALSE;
	if ( fS2 > fS )
	{
		fS            = fS2;
		vNormalC      = b0;
		bWorldNormal  = TTRUE;
		bInvertNormal = TVector4::DotProduct3( b0, vDistance ) < 0.0f;
		iCode         = 4;
	}

	fS2 = TMath::Abs( TVector4::DotProduct3( b1, vDistance ) ) - ( Q32 * A[ 2 ] + Q22 * A[ 1 ] + Q12 * A[ 0 ] + B[ 1 ] );
	if ( fS2 > 0.0f ) return TFALSE;
	if ( fS2 > fS )
	{
		fS            = fS2;
		vNormalC      = b1;
		bWorldNormal  = TTRUE;
		bInvertNormal = TVector4::DotProduct3( b1, vDistance ) < 0.0f;
		iCode         = 5;
	}

	fS2 = TMath::Abs( TVector4::DotProduct3( b2, vDistance ) ) - ( Q33 * A[ 2 ] + Q23 * A[ 1 ] + Q13 * A[ 0 ] + B[ 2 ] );
	if ( fS2 > 0.0f ) return TFALSE;
	if ( fS2 > fS )
	{
		fS            = fS2;
		vNormalC      = b2;
		bWorldNormal  = TTRUE;
		bInvertNormal = TVector4::DotProduct3( b2, vDistance ) < 0.0f;
		iCode         = 6;
	}

	// Edges, the axis is the cross product of two edges (in the space of the first box)
	auto TestEdgeAxis = [ & ]( TFLOAT a_fExpr1, TFLOAT a_fExpr2, TFLOAT a_fN1, TFLOAT a_fN2, TFLOAT a_fN3, TINT a_iCode ) -> TBOOL {
		TFLOAT fSeparation = TMath::Abs( a_fExpr1 ) - a_fExpr2;

		if ( fSeparation > 0.0f )
			return TFALSE;

		TVector4 vAxis( a_fN1, a_fN2, a_fN3, 1.0f );
		TFLOAT   fLength = VectorLength( vAxis );

		if ( fLength > 0.0f )
		{
			if ( fS < ( fSeparation / fLength ) * 1.05f )
			{
				fS           = fSeparation / fLength;
				bWorldNormal = TFALSE;
				vNormalC.Multiply( TVector4( a_fN1, a_fN2, a_fN3, 1.0f ), 1.0f / fLength );
				bInvertNormal = a_fExpr1 < 0.0f;
				iCode         = a_iCode;
			}
		}

		return TTRUE;
	};

	if ( !TestEdgeAxis( vPP.z * R21 - vPP.y * R31, Q31 * A[ 1 ] + Q12 * B[ 2 ] + Q13 * B[ 1 ] + Q21 * A[ 2 ], 0.0f, -R31, R21, 7 ) ) return TFALSE;
	if ( !TestEdgeAxis( vPP.z * R22 - vPP.y * R32, Q32 * A[ 1 ] + Q11 * B[ 2 ] + Q22 * A[ 2 ] + Q13 * B[ 0 ], 0.0f, -R32, R22, 8 ) ) return TFALSE;
	if ( !TestEdgeAxis( vPP.z * R23 - vPP.y * R33, Q33 * A[ 1 ] + Q23 * A[ 2 ] + Q11 * B[ 1 ] + Q12 * B[ 0 ], 0.0f, -R33, R23, 9 ) ) return TFALSE;
	if ( !TestEdgeAxis( vPP.x * R31 - vPP.z * R11, Q31 * A[ 0 ] + Q22 * B[ 2 ] + Q23 * B[ 1 ] + Q11 * A[ 2 ], R31, 0.0f, -R11, 10 ) ) return TFALSE;
	if ( !TestEdgeAxis( vPP.x * R32 - vPP.z * R12, Q32 * A[ 0 ] + Q21 * B[ 2 ] + Q23 * B[ 0 ] + Q12 * A[ 2 ], R32, 0.0f, -R12, 11 ) ) return TFALSE;
	if ( !TestEdgeAxis( vPP.x * R33 - vPP.z * R13, Q33 * A[ 0 ] + Q21 * B[ 1 ] + Q22 * B[ 0 ] + Q13 * A[ 2 ], R33, 0.0f, -R13, 12 ) ) return TFALSE;
	if ( !TestEdgeAxis( vPP.y * R11 - vPP.x * R21, Q32 * B[ 2 ] + Q33 * B[ 1 ] + Q21 * A[ 0 ] + Q11 * A[ 1 ], -R21, R11, 0.0f, 13 ) ) return TFALSE;
	if ( !TestEdgeAxis( vPP.y * R12 - vPP.x * R22, Q31 * B[ 2 ] + Q33 * B[ 0 ] + Q22 * A[ 0 ] + Q12 * A[ 1 ], -R22, R12, 0.0f, 14 ) ) return TFALSE;
	if ( !TestEdgeAxis( vPP.y * R13 - vPP.x * R23, Q31 * B[ 1 ] + Q32 * B[ 0 ] + Q23 * A[ 0 ] + Q13 * A[ 1 ], -R23, R13, 0.0f, 15 ) ) return TFALSE;

	if ( iCode == 0 )
		return TFALSE;

	TVector4 vNormal;

	if ( bWorldNormal == TTRUE )
		vNormal = vNormalC;
	else
		TMatrix44::RotateVector( vNormal, a_rcBox1.m_oTransform, vNormalC );

	if ( bInvertNormal )
		vNormal.Multiply( -1.0f );

	if ( iCode > 6 )
	{
		// Edge-edge contact, find a point on the intersecting edge of both boxes
		TVector4 vPointA = a_rcBox1.GetCentre();

		for ( TINT j = 0; j < 3; j++ )
		{
			TFLOAT fSign = ( TVector4::DotProduct3( vNormal, a_rcBox1.GetAxis( j ) ) <= 0.0f ) ? -1.0f : 1.0f;

			TVector4 vOffset;
			vOffset.Multiply( a_rcBox1.GetAxis( j ), A[ j ] );
			vOffset.Multiply( vOffset, fSign );
			vPointA.x += vOffset.x;
			vPointA.y += vOffset.y;
			vPointA.z += vOffset.z;
		}

		TVector4 vPointB = a_rcBox2.GetCentre();

		for ( TINT j = 0; j < 3; j++ )
		{
			// NOTE: ODE subtracts here (the sign is the same as for the first box in the original)
			TFLOAT fSign = ( TVector4::DotProduct3( vNormal, a_rcBox2.GetAxis( j ) ) <= 0.0f ) ? -1.0f : 1.0f;

			TVector4 vOffset;
			vOffset.Multiply( a_rcBox2.GetAxis( j ), B[ j ] );
			vOffset.Multiply( vOffset, fSign );
			vPointB.x += vOffset.x;
			vPointB.y += vOffset.y;
			vPointB.z += vOffset.z;
		}

		TVector4 vDirA = a_rcBox1.GetAxis( ( iCode - 7 ) / 3 );
		TVector4 vDirB = a_rcBox2.GetAxis( ( iCode - 7 ) % 3 );

		TFLOAT fAlpha;
		TFLOAT fBeta;
		LineClosestApproach( vPointA, vDirA, vPointB, vDirB, fAlpha, fBeta );

		TVector4 vOffset;
		vOffset.Multiply( vDirA, fAlpha );
		vPointA.x += vOffset.x;
		vPointA.y += vOffset.y;
		vPointA.z += vOffset.z;
		vOffset.Multiply( vDirB, fBeta );
		vPointB.x += vOffset.x;
		vPointB.y += vOffset.y;
		vPointB.z += vOffset.z;

		TVector4 vPosition;
		vPosition.Add( vPointA, vPointB );
		vPosition.Multiply( vPosition, 0.5f );

		a_rContact.vPosition = vPosition;
		a_rContact.vNormal   = vNormal;
		a_rContact.fDepth    = -fS;
		a_rContact.vNormal.w = 1.0f;

		return TTRUE;
	}

	// Face-something contact, the box with the face is the reference box
	const TOBBox* pBoxA = &a_rcBox1;
	const TOBBox* pBoxB = &a_rcBox2;

	if ( iCode > 3 )
	{
		pBoxA = &a_rcBox2;
		pBoxB = &a_rcBox1;
	}

	const TVector4& rcPointA  = pBoxA->GetCentre();
	const TVector4& rcPointB  = pBoxB->GetCentre();
	const TFLOAT*   pExtentsA = pBoxA->m_vExtents.AsArray();
	const TFLOAT*   pExtentsB = pBoxB->m_vExtents.AsArray();

	TVector4 vNormal2 = vNormal;

	if ( iCode > 3 )
		vNormal2.Multiply( -1.0f );

	TMatrix44 oInverseB = pBoxB->m_oTransform;
	oInverseB.Invert( oInverseB );

	// Normal in the space of the incident box
	TVector4 vNormalB;
	TMatrix44::RotateVector( vNormalB, oInverseB, vNormal2 );

	// The largest component of the normal is the normal of the incident face
	TFLOAT fAbsY = TMath::Abs( vNormalB.y );
	TFLOAT fAbsX = TMath::Abs( vNormalB.x );
	TINT   iLargest;
	TINT   iA1;
	TINT   iA2;

	if ( fAbsY <= fAbsX )
	{
		if ( TMath::Abs( vNormalB.z ) < fAbsX )
		{
			iLargest = 0;
			iA1      = 1;
			iA2      = 2;
		}
		else
		{
			iA1      = 0;
			iA2      = 1;
			iLargest = 2;
		}
	}
	else
	{
		iA1 = 0;

		if ( fAbsY > TMath::Abs( vNormalB.z ) )
		{
			iLargest = 1;
			iA2      = 2;
		}
		else
		{
			iA2      = 1;
			iLargest = 2;
		}
	}

	// Centre of the incident face relative to the reference box
	TVector4 vFaceOffset;
	vFaceOffset.Multiply( pBoxB->GetAxis( iLargest ), pExtentsB[ iLargest ] );

	TVector4 vCentre;
	vCentre.Substract( rcPointB, rcPointA );

	if ( vNormalB.AsArray()[ iLargest ] >= 0.0f )
		vCentre.Substract( vCentre, vFaceOffset );
	else
		vCentre.Add( vCentre, vFaceOffset );

	// Normal and the other axes of the reference box
	TINT iCodeN = ( iCode > 3 ) ? iCode - 4 : iCode - 1;
	TINT iCode1;
	TINT iCode2 = 2;

	if ( iCodeN == 0 )
	{
		iCode1 = 1;
	}
	else
	{
		iCode1 = 0;

		if ( iCodeN != 1 )
			iCode2 = 1;
	}

	// The incident face in the space of the reference face
	const TVector4& rcAxis1  = pBoxA->GetAxis( iCode1 );
	const TVector4& rcAxis2  = pBoxA->GetAxis( iCode2 );
	const TVector4& rcAxisB1 = pBoxB->GetAxis( iA1 );
	const TVector4& rcAxisB2 = pBoxB->GetAxis( iA2 );

	TFLOAT fC1  = TVector4::DotProduct3( vCentre, rcAxis1 );
	TFLOAT fC2  = TVector4::DotProduct3( vCentre, rcAxis2 );
	TFLOAT fM11 = TVector4::DotProduct3( rcAxis1, rcAxisB1 );
	TFLOAT fM12 = TVector4::DotProduct3( rcAxis1, rcAxisB2 );
	TFLOAT fM21 = TVector4::DotProduct3( rcAxis2, rcAxisB1 );
	TFLOAT fM22 = TVector4::DotProduct3( rcAxis2, rcAxisB2 );

	TFLOAT fK1 = fM11 * pExtentsB[ iA1 ];
	TFLOAT fK2 = fM21 * pExtentsB[ iA1 ];
	TFLOAT fK3 = fM12 * pExtentsB[ iA2 ];
	TFLOAT fK4 = fM22 * pExtentsB[ iA2 ];

	TFLOAT afQuad[ 8 ];
	afQuad[ 0 ] = fC1 - fK1 - fK3;
	afQuad[ 1 ] = fC2 - fK2 - fK4;
	afQuad[ 2 ] = fC1 - fK1 + fK3;
	afQuad[ 3 ] = fC2 - fK2 + fK4;
	afQuad[ 4 ] = fK3 + fK1 + fC1;
	afQuad[ 5 ] = fK4 + fK2 + fC2;
	afQuad[ 6 ] = fK1 + fC1 - fK3;
	afQuad[ 7 ] = fK2 + fC2 - fK4;

	TFLOAT afRectangle[ 2 ] = { pExtentsA[ iCode1 ], pExtentsA[ iCode2 ] };

	TFLOAT afPoints[ 16 ];
	TINT   iNumPoints = IntersectRectQuad( afRectangle, afQuad, afPoints );

	if ( iNumPoints < 1 )
		return TFALSE;

	// Convert the points into the space of the reference box and calculate the depths
	TVector4 aPoints[ 8 ];
	TFLOAT   afDepths[ 8 ];
	TINT     iNumContacts = 0;

	TFLOAT fDet = 1.0f / ( fM22 * fM11 - fM21 * fM12 );
	fM11        = fM11 * fDet;
	fM12        = fM12 * fDet;
	fM21        = fM21 * fDet;
	fM22        = fDet * fM22;

	for ( TINT j = 0; j < iNumPoints; j++ )
	{
		TFLOAT fY = afPoints[ j * 2 + 1 ] - fC2;
		TFLOAT fX = afPoints[ j * 2 ] - fC1;

		TVector4 vOffset2;
		vOffset2.Multiply( rcAxisB2, fY * fM11 - fX * fM21 );

		TVector4 vOffset1;
		vOffset1.Multiply( rcAxisB1, fX * fM22 - fY * fM12 );

		TVector4 vPoint;
		vPoint.Add( vCentre, vOffset1 );
		aPoints[ iNumContacts ].Add( vPoint, vOffset2 );

		TFLOAT fDepth            = pExtentsA[ iCodeN ] - TVector4::DotProduct3( vNormal2, aPoints[ iNumContacts ] );
		afDepths[ iNumContacts ] = fDepth;

		if ( fDepth >= 0.0f )
		{
			afPoints[ iNumContacts * 2 ]     = afPoints[ j * 2 ];
			afPoints[ iNumContacts * 2 + 1 ] = afPoints[ j * 2 + 1 ];
			iNumContacts++;
		}
	}

	if ( iNumContacts < 1 )
		return TFALSE;

	TINT iMaxContacts = 1;
	TMath::Clip( iMaxContacts, 1, iNumContacts );

	if ( iNumContacts > iMaxContacts )
	{
		// Use the deepest point
		TINT   iDeepest  = 0;
		TFLOAT fMaxDepth = afDepths[ 0 ];

		for ( TINT j = 1; j < iNumContacts; j++ )
		{
			if ( fMaxDepth < afDepths[ j ] )
			{
				iDeepest  = j;
				fMaxDepth = afDepths[ j ];
			}
		}

		a_rContact.vPosition.Add( aPoints[ iDeepest ], rcPointA );
		a_rContact.fDepth  = afDepths[ iDeepest ];
		a_rContact.vNormal = vNormal;
	}
	else
	{
		for ( TINT j = 0; j < iNumContacts; j++ )
		{
			a_rContact.vPosition.Add( aPoints[ j ], rcPointA );
			a_rContact.vNormal = vNormal;
			a_rContact.fDepth  = afDepths[ j ];
		}
	}

	a_rContact.vNormal.w = 1.0f;
	return TTRUE;
}

// $deBlob: FUNCTION 0082ec40
// $Barnyard: FUNCTION 006d07c0
TBOOL TOBBox::Collide( const TOBBox& a_rcBox, const TSphere& a_rcSphere, TCollisionContact& a_rContact )
{
	const TVector4& rcSphere = a_rcSphere.AsVector4();

	TVector4 vDistance( rcSphere.x - a_rcBox.m_oTransform.m_f41, rcSphere.y - a_rcBox.m_oTransform.m_f42, rcSphere.z - a_rcBox.m_oTransform.m_f43, 1.0f );

	TBOOL    bOutside = TFALSE;
	TVector4 vLocal;
	vLocal.x = TVector4::DotProduct3( vDistance, a_rcBox.GetAxis( 0 ) );
	vLocal.y = TVector4::DotProduct3( vDistance, a_rcBox.GetAxis( 1 ) );
	vLocal.z = TVector4::DotProduct3( vDistance, a_rcBox.GetAxis( 2 ) );
	vLocal.w = TVector4::VEC_ZERO.w;

	if ( vLocal.x < -a_rcBox.m_vExtents.x )
	{
		vLocal.x = -a_rcBox.m_vExtents.x;
		bOutside = TTRUE;
	}

	if ( vLocal.x > a_rcBox.m_vExtents.x )
	{
		vLocal.x = a_rcBox.m_vExtents.x;
		bOutside = TTRUE;
	}

	if ( vLocal.y < -a_rcBox.m_vExtents.y )
	{
		vLocal.y = -a_rcBox.m_vExtents.y;
		bOutside = TTRUE;
	}

	if ( vLocal.y > a_rcBox.m_vExtents.y )
	{
		vLocal.y = a_rcBox.m_vExtents.y;
		bOutside = TTRUE;
	}

	if ( vLocal.z < -a_rcBox.m_vExtents.z )
	{
		vLocal.z = -a_rcBox.m_vExtents.z;
		bOutside = TTRUE;
	}

	if ( vLocal.z > a_rcBox.m_vExtents.z )
	{
		vLocal.z = a_rcBox.m_vExtents.z;
	}
	else if ( bOutside == TFALSE )
	{
		// The centre is inside of the box, push it out through the closest face
		TINT   iAxis  = 0;
		TFLOAT fDepth = a_rcBox.m_vExtents.x - TMath::Abs( vLocal.x );

		if ( a_rcBox.m_vExtents.y - TMath::Abs( vLocal.y ) < fDepth )
		{
			fDepth = a_rcBox.m_vExtents.y - TMath::Abs( vLocal.y );
			iAxis  = 1;
		}

		if ( a_rcBox.m_vExtents.z - TMath::Abs( vLocal.z ) < fDepth )
		{
			fDepth = a_rcBox.m_vExtents.z - TMath::Abs( vLocal.z );
			iAxis  = 2;
		}

		a_rContact.vPosition.Set( rcSphere.x, rcSphere.y, rcSphere.z, 1.0f );

		TVector4 vNormal           = TVector4::VEC_ZERO;
		vNormal.AsArray()[ iAxis ] = ( vLocal.AsArray()[ iAxis ] <= 0.0f ) ? -1.0f : 1.0f;
		TMatrix44::RotateVector( a_rContact.vNormal, a_rcBox.m_oTransform, vNormal );

		a_rContact.fDepth = fDepth + rcSphere.w;
		return TTRUE;
	}

	TVector4 vClosest;
	TMatrix44::RotateVector( vClosest, a_rcBox.m_oTransform, vLocal );

	TVector4 vToCentre( vDistance.x - vClosest.x, vDistance.y - vClosest.y, vDistance.z - vClosest.z, 1.0f );
	TFLOAT   fDepth = rcSphere.w - VectorLength( vToCentre );

	if ( fDepth < 0.0f )
		return TFALSE;

	a_rContact.vPosition.Add( vClosest, a_rcBox.GetCentre() );
	a_rContact.vNormal = vToCentre;
	a_rContact.vNormal.Normalise();
	a_rContact.fDepth = fDepth;

	return TTRUE;
}

// $deBlob: FUNCTION 0082ef60
// $Barnyard: FUNCTION 006d1130
TFLOAT TOBBox::IntersectTriangle( const TOBBox& a_rcBox, const TVector4& a_rcMovement, const TCollisionTriangle& a_rcTriangle, TVector4& a_rOutPoint, TVector4& a_rOutNormal )
{
	TFLOAT fEnter = -3.402823466e+38f;
	TFLOAT fExit  = 3.402823466e+38f;

	TFLOAT fTriMin = TVector4::DotProduct3( a_rcTriangle.aVertices[ 0 ], a_rcTriangle.vNormal );
	TFLOAT fTriMax = fTriMin;
	TFLOAT fBoxMin;
	TFLOAT fBoxMax;

	// Normal of the triangle
	ProjectBoxOntoAxis( a_rcTriangle.vNormal, a_rcBox.m_oTransform, a_rcBox.m_vExtents, fBoxMin, fBoxMax );

	if ( TestForNoIntersection( TVector4::DotProduct3( a_rcTriangle.vNormal, a_rcMovement ), fTriMin, fTriMin, fBoxMin, fBoxMax, fEnter, fExit ) )
		return 2.0f;

	TVector4 vMovement;
	vMovement.Multiply( a_rcMovement, fEnter );

	TVector4 vPoint(
	    vMovement.x + a_rcBox.m_oTransform.m_f41,
	    vMovement.y + a_rcBox.m_oTransform.m_f42,
	    vMovement.z + a_rcBox.m_oTransform.m_f43,
	    a_rcBox.m_oTransform.m_f44
	);

	TVector4 vNormal = a_rcTriangle.vNormal;

	// Deepest point of the box along the normal
	for ( TINT i = 0; i < 3; i++ )
	{
		const TVector4& rcAxis = a_rcBox.GetAxis( i );

		TVector4 vOffset;

		if ( TVector4::DotProduct3( rcAxis, a_rcTriangle.vNormal ) <= 0.0f )
			vOffset.Multiply( rcAxis, a_rcBox.m_vExtents.AsArray()[ i ] );
		else
			vOffset.Multiply( rcAxis, -a_rcBox.m_vExtents.AsArray()[ i ] );

		vPoint.x += vOffset.x;
		vPoint.y += vOffset.y;
		vPoint.z += vOffset.z;
	}

	// Axes of the box
	for ( TINT i = 0; i < 3; i++ )
	{
		const TVector4& rcAxis  = a_rcBox.GetAxis( i );
		TFLOAT          fExtent = a_rcBox.m_vExtents.AsArray()[ i ];

		ProjectTriangleOntoAxis( rcAxis, a_rcTriangle, fTriMin, fTriMax );

		TFLOAT fCentre = TVector4::DotProduct3( rcAxis, a_rcBox.GetCentre() );
		fBoxMin        = fCentre - fExtent;
		fBoxMax        = fCentre + fExtent;

		TFLOAT fPrevEnter = fEnter;

		if ( TestForNoIntersection( TVector4::DotProduct3( rcAxis, a_rcMovement ), fTriMin, fTriMax, fBoxMin, fBoxMax, fEnter, fExit ) )
			return 2.0f;

		if ( fPrevEnter != fEnter )
		{
			// A vertex of the triangle hits a face of the box
			TFLOAT fClosest = 3.402823466e+38f;

			for ( TINT k = 0; k < 3; k++ )
			{
				const TVector4& rcVertex = a_rcTriangle.aVertices[ k ];

				TVector4 vToVertex( rcVertex.x - a_rcBox.m_oTransform.m_f41, rcVertex.y - a_rcBox.m_oTransform.m_f42, rcVertex.z - a_rcBox.m_oTransform.m_f43, rcVertex.w );
				TFLOAT   fDistance = TMath::Abs( TVector4::DotProduct3( rcAxis, vToVertex ) );

				if ( fClosest > fDistance )
				{
					fClosest = fDistance;
					vPoint   = rcVertex;
					vNormal.Multiply( rcAxis, -1.0f );
				}
			}
		}
	}

	// Edges of the triangle against the axes of the box
	TVector4 aEdges[ 3 ];
	aEdges[ 0 ].Set( a_rcTriangle.aVertices[ 1 ].x - a_rcTriangle.aVertices[ 0 ].x, a_rcTriangle.aVertices[ 1 ].y - a_rcTriangle.aVertices[ 0 ].y, a_rcTriangle.aVertices[ 1 ].z - a_rcTriangle.aVertices[ 0 ].z, a_rcTriangle.aVertices[ 1 ].w );
	aEdges[ 1 ].Set( a_rcTriangle.aVertices[ 2 ].x - a_rcTriangle.aVertices[ 0 ].x, a_rcTriangle.aVertices[ 2 ].y - a_rcTriangle.aVertices[ 0 ].y, a_rcTriangle.aVertices[ 2 ].z - a_rcTriangle.aVertices[ 0 ].z, a_rcTriangle.aVertices[ 2 ].w );
	aEdges[ 2 ].Set( aEdges[ 1 ].x - aEdges[ 0 ].x, aEdges[ 1 ].y - aEdges[ 0 ].y, aEdges[ 1 ].z - aEdges[ 0 ].z, a_rcTriangle.aVertices[ 2 ].w );

	for ( TINT k = 0; k < 3; k++ )
	{
		for ( TINT i = 0; i < 3; i++ )
		{
			const TVector4& rcAxis = a_rcBox.GetAxis( i );

			TVector4 vSeparatingAxis;
			vSeparatingAxis.CrossProduct( aEdges[ k ], rcAxis );

			// NOTE: de Blob normalises the axis (Barnyard doesn't), an edge parallel to the axis gives ( 0, 0, 1 )
			vSeparatingAxis.Normalise();

			ProjectTriangleOntoAxis( vSeparatingAxis, a_rcTriangle, fTriMin, fTriMax );

			TFLOAT fCentre = TVector4::DotProduct3( vSeparatingAxis, a_rcBox.GetCentre() );
			TFLOAT fDot0   = TVector4::DotProduct3( vSeparatingAxis, a_rcBox.GetAxis( 0 ) );
			TFLOAT fDot1   = TVector4::DotProduct3( vSeparatingAxis, a_rcBox.GetAxis( 1 ) );
			TFLOAT fRadius = TMath::Abs( TVector4::DotProduct3( vSeparatingAxis, a_rcBox.GetAxis( 2 ) ) ) * a_rcBox.m_vExtents.z + TMath::Abs( fDot1 ) * a_rcBox.m_vExtents.y + TMath::Abs( fDot0 ) * a_rcBox.m_vExtents.x;
			fBoxMin        = fCentre - fRadius;
			fBoxMax        = fRadius + fCentre;

			TFLOAT fPrevEnter = fEnter;

			if ( TestForNoIntersection( TVector4::DotProduct3( vSeparatingAxis, a_rcMovement ), fTriMin, fTriMax, fBoxMin, fBoxMax, fEnter, fExit ) )
				return 2.0f;

			if ( fPrevEnter != fEnter )
			{
				// An edge of the triangle hits an edge of the box
				TVector4 vEdgePoint;
				vMovement.Multiply( a_rcMovement, fEnter );
				vEdgePoint.Set( vMovement.x + a_rcBox.m_oTransform.m_f41, vMovement.y + a_rcBox.m_oTransform.m_f42, vMovement.z + a_rcBox.m_oTransform.m_f43, a_rcBox.m_oTransform.m_f44 );

				vNormal = vSeparatingAxis;
				vNormal.Normalise();

				if ( TVector4::DotProduct3( vNormal, a_rcTriangle.vNormal ) < 0.0f )
					vNormal.Multiply( -1.0f );

				for ( TINT j = 0; j < 3; j++ )
				{
					if ( j != i )
					{
						const TVector4& rcOtherAxis = a_rcBox.GetAxis( j );

						TVector4 vOffset;

						if ( TVector4::DotProduct3( rcOtherAxis, vNormal ) <= 0.0f )
							vOffset.Multiply( rcOtherAxis, a_rcBox.m_vExtents.AsArray()[ j ] );
						else
							vOffset.Multiply( rcOtherAxis, -a_rcBox.m_vExtents.AsArray()[ j ] );

						vEdgePoint.x += vOffset.x;
						vEdgePoint.y += vOffset.y;
						vEdgePoint.z += vOffset.z;
					}
				}

				// The edge of the box
				TFLOAT fExtent = a_rcBox.m_vExtents.AsArray()[ i ];

				TVector4 vHalfEdge;
				vHalfEdge.Multiply( rcAxis, fExtent );

				TVector4 vSegmentStart( vEdgePoint.x - vHalfEdge.x, vEdgePoint.y - vHalfEdge.y, vEdgePoint.z - vHalfEdge.z, vEdgePoint.w );

				TVector4 vSegment;
				vSegment.Multiply( rcAxis, fExtent );
				vSegment.Multiply( vSegment, 2.0f );

				// NOTE: the original uses the k-th vertex of the triangle as the point of the k-th edge
				vPoint = GetClosestPointOnLineToRay( vSegmentStart, vSegment, a_rcTriangle.aVertices[ k ], aEdges[ k ] );
			}
		}
	}

	if ( fEnter > 0.0f && fEnter < 1.0f )
	{
		a_rOutPoint  = vPoint;
		a_rOutNormal = vNormal;
		return fEnter;
	}

	return 2.0f;
}

// $deBlob: FUNCTION 00830200
// $Barnyard: FUNCTION 006cf6d0
void TOBBox::ProjectBoxOntoAxis( const TVector4& a_rcAxis, const TMatrix44& a_rcTransform, const TVector4& a_rcExtents, TFLOAT& a_rfMin, TFLOAT& a_rfMax )
{
	TFLOAT fCentre = TVector4::DotProduct3( a_rcAxis, a_rcTransform.AsBasisVector4( 3 ) );
	TFLOAT fX      = TVector4::DotProduct3( a_rcAxis, a_rcTransform.AsBasisVector4( 0 ) );
	TFLOAT fY      = TVector4::DotProduct3( a_rcAxis, a_rcTransform.AsBasisVector4( 1 ) );
	TFLOAT fRadius = TMath::Abs( TVector4::DotProduct3( a_rcAxis, a_rcTransform.AsBasisVector4( 2 ) ) ) * a_rcExtents.z + TMath::Abs( fY ) * a_rcExtents.y + TMath::Abs( fX ) * a_rcExtents.x;

	a_rfMin = fCentre - fRadius;
	a_rfMax = fRadius + fCentre;
}

// $deBlob: FUNCTION 00830300
// $Barnyard: FUNCTION 006d1080
void TOBBox::ProjectTriangleOntoAxis( const TVector4& a_rcAxis, const TCollisionTriangle& a_rcTriangle, TFLOAT& a_rfMin, TFLOAT& a_rfMax )
{
	TFLOAT fProjection = TVector4::DotProduct3( a_rcAxis, a_rcTriangle.aVertices[ 0 ] );
	a_rfMin            = fProjection;
	a_rfMax            = fProjection;

	fProjection = TVector4::DotProduct3( a_rcAxis, a_rcTriangle.aVertices[ 1 ] );
	a_rfMin     = ( fProjection >= a_rfMin ) ? a_rfMin : fProjection;
	a_rfMax     = ( a_rfMax >= fProjection ) ? a_rfMax : fProjection;

	fProjection = TVector4::DotProduct3( a_rcAxis, a_rcTriangle.aVertices[ 2 ] );
	a_rfMin     = ( fProjection >= a_rfMin ) ? a_rfMin : fProjection;
	a_rfMax     = ( a_rfMax >= fProjection ) ? a_rfMax : fProjection;
}

// $deBlob: FUNCTION 008303e0
// $Barnyard: FUNCTION 006d0f40
TBOOL TOBBox::TestForNoIntersection( TFLOAT a_fVelocity, TFLOAT a_fMin1, TFLOAT a_fMax1, TFLOAT a_fMin2, TFLOAT a_fMax2, TFLOAT& a_rfEnter, TFLOAT& a_rfExit )
{
	TFLOAT fInvVelocity = 1.0f / a_fVelocity;
	TFLOAT fExit;

	if ( a_fMax2 < a_fMin1 )
	{
		// The second interval is below the first one
		if ( a_fVelocity <= 0.0f )
			return TTRUE;

		TFLOAT fEnter = ( a_fMin1 - a_fMax2 ) * fInvVelocity;
		a_rfEnter     = ( a_rfEnter >= fEnter ) ? a_rfEnter : fEnter;
		fExit         = a_fMax1 - a_fMin2;
	}
	else if ( a_fMax1 < a_fMin2 )
	{
		// The second interval is above the first one
		if ( a_fVelocity >= 0.0f )
			return TTRUE;

		TFLOAT fEnter = ( a_fMax1 - a_fMin2 ) * fInvVelocity;
		a_rfEnter     = ( a_rfEnter >= fEnter ) ? a_rfEnter : fEnter;
		fExit         = a_fMin1 - a_fMax2;
	}
	else if ( a_fVelocity <= 0.0f )
	{
		if ( a_fVelocity >= 0.0f )
			return TFALSE;

		TFLOAT fEnter = ( a_fMax1 - a_fMin2 ) * fInvVelocity;
		a_rfEnter     = ( a_rfEnter >= fEnter ) ? a_rfEnter : fEnter;
		fExit         = a_fMin1 - a_fMax2;
	}
	else
	{
		TFLOAT fEnter = ( a_fMin1 - a_fMax2 ) * fInvVelocity;
		a_rfEnter     = ( a_rfEnter >= fEnter ) ? a_rfEnter : fEnter;
		fExit         = a_fMax1 - a_fMin2;
	}

	fExit    = fExit * fInvVelocity;
	a_rfExit = ( fExit >= a_rfExit ) ? a_rfExit : fExit;

	return a_rfEnter > a_rfExit;
}

// $deBlob: FUNCTION 008304e0
// $Barnyard: FUNCTION 006cf750
// Returns the point of the segment that is the closest to the line
static TVector4 GetClosestPointOnLineToRay( const TVector4& a_rcSegmentStart, const TVector4& a_rcSegmentVector, const TVector4& a_rcLinePoint, const TVector4& a_rcLineDirection )
{
	TVector4 vOffset;
	vOffset.Substract( a_rcSegmentStart, a_rcLinePoint );
	vOffset.w = a_rcSegmentStart.w;

	TVector4 vB;
	vB.CrossProduct( vOffset, a_rcLineDirection );

	TVector4 vA;
	vA.CrossProduct( a_rcSegmentVector, a_rcLineDirection );

	TFLOAT fAA = vA.x * vA.x + vA.y * vA.y + vA.z * vA.z;
	TFLOAT fT;

	if ( fAA < 0.000001f || ( fT = -( ( vB.x * vA.x * 2.0f + vB.y * vA.y * 2.0f + vB.z * vA.z * 2.0f ) / ( fAA * 2.0f ) ) ) < 0.0f )
		return a_rcSegmentStart;

	TVector4 vResult;

	if ( fT <= 1.0f )
	{
		TVector4 vPart;
		vPart.Multiply( a_rcSegmentVector, fT );
		vResult.Add( a_rcSegmentStart, vPart );
	}
	else
	{
		vResult.Add( a_rcSegmentStart, a_rcSegmentVector );
	}

	return vResult;
}

// $deBlob: FUNCTION 00830660
// $Barnyard: FUNCTION 006cfdf0
void ClosestLineBoxPoints( const TVector4& a_rcStart, const TVector4& a_rcEnd, const TOBBox& a_rcBox, TVector4& a_rOutSegmentPoint, TVector4& a_rOutBoxPoint )
{
	TMatrix44 oInverse = a_rcBox.m_oTransform;
	oInverse.Invert( oInverse );

	// Segment in the space of the box
	TVector4 vTemp;
	vTemp.x = a_rcStart.x - a_rcBox.m_oTransform.m_f41;
	vTemp.y = a_rcStart.y - a_rcBox.m_oTransform.m_f42;
	vTemp.z = a_rcStart.z - a_rcBox.m_oTransform.m_f43;
	vTemp.w = a_rcStart.w;

	TVector4 vPoint;
	TMatrix44::RotateVector( vPoint, oInverse, vTemp );

	vTemp.x = a_rcEnd.x - a_rcStart.x;
	vTemp.y = a_rcEnd.y - a_rcStart.y;
	vTemp.z = a_rcEnd.z - a_rcStart.z;
	vTemp.w = a_rcEnd.w;

	TVector4 vDirection;
	TMatrix44::RotateVector( vDirection, oInverse, vTemp );

	// Mirror the segment so that the direction is positive
	TFLOAT afSign[ 3 ];

	for ( TINT i = 0; i < 3; i++ )
	{
		if ( vDirection.AsArray()[ i ] >= 0.0f )
		{
			afSign[ i ] = 1.0f;
		}
		else
		{
			afSign[ i ]               = -1.0f;
			vPoint.AsArray()[ i ]     = -vPoint.AsArray()[ i ];
			vDirection.AsArray()[ i ] = -vDirection.AsArray()[ i ];
		}
	}

	TFLOAT afExtents[ 3 ] = { a_rcBox.m_vExtents.x, a_rcBox.m_vExtents.y, a_rcBox.m_vExtents.z };
	TFLOAT afDirSq[ 3 ]   = { vDirection.x * vDirection.x, vDirection.y * vDirection.y, vDirection.z * vDirection.z };

	TINT   aiRegion[ 3 ];
	TFLOAT afTanchor[ 3 ];

	for ( TINT i = 0; i < 3; i++ )
	{
		if ( vDirection.AsArray()[ i ] <= 0.0f )
		{
			aiRegion[ i ]  = 0;
			afTanchor[ i ] = 2.0f;
		}
		else
		{
			TFLOAT fMin = -afExtents[ i ];

			if ( vPoint.AsArray()[ i ] >= fMin )
			{
				aiRegion[ i ]  = vPoint.AsArray()[ i ] > afExtents[ i ];
				afTanchor[ i ] = ( afExtents[ i ] - vPoint.AsArray()[ i ] ) / vDirection.AsArray()[ i ];
			}
			else
			{
				aiRegion[ i ]  = -1;
				afTanchor[ i ] = ( fMin - vPoint.AsArray()[ i ] ) / vDirection.AsArray()[ i ];
			}
		}
	}

	// Find the t of the closest point by stepping between the region changes
	TFLOAT fT = 0.0f;
	TFLOAT fDd2dt =
	    -( ( aiRegion[ 0 ] ? afDirSq[ 0 ] : 0.0f ) * afTanchor[ 0 ] ) -
	    ( aiRegion[ 1 ] ? afDirSq[ 1 ] : 0.0f ) * afTanchor[ 1 ] -
	    ( aiRegion[ 2 ] ? afDirSq[ 2 ] : 0.0f ) * afTanchor[ 2 ];

	if ( fDd2dt < 0.0f )
	{
		while ( TTRUE )
		{
			TFLOAT fNextT = 1.0f;

			if ( afTanchor[ 0 ] > fT && afTanchor[ 0 ] < 1.0f )
				fNextT = afTanchor[ 0 ];

			if ( afTanchor[ 1 ] > fT && afTanchor[ 1 ] < 1.0f && afTanchor[ 1 ] < fNextT )
				fNextT = afTanchor[ 1 ];

			if ( afTanchor[ 2 ] > fT && afTanchor[ 2 ] < 1.0f && afTanchor[ 2 ] < fNextT )
				fNextT = afTanchor[ 2 ];

			TFLOAT fNextDd2dt =
			    ( aiRegion[ 0 ] ? afDirSq[ 0 ] : 0.0f ) * ( fNextT - afTanchor[ 0 ] ) +
			    ( fNextT - afTanchor[ 1 ] ) * ( aiRegion[ 1 ] ? afDirSq[ 1 ] : 0.0f ) +
			    ( fNextT - afTanchor[ 2 ] ) * ( aiRegion[ 2 ] ? afDirSq[ 2 ] : 0.0f );

			if ( fNextDd2dt >= 0.0f )
			{
				fT = fT - fDd2dt / ( ( fNextDd2dt - fDd2dt ) / ( fNextT - fT ) );
				break;
			}

			for ( TINT i = 0; i < 3; i++ )
			{
				if ( afTanchor[ i ] == fNextT )
				{
					aiRegion[ i ]++;
					afTanchor[ i ] = ( afExtents[ i ] - vPoint.AsArray()[ i ] ) / vDirection.AsArray()[ i ];
				}
			}

			fT     = fNextT;
			fDd2dt = fNextDd2dt;

			if ( fNextT >= 1.0f )
			{
				fT = 1.0f;
				break;
			}
		}
	}

	// Closest point of the segment
	TVector4 vSegment;
	vSegment.x = a_rcEnd.x - a_rcStart.x;
	vSegment.y = a_rcEnd.y - a_rcStart.y;
	vSegment.z = a_rcEnd.z - a_rcStart.z;
	vSegment.w = a_rcEnd.w;
	vSegment.Multiply( vSegment, fT );

	a_rOutSegmentPoint.Set( vSegment.x + a_rcStart.x, vSegment.y + a_rcStart.y, vSegment.z + a_rcStart.z, a_rcStart.w );

	// Closest point of the box
	TVector4 vBoxPoint;
	vBoxPoint.Multiply( vDirection, fT );
	vBoxPoint.Set( vBoxPoint.x + vPoint.x, vBoxPoint.y + vPoint.y, vBoxPoint.z + vPoint.z, vPoint.w );

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fExtent           = afExtents[ i ];
		vBoxPoint.AsArray()[ i ] = afSign[ i ] * vBoxPoint.AsArray()[ i ];
		TMath::Clip( vBoxPoint.AsArray()[ i ], -fExtent, fExtent );
	}

	TMatrix44::RotateVector( a_rOutBoxPoint, a_rcBox.m_oTransform, vBoxPoint );
	a_rOutBoxPoint.x = a_rcBox.m_oTransform.m_f41 + a_rOutBoxPoint.x;
	a_rOutBoxPoint.y = a_rcBox.m_oTransform.m_f42 + a_rOutBoxPoint.y;
	a_rOutBoxPoint.z = a_rcBox.m_oTransform.m_f43 + a_rOutBoxPoint.z;
}

// $deBlob: FUNCTION 00830c20
// $Barnyard: FUNCTION 006d0af0
static void LineClosestApproach( const TVector4& a_rcPointA, const TVector4& a_rcDirA, const TVector4& a_rcPointB, const TVector4& a_rcDirB, TFLOAT& a_rfAlpha, TFLOAT& a_rfBeta )
{
	TVector4 vDistance( a_rcPointB.x - a_rcPointA.x, a_rcPointB.y - a_rcPointA.y, a_rcPointB.z - a_rcPointA.z, a_rcPointB.w );

	TFLOAT fDirDot = TVector4::DotProduct3( a_rcDirA, a_rcDirB );
	TFLOAT fQ1     = TVector4::DotProduct3( a_rcDirA, vDistance );
	TFLOAT fQ2     = -TVector4::DotProduct3( a_rcDirB, vDistance );
	TFLOAT fDet    = 1.0f - fDirDot * fDirDot;

	if ( fDet > 0.000099999997f )
	{
		fDet      = 1.0f / fDet;
		a_rfAlpha = ( fQ2 * fDirDot + fQ1 ) * fDet;
		a_rfBeta  = ( fQ1 * fDirDot + fQ2 ) * fDet;
	}
	else
	{
		a_rfAlpha = 0.0f;
		a_rfBeta  = 0.0f;
	}
}

// $deBlob: FUNCTION 00830d50
// $Barnyard: FUNCTION 006d0bd0
static TINT IntersectRectQuad( const TFLOAT* a_pRectangle, TFLOAT* a_pQuad, TFLOAT* a_pOutPoints )
{
	// Clips the quad to the rectangle (-a_pRectangle[ 0 ], -a_pRectangle[ 1 ]) - (a_pRectangle[ 0 ], a_pRectangle[ 1 ])
	TINT    iNumQuad = 4;
	TINT    iNumResult;
	TFLOAT  afBuffer[ 16 ];
	TFLOAT* pQuad   = a_pQuad;
	TFLOAT* pResult = a_pOutPoints;

	for ( TINT iDir = 0; iDir <= 1; iDir++ )
	{
		for ( TINT iSign = -1; iSign <= 1; iSign += 2 )
		{
			TFLOAT* pQ = pQuad;
			TFLOAT* pR = pResult;
			iNumResult = 0;

			for ( TINT i = iNumQuad; i > 0; i-- )
			{
				if ( iSign * pQ[ iDir ] < a_pRectangle[ iDir ] )
				{
					// The point is inside
					pR[ 0 ] = pQ[ 0 ];
					pR[ 1 ] = pQ[ 1 ];
					pR += 2;
					iNumResult++;

					if ( iNumResult & 8 )
					{
						pQuad = pResult;
						goto DONE;
					}
				}

				TFLOAT* pNextQ = ( i > 1 ) ? pQ + 2 : pQuad;

				if ( ( iSign * pQ[ iDir ] < a_pRectangle[ iDir ] ) ^ ( iSign * pNextQ[ iDir ] < a_pRectangle[ iDir ] ) )
				{
					// The edge crosses the side of the rectangle
					pR[ 1 - iDir ] = pQ[ 1 - iDir ] + ( pNextQ[ 1 - iDir ] - pQ[ 1 - iDir ] ) / ( pNextQ[ iDir ] - pQ[ iDir ] ) * ( iSign * a_pRectangle[ iDir ] - pQ[ iDir ] );
					pR[ iDir ]     = iSign * a_pRectangle[ iDir ];
					pR += 2;
					iNumResult++;

					if ( iNumResult & 8 )
					{
						pQuad = pResult;
						goto DONE;
					}
				}

				pQ += 2;
			}

			pQuad    = pResult;
			pResult  = ( pQuad == a_pOutPoints ) ? afBuffer : a_pOutPoints;
			iNumQuad = iNumResult;
		}
	}

DONE:
	if ( pQuad != a_pOutPoints )
		TUtil::MemCopy( a_pOutPoints, pQuad, iNumResult * 2 * sizeof( TFLOAT ) );

	return iNumResult;
}

// $deBlob: FUNCTION 008313a0
// $Barnyard: FUNCTION 006d8520
TBOOL TOBBox::CollideSphereSphere( const TVector4& a_rcSphere1, const TVector4& a_rcSphere2, TCollisionContact& a_rContact )
{
	TVector4 vDistance( a_rcSphere1.x - a_rcSphere2.x, a_rcSphere1.y - a_rcSphere2.y, a_rcSphere1.z - a_rcSphere2.z, 1.0f );
	TFLOAT   fDistance = VectorLength( vDistance );

	if ( a_rcSphere1.w + a_rcSphere2.w < fDistance )
		return TFALSE;

	if ( fDistance > 0.0f )
	{
		TFLOAT fInvDistance = 1.0f / fDistance;

		a_rContact.vNormal.w = 1.0f;
		a_rContact.vNormal.z = ( a_rcSphere1.z - a_rcSphere2.z ) * fInvDistance;
		a_rContact.vNormal.x = ( a_rcSphere1.x - a_rcSphere2.x ) * fInvDistance;
		a_rContact.vNormal.y = ( a_rcSphere1.y - a_rcSphere2.y ) * fInvDistance;

		TVector4 vOffset;
		vOffset.Multiply( a_rContact.vNormal, ( a_rcSphere2.w - a_rcSphere1.w - fDistance ) * 0.5f );

		TVector4 vCentre( a_rcSphere1.x, a_rcSphere1.y, a_rcSphere1.z, 1.0f );
		a_rContact.vPosition.Add( vCentre, vOffset );
		a_rContact.fDepth = a_rcSphere1.w + a_rcSphere2.w - fDistance;
	}
	else
	{
		a_rContact.vNormal = TVector4::VEC_POSY;
		a_rContact.vPosition.Set( a_rcSphere1.x, a_rcSphere1.y, a_rcSphere1.z, 1.0f );
		a_rContact.fDepth = a_rcSphere1.w + a_rcSphere2.w;
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END

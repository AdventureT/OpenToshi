#include "ToshiPCH.h"

#include "TMath.h"
#include "TVector3.h"
#include "TMathInline.h"
#include "TMatrix44.h"
#include "TSphere.h"

TOSHI_NAMESPACE_START

// NOTE: retail sets the x87 precision control to double (control word 0x27f) around the CRT's math functions (ASin, ACos,
// ATan, Exp, Log, Log10, Pow) and restores it afterwards
#define TMATH_DOUBLE_PRECISION_BEGIN()                  \
	TUINT32 uiControlWord;                               \
	TUINT32 uiOldControlWord;                            \
	__asm { fnstcw word ptr[ uiControlWord ] }           \
	uiOldControlWord = TUINT16( uiControlWord );         \
	uiControlWord    = 0x27f;                            \
	__asm { fldcw word ptr[ uiControlWord ] }

#define TMATH_DOUBLE_PRECISION_END() __asm { fldcw word ptr[ uiOldControlWord ] }

// $deBlob: FUNCTION 00666eb0
// $Barnyard: FUNCTION 006b7370
void TMath::SinCos( TFLOAT fVal, TFLOAT& a_rSin, TFLOAT& a_rCos )
{
	a_rSin = TFLOAT( sin( fVal ) );
	a_rCos = TFLOAT( cos( fVal ) );
}

// NOTE: x87 code like the original (rounds with fist and then moves the result one towards zero if it passed the value, comparing the
// float's bits), so it is never inlined
// $deBlob: FUNCTION 006671a0
__declspec( naked ) TINT TMath::TruncToInt( TFLOAT a_fVal )
{
	__asm
	{
		fld dword ptr [esp + 4]
		sub esp, 0xc
		fist dword ptr [esp]
		fst dword ptr [esp + 4]
		fisub dword ptr [esp]
		fstp dword ptr [esp + 8]
		pop eax
		pop ecx
		pop edx
		test ecx, ecx
		js negative
		add edx, 0x7fffffff
		sbb eax, 0
		ret 4
	negative:
		xor ecx, ecx
		test edx, edx
		setg cl
		add eax, ecx
		ret 4
	}
}

// $deBlob: FUNCTION 006671e0
// $Barnyard: FUNCTION 006b69b0
// NOTE: x87 code like the original (rounds with fist and then subtracts one if the difference is negative, comparing the float's
// bits), so it is never inlined
__declspec( naked ) TINT TMath::FloorToInt( TFLOAT a_fVal )
{
	__asm
	{
		fld dword ptr [esp + 4]
		sub esp, 8
		fist dword ptr [esp]
		fisub dword ptr [esp]
		fstp dword ptr [esp + 4]
		pop eax
		pop edx
		add edx, 0x7fffffff
		sbb eax, 0
		ret 4
	}
}

// $deBlob: FUNCTION 00667200
// $Barnyard: FUNCTION 006b69d0
// NOTE: x87 code like the original (rounds with fist and then adds one if the difference is positive, comparing the float's bits),
// so it is never inlined
__declspec( naked ) TINT TMath::CeilToInt( TFLOAT a_fVal )
{
	__asm
	{
		fld dword ptr [esp + 4]
		sub esp, 8
		fist dword ptr [esp]
		fisub dword ptr [esp]
		fstp dword ptr [esp + 4]
		pop eax
		pop edx
		xor ecx, ecx
		test edx, edx
		setg cl
		add eax, ecx
		ret 4
	}
}

// $deBlob: FUNCTION 00667460
// $deBlob: FUNCTION 00667490
// $Barnyard: FUNCTION 006b6870
// $Barnyard: FUNCTION 006b68a0
template <typename T>
void TMath::Clip( T& rVal, T Min, T Max )
{
	// NOTE: else-if like the original: with Min > Max a value above Max ends up at Max, not at Min
	if ( rVal > Max ) { rVal = Max; }
	else if ( rVal < Min ) { rVal = Min; }
}

// $deBlob: FUNCTION 006674b0
// $deBlob: FUNCTION 006674e0
// $Barnyard: FUNCTION 006b68c0
template <typename T>
void TMath::Wrap( T& a_rfValue, T a_fMin, T a_fMax )
{
	if ( a_rfValue > a_fMax )
		a_rfValue = a_fMin;
	else if ( a_rfValue < a_fMin )
		a_rfValue = a_fMax;
}

template void TMath::Wrap<TFLOAT>( TFLOAT&, TFLOAT, TFLOAT );
template void TMath::Wrap<TINT>( TINT&, TINT, TINT );
template void TMath::Clip<TFLOAT>( TFLOAT&, TFLOAT, TFLOAT );
template void TMath::Clip<TINT>( TINT&, TINT, TINT );

// $deBlob: FUNCTION 00667760
// $Barnyard: FUNCTION 006b6640
TFLOAT TMath::LERP( TFLOAT a, TFLOAT b, TFLOAT t )
{
	return ( b - a ) * t + a;
}

// Moves a_fFrom towards a_fTo by a_fT without passing it
// $deBlob: FUNCTION 00667790
TFLOAT TMath::LerpTowards( TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fT )
{
	TFLOAT fValue = ( a_fTo - a_fFrom ) * a_fT + a_fFrom;

	if ( ( a_fFrom < a_fTo && a_fTo < fValue ) || ( a_fTo < a_fFrom && fValue < a_fTo ) )
		fValue = a_fTo;

	return fValue;
}

// Interpolates from the angle a_fFrom to a_fTo (radians) by a_fProgress, going the short way around
// $deBlob: FUNCTION 0066d5f0
// $Barnyard: FUNCTION 006b8f40
TFLOAT TMath::LerpAngle( TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fProgress )
{
	TFLOAT fDiff    = a_fTo - a_fFrom;
	TFLOAT fAbsDiff = fabsf( fDiff );

	if ( fAbsDiff <= PI )
		return fDiff * a_fProgress + a_fFrom;

	TFLOAT fStep = ( TWO_PI - fAbsDiff ) * a_fProgress;

	if ( a_fFrom <= a_fTo )
		fStep = -fStep;

	TFLOAT fResult = fStep + a_fFrom;
	NormaliseAngle( fResult );

	return fResult;
}

// $deBlob: FUNCTION 0066d7a0
TFLOAT TMath::GetAngleDifference( TFLOAT a_fFrom, TFLOAT a_fTo )
{
	TFLOAT fDifference = a_fTo - a_fFrom;
	TFLOAT fResult     = fDifference;

	if ( TMath::Abs( fDifference ) > TMath::PI )
	{
		fResult = TMath::TWO_PI - TMath::Abs( fDifference );
		if ( fDifference > 0.0f )
			fResult = -fResult;
	}

	return fResult;
}

// $deBlob: FUNCTION 0066d530
TVector4 TMath::LERPVector4Clip( TVector4 a_vFrom, TVector4 a_vTo, TFLOAT a_fT )
{
	TFLOAT afResult[ 3 ];

	for ( TINT i = 0; i < 3; i++ )
	{
		const TFLOAT fFrom = a_vFrom.AsArray()[ i ];
		const TFLOAT fTo   = a_vTo.AsArray()[ i ];
		TFLOAT       fLerp = ( fTo - fFrom ) * a_fT + fFrom;

		if ( ( fFrom < fTo && fTo < fLerp ) || ( fTo < fFrom && fLerp < fTo ) )
			fLerp = fTo;

		afResult[ i ] = fLerp;
	}

	return TVector4( afResult[ 0 ], afResult[ 1 ], afResult[ 2 ], a_vFrom.w );
}

// $deBlob: FUNCTION 0066d800
TFLOAT TMath::GetDirectedAngleDifference( TFLOAT a_fFrom, TFLOAT a_fTo, TBOOL a_bPositive )
{
	TFLOAT fDifference = a_fTo - a_fFrom;
	TFLOAT fResult     = fDifference;

	if ( ( 0.0f <= fDifference ) != a_bPositive )
	{
		fResult = TMath::TWO_PI - TMath::Abs( fDifference );

		if ( 0.0f < fDifference )
			fResult = -fResult;
	}

	return fResult;
}

// $deBlob: FUNCTION 00669f80
TBOOL TMath::PointVsPlane2D( const TVector2& a_rcNormal, const TVector2& a_rcPlanePoint, const TVector2& a_rcPoint )
{
	return 0.0f <= a_rcNormal.y * ( a_rcPoint.y - a_rcPlanePoint.y ) + a_rcNormal.x * ( a_rcPoint.x - a_rcPlanePoint.x );
}

// $deBlob: FUNCTION 00669fe0
TFLOAT TMath::PointVsPlane2DF( const TVector2& a_rcNormal, const TVector2& a_rcPlanePoint, const TVector2& a_rcPoint )
{
	TVector2 vDiff = a_rcPoint;
	vDiff -= a_rcPlanePoint;
	return a_rcNormal.y * vDiff.y + a_rcNormal.x * vDiff.x;
}

// $deBlob: FUNCTION 0066e2c0
void TMath::CalcCardinalSplineTangent( TVector3& a_rOutTangent, const TVector3& a_rcPrevious, const TVector3& a_rcNext, TFLOAT a_fTension )
{
	a_rOutTangent.x = ( a_rcNext.x - a_rcPrevious.x ) * a_fTension;
	a_rOutTangent.y = ( a_rcNext.y - a_rcPrevious.y ) * a_fTension;
	a_rOutTangent.z = ( a_rcNext.z - a_rcPrevious.z ) * a_fTension;
}

// $deBlob: FUNCTION 0066da60
TFLOAT TMath::AngleBetweenXZ( const TVector4& a_vA, const TVector4& a_vB )
{
	TASSERT( a_vA.IsNormalised() );
	TASSERT( a_vB.IsNormalised() );

	return -TMath::ATan2( a_vA.x * a_vB.z - a_vB.x * a_vA.z, a_vB.x * a_vA.x + a_vA.z * a_vB.z );
}

// $deBlob: FUNCTION 0066dbb0
// $Barnyard: FUNCTION 006b7670
TBOOL TMath::ConeVsSphere( const TVector4& a_rvConePosition, const TVector4& a_rvConeDirection, TFLOAT a_fConeFOV, TFLOAT a_fSphereRadius, const TVector4& a_rvSpherePosition )
{
	TASSERT( a_fConeFOV < TMath::HALF_PI );
	TASSERT( a_rvConeDirection.IsNormalised() );

	TVector3 direction = a_rvSpherePosition.AsVector3() - a_rvConePosition.AsVector3();

	// NOTE: de Blob sums y, x, z
	TFLOAT fDotProduct = a_rvConeDirection.y * direction.y + a_rvConeDirection.x * direction.x + a_rvConeDirection.z * direction.z;

	if ( fDotProduct > 0.0f && fDotProduct <= a_fSphereRadius )
	{
		direction.Normalize();

		fDotProduct = a_rvConeDirection.y * direction.y + a_rvConeDirection.x * direction.x + a_rvConeDirection.z * direction.z;
		TMath::Clip( fDotProduct, -1.0f, 1.0f );

		return ACos( fDotProduct ) <= a_fConeFOV;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0066dd10
TBOOL TMath::ConeVsSphere( const TVector4& a_rvConePosition, const TVector4& a_rvConeDirection, TFLOAT a_fConeFOV, TFLOAT a_fConeLength, const TVector4& a_rvSpherePosition, TFLOAT a_fSphereRadius )
{
	TASSERT( a_fConeFOV < TMath::HALF_PI );

	TVector3 direction = a_rvSpherePosition.AsVector3() - a_rvConePosition.AsVector3();
	TFLOAT   fDistSq   = direction.MagnitudeSq();

	if ( fDistSq <= a_fSphereRadius * a_fSphereRadius )
		return TTRUE;

	TFLOAT fReach = a_fConeLength + a_fSphereRadius;
	if ( fDistSq > fReach * fReach )
		return TFALSE;

	// NOTE: de Blob sums y, x, z
	TFLOAT fDotProduct = a_rvConeDirection.y * direction.y + a_rvConeDirection.x * direction.x + a_rvConeDirection.z * direction.z;
	if ( fDotProduct < 0.0f )
		return TFALSE;

	TFLOAT fDistance    = TMath::Sqrt( fDistSq );
	TFLOAT fInvDistance = 1.0f / fDistance;
	TFLOAT fHalfAngle   = ATan( fInvDistance * a_fSphereRadius );

	fDotProduct = fInvDistance * fDotProduct;
	TMath::Clip( fDotProduct, -1.0f, 1.0f );

	return ACos( fDotProduct ) - fHalfAngle <= a_fConeFOV;
}

// $deBlob: FUNCTION 00669bd0
// $Barnyard: FUNCTION 006b6080
TINT TMath::RayVsCircle3D( const TVector4& a_rcRayOrigin, const TVector4& a_rcRayDir, const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_pNearIntersection, TFLOAT& a_pFarIntersection )
{
	TVector4 vecRayToCenter  = a_rcCenter - a_rcRayOrigin;
	TFLOAT   fRadiusSq       = a_fRadius * a_fRadius;
	TFLOAT   fDistToCenterSq = vecRayToCenter.MagnitudeSq();

	if ( fRadiusSq <= fDistToCenterSq )
	{
		TFLOAT fDot = TVector4::DotProduct3( vecRayToCenter, a_rcRayDir );

		if ( fDot < 0.0f )
		{
			// The ray direction is pointing against the center
			return 0;
		}

		if ( fRadiusSq < fDistToCenterSq - fDot * fDot )
		{
			// The ray won't hit the circle
			return 0;
		}
	}

	// Negate to make it point from the center to the ray
	vecRayToCenter *= -1.0f;
	TFLOAT fDot = TVector4::DotProduct3( vecRayToCenter, a_rcRayDir ) * 2;

	fRadiusSq = fDot * fDot - ( fDistToCenterSq - fRadiusSq ) * 4.0f;

	if ( fRadiusSq < 0.0f )
		return 0;

	TFLOAT fRadius      = TMath::Sqrt( fRadiusSq );
	a_pNearIntersection = ( -fDot - fRadius ) * 0.5f;
	a_pFarIntersection  = ( fRadius - fDot ) * 0.5f;

	return 2;
}

// $deBlob: FUNCTION 0066e890
// $Barnyard: FUNCTION 006b7c40
TBOOL TMath::SphereVsLine( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TINT* a_pNumHits, TFLOAT* a_pNearIntersection, TFLOAT* a_pFarIntersection )
{
	TVector4 vecStartToEnd = a_rcLineEnd - a_rcLineStart;

	TFLOAT a = vecStartToEnd.MagnitudeSq();
	TFLOAT b = ( ( a_rcLineStart.y - a_rcCenter.y ) * vecStartToEnd.y + ( a_rcLineStart.z - a_rcCenter.z ) * vecStartToEnd.z + ( a_rcLineStart.x - a_rcCenter.x ) * vecStartToEnd.x ) * 2.0f;

	TFLOAT fDot          = TVector4::DotProduct3( a_rcCenter, a_rcLineStart );
	TFLOAT fDiscriminant = b * b - ( ( ( a_rcCenter.MagnitudeSq() + a_rcLineStart.MagnitudeSq() ) - ( fDot * 2.0f ) ) - a_fRadius * a_fRadius ) * a * 4.0f;

	if ( fDiscriminant < 0.0f )
		return TFALSE;

	if ( fDiscriminant == 0.0f )
	{
		if ( a_pNumHits )
			*a_pNumHits = 1;

		if ( a_pNearIntersection )
			*a_pNearIntersection = -( b / ( a * 2.0f ) );
	}
	else
	{
		if ( a_pNumHits )
			*a_pNumHits = 2;

		if ( a_pNearIntersection )
			*a_pNearIntersection = ( TMath::Sqrt( fDiscriminant ) - b ) / ( a * 2.0f );

		if ( a_pFarIntersection )
			*a_pFarIntersection = ( -b - TMath::Sqrt( fDiscriminant ) ) / ( a * 2.0f );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00671ee0
// $Barnyard: FUNCTION 006b7dc0
TFLOAT TMath::DistancePointToLineSq( const TVector4& a_rcPoint, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TVector4& a_rClosestPoint )
{
	TVector4 vecStartToPoint = a_rcPoint - a_rcLineStart;
	TVector4 vecStartToEnd   = a_rcLineEnd - a_rcLineStart;

	TFLOAT fLineLengthSq = vecStartToEnd.MagnitudeSq();
	TFLOAT fDot          = TVector4::DotProduct3( vecStartToPoint, vecStartToEnd );

	TVector4 vecProjection;
	vecProjection.Multiply( vecStartToEnd, fDot / fLineLengthSq );

	a_rClosestPoint.Set( vecProjection.x + a_rcLineStart.x, vecProjection.y + a_rcLineStart.y, vecProjection.z + a_rcLineStart.z, a_rcLineStart.w );

	TVector4 vecDiff = vecStartToPoint - vecProjection;
	return vecDiff.MagnitudeSq();
}

// $deBlob: FUNCTION 00669530
// $Barnyard: FUNCTION 006b7770
void TMath::TriangleNormalCounterClockwise( TVector3& a_rNormal, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 )
{
	TVector3 vecVertex1To2 = a_rcVertex2 - a_rcVertex1;
	TVector3 vecVertex1To3 = a_rcVertex3 - a_rcVertex1;

	a_rNormal.CrossProduct( vecVertex1To2, vecVertex1To3 );
	a_rNormal.Normalize();
}

// $deBlob: FUNCTION 00669490
void TMath::TriangleNormalCounterClockwise( TVector4& a_rNormal, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 )
{
	TVector4 vecEdge1( a_rcVertex2.x - a_rcVertex1.x, a_rcVertex2.y - a_rcVertex1.y, a_rcVertex2.z - a_rcVertex1.z, 1.0f );
	TVector4 vecEdge2( a_rcVertex3.x - a_rcVertex1.x, a_rcVertex3.y - a_rcVertex1.y, a_rcVertex3.z - a_rcVertex1.z, 1.0f );

	a_rNormal.CrossProduct( vecEdge1, vecEdge2 );
	a_rNormal.Normalise();
}

// $deBlob: FUNCTION 006695c0
void TMath::TriangleNormalCounterClockwise( TVector4& a_rNormal, const TVector3* const* a_ppVertices )
{
	TVector4 vecEdge1( a_ppVertices[ 1 ]->x - a_ppVertices[ 0 ]->x, a_ppVertices[ 1 ]->y - a_ppVertices[ 0 ]->y, a_ppVertices[ 1 ]->z - a_ppVertices[ 0 ]->z, 1.0f );
	TVector4 vecEdge2( a_ppVertices[ 2 ]->x - a_ppVertices[ 0 ]->x, a_ppVertices[ 2 ]->y - a_ppVertices[ 0 ]->y, a_ppVertices[ 2 ]->z - a_ppVertices[ 0 ]->z, 1.0f );

	a_rNormal.CrossProduct( vecEdge1, vecEdge2 );
	a_rNormal.Normalise();
}

// $deBlob: FUNCTION 00669660
void TMath::TriangleNormalCounterClockwise( TVector3& a_rNormal, const TVector3* const* a_ppVertices )
{
	TriangleUnscaledNormalCounterClockwise( a_rNormal, a_ppVertices );
	a_rNormal.Normalize();
}

// $deBlob: FUNCTION 006696f0
void TMath::TriangleUnscaledNormalCounterClockwise( TVector4& a_rNormal, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 )
{
	TVector4 vecEdge1( a_rcVertex2.x - a_rcVertex1.x, a_rcVertex2.y - a_rcVertex1.y, a_rcVertex2.z - a_rcVertex1.z, 1.0f );
	TVector4 vecEdge2( a_rcVertex3.x - a_rcVertex1.x, a_rcVertex3.y - a_rcVertex1.y, a_rcVertex3.z - a_rcVertex1.z, 1.0f );

	a_rNormal.CrossProduct( vecEdge1, vecEdge2 );
}

// $deBlob: FUNCTION 00669780
void TMath::TriangleUnscaledNormalCounterClockwise( TVector3& a_rNormal, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 )
{
	TFLOAT fEdge1X = a_rcVertex2.x - a_rcVertex1.x, fEdge1Y = a_rcVertex2.y - a_rcVertex1.y, fEdge1Z = a_rcVertex2.z - a_rcVertex1.z;
	TFLOAT fEdge2X = a_rcVertex3.x - a_rcVertex1.x, fEdge2Y = a_rcVertex3.y - a_rcVertex1.y, fEdge2Z = a_rcVertex3.z - a_rcVertex1.z;

	a_rNormal.x = fEdge2Z * fEdge1Y - fEdge2Y * fEdge1Z;
	a_rNormal.y = fEdge2X * fEdge1Z - fEdge2Z * fEdge1X;
	a_rNormal.z = fEdge2Y * fEdge1X - fEdge2X * fEdge1Y;
}

// $deBlob: FUNCTION 00669810
void TMath::TriangleUnscaledNormalCounterClockwise( TVector4& a_rNormal, const TVector4* const* a_ppVertices )
{
	TVector4 vecEdge1( a_ppVertices[ 1 ]->x - a_ppVertices[ 0 ]->x, a_ppVertices[ 1 ]->y - a_ppVertices[ 0 ]->y, a_ppVertices[ 1 ]->z - a_ppVertices[ 0 ]->z, a_ppVertices[ 1 ]->w );
	TVector4 vecEdge2( a_ppVertices[ 2 ]->x - a_ppVertices[ 0 ]->x, a_ppVertices[ 2 ]->y - a_ppVertices[ 0 ]->y, a_ppVertices[ 2 ]->z - a_ppVertices[ 0 ]->z, a_ppVertices[ 2 ]->w );

	a_rNormal.CrossProduct( vecEdge1, vecEdge2 );
}

// $deBlob: FUNCTION 006698b0
void TMath::TriangleUnscaledNormalCounterClockwise( TVector3& a_rNormal, const TVector3* const* a_ppVertices )
{
	TriangleUnscaledNormalCounterClockwise( a_rNormal, *a_ppVertices[ 0 ], *a_ppVertices[ 1 ], *a_ppVertices[ 2 ] );
}

// $deBlob: FUNCTION 00669390
TBOOL TMath::AABBVsAABB( const TVector4& a_rcCenter1, const TVector4& a_rcHalfExtents1, const TVector4& a_rcCenter2, const TVector4& a_rcHalfExtents2, TBOOL* a_pCenter1Inside )
{
	for ( TINT i = 0; i < 3; i++ )
	{
		const TFLOAT fCenter1 = a_rcCenter1.AsArray()[ i ];
		const TFLOAT fHalf1   = a_rcHalfExtents1.AsArray()[ i ];
		const TFLOAT fCenter2 = a_rcCenter2.AsArray()[ i ];
		const TFLOAT fHalf2   = a_rcHalfExtents2.AsArray()[ i ];

		if ( fHalf2 + fCenter2 < fCenter1 - fHalf1 || fCenter1 + fHalf1 < fCenter2 - fHalf2 )
		{
			if ( a_pCenter1Inside != TNULL )
			{
				if ( a_rcCenter2.x - a_rcHalfExtents2.x <= a_rcCenter1.x && a_rcCenter2.y - a_rcHalfExtents2.y <= a_rcCenter1.y && a_rcCenter2.z - a_rcHalfExtents2.z <= a_rcCenter1.z &&
				     a_rcCenter1.x <= a_rcHalfExtents2.x + a_rcCenter2.x && a_rcCenter1.y <= a_rcHalfExtents2.y + a_rcCenter2.y && a_rcCenter1.z <= a_rcHalfExtents2.z + a_rcCenter2.z )
				{
					*a_pCenter1Inside = TTRUE;
					return TFALSE;
				}

				*a_pCenter1Inside = TFALSE;
			}

			return TFALSE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006b77f0
void TMath::TriangleUnscaledNormalCounterClockwise( TVector4& a_rNormal, const TVector3* const* a_ppVertices )
{
	TVector4 vecEdge1( a_ppVertices[ 1 ]->x - a_ppVertices[ 0 ]->x, a_ppVertices[ 1 ]->y - a_ppVertices[ 0 ]->y, a_ppVertices[ 1 ]->z - a_ppVertices[ 0 ]->z, 1.0f );
	TVector4 vecEdge2( a_ppVertices[ 2 ]->x - a_ppVertices[ 0 ]->x, a_ppVertices[ 2 ]->y - a_ppVertices[ 0 ]->y, a_ppVertices[ 2 ]->z - a_ppVertices[ 0 ]->z, 1.0f );

	a_rNormal.CrossProduct( vecEdge1, vecEdge2 );
}

// $Barnyard: FUNCTION 006d8490
void TMath::TrianglePlane( TPlane& a_rOutPlane, const TVector3& a_rcVertex1, const TVector3& a_rcVertex2, const TVector3& a_rcVertex3 )
{
	TriangleNormalCounterClockwise( a_rOutPlane.AsNormal(), a_rcVertex1, a_rcVertex2, a_rcVertex3 );
	a_rOutPlane.SetD( TVector3::DotProduct( a_rcVertex1, a_rOutPlane.GetNormal() ) );
}

// $deBlob: FUNCTION 0066eac0
// $Barnyard: FUNCTION 006b7860
TBOOL TMath::IsPointInTriangle( const TVector4& a_rcVertex1, const TVector4& a_rcVertex2, const TVector4& a_rcVertex3, const TVector4& a_rcPoint )
{
	// Barycentric coordinates of the point
	TVector4 vecEdge1 = a_rcVertex2 - a_rcVertex1;
	TVector4 vecEdge2 = a_rcVertex3 - a_rcVertex1;
	TVector4 vecPoint = a_rcPoint - a_rcVertex1;

	TFLOAT fDot11 = TVector4::DotProduct3( vecEdge1, vecEdge1 );
	TFLOAT fDot12 = TVector4::DotProduct3( vecEdge1, vecEdge2 );
	TFLOAT fDot22 = TVector4::DotProduct3( vecEdge2, vecEdge2 );
	TFLOAT fDotP1 = TVector4::DotProduct3( vecPoint, vecEdge1 );
	TFLOAT fDotP2 = TVector4::DotProduct3( vecPoint, vecEdge2 );

	TFLOAT fInvDenom = 1.0f / ( fDot12 * fDot12 - fDot22 * fDot11 );
	TFLOAT fU        = ( fDotP2 * fDot12 - fDotP1 * fDot22 ) * fInvDenom;

	if ( fU >= 0.0f && fU <= 1.0f )
	{
		TFLOAT fV = ( fDotP1 * fDot12 - fDotP2 * fDot11 ) * fInvDenom;

		if ( fV >= 0.0f && fU + fV <= 1.0f )
			return TTRUE;
	}

	return TFALSE;
}

// NOTE: retail inlines the barycentric test of IsPointInTriangle
// $deBlob: FUNCTION 0066edd0
// $Barnyard: FUNCTION 006b7b80
TBOOL TMath::IsPointInPolygon( TINT a_iNumVertices, const TVector3* const* a_ppVertices, const TVector4& a_rcPoint )
{
	TVector4 vecFirst( *a_ppVertices[ 0 ] );

	// Test every triangle of the fan
	for ( TINT i = 0; i < a_iNumVertices - 2; i++ )
	{
		TVector4 vecSecond( *a_ppVertices[ i + 1 ] );
		TVector4 vecThird( *a_ppVertices[ i + 2 ] );

		if ( IsPointInTriangle( vecFirst, vecSecond, vecThird, a_rcPoint ) )
			return TTRUE;
	}

	return TFALSE;
}

// The Hermite basis and its derivative
static TCONSTEXPR_VAR TMatrix44 s_mHermite = TMatrix44(
    2.0f,
    -2.0f,
    1.0f,
    1.0f,
    -3.0f,
    3.0f,
    -2.0f,
    -1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f
);

static TCONSTEXPR_VAR TMatrix44 s_mHermiteTangent = TMatrix44(
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    6.0f,
    -6.0f,
    3.0f,
    3.0f,
    -6.0f,
    6.0f,
    -4.0f,
    -2.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f
);

// The point at a_fT of the Hermite curve from a_rvStart to a_rvEnd
// $deBlob: FUNCTION 0066e320
void TMath::HermitePosition( TVector4& a_rvOut, const TVector4& a_rvStart, const TVector4& a_rvStartTangent, const TVector4& a_rvEnd, const TVector4& a_rvEndTangent, TFLOAT a_fT )
{
	TVector4 vPowers( a_fT * a_fT * a_fT, a_fT * a_fT, a_fT, 1.0f );
	TVector4 vBasis;
	TMatrix44::TransformVector( vBasis, s_mHermite, vPowers );

	TVector4 vTerm;
	a_rvOut.Multiply( a_rvStart, vBasis.x );
	vTerm.Multiply( a_rvEnd, vBasis.y );
	a_rvOut.Add( vTerm );
	vTerm.Multiply( a_rvStartTangent, vBasis.z );
	a_rvOut.Add( vTerm );
	vTerm.Multiply( a_rvEndTangent, vBasis.w );
	a_rvOut.Add( vTerm );
}

// The tangent at a_fT of the Hermite curve from a_rvStart to a_rvEnd
// $deBlob: FUNCTION 0066e450
void TMath::HermiteTangent( TVector4& a_rvOut, const TVector4& a_rvStart, const TVector4& a_rvStartTangent, const TVector4& a_rvEnd, const TVector4& a_rvEndTangent, TFLOAT a_fT )
{
	TVector4 vPowers( a_fT * a_fT * a_fT, a_fT * a_fT, a_fT, 1.0f );
	TVector4 vBasis;
	TMatrix44::TransformVector( vBasis, s_mHermiteTangent, vPowers );

	TVector4 vTerm;
	a_rvOut.Multiply( a_rvStart, vBasis.x );
	vTerm.Multiply( a_rvEnd, vBasis.y );
	a_rvOut.Add( vTerm );
	vTerm.Multiply( a_rvStartTangent, vBasis.z );
	a_rvOut.Add( vTerm );
	vTerm.Multiply( a_rvEndTangent, vBasis.w );
	a_rvOut.Add( vTerm );
}

// $deBlob: FUNCTION 0066ec90
// $Barnyard: FUNCTION 006b7a40
TBOOL TMath::IsPointInTriangle2D( const TVector2& a_rcVertex1, const TVector2& a_rcVertex2, const TVector2& a_rcVertex3, const TVector2& a_rcPoint )
{
	TFLOAT fEdge1X = a_rcVertex2.x - a_rcVertex1.x;
	TFLOAT fEdge1Y = a_rcVertex2.y - a_rcVertex1.y;
	TFLOAT fEdge2X = a_rcVertex3.x - a_rcVertex1.x;
	TFLOAT fEdge2Y = a_rcVertex3.y - a_rcVertex1.y;
	TFLOAT fPointX = a_rcPoint.x - a_rcVertex1.x;
	TFLOAT fPointY = a_rcPoint.y - a_rcVertex1.y;

	TFLOAT fDot11 = fEdge1X * fEdge1X + fEdge1Y * fEdge1Y;
	TFLOAT fDot12 = fEdge2X * fEdge1X + fEdge1Y * fEdge2Y;
	TFLOAT fDot22 = fEdge2X * fEdge2X + fEdge2Y * fEdge2Y;
	TFLOAT fDot1P = fPointX * fEdge1X + fPointY * fEdge1Y;
	TFLOAT fDot2P = fPointX * fEdge2X + fPointY * fEdge2Y;

	TFLOAT fInvDenom = 1.0f / ( fDot12 * fDot12 - fDot22 * fDot11 );
	TFLOAT fU        = ( fDot2P * fDot12 - fDot1P * fDot22 ) * fInvDenom;

	// NOTE: written as negated comparisons because the original's x87 compares let NaN through
	// (a degenerate triangle gives NaN and counts as containing the point)
	if ( !( fU < 0.0f ) && !( fU > 1.0f ) )
	{
		TFLOAT fV = ( fDot1P * fDot12 - fDot2P * fDot11 ) * fInvDenom;

		if ( !( fV < 0.0f ) && !( fU + fV > 1.0f ) )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0066b8b0
// $Barnyard: FUNCTION 006b8bc0
TFLOAT TMath::DistancePointToSegmentSq( const TVector4& a_rcPoint, const TVector3& a_rcStart, const TVector3& a_rcEnd )
{
	TVector3 vecSegment = a_rcEnd - a_rcStart;

	TFLOAT fT = ( vecSegment.z * ( a_rcPoint.z - a_rcStart.z ) + vecSegment.x * ( a_rcPoint.x - a_rcStart.x ) + vecSegment.y * ( a_rcPoint.y - a_rcStart.y ) ) /
	    ( vecSegment.z * vecSegment.z + vecSegment.y * vecSegment.y + vecSegment.x * vecSegment.x );

	if ( fT > 1.0f )
		fT = 1.0f;
	else if ( fT < 0.0f )
		fT = 0.0f;

	TFLOAT fDX = vecSegment.x * fT + a_rcStart.x - a_rcPoint.x;
	TFLOAT fDY = vecSegment.y * fT + a_rcStart.y - a_rcPoint.y;
	TFLOAT fDZ = vecSegment.z * fT + a_rcStart.z - a_rcPoint.z;

	return fDZ * fDZ + fDY * fDY + fDX * fDX;
}

// $deBlob: FUNCTION 0066b9f0
// $Barnyard: FUNCTION 006b8d00
TFLOAT TMath::DistancePointToSegmentSq( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcEnd, TFLOAT& a_rfT )
{
	TVector4 vecStartToPoint = a_rcPoint;
	vecStartToPoint.Substract( a_rcStart );

	TVector4 vecSegment = a_rcEnd;
	vecSegment.Substract( a_rcStart );

	a_rfT = TVector4::DotProduct3( vecStartToPoint, vecSegment ) / vecSegment.MagnitudeSq();

	if ( a_rfT > 1.0f )
		a_rfT = 1.0f;
	else if ( a_rfT < 0.0f )
		a_rfT = 0.0f;

	TVector4 vecClosest = vecSegment;
	vecClosest.Multiply( a_rfT );
	vecClosest.Add( a_rcStart );
	vecClosest.Substract( a_rcPoint );

	return vecClosest.MagnitudeSq();
}

// $deBlob: FUNCTION 0066b1e0
// $Barnyard: FUNCTION 006b6470
TFLOAT TMath::ClosestPointOnLine2D( const TVector2& a_rcLineStart, const TVector2& a_rcLineEnd, const TVector2& a_rcPoint )
{
	TFLOAT fDirX = a_rcLineEnd.x - a_rcLineStart.x;
	TFLOAT fDirY = a_rcLineEnd.y - a_rcLineStart.y;

	TASSERT( fDirY * fDirY + fDirX * fDirX > TFLOAT_EPSILON );

	return ( ( a_rcPoint.x - a_rcLineStart.x ) * fDirX + fDirY * ( a_rcPoint.y - a_rcLineStart.y ) ) / ( fDirX * fDirX + fDirY * fDirY );
}

// $deBlob: FUNCTION 0066a730
// $Barnyard: FUNCTION 006b6b20
TBOOL TMath::CircleVsSegment2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fRadius, const TVector2& a_rcCenter, TFLOAT* a_pOutPenetration )
{
	TFLOAT fT = ClosestPointOnLine2D( a_rcStart, a_rcEnd, a_rcCenter );

	if ( fT > 1.0f )
		fT = 1.0f;
	else if ( fT < 0.0f )
		fT = 0.0f;

	TVector2 vecClosest;
	vecClosest.Lerp( a_rcStart, a_rcEnd, fT );

	TFLOAT fDistanceSq = ( vecClosest.x - a_rcCenter.x ) * ( vecClosest.x - a_rcCenter.x ) + ( vecClosest.y - a_rcCenter.y ) * ( vecClosest.y - a_rcCenter.y );

	if ( a_fRadius * a_fRadius < fDistanceSq )
		return TFALSE;

	if ( a_pOutPenetration )
		*a_pOutPenetration = a_fRadius - sqrtf( fDistanceSq );

	return TTRUE;
}

// $deBlob: FUNCTION 00669940
TINT TMath::RayVsCircle2D( const TVector2& a_rcOrigin, const TVector2& a_rcDirection, const TVector2& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfNearIntersection, TFLOAT& a_rfFarIntersection )
{
	TFLOAT fRadiusSq  = a_fRadius * a_fRadius;
	TFLOAT fToCenterX = a_rcCenter.x - a_rcOrigin.x;
	TFLOAT fToCenterY = a_rcCenter.y - a_rcOrigin.y;
	TFLOAT fDistSq    = fToCenterY * fToCenterY + fToCenterX * fToCenterX;

	if ( fDistSq >= fRadiusSq )
	{
		// The origin is outside of the circle: the circle has to be in front of the ray and close enough to it
		TFLOAT fDot = a_rcDirection.y * fToCenterY + a_rcDirection.x * fToCenterX;

		if ( fDot < 0.0f || fDistSq - fDot * fDot > fRadiusSq )
			return 0;
	}

	TFLOAT fB    = ( a_rcDirection.y * ( fToCenterY * -1.0f ) + a_rcDirection.x * ( fToCenterX * -1.0f ) ) * 2.0f;
	TFLOAT fDisc = fB * fB - ( fDistSq - fRadiusSq ) * 4.0f;

	if ( fDisc < 0.0f )
		return 0;

	TFLOAT fSqrt         = sqrtf( fDisc );
	a_rfNearIntersection = ( -fB - fSqrt ) * 0.5f;
	a_rfFarIntersection  = ( fSqrt - fB ) * 0.5f;
	return 2;
}

// $deBlob: FUNCTION 0066a030
TBOOL TMath::RayVsCapsule2D( const TVector2& a_rcOrigin, const TVector2& a_rcDirection, TFLOAT& a_rfDistance, const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fRadius )
{
	TVector2 vecRayEnd( a_rcOrigin.x + a_rcDirection.x * a_rfDistance, a_rcDirection.y * a_rfDistance + a_rcOrigin.y );
	TFLOAT   fAxisX = a_rcEnd.x - a_rcStart.x;
	TFLOAT   fAxisY = a_rcEnd.y - a_rcStart.y;

	// Region of the ray's start and end along the capsule's axis: 0: between the caps, 1: before the start, 2: after the end
	TINT iOriginRegion;
	if ( ( a_rcOrigin.y - a_rcEnd.y ) * fAxisY + ( a_rcOrigin.x - a_rcEnd.x ) * fAxisX < 0.0f )
		iOriginRegion = ( ( a_rcOrigin.y - a_rcStart.y ) * fAxisY + ( a_rcOrigin.x - a_rcStart.x ) * fAxisX < 0.0f );
	else
		iOriginRegion = 2;

	TINT iEndRegion;
	if ( ( vecRayEnd.y - a_rcEnd.y ) * fAxisY + ( vecRayEnd.x - a_rcEnd.x ) * fAxisX < 0.0f )
		iEndRegion = ( ( vecRayEnd.y - a_rcStart.y ) * fAxisY + ( vecRayEnd.x - a_rcStart.x ) * fAxisX < 0.0f );
	else
		iEndRegion = 2;

	TBOOL abRegions[ 3 ]       = { TFALSE, TFALSE, TFALSE };
	abRegions[ iOriginRegion ] = TTRUE;
	abRegions[ iEndRegion ]    = TTRUE;

	TBOOL bHit = TFALSE;

	if ( ( abRegions[ 1 ] && abRegions[ 2 ] ) || abRegions[ 0 ] )
	{
		// The sides of the capsule
		TVector2 vecAxis( a_rcEnd.x - a_rcStart.x, a_rcEnd.y - a_rcStart.y );
		vecAxis.Normalize();

		TFLOAT fOffsetX = -vecAxis.y * a_fRadius;
		TFLOAT fOffsetY = vecAxis.x * a_fRadius;

		TFLOAT fRayX = vecRayEnd.x - a_rcOrigin.x;
		TFLOAT fRayY = vecRayEnd.y - a_rcOrigin.y;

		TBOOL  bSideHit = TFALSE;
		TFLOAT fBestT   = 2.0f;

		// Side 1: offset by +perpendicular
		TFLOAT fSide1StartX = a_rcStart.x + fOffsetX;
		TFLOAT fSide1StartY = a_rcStart.y + fOffsetY;
		TFLOAT fSide1X      = ( a_rcEnd.x + fOffsetX ) - fSide1StartX;
		TFLOAT fSide1Y      = ( a_rcEnd.y + fOffsetY ) - fSide1StartY;
		TFLOAT fDenom1      = fRayX * fSide1Y - fRayY * fSide1X;

		if ( fDenom1 != 0.0f )
		{
			TFLOAT fDX = a_rcOrigin.x - fSide1StartX;
			TFLOAT fDY = a_rcOrigin.y - fSide1StartY;
			TFLOAT fU  = ( fDY * fRayX - fDX * fRayY ) / fDenom1;
			TFLOAT fT  = ( fDY * fSide1X - fDX * fSide1Y ) / fDenom1;

			if ( fU >= 0.0f && fU <= 1.0f && fT >= 0.0f && fT < 2.0f )
			{
				bSideHit = TTRUE;
				fBestT   = fT;
			}
		}

		// Side 2: offset by -perpendicular
		TFLOAT fSide2StartX = a_rcStart.x - fOffsetX;
		TFLOAT fSide2StartY = a_rcStart.y - fOffsetY;
		TFLOAT fSide2X      = ( a_rcEnd.x - fOffsetX ) - fSide2StartX;
		TFLOAT fSide2Y      = ( a_rcEnd.y - fOffsetY ) - fSide2StartY;
		TFLOAT fDenom2      = fRayX * fSide2Y - fRayY * fSide2X;

		if ( fDenom2 != 0.0f )
		{
			TFLOAT fDX = a_rcOrigin.x - fSide2StartX;
			TFLOAT fDY = a_rcOrigin.y - fSide2StartY;
			TFLOAT fU  = ( fDY * fRayX - fDX * fRayY ) / fDenom2;
			TFLOAT fT  = ( fDY * fSide2X - fDX * fSide2Y ) / fDenom2;

			if ( fU >= 0.0f && fU <= 1.0f && fT >= 0.0f && fBestT > fT )
			{
				bSideHit = TTRUE;
				fBestT   = fT;
			}
		}

		if ( bSideHit )
		{
			bHit         = TTRUE;
			a_rfDistance = TVector2( fRayX, fRayY ).Magnitude() * fBestT;
		}
	}

	// The caps
	TFLOAT fNear;
	TFLOAT fFar;

	if ( abRegions[ 1 ] && RayVsCircle2D( a_rcOrigin, a_rcDirection, a_rcStart, a_fRadius, fNear, fFar ) > 0 && a_rfDistance > fNear && fNear >= 0.0f )
	{
		a_rfDistance = fNear;
		bHit         = TTRUE;
	}

	if ( abRegions[ 2 ] && RayVsCircle2D( a_rcOrigin, a_rcDirection, a_rcEnd, a_fRadius, fNear, fFar ) > 0 && a_rfDistance > fNear && fNear >= 0.0f )
	{
		a_rfDistance = fNear;
		return TTRUE;
	}

	return bHit;
}

// $deBlob: FUNCTION 0066ac00
// $Barnyard: FUNCTION 006b61a0
TBOOL TMath::LineVsLine2D( const TFLOAT* a_pLine1Start, const TFLOAT* a_pLine1End, const TFLOAT* a_pLine2Start, const TFLOAT* a_pLine2End, TFLOAT& a_rfT )
{
	TFLOAT fLine2X = a_pLine2End[ 0 ] - a_pLine2Start[ 0 ];
	TFLOAT fLine2Y = a_pLine2End[ 1 ] - a_pLine2Start[ 1 ];
	TFLOAT fDenom  = ( a_pLine1End[ 0 ] - a_pLine1Start[ 0 ] ) * fLine2Y - ( a_pLine1End[ 1 ] - a_pLine1Start[ 1 ] ) * fLine2X;

	if ( fDenom == 0.0f )
		return TFALSE;

	a_rfT = ( ( a_pLine1Start[ 1 ] - a_pLine2Start[ 1 ] ) * fLine2X - ( a_pLine1Start[ 0 ] - a_pLine2Start[ 0 ] ) * fLine2Y ) / fDenom;
	return TTRUE;
}

// $deBlob: FUNCTION 0066acc0
// $Barnyard: FUNCTION 006b6250
TBOOL TMath::LineVsLine2D( const TVector2& a_rcLine1Start, const TVector2& a_rcLine1End, const TVector2& a_rcLine2Start, const TVector2& a_rcLine2End, TFLOAT& a_rfT1, TFLOAT& a_rfT2 )
{
	TFLOAT fLine1X = a_rcLine1End.x - a_rcLine1Start.x;
	TFLOAT fLine1Y = a_rcLine1End.y - a_rcLine1Start.y;
	TFLOAT fLine2X = a_rcLine2End.x - a_rcLine2Start.x;
	TFLOAT fLine2Y = a_rcLine2End.y - a_rcLine2Start.y;
	TFLOAT fDenom  = fLine1X * fLine2Y - fLine1Y * fLine2X;

	if ( fDenom == 0.0f )
		return TFALSE;

	TFLOAT fDiffX = a_rcLine1Start.x - a_rcLine2Start.x;
	TFLOAT fDiffY = a_rcLine1Start.y - a_rcLine2Start.y;

	// NOTE: de Blob divides by the denominator (Barnyard multiplies by its inverse)
	a_rfT1 = ( fDiffY * fLine2X - fDiffX * fLine2Y ) / fDenom;
	a_rfT2 = ( fDiffY * fLine1X - fDiffX * fLine1Y ) / fDenom;
	return TTRUE;
}

// NOTE: de Blob inlines LineVsLine2D and divides by the denominator (Barnyard multiplies by its inverse)
// $deBlob: FUNCTION 0066af90
// $Barnyard: FUNCTION 006b6330
TBOOL TMath::LineSegmentVsLineSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2, TVector2& a_rOutPoint )
{
	TFLOAT fDX1   = a_rcEnd1.x - a_rcStart1.x;
	TFLOAT fDY1   = a_rcEnd1.y - a_rcStart1.y;
	TFLOAT fDX2   = a_rcEnd2.x - a_rcStart2.x;
	TFLOAT fDY2   = a_rcEnd2.y - a_rcStart2.y;
	TFLOAT fDenom = fDX1 * fDY2 - fDY1 * fDX2;

	if ( fDenom == 0.0f )
		return TFALSE;

	TFLOAT fOX = a_rcStart1.x - a_rcStart2.x;
	TFLOAT fOY = a_rcStart1.y - a_rcStart2.y;
	TFLOAT fT2 = ( fOY * fDX1 - fOX * fDY1 ) / fDenom;
	TFLOAT fT1 = ( fOY * fDX2 - fOX * fDY2 ) / fDenom;

	// NOTE: written as retail tests it (a NaN is no intersection)
	if ( !( fT1 >= 0.0f && fT2 >= 0.0f && fT1 <= 1.0f && fT2 <= 1.0f ) )
		return TFALSE;

	a_rOutPoint.x = a_rcStart1.x + ( a_rcEnd1.x - a_rcStart1.x ) * fT1;
	a_rOutPoint.y = ( a_rcEnd1.y - a_rcStart1.y ) * fT1 + a_rcStart1.y;
	return TTRUE;
}

// NOTE: de Blob inlines LineVsLine2D and divides by the denominator (Barnyard multiplies by its inverse)
// $deBlob: FUNCTION 0066aea0
// $Barnyard: FUNCTION 006b63f0
TBOOL TMath::LineSegmentVsLineSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2 )
{
	TFLOAT fDX1   = a_rcEnd1.x - a_rcStart1.x;
	TFLOAT fDY1   = a_rcEnd1.y - a_rcStart1.y;
	TFLOAT fDX2   = a_rcEnd2.x - a_rcStart2.x;
	TFLOAT fDY2   = a_rcEnd2.y - a_rcStart2.y;
	TFLOAT fDenom = fDX1 * fDY2 - fDY1 * fDX2;

	if ( fDenom != 0.0f )
	{
		TFLOAT fOX = a_rcStart1.x - a_rcStart2.x;
		TFLOAT fOY = a_rcStart1.y - a_rcStart2.y;
		TFLOAT fT2 = ( fOY * fDX1 - fOX * fDY1 ) / fDenom;
		TFLOAT fT1 = ( fOY * fDX2 - fOX * fDY2 ) / fDenom;

		if ( fT1 >= 0.0f && fT2 >= 0.0f && fT1 <= 1.0f && fT2 <= 1.0f )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006b8e50
TFLOAT TMath::DistanceAlongSegment( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcEnd )
{
	TVector4 vecToPoint;
	vecToPoint.Substract( a_rcPoint, a_rcStart );

	TVector4 vecSegment;
	vecSegment.Substract( a_rcEnd, a_rcStart );

	TFLOAT fLength   = vecSegment.Magnitude();
	TFLOAT fDistance = TVector4::DotProduct3( vecToPoint, vecSegment ) / fLength;

	if ( fDistance > fLength )
		return fLength;

	return ( fDistance >= 0.0f ) ? fDistance : 0.0f;
}

// $deBlob: FUNCTION 0066bce0
// $Barnyard: FUNCTION 006b6ca0
TFLOAT TMath::DistanceSegmentToSegmentSq( const TVector3& a_rcStart1, const TVector3& a_rcEnd1, const TVector3& a_rcStart2, const TVector3& a_rcEnd2 )
{
	TVector3 vecDir1( a_rcEnd1.x - a_rcStart1.x, a_rcEnd1.y - a_rcStart1.y, a_rcEnd1.z - a_rcStart1.z );
	TVector3 vecDir2( a_rcEnd2.x - a_rcStart2.x, a_rcEnd2.y - a_rcStart2.y, a_rcEnd2.z - a_rcStart2.z );
	TVector3 vecDiff( a_rcStart1.x - a_rcStart2.x, a_rcStart1.y - a_rcStart2.y, a_rcStart1.z - a_rcStart2.z );

	TFLOAT fA = vecDir1.x * vecDir1.x + vecDir1.z * vecDir1.z + vecDir1.y * vecDir1.y;
	TFLOAT fB = vecDir2.x * vecDir1.x + vecDir2.z * vecDir1.z + vecDir2.y * vecDir1.y;
	TFLOAT fC = vecDir2.y * vecDir2.y + vecDir2.z * vecDir2.z + vecDir2.x * vecDir2.x;
	TFLOAT fD = vecDiff.x * vecDir1.x + vecDiff.z * vecDir1.z + vecDiff.y * vecDir1.y;
	TFLOAT fE = vecDiff.y * vecDir2.y + vecDiff.z * vecDir2.z + vecDiff.x * vecDir2.x;

	TFLOAT fDenom  = fC * fA - fB * fB;
	TFLOAT fSDenom = fDenom;
	TFLOAT fTDenom = fDenom;
	TFLOAT fSNumer;
	TFLOAT fTNumer;

	if ( fDenom < 1e-9f )
	{
		// The segments are parallel
		fSNumer = 0.0f;
		fSDenom = 1.0f;
		fTNumer = fE;
		fTDenom = fC;
	}
	else
	{
		fSNumer = fE * fB - fD * fC;
		fTNumer = fE * fA - fD * fB;

		if ( fSNumer < 0.0f )
		{
			fSNumer = 0.0f;
			fTNumer = fE;
			fTDenom = fC;
		}
		else if ( fSNumer > fDenom )
		{
			fSNumer = fDenom;
			fTNumer = fE + fB;
			fTDenom = fC;
		}
	}

	if ( fTNumer < 0.0f )
	{
		fTNumer = 0.0f;
		fSNumer = -fD;

		if ( fSNumer < 0.0f )
			fSNumer = 0.0f;
		else if ( fSNumer > fA )
			fSNumer = fSDenom;
		else
			fSDenom = fA;
	}
	else if ( fTNumer > fTDenom )
	{
		fTNumer = fTDenom;
		fSNumer = fB - fD;

		if ( fSNumer < 0.0f )
			fSNumer = 0.0f;
		else if ( fSNumer > fA )
			fSNumer = fSDenom;
		else
			fSDenom = fA;
	}

	TFLOAT fS = ( TMath::Abs( fSNumer ) >= 1e-9f ) ? fSNumer / fSDenom : 0.0f;
	TFLOAT fT = ( TMath::Abs( fTNumer ) >= 1e-9f ) ? fTNumer / fTDenom : 0.0f;

	TFLOAT fX = ( vecDir1.x * fS + vecDiff.x ) - vecDir2.x * fT;
	TFLOAT fY = ( vecDir1.y * fS + vecDiff.y ) - vecDir2.y * fT;
	TFLOAT fZ = ( vecDiff.z + vecDir1.z * fS ) - vecDir2.z * fT;

	return fZ * fZ + fY * fY + fX * fX;
}

// $deBlob: FUNCTION 0066bc00
TFLOAT TMath::DistanceSegmentToSegment( const TVector3& a_rcStart1, const TVector3& a_rcEnd1, const TVector3& a_rcStart2, const TVector3& a_rcEnd2 )
{
	return TMath::Sqrt( DistanceSegmentToSegmentSq( a_rcStart1, a_rcEnd1, a_rcStart2, a_rcEnd2 ) );
}

// $deBlob: FUNCTION 0066bc40
// $Barnyard: FUNCTION 006b9170
TFLOAT TMath::DistanceSegmentToSegmentSq2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2 )
{
	return DistanceSegmentToSegmentSq(
	    TVector3( a_rcStart1.x, a_rcStart1.y, 0.0f ),
	    TVector3( a_rcEnd1.x, a_rcEnd1.y, 0.0f ),
	    TVector3( a_rcStart2.x, a_rcStart2.y, 0.0f ),
	    TVector3( a_rcEnd2.x, a_rcEnd2.y, 0.0f )
	);
}

// $deBlob: FUNCTION 0066bb40
// $Barnyard: FUNCTION 006b9200
TFLOAT TMath::DistanceSegmentToSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2 )
{
	return TMath::Sqrt( DistanceSegmentToSegmentSq(
	    TVector3( a_rcStart1.x, a_rcStart1.y, 0.0f ),
	    TVector3( a_rcEnd1.x, a_rcEnd1.y, 0.0f ),
	    TVector3( a_rcStart2.x, a_rcStart2.y, 0.0f ),
	    TVector3( a_rcEnd2.x, a_rcEnd2.y, 0.0f )
	) );
}

// $deBlob: FUNCTION 0066a530
// $Barnyard: FUNCTION 006b9290
TBOOL TMath::IsSegmentNearSegment2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2, TFLOAT a_fDistance )
{
	return a_fDistance * a_fDistance >= DistanceSegmentToSegmentSq2D( a_rcStart1, a_rcEnd1, a_rcStart2, a_rcEnd2 );
}

// $deBlob: FUNCTION 0066a7f0
// $Barnyard: FUNCTION 006b6be0
TBOOL TMath::CircleVsCapsule2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fCapsuleRadius, const TVector2& a_rcCenter, TFLOAT a_fRadius, TFLOAT* a_pOutPenetration )
{
	TFLOAT fT = ClosestPointOnLine2D( a_rcStart, a_rcEnd, a_rcCenter );

	if ( fT > 1.0f )
		fT = 1.0f;
	else if ( fT < 0.0f )
		fT = 0.0f;

	TVector2 vecClosest;
	vecClosest.Lerp( a_rcStart, a_rcEnd, fT );

	TFLOAT fDistanceSq = ( vecClosest.x - a_rcCenter.x ) * ( vecClosest.x - a_rcCenter.x ) + ( vecClosest.y - a_rcCenter.y ) * ( vecClosest.y - a_rcCenter.y );
	TFLOAT fRadius     = a_fCapsuleRadius + a_fRadius;

	if ( fRadius * fRadius < fDistanceSq )
		return TFALSE;

	if ( a_pOutPenetration )
		*a_pOutPenetration = fRadius - sqrtf( fDistanceSq );

	return TTRUE;
}

// $deBlob: FUNCTION 00831340
// $Barnyard: FUNCTION 006d84d0
TBOOL TMath::SphereVsSphere( const TVector4& a_rcSphere1, const TVector4& a_rcSphere2 )
{
	TFLOAT fDX     = a_rcSphere1.x - a_rcSphere2.x;
	TFLOAT fDY     = a_rcSphere1.y - a_rcSphere2.y;
	TFLOAT fDZ     = a_rcSphere1.z - a_rcSphere2.z;
	TFLOAT fRadius = a_rcSphere1.w + a_rcSphere2.w;

	return fRadius * fRadius > fDX * fDX + fDZ * fDZ + fDY * fDY;
}

// $deBlob: FUNCTION 00667d00
// $Barnyard: FUNCTION 006b5d30
TBOOL TMath::SphereVsSphere( const TVector4& a_rcCenter1, TFLOAT a_fRadius1, const TVector4& a_rcCenter2, TFLOAT a_fRadius2 )
{
	TFLOAT fRadius = a_fRadius1 + a_fRadius2;
	return fRadius * fRadius >= TVector4::DistanceSq( a_rcCenter1, a_rcCenter2 );
}

// NOTE: de Blob's asserts name the arguments a_rvSweptSphereCentre, a_fSweptSphereRadius, a_rvSphereCentre and
// a_fSphereRadius; unused in de Blob
// $deBlob: FUNCTION 0066fdc0
// $Barnyard: FUNCTION 006b89c0
TBOOL TMath::SweptSphereVsSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector4& a_rcOtherCenter, TFLOAT a_fOtherRadius, TFLOAT& a_rfDistance, TVector4& a_rOutNormal, TFLOAT a_fEpsilon )
{
	TASSERT( !a_rcCenter.IsEqual( a_rcOtherCenter ) );
	TASSERT( TFALSE == TMath::SphereVsSphere( a_rcCenter, a_fRadius - 0.01f, a_rcOtherCenter, a_fOtherRadius - 0.01f ) );

	TFLOAT fMaxDistance = a_rfDistance;

	TVector4 vecMove;
	vecMove.Multiply( a_rcDirection, fMaxDistance );

	TVector4 vecEnd( a_rcCenter.x + vecMove.x, a_rcCenter.y + vecMove.y, a_rcCenter.z + vecMove.z, a_rcCenter.w );

	TFLOAT fT;
	TFLOAT fDistanceSq = DistancePointToLineSq( a_rcOtherCenter, a_rcCenter, vecEnd, fT );

	if ( fT < 0.0f )
	{
		// Moving away from the other sphere
		a_rfDistance = fMaxDistance;
		TASSERT( TFALSE == TMath::SphereVsSphere( vecEnd, a_fRadius - 0.01f, a_rcOtherCenter, a_fOtherRadius - 0.01f ) );
		return TFALSE;
	}

	TFLOAT fRadiusSq = ( a_fRadius + a_fOtherRadius ) * ( a_fRadius + a_fOtherRadius );

	// NOTE: written as !( < ) so that a NaN distance (a_rfDistance of 0, the line has no length) returns
	// TFALSE like the x87 compare of the original does
	if ( !( fDistanceSq - fRadiusSq < 0.0f ) )
		return TFALSE;

	TFLOAT fDistance = fT * fMaxDistance - ( TMath::Sqrt( fRadiusSq - fDistanceSq ) + a_fEpsilon );
	a_rfDistance     = fDistance;

	if ( fDistance < 0.01f )
	{
		a_rfDistance = 0.0f;
		return TTRUE;
	}

	if ( fDistance > fMaxDistance || -a_fRadius > fDistance )
	{
		a_rfDistance = fMaxDistance;
		return TFALSE;
	}

	TVector4 vecContactMove;
	vecContactMove.Multiply( a_rcDirection, fDistance );

	TVector4 vecContact( a_rcCenter.x + vecContactMove.x, a_rcCenter.y + vecContactMove.y, a_rcCenter.z + vecContactMove.z, a_rcCenter.w );

	a_rOutNormal.Set( vecContact.x - a_rcOtherCenter.x, vecContact.y - a_rcOtherCenter.y, vecContact.z - a_rcOtherCenter.z, vecContact.w );
	a_rOutNormal.Normalise();

	TASSERT( TFALSE == TMath::SphereVsSphere( vecContact, a_fRadius - 0.01f, a_rcOtherCenter, a_fOtherRadius - 0.01f ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 006b5f30
TINT TMath::RayVsSphere( const TVector3& a_rcOrigin, const TVector3& a_rcDirection, const TVector3& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfNear, TFLOAT& a_rfFar )
{
	TVector3 vecToCenter( a_rcCenter.x - a_rcOrigin.x, a_rcCenter.y - a_rcOrigin.y, a_rcCenter.z - a_rcOrigin.z );

	TFLOAT fRadiusSq   = a_fRadius * a_fRadius;
	TFLOAT fDistanceSq = vecToCenter.x * vecToCenter.x + vecToCenter.z * vecToCenter.z + vecToCenter.y * vecToCenter.y;

	if ( fDistanceSq >= fRadiusSq )
	{
		// Outside: the sphere must be in front of the ray and close enough to it
		TFLOAT fProjection = vecToCenter.z * a_rcDirection.z + vecToCenter.y * a_rcDirection.y + vecToCenter.x * a_rcDirection.x;

		if ( fProjection < 0.0f )
			return 0;

		if ( fDistanceSq - fProjection * fProjection > fRadiusSq )
			return 0;
	}

	TFLOAT fB = ( vecToCenter.z * -1.0f * a_rcDirection.z + vecToCenter.y * -1.0f * a_rcDirection.y + vecToCenter.x * -1.0f * a_rcDirection.x );
	fB        = fB + fB;

	TFLOAT fDiscriminant = fB * fB - ( fDistanceSq - fRadiusSq ) * 4.0f;

	if ( fDiscriminant < 0.0f )
		return 0;

	TFLOAT fRoot = TMath::Sqrt( fDiscriminant );
	a_rfNear     = ( -fB - fRoot ) * 0.5f;
	a_rfFar      = ( fRoot - fB ) * 0.5f;

	return 2;
}

// $deBlob: FUNCTION 00667d40
// $Barnyard: FUNCTION 006b5d70
TBOOL TMath::SweptSphereVsPlane( const TVector4& a_rcCenter, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4 a_vPlanePoint, TVector4 a_vPlaneNormal )
{
	TVector4 vecToCenter( a_rcCenter.x - a_vPlanePoint.x, a_rcCenter.y - a_vPlanePoint.y, a_rcCenter.z - a_vPlanePoint.z, a_rcCenter.w );

	// Make the normal face the sphere
	if ( TVector4::DotProduct3( a_vPlaneNormal, vecToCenter ) < 0.0f )
	{
		a_vPlaneNormal.x = -a_vPlaneNormal.x;
		a_vPlaneNormal.y = -a_vPlaneNormal.y;
		a_vPlaneNormal.z = -a_vPlaneNormal.z;
	}

	TFLOAT fDot = TVector4::DotProduct3( a_rcDirection, a_vPlaneNormal );

	if ( fDot == 0.0f )
		return TFALSE;

	// The plane moved towards the sphere by its radius, relative to the centre
	TVector4 vecOffset;
	vecOffset.Multiply( a_vPlaneNormal, a_fRadius );

	a_vPlanePoint.x = vecOffset.x + a_vPlanePoint.x - a_rcCenter.x;
	a_vPlanePoint.y = vecOffset.y + a_vPlanePoint.y - a_rcCenter.y;
	a_vPlanePoint.z = vecOffset.z + a_vPlanePoint.z - a_rcCenter.z;

	TFLOAT fDistance = TVector4::DotProduct3( a_vPlaneNormal, a_vPlanePoint ) / fDot;

	if ( fDistance < 0.0f || fDistance > a_rfDistance )
		return TFALSE;

	a_rfDistance = ( fDistance <= 0.1f ) ? 0.0f : fDistance - 0.1f;
	return TTRUE;
}

// $Barnyard: FUNCTION 006b92d0
TBOOL TMath::IsPointInCapsule( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT a_fLength )
{
	TFLOAT fDistanceSq;

	if ( a_fLength > 0.0f )
	{
		TVector4 vecEnd;
		vecEnd.Multiply( a_rcDirection, a_fLength );
		vecEnd.Add( a_rcStart );
		vecEnd.w = a_rcStart.w;

		fDistanceSq = DistancePointToSegmentSq( a_rcPoint, a_rcStart, vecEnd, a_fLength );
	}
	else
	{
		fDistanceSq = TVector4::DistanceSq( a_rcPoint, a_rcStart );
	}

	return a_fRadius * a_fRadius >= fDistanceSq;
}

// $deBlob: FUNCTION 006712b0
// $Barnyard: FUNCTION 006ba210
TBOOL TMath::SphereVsPolygon( const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4& a_rNormal, const TVector3* const* a_ppVertices, const TPlane& a_rcPlane, TINT a_iNumVertices, TFLOAT a_fEpsilon )
{
	const TVector3& rcNormal = a_rcPlane.GetNormal();

	TFLOAT fPlaneDist    = rcNormal.x * a_rcCenter.x + rcNormal.z * a_rcCenter.z + rcNormal.y * a_rcCenter.y - a_rcPlane.GetD();
	TFLOAT fAbsPlaneDist = TMath::Abs( fPlaneDist );

	if ( a_fRadius > a_rfDistance )
		a_fRadius = a_rfDistance;

	if ( fAbsPlaneDist >= a_fRadius - a_fEpsilon )
		return TFALSE;

	// NOTE: the point is moved along the normal (not projected back on the plane)
	TVector4 vecPoint( fPlaneDist * rcNormal.x + a_rcCenter.x, fPlaneDist * rcNormal.y + a_rcCenter.y, fPlaneDist * rcNormal.z + a_rcCenter.z, 1.0f );

	if ( IsPointInPolygon( a_iNumVertices, a_ppVertices, vecPoint ) )
	{
		a_rfDistance = fAbsPlaneDist;
		a_rNormal.Set( rcNormal.x, rcNormal.y, rcNormal.z, 1.0f );
		return TTRUE;
	}

	// Find the closest edge
	TINT   iClosestEdge = -1;
	TFLOAT fClosestSq   = a_fRadius * a_fRadius;

	for ( TINT i = a_iNumVertices - 1, k = a_iNumVertices; i >= 0; i--, k-- )
	{
		TFLOAT fDistanceSq = DistancePointToSegmentSq( a_rcCenter, *a_ppVertices[ i ], *a_ppVertices[ k % a_iNumVertices ] );

		if ( fClosestSq - a_fEpsilon > fDistanceSq )
		{
			fClosestSq   = fDistanceSq;
			iClosestEdge = i;
		}
	}

	if ( iClosestEdge == -1 )
		return TFALSE;

	a_rfDistance = sqrtf( fClosestSq );

	// NOTE: the normal points from the first vertex of the edge, not from the closest point
	const TVector3* pVertex = a_ppVertices[ iClosestEdge ];
	a_rNormal.Set( a_rcCenter.x - pVertex->x, a_rcCenter.y - pVertex->y, a_rcCenter.z - pVertex->z, 1.0f );
	a_rNormal.Normalise();

	return TTRUE;
}

// $deBlob: FUNCTION 0066efc0
// $Barnyard: FUNCTION 006b7f30
TINT TMath::SweptSphereVsTriangle( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance, const TPlane& a_rcPlane, TVector4& a_rNormal )
{
	TCONSTEXPR_VAR TFLOAT EPSILON     = 0.001f;
	static TFLOAT         s_fPushBack = EPSILON * 4.0f;

	const TVector3& rcPlaneNormal = a_rcPlane.GetNormal();
	TVector4        vecDirection  = a_rcDirection;

	TASSERT( a_fRadius >= 0 );

	TFLOAT fDirDot = vecDirection.z * rcPlaneNormal.z + vecDirection.x * rcPlaneNormal.x + vecDirection.y * rcPlaneNormal.y;

	// Moving away from the plane
	if ( fDirDot >= 0.1f )
		return 0;

	TFLOAT fPlaneDist = a_rcCenter.y * rcPlaneNormal.y + a_rcCenter.x * rcPlaneNormal.x + a_rcCenter.z * rcPlaneNormal.z - a_rcPlane.GetD();
	TFLOAT fNegRadius = -a_fRadius;

	// Behind the plane
	if ( fPlaneDist < fNegRadius )
		return 0;

	TFLOAT   fOriginalDistance = a_rfDistance;
	TVector4 vecOriginalNormal = a_rNormal;
	a_rfDistance               = EPSILON + a_rfDistance;

	TINT iResult = 0;

	// Face
	if ( fDirDot < 0.0f && a_fRadius - EPSILON < fPlaneDist )
	{
		TFLOAT fGap = fPlaneDist - a_fRadius;
		if ( fGap < 0.0f ) fGap = 0.0f;

		TFLOAT fT = -( fGap / fDirDot );

		TVector4 vecPoint;
		vecPoint.Multiply( vecDirection, fT );
		vecPoint = a_rcCenter + vecPoint;

		if ( fT < a_rfDistance && IsPointInTriangle( *a_ppVertices[ 0 ], *a_ppVertices[ 1 ], *a_ppVertices[ 2 ], vecPoint ) )
		{
			a_rfDistance = fT;
			a_rNormal.Set( rcPlaneNormal.x, rcPlaneNormal.y, rcPlaneNormal.z, 1.0f );
			iResult = 1;
		}
	}

	// Vertices
	for ( TINT i = 0; i < 3; i++ )
	{
		TVector4 vecVertex = *a_ppVertices[ i ];
		TVector4 vecVertexEnd( vecVertex.x - vecDirection.x, vecVertex.y - vecDirection.y, vecVertex.z - vecDirection.z, vecVertex.w );

		TINT   iNumHits;
		TFLOAT fHit1 = TMath::MAXFLOAT;
		TFLOAT fHit2 = TMath::MAXFLOAT;

		if ( !SphereVsLine( a_rcCenter, a_fRadius, vecVertex, vecVertexEnd, &iNumHits, &fHit1, &fHit2 ) )
			continue;

		TFLOAT fT;

		if ( fHit2 < -EPSILON || fHit2 >= a_rfDistance )
			fT = fHit1;
		else if ( fHit1 < -EPSILON || fHit1 >= a_rfDistance )
			fT = fHit2;
		else
			fT = ( fHit2 < fHit1 ) ? fHit2 : fHit1;

		if ( fT >= -EPSILON && fT < a_rfDistance )
		{
			if ( fT < 0.0f ) fT = 0.0f;

			TVector4 vecOffset;
			vecOffset.Multiply( vecDirection, fT );

			TVector4 vecHitPoint( vecVertex.x - vecOffset.x, vecVertex.y - vecOffset.y, vecVertex.z - vecOffset.z, vecVertex.w );
			a_rNormal.Set( a_rcCenter.x - vecHitPoint.x, a_rcCenter.y - vecHitPoint.y, a_rcCenter.z - vecHitPoint.z, a_rcCenter.w );

			iResult += 2;
			a_rfDistance = fT;
		}
	}

	// Edges
	for ( TINT i = 0; i < 3; i++ )
	{
		TVector4 vecStart = *a_ppVertices[ i ];
		TVector4 vecEnd   = *a_ppVertices[ ( i + 1 == 3 ) ? 0 : i + 1 ];
		TVector3 vecEndBack( vecEnd.x - vecDirection.x, vecEnd.y - vecDirection.y, vecEnd.z - vecDirection.z );

		// Plane containing the edge and the movement direction
		TPlane oEdgePlane;
		oEdgePlane.Set( vecStart.AsVector3(), vecEnd.AsVector3(), vecEndBack );

		const TVector3& rcEdgeNormal = oEdgePlane.GetNormal();
		TFLOAT          fEdgeDist    = rcEdgeNormal.y * a_rcCenter.y + rcEdgeNormal.z * a_rcCenter.z + rcEdgeNormal.x * a_rcCenter.x - oEdgePlane.GetD();

		if ( fEdgeDist > a_fRadius || fEdgeDist < fNegRadius )
			continue;

		// Circle where the sphere cuts the plane
		TVector4 vecCircleCenter( a_rcCenter.x - rcEdgeNormal.x * fEdgeDist, a_rcCenter.y - rcEdgeNormal.y * fEdgeDist, a_rcCenter.z - rcEdgeNormal.z * fEdgeDist, 1.0f );

		TASSERT( ( vecStart - vecEnd ).Magnitude() >= TMath::FLOATEPSILON );
		TVector4 vecClosest;
		DistancePointToLineSq( vecCircleCenter, vecStart, vecEnd, vecClosest );

		TVector4 vecToEdge( vecClosest.x - vecCircleCenter.x, vecClosest.y - vecCircleCenter.y, vecClosest.z - vecCircleCenter.z, vecClosest.w );
		vecToEdge.Normalise();

		TVector4 vecTouch;
		vecTouch.Multiply( vecToEdge, sqrtf( a_fRadius * a_fRadius - fEdgeDist * fEdgeDist ) );
		vecTouch.x += vecCircleCenter.x;
		vecTouch.y += vecCircleCenter.y;
		vecTouch.z += vecCircleCenter.z;

		// Drop the dominant axis of the plane normal and intersect the lines in 2D
		TFLOAT fAbsX = TMath::Abs( rcEdgeNormal.x );
		TFLOAT fAbsY = TMath::Abs( rcEdgeNormal.y );
		TFLOAT fAbsZ = TMath::Abs( rcEdgeNormal.z );

		TINT iAxis1 = 0;
		TINT iAxis2 = 1;

		if ( fAbsX > fAbsY && fAbsX > fAbsZ )
		{
			iAxis1 = 1;
			iAxis2 = 2;
		}
		else if ( fAbsY > fAbsZ )
		{
			iAxis1 = 0;
			iAxis2 = 2;
		}

		TVector4 vecTouchEnd( vecTouch.x + vecDirection.x, vecTouch.y + vecDirection.y, vecTouch.z + vecDirection.z, vecTouch.w );

		const TFLOAT* pTouch    = &vecTouch.x;
		const TFLOAT* pTouchEnd = &vecTouchEnd.x;
		const TFLOAT* pStart    = &vecStart.x;
		const TFLOAT* pEnd      = &vecEnd.x;

		TFLOAT afLine1Start[ 2 ] = { pTouch[ iAxis1 ], pTouch[ iAxis2 ] };
		TFLOAT afLine1End[ 2 ]   = { pTouchEnd[ iAxis1 ], pTouchEnd[ iAxis2 ] };
		TFLOAT afLine2Start[ 2 ] = { pStart[ iAxis1 ], pStart[ iAxis2 ] };
		TFLOAT afLine2End[ 2 ]   = { pEnd[ iAxis1 ], pEnd[ iAxis2 ] };

		TFLOAT fT;
		if ( LineVsLine2D( afLine1Start, afLine1End, afLine2Start, afLine2End, fT ) && fT >= 0.0f && fT < a_rfDistance )
		{
			TVector4 vecHit;
			vecHit.Multiply( vecDirection, fT );
			vecHit = vecTouch + vecHit;

			TVector4 vecToStart = vecStart - vecHit;
			TVector4 vecToEnd   = vecEnd - vecHit;

			// The hit point should be between the edge vertices
			if ( TVector4::DotProduct3( vecToStart, vecToEnd ) <= 0.0f )
			{
				a_rfDistance = fT;
				a_rNormal.Set( a_rcCenter.x - vecTouch.x, a_rcCenter.y - vecTouch.y, a_rcCenter.z - vecTouch.z, a_rcCenter.w );
				iResult += 16;
			}
		}
	}

	a_rfDistance = a_rfDistance - EPSILON;

	if ( iResult != 0 )
	{
		a_rNormal.Normalise();
		TFLOAT fNormalDot = TVector4::DotProduct3( a_rNormal, vecDirection );

		if ( a_rfDistance < 0.0f && fNormalDot >= -0.0001f )
		{
			// Already touching and not moving into it
			a_rfDistance = fOriginalDistance;
			a_rNormal    = vecOriginalNormal;
			iResult      = 0;
		}

		if ( fNormalDot <= -0.0001f )
			a_rfDistance = a_rfDistance - s_fPushBack;

		if ( a_rfDistance < 0.0f )
			a_rfDistance = 0.0f;
	}

	return iResult;
}

// $deBlob: FUNCTION 0066fca0
// $Barnyard: FUNCTION 006b93b0
TINT TMath::SweptSphereVsTriangle( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector3* const* a_ppVertices, TFLOAT& a_rfDistance, const TPlane& a_rcPlane, TVector4& a_rNormal )
{
	static TVector4 s_aVertices[ 3 ];

	s_aVertices[ 0 ] = TVector4( *a_ppVertices[ 0 ] );
	s_aVertices[ 1 ] = TVector4( *a_ppVertices[ 1 ] );
	s_aVertices[ 2 ] = TVector4( *a_ppVertices[ 2 ] );

	const TVector4* apVertices[ 3 ] = { &s_aVertices[ 0 ], &s_aVertices[ 1 ], &s_aVertices[ 2 ] };
	return SweptSphereVsTriangle( a_rcCenter, a_fRadius, a_rcDirection, apVertices, a_rfDistance, a_rcPlane, a_rNormal );
}

// $deBlob: FUNCTION 006718d0
// $Barnyard: FUNCTION 006ba3c0
void TMath::SlideAlongPlane( const TVector4& a_rcDirection, const TVector4& a_rcNormal, TFLOAT a_fSpeedScale, TFLOAT a_fCompleteDist, TFLOAT a_fTravelledDistance, TFLOAT& a_rfOutDistance, TVector4& a_rOutDirection, TFLOAT a_fPushOut, TFLOAT a_fFriction, TFLOAT a_fMaxDot )
{
	TASSERT( a_fCompleteDist >= 0.0f );

	TFLOAT fRemaining = a_fCompleteDist - a_fTravelledDistance;
	TFLOAT fDot       = -TVector4::DotProduct3( a_rcNormal, a_rcDirection );

	if ( fDot <= 0.0f )
		fDot = 0.0f;

	if ( fDot < a_fMaxDot )
	{
		// Push the direction off the plane
		TVector4 vecPush;
		vecPush.Multiply( a_rcNormal, fDot + a_fPushOut );

		a_rOutDirection.Set( vecPush.x + a_rcDirection.x, vecPush.y + a_rcDirection.y, vecPush.z + a_rcDirection.z, a_rcDirection.w );
		a_rOutDirection.Normalise();

		TFLOAT fFactor = fDot + a_fFriction;

		if ( fFactor > 1.0f )
			fFactor = 1.0f;
		else if ( fFactor < 0.0f )
			fFactor = 0.0f;

		TFLOAT fDistance = ( 1.0f - fFactor * fFactor ) * ( fRemaining * a_fSpeedScale );
		a_rfOutDistance  = ( fDistance > 0.0f ) ? fDistance : 0.0f;
	}
	else
	{
		a_rfOutDistance = 0.0f;
	}
}

// $deBlob: FUNCTION 0066b780
// $Barnyard: FUNCTION 006b64e0
TFLOAT TMath::DistancePointToLineSq( const TVector4& a_rcPoint, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd, TFLOAT& a_rfT )
{
	TFLOAT fLineX = a_rcLineEnd.x - a_rcLineStart.x;
	TFLOAT fLineY = a_rcLineEnd.y - a_rcLineStart.y;
	TFLOAT fLineZ = a_rcLineEnd.z - a_rcLineStart.z;

	a_rfT = ( fLineZ * ( a_rcPoint.z - a_rcLineStart.z ) + fLineX * ( a_rcPoint.x - a_rcLineStart.x ) + fLineY * ( a_rcPoint.y - a_rcLineStart.y ) ) /
	    ( fLineZ * fLineZ + fLineY * fLineY + fLineX * fLineX );

	TFLOAT fDX = fLineX * a_rfT + a_rcLineStart.x - a_rcPoint.x;
	TFLOAT fDY = fLineY * a_rfT + a_rcLineStart.y - a_rcPoint.y;
	TFLOAT fDZ = fLineZ * a_rfT + a_rcLineStart.z - a_rcPoint.z;

	return fDZ * fDZ + fDY * fDY + fDX * fDX;
}

// $deBlob: FUNCTION 0066bff0
// $Barnyard: FUNCTION 006b7000
TFLOAT TMath::DistanceSegmentToSegmentSq( const TVector4& a_rcStart1, const TVector4& a_rcEnd1, const TVector4& a_rcStart2, const TVector4& a_rcEnd2, TFLOAT& a_rfT1, TFLOAT& a_rfT2 )
{
	TVector3 vecDir1( a_rcEnd1.x - a_rcStart1.x, a_rcEnd1.y - a_rcStart1.y, a_rcEnd1.z - a_rcStart1.z );
	TVector3 vecDir2( a_rcEnd2.x - a_rcStart2.x, a_rcEnd2.y - a_rcStart2.y, a_rcEnd2.z - a_rcStart2.z );
	TVector3 vecDiff( a_rcStart1.x - a_rcStart2.x, a_rcStart1.y - a_rcStart2.y, a_rcStart1.z - a_rcStart2.z );

	TFLOAT fA = vecDir1.x * vecDir1.x + vecDir1.z * vecDir1.z + vecDir1.y * vecDir1.y;
	TFLOAT fB = vecDir2.x * vecDir1.x + vecDir2.z * vecDir1.z + vecDir2.y * vecDir1.y;
	TFLOAT fE = vecDir2.y * vecDir2.y + vecDir2.z * vecDir2.z + vecDir2.x * vecDir2.x;
	TFLOAT fC = vecDiff.x * vecDir1.x + vecDiff.z * vecDir1.z + vecDiff.y * vecDir1.y;
	TFLOAT fF = vecDiff.y * vecDir2.y + vecDiff.z * vecDir2.z + vecDiff.x * vecDir2.x;

	TFLOAT fAE    = fE * fA;
	TFLOAT fDenom = fAE - fB * fB;

	TFLOAT fParallel = fDenom;
	if ( TMath::Abs( fAE ) > 0.01f )
		fParallel = fDenom / fAE;

	TFLOAT fNum1, fDenom1;
	TFLOAT fNum2, fDenom2;

	if ( fParallel < 0.00001f )
	{
		// The segments are parallel
		fNum1   = 0.0f;
		fDenom1 = 1.0f;
		fNum2   = fF;
		fDenom2 = fE;
	}
	else
	{
		fNum1   = fF * fB - fC * fE;
		fNum2   = fF * fA - fC * fB;
		fDenom1 = fDenom;
		fDenom2 = fDenom;

		if ( fNum1 < 0.0f )
		{
			fNum1   = 0.0f;
			fNum2   = fF;
			fDenom2 = fE;
		}
		else if ( fNum1 > fDenom1 )
		{
			fNum1   = fDenom1;
			fNum2   = fF + fB;
			fDenom2 = fE;
		}
	}

	if ( fNum2 < 0.0f )
	{
		fNum2 = 0.0f;

		if ( -fC < 0.0f )
		{
			fNum1 = 0.0f;
		}
		else if ( -fC > fA )
		{
			fNum1 = fDenom1;
		}
		else
		{
			fNum1   = -fC;
			fDenom1 = fA;
		}
	}
	else if ( fNum2 > fDenom2 )
	{
		fNum2 = fDenom2;

		if ( fB - fC < 0.0f )
		{
			fNum1 = 0.0f;
		}
		else if ( fB - fC > fA )
		{
			fNum1 = fDenom1;
		}
		else
		{
			fNum1   = fB - fC;
			fDenom1 = fA;
		}
	}

	a_rfT1 = ( TMath::Abs( fNum1 ) < 0.00001f ) ? 0.0f : fNum1 / fDenom1;
	a_rfT2 = ( TMath::Abs( fNum2 ) < 0.00001f ) ? 0.0f : fNum2 / fDenom2;

	TFLOAT fDX = vecDir1.x * a_rfT1 + vecDiff.x - vecDir2.x * a_rfT2;
	TFLOAT fDY = vecDir1.y * a_rfT1 + vecDiff.y - vecDir2.y * a_rfT2;
	TFLOAT fDZ = vecDir1.z * a_rfT1 + vecDiff.z - vecDir2.z * a_rfT2;

	return fDY * fDY + fDX * fDX + fDZ * fDZ;
}

// $deBlob: FUNCTION 00670210
// $Barnyard: FUNCTION 006b9c30
TBOOL TMath::RayVsCapsuleOutside( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, const TVector4& a_rcBase, TFLOAT a_fRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon )
{
	if ( a_fHeight == 0.0f )
	{
		// Sphere
		TFLOAT fMaxDistance = a_rfDistance;

		TVector4 vecFar;
		vecFar.Multiply( a_rcDirection, 10000.0f );
		vecFar.Set( a_rcOrigin.x + vecFar.x, a_rcOrigin.y + vecFar.y, a_rcOrigin.z + vecFar.z, a_rcOrigin.w );

		TFLOAT fT;
		TFLOAT fDistanceSq = DistancePointToSegmentSq( a_rcBase, a_rcOrigin, vecFar, fT );

		// NOTE: written as retail tests it (a NaN is no hit)
		if ( !( fDistanceSq - a_fRadius * a_fRadius < 0.0f ) )
			return TFALSE;

		TFLOAT fHit  = fT * 10000.0f - sqrtf( ( a_fRadius + a_fEpsilon ) * ( a_fRadius + a_fEpsilon ) - fDistanceSq );
		a_rfDistance = fHit;

		if ( fHit > fMaxDistance || 0.0f > fHit )
		{
			a_rfDistance = fMaxDistance;
			return TFALSE;
		}

		TVector4 vecHit;
		vecHit.Multiply( a_rcDirection, fHit );

		a_rNormal.Set( vecHit.x + a_rcOrigin.x - a_rcBase.x, a_rcOrigin.y + vecHit.y - a_rcBase.y, a_rcOrigin.z + vecHit.z - a_rcBase.z, a_rcOrigin.w );
		a_rNormal.Normalise();
		return TTRUE;
	}

	// Capsule
	TFLOAT fMaxDistance = a_rfDistance;

	TVector4 vecFar;
	vecFar.Multiply( a_rcDirection, 10000.0f );
	vecFar.Set( vecFar.x + a_rcOrigin.x, a_rcOrigin.y + vecFar.y, a_rcOrigin.z + vecFar.z, a_rcOrigin.w );

	TVector4 vecTop;
	vecTop.Multiply( a_rcUp, a_fHeight );
	vecTop.Set( a_rcBase.x + vecTop.x, a_rcBase.y + vecTop.y, a_rcBase.z + vecTop.z, a_rcBase.w );

	TFLOAT fRayT;
	TFLOAT fCapsuleT;
	TFLOAT fDistanceSq = DistanceSegmentToSegmentSq( a_rcOrigin, vecFar, a_rcBase, vecTop, fRayT, fCapsuleT );

	// NOTE: written as retail tests it (a NaN is no hit)
	if ( !( fDistanceSq - a_fRadius * a_fRadius < 0.0f ) )
		return TFALSE;

	TFLOAT fHit  = fRayT * 10000.0f - sqrtf( ( a_fRadius + a_fEpsilon ) * ( a_fRadius + a_fEpsilon ) - fDistanceSq );
	a_rfDistance = fHit;

	if ( fHit > fMaxDistance || 0.0f > fHit )
	{
		a_rfDistance = fMaxDistance;
		return TFALSE;
	}

	// NOTE: retail scales the hit offset by the ray parameter again (the hit point is a_rcOrigin + dir * fHit * fRayT) and
	// has no cylinder case (Barnyard flattens the normal when the hit is between the caps)
	TVector4 vecHitOffset;
	vecHitOffset.Multiply( a_rcDirection, fHit );
	TVector4 vecScaled;
	vecScaled.Multiply( vecHitOffset, fRayT );
	TVector4 vecHit( vecScaled.x + a_rcOrigin.x, a_rcOrigin.y + vecScaled.y, a_rcOrigin.z + vecScaled.z, a_rcOrigin.w );

	TVector4 vecAxis;
	vecAxis.Multiply( a_rcUp, a_fHeight );
	TVector4 vecAxisOffset;
	vecAxisOffset.Multiply( vecAxis, fCapsuleT );

	a_rNormal.Set( vecHit.x - ( a_rcBase.x + vecAxisOffset.x ), vecHit.y - ( a_rcBase.y + vecAxisOffset.y ), vecHit.z - ( a_rcBase.z + vecAxisOffset.z ), vecHit.w );
	a_rNormal.Normalise();
	return TTRUE;
}

// $Barnyard: FUNCTION 006ba760
TBOOL TMath::RayVsCapsule( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, const TVector4& a_rcBase, TFLOAT a_fRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon )
{
	if ( !IsPointInCapsule( a_rcOrigin, a_rcBase, a_rcUp, a_fRadius, a_fHeight ) )
		return RayVsCapsuleOutside( a_rcOrigin, a_rcDirection, a_rcBase, a_fRadius, a_rcUp, a_fHeight, a_rfDistance, a_rNormal, a_fEpsilon );

	TVector4 vecEnd( a_rcOrigin.x + a_rcDirection.x, a_rcOrigin.y + a_rcDirection.y, a_rcOrigin.z + a_rcDirection.z, a_rcOrigin.w );

	// The whole ray is inside of the capsule
	if ( IsPointInCapsule( vecEnd, a_rcBase, a_rcUp, a_fRadius, a_fHeight ) )
		return TFALSE;

	// Cast the ray back from its end
	TVector4 vecBackDirection;
	vecBackDirection.Multiply( a_rcDirection, -1.0f );

	TVector4 vecStart = a_rcOrigin + a_rcDirection;
	TBOOL    bResult  = RayVsCapsuleOutside( vecStart, vecBackDirection, a_rcBase, a_fRadius, a_rcUp, a_fHeight, a_rfDistance, a_rNormal, a_fEpsilon );

	if ( bResult == TTRUE )
		a_rfDistance = a_rcDirection.Magnitude() - a_rfDistance;

	return bResult;
}

// $deBlob: FUNCTION 00671120
// $Barnyard: FUNCTION 006ba090
TBOOL TMath::SphereVsCapsule( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcBase, TFLOAT a_fCapsuleRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon )
{
	TFLOAT fOriginalDistance = a_rfDistance;

	TVector4 vecTop;
	vecTop.Multiply( a_rcUp, a_fHeight );
	vecTop.Set( vecTop.x + a_rcBase.x, vecTop.y + a_rcBase.y, vecTop.z + a_rcBase.z, a_rcBase.w );

	TFLOAT fT;
	TFLOAT fDistanceSq = DistancePointToSegmentSq( a_rcCenter, a_rcBase, vecTop, fT );
	TFLOAT fRadius     = a_fRadius + a_fCapsuleRadius;

	// NOTE: negated so that NaN (a zero height capsule, i.e. a sphere, makes a degenerate segment) returns TFALSE like the original's x87 compare
	if ( !( fDistanceSq - fRadius * fRadius < 0.0f ) )
		return TFALSE;

	// Penetration depth
	TFLOAT fDepth = fRadius - sqrtf( fDistanceSq ) + a_fEpsilon;
	a_rfDistance  = fDepth;

	if ( fDepth > fOriginalDistance || fDepth < -a_fRadius )
	{
		a_rfDistance = fOriginalDistance;
		return TFALSE;
	}

	TVector4 vecAxisPoint;
	vecAxisPoint.Multiply( a_rcUp, fT * a_fHeight );
	vecAxisPoint = a_rcBase + vecAxisPoint;

	a_rNormal.Set( a_rcCenter.x - vecAxisPoint.x, a_rcCenter.y - vecAxisPoint.y, a_rcCenter.z - vecAxisPoint.z, a_rcCenter.w );
	a_rNormal.Normalise();
	return TTRUE;
}

// $deBlob: FUNCTION 00670610
// $Barnyard: FUNCTION 006b9490
TBOOL TMath::SweptSphereVsCapsule( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, const TVector4& a_rcBase, TFLOAT a_fCapsuleRadius, const TVector4& a_rcUp, TFLOAT a_fHeight, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fEpsilon )
{
	TASSERT( a_rfDistance > 0.0f );

	TFLOAT fMaxDistance = a_rfDistance;
	TFLOAT fRadius      = a_fRadius + a_fCapsuleRadius;

	if ( a_fHeight == 0.0f )
	{
		// Sphere
		TVector4 vecEnd;
		vecEnd.Multiply( a_rcDirection, fMaxDistance );
		vecEnd.Set( vecEnd.x + a_rcCenter.x, vecEnd.y + a_rcCenter.y, vecEnd.z + a_rcCenter.z, a_rcCenter.w );

		TFLOAT fT;
		TFLOAT fDistanceSq = DistancePointToLineSq( a_rcBase, a_rcCenter, vecEnd, fT );
		TFLOAT fRadiusSq   = fRadius * fRadius;

		if ( !( fDistanceSq - fRadiusSq < 0.0f ) )
			return TFALSE;

		TFLOAT fHit  = fT * fMaxDistance - ( sqrtf( fRadiusSq - fDistanceSq ) + a_fEpsilon );
		a_rfDistance = fHit;

		if ( fHit > fMaxDistance )
		{
			// Only check if the spheres are already touching on the XZ plane
			TVector4 vecToCenter( a_rcCenter.x - a_rcBase.x, 0.0f, a_rcCenter.z - a_rcBase.z, a_rcCenter.w );
			TFLOAT   fDistSq = vecToCenter.MagnitudeSq();

			if ( !( fDistSq > 0.0f ) || !( a_fEpsilon * a_fEpsilon + fRadiusSq > fDistSq ) )
			{
				a_rfDistance = fMaxDistance;
				return TFALSE;
			}

			if ( !( TVector4::DotProduct3( vecToCenter, a_rcDirection ) < 0.0f ) )
			{
				a_rfDistance = fMaxDistance;
				return TTRUE;
			}

			TFLOAT fDist = sqrtf( fDistSq );
			vecToCenter.Divide( fDist );

			a_rNormal    = vecToCenter;
			a_rfDistance = -fRadius + fDist;
			return TTRUE;
		}

		if ( fHit < 0.0f )
		{
			// Already overlapping
			if ( !( TVector4::DistanceSq( a_rcCenter, a_rcBase ) - fRadiusSq - a_fEpsilon * a_fEpsilon < 0.0f ) )
			{
				a_rfDistance = fMaxDistance;
				return TFALSE;
			}

			// NOTE: the vector to the base is not normalised
			TVector4 vecToBase = a_rcBase - a_rcCenter;
			TVector4 vecOffset;
			vecOffset.Multiply( vecToBase, fRadius );

			TVector4 vecPoint  = a_rcCenter + vecOffset;
			TVector4 vecNormal = a_rcCenter - vecPoint;

			if ( !( TVector4::DotProduct3( vecNormal, a_rcDirection ) < 0.0f ) )
			{
				a_rfDistance = fMaxDistance;
				return TTRUE;
			}

			a_rNormal    = vecNormal;
			a_rfDistance = a_rNormal.NormaliseMag() - fRadius - a_fEpsilon;

			if ( a_rfDistance >= 0.0f )
				a_rfDistance = -a_fEpsilon;

			return TTRUE;
		}

		TVector4 vecHit;
		vecHit.Multiply( a_rcDirection, fHit );
		vecHit = a_rcCenter + vecHit;

		a_rNormal = vecHit - a_rcBase;

		if ( a_rfDistance < 0.0f )
		{
			a_rfDistance = a_rNormal.NormaliseMag() - fRadius - a_fEpsilon;
			return TTRUE;
		}

		a_rNormal.Normalise();
		return TTRUE;
	}

	// Capsule
	TFLOAT fLength = ( fRadius + fMaxDistance + a_fHeight ) * 3.0f;

	TVector4 vecOffset;
	vecOffset.Multiply( a_rcDirection, fLength );

	TVector4 vecFar( vecOffset.x + a_rcCenter.x, vecOffset.y + a_rcCenter.y, vecOffset.z + a_rcCenter.z, a_rcCenter.w );

	TVector4 vecTop;
	vecTop.Multiply( a_rcUp, a_fHeight );
	vecTop.Set( vecTop.x + a_rcBase.x, vecTop.y + a_rcBase.y, vecTop.z + a_rcBase.z, a_rcBase.w );

	TVector4 vecNear( a_rcCenter.x - vecOffset.x, a_rcCenter.y - vecOffset.y, a_rcCenter.z - vecOffset.z, a_rcCenter.w );

	TFLOAT fMoveT;
	TFLOAT fCapsuleT;
	TFLOAT fDistanceSq = DistanceSegmentToSegmentSq( vecNear, vecFar, a_rcBase, vecTop, fMoveT, fCapsuleT );

	// vStartSwept: vecNear, vEndSwept: vecFar, a_rvCapsuleCentre: a_rcBase, vCapEnd: vecTop
	TASSERT( fDistanceSq <= TVector4::DistanceSq( vecNear, a_rcBase ) + .01f );
	TASSERT( fDistanceSq <= TVector4::DistanceSq( vecNear, vecTop ) + .01f );
	TASSERT( fDistanceSq <= TVector4::DistanceSq( vecFar, a_rcBase ) + .01f );
	TASSERT( fDistanceSq <= TVector4::DistanceSq( vecFar, vecTop ) + .01f );

	TFLOAT fRadiusSq = fRadius * fRadius;

	if ( !( fDistanceSq - fRadiusSq <= 0.0f ) )
		return TFALSE;

	TFLOAT fHit  = ( fMoveT * fLength + fMoveT * fLength - sqrtf( fRadiusSq - fDistanceSq + a_fEpsilon ) ) - fLength;
	a_rfDistance = fHit;

	if ( fHit > fMaxDistance )
	{
		a_rfDistance = fMaxDistance;
		return TFALSE;
	}

	if ( fHit < 0.0f )
	{
		// Already overlapping
		TFLOAT fT;

		if ( !( DistancePointToSegmentSq( a_rcCenter, a_rcBase, vecTop, fT ) - fRadiusSq - a_fEpsilon * a_fEpsilon < 0.0f ) )
		{
			a_rfDistance = fMaxDistance;
			return TFALSE;
		}

		TVector4 vecAxis = vecTop - a_rcBase;
		TVector4 vecAxisOffset;
		vecAxisOffset.Multiply( vecAxis, fT );

		TVector4 vecAxisPoint = a_rcBase + vecAxisOffset;
		TVector4 vecNormal    = a_rcCenter - vecAxisPoint;

		if ( !( TVector4::DotProduct3( vecNormal, a_rcDirection ) < 0.0f ) )
		{
			a_rfDistance = fMaxDistance;
			return TTRUE;
		}

		a_rNormal    = vecNormal;
		a_rfDistance = a_rNormal.NormaliseMag() - fRadius - a_fEpsilon;

		if ( a_rfDistance >= 0.0f )
			a_rfDistance = -a_fEpsilon;

		return TTRUE;
	}

	fHit         = fHit - a_fEpsilon * 10.0f;
	a_rfDistance = fHit;

	TVector4 vecHit;
	vecHit.Multiply( a_rcDirection, fHit );
	vecHit = a_rcCenter + vecHit;

	TVector4 vecAxisPoint;
	vecAxisPoint.Multiply( a_rcUp, a_fHeight );
	vecAxisPoint.Multiply( fCapsuleT );
	vecAxisPoint = a_rcBase + vecAxisPoint;

	a_rNormal = vecHit - vecAxisPoint;

	if ( a_rfDistance < 0.0f )
	{
		a_rfDistance = a_rNormal.NormaliseMag() - fRadius - a_fEpsilon;
		return TTRUE;
	}

	a_rNormal.Normalise();
	return TTRUE;
}

// $deBlob: FUNCTION 00671a50
// $Barnyard: FUNCTION 006ba530
void TMath::SlideAlongPlane( const TVector4& a_rcDirection, const TVector4& a_rcNormal, TFLOAT a_fSpeedScale, TFLOAT a_fCompleteDist, TFLOAT a_fTravelledDistance, TFLOAT& a_rfOutDistance, TVector4& a_rOutDirection, TBOOL a_bLinearFriction, TFLOAT a_fPushOut, TFLOAT a_fFriction, TFLOAT a_fMaxDot )
{
	TASSERT( a_fCompleteDist >= 0.0f );

	TFLOAT fRemaining = a_fCompleteDist - a_fTravelledDistance;
	TFLOAT fDot       = -TVector4::DotProduct3( a_rcDirection, a_rcNormal );

	if ( fDot <= 0.0f )
		fDot = 0.0f;

	if ( fDot < a_fMaxDot )
	{
		// Push the direction off the plane
		TVector4 vecPush;
		vecPush.Multiply( a_rcNormal, fDot + a_fPushOut );

		a_rOutDirection.Set( vecPush.x + a_rcDirection.x, vecPush.y + a_rcDirection.y, vecPush.z + a_rcDirection.z, a_rcDirection.w );
		a_rOutDirection.Normalise();

		TFLOAT fFactor = fDot + a_fFriction;

		if ( fFactor > 1.0f )
			fFactor = 1.0f;
		else if ( fFactor < 0.0f )
			fFactor = 0.0f;

		TFLOAT fDistance = a_bLinearFriction ? ( fRemaining * a_fSpeedScale ) * ( 1.0f - fFactor ) : ( 1.0f - fFactor * fFactor ) * ( fRemaining * a_fSpeedScale );
		a_rfOutDistance  = ( fDistance > 0.0f ) ? fDistance : 0.0f;
	}
	else
	{
		a_rfOutDistance = 0.0f;
	}
}

// 1 / log( 0.5 ), used by Bias and Gain
// $deBlob: FUNCTION 0053e760
static const TFLOAT s_fInvLogHalf = 1.0f / TMath::Log( 0.5f );

// $deBlob: FUNCTION 00667680
void TMath::IncrMaxf( TFLOAT& a_rfValue, TFLOAT a_fAmount, TFLOAT a_fMax )
{
	TASSERT( a_fAmount >= 0.0f );

	TFLOAT fValue = a_rfValue + a_fAmount;
	a_rfValue     = fValue;

	if ( a_fMax < fValue )
		a_rfValue = a_fMax;
}

// $deBlob: FUNCTION 006676f0
void TMath::DecrMinf( TFLOAT& a_rfValue, TFLOAT a_fAmount, TFLOAT a_fMin )
{
	TASSERT( a_fAmount >= 0.0f );

	TFLOAT fValue = a_rfValue - a_fAmount;
	a_rfValue     = fValue;

	if ( fValue < a_fMin )
		a_rfValue = a_fMin;
}

// $deBlob: FUNCTION 00666ef0
// $Barnyard: FUNCTION 006b69f0
TFLOAT TMath::ASin( TFLOAT a_fX )
{
	TASSERT( a_fX >= -1.0f );
	TASSERT( a_fX <= 1.0f );

	TMATH_DOUBLE_PRECISION_BEGIN();
	TFLOAT fResult = TFLOAT( asin( a_fX ) );
	TMATH_DOUBLE_PRECISION_END();
	TASSERT( TMath::IsFinite( fResult ) );

	return fResult;
}

// $deBlob: FUNCTION 00667000
// $Barnyard: FUNCTION 006b6a20
TFLOAT TMath::ACos( TFLOAT a_fX )
{
	TASSERT( a_fX >= -1.0f );
	TASSERT( a_fX <= 1.0f );

	TMATH_DOUBLE_PRECISION_BEGIN();
	TFLOAT fResult = TFLOAT( acos( a_fX ) );
	TMATH_DOUBLE_PRECISION_END();
	TASSERT( TMath::IsFinite( fResult ) );

	return fResult;
}

// $deBlob: FUNCTION 00667110
// $Barnyard: FUNCTION 006b6a50
TFLOAT TMath::ATan( TFLOAT a_fX )
{
	TMATH_DOUBLE_PRECISION_BEGIN();
	TFLOAT fResult = TFLOAT( atan( a_fX ) );
	TMATH_DOUBLE_PRECISION_END();
	TASSERT( TMath::IsFinite( fResult ) );

	return fResult;
}

// $deBlob: FUNCTION 00667220
// $Barnyard: FUNCTION 006b6a80
TFLOAT TMath::Exp( TFLOAT a_fX )
{
	TMATH_DOUBLE_PRECISION_BEGIN();
	TFLOAT fResult = TFLOAT( exp( a_fX ) );
	TMATH_DOUBLE_PRECISION_END();
	TASSERT( TMath::IsFinite( fResult ) );

	return fResult;
}

// Natural logarithm
// $deBlob: FUNCTION 006672b0
// $Barnyard: FUNCTION 006b6ac0
TFLOAT TMath::Log( TFLOAT a_fX )
{
	TMATH_DOUBLE_PRECISION_BEGIN();
	TFLOAT fResult = TFLOAT( log( a_fX ) );
	TMATH_DOUBLE_PRECISION_END();
	TASSERT( TMath::IsFinite( fResult ) );

	return fResult;
}

// $deBlob: FUNCTION 00667340
TFLOAT TMath::Log10( TFLOAT a_fX )
{
	TMATH_DOUBLE_PRECISION_BEGIN();
	TFLOAT fResult = TFLOAT( log10( a_fX ) );
	TMATH_DOUBLE_PRECISION_END();
	TASSERT( TMath::IsFinite( fResult ) );

	return fResult;
}

// $deBlob: FUNCTION 006673d0
// $Barnyard: FUNCTION 006b6af0
TFLOAT TMath::Pow( TFLOAT a_fX, TFLOAT a_fY )
{
	TMATH_DOUBLE_PRECISION_BEGIN();
	TFLOAT fResult = TFLOAT( pow( a_fX, a_fY ) );
	TMATH_DOUBLE_PRECISION_END();
	TASSERT( TMath::IsFinite( fResult ) );

	return fResult;
}

// $deBlob: FUNCTION 00671df0
// $Barnyard: FUNCTION 006b90d0
TFLOAT TMath::Bias( TFLOAT a_fBias, TFLOAT a_fValue )
{
	return Pow( a_fValue, s_fInvLogHalf * Log( a_fBias ) );
}

// $deBlob: FUNCTION 00667510
// $Barnyard: FUNCTION 006ba6f0
TFLOAT TMath::Gain( TFLOAT a_fGain, TFLOAT a_fValue )
{
	if ( a_fValue < 0.5f )
		return Pow( a_fValue * 2.0f, s_fInvLogHalf * Log( 1.0f - a_fGain ) ) * 0.5f;

	return 1.0f - Pow( 2.0f - a_fValue * 2.0f, s_fInvLogHalf * Log( 1.0f - a_fGain ) ) * 0.5f;
}

// $deBlob: FUNCTION 006675c0
// $Barnyard: FUNCTION 006b8fb0
TFLOAT TMath::Gain075( TFLOAT a_fValue )
{
	static const TFLOAT s_fLogQuarter = Log( 0.25f );
	static const TFLOAT s_fExponent   = s_fInvLogHalf * s_fLogQuarter;

	if ( 0.5f <= a_fValue )
		return 1.0f - Pow( 2.0f - a_fValue * 2.0f, s_fExponent ) * 0.5f;

	return Pow( a_fValue * 2.0f, s_fExponent ) * 0.5f;
}

// $deBlob: FUNCTION 00667ea0
TBOOL TMath::RayVsAABB( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, TFLOAT& a_rfDistance )
{
	TFLOAT fNear = -1.0e10f;
	TFLOAT fFar  = TMath::MAXFLOAT;
	TINT   iFace = -1;

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fDirection = ( &a_rcDirection.x )[ i ];
		TFLOAT fOrigin    = ( &a_rcOrigin.x )[ i ];
		TFLOAT fMin       = ( &a_rcCenter.x )[ i ] - ( &a_rcHalfExtents.x )[ i ];
		TFLOAT fMax       = ( &a_rcCenter.x )[ i ] + ( &a_rcHalfExtents.x )[ i ];

		if ( fDirection <= -TMath::TFLOAT_EPSILON || fDirection >= TMath::TFLOAT_EPSILON )
		{
			TFLOAT fInvDirection = 1.0f / fDirection;
			TFLOAT fT1           = ( fMin - fOrigin ) * fInvDirection;
			TFLOAT fT2           = ( fMax - fOrigin ) * fInvDirection;
			TINT   iSlabFace     = i;

			if ( fT1 > fT2 )
			{
				TFLOAT fTemp = fT1;
				fT1          = fT2;
				fT2          = fTemp;
				iSlabFace    = i + 3;
			}

			if ( fT1 > fNear )
			{
				fNear = fT1;
				iFace = iSlabFace;
			}

			if ( fFar > fT2 )
				fFar = fT2;

			if ( fNear > fFar || fFar < TMath::TFLOAT_EPSILON )
				return TFALSE;
		}
		else if ( fMin > fOrigin || fOrigin > fMax )
		{
			// Parallel to the slab and outside of it
			return TFALSE;
		}
	}

	if ( fNear > fFar || fFar < TMath::TFLOAT_EPSILON || iFace == -1 )
		return TFALSE;

	a_rfDistance = fNear;
	return TTRUE;
}

// $deBlob: FUNCTION 00667fa0
TBOOL TMath::RayVsTriangle( const TVector4& a_rcOrigin, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance )
{
	const TVector4& rcVertex0 = *a_ppVertices[ 0 ];

	TVector4 vecEdge1 = *a_ppVertices[ 1 ] - rcVertex0;
	vecEdge1.w        = a_ppVertices[ 1 ]->w;
	TVector4 vecEdge2 = *a_ppVertices[ 2 ] - rcVertex0;
	vecEdge2.w        = a_ppVertices[ 2 ]->w;

	TVector4 vecP;
	vecP.CrossProduct( a_rcDirection, vecEdge2 );

	TFLOAT fDet = TVector4::DotProduct3( vecP, vecEdge1 );

	if ( fDet < TMath::FLOATEPSILON )
		return TFALSE;

	TVector4 vecT = a_rcOrigin - rcVertex0;
	TFLOAT   fU   = TVector4::DotProduct3( vecT, vecP );

	if ( fU < 0.0f || fU > fDet )
		return TFALSE;

	TVector4 vecQ;
	vecQ.CrossProduct( vecT, vecEdge1 );

	TFLOAT fV = TVector4::DotProduct3( a_rcDirection, vecQ );

	if ( fV < 0.0f || fV + fU > fDet )
		return TFALSE;

	a_rfDistance = TVector4::DotProduct3( vecQ, vecEdge2 ) / fDet;
	return TTRUE;
}

// $deBlob: FUNCTION 006684d0
TBOOL TMath::SweptSegmentVsTriangle( const TVector4& a_rcStart, const TVector4& a_rcEnd, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance )
{
	TFLOAT fLength = TVector4::Distance( a_rcStart, a_rcEnd );

	TVector4 aEdgeDirs[ 3 ];
	GetSweptSegmentTriangleEdgeDirs( a_ppVertices, aEdgeDirs );

	const TVector4* apEdgeDirs[ 3 ] = { &aEdgeDirs[ 0 ], &aEdgeDirs[ 1 ], &aEdgeDirs[ 2 ] };
	return SweptSegmentVsTriangle( a_rcStart, a_rcEnd, fLength, a_rcDirection, a_ppVertices, apEdgeDirs, a_rfDistance );
}

// $deBlob: FUNCTION 00668550
TBOOL TMath::SweptSegmentVsTriangle( const TVector4& a_rcStart, const TVector4& a_rcEnd, TFLOAT a_fLength, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, const TVector4* const* a_ppEdgeDirs, TFLOAT& a_rfDistance )
{
	TVector4 vecSegment = a_rcEnd - a_rcStart;

	TVector4 vecSegmentDir = vecSegment;
	vecSegmentDir.Divide( a_fLength );

	// The segment itself crosses the triangle
	TFLOAT fT;

	if ( RayVsTriangle( a_rcStart, vecSegmentDir, a_ppVertices, fT ) && fT >= 0.0f && a_fLength >= fT )
	{
		a_rfDistance = 0.0f;
		return TTRUE;
	}

	TVector4 vecBackDir( -vecSegmentDir.x, -vecSegmentDir.y, -vecSegmentDir.z, vecSegmentDir.w );

	if ( RayVsTriangle( a_rcEnd, vecBackDir, a_ppVertices, fT ) && fT >= 0.0f && a_fLength >= fT )
	{
		a_rfDistance = 0.0f;
		return TTRUE;
	}

	// Moving along the segment
	if ( TMath::Abs( a_rcDirection.y * vecSegmentDir.y + a_rcDirection.x * vecSegmentDir.x + a_rcDirection.z * vecSegmentDir.z ) > 0.999f )
		return TFALSE;

	// The plane swept by the segment
	TVector4 vecNormal;
	vecNormal.CrossProduct( vecSegment, a_rcDirection );
	vecNormal.Normalise();

	TFLOAT fPlaneD = vecNormal.y * a_rcStart.y + a_rcStart.x * vecNormal.x + a_rcStart.z * vecNormal.z;

	// Drop the dominant axis of the normal
	TVector4 vecAbsNormal = vecNormal;
	vecAbsNormal.Abs4();

	TINT iDominantAxis = 0;

	if ( vecAbsNormal.x <= vecAbsNormal.y )
		iDominantAxis = ( vecAbsNormal.z > vecAbsNormal.y ) + 1;
	else if ( vecAbsNormal.z > vecAbsNormal.x )
		iDominantAxis = 2;

	TINT iAxis1 = ( iDominantAxis == 0 );
	TINT iAxis2 = ( iDominantAxis != 2 ) + 1;

	TBOOL bHit = TFALSE;

	for ( TINT i = 0; i < 3; i++ )
	{
		const TVector4& rcVertex     = *a_ppVertices[ i ];
		const TVector4& rcNextVertex = *a_ppVertices[ ( i == 2 ) ? 0 : i + 1 ];

		TFLOAT fVertexDist = vecNormal.y * rcVertex.y + vecNormal.x * rcVertex.x + vecNormal.z * rcVertex.z;
		TFLOAT fNextDist   = vecNormal.y * rcNextVertex.y + vecNormal.x * rcNextVertex.x + vecNormal.z * rcNextVertex.z;

		// The edge has to cross the plane
		if ( ( fNextDist - fPlaneD ) * ( fVertexDist - fPlaneD ) >= 0.0f )
			continue;

		const TVector4& rcEdgeDir = *a_ppEdgeDirs[ i ];
		TFLOAT          fEdgeDot  = rcEdgeDir.y * vecNormal.y + rcEdgeDir.x * vecNormal.x + rcEdgeDir.z * vecNormal.z;

		if ( fEdgeDot > -TMath::FLOATEPSILON && fEdgeDot < TMath::FLOATEPSILON )
			continue;

		// Where the edge crosses the plane
		TVector4 vecOffset;
		vecOffset.Multiply( rcEdgeDir, ( fPlaneD - fVertexDist ) / fEdgeDot );

		TVector4 vecPoint = rcVertex;
		vecPoint.x += vecOffset.x;
		vecPoint.y += vecOffset.y;
		vecPoint.z += vecOffset.z;

		const TFLOAT* pPoint   = &vecPoint.x;
		const TFLOAT* pStart   = &a_rcStart.x;
		const TFLOAT* pSegment = &vecSegment.x;
		const TFLOAT* pDir     = &a_rcDirection.x;

		TFLOAT fDist = ( ( pPoint[ iAxis2 ] - pStart[ iAxis2 ] ) * pSegment[ iAxis1 ] - ( pPoint[ iAxis1 ] - pStart[ iAxis1 ] ) * pSegment[ iAxis2 ] ) /
		    ( pDir[ iAxis2 ] * pSegment[ iAxis1 ] - pDir[ iAxis1 ] * pSegment[ iAxis2 ] );

		if ( fDist < 0.0f )
			continue;

		// The point moved back by fDist has to be on the segment
		TVector4 vecMove;
		vecMove.Multiply( a_rcDirection, fDist );

		TFLOAT fOnSegmentX = vecPoint.x - vecMove.x;
		TFLOAT fOnSegmentY = vecPoint.y - vecMove.y;
		TFLOAT fOnSegmentZ = vecPoint.z - vecMove.z;

		if ( ( a_rcEnd.y - fOnSegmentY ) * ( a_rcStart.y - fOnSegmentY ) + ( a_rcEnd.x - fOnSegmentX ) * ( a_rcStart.x - fOnSegmentX ) + ( a_rcStart.z - fOnSegmentZ ) * ( a_rcEnd.z - fOnSegmentZ ) >= 0.0f ||
		     a_rfDistance < fDist )
			continue;

		a_rfDistance = fDist;
		bHit         = TTRUE;
	}

	return bHit;
}

// $deBlob: FUNCTION 006689e0
void TMath::GetSweptSegmentTriangleEdgeDirs( const TVector4* const* a_ppVertices, TVector4* a_pOutEdgeDirs )
{
	for ( TINT i = 0; i < 3; i++ )
	{
		const TVector4& rcNext = *a_ppVertices[ ( i == 2 ) ? 0 : i + 1 ];
		const TVector4& rcThis = *a_ppVertices[ i ];

		a_pOutEdgeDirs[ i ].Set( rcNext.x - rcThis.x, rcNext.y - rcThis.y, rcNext.z - rcThis.z, rcNext.w );
		a_pOutEdgeDirs[ i ].Normalise();
	}
}

// The corners of the unit box, then its 12 edges (start, end); scaled by the half extents
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

// $deBlob: FUNCTION 00668a50
TBOOL TMath::SweptAABBVsTriangle( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, const TVector4* const* a_ppVertices, TFLOAT& a_rfDistance )
{
	const TVector4& rcVertex0 = *a_ppVertices[ 0 ];

	TVector4 vecEdge1 = *a_ppVertices[ 1 ] - rcVertex0;
	vecEdge1.w        = a_ppVertices[ 1 ]->w;
	TVector4 vecEdge2 = *a_ppVertices[ 2 ] - rcVertex0;
	vecEdge2.w        = a_ppVertices[ 2 ]->w;

	TVector4 vecNormal( 0.0f, 0.0f, 0.0f, 1.0f );
	vecNormal.CrossProduct( vecEdge1, vecEdge2 );
	vecNormal.Normalise();

	// Moving away from the triangle
	if ( vecNormal.y * a_rcDirection.y + a_rcDirection.x * vecNormal.x + a_rcDirection.z * vecNormal.z >= 0.0f )
		return TFALSE;

	TBOOL bHit = TFALSE;

	// The leading corners of the box against the triangle
	for ( TINT i = 0; i < TARRAYSIZE( s_avCornerDims ); i++ )
	{
		const TVector4& rcCorner = s_avCornerDims[ i ];

		if ( rcCorner.y * a_rcDirection.y + a_rcDirection.x * rcCorner.x + a_rcDirection.z * rcCorner.z < 0.0f )
			continue;

		TVector4 vecCorner = rcCorner;
		vecCorner.Multiply( a_rcHalfExtents );
		vecCorner.x = a_rcCenter.x + vecCorner.x;
		vecCorner.y = a_rcCenter.y + vecCorner.y;
		vecCorner.z = a_rcCenter.z + vecCorner.z;

		TFLOAT fT;

		if ( RayVsTriangle( vecCorner, a_rcDirection, a_ppVertices, fT ) && -TMath::FLOATEPSILON < fT && fT < a_rfDistance )
		{
			bHit         = TTRUE;
			a_rfDistance = fT;

			if ( fT <= 0.0f )
				return bHit;
		}
	}

	// The vertices of the triangle moving the other way against the box (slabs)
	TVector4 vecBackDir( -a_rcDirection.x, -a_rcDirection.y, -a_rcDirection.z, a_rcDirection.w );

	for ( TINT i = 0; i < 3; i++ )
	{
		const TVector4 vecVertex = *a_ppVertices[ i ];

		TINT   iFace = -1;
		TFLOAT fNear = -1e10f;
		TFLOAT fFar  = TMath::MAXFLOAT;
		TBOOL  bMiss = TFALSE;

		for ( TINT k = 0; k < 3; k++ )
		{
			TFLOAT fMin = ( &a_rcCenter.x )[ k ] - ( &a_rcHalfExtents.x )[ k ];
			TFLOAT fMax = ( &a_rcHalfExtents.x )[ k ] + ( &a_rcCenter.x )[ k ];
			TFLOAT fDir = ( &vecBackDir.x )[ k ];
			TFLOAT fPos = ( &vecVertex.x )[ k ];

			if ( fDir <= -TMath::FLOATEPSILON || TMath::FLOATEPSILON <= fDir )
			{
				TFLOAT fSlabNear = ( fMin - fPos ) * ( 1.0f / fDir );
				TFLOAT fSlabFar  = ( fMax - fPos ) * ( 1.0f / fDir );
				TINT   iSlabFace = k;

				if ( fSlabFar < fSlabNear )
				{
					TFLOAT fTemp = fSlabNear;
					fSlabNear    = fSlabFar;
					fSlabFar     = fTemp;
					iSlabFace    = k + 3;
				}

				if ( fNear < fSlabNear )
				{
					iFace = iSlabFace;
					fNear = fSlabNear;
				}

				if ( fSlabFar < fFar )
					fFar = fSlabFar;

				if ( fFar < fNear || fFar < TMath::FLOATEPSILON )
				{
					bMiss = TTRUE;
					break;
				}
			}
			else if ( fPos < fMin || fMax < fPos )
			{
				bMiss = TTRUE;
				break;
			}
		}

		if ( bMiss )
			continue;

		if ( fNear <= fFar && TMath::FLOATEPSILON <= fFar && iFace != -1 && -TMath::FLOATEPSILON < fNear && fNear < a_rfDistance )
		{
			a_rfDistance = fNear;
			bHit         = TTRUE;

			if ( fNear <= 0.0f )
				return bHit;
		}
	}

	// The leading edges of the box against the triangle
	TVector4 aEdgeDirs[ 3 ];
	GetSweptSegmentTriangleEdgeDirs( a_ppVertices, aEdgeDirs );

	const TVector4* apEdgeDirs[ 3 ] = { &aEdgeDirs[ 0 ], &aEdgeDirs[ 1 ], &aEdgeDirs[ 2 ] };

	// NOTE: retail initialises this static and never reads it
	static TFLOAT s_fOneOverSqrt8 = 1.0f / TFLOAT( sqrt( 8.0 ) );
	(void)s_fOneOverSqrt8;

	for ( TINT i = 0; i < TARRAYSIZE( s_avEdgeDims ); i++ )
	{
		const TVector4& rcEdgeStart = s_avEdgeDims[ i ][ 0 ];
		const TVector4& rcEdgeEnd   = s_avEdgeDims[ i ][ 1 ];

		if ( a_rcDirection.x * ( rcEdgeStart.x + rcEdgeEnd.x ) + ( rcEdgeStart.y + rcEdgeEnd.y ) * a_rcDirection.y + a_rcDirection.z * ( rcEdgeStart.z + rcEdgeEnd.z ) < 0.0f )
			continue;

		TVector4 vecStart;
		vecStart.Multiply( rcEdgeStart, a_rcHalfExtents );
		TVector4 vecEnd;
		vecEnd.Multiply( rcEdgeEnd, a_rcHalfExtents );

		vecStart.x += a_rcCenter.x;
		vecStart.y += a_rcCenter.y;
		vecStart.z += a_rcCenter.z;
		vecEnd.x += a_rcCenter.x;
		vecEnd.y += a_rcCenter.y;
		vecEnd.z += a_rcCenter.z;

		// The axis of the edge
		TVector4 vecEdge( rcEdgeEnd.x - rcEdgeStart.x, rcEdgeEnd.y - rcEdgeStart.y, rcEdgeEnd.z - rcEdgeStart.z, rcEdgeEnd.w );
		vecEdge.Abs4();

		TINT iAxis = 0;

		if ( vecEdge.x <= vecEdge.y )
			iAxis = ( vecEdge.y < vecEdge.z ) + 1;
		else if ( vecEdge.x < vecEdge.z )
			iAxis = 2;

		if ( SweptSegmentVsTriangle( vecStart, vecEnd, ( &a_rcHalfExtents.x )[ iAxis ] * 2.0f, a_rcDirection, a_ppVertices, apEdgeDirs, a_rfDistance ) )
			bHit = TTRUE;
	}

	return bHit;
}

// $deBlob: FUNCTION 0066c3e0
TFLOAT TMath::ClippedClosestPointOnLine( const TVector4& a_rcPoint, const TVector4& a_rcStart, const TVector4& a_rcEnd )
{
	TVector4 vToPoint = a_rcPoint - a_rcStart;
	TVector4 vLine    = a_rcEnd - a_rcStart;

	TFLOAT fDot    = vLine.x * vToPoint.x + vLine.y * vToPoint.y + vLine.z * vToPoint.z;
	TFLOAT fLength = vLine.Magnitude();
	TFLOAT fDist   = fDot / fLength;

	if ( fDist > fLength )
		return fLength;

	if ( fDist < 0.0f )
		return 0.0f;

	return fDist;
}

// $deBlob: FUNCTION 0066c5f0
void TMath::ClosestPointsBetweenLineSegments( const TVector4& a_rcStart1, const TVector4& a_rcEnd1, const TVector4& a_rcStart2, const TVector4& a_rcEnd2, TVector4& a_rOutPoint1, TVector4& a_rOutPoint2 )
{
	TVector4 vecSeg1 = a_rcEnd1 - a_rcStart1;
	TVector4 vecSeg2 = a_rcEnd2 - a_rcStart2;

	// Start1 -> Start2
	TVector4 vecS1S2 = a_rcStart2 - a_rcStart1;
	TFLOAT   fS1S2_1 = vecS1S2.y * vecSeg1.y + vecS1S2.x * vecSeg1.x + vecS1S2.z * vecSeg1.z;
	TFLOAT   fS1S2_2 = vecS1S2.x * vecSeg2.x + vecS1S2.y * vecSeg2.y + vecS1S2.z * vecSeg2.z;

	if ( 0.0f < fS1S2_1 || fS1S2_2 < 0.0f )
	{
		// Start1 -> End2
		TVector4 vecS1E2 = a_rcEnd2 - a_rcStart1;
		TFLOAT   fS1E2_1 = vecS1E2.y * vecSeg1.y + vecS1E2.x * vecSeg1.x + vecS1E2.z * vecSeg1.z;
		TFLOAT   fS1E2_2 = vecS1E2.x * vecSeg2.x + vecS1E2.y * vecSeg2.y + vecS1E2.z * vecSeg2.z;

		if ( fS1E2_1 <= 0.0f && fS1E2_2 <= 0.0f )
		{
			a_rOutPoint1 = a_rcStart1;
			a_rOutPoint2 = a_rcEnd2;
			return;
		}

		// End1 -> Start2
		TVector4 vecE1S2 = a_rcStart2 - a_rcEnd1;
		TFLOAT   fE1S2_1 = vecE1S2.y * vecSeg1.y + vecE1S2.x * vecSeg1.x + vecE1S2.z * vecSeg1.z;
		TFLOAT   fE1S2_2 = vecE1S2.x * vecSeg2.x + vecE1S2.y * vecSeg2.y + vecE1S2.z * vecSeg2.z;

		if ( 0.0f <= fE1S2_1 && 0.0f <= fE1S2_2 )
		{
			a_rOutPoint1 = a_rcEnd1;
			a_rOutPoint2 = a_rcStart2;
			return;
		}

		// End1 -> End2
		TVector4 vecE1E2 = a_rcEnd2 - a_rcEnd1;
		TFLOAT   fE1E2_1 = vecE1E2.y * vecSeg1.y + vecE1E2.x * vecSeg1.x + vecE1E2.z * vecSeg1.z;
		TFLOAT   fE1E2_2 = vecE1E2.x * vecSeg2.x + vecE1E2.y * vecSeg2.y + vecE1E2.z * vecSeg2.z;

		if ( 0.0f <= fE1E2_1 && fE1E2_2 <= 0.0f )
		{
			a_rOutPoint1 = a_rcEnd1;
			a_rOutPoint2 = a_rcEnd2;
			return;
		}

		// An end point of the second segment against the first one
		TFLOAT fSeg1LengthSq = vecSeg1.y * vecSeg1.y + vecSeg1.x * vecSeg1.x + vecSeg1.z * vecSeg1.z;

		if ( 0.0f <= fS1S2_1 && fE1S2_1 <= 0.0f )
		{
			TFLOAT   fT = fS1S2_1 / fSeg1LengthSq;
			TVector4 vecOffset;
			vecOffset.Multiply( vecSeg1, fT );

			if ( 0.0f <= ( vecS1S2.x - vecOffset.x ) * vecSeg2.x + ( vecS1S2.y - vecOffset.y ) * vecSeg2.y + ( vecS1S2.z - vecOffset.z ) * vecSeg2.z )
			{
				vecOffset.Multiply( vecSeg1, fT );
				a_rOutPoint1.Set( a_rcStart1.x + vecOffset.x, a_rcStart1.y + vecOffset.y, vecOffset.z + a_rcStart1.z, a_rcStart1.w );
				a_rOutPoint2 = a_rcStart2;
				return;
			}
		}

		if ( 0.0f <= fS1E2_1 && fE1E2_1 <= 0.0f )
		{
			TFLOAT   fT = fS1E2_1 / fSeg1LengthSq;
			TVector4 vecOffset;
			vecOffset.Multiply( vecSeg1, fT );

			if ( ( vecS1E2.x - vecOffset.x ) * vecSeg2.x + ( vecS1E2.y - vecOffset.y ) * vecSeg2.y + ( vecS1E2.z - vecOffset.z ) * vecSeg2.z <= 0.0f )
			{
				vecOffset.Multiply( vecSeg1, fT );
				a_rOutPoint1.Set( a_rcStart1.x + vecOffset.x, a_rcStart1.y + vecOffset.y, vecOffset.z + a_rcStart1.z, a_rcStart1.w );
				a_rOutPoint2 = a_rcEnd2;
				return;
			}
		}

		// An end point of the first segment against the second one
		TFLOAT fSeg2LengthSq = vecSeg2.x * vecSeg2.x + vecSeg2.y * vecSeg2.y + vecSeg2.z * vecSeg2.z;

		if ( fS1S2_2 <= 0.0f && 0.0f <= fS1E2_2 )
		{
			TFLOAT   fT = fS1S2_2 / fSeg2LengthSq;
			TVector4 vecOffset;
			vecOffset.Multiply( vecSeg2, -fT );
			TVector4 vecS2S1;
			vecS2S1.Multiply( vecS1S2, -1.0f );

			if ( 0.0f <= ( vecS2S1.y - vecOffset.y ) * vecSeg1.y + ( vecS2S1.x - vecOffset.x ) * vecSeg1.x + ( vecS2S1.z - vecOffset.z ) * vecSeg1.z )
			{
				a_rOutPoint1 = a_rcStart1;
				vecOffset.Multiply( vecSeg2, -fT );
				a_rOutPoint2.Set( a_rcStart2.x + vecOffset.x, vecOffset.y + a_rcStart2.y, a_rcStart2.z + vecOffset.z, a_rcStart2.w );
				return;
			}
		}

		if ( fE1S2_2 <= 0.0f && 0.0f <= fE1E2_2 )
		{
			TFLOAT   fT = -( fE1S2_2 / fSeg2LengthSq );
			TVector4 vecOffset;
			vecOffset.Multiply( vecSeg2, fT );
			TVector4 vecS2E1;
			vecS2E1.Multiply( vecE1S2, -1.0f );

			if ( ( vecS2E1.y - vecOffset.y ) * vecSeg1.y + ( vecS2E1.x - vecOffset.x ) * vecSeg1.x + ( vecS2E1.z - vecOffset.z ) * vecSeg1.z <= 0.0f )
			{
				a_rOutPoint1 = a_rcEnd1;
				vecOffset.Multiply( vecSeg2, fT );
				a_rOutPoint2.Set( a_rcStart2.x + vecOffset.x, vecOffset.y + a_rcStart2.y, a_rcStart2.z + vecOffset.z, a_rcStart2.w );
				return;
			}
		}

		// The closest points are inside of both segments
		TFLOAT fSeg2DotSeg1 = vecSeg2.x * vecSeg1.x + vecSeg2.y * vecSeg1.y + vecSeg2.z * vecSeg1.z;
		TFLOAT fDenom       = fSeg2LengthSq * fSeg1LengthSq - fSeg2DotSeg1 * fSeg2DotSeg1;

		if ( 0.0f < fDenom )
		{
			fDenom = 1.0f / fDenom;

			TVector4 vecOffset;
			vecOffset.Multiply( vecSeg1, ( fSeg2LengthSq * fS1S2_1 - fSeg2DotSeg1 * fS1S2_2 ) * fDenom );
			a_rOutPoint1.Set( a_rcStart1.x + vecOffset.x, a_rcStart1.y + vecOffset.y, vecOffset.z + a_rcStart1.z, a_rcStart1.w );

			vecOffset.Multiply( vecSeg2, ( fSeg2DotSeg1 * fS1S2_1 - fSeg1LengthSq * fS1S2_2 ) * fDenom );
			a_rOutPoint2.Set( a_rcStart2.x + vecOffset.x, vecOffset.y + a_rcStart2.y, a_rcStart2.z + vecOffset.z, a_rcStart2.w );
			return;
		}
	}

	a_rOutPoint1 = a_rcStart1;
	a_rOutPoint2 = a_rcStart2;
}

// $deBlob: FUNCTION 0066d120
TBOOL TMath::LineIntersectsPlane3D( const TPlane& a_rcPlane, const TVector3& a_rcStart, const TVector3& a_rcEnd, TVector3& a_rOut )
{
	const TVector3& vNormal = a_rcPlane.GetNormal();
	TFLOAT          fD      = a_rcPlane.GetD();

	TVector3 vToPlane( a_rcStart.x - vNormal.x * fD, a_rcStart.y - vNormal.y * fD, a_rcStart.z - vNormal.z * fD );
	TVector3 vLine( a_rcEnd.x - a_rcStart.x, a_rcEnd.y - a_rcStart.y, a_rcEnd.z - a_rcStart.z );

	TFLOAT fDenom = vNormal.y * vLine.y + vNormal.x * vLine.x + vNormal.z * vLine.z;

	if ( TMath::Abs( fDenom ) < TMath::TFLOAT_EPSILON )
		return TFALSE;

	TFLOAT fT      = -( vNormal.y * vToPlane.y + vNormal.x * vToPlane.x + vNormal.z * vToPlane.z ) / fDenom;
	TFLOAT fLength = vLine.Magnitude();

	if ( fT < 0.0f || fLength * fT > fLength )
		return TFALSE;

	a_rOut.x = a_rcStart.x + vLine.x * fT;
	a_rOut.y = a_rcStart.y + vLine.y * fT;
	a_rOut.z = a_rcStart.z + vLine.z * fT;
	return TTRUE;
}

// $deBlob: FUNCTION 0066cf40
TBOOL __stdcall TMath::IsPointInFrontOfPlane( const TVector3& a_rcNormal, const TVector3& a_rcPlanePoint, const TVector3& a_rcPoint )
{
	return a_rcNormal.y * ( a_rcPoint.y - a_rcPlanePoint.y ) + a_rcNormal.x * ( a_rcPoint.x - a_rcPlanePoint.x ) + a_rcNormal.z * ( a_rcPoint.z - a_rcPlanePoint.z ) >= 0.0f;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// Whether two spheres overlap; a_pOutDistanceSq gets the squared distance minus the second radius squared, a_pOutPush
// the push out of the second sphere (from the first) and a_pOutPenetration the overlap
// $deBlob: FUNCTION 006677e0
TBOOL TMath::SphereIntersection( const TVector3& a_rcCentre1, TFLOAT a_fRadius1, const TVector3& a_rcCentre2, TFLOAT a_fRadius2, TFLOAT* a_pOutDistanceSq, TVector3* a_pOutPush, TFLOAT* a_pOutPenetration )
{
	TFLOAT fPenetration = 0.0f;
	TFLOAT fDX          = a_rcCentre2.x - a_rcCentre1.x;
	TFLOAT fRadius      = a_fRadius1 + a_fRadius2;
	TFLOAT fDY          = a_rcCentre2.y - a_rcCentre1.y;
	TFLOAT fDZ          = a_rcCentre2.z - a_rcCentre1.z;
	TFLOAT fDistanceSq  = fDX * fDX + fDY * fDY + fDZ * fDZ;

	if ( fDistanceSq > fRadius * fRadius )
		return TFALSE;

	if ( a_pOutPenetration )
	{
		fPenetration       = fRadius - TMath::Sqrt( fDistanceSq );
		*a_pOutPenetration = fPenetration;
	}

	if ( a_pOutPush )
	{
		if ( !a_pOutPenetration )
			fPenetration = fRadius - TMath::Sqrt( fDistanceSq );

		TFLOAT fScale = fPenetration / fRadius;
		a_pOutPush->x = fDX * fScale;
		a_pOutPush->y = fDY * fScale;
		a_pOutPush->z = fDZ * fScale;
	}

	if ( a_pOutDistanceSq )
		*a_pOutDistanceSq = fDistanceSq - a_fRadius2 * a_fRadius2;

	return TTRUE;
}

// SphereIntersection in the XZ plane (the push has no y)
// $deBlob: FUNCTION 00667920
TBOOL TMath::SphereIntersectionPlanar( const TVector3& a_rcCentre1, TFLOAT a_fRadius1, const TVector3& a_rcCentre2, TFLOAT a_fRadius2, TFLOAT* a_pOutDistanceSq, TVector3* a_pOutPush, TFLOAT* a_pOutPenetration )
{
	TFLOAT fPenetration = 0.0f;
	TFLOAT fDZ          = a_rcCentre2.z - a_rcCentre1.z;
	TFLOAT fRadius      = a_fRadius1 + a_fRadius2;
	TFLOAT fDX          = a_rcCentre2.x - a_rcCentre1.x;
	TFLOAT fDistanceSq  = fDZ * fDZ + fDX * fDX;

	if ( fDistanceSq > fRadius * fRadius )
		return TFALSE;

	if ( a_pOutPenetration )
	{
		fPenetration       = fRadius - TMath::Sqrt( fDistanceSq );
		*a_pOutPenetration = fPenetration;
	}

	if ( a_pOutPush )
	{
		if ( !a_pOutPenetration )
			fPenetration = fRadius - TMath::Sqrt( fDistanceSq );

		TFLOAT fScale = fPenetration / fRadius;
		a_pOutPush->y = 0.0f;
		a_pOutPush->x = fDX * fScale;
		a_pOutPush->z = fDZ * fScale;
	}

	if ( a_pOutDistanceSq )
		*a_pOutDistanceSq = fDistanceSq - a_fRadius2 * a_fRadius2;

	return TTRUE;
}

// When two moving spheres meet: 0 if they already overlap, -1 if they never do, else the time (in units of their
// velocities)
// $deBlob: FUNCTION 00667a40
TFLOAT TMath::SphereIntersectionDynamic( const TVector3& a_rcPosA, TFLOAT a_fRadA, const TVector3& a_rcVelA, const TVector3& a_rcPosB, TFLOAT a_fRadB, const TVector3& a_rcVelB )
{
	TASSERT( a_fRadA > TFLOAT_EPSILON );
	TASSERT( a_fRadB > TFLOAT_EPSILON );

	TVector3 vVelocity( a_rcVelA.x - a_rcVelB.x, a_rcVelA.y - a_rcVelB.y, a_rcVelA.z - a_rcVelB.z );
	TFLOAT   fSpeed = vVelocity.Magnitude();

	TFLOAT fDirX = ( 1.0f / fSpeed ) * vVelocity.x;
	TFLOAT fDirZ = ( 1.0f / fSpeed ) * vVelocity.z;
	TFLOAT fDirY = ( 1.0f / fSpeed ) * vVelocity.y;

	TVector3 vOffset( a_rcPosB.x - a_rcPosA.x, a_rcPosB.y - a_rcPosA.y, a_rcPosB.z - a_rcPosA.z );
	TFLOAT   fDistance = vOffset.Magnitude();

	if ( a_fRadA + a_fRadB >= fDistance )
		return 0.0f;

	if ( fSpeed < TFLOAT_EPSILON )
		return -1.0f;

	if ( vOffset.x * vVelocity.x + vOffset.y * vVelocity.y + vOffset.z * vVelocity.z < 0.0f )
		return -1.0f;

	TFLOAT fRadiusSq = ( a_fRadA + a_fRadB ) * ( a_fRadA + a_fRadB );
	TFLOAT fAlong    = vOffset.y * fDirY + vOffset.x * fDirX + vOffset.z * fDirZ;
	TFLOAT fSideSq   = fDistance * fDistance - fAlong * fAlong;

	if ( fSideSq > fRadiusSq || fRadiusSq - fSideSq < 0.0f )
		return -1.0f;

	return ( fAlong - TMath::Sqrt( fRadiusSq - fSideSq ) ) / fSpeed;
}

// Whether the sphere is inside the planes (TFALSE when it's fully behind one); a_rbIntersects tells if it crosses one
// $deBlob: FUNCTION 00667c70
TBOOL TMath::SphereInsideFrustum( const TPlane* a_pPlanes, TINT a_iNumPlanes, const TSphere& a_rcSphere, TBOOL& a_rbIntersects )
{
	a_rbIntersects = TFALSE;

	for ( TINT i = 0; i < a_iNumPlanes; i++ )
	{
		const TVector3& rcNormal  = a_pPlanes[ i ].GetNormal();
		const TVector3& rcOrigin  = a_rcSphere.GetOrigin();
		TFLOAT          fDistance = ( rcNormal.x * rcOrigin.x + rcNormal.y * rcOrigin.y + rcOrigin.z * rcNormal.z ) - a_pPlanes[ i ].GetD();

		if ( -a_rcSphere.GetRadius() > fDistance )
			return TFALSE;

		if ( a_rcSphere.GetRadius() > fDistance )
			a_rbIntersects = TTRUE;
	}

	return TTRUE;
}

// The segment a_rcStart1 - a_rcEnd1 moving along a_rcDirection against the segment a_rcStart2 - a_rcEnd2: TTRUE if they
// meet (a_rfDistance: how far along a_rcDirection; a_bAllowNegative: also behind)
// $deBlob: FUNCTION 00668170
TBOOL TMath::SweptSegmentVsSegment( const TVector3& a_rcStart1, const TVector3& a_rcEnd1, const TVector3& a_rcDirection, const TVector3& a_rcStart2, const TVector3& a_rcEnd2, TFLOAT& a_rfDistance, TBOOL a_bAllowNegative )
{
	TVector3 vEdge2( a_rcEnd2.x - a_rcStart2.x, a_rcEnd2.y - a_rcStart2.y, a_rcEnd2.z - a_rcStart2.z );
	TFLOAT   afEdge1[ 3 ] = { a_rcEnd1.x - a_rcStart1.x, a_rcEnd1.y - a_rcStart1.y, a_rcEnd1.z - a_rcStart1.z };

	// The plane the first segment sweeps
	TVector3 vNormal(
	    a_rcDirection.z * afEdge1[ 1 ] - a_rcDirection.y * afEdge1[ 2 ],
	    a_rcDirection.x * afEdge1[ 2 ] - a_rcDirection.z * afEdge1[ 0 ],
	    a_rcDirection.y * afEdge1[ 0 ] - a_rcDirection.x * afEdge1[ 1 ]
	);
	vNormal.Normalize();

	TFLOAT fPlane = a_rcStart1.x * vNormal.x + a_rcStart1.y * vNormal.y + a_rcStart1.z * vNormal.z;

	// The second segment has to cross it
	if ( ( ( a_rcEnd2.x * vNormal.x + vNormal.y * a_rcEnd2.y + a_rcEnd2.z * vNormal.z ) - fPlane ) * ( ( a_rcStart2.x * vNormal.x + a_rcStart2.y * vNormal.y + vNormal.z * a_rcStart2.z ) - fPlane ) >= 0.0f )
		return TFALSE;

	vEdge2.Normalize();

	TFLOAT fDot = vEdge2.y * vNormal.y + vEdge2.x * vNormal.x + vEdge2.z * vNormal.z;

	if ( fDot > -TFLOAT_EPSILON && fDot < TFLOAT_EPSILON )
		return TFALSE;

	TFLOAT fT         = ( fPlane - ( a_rcStart2.x * vNormal.x + a_rcStart2.y * vNormal.y + vNormal.z * a_rcStart2.z ) ) / fDot;
	TFLOAT afHit[ 3 ] = { fT * vEdge2.x + a_rcStart2.x, vEdge2.y * fT + a_rcStart2.y, vEdge2.z * fT + a_rcStart2.z };

	// Solved in the plane of the two largest axes of the normal
	vNormal.Abs();

	TINT iMajor = ( vNormal.x <= vNormal.y ) ? 1 : 0;
	if ( vNormal.z > ( &vNormal.x )[ iMajor ] )
		iMajor = 2;

	TINT iU, iV;
	if ( iMajor == 0 )
	{
		iV = 1;
		iU = 2;
	}
	else if ( iMajor == 1 )
	{
		iV = 0;
		iU = 2;
	}
	else
	{
		iV = 0;
		iU = 1;
	}

	const TFLOAT* pStart1 = &a_rcStart1.x;
	const TFLOAT* pDir    = &a_rcDirection.x;

	TFLOAT fDistance = ( ( afHit[ iU ] - pStart1[ iU ] ) * afEdge1[ iV ] - ( afHit[ iV ] - pStart1[ iV ] ) * afEdge1[ iU ] ) /
	    ( pDir[ iU ] * afEdge1[ iV ] - pDir[ iV ] * afEdge1[ iU ] );

	if ( !a_bAllowNegative && fDistance < 0.0f )
		return TFALSE;

	// The meeting point has to be on the first segment
	TFLOAT fPX = afHit[ 0 ] - a_rcDirection.x * fDistance;
	TFLOAT fPY = afHit[ 1 ] - a_rcDirection.y * fDistance;
	TFLOAT fPZ = afHit[ 2 ] - a_rcDirection.z * fDistance;

	if ( ( a_rcStart1.x - fPX ) * ( a_rcEnd1.x - fPX ) + ( a_rcStart1.y - fPY ) * ( a_rcEnd1.y - fPY ) + ( a_rcStart1.z - fPZ ) * ( a_rcEnd1.z - fPZ ) >= 0.0f )
		return TFALSE;

	a_rfDistance = fDistance;
	return TTRUE;
}

// The box a_rcCenter1 +- a_rcHalfExtents1 moving along a_rcDirection against the box a_rcCenter2 +- a_rcHalfExtents2:
// TTRUE if they meet before a_rfDistance and the boxes don't stop overlapping before a_fMinExit (a_rfDistance: the
// distance, a_pOutNormal: the axis of the hit)
// $deBlob: FUNCTION 006690e0
TBOOL TMath::SweptAABBVsAABB( const TVector4& a_rcCenter1, const TVector4& a_rcHalfExtents1, const TVector4& a_rcDirection, const TVector4& a_rcCenter2, const TVector4& a_rcHalfExtents2, TFLOAT& a_rfDistance, TFLOAT a_fMinExit, TVector4* a_pOutNormal )
{
	TASSERT( a_rfDistance >= 0.0f );
	TASSERT( a_rfDistance <= 1e10f );

	TFLOAT afEnter[ 3 ];
	TFLOAT afExit[ 3 ];

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fDir     = ( &a_rcDirection.x )[ i ];
		TFLOAT fOffset  = ( &a_rcCenter2.x )[ i ] - ( &a_rcCenter1.x )[ i ];
		TFLOAT fExtents = ( &a_rcHalfExtents2.x )[ i ] + ( &a_rcHalfExtents1.x )[ i ];

		if ( fDir < -TFLOAT_EPSILON )
		{
			afEnter[ i ] = ( fExtents + fOffset ) * ( 1.0f / fDir );
			afExit[ i ]  = ( fOffset - fExtents ) * ( 1.0f / fDir );
		}
		else if ( fDir > TFLOAT_EPSILON )
		{
			afEnter[ i ] = ( fOffset - fExtents ) * ( 1.0f / fDir );
			afExit[ i ]  = ( fExtents + fOffset ) * ( 1.0f / fDir );
		}
		else
		{
			// Not moving on this axis: they have to overlap on it
			if ( fOffset - fExtents >= -0.0001f || fExtents + fOffset <= 0.0001f )
				return TFALSE;

			afEnter[ i ] = -1e10f;
			afExit[ i ]  = 1e10f;
		}
	}

	TINT iAxis;
	if ( afEnter[ 0 ] <= afEnter[ 1 ] )
		iAxis = ( afEnter[ 2 ] > afEnter[ 1 ] ) ? 2 : 1;
	else
		iAxis = ( afEnter[ 2 ] > afEnter[ 0 ] ) ? 2 : 0;

	TFLOAT fEnter = afEnter[ iAxis ];

	TFLOAT* pExit;
	TBOOL   bSmaller;
	if ( afExit[ 1 ] <= afExit[ 0 ] )
	{
		bSmaller = afExit[ 1 ] <= afExit[ 2 ];
		pExit    = &afExit[ 1 ];
	}
	else
	{
		bSmaller = afExit[ 0 ] <= afExit[ 2 ];
		pExit    = &afExit[ 0 ];
	}

	TFLOAT fExit = bSmaller ? *pExit : afExit[ 2 ];

	if ( fExit < fEnter )
		return TFALSE;

	if ( fExit < a_fMinExit )
		return TFALSE;

	if ( a_rfDistance < fEnter )
		return TFALSE;

	if ( a_pOutNormal )
	{
		*a_pOutNormal                 = TVector4::VEC_ZERO;
		( &a_pOutNormal->x )[ iAxis ] = -TMath::Sign( ( &a_rcDirection.x )[ iAxis ] );
	}

	a_rfDistance = ( fEnter <= 0.0f ) ? 0.0f : fEnter;
	return TTRUE;
}

// RayVsCircle3D with TVector3s
// $deBlob: FUNCTION 00669a70
TINT TMath::RayVsCircle3D( const TVector3& a_rcRayOrigin, const TVector3& a_rcRayDir, const TVector3& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfNearIntersection, TFLOAT& a_rfFarIntersection )
{
	TFLOAT fRadiusSq = a_fRadius * a_fRadius;
	TFLOAT fX        = a_rcCenter.x - a_rcRayOrigin.x;
	TFLOAT fY        = a_rcCenter.y - a_rcRayOrigin.y;
	TFLOAT fZ        = a_rcCenter.z - a_rcRayOrigin.z;
	TFLOAT fDistSq   = fY * fY + fX * fX + fZ * fZ;

	if ( fDistSq >= a_fRadius * a_fRadius )
	{
		TFLOAT fDot = a_rcRayDir.y * fY + a_rcRayDir.x * fX + a_rcRayDir.z * fZ;

		if ( fDot < 0.0f || fDistSq - fDot * fDot > fRadiusSq )
			return 0;
	}

	TFLOAT fB = ( a_rcRayDir.y * ( fY * -1.0f ) + a_rcRayDir.x * ( fX * -1.0f ) + a_rcRayDir.z * ( fZ * -1.0f ) ) * 2.0f;

	if ( fB * fB - ( fDistSq - fRadiusSq ) * 4.0f < 0.0f )
		return 0;

	TFLOAT fRoot         = TMath::Sqrt( fB * fB - ( fDistSq - fRadiusSq ) * 4.0f );
	a_rfNearIntersection = ( -fB - fRoot ) * 0.5f;
	a_rfFarIntersection  = ( fRoot - fB ) * 0.5f;
	return 2;
}

// RayVsCircle3D with a TSphere
// $deBlob: FUNCTION 00669d60
TINT TMath::RayVsCircle3D( const TVector4& a_rcRayOrigin, const TVector4& a_rcRayDir, const TSphere& a_rcSphere, TFLOAT& a_rfNearIntersection, TFLOAT& a_rfFarIntersection )
{
	TVector4 vToCenter( a_rcSphere.GetOrigin().x - a_rcRayOrigin.x, a_rcSphere.GetOrigin().y - a_rcRayOrigin.y, a_rcSphere.GetOrigin().z - a_rcRayOrigin.z, 1.0f );
	TFLOAT   fRadiusSq = a_rcSphere.GetRadius() * a_rcSphere.GetRadius();
	TFLOAT   fDistSq   = vToCenter.MagnitudeSq();

	if ( fDistSq >= fRadiusSq )
	{
		TFLOAT fDot = a_rcRayDir.y * vToCenter.y + a_rcRayDir.x * vToCenter.x + a_rcRayDir.z * vToCenter.z;

		if ( fDot < 0.0f || fDistSq - fDot * fDot > fRadiusSq )
			return 0;
	}

	vToCenter.Multiply( -1.0f );

	TFLOAT fB = ( a_rcRayDir.y * vToCenter.y + a_rcRayDir.x * vToCenter.x + a_rcRayDir.z * vToCenter.z ) * 2.0f;

	if ( fB * fB - ( fDistSq - fRadiusSq ) * 4.0f < 0.0f )
		return 0;

	TFLOAT fRoot         = TMath::Sqrt( fB * fB - ( fDistSq - fRadiusSq ) * 4.0f );
	a_rfNearIntersection = ( -fB - fRoot ) * 0.5f;
	a_rfFarIntersection  = ( fRoot - fB ) * 0.5f;
	return 2;
}

// The 2D ray a_rcOrigin along a_rcDirection against the line through a_rcLinePoint along a_rcLineDirection: TTRUE if it
// crosses it closer than a_rfDistance (a_rfDistance: the ray parameter)
// $deBlob: FUNCTION 00669ef0
TBOOL TMath::RaySegmentVsLine2D( const TVector2& a_rcOrigin, const TVector2& a_rcDirection, const TVector2& a_rcLinePoint, const TVector2& a_rcLineDirection, TFLOAT& a_rfDistance )
{
	TFLOAT fDenom = a_rcDirection.x * a_rcLineDirection.y - a_rcDirection.y * a_rcLineDirection.x;

	if ( fDenom == 0.0f )
		return TFALSE;

	TFLOAT fT = ( a_rcLineDirection.x * ( a_rcOrigin.y - a_rcLinePoint.y ) - a_rcLineDirection.y * ( a_rcOrigin.x - a_rcLinePoint.x ) ) / fDenom;

	if ( a_rfDistance <= fT || fT < 0.0f )
		return TFALSE;

	a_rfDistance = fT;
	return TTRUE;
}

// The normal of the 2D capsule a_rcStart - a_rcEnd at a_rcPoint (away from the segment)
// $deBlob: FUNCTION 0066a5e0
void TMath::CalculateCapsuleNormal2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, const TVector2& a_rcPoint, TVector2& a_rOutNormal )
{
	TFLOAT fT = ClosestPointOnLine2D( a_rcStart, a_rcEnd, a_rcPoint );

	if ( fT < 0.0f )
	{
		a_rOutNormal.x = a_rcPoint.x - a_rcStart.x;
		a_rOutNormal.y = a_rcPoint.y - a_rcStart.y;
		a_rOutNormal.Normalize();
	}
	else if ( fT > 1.0f )
	{
		a_rOutNormal.x = a_rcPoint.x - a_rcEnd.x;
		a_rOutNormal.y = a_rcPoint.y - a_rcEnd.y;
		a_rOutNormal.Normalize();
	}
	else
	{
		a_rOutNormal.x = a_rcEnd.x - a_rcStart.x;
		a_rOutNormal.y = a_rcEnd.y - a_rcStart.y;
		a_rOutNormal.Normalize();

		// The perpendicular on the point's side
		TFLOAT fX      = a_rOutNormal.x;
		TFLOAT fY      = -a_rOutNormal.y;
		a_rOutNormal.y = fX;
		a_rOutNormal.x = fY;

		if ( fY * ( a_rcPoint.x - a_rcStart.x ) + ( a_rcPoint.y - a_rcStart.y ) * fX < 0.0f )
		{
			a_rOutNormal.x = -a_rOutNormal.x;
			a_rOutNormal.y = -fX;
		}
	}
}

// How far a_rcPoint has to move along a_rcDirection to leave the 2D capsule a_rcStart - a_rcEnd of a_fRadius (a_rcNormal:
// the side of the capsule the point is on): TFALSE if the point isn't inside (a_rfDistance: the distance, plus 0.01)
// $deBlob: FUNCTION 0066a8b0
TBOOL TMath::PushPointOutOfCapsule2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fRadius, const TVector2& a_rcNormal, const TVector2& a_rcDirection, const TVector2& a_rcPoint, TFLOAT& a_rfDistance )
{
	TFLOAT fBest = 999999.875f;
	TFLOAT fDX   = a_rcEnd.x - a_rcStart.x;
	TFLOAT fDY   = a_rcEnd.y - a_rcStart.y;
	TFLOAT fNear, fFar, fResult;

	a_rfDistance = 0.0f;

	if ( ( a_rcPoint.y - a_rcEnd.y ) * fDY + ( a_rcPoint.x - a_rcEnd.x ) * fDX > 0.0f )
	{
		// Past the end: the end's circle
		if ( ( a_rcPoint.x - a_rcEnd.x ) * ( a_rcPoint.x - a_rcEnd.x ) + ( a_rcPoint.y - a_rcEnd.y ) * ( a_rcPoint.y - a_rcEnd.y ) > a_fRadius * a_fRadius )
			return TFALSE;

		if ( RayVsCircle2D( a_rcPoint, a_rcDirection, a_rcEnd, a_fRadius, fNear, fFar ) == 0 )
			return TFALSE;

		fResult = ( fNear <= fFar ) ? fFar : fNear;
	}
	else if ( ( a_rcPoint.y - a_rcStart.y ) * fDY + ( a_rcPoint.x - a_rcStart.x ) * fDX < 0.0f )
	{
		// Before the start: the start's circle
		if ( ( a_rcPoint.y - a_rcStart.y ) * ( a_rcPoint.y - a_rcStart.y ) + ( a_rcPoint.x - a_rcStart.x ) * ( a_rcPoint.x - a_rcStart.x ) > a_fRadius * a_fRadius )
			return TFALSE;

		if ( RayVsCircle2D( a_rcPoint, a_rcDirection, a_rcStart, a_fRadius, fNear, fFar ) == 0 )
			return TFALSE;

		fResult = ( fNear <= fFar ) ? fFar : fNear;
	}
	else
	{
		// Along the segment: the side of the capsule
		TFLOAT fSideX = a_rcStart.x + a_rcNormal.x * a_fRadius;
		TFLOAT fSideY = a_rcStart.y + a_rcNormal.y * a_fRadius;

		if ( a_rcNormal.x * ( a_rcPoint.x - fSideX ) + a_rcNormal.y * ( a_rcPoint.y - fSideY ) >= 0.0f )
			return TFALSE;

		TFLOAT fSide  = 999999.875f;
		TFLOAT fDenom = a_rcDirection.x * fDY - a_rcDirection.y * fDX;

		if ( fDenom != 0.0f )
		{
			TFLOAT fT = ( ( a_rcPoint.y - fSideY ) * fDX - ( a_rcPoint.x - fSideX ) * fDY ) / fDenom;

			if ( fT < 999999.875f && fT >= 0.0f )
			{
				fSide = fT;
				fBest = fT;
			}
		}

		TFLOAT fDenom2 = a_rcDirection.x * a_rcNormal.y - a_rcDirection.y * a_rcNormal.x;
		TFLOAT fCap;

		if ( fDenom2 == 0.0f ||
		     ( fCap = ( a_rcNormal.x * ( a_rcPoint.y - a_rcStart.y ) - a_rcNormal.y * ( a_rcPoint.x - a_rcStart.x ) ) / fDenom2, fSide <= fCap ) ||
		     fCap < 0.0f )
		{
			if ( RaySegmentVsLine2D( a_rcPoint, a_rcDirection, a_rcEnd, a_rcNormal, fBest ) == TTRUE && RayVsCircle2D( a_rcPoint, a_rcDirection, a_rcEnd, a_fRadius, fNear, fFar ) > 0 )
				fResult = ( fNear <= fFar ) ? fFar : fNear;
			else
				fResult = fBest;
		}
		else if ( RayVsCircle2D( a_rcPoint, a_rcDirection, a_rcStart, a_fRadius, fNear, fFar ) > 0 )
		{
			fResult = ( fNear <= fFar ) ? fFar : fNear;
		}
		else
		{
			fResult = fCap;
		}
	}

	if ( fResult > 0.0f )
		fResult += 0.01f;

	a_rfDistance = fResult;
	return TTRUE;
}

// The point where the 2D lines cross
// $deBlob: FUNCTION 0066ada0
TBOOL TMath::LineVsLine2D( const TVector2& a_rcLine1Start, const TVector2& a_rcLine1End, const TVector2& a_rcLine2Start, const TVector2& a_rcLine2End, TVector2& a_rOutPoint )
{
	TFLOAT fLine2X = a_rcLine2End.x - a_rcLine2Start.x;
	TFLOAT fLine2Y = a_rcLine2End.y - a_rcLine2Start.y;
	TFLOAT fDenom  = ( a_rcLine1End.x - a_rcLine1Start.x ) * fLine2Y - ( a_rcLine1End.y - a_rcLine1Start.y ) * fLine2X;

	if ( fDenom == 0.0f )
		return TFALSE;

	TFLOAT fDiffX = a_rcLine1Start.x - a_rcLine2Start.x;

	a_rOutPoint.x = a_rcLine1End.x;
	a_rOutPoint.y = a_rcLine1End.y;
	TFLOAT fT     = ( ( a_rcLine1Start.y - a_rcLine2Start.y ) * fLine2X - fDiffX * fLine2Y ) / fDenom;
	a_rOutPoint.x = ( a_rOutPoint.x - a_rcLine1Start.x ) * fT + a_rcLine1Start.x;
	a_rOutPoint.y = ( a_rOutPoint.y - a_rcLine1Start.y ) * fT + a_rcLine1Start.y;
	return TTRUE;
}

// The point where the 2D segment a_rcStart1 - a_rcEnd1 crosses the line through a_rcStart2 and a_rcEnd2
// $deBlob: FUNCTION 0066b0d0
TBOOL TMath::LineSegmentVsLine2D( const TVector2& a_rcStart1, const TVector2& a_rcEnd1, const TVector2& a_rcStart2, const TVector2& a_rcEnd2, TVector2& a_rOutPoint )
{
	TFLOAT fLine2X = a_rcEnd2.x - a_rcStart2.x;
	TFLOAT fLine2Y = a_rcEnd2.y - a_rcStart2.y;
	TFLOAT fDenom  = ( a_rcEnd1.x - a_rcStart1.x ) * fLine2Y - ( a_rcEnd1.y - a_rcStart1.y ) * fLine2X;

	if ( fDenom == 0.0f )
		return TFALSE;

	TFLOAT fT = ( ( a_rcStart1.y - a_rcStart2.y ) * fLine2X - ( a_rcStart1.x - a_rcStart2.x ) * fLine2Y ) / fDenom;

	if ( fT < 0.0f || fT > 1.0f )
		return TFALSE;

	a_rOutPoint.x = a_rcEnd1.x;
	a_rOutPoint.y = a_rcEnd1.y;
	a_rOutPoint.x = ( a_rOutPoint.x - a_rcStart1.x ) * fT + a_rcStart1.x;
	a_rOutPoint.y = ( a_rOutPoint.y - a_rcStart1.y ) * fT + a_rcStart1.y;
	return TTRUE;
}

// The distance from a_rcPoint to the 2D segment a_rcStart - a_rcEnd
// $deBlob: FUNCTION 0066b2d0
TFLOAT TMath::DistancePointToLineSegment( const TVector2& a_rcPoint, const TVector2& a_rcStart, const TVector2& a_rcEnd )
{
	return TMath::Sqrt( DistancePointToLineSegmentSq( a_rcPoint, a_rcStart, a_rcEnd ) );
}

// The squared distance from a_rcPoint to the 2D segment a_rcStart - a_rcEnd
// $deBlob: FUNCTION 0066b3d0
TFLOAT TMath::DistancePointToLineSegmentSq( const TVector2& a_rcPoint, const TVector2& a_rcStart, const TVector2& a_rcEnd )
{
	TFLOAT fDX = a_rcEnd.x - a_rcStart.x;
	TFLOAT fDY = a_rcEnd.y - a_rcStart.y;
	TFLOAT fT  = ( fDY * ( a_rcPoint.y - a_rcStart.y ) + ( a_rcPoint.x - a_rcStart.x ) * fDX ) / ( fDY * fDY + fDX * fDX );

	if ( fT > 1.0f )
		fT = 1.0f;
	else if ( fT < 0.0f )
		fT = 0.0f;

	TFLOAT fY = ( a_rcStart.y + fDY * fT ) - a_rcPoint.y;
	TFLOAT fX = ( a_rcStart.x + fDX * fT ) - a_rcPoint.x;
	return fY * fY + fX * fX;
}

// The distance from a_rcPoint to the 2D line through a_rcStart and a_rcEnd
// $deBlob: FUNCTION 0066b4b0
TFLOAT TMath::DistancePointToLine( const TVector2& a_rcPoint, const TVector2& a_rcStart, const TVector2& a_rcEnd )
{
	return TMath::Sqrt( DistancePointToLineSq( a_rcPoint, a_rcStart, a_rcEnd ) );
}

// The squared distance from a_rcPoint to the 2D line through a_rcStart and a_rcEnd
// $deBlob: FUNCTION 0066b590
TFLOAT TMath::DistancePointToLineSq( const TVector2& a_rcPoint, const TVector2& a_rcStart, const TVector2& a_rcEnd )
{
	TFLOAT fDX = a_rcEnd.x - a_rcStart.x;
	TFLOAT fDY = a_rcEnd.y - a_rcStart.y;
	TFLOAT fT  = ( fDX * ( a_rcPoint.x - a_rcStart.x ) + fDY * ( a_rcPoint.y - a_rcStart.y ) ) / ( fDX * fDX + fDY * fDY );

	TFLOAT fY = ( a_rcStart.y + fDY * fT ) - a_rcPoint.y;
	TFLOAT fX = ( a_rcStart.x + fDX * fT ) - a_rcPoint.x;
	return fY * fY + fX * fX;
}

// The squared distance from a_rcPoint to the line through a_rcStart and a_rcEnd
// $deBlob: FUNCTION 0066b660
TFLOAT TMath::DistancePointToLineSq( const TVector3& a_rcPoint, const TVector3& a_rcStart, const TVector3& a_rcEnd )
{
	TFLOAT fDX = a_rcEnd.x - a_rcStart.x;
	TFLOAT fDY = a_rcEnd.y - a_rcStart.y;
	TFLOAT fDZ = a_rcEnd.z - a_rcStart.z;
	TFLOAT fT  = ( fDY * ( a_rcPoint.y - a_rcStart.y ) + fDX * ( a_rcPoint.x - a_rcStart.x ) + fDZ * ( a_rcPoint.z - a_rcStart.z ) ) / ( fDY * fDY + fDX * fDX + fDZ * fDZ );

	TFLOAT fY = ( a_rcStart.y + fDY * fT ) - a_rcPoint.y;
	TFLOAT fX = ( a_rcStart.x + fT * fDX ) - a_rcPoint.x;
	TFLOAT fZ = ( a_rcStart.z + fDZ * fT ) - a_rcPoint.z;
	return fY * fY + fX * fX + fZ * fZ;
}

// The parameter (not clamped) of the point of the line through a_rcStart and a_rcEnd closest to a_rcPoint
// $deBlob: FUNCTION 0066c330
TFLOAT TMath::ClosestPointOnLine( const TVector3& a_rcPoint, const TVector3& a_rcStart, const TVector3& a_rcEnd )
{
	TFLOAT fDY = a_rcEnd.y - a_rcStart.y;
	TFLOAT fDZ = a_rcEnd.z - a_rcStart.z;

	return ( fDY * ( a_rcPoint.y - a_rcStart.y ) + ( a_rcEnd.x - a_rcStart.x ) * ( a_rcPoint.x - a_rcStart.x ) + fDZ * ( a_rcPoint.z - a_rcStart.z ) ) /
	    ( fDY * fDY + ( a_rcEnd.x - a_rcStart.x ) * ( a_rcEnd.x - a_rcStart.x ) + fDZ * fDZ );
}

// When the point moving from a_rcPoint along a_rcVelocity is closest to the 2D line a_rcStart - a_rcEnd (in a_fTime units)
// $deBlob: FUNCTION 0066c4b0
TFLOAT TMath::ClosestPointOnLineOverTime2D( const TVector2& a_rcStart, const TVector2& a_rcEnd, const TVector2& a_rcVelocity, const TVector2& a_rcPoint, TFLOAT a_fTime )
{
	TFLOAT fDY          = a_rcEnd.y - a_rcStart.y;
	TFLOAT fDenominator = ( a_rcVelocity.y * -fDY + a_rcVelocity.x * -( a_rcEnd.x - a_rcStart.x ) + a_fTime ) * ( fDY * fDY + ( a_rcEnd.x - a_rcStart.x ) * ( a_rcEnd.x - a_rcStart.x ) );

	TASSERT( fDenominator != 0.0f );

	return ( ( a_rcStart.x * ( ( a_rcStart.x - a_rcPoint.x ) - a_rcEnd.x ) + ( ( a_rcStart.y - a_rcPoint.y ) - a_rcEnd.y ) * a_rcStart.y + ( a_rcEnd.y * a_rcPoint.y + a_rcEnd.x * a_rcPoint.x ) ) * a_fTime ) / fDenominator;
}

// Whether a_rcPoint is on the plane through a_rcPlanePoint (within 0.001)
// $deBlob: FUNCTION 0066cfb0
TBOOL TMath::PointOnPlane3D( const TVector3& a_rcNormal, const TVector3& a_rcPlanePoint, const TVector3& a_rcPoint )
{
	TFLOAT fDistance = a_rcNormal.y * ( a_rcPoint.y - a_rcPlanePoint.y ) + a_rcNormal.x * ( a_rcPoint.x - a_rcPlanePoint.x ) + a_rcNormal.z * ( a_rcPoint.z - a_rcPlanePoint.z );
	return fDistance > -0.001f && fDistance < 0.001f;
}

// Whether the segment a_rcStart - a_rcEnd crosses the plane
// $deBlob: FUNCTION 0066d040
TBOOL TMath::LineIntersectsPlane3D( const TPlane& a_rcPlane, const TVector3& a_rcStart, const TVector3& a_rcEnd )
{
	const TVector3& rcNormal = a_rcPlane.GetNormal();
	TFLOAT          fD       = a_rcPlane.GetD();

	TBOOL bStart = rcNormal.y * ( a_rcStart.y - rcNormal.y * fD ) + rcNormal.x * ( a_rcStart.x - rcNormal.x * fD ) + rcNormal.z * ( a_rcStart.z - rcNormal.z * fD ) >= 0.0f;
	TBOOL bEnd   = rcNormal.y * ( a_rcEnd.y - rcNormal.y * fD ) + rcNormal.x * ( a_rcEnd.x - rcNormal.x * fD ) + rcNormal.z * ( a_rcEnd.z - rcNormal.z * fD ) >= 0.0f;
	return bStart != bEnd;
}

// Turns the direction a_rcFrom towards a_rcTo by a_fMaxAngle at most (both normalised; a_rcFallbackAxis gives the turning
// axis when they're opposite)
// $deBlob: FUNCTION 0066d290
void TMath::GetAxisChangeLimit3D( const TVector3& a_rcFrom, const TVector3& a_rcTo, const TVector3& a_rcFallbackAxis, TFLOAT a_fMaxAngle, TVector3& a_rOut )
{
	TFLOAT fDot = a_rcTo.x * a_rcFrom.x + a_rcTo.y * a_rcFrom.y + a_rcTo.z * a_rcFrom.z;

	if ( a_fMaxAngle > -PI )
	{
		if ( !( PI > a_fMaxAngle ) )
			a_fMaxAngle = PI;
	}
	else
	{
		a_fMaxAngle = -PI;
	}

	TFLOAT fCos = TFLOAT( cos( a_fMaxAngle ) );

	if ( fCos <= fDot )
	{
		a_rOut = a_rcTo;
		return;
	}

	TFLOAT fSin = TFLOAT( sin( a_fMaxAngle ) );

	TVector3 vAxis(
	    a_rcFrom.z * a_rcTo.y - a_rcTo.z * a_rcFrom.y,
	    a_rcTo.z * a_rcFrom.x - a_rcFrom.z * a_rcTo.x,
	    a_rcTo.x * a_rcFrom.y - a_rcFrom.x * a_rcTo.y
	);

	TFLOAT fMagnitude = vAxis.Magnitude();

	if ( fMagnitude < 0.000001f )
	{
		// Opposite directions: turn around the fallback axis
		vAxis.Set(
		    a_rcFallbackAxis.y * a_rcFrom.z - a_rcFallbackAxis.z * a_rcFrom.y,
		    a_rcFrom.x * a_rcFallbackAxis.z - a_rcFallbackAxis.x * a_rcFrom.z,
		    a_rcFallbackAxis.x * a_rcFrom.y - a_rcFrom.x * a_rcFallbackAxis.y
		);

		TASSERT( vAxis.MagnitudeSq() >= 0.000001f );
		vAxis.Normalize();
	}
	else
	{
		vAxis.x *= 1.0f / fMagnitude;
		vAxis.y *= 1.0f / fMagnitude;
		vAxis.z *= 1.0f / fMagnitude;
	}

	// Rotates a_rcFrom around the axis
	TFLOAT fX = ( a_rcFrom.y * vAxis.z - a_rcFrom.z * vAxis.y ) * fSin;
	TFLOAT fZ = ( a_rcFrom.x * vAxis.y - a_rcFrom.y * vAxis.x ) * fSin;
	TFLOAT fY = ( a_rcFrom.z * vAxis.x - a_rcFrom.x * vAxis.z ) * fSin;

	a_rOut   = a_rcFrom;
	a_rOut.x = a_rOut.x * fCos + fX;
	a_rOut.y = fCos * a_rOut.y + fY;
	a_rOut.z = fCos * a_rOut.z + fZ;
}

// The signed angle from a_rcA to a_rcB
// $deBlob: FUNCTION 0066d860
TFLOAT TMath::AngleBetweenVectors2D( const TVector2& a_rcA, const TVector2& a_rcB )
{
	return TMath::ATan2( a_rcB.x * a_rcA.y - a_rcA.x * a_rcB.y, a_rcA.x * a_rcB.x + a_rcB.y * a_rcA.y );
}

// The signed angle from a_vA to a_vB in the XZ plane; both must be normalised
// $deBlob: FUNCTION 0066d910
TFLOAT TMath::AngleBetweenXZ( const TVector3& a_vA, const TVector3& a_vB )
{
	TASSERT( a_vA.IsNormalised() );
	TASSERT( a_vB.IsNormalised() );

	return -TMath::ATan2( a_vA.x * a_vB.z - a_vB.x * a_vA.z, a_vB.x * a_vA.x + a_vA.z * a_vB.z );
}

// HermitePosition with TVector3s
// $deBlob: FUNCTION 0066dea0
void TMath::HermitePosition( TVector3& a_rvOut, const TVector3& a_rvStart, const TVector3& a_rvStartTangent, const TVector3& a_rvEnd, const TVector3& a_rvEndTangent, TFLOAT a_fT )
{
	TVector4 vPowers( a_fT * a_fT * a_fT, a_fT * a_fT, a_fT, 1.0f );
	TVector4 vBasis;
	TMatrix44::TransformVector( vBasis, s_mHermite, vPowers );

	a_rvOut.x = a_rvStart.x * vBasis.x;
	a_rvOut.y = a_rvStart.y * vBasis.x;
	a_rvOut.z = a_rvStart.z * vBasis.x;
	a_rvOut.x += a_rvEnd.x * vBasis.y;
	a_rvOut.y += a_rvEnd.y * vBasis.y;
	a_rvOut.z += a_rvEnd.z * vBasis.y;
	a_rvOut.x += a_rvStartTangent.x * vBasis.z;
	a_rvOut.y += a_rvStartTangent.y * vBasis.z;
	a_rvOut.z += a_rvStartTangent.z * vBasis.z;
	a_rvOut.x += a_rvEndTangent.x * vBasis.w;
	a_rvOut.y += a_rvEndTangent.y * vBasis.w;
	a_rvOut.z += a_rvEndTangent.z * vBasis.w;
}

// HermiteTangent with TVector3s
// $deBlob: FUNCTION 0066dff0
void TMath::HermiteTangent( TVector3& a_rvOut, const TVector3& a_rvStart, const TVector3& a_rvStartTangent, const TVector3& a_rvEnd, const TVector3& a_rvEndTangent, TFLOAT a_fT )
{
	TVector4 vPowers( a_fT * a_fT * a_fT, a_fT * a_fT, a_fT, 1.0f );
	TVector4 vBasis;
	TMatrix44::TransformVector( vBasis, s_mHermiteTangent, vPowers );

	a_rvOut.x = a_rvStart.x * vBasis.x;
	a_rvOut.y = a_rvStart.y * vBasis.x;
	a_rvOut.z = a_rvStart.z * vBasis.x;
	a_rvOut.x += a_rvEnd.x * vBasis.y;
	a_rvOut.y += a_rvEnd.y * vBasis.y;
	a_rvOut.z += a_rvEnd.z * vBasis.y;
	a_rvOut.x += a_rvStartTangent.x * vBasis.z;
	a_rvOut.y += a_rvStartTangent.y * vBasis.z;
	a_rvOut.z += a_rvStartTangent.z * vBasis.z;
	a_rvOut.x += a_rvEndTangent.x * vBasis.w;
	a_rvOut.y += a_rvEndTangent.y * vBasis.w;
	a_rvOut.z += a_rvEndTangent.z * vBasis.w;
}

// The length of the Hermite curve, summed over 0.1 steps
// $deBlob: FUNCTION 0066e140
TFLOAT TMath::GetHermiteCurveLength( const TVector3& a_rvStart, const TVector3& a_rvStartTangent, const TVector3& a_rvEnd, const TVector3& a_rvEndTangent )
{
	TFLOAT   fT      = 0.0f;
	TFLOAT   fLength = 0.0f;
	TVector3 vPrevious( a_rvStart );

	do
	{
		TVector4 vPowers( fT * fT * fT, fT * fT, fT, 1.0f );
		TVector4 vBasis;
		TMatrix44::TransformVector( vBasis, s_mHermite, vPowers );

		TVector3 vPoint(
		    a_rvEndTangent.x * vBasis.w + ( a_rvStartTangent.x * vBasis.z + ( a_rvEnd.x * vBasis.y + a_rvStart.x * vBasis.x ) ),
		    a_rvEndTangent.y * vBasis.w + ( vBasis.z * a_rvStartTangent.y + ( a_rvEnd.y * vBasis.y + a_rvStart.y * vBasis.x ) ),
		    a_rvEndTangent.z * vBasis.w + ( vBasis.z * a_rvStartTangent.z + ( a_rvEnd.z * vBasis.y + a_rvStart.z * vBasis.x ) )
		);

		TFLOAT fDistance = TVector3::Distance( vPrevious, vPoint );
		fT               = fT + 0.1f;
		vPrevious        = vPoint;
		fLength          = fDistance + fLength;
	} while ( !( fT > 1.0f ) );

	return fLength;
}

// GetHermiteCurveLength with TVector4s
// $deBlob: FUNCTION 0066e580
TFLOAT TMath::GetHermiteCurveLength( const TVector4& a_rvStart, const TVector4& a_rvStartTangent, const TVector4& a_rvEnd, const TVector4& a_rvEndTangent )
{
	TFLOAT   fT      = 0.0f;
	TFLOAT   fLength = 0.0f;
	TVector4 vPrevious( a_rvStart );

	do
	{
		TVector4 vPowers( fT * fT * fT, fT * fT, fT, 1.0f );
		TVector4 vBasis;
		TMatrix44::TransformVector( vBasis, s_mHermite, vPowers );

		TVector4 vPoint, vTerm;
		vPoint.Multiply( a_rvStart, vBasis.x );
		vTerm.Multiply( a_rvEnd, vBasis.y );
		vPoint.x += vTerm.x;
		vPoint.y += vTerm.y;
		vPoint.z += vTerm.z;
		vTerm.Multiply( a_rvStartTangent, vBasis.z );
		vPoint.x += vTerm.x;
		vPoint.y += vTerm.y;
		vPoint.z += vTerm.z;
		vTerm.Multiply( a_rvEndTangent, vBasis.w );
		vPoint.x += vTerm.x;
		vPoint.y += vTerm.y;
		vPoint.z += vTerm.z;

		TFLOAT fDistance = TVector4::Distance( vPrevious, vPoint );
		fT               = fT + 0.1f;
		vPrevious        = vPoint;
		fLength          = fDistance + fLength;
	} while ( !( fT > 1.0f ) );

	return fLength;
}

// The tangent of a cardinal spline: (a_rcNext - a_rcPrevious) * a_fTension (w is a_rcNext's, scaled)
// $deBlob: FUNCTION 0066e710
TVector4 TMath::CalcCardinalSplineTangent( const TVector4& a_rcPrevious, const TVector4& a_rcNext, TFLOAT a_fTension )
{
	TVector4 vDifference( a_rcNext.x - a_rcPrevious.x, a_rcNext.y - a_rcPrevious.y, a_rcNext.z - a_rcPrevious.z, a_rcNext.w );
	TVector4 vTangent;
	vTangent.Multiply( vDifference, a_fTension );
	return vTangent;
}

// Scales the a_iCount values so that they add up to a_fTotal
// $deBlob: FUNCTION 0066e780
void TMath::NormaliseFloatArray( TFLOAT* a_pValues, TINT a_iCount, TFLOAT a_fTotal )
{
	TFLOAT fSum = 0.0f;

	for ( TINT i = 0; i < a_iCount; i++ )
		fSum += a_pValues[ i ];

	TFLOAT fScale = a_fTotal / fSum;

	for ( TINT i = 0; i < a_iCount; i++ )
		a_pValues[ i ] *= fScale;
}

// SphereVsPolygon on the plane of the polygon's first three vertices
// $deBlob: FUNCTION 006715f0
TBOOL TMath::SphereVsPolygon( const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4& a_rNormal, const TVector3* const* a_ppVertices, TINT a_iNumVertices, TFLOAT a_fEpsilon )
{
	TPlane oPlane( *a_ppVertices[ 0 ], *a_ppVertices[ 1 ], *a_ppVertices[ 2 ] );
	return SphereVsPolygon( a_rcCenter, a_fRadius, a_rfDistance, a_rNormal, a_ppVertices, oPlane, a_iNumVertices, a_fEpsilon );
}

// SphereVsPolygon with TVector4 vertices
// NOTE: only the first three vertices are copied (into a static holder), whatever a_iNumVertices is, as in retail
// $deBlob: FUNCTION 00671640
TBOOL TMath::SphereVsPolygon( const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4& a_rNormal, const TVector4* const* a_ppVertices, const TPlane& a_rcPlane, TINT a_iNumVertices, TFLOAT a_fEpsilon )
{
	static TVector3 s_apvPolyVertsHolder[ 3 ];

	const TVector3* apVertices[ 3 ] = { &s_apvPolyVertsHolder[ 0 ], &s_apvPolyVertsHolder[ 1 ], &s_apvPolyVertsHolder[ 2 ] };
	s_apvPolyVertsHolder[ 0 ]       = a_ppVertices[ 0 ]->AsVector3();
	s_apvPolyVertsHolder[ 1 ]       = a_ppVertices[ 1 ]->AsVector3();
	s_apvPolyVertsHolder[ 2 ]       = a_ppVertices[ 2 ]->AsVector3();

	return SphereVsPolygon( a_rcCenter, a_fRadius, a_rfDistance, a_rNormal, apVertices, a_rcPlane, a_iNumVertices, a_fEpsilon );
}

// SphereVsPolygon with TVector4 vertices, on the plane of the first three
// NOTE: only the first three vertices are copied (into a static holder), whatever a_iNumVertices is, as in retail
// $deBlob: FUNCTION 00671710
TBOOL TMath::SphereVsPolygon( const TVector4& a_rcCenter, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4& a_rNormal, const TVector4* const* a_ppVertices, TINT a_iNumVertices, TFLOAT a_fEpsilon )
{
	static TVector3 s_apvPolyVertsHolder[ 3 ];

	const TVector3* apVertices[ 3 ] = { &s_apvPolyVertsHolder[ 0 ], &s_apvPolyVertsHolder[ 1 ], &s_apvPolyVertsHolder[ 2 ] };
	s_apvPolyVertsHolder[ 0 ]       = a_ppVertices[ 0 ]->AsVector3();
	s_apvPolyVertsHolder[ 1 ]       = a_ppVertices[ 1 ]->AsVector3();
	s_apvPolyVertsHolder[ 2 ]       = a_ppVertices[ 2 ]->AsVector3();

	TPlane oPlane( s_apvPolyVertsHolder[ 0 ], s_apvPolyVertsHolder[ 1 ], s_apvPolyVertsHolder[ 2 ] );
	return SphereVsPolygon( a_rcCenter, a_fRadius, a_rfDistance, a_rNormal, apVertices, oPlane, a_iNumVertices, a_fEpsilon );
}

// Slides the travelled part of a move along the plane (push out 0.1, friction 0.004): a_rOutPosition is where the move
// ends, a_rOutTravelled the travelled part
// $deBlob: FUNCTION 006717f0
void TMath::SlideAlongPlane( const TVector4& a_rcDirection, const TVector4& a_rcNormal, TFLOAT a_fSpeedScale, TFLOAT a_fCompleteDist, TFLOAT a_fTravelledDistance, TVector4& a_rOutPosition, TVector4& a_rOutTravelled )
{
	TFLOAT   fDistance;
	TVector4 vDirection;
	SlideAlongPlane( a_rcDirection, a_rcNormal, a_fSpeedScale, a_fCompleteDist, a_fTravelledDistance, fDistance, vDirection, 0.1f, 0.004f, 1.0f );

	vDirection.Multiply( fDistance );

	TVector4 vTravelled;
	vTravelled.Multiply( a_rcDirection, a_fTravelledDistance );
	a_rOutTravelled = vTravelled;

	a_rOutPosition.Set( a_rOutTravelled.x + vDirection.x, a_rOutTravelled.y + vDirection.y, a_rOutTravelled.z + vDirection.z, a_rOutTravelled.w );
}

// Whether the triangle is (almost) flat: the two shorter sides add up to the longest one within a_fEpsilon
// $deBlob: FUNCTION 00671c00
TBOOL TMath::IsCollinearTriangle( const TVector4* const* a_ppVertices, TFLOAT a_fEpsilon )
{
	return IsCollinearTriangle( *a_ppVertices[ 0 ], *a_ppVertices[ 1 ], *a_ppVertices[ 2 ], a_fEpsilon );
}

// $deBlob: FUNCTION 00671ca0
TBOOL TMath::IsCollinearTriangle( const TVector4& a_rcVertex1, const TVector4& a_rcVertex2, const TVector4& a_rcVertex3, TFLOAT a_fEpsilon )
{
	TFLOAT fSide12 = TVector4::Distance( a_rcVertex1, a_rcVertex2 );
	TFLOAT fSide23 = TVector4::Distance( a_rcVertex2, a_rcVertex3 );
	TFLOAT fSide13 = TVector4::Distance( a_rcVertex1, a_rcVertex3 );

	TFLOAT fShortest, fMiddle;
	TFLOAT fLongest = fSide13;

	if ( fSide12 <= fSide23 )
	{
		fShortest = fSide12;

		if ( fSide13 <= fSide23 )
		{
			fMiddle  = fSide13;
			fLongest = fSide23;
		}
		else
		{
			fMiddle = fSide23;
		}
	}
	else
	{
		fShortest = fSide23;

		if ( fSide13 <= fSide12 )
		{
			fMiddle  = fSide13;
			fLongest = fSide12;
		}
		else
		{
			fMiddle = fSide12;
		}
	}

	return a_fEpsilon > TMath::Abs( ( fMiddle + fShortest ) - fLongest );
}

// Moves a_rPosition by a_rVelocity accelerated by a_rcAcceleration for a_fDeltaTime (and updates the velocity)
// $deBlob: FUNCTION 00671d40
void TMath::Accelerate( TVector4& a_rPosition, TVector4& a_rVelocity, const TVector4& a_rcAcceleration, TFLOAT a_fDeltaTime )
{
	TFLOAT fX = a_rcAcceleration.x * a_fDeltaTime;
	TFLOAT fY = a_rcAcceleration.y * a_fDeltaTime;
	TFLOAT fZ = a_rcAcceleration.z * a_fDeltaTime;

	a_rPosition.x = ( fX * 0.5f + a_rVelocity.x ) * a_fDeltaTime + a_rPosition.x;
	a_rPosition.y = ( fY * 0.5f + a_rVelocity.y ) * a_fDeltaTime + a_rPosition.y;
	a_rPosition.z = ( fZ * 0.5f + a_rVelocity.z ) * a_fDeltaTime + a_rPosition.z;

	a_rVelocity.x = a_rVelocity.x + fX;
	a_rVelocity.y = a_rVelocity.y + fY;
	a_rVelocity.z = a_rVelocity.z + fZ;
}

// Where a_rcPoint is along the 2D capsule a_rcStart - a_rcEnd: 1 before the start, 0 along it, 2 past the end
// (a_fRadius isn't used)
// $deBlob: FUNCTION 00671e30
TINT TMath::GetCapsuleZone( const TVector2& a_rcPoint, const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fRadius )
{
	TFLOAT fDX = a_rcEnd.x - a_rcStart.x;
	TFLOAT fDY = a_rcEnd.y - a_rcStart.y;

	if ( ( a_rcPoint.y - a_rcEnd.y ) * fDY + ( a_rcPoint.x - a_rcEnd.x ) * fDX < 0.0f )
		return ( a_rcPoint.y - a_rcStart.y ) * fDY + ( a_rcPoint.x - a_rcStart.x ) * fDX < 0.0f;

	return 2;
}

TOSHI_NAMESPACE_END

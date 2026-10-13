#include "ToshiPCH.h"
#include "TVector4.h"

TOSHI_NAMESPACE_START

constinit const TVector4 TVector4::VEC_ZERO = TVector4( 0, 0, 0, 1 );
constinit const TVector4 TVector4::VEC_POSX = TVector4( 1, 0, 0, 1 );
constinit const TVector4 TVector4::VEC_POSY = TVector4( 0, 1, 0, 1 );
constinit const TVector4 TVector4::VEC_POSZ = TVector4( 0, 0, 1, 1 );
constinit const TVector4 TVector4::VEC_NEGX = TVector4( -1, 0, 0, 1 );
constinit const TVector4 TVector4::VEC_NEGY = TVector4( 0, -1, 0, 1 );
constinit const TVector4 TVector4::VEC_NEGZ = TVector4( 0, 0, -1, 1 );

// NOTE: called GetAngleBeteenVector in other TOSHI builds
// $deBlob: FUNCTION 00673730
// $Barnyard: FUNCTION 006c7a70
TFLOAT TVector4::GetAngleBetweenVector( const TVector4& a_rcVec ) const
{
	// NOTE: same order of operations as de Blob (Barnyard sums in another order)
	TFLOAT fCos = ( x * a_rcVec.x + y * a_rcVec.y + z * a_rcVec.z ) /
	    ( TMath::Sqrt( a_rcVec.x * a_rcVec.x + a_rcVec.y * a_rcVec.y + a_rcVec.z * a_rcVec.z ) * TMath::Sqrt( x * x + y * y + z * z ) );

	if ( 1.0f < fCos )
		return 0.0f;

	if ( fCos < -1.0f )
		return TMath::PI;

	return TMath::ACos( fCos );
}

// $deBlob: FUNCTION 00672f50
// $Barnyard: FUNCTION 006c7600
TFLOAT TVector4::NormaliseMag()
{
	TFLOAT fMagnitudeSq = MagnitudeSq();

	if ( fMagnitudeSq != 0.0f )
	{
		TFLOAT fMultiplier = TMath::OneOverSqrt( fMagnitudeSq );
		x *= fMultiplier;
		y *= fMultiplier;
		z *= fMultiplier;

		return 1.0f / fMultiplier;
	}

	x = 0.0f;
	y = 0.0f;
	z = 1.0f;
	return fMagnitudeSq;
}

// $deBlob: FUNCTION 00673000
// $Barnyard: FUNCTION 006c7590
void TVector4::Normalise()
{
	TFLOAT fMagnitudeSq = MagnitudeSq();

	if ( fMagnitudeSq != 0.0f )
	{
		TFLOAT fMultiplier = TMath::OneOverSqrt( fMagnitudeSq );
		x *= fMultiplier;
		y *= fMultiplier;
		z *= fMultiplier;
		return;
	}

	x = 0.0f;
	y = 0.0f;
	z = 1.0f;
}

// $deBlob: FUNCTION 00673960
// $Barnyard: FUNCTION 006c7930
void TVector4::ProjectNormalToPlane( const TVector4& vec, const TVector4& vec2 )
{
	TVector4 crossProduct;
	crossProduct.CrossProduct( vec2, vec );
	CrossProduct( crossProduct, vec2 );
	Normalise();
}

// $Barnyard: FUNCTION 006c7c60
void TVector4::RotateAroundAxis( const TVector4& a_rcAxis, TFLOAT a_fAngle )
{
	TFLOAT fCrossX = z * a_rcAxis.y - y * a_rcAxis.z;
	TFLOAT fCrossY = x * a_rcAxis.z - z * a_rcAxis.x;
	TFLOAT fCrossZ = y * a_rcAxis.x - x * a_rcAxis.y;

	TFLOAT fSin = TMath::Sin( a_fAngle );
	TFLOAT fDot = z * a_rcAxis.z + y * a_rcAxis.y + a_rcAxis.x * x;
	TFLOAT fCos = TMath::Cos( a_fAngle );

	TFLOAT fX = fCos * x + fCrossX * fSin;
	TFLOAT fY = fCos * y + fCrossY * fSin;
	TFLOAT fZ = fCos * z + fCrossZ * fSin;

	TFLOAT fScale = ( 1.0f - fCos ) * fDot;

	x = fX + fScale * a_rcAxis.x;
	y = fY + fScale * a_rcAxis.y;
	z = fZ + fScale * a_rcAxis.z;
}

// $Barnyard: FUNCTION 006c7b40
void TVector4::RotateAroundAxis( const TVector4& a_rcVec, const TVector4& a_rcAxis, TFLOAT a_fAngle, TVector4& a_rOut )
{
	TFLOAT fCrossX = a_rcAxis.y * a_rcVec.z - a_rcAxis.z * a_rcVec.y;
	TFLOAT fCrossY = a_rcAxis.z * a_rcVec.x - a_rcAxis.x * a_rcVec.z;
	TFLOAT fCrossZ = a_rcAxis.x * a_rcVec.y - a_rcAxis.y * a_rcVec.x;
	TFLOAT fDot    = a_rcAxis.z * a_rcVec.z + a_rcAxis.y * a_rcVec.y + a_rcVec.x * a_rcAxis.x;

	TFLOAT fCos = TMath::Cos( a_fAngle );
	a_rOut.x    = fCos * a_rcVec.x;
	a_rOut.y    = fCos * a_rcVec.y;
	a_rOut.z    = fCos * a_rcVec.z;
	a_rOut.w    = a_rcVec.w;

	TFLOAT fSin = TMath::Sin( a_fAngle );
	a_rOut.x += fCrossX * fSin;
	a_rOut.y += fCrossY * fSin;
	a_rOut.z += fCrossZ * fSin;

	TFLOAT fScale = ( 1.0f - fCos ) * fDot;
	a_rOut.x += fScale * a_rcAxis.x;
	a_rOut.y += fScale * a_rcAxis.y;
	a_rOut.z += fScale * a_rcAxis.z;
}

// de Blob's variant (out first, one SinCos, different rounding than Barnyard's)
// $deBlob: FUNCTION 00673b50
void TVector4::RotateArbitaryVector( TVector4& a_rOut, const TVector4& a_rcVec, const TVector4& a_rcAxis, TFLOAT a_fAngle )
{
	TFLOAT fDot = a_rcAxis.x * a_rcVec.x + a_rcVec.y * a_rcAxis.y + a_rcAxis.z * a_rcVec.z;

	TFLOAT fSin, fCos;
	TMath::SinCos( a_fAngle, fSin, fCos );

	TFLOAT fOneMinusCos = 1.0f - fCos;
	TFLOAT fX           = a_rcVec.x;
	TFLOAT fY           = a_rcVec.y;
	TFLOAT fZ           = a_rcVec.z;

	a_rOut.w = a_rcVec.w;
	a_rOut.y = fCos * fY + ( fX * a_rcAxis.z - a_rcAxis.x * fZ ) * fSin + a_rcAxis.y * fDot * fOneMinusCos;
	a_rOut.x = fX * fCos + ( a_rcAxis.y * fZ - a_rcAxis.z * fY ) * fSin + a_rcAxis.x * fDot * fOneMinusCos;
	a_rOut.z = fCos * fZ + ( a_rcAxis.x * fY - fX * a_rcAxis.y ) * fSin + a_rcAxis.z * fDot * fOneMinusCos;
}

// $Barnyard: FUNCTION 006c79c0
TFLOAT TVector4::GetYaw() const
{
	if ( z < 0.0f )
	{
		if ( x < 0.0f )
			return TMath::ATan( x / z ) - TMath::PI;

		if ( x <= 0.0f )
			return TMath::PI;

		return TMath::ATan( x / z ) + TMath::PI;
	}

	if ( z <= 0.0f )
		return ( x > 0.0f ) ? TMath::HALF_PI : -TMath::HALF_PI;

	if ( x < 0.0f )
		return TMath::ATan( x / z );

	if ( x <= 0.0f )
		return 0.0f;

	return TMath::ATan( x / z );
}

// $Barnyard: FUNCTION 00544160
void TVector4::Normalise4()
{
	Divide4( Magnitude4() );
}

TOSHI_NAMESPACE_END

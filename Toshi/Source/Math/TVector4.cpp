#include "ToshiPCH.h"
#include "TVector4.h"

TOSHI_NAMESPACE_START

TCONSTINIT const TVector4 TVector4::VEC_ZERO = TVector4( 0, 0, 0, 1 );
TCONSTINIT const TVector4 TVector4::VEC_POSX = TVector4( 1, 0, 0, 1 );
TCONSTINIT const TVector4 TVector4::VEC_POSY = TVector4( 0, 1, 0, 1 );
TCONSTINIT const TVector4 TVector4::VEC_POSZ = TVector4( 0, 0, 1, 1 );
TCONSTINIT const TVector4 TVector4::VEC_NEGX = TVector4( -1, 0, 0, 1 );
TCONSTINIT const TVector4 TVector4::VEC_NEGY = TVector4( 0, -1, 0, 1 );
TCONSTINIT const TVector4 TVector4::VEC_NEGZ = TVector4( 0, 0, -1, 1 );

// $deBlob: FUNCTION 006725c0
void TVector4::Abs4( const TVector4& vec4 )
{
	x = TMath::Abs( vec4.x );
	y = TMath::Abs( vec4.y );
	z = TMath::Abs( vec4.z );
	w = TMath::Abs( vec4.w );
}

// $deBlob: FUNCTION 00672630
// $Barnyard: FUNCTION 006c7350
void TVector4::Multiply( const TVector4& vec1, const TVector4& vec2 )
{
	x = vec1.x * vec2.x;
	y = vec1.y * vec2.y;
	z = vec1.z * vec2.z;
	w = vec1.w;
}

// $deBlob: FUNCTION 00672670
// $Barnyard: FUNCTION 006c7380
void TVector4::Multiply( const TVector4& vec, TFLOAT scalar )
{
	x = vec.x * scalar;
	y = vec.y * scalar;
	z = vec.z * scalar;
	w = vec.w;
}

// $deBlob: FUNCTION 00672700
// $Barnyard: FUNCTION 006c73b0
void TVector4::Multiply4( const TVector4& vec, TFLOAT scalar )
{
	x = vec.x * scalar;
	y = vec.y * scalar;
	z = vec.z * scalar;
	w = vec.w * scalar;
}

// $deBlob: FUNCTION 00672790
// $Barnyard: FUNCTION 006c73e0
void TVector4::Divide( const TVector4& vec, TFLOAT scalar )
{
	TFLOAT fInv = 1.0f / scalar;
	x           = vec.x * fInv;
	y           = vec.y * fInv;
	z           = vec.z * fInv;
	w           = vec.w;
}

// $deBlob: FUNCTION 00672a70
// $Barnyard: FUNCTION 006c7410
void TVector4::Abs4()
{
	Set( TMath::Abs( x ), TMath::Abs( y ), TMath::Abs( z ), TMath::Abs( w ) );
}

// $deBlob: FUNCTION 00672ad0
// $Barnyard: FUNCTION 006c7430
void TVector4::Multiply( const TVector4& vec )
{
	x *= vec.x;
	y *= vec.y;
	z *= vec.z;
}

// $deBlob: FUNCTION 00672ba0
// $Barnyard: FUNCTION 006c7470
void TVector4::Multiply4( TFLOAT scalar )
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
}

// $deBlob: FUNCTION 00672c00
// $Barnyard: FUNCTION 006c74a0
void TVector4::Divide( TFLOAT scalar )
{
	TFLOAT ratio = 1.0f / scalar;
	x *= ratio;
	y *= ratio;
	z *= ratio;
}

// $deBlob: FUNCTION 00672e20
// $Barnyard: FUNCTION 006a7a60
TFLOAT TVector4::MagnitudeSq() const
{
	return x * x + y * y + z * z;
}

// $deBlob: FUNCTION 00672e60
TFLOAT TVector4::Magnitude() const
{
	return TMath::Sqrt( x * x + y * y + z * z );
}

// $deBlob: FUNCTION 00672eb0
// $Barnyard: FUNCTION 006c74d0
TFLOAT TVector4::MagnitudeSqXZ() const
{
	return x * x + z * z;
}

// $deBlob: FUNCTION 00672ee0
// $Barnyard: FUNCTION 006c7520
TFLOAT TVector4::MagnitudeXZ() const
{
	return TMath::Sqrt( x * x + z * z );
}

// Clears y and normalises x and z ((0, 0, 1) if both are 0). Returns the length on the XZ plane
// $deBlob: FUNCTION 00673090
// $Barnyard: FUNCTION 006c7540
TFLOAT TVector4::NormaliseXZ()
{
	TFLOAT fMag = x * x + z * z;
	TFLOAT fX   = x;
	TFLOAT fZ   = z;

	if ( fMag != 0.0f )
	{
		// NOTE: de Blob divides (Barnyard multiplies by the reciprocal)
		fMag = TMath::Sqrt( fMag );
		y    = 0.0f;
		x    = fX / fMag;
		z    = fZ / fMag;
		return fMag;
	}

	x = 0.0f;
	z = 1.0f;
	y = 0.0f;
	return fMag;
}

// $deBlob: FUNCTION 00673140
TBOOL TVector4::IsNormalised( TFLOAT fVal ) const
{
	TFLOAT fMagSq = MagnitudeSq();
	return ( ( ( 1.0f - fVal ) * ( 1.0f - fVal ) ) < fMagSq ) && ( fMagSq < ( ( fVal + 1.0f ) * ( fVal + 1.0f ) ) );
}

// Returns TTRUE if all four components are within [-a_fTolerance, a_fTolerance].
// $deBlob: FUNCTION 006731a0
// $Barnyard: FUNCTION 006c78a0
TBOOL TVector4::IsZero4( TFLOAT a_fTolerance ) const
{
	TFLOAT fMin = -a_fTolerance;
	return x >= fMin && x <= a_fTolerance && y >= fMin && y <= a_fTolerance && z >= fMin && z <= a_fTolerance && w >= fMin && w <= a_fTolerance;
}

// $deBlob: FUNCTION 00673200
// $Barnyard: FUNCTION 006c7690
TFLOAT TVector4::DotProduct4( const TVector4& vec1, const TVector4& vec2 )
{
	return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z + vec1.w * vec2.w;
}

// $deBlob: FUNCTION 006732a0
// $Barnyard: FUNCTION 006c7700
TFLOAT TVector4::Distance( const TVector4& vec1, const TVector4& vec2 )
{
	TFLOAT fX = vec2.x - vec1.x;
	TFLOAT fY = vec2.y - vec1.y;
	TFLOAT fZ = vec2.z - vec1.z;
	return TMath::Sqrt( fX * fX + fY * fY + fZ * fZ );
}

// $deBlob: FUNCTION 00673300
// $Barnyard: FUNCTION 006cbaf0
TFLOAT TVector4::DistanceXZ( const TVector4& vec1, const TVector4& vec2 )
{
	return ( vec2 - vec1 ).MagnitudeXZ();
}

// $deBlob: FUNCTION 00673350
// $Barnyard: FUNCTION 006c7740
void TVector4::CrossProduct( const TVector4& vec1, const TVector4& vec2 )
{
	// NOTE: computed before storing like the original, so this can be vec1 or vec2 (ARigidBody::AddAngularResponse)
	TFLOAT fX = vec2.z * vec1.y - vec2.y * vec1.z;
	TFLOAT fY = vec1.z * vec2.x - vec2.z * vec1.x;
	TFLOAT fZ = vec1.x * vec2.y - vec1.y * vec2.x;
	w         = vec1.w;
	x         = fX;
	y         = fY;
	z         = fZ;
}

// $deBlob: FUNCTION 006733c0
// $Barnyard: FUNCTION 006c7790
void TVector4::Lerp4( const TVector4& start, const TVector4& finish, TFLOAT t )
{
	TVector4::x = ( finish.x - start.x ) * t + start.x;
	TVector4::y = ( finish.y - start.y ) * t + start.y;
	TVector4::z = ( finish.z - start.z ) * t + start.z;
	TVector4::w = ( finish.w - start.w ) * t + start.w;
}

// $deBlob: FUNCTION 00673430
// $Barnyard: FUNCTION 006c77e0
void TVector4::Lerp3( const TVector4& start, const TVector4& finish, TFLOAT t )
{
	TVector4::x = ( finish.x - start.x ) * t + start.x;
	TVector4::y = ( finish.y - start.y ) * t + start.y;
	TVector4::z = ( finish.z - start.z ) * t + start.z;
	TVector4::w = start.w;
}

// $deBlob: FUNCTION 00673490
void TVector4::RotateX( TFLOAT a_fAngle )
{
	TFLOAT fSin, fCos;
	TMath::SinCos( a_fAngle, fSin, fCos );
	TFLOAT fZ = z;
	z         = fZ * fCos + y * fSin;
	y         = y * fCos - fZ * fSin;
}

// $deBlob: FUNCTION 006734f0
void TVector4::RotateY( TFLOAT a_fAngle )
{
	TFLOAT fSin, fCos;
	TMath::SinCos( a_fAngle, fSin, fCos );
	TFLOAT fZ = z;
	z         = fZ * fCos - x * fSin;
	x         = x * fCos + fZ * fSin;
}

// $deBlob: FUNCTION 00673550
void TVector4::RotateZ( TFLOAT a_fAngle )
{
	TFLOAT fSin, fCos;
	TMath::SinCos( a_fAngle, fSin, fCos );
	TFLOAT fY = y;
	y         = x * fSin + fY * fCos;
	x         = x * fCos - fY * fSin;
}

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

// $deBlob: FUNCTION 00672b60
// $Barnyard: FUNCTION 006c7450
void TVector4::Multiply( TFLOAT scalar )
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
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

// $deBlob: FUNCTION 00673250
// $deBlob: FUNCTION 00572c90
// $Barnyard: FUNCTION 006c76c0
TFLOAT TVector4::DistanceSq( const TVector4& vec1, const TVector4& vec2 )
{
	return ( vec2 - vec1 ).MagnitudeSq();
}

// Scales the vector down to a_fMaxLength if it's longer
// $deBlob: FUNCTION 00673820
void TVector4::ClampLength( TFLOAT a_fMaxLength )
{
	TFLOAT fMagSq = x * x + y * y + z * z;

	if ( a_fMaxLength * a_fMaxLength < fMagSq )
	{
		TFLOAT fScale = a_fMaxLength / TMath::Sqrt( fMagSq );
		x             = x * fScale;
		y             = y * fScale;
		z             = z * fScale;
	}
}

// Scales x and z down so the length on the XZ plane is at most a_fMaxLength
// $deBlob: FUNCTION 006738d0
void TVector4::ClampLengthXZ( TFLOAT a_fMaxLength )
{
	TFLOAT fMagSq = x * x + z * z;

	if ( a_fMaxLength * a_fMaxLength < fMagSq )
	{
		TFLOAT fMag = TMath::Sqrt( fMagSq );
		x           = x * ( a_fMaxLength / fMag );
		z           = z * ( a_fMaxLength / fMag );
	}
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

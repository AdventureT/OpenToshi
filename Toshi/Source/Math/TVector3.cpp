#include "ToshiPCH.h"
#include "TVector3.h"

TOSHI_NAMESPACE_START

TCONSTINIT const TVector3 TVector3::VEC_ZERO     = TVector3( 0, 0, 0 );
TCONSTINIT const TVector3 TVector3::VEC_POSX     = TVector3( 1, 0, 0 );
TCONSTINIT const TVector3 TVector3::VEC_POSY     = TVector3( 0, 1, 0 );
TCONSTINIT const TVector3 TVector3::VEC_POSZ     = TVector3( 0, 0, 1 );
TCONSTINIT const TVector3 TVector3::VEC_NEGX     = TVector3( -1, 0, 0 );
TCONSTINIT const TVector3 TVector3::VEC_NEGY     = TVector3( 0, -1, 0 );
TCONSTINIT const TVector3 TVector3::VEC_NEGZ     = TVector3( 0, 0, -1 );
TCONSTINIT const TVector3 TVector3::VEC_POSXPOSZ = TVector3( TMath::ONEOVER_SQRT_TWO, 0, TMath::ONEOVER_SQRT_TWO );
TCONSTINIT const TVector3 TVector3::VEC_POSXNEGZ = TVector3( TMath::ONEOVER_SQRT_TWO, 0, -TMath::ONEOVER_SQRT_TWO );
TCONSTINIT const TVector3 TVector3::VEC_NEGXPOSZ = TVector3( -TMath::ONEOVER_SQRT_TWO, 0, TMath::ONEOVER_SQRT_TWO );
TCONSTINIT const TVector3 TVector3::VEC_NEGXNEGZ = TVector3( -TMath::ONEOVER_SQRT_TWO, 0, -TMath::ONEOVER_SQRT_TWO );

// $deBlob: FUNCTION 00676350
void TVector3::Abs( const TVector3& vec3 )
{
	Set( TMath::Abs( vec3.x ), TMath::Abs( vec3.y ), TMath::Abs( vec3.z ) );
}

// $deBlob: FUNCTION 006763a0
void TVector3::Abs()
{
	Set( TMath::Abs( x ), TMath::Abs( y ), TMath::Abs( z ) );
}

// $deBlob: FUNCTION 006763f0
// $Barnyard: FUNCTION 006cbb20
TBOOL TVector3::Equals( const TVector3& vec )
{
	return x == vec.x && y == vec.y && z == vec.z;
}

// $deBlob: FUNCTION 00676430
// $Barnyard: FUNCTION 006c74f0
TFLOAT TVector3::Magnitude() const
{
	return TMath::Sqrt( x * x + y * y + z * z );
}

// $deBlob: FUNCTION 00676620
TBOOL TVector3::IsNormalised( TFLOAT fVal ) const
{
	TFLOAT fMagSq = MagnitudeSq();
	return ( ( 1.0f - fVal ) * ( 1.0f - fVal ) ) < fMagSq && fMagSq < ( ( fVal + 1.0f ) * ( fVal + 1.0f ) );
}

// $deBlob: FUNCTION 006766d0
TFLOAT TVector3::Distance( const TVector3& vec1, const TVector3& vec2 )
{
	return ( vec2 - vec1 ).Magnitude();
}

// $deBlob: FUNCTION 00676780
// $Barnyard: FUNCTION 006cbb60
void TVector3::Lerp( const TVector3& start, const TVector3& finish, float t )
{
	TVector3 progress = finish - start;
	progress.Multiply( t );

	Set( start + progress );
}

// $deBlob: FUNCTION 006767e0
// $Barnyard: FUNCTION 006cbba0
void TVector3::Lerp( const TVector3& finish, float t )
{
	Lerp( *this, finish, t );
}

// $deBlob: FUNCTION 00676840
void TVector3::Clip( TFLOAT fVal, TFLOAT fVal2 )
{
	TMath::Clip( x, fVal, fVal2 );
	TMath::Clip( y, fVal, fVal2 );
	TMath::Clip( z, fVal, fVal2 );
}

// $deBlob: FUNCTION 00666e10
void TVector3::CrossProduct( const TVector3& vec1, const TVector3& vec2 )
{
	TFLOAT fX = vec2.z * vec1.y - vec2.y * vec1.z;
	TFLOAT fY = vec1.z * vec2.x - vec2.z * vec1.x;
	TFLOAT fZ = vec1.x * vec2.y - vec1.y * vec2.x;
	Set( fX, fY, fZ );
}

// $deBlob: FUNCTION 006768b0
// $Barnyard: FUNCTION 006cbbe0
void TVector3::RotateX( TFLOAT a_fRotation )
{
	TFLOAT f1;
	TFLOAT f2;
	TFLOAT oldY = y;

	TMath::SinCos( a_fRotation, f1, f2 );

	y = -f1 * z + f2 * oldY;
	z = oldY * f1 + z * f2;
}

// $deBlob: FUNCTION 00676b30
// $Barnyard: FUNCTION 006cbc30
TFLOAT TVector3::GetAngleBetweenVector( const TVector3& a_rcVec ) const
{
	// NOTE: same order of operations as de Blob (Barnyard sums in another order)
	TFLOAT fCos = ( x * a_rcVec.x + y * a_rcVec.y + z * a_rcVec.z ) /
	    ( TMath::Sqrt( a_rcVec.x * a_rcVec.x + a_rcVec.y * a_rcVec.y + a_rcVec.z * a_rcVec.z ) * TMath::Sqrt( x * x + y * y + z * z ) );

	// NOTE: unlike TVector4::GetAngleBetweenVector, values below -1 are not clamped
	if ( 1.0f < fCos )
		return 0.0f;

	return TMath::ACos( fCos );
}

// $deBlob: FUNCTION 00676910
// $Barnyard: FUNCTION 006c7820
void TVector3::RotateY( TFLOAT a_fRotation )
{
	TFLOAT f1;
	TFLOAT f2;
	TFLOAT oldX = x;

	TMath::SinCos( a_fRotation, f1, f2 );

	x = f1 * z + f2 * oldX;
	z = z * f2 + -f1 * oldX;
}

// $deBlob: FUNCTION 00676970
// $Barnyard: FUNCTION 006c7860
void TVector3::RotateZ( TFLOAT a_fRotation )
{
	TFLOAT f1;
	TFLOAT f2;
	TFLOAT oldX = x;

	TMath::SinCos( a_fRotation, f1, f2 );

	x = -f1 * y + f2 * oldX;
	y = f1 * oldX + y * f2;
}

// $deBlob: FUNCTION 006764f0
void TVector3::Normalize()
{
	if ( MagnitudeSq() != 0.0f )
	{
		TFLOAT magnitude = TMath::OneOverSqrt( MagnitudeSq() );
		Set( x * magnitude, y * magnitude, z * magnitude );
	}
	else
	{
		Set( 0.0f, 0.0f, 1.0f );
	}
}

TOSHI_NAMESPACE_END

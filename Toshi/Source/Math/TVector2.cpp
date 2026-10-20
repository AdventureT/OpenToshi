#include "ToshiPCH.h"
#include "TVector2.h"

TOSHI_NAMESPACE_START

TCONSTINIT const TVector2 TVector2::VEC_ZERO = TVector2( 0, 0 );
TCONSTINIT const TVector2 TVector2::VEC_POSX = TVector2( 1, 0 );
TCONSTINIT const TVector2 TVector2::VEC_POSY = TVector2( 0, 1 );
TCONSTINIT const TVector2 TVector2::VEC_NEGX = TVector2( -1, 0 );
TCONSTINIT const TVector2 TVector2::VEC_NEGY = TVector2( 0, -1 );

// $deBlob: FUNCTION 00676e40
TBOOL TVector2::IsEqual( const TVector2& vec )
{
	return TMath::Abs( x - vec.x ) < 0.00001f && TMath::Abs( y - vec.y ) < 0.00001f;
}

// $deBlob: FUNCTION 00676ea0
// $Barnyard: FUNCTION 006ccdf0
TFLOAT TVector2::Magnitude() const
{
	return TMath::Sqrt( x * x + y * y );
}

// $deBlob: FUNCTION 00676f10
// $Barnyard: FUNCTION 006cce70
void TVector2::Normalize()
{
	if ( MagnitudeSq() != 0.0f )
	{
		TFLOAT magnitude = TMath::OneOverSqrt( MagnitudeSq() );
		Set( x * magnitude, y * magnitude );
	}
	else
	{
		Set( 1.0f, 0.0f );
	}
}

// Normalizes the vector and returns its old magnitude
// $deBlob: FUNCTION 00676f80
// $Barnyard: FUNCTION 006ccec0
TFLOAT TVector2::NormalizeMag()
{
	TFLOAT fMagSq = x * x + y * y;

	if ( fMagSq == 0.0f )
	{
		Set( 1.0f, 0.0f );
		return 0.0f;
	}

	// NOTE: de Blob uses OneOverSqrt (Barnyard 1 / Sqrt)
	TFLOAT fOneOverMag = TMath::OneOverSqrt( fMagSq );
	Set( x * fOneOverMag, y * fOneOverMag );

	return 1.0f / fOneOverMag;
}

// $deBlob: FUNCTION 00676ff0
TBOOL TVector2::IsNormalised( TFLOAT fVal ) const
{
	return ( ( 1.0f - fVal ) * ( 1.0f - fVal ) ) < MagnitudeSq() && MagnitudeSq() < ( ( fVal + 1.0f ) * ( fVal + 1.0f ) );
}

// $deBlob: FUNCTION 00677080
// $Barnyard: FUNCTION 006cce10
TFLOAT TVector2::Distance( const TVector2& vec1, const TVector2& vec2 )
{
	return ( vec2 - vec1 ).Magnitude();
}

// Sets the vector to the point a_fT of the way from a_rcStart to a_rcEnd
// $deBlob: FUNCTION 006770d0
// $Barnyard: FUNCTION 006ccf20
void TVector2::Lerp( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fT )
{
	x = ( a_rcEnd.x - a_rcStart.x ) * a_fT + a_rcStart.x;
	y = ( a_rcEnd.y - a_rcStart.y ) * a_fT + a_rcStart.y;
}

// Rotates the vector clockwise by a_fAngle radians
// $deBlob: FUNCTION 00677110
// $Barnyard: FUNCTION 006ccf50
void TVector2::Rotate( TFLOAT a_fAngle )
{
	TFLOAT fSin, fCos;
	TMath::SinCos( a_fAngle, fSin, fCos );

	TFLOAT fNewY = fCos * y - fSin * x;
	x            = fCos * x + fSin * y;
	y            = fNewY;
}

TOSHI_NAMESPACE_END

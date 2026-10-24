#pragma once
#include "Math/TVector3.h"
#include "Math/TVector2.h"

TOSHI_NAMESPACE_START

class TVector4
{
public:
	TCONSTEXPR TVector4() = default;
	TCONSTEXPR TVector4( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW = 1.0f )
	    : x( a_fX ), y( a_fY ), z( a_fZ ), w( a_fW )
	{}

	TCONSTEXPR TVector4( TFLOAT a_pFloats[ 4 ] )
	    : x( a_pFloats[ 0 ] ), y( a_pFloats[ 1 ] ), z( a_pFloats[ 2 ] ), w( a_pFloats[ 3 ] )
	{}

	// $Barnyard: FUNCTION 0041cd50
	TCONSTEXPR TVector4( const TVector3& a_rcOther )
	    : x( a_rcOther.x ), y( a_rcOther.y ), z( a_rcOther.z ), w( 1.0f )
	{}

	// $Barnyard: FUNCTION 006c7330
	// NOTE: retail copies the whole vector at once (movdqu) but still constructs temporaries in place
	TVector4( const TVector4& a_rcOther ) { memcpy( this, &a_rcOther, sizeof( TVector4 ) ); }

	TCONSTEXPR void Set( const TVector3& vec )
	{
		TVector4::x = vec.x;
		TVector4::y = vec.y;
		TVector4::z = vec.z;
		TVector4::w = 1.0f;
	}

	// $deBlob: FUNCTION 006725a0
	// $Barnyard: FUNCTION 0046b360
	TCONSTEXPR void Set( const TVector4& vec )
	{
		TVector4::x = vec.x;
		TVector4::y = vec.y;
		TVector4::z = vec.z;
		TVector4::w = vec.w;
	}

	TCONSTEXPR void Set( TFLOAT floats[ 4 ] )
	{
		TVector4::x = floats[ 0 ];
		TVector4::y = floats[ 1 ];
		TVector4::z = floats[ 2 ];
		TVector4::w = floats[ 3 ];
	}

	// $deBlob: FUNCTION 006762c0
	TCONSTEXPR void Set( TFLOAT x, TFLOAT y, TFLOAT z, TFLOAT w )
	{
		TVector4::x = x;
		TVector4::y = y;
		TVector4::z = z;
		TVector4::w = w;
	}

	TFLOAT NormaliseMag();
	void   Normalise();
	void   Normalise4();
	TBOOL IsNormalised( TFLOAT fVal = 0.05f ) const;

	void ProjectNormalToPlane( const TVector4& vec, const TVector4& vec2 );

	// Rotates the vector around a_rcAxis (normalised) by a_fAngle radians (Rodrigues), w is kept
	void RotateAroundAxis( const TVector4& a_rcAxis, TFLOAT a_fAngle );

	// Writes a_rcVec rotated around a_rcAxis (normalised) by a_fAngle radians (Rodrigues) to a_rOut, w is copied
	static void RotateAroundAxis( const TVector4& a_rcVec, const TVector4& a_rcAxis, TFLOAT a_fAngle, TVector4& a_rOut );
	static void RotateArbitaryVector( TVector4& a_rOut, const TVector4& a_rcVec, const TVector4& a_rcAxis, TFLOAT a_fAngle );

	// Returns the angle around the Y axis from +Z to the vector (atan2( x, z ))
	TFLOAT GetYaw() const;

	TCONSTEXPR void Add( const TVector3& vec )
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
	}

	// $Barnyard: FUNCTION 0041cd00
	TCONSTEXPR void Add( const TVector4& vec )
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
	}

	TCONSTEXPR void Add4( const TVector4& vec )
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
		w += vec.w;
	}

	// $Barnyard: FUNCTION 0043ccb0
	TCONSTEXPR void Add( const TVector4& a, const TVector4& b )
	{
		Set( a + b );
		w = a.w;
	}

	// $deBlob: FUNCTION 006761d0
	TCONSTEXPR void Add4( const TVector4& a, const TVector4& b )
	{
		Set( a );
		Add4( b );
	}

	TCONSTEXPR void Substract4( const TVector4& vec )
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		w -= vec.w;
	}

	TCONSTEXPR void Substract( const TVector4& vec )
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
	}

	// $Barnyard: FUNCTION 0041cd20
	TCONSTEXPR void Substract( const TVector4& a, const TVector4& b )
	{
		x = a.x - b.x;
		y = a.y - b.y;
		z = a.z - b.z;
		w = a.w;
	}

	// this = a_rcBase + a_rcA * a_rcB (xyz, the w of a_rcBase)
	// $deBlob: FUNCTION 00829e10
	TCONSTEXPR void MultiplyAdd( const TVector4& a_rcBase, const TVector4& a_rcA, const TVector4& a_rcB )
	{
		x = a_rcA.x * a_rcB.x + a_rcBase.x;
		y = a_rcA.y * a_rcB.y + a_rcBase.y;
		z = a_rcA.z * a_rcB.z + a_rcBase.z;
		w = a_rcBase.w;
	}

	// this = a_rcBase + a_rcA * a_fScale (xyz, the w of a_rcBase)
	// $deBlob: FUNCTION 0075f720
	TCONSTEXPR void MultiplyAdd( const TVector4& a_rcBase, const TVector4& a_rcA, TFLOAT a_fScale )
	{
		x = a_rcA.x * a_fScale + a_rcBase.x;
		y = a_rcA.y * a_fScale + a_rcBase.y;
		z = a_rcA.z * a_fScale + a_rcBase.z;
		w = a_rcBase.w;
	}

	// $deBlob: FUNCTION 00597e70
	TCONSTEXPR void Substract4( const TVector4& a, const TVector4& b )
	{
		Set( a );
		Substract4( b );
	}

	// $deBlob: FUNCTION 00672bc0
	TCONSTEXPR void Divide( const TVector4& vec )
	{
		x /= vec.x;
		y /= vec.y;
		z /= vec.z;
	}

	void Divide( TFLOAT scalar );

	// $Barnyard: FUNCTION 006c9250
	TCONSTEXPR void Divide4( TFLOAT scalar )
	{
		if ( scalar != 0.0f )
		{
			TFLOAT ratio = 1.0f / scalar;
			x *= ratio;
			y *= ratio;
			z *= ratio;
			w *= ratio;
		}
	}

	// $deBlob: FUNCTION 00672750
	TCONSTEXPR void Divide( const TVector4& vec1, const TVector4& vec2 )
	{
		Set( vec1 / vec2 );
	}

	void Divide( const TVector4& vec, TFLOAT scalar );

	void Multiply( const TVector4& vec );

	void Multiply( const TVector4& vec1, const TVector4& vec2 );

	void Multiply( TFLOAT scalar );

	void Multiply( const TVector4& vec, TFLOAT scalar );

	// $deBlob: FUNCTION 00672b10
	TCONSTEXPR void Multiply4( const TVector4& vec )
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
		w *= vec.w;
	}

	// $deBlob: FUNCTION 006726b0
	TCONSTEXPR void Multiply4( const TVector4& vec1, const TVector4& vec2 )
	{
		x = vec1.x * vec2.x;
		y = vec1.y * vec2.y;
		z = vec1.z * vec2.z;
		w = vec1.w * vec2.w;
	}

	void Multiply4( TFLOAT scalar );

	void Multiply4( const TVector4& vec, TFLOAT scalar );

	// Per component maximum/minimum of all four components
	// NOTE: the comparisons are written like the original, they decide which value is kept for NaNs

	// $deBlob: FUNCTION 006727e0
	TCONSTEXPR void Max( const TVector4& a, const TVector4& b )
	{
		x = ( a.x <= b.x && b.x != a.x ) ? b.x : a.x;
		y = ( a.y <= b.y && b.y != a.y ) ? b.y : a.y;
		z = ( a.z <= b.z && b.z != a.z ) ? b.z : a.z;
		w = ( a.w <= b.w && b.w != a.w ) ? b.w : a.w;
	}

	// $deBlob: FUNCTION 00672850
	TCONSTEXPR void Max( const TVector4& a, TFLOAT s )
	{
		x = ( s < a.x || s == a.x ) ? a.x : s;
		y = ( s < a.y || s == a.y ) ? a.y : s;
		z = ( s < a.z || s == a.z ) ? a.z : s;
		w = ( s < a.w || s == a.w ) ? a.w : s;
	}

	// $deBlob: FUNCTION 006728b0
	TCONSTEXPR void Min( const TVector4& a, const TVector4& b )
	{
		x = ( b.x <= a.x && a.x != b.x ) ? b.x : a.x;
		y = ( b.y <= a.y && a.y != b.y ) ? b.y : a.y;
		z = ( b.z <= a.z && a.z != b.z ) ? b.z : a.z;
		w = ( b.w <= a.w && a.w != b.w ) ? b.w : a.w;
	}

	// $deBlob: FUNCTION 00672920
	TCONSTEXPR void Min( const TVector4& a, TFLOAT s )
	{
		x = ( a.x <= s ) ? a.x : s;
		y = ( a.y <= s ) ? a.y : s;
		z = ( a.z <= s ) ? a.z : s;
		w = ( a.w <= s ) ? a.w : s;
	}

	// $deBlob: FUNCTION 00672c40
	TCONSTEXPR void Max( const TVector4& vec ) { Max( *this, vec ); }

	// $deBlob: FUNCTION 00672cb0
	TCONSTEXPR void Max( TFLOAT s ) { Max( *this, s ); }

	// $deBlob: FUNCTION 00672d00
	TCONSTEXPR void Min( const TVector4& vec ) { Min( *this, vec ); }

	// $deBlob: FUNCTION 00672d70
	TCONSTEXPR void Min( TFLOAT s ) { Min( *this, s ); }

	// $deBlob: FUNCTION 00672990
	void Sin( const TVector4& vec )
	{
		x = TMath::Sin( vec.x );
		y = TMath::Sin( vec.y );
		z = TMath::Sin( vec.z );
		w = TMath::Sin( vec.w );
	}

	// $deBlob: FUNCTION 00672a00
	void Cos( const TVector4& vec )
	{
		x = TMath::Cos( vec.x );
		y = TMath::Cos( vec.y );
		z = TMath::Cos( vec.z );
		w = TMath::Cos( vec.w );
	}

	void RotateX( TFLOAT a_fAngle );

	void RotateY( TFLOAT a_fAngle );

	void RotateZ( TFLOAT a_fAngle );

	void CrossProduct( const TVector4& vec1, const TVector4& vec2 );

	void Lerp3( const TVector4& finish, TFLOAT t ) { Lerp3( *this, finish, t ); }

	void Lerp3( const TVector4& start, const TVector4& finish, TFLOAT t );

	void Lerp4( const TVector4& finish, TFLOAT t ) { Lerp4( *this, finish, t ); }

	void Lerp4( const TVector4& start, const TVector4& finish, TFLOAT t );

	// $deBlob: FUNCTION 00673710
	TCONSTEXPR double GetScalarProjectionOnUnit( const TVector4& vec )
	{
		return (double)( TVector4::z * vec.z + TVector4::x * vec.x + TVector4::y * vec.y );
	}

	// Returns the length of the projection onto a_rcVec in units of a_rcVec
	// $deBlob: FUNCTION 006736b0
	TCONSTEXPR TFLOAT GetScalarProjection( const TVector4& a_rcVec ) const
	{
		return ( y * a_rcVec.y + a_rcVec.x * x + z * a_rcVec.z ) / ( a_rcVec.x * a_rcVec.x + a_rcVec.y * a_rcVec.y + a_rcVec.z * a_rcVec.z );
	}

	// Replaces the vector with its projection onto a_rcVec (w is taken from a_rcVec)
	// $deBlob: FUNCTION 006735b0
	// $Barnyard: FUNCTION 006c7d70
	TCONSTEXPR void ProjectOnto( const TVector4& a_rcVec )
	{
		TFLOAT fScale = ( y * a_rcVec.y + a_rcVec.x * x + z * a_rcVec.z ) / ( a_rcVec.x * a_rcVec.x + a_rcVec.y * a_rcVec.y + a_rcVec.z * a_rcVec.z );
		Set( a_rcVec.x * fScale, a_rcVec.y * fScale, a_rcVec.z * fScale, a_rcVec.w );
	}

	// Replaces the vector with its projection onto the unit vector a_rcUnit (w is taken from a_rcUnit)
	// $deBlob: FUNCTION 00673640
	TCONSTEXPR void SetUnitVectorProjection( const TVector4& a_rcUnit )
	{
		TFLOAT fScale = y * a_rcUnit.y + a_rcUnit.x * x + z * a_rcUnit.z;
		Set( a_rcUnit.x * fScale, a_rcUnit.y * fScale, a_rcUnit.z * fScale, a_rcUnit.w );
	}

	// Scales the vector down to a_fMaxLength if it's longer
	void ClampLength( TFLOAT a_fMaxLength );

	// Scales x and z down so the length on the XZ plane is at most a_fMaxLength
	void ClampLengthXZ( TFLOAT a_fMaxLength );

	// Writes a unit vector perpendicular to this one (built from the two largest components) to a_rOut
	// $deBlob: FUNCTION 00673a80
	void GetPerpendicular( TVector4& a_rOut ) const
	{
		TFLOAT fZSq = z * z;
		TFLOAT fMag;
		TFLOAT fLast;

		if ( TMath::Abs( x ) <= TMath::Abs( y ) )
		{
			fMag     = TMath::Sqrt( fZSq + y * y );
			a_rOut.x = 0.0f;
			a_rOut.y = z / fMag;
			fLast    = y;
		}
		else
		{
			fMag     = TMath::Sqrt( fZSq + x * x );
			a_rOut.y = 0.0f;
			a_rOut.x = z / fMag;
			fLast    = x;
		}

		a_rOut.w = 1.0f;
		a_rOut.z = -( fLast / fMag );
	}

	// Clears y and normalises x and z ((0, 0, 1) if both are 0). Returns the length on the XZ plane
	TFLOAT NormaliseXZ();

	void Abs( const TVector4& vec4 ) { Set( TMath::Abs( vec4.x ), TMath::Abs( vec4.y ), TMath::Abs( vec4.z ), w ); }
	void Abs() { Set( TMath::Abs( x ), TMath::Abs( y ), TMath::Abs( z ), w ); }

	void Abs4( const TVector4& vec4 );

	void Abs4();

	TCONSTEXPR void Negate( const TVector4& vec ) { Negate3( vec ); }
	TCONSTEXPR void Negate() { Negate3(); }

	// $Barnyard: FUNCTION 005f1740
	TCONSTEXPR void Negate3( const TVector4& vec ) { Set( -vec.x, -vec.y, -vec.z, vec.w ); }

	// $deBlob: FUNCTION 006762f0
	TCONSTEXPR void Negate3()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	// $deBlob: FUNCTION 005f0590
	TCONSTEXPR void Negate4( const TVector4& vec ) { Set( -vec.x, -vec.y, -vec.z, -vec.w ); }
	TCONSTEXPR void Negate4() { Set( -x, -y, -z, -w ); }

	// Returns angle between this and a_rcVec in radians (0..PI)
	TFLOAT GetAngleBetweenVector( const TVector4& a_rcVec ) const;

	TFLOAT Magnitude() const;
	TFLOAT Magnitude4() const { return TMath::Sqrt( w * w + x * x + y * y + z * z ); }
	TFLOAT MagnitudeSq() const;
	TCONSTEXPR TFLOAT MagnitudeSq4() const { return w * w + x * x + y * y + z * z; }
	TFLOAT MagnitudeXZ() const;
	TFLOAT MagnitudeSqXZ() const;
	// $deBlob: FUNCTION 00672f20
	TFLOAT InverseMagnitude() const { return TMath::OneOverSqrt( x * x + y * y + z * z ); }

	// $Barnyard: FUNCTION 006c92f0
	// NOTE: compares all four components, w included
	TCONSTEXPR TBOOL IsEqual( const TVector4& a_rcVec ) const
	{
		return a_rcVec.x == x && a_rcVec.y == y && a_rcVec.z == z && a_rcVec.w == w;
	}

	// Compares xyz only (used by T2Vector<TVector4>::Find)
	TCONSTEXPR TBOOL operator==( const TVector4& a_rcVec ) const { return a_rcVec.x == x && a_rcVec.y == y && a_rcVec.z == z; }

	// Returns TTRUE if all four components are within [-a_fTolerance, a_fTolerance].
	TBOOL IsZero4( TFLOAT a_fTolerance ) const;

	// NOTE: retail tests IsZero4( 1.0e10f )
	TBOOL IsReasonable() const { return IsZero4( 1.0e10f ); }

	TCONSTEXPR TVector4 operator+( const TVector3& other ) const { return { x + other.x, y + other.y, z + other.z, w }; }
	TCONSTEXPR TVector4 operator-( const TVector3& other ) const { return { x - other.x, y - other.y, z - other.z, w }; }
	TCONSTEXPR TVector4 operator*( const TVector3& other ) const { return { x * other.x, y * other.y, z * other.z, w }; }
	TCONSTEXPR TVector4 operator/( const TVector3& other ) const { return { x / other.x, y / other.y, z / other.z, w }; }

	// NOTE: the binary operators work on xyz and keep the w of this (like Add/Substract/Multiply( a, b ) keep a.w)
	// $Barnyard: FUNCTION 0043cce0
	TCONSTEXPR TVector4 operator+( const TVector4& other ) const { return { x + other.x, y + other.y, z + other.z, w }; }
	// $Barnyard: FUNCTION 0041cd70
	TCONSTEXPR TVector4 operator-( const TVector4& other ) const { return { x - other.x, y - other.y, z - other.z, w }; }
	TCONSTEXPR TVector4 operator*( const TVector4& other ) const { return { x * other.x, y * other.y, z * other.z, w }; }
	TCONSTEXPR TVector4 operator/( const TVector4& other ) const { return { x / other.x, y / other.y, z / other.z, w }; }

	// $Barnyard: FUNCTION 004159e0
	TVector4 operator*( TFLOAT a_fScalar ) const
	{
		TVector4 vResult;
		vResult.Multiply( *this, a_fScalar );
		return vResult;
	}

	// $Barnyard: FUNCTION 00464ab0
	TVector4 operator/( TFLOAT a_fScalar ) const
	{
		TVector4 vResult;
		vResult.Divide( *this, a_fScalar );
		return vResult;
	}

	void operator=( const TVector3& other ) { Set( other ); }
	TVector4& operator=( const TVector4& other ) = default;
	void operator+=( const TVector4& other ) { Add( other ); }

	TVector4& operator-=( const TVector4& other )
	{
		Substract( other );
		return *this;
	}

	void operator/=( const TVector4& other ) { Divide( other ); }
	void operator*=( const TVector4& other ) { Multiply( other ); }

	void operator/=( TFLOAT a_fScalar ) { Divide( a_fScalar ); }
	void operator*=( TFLOAT a_fScalar ) { Multiply( a_fScalar ); }

	TFLOAT*       AsArray() { return TREINTERPRETCAST( TFLOAT*, this ); }
	const TFLOAT* AsArray() const { return TREINTERPRETCAST( const TFLOAT*, this ); }

	TVector3&       AsVector3() { return TREINTERPRETCAST( TVector3&, *this ); }
	const TVector3& AsVector3() const { return TREINTERPRETCAST( const TVector3&, *this ); }

	TVector2&       AsVector2() { return TREINTERPRETCAST( TVector2&, *this ); }
	const TVector2& AsVector2() const { return TREINTERPRETCAST( const TVector2&, *this ); }

public:
	static TFLOAT Distance( const TVector4& vec1, const TVector4& vec2 );

	static TFLOAT DistanceSq( const TVector4& vec1, const TVector4& vec2 );

	static TFLOAT DistanceXZ( const TVector4& vec1, const TVector4& vec2 );
	// $Barnyard: FUNCTION 00414e00
	static TFLOAT DistanceSqXZ( const TVector4& vec1, const TVector4& vec2 ) { return ( vec2 - vec1 ).MagnitudeSqXZ(); }

	TFLOAT TCONSTEXPR        DotProduct( const TVector4& vec ) const { return DotProduct3( *this, vec ); }
	static TCONSTEXPR TFLOAT DotProduct( const TVector4& vec1, const TVector4& vec2 ) { return DotProduct3( vec1, vec2 ); }

	TFLOAT TCONSTEXPR DotProduct3( const TVector4& vec ) const { return DotProduct3( *this, vec ); }

	// $Barnyard: FUNCTION 006c7670
	static TCONSTEXPR TFLOAT DotProduct3( const TVector4& vec1, const TVector4& vec2 ) { return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z; }

	TFLOAT DotProduct4( const TVector4& vec ) const { return DotProduct4( *this, vec ); }

	static TFLOAT DotProduct4( const TVector4& vec1, const TVector4& vec2 );

	// $Barnyard: FUNCTION 006085d0
	static void Swap( TVector4& a_rVec1, TVector4& a_rVec2 )
	{
		TVector4 temp = a_rVec1;
		a_rVec1       = a_rVec2;
		a_rVec2       = temp;
	}

public:
	static const TCONSTINIT TVector4 VEC_ZERO;
	static const TCONSTINIT TVector4 VEC_POSX;
	static const TCONSTINIT TVector4 VEC_POSY;
	static const TCONSTINIT TVector4 VEC_POSZ;
	static const TCONSTINIT TVector4 VEC_NEGX;
	static const TCONSTINIT TVector4 VEC_NEGY;
	static const TCONSTINIT TVector4 VEC_NEGZ;

public:
	TFLOAT x, y, z, w;
};

TOSHI_NAMESPACE_END

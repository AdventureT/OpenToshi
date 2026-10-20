/**
 * @file TVector3.h
 * 3D vector implementation for the Toshi engine
 * 
 * This file provides 3D vector functionality including:
 * - Vector operations (addition, subtraction, multiplication)
 * - Vector normalization and length calculations
 * - Vector transformations
 * - Vector utility functions
 */

#pragma once

TOSHI_NAMESPACE_START

class TVector3
{
public:
	TCONSTEXPR TVector3() = default;
	// $deBlob: FUNCTION 00553120
	TCONSTEXPR TVector3( TFLOAT x, TFLOAT y, TFLOAT z ) { Set( x, y, z ); }
	TCONSTEXPR TVector3( TFLOAT floats[ 3 ] ) { Set( floats ); }
	TCONSTEXPR TVector3( const TVector3& other ) { Set( other.x, other.y, other.z ); }

	void Clip( TFLOAT fVal, TFLOAT fVal2 );
	void CrossProduct( const TVector3&, const TVector3& );

	void RotateX( TFLOAT a_fRotation );
	void RotateY( TFLOAT a_fRotation );

	// Returns the angle (radians, 0..PI) between this vector and a_rcVec
	TFLOAT GetAngleBetweenVector( const TVector3& a_rcVec ) const;
	void   RotateZ( TFLOAT a_fRotation );

	void Normalize();

	TCONSTEXPR void Set( const TVector3& vec )
	{
		TVector3::x = vec.x;
		TVector3::y = vec.y;
		TVector3::z = vec.z;
	}

	TCONSTEXPR void Set( TFLOAT floats[ 3 ] )
	{
		TVector3::x = floats[ 0 ];
		TVector3::y = floats[ 1 ];
		TVector3::z = floats[ 2 ];
	}

	TCONSTEXPR void Set( TFLOAT x, TFLOAT y, TFLOAT z )
	{
		TVector3::x = x;
		TVector3::y = y;
		TVector3::z = z;
	}

	void Lerp( const TVector3& finish, float t );

	void Lerp( const TVector3& start, const TVector3& finish, float t );

	// $Barnyard: FUNCTION 0040f390
	TCONSTEXPR void Add( const TVector3& vec )
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;
	}

	TCONSTEXPR void Add( const TVector3& a, const TVector3& b )
	{
		Set( a + b );
	}

	// $deBlob: FUNCTION 00666e70
	TCONSTEXPR void Substract( const TVector3& a, const TVector3& b )
	{
		x = a.x - b.x;
		y = a.y - b.y;
		z = a.z - b.z;
	}

	TCONSTEXPR void Divide( const TVector3& vec )
	{
		x /= vec.x;
		y /= vec.y;
		z /= vec.z;
	}

	// $deBlob: FUNCTION 00676240
	TCONSTEXPR void Divide( TFLOAT scalar )
	{
		TFLOAT ratio = 1.0f / scalar;
		x *= ratio;
		y *= ratio;
		z *= ratio;
	}

	// $deBlob: FUNCTION 00676190
	TCONSTEXPR TVector3& operator/=( TFLOAT scalar )
	{
		Divide( scalar );
		return *this;
	}

	// $deBlob: FUNCTION 00676290
	TCONSTEXPR void Negate()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	// $deBlob: FUNCTION 006769d0
	TCONSTEXPR void SetVectorProjection( const TVector3& a_rcVec )
	{
		TFLOAT fScale = ( y * a_rcVec.y + a_rcVec.x * x + z * a_rcVec.z ) / ( a_rcVec.x * a_rcVec.x + a_rcVec.y * a_rcVec.y + a_rcVec.z * a_rcVec.z );
		Set( a_rcVec.x * fScale, a_rcVec.y * fScale, a_rcVec.z * fScale );
	}

	// $deBlob: FUNCTION 00676a50
	TCONSTEXPR void SetUnitVectorProjection( const TVector3& a_rcUnit )
	{
		TFLOAT fScale = y * a_rcUnit.y + a_rcUnit.x * x + z * a_rcUnit.z;
		Set( a_rcUnit.x * fScale, a_rcUnit.y * fScale, a_rcUnit.z * fScale );
	}

	// $deBlob: FUNCTION 00676ab0
	TCONSTEXPR TFLOAT GetScalarProjection( const TVector3& a_rcVec ) const
	{
		return ( y * a_rcVec.y + a_rcVec.x * x + z * a_rcVec.z ) / ( a_rcVec.x * a_rcVec.x + a_rcVec.y * a_rcVec.y + a_rcVec.z * a_rcVec.z );
	}

	// $deBlob: FUNCTION 00676b10
	TCONSTEXPR TFLOAT DotProduct( const TVector3& a_rcVec ) const { return a_rcVec.z * z + a_rcVec.x * x + a_rcVec.y * y; }

	// Scales the vector down to a_fMaxLength if it's longer
	// $deBlob: FUNCTION 00676c10
	void ClampLength( TFLOAT a_fMaxLength )
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

	// Writes a unit vector perpendicular to this one (built from the two largest components) to a_rOut
	// $deBlob: FUNCTION 00676cc0
	void GetPerpendicular( TVector3& a_rOut ) const
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

		a_rOut.z = -( fLast / fMag );
	}

	// Clears y and normalises x and z ((0, 0, 1) if both are 0). Returns the length on the XZ plane
	// $deBlob: FUNCTION 00676580
	TFLOAT NormalizeXZ()
	{
		TFLOAT fX     = x;
		TFLOAT fZ     = z;
		TFLOAT fMagSq = fX * fX + fZ * fZ;

		if ( fMagSq == 0.0f )
		{
			x = 0.0f;
			z = 1.0f;
			y = 0.0f;

			// NOTE: unlike TVector4::NormalizeXZ, 1 is returned for a zero vector
			return 1.0f;
		}

		TFLOAT fMag = TMath::Sqrt( fMagSq );
		y           = 0.0f;
		x           = fX / fMag;
		z           = fZ / fMag;
		return fMag;
	}

	TBOOL IsNormalised( TFLOAT fVal = 0.05f ) const;

	// $deBlob: FUNCTION 00676680
	TCONSTEXPR TBOOL IsReasonable( TFLOAT a_fTolerance ) const
	{
		TFLOAT fMin = -a_fTolerance;
		return fMin <= x && x <= a_fTolerance && fMin <= y && y <= a_fTolerance && fMin <= z && z <= a_fTolerance;
	}

	TCONSTEXPR void Divide( const TVector3& vec1, const TVector3& vec2 )
	{
		Set( vec1 / vec2 );
	}

	TCONSTEXPR void Divide( const TVector3& vec, TFLOAT scalar )
	{
		Set( vec );
		Divide( scalar );
	}

	TCONSTEXPR void Multiply( const TVector3& vec )
	{
		x *= vec.x;
		y *= vec.y;
		z *= vec.z;
	}

	TCONSTEXPR void Multiply( TFLOAT scalar )
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
	}

	TCONSTEXPR void Multiply( const TVector3& vec1, const TVector3& vec2 )
	{
		Set( vec1 * vec2 );
	}

	TCONSTEXPR void Multiply( const TVector3& vec, TFLOAT scalar )
	{
		Set( vec );
		Multiply( scalar );
	}

	TBOOL Equals( const TVector3& vec );

	void Abs( const TVector3& vec3 );
	void Abs();

	TFLOAT Magnitude() const;
	TCONSTEXPR TFLOAT MagnitudeSq() const { return x * x + y * y + z * z; }
	// $deBlob: FUNCTION 00676480
	TFLOAT MagnitudeXZ() const { return TMath::Sqrt( x * x + z * z ); }
	// $deBlob: FUNCTION 00676d90
	TCONSTEXPR TFLOAT MagnitudeSqXZ() const { return x * x + z * z; }
	// $deBlob: FUNCTION 006764c0
	TFLOAT InverseMagnitude() const { return TMath::OneOverSqrt( x * x + y * y + z * z ); }

	TCONSTEXPR TVector3 operator+( const TVector3& other ) const { return { x + other.x, y + other.y, z + other.z }; }
	// $Barnyard: FUNCTION 004610a0
	TCONSTEXPR TVector3 operator-( const TVector3& other ) const { return { x - other.x, y - other.y, z - other.z }; }
	TCONSTEXPR TVector3 operator*( const TVector3& other ) const { return { x * other.x, y * other.y, z * other.z }; }
	TCONSTEXPR TVector3 operator/( const TVector3& other ) const { return { x / other.x, y / other.y, z / other.z }; }

	TCONSTEXPR void operator=( const TVector3& other ) { Set( other ); }
	TCONSTEXPR void operator+=( const TVector3& other ) { Add( other ); }

	// $Barnyard: FUNCTION 0040f3c0
	TCONSTEXPR TVector3& operator-=( const TVector3& other )
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	TCONSTEXPR void operator/=( const TVector3& other ) { Divide( other ); }

public:
	static TFLOAT Distance( const TVector3& vec1, const TVector3& vec2 );
	// $deBlob: FUNCTION 00676730
	static TFLOAT DistanceXZ( const TVector3& vec1, const TVector3& vec2 )
	{
		TVector3 vDiff = vec2 - vec1;
		return TMath::Sqrt( vDiff.z * vDiff.z + vDiff.x * vDiff.x );
	}
	static TCONSTEXPR TFLOAT DistanceSq( const TVector3& vec1, const TVector3& vec2 ) { return ( vec2 - vec1 ).MagnitudeSq(); }
	static TCONSTEXPR TFLOAT DotProduct( const TVector3& vec1, const TVector3& vec2 ) { return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z; }

public:
	static TCONSTINIT const TVector3 VEC_ZERO;
	static TCONSTINIT const TVector3 VEC_POSX;
	static TCONSTINIT const TVector3 VEC_POSY;
	static TCONSTINIT const TVector3 VEC_POSZ;
	static TCONSTINIT const TVector3 VEC_NEGX;
	static TCONSTINIT const TVector3 VEC_NEGY;
	static TCONSTINIT const TVector3 VEC_NEGZ;
	static TCONSTINIT const TVector3 VEC_NEGXPOSZ;
	static TCONSTINIT const TVector3 VEC_NEGXNEGZ;
	static TCONSTINIT const TVector3 VEC_POSXPOSZ;
	static TCONSTINIT const TVector3 VEC_POSXNEGZ;

public:
	TFLOAT x, y, z;
};

TOSHI_NAMESPACE_END

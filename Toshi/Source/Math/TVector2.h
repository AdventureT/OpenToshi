#pragma once

TOSHI_NAMESPACE_START

class TVector2
{
public:
	TCONSTEXPR TVector2() = default;
	// $deBlob: FUNCTION 006772a0
	TCONSTEXPR TVector2( TFLOAT x, TFLOAT y ) { Set( x, y ); }
	TCONSTEXPR TVector2( TFLOAT coords[ 2 ] ) { Set( coords[ 0 ], coords[ 1 ] ); }
	TCONSTEXPR TVector2( const TVector2& other ) = default;

	// $deBlob: FUNCTION 00550f80
	TCONSTEXPR void Set( TFLOAT x, TFLOAT y )
	{
		TVector2::x = x;
		TVector2::y = y;
	}

	TCONSTEXPR void Set( TFLOAT coords[ 2 ] )
	{
		TVector2::x = coords[ 0 ];
		TVector2::y = coords[ 1 ];
	}

	TCONSTEXPR void Set( const TVector2& other )
	{
		TVector2::x = other.x;
		TVector2::y = other.y;
	}

	TCONSTEXPR void Add( const TVector2& vec )
	{
		x += vec.x;
		y += vec.y;
	}

	TCONSTEXPR void Add( const TVector2& a, const TVector2& b )
	{
		Set( a + b );
	}

	TCONSTEXPR void Divide( const TVector2& vec )
	{
		x /= vec.x;
		y /= vec.y;
	}

	TCONSTEXPR void Divide( TFLOAT scalar )
	{
		TFLOAT ratio = 1.0f / scalar;
		x *= ratio;
		y *= ratio;
	}

	// Rotates the vector clockwise by a_fAngle radians
	void Rotate( TFLOAT a_fAngle );

	// Sets the vector to the point a_fT of the way from a_rcStart to a_rcEnd
	void Lerp( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fT );

	TCONSTEXPR void Divide( const TVector2& vec1, const TVector2& vec2 )
	{
		Set( vec1 / vec2 );
	}

	TCONSTEXPR void Divide( const TVector2& vec, TFLOAT scalar )
	{
		Set( vec );
		Divide( scalar );
	}

	TCONSTEXPR void Multiply( const TVector2& vec )
	{
		x *= vec.x;
		y *= vec.y;
	}

	TCONSTEXPR void Multiply( TFLOAT scalar )
	{
		x *= scalar;
		y *= scalar;
	}

	TCONSTEXPR void Multiply( const TVector2& vec1, const TVector2& vec2 )
	{
		Set( vec1 * vec2 );
	}

	TCONSTEXPR void Multiply( const TVector2& vec, TFLOAT scalar )
	{
		Set( vec );
		Multiply( scalar );
	}

	// $deBlob: FUNCTION 006722f0
	TCONSTEXPR void Negate()
	{
		Set( -x, -y );
	}

	// Rotates the vector by 90 degrees: ( -y, x )
	// $deBlob: FUNCTION 00672420
	TCONSTEXPR void Perpendicular()
	{
		TFLOAT fX = x;
		x         = -y;
		y         = fX;
	}

	// $deBlob: FUNCTION 00672440
	TCONSTEXPR void Substract( const TVector2& vec )
	{
		x -= vec.x;
		y -= vec.y;
	}

	// $deBlob: FUNCTION 006721a0
	TCONSTEXPR TVector2& operator-=( const TVector2& vec )
	{
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	// 2D cross product (the z of the 3D one)
	// $deBlob: FUNCTION 006721e0
	TCONSTEXPR TFLOAT CrossProduct( const TVector2& vec ) const { return vec.y * x - y * vec.x; }

	// $deBlob: FUNCTION 00672200
	static TCONSTEXPR TFLOAT CrossProduct( const TVector2& vec1, const TVector2& vec2 ) { return vec2.y * vec1.x - vec1.y * vec2.x; }

	void Normalize();

	// Normalizes the vector and returns its old magnitude
	TFLOAT NormalizeMag();

	// $deBlob: FUNCTION 00676dc0
	void Abs( const TVector2& vec3 ) { Set( TMath::Abs( vec3.x ), TMath::Abs( vec3.y ) ); }
	// $deBlob: FUNCTION 00676e00
	void Abs() { Set( TMath::Abs( x ), TMath::Abs( y ) ); }

	TFLOAT Magnitude() const;
	// $deBlob: FUNCTION 006722c0
	TCONSTEXPR TFLOAT MagnitudeSq() const { return x * x + y * y; }
	// $deBlob: FUNCTION 00676ee0
	TFLOAT InverseMagnitude() const { return TMath::OneOverSqrt( x * x + y * y ); }

	TBOOL IsNormalised( TFLOAT fVal = 0.05f ) const;

	// $deBlob: FUNCTION 00677040
	TCONSTEXPR TBOOL IsReasonable( TFLOAT a_fTolerance ) const { return -a_fTolerance <= x && x <= a_fTolerance && -a_fTolerance <= y && y <= a_fTolerance; }

	TBOOL IsEqual( const TVector2& vec );

	// $Barnyard: FUNCTION 006cce40
	TCONSTEXPR TBOOL operator==( const TVector2& a_rcVec ) const { return x == a_rcVec.x && y == a_rcVec.y; }

	// Scales the vector down to a_fMaxMagnitude if it's longer. Returns TTRUE if it was
	// NOTE: de Blob's version returns nothing
	// $deBlob: FUNCTION 00677210
	// $Barnyard: FUNCTION 006ccfe0
	TBOOL ClampMagnitude( TFLOAT a_fMaxMagnitude )
	{
		TFLOAT fMagSq = x * x + y * y;

		if ( !( a_fMaxMagnitude * a_fMaxMagnitude < fMagSq ) )
			return TFALSE;

		TFLOAT fMag = TMath::Sqrt( fMagSq );
		x           = x * ( a_fMaxMagnitude / fMag );
		y           = y * ( a_fMaxMagnitude / fMag );
		return TTRUE;
	}

	TCONSTEXPR TVector2 operator+( const TVector2& other ) const { return { x + other.x, y + other.y }; }
	TCONSTEXPR TVector2 operator-( const TVector2& other ) const { return { x - other.x, y - other.y }; }
	TCONSTEXPR TVector2 operator*( const TVector2& other ) const { return { x * other.x, y * other.y }; }
	TCONSTEXPR TVector2 operator/( const TVector2& other ) const { return { x / other.x, y / other.y }; }

	TCONSTEXPR void operator=( const TVector2& other ) { Set( other ); }
	TCONSTEXPR void operator+=( const TVector2& other ) { Add( other ); }
	TCONSTEXPR void operator/=( const TVector2& other ) { Divide( other ); }

public:
	static TFLOAT Distance( const TVector2& vec1, const TVector2& vec2 );

	// Returns TTRUE if a_rcPoint is on the right of (or on) the line going from a_rcStart to a_rcEnd
	// Returns TTRUE if a_rcPoint is on the left of (or on) the line going from a_rcStart to a_rcEnd
	// $deBlob: FUNCTION 00677170
	static TCONSTEXPR TBOOL IsPointLeftOfLine( const TVector2& a_rcStart, const TVector2& a_rcEnd, const TVector2& a_rcPoint ) { return 0.0f <= ( a_rcPoint.y - a_rcStart.y ) * ( a_rcEnd.x - a_rcStart.x ) - ( a_rcPoint.x - a_rcStart.x ) * ( a_rcEnd.y - a_rcStart.y ); }

	// $deBlob: FUNCTION 006771c0
	// $Barnyard: FUNCTION 006ccf90
	static TCONSTEXPR TBOOL IsOnRight( const TVector2& a_rcStart, const TVector2& a_rcEnd, const TVector2& a_rcPoint ) { return ( a_rcEnd.x - a_rcStart.x ) * ( a_rcPoint.y - a_rcStart.y ) - ( a_rcPoint.x - a_rcStart.x ) * ( a_rcEnd.y - a_rcStart.y ) <= 0.0f; }

	// $deBlob: FUNCTION 00629df0
	static TCONSTEXPR TFLOAT DistanceSq( const TVector2& vec1, const TVector2& vec2 ) { return ( vec2 - vec1 ).MagnitudeSq(); }
	static TCONSTEXPR TFLOAT DotProduct( const TVector2& vec1, const TVector2& vec2 ) { return vec1.x * vec2.x + vec1.y * vec2.y; }

public:
	TCONSTINIT const static TVector2 VEC_ZERO;
	TCONSTINIT const static TVector2 VEC_POSX;
	TCONSTINIT const static TVector2 VEC_POSY;
	TCONSTINIT const static TVector2 VEC_NEGX;
	TCONSTINIT const static TVector2 VEC_NEGY;

public:
	TFLOAT x, y;
};

TOSHI_NAMESPACE_END

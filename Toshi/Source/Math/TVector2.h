#pragma once

TOSHI_NAMESPACE_START

class TVector2
{
public:
	constexpr TVector2() = default;
	// $deBlob: FUNCTION 006772a0
	constexpr TVector2( TFLOAT x, TFLOAT y ) { Set( x, y ); }
	constexpr TVector2( TFLOAT coords[ 2 ] ) { Set( coords[ 0 ], coords[ 1 ] ); }
	constexpr TVector2( const TVector2& other ) { Set( other ); }

	constexpr void Set( TFLOAT x, TFLOAT y )
	{
		TVector2::x = x;
		TVector2::y = y;
	}

	constexpr void Set( TFLOAT coords[ 2 ] )
	{
		TVector2::x = coords[ 0 ];
		TVector2::y = coords[ 1 ];
	}

	constexpr void Set( const TVector2& other )
	{
		TVector2::x = other.x;
		TVector2::y = other.y;
	}

	constexpr void Add( const TVector2& vec )
	{
		x += vec.x;
		y += vec.y;
	}

	constexpr void Add( const TVector2& a, const TVector2& b )
	{
		Set( a + b );
	}

	constexpr void Divide( const TVector2& vec )
	{
		x /= vec.x;
		y /= vec.y;
	}

	constexpr void Divide( TFLOAT scalar )
	{
		TFLOAT ratio = 1.0f / scalar;
		x *= ratio;
		y *= ratio;
	}

	// Rotates the vector clockwise by a_fAngle radians
	// $deBlob: FUNCTION 00677110
	// $Barnyard: FUNCTION 006ccf50
	void Rotate( TFLOAT a_fAngle )
	{
		TFLOAT fSin, fCos;
		TMath::SinCos( a_fAngle, fSin, fCos );

		TFLOAT fNewY = fCos * y - fSin * x;
		x            = fCos * x + fSin * y;
		y            = fNewY;
	}

	// Sets the vector to the point a_fT of the way from a_rcStart to a_rcEnd
	// $deBlob: FUNCTION 006770d0
	// $Barnyard: FUNCTION 006ccf20
	constexpr void Lerp( const TVector2& a_rcStart, const TVector2& a_rcEnd, TFLOAT a_fT )
	{
		x = ( a_rcEnd.x - a_rcStart.x ) * a_fT + a_rcStart.x;
		y = ( a_rcEnd.y - a_rcStart.y ) * a_fT + a_rcStart.y;
	}

	constexpr void Divide( const TVector2& vec1, const TVector2& vec2 )
	{
		Set( vec1 / vec2 );
	}

	constexpr void Divide( const TVector2& vec, TFLOAT scalar )
	{
		Set( vec );
		Divide( scalar );
	}

	constexpr void Multiply( const TVector2& vec )
	{
		x *= vec.x;
		y *= vec.y;
	}

	constexpr void Multiply( TFLOAT scalar )
	{
		x *= scalar;
		y *= scalar;
	}

	constexpr void Multiply( const TVector2& vec1, const TVector2& vec2 )
	{
		Set( vec1 * vec2 );
	}

	constexpr void Multiply( const TVector2& vec, TFLOAT scalar )
	{
		Set( vec );
		Multiply( scalar );
	}

	// $deBlob: FUNCTION 006722f0
	constexpr void Negate()
	{
		Set( -x, -y );
	}

	// Rotates the vector by 90 degrees: ( -y, x )
	// $deBlob: FUNCTION 00672420
	constexpr void Perpendicular()
	{
		TFLOAT fX = x;
		x         = -y;
		y         = fX;
	}

	// $deBlob: FUNCTION 00672440
	constexpr void Substract( const TVector2& vec )
	{
		x -= vec.x;
		y -= vec.y;
	}

	// $deBlob: FUNCTION 006721a0
	constexpr TVector2& operator-=( const TVector2& vec )
	{
		x -= vec.x;
		y -= vec.y;
		return *this;
	}

	// 2D cross product (the z of the 3D one)
	// $deBlob: FUNCTION 006721e0
	constexpr TFLOAT CrossProduct( const TVector2& vec ) const { return vec.y * x - y * vec.x; }

	// $deBlob: FUNCTION 00672200
	static constexpr TFLOAT CrossProduct( const TVector2& vec1, const TVector2& vec2 ) { return vec2.y * vec1.x - vec1.y * vec2.x; }

	// $deBlob: FUNCTION 00676f10
	// $Barnyard: FUNCTION 006cce70
	void Normalize()
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
	TFLOAT NormalizeMag()
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

	// $deBlob: FUNCTION 00676dc0
	void Abs( const TVector2& vec3 ) { Set( TMath::Abs( vec3.x ), TMath::Abs( vec3.y ) ); }
	// $deBlob: FUNCTION 00676e00
	void Abs() { Set( TMath::Abs( x ), TMath::Abs( y ) ); }

	// $deBlob: FUNCTION 00676ea0
	// $Barnyard: FUNCTION 006ccdf0
	TFLOAT Magnitude() const { return TMath::Sqrt( x * x + y * y ); }
	// $deBlob: FUNCTION 006722c0
	constexpr TFLOAT MagnitudeSq() const { return x * x + y * y; }
	// $deBlob: FUNCTION 00676ee0
	TFLOAT InverseMagnitude() const { return TMath::OneOverSqrt( x * x + y * y ); }

	// $deBlob: FUNCTION 00676ff0
	constexpr TBOOL IsNormalised( TFLOAT fVal = 0.05f ) const { return ( ( 1.0f - fVal ) * ( 1.0f - fVal ) ) < MagnitudeSq() && MagnitudeSq() < ( ( fVal + 1.0f ) * ( fVal + 1.0f ) ); }

	// $deBlob: FUNCTION 00677040
	constexpr TBOOL IsReasonable( TFLOAT a_fTolerance ) const { return -a_fTolerance <= x && x <= a_fTolerance && -a_fTolerance <= y && y <= a_fTolerance; }

	// $deBlob: FUNCTION 00676e40
	TBOOL IsEqual( const TVector2& vec ) { return TMath::Abs( x - vec.x ) < 0.00001f && TMath::Abs( y - vec.y ) < 0.00001f; }

	// $Barnyard: FUNCTION 006cce40
	constexpr TBOOL operator==( const TVector2& a_rcVec ) const { return x == a_rcVec.x && y == a_rcVec.y; }

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

	constexpr TVector2 operator+( const TVector2& other ) const { return { x + other.x, y + other.y }; }
	constexpr TVector2 operator-( const TVector2& other ) const { return { x - other.x, y - other.y }; }
	constexpr TVector2 operator*( const TVector2& other ) const { return { x * other.x, y * other.y }; }
	constexpr TVector2 operator/( const TVector2& other ) const { return { x / other.x, y / other.y }; }

	constexpr void operator=( const TVector2& other ) { Set( other ); }
	constexpr void operator+=( const TVector2& other ) { Add( other ); }
	constexpr void operator/=( const TVector2& other ) { Divide( other ); }

public:
	// $deBlob: FUNCTION 00677080
	// $Barnyard: FUNCTION 006cce10
	static TFLOAT Distance( const TVector2& vec1, const TVector2& vec2 ) { return ( vec2 - vec1 ).Magnitude(); }

	// Returns TTRUE if a_rcPoint is on the right of (or on) the line going from a_rcStart to a_rcEnd
	// Returns TTRUE if a_rcPoint is on the left of (or on) the line going from a_rcStart to a_rcEnd
	// $deBlob: FUNCTION 00677170
	static constexpr TBOOL IsPointLeftOfLine( const TVector2& a_rcStart, const TVector2& a_rcEnd, const TVector2& a_rcPoint ) { return 0.0f <= ( a_rcPoint.y - a_rcStart.y ) * ( a_rcEnd.x - a_rcStart.x ) - ( a_rcPoint.x - a_rcStart.x ) * ( a_rcEnd.y - a_rcStart.y ); }

	// $deBlob: FUNCTION 006771c0
	// $Barnyard: FUNCTION 006ccf90
	static constexpr TBOOL IsOnRight( const TVector2& a_rcStart, const TVector2& a_rcEnd, const TVector2& a_rcPoint ) { return ( a_rcEnd.x - a_rcStart.x ) * ( a_rcPoint.y - a_rcStart.y ) - ( a_rcPoint.x - a_rcStart.x ) * ( a_rcEnd.y - a_rcStart.y ) <= 0.0f; }

	static constexpr TFLOAT DistanceSq( const TVector2& vec1, const TVector2& vec2 ) { return ( vec2 - vec1 ).MagnitudeSq(); }
	static constexpr TFLOAT DotProduct( const TVector2& vec1, const TVector2& vec2 ) { return vec1.x * vec2.x + vec1.y * vec2.y; }

public:
	constinit const static TVector2 VEC_ZERO;
	constinit const static TVector2 VEC_POSX;
	constinit const static TVector2 VEC_POSY;
	constinit const static TVector2 VEC_NEGX;
	constinit const static TVector2 VEC_NEGY;

public:
	TFLOAT x, y;
};

TOSHI_NAMESPACE_END

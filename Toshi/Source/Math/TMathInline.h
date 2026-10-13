#pragma once
#include "Toshi/Defines.h"
#include "Toshi/TDebug.h"

#include <cstdint>
#include <cfloat>
#include <cmath>

TOSHI_NAMESPACE_START

namespace TMath
{

constexpr TINT8  TINT8_MAX  = INT8_MAX;
constexpr TINT8  TINT8_MIN  = INT8_MIN;
constexpr TUINT8 TUINT8_MAX = UINT8_MAX;

constexpr TUINT TINT16_MAX  = INT16_MAX;
constexpr TUINT TINT16_MIN  = INT16_MIN;
constexpr TUINT TUINT16_MAX = UINT16_MAX;

constexpr TINT  TINT32_MAX  = INT32_MAX;
constexpr TINT  TINT32_MIN  = INT32_MIN;
constexpr TUINT TUINT32_MAX = UINT32_MAX;

constexpr TUINT16 MAXWCHAR = WCHAR_MAX;

constexpr TUINTPTR MAXPTR = UINTPTR_MAX;

constexpr TFLOAT MAXFLOAT       = FLT_MAX;
constexpr TFLOAT MINFLOAT       = FLT_MIN;
constexpr TFLOAT TFLOAT_EPSILON = FLT_EPSILON; // Renamed according to de Blob
constexpr TFLOAT FLOATEPSILON   = FLT_EPSILON; // Both exist for some reason
constexpr TFLOAT FLOAT_NAN      = NAN;

constexpr TFLOAT PI               = 3.1415927f;
constexpr TFLOAT TWO_PI           = PI * 2;
constexpr TFLOAT HALF_PI          = PI / 2;
constexpr TFLOAT ONEOVER_SQRT_TWO = 0.70710677f;
constexpr TFLOAT ONEOVERTWO_PI    = 1 / TWO_PI;

TFORCEINLINE TBOOL  IsFinite( TFLOAT fVal ) { return _finite( fVal ) != 0; }
// NOTE: de Blob calls the double precision CRT functions and rounds the results to float
TFORCEINLINE TFLOAT Sin( TFLOAT fVal ) { return TFLOAT( sin( fVal ) ); }
TFORCEINLINE TFLOAT Cos( TFLOAT fVal ) { return TFLOAT( cos( fVal ) ); }
TFORCEINLINE TFLOAT Tan( TFLOAT fVal ) { return TFLOAT( tan( fVal ) ); }

// Out of line in TMath.cpp (they assert)
TFLOAT ASin( TFLOAT a_fX );
TFLOAT ACos( TFLOAT a_fX );
TFLOAT ATan( TFLOAT a_fX );
TFLOAT Exp( TFLOAT a_fX );
TFLOAT Log( TFLOAT a_fX );
TFLOAT Log10( TFLOAT a_fX );
TFLOAT Pow( TFLOAT a_fX, TFLOAT a_fY );

TFORCEINLINE TFLOAT ATan2( TFLOAT fVal1, TFLOAT fVal2 )
{
	TFLOAT fVal = TFLOAT( atan2( fVal1, fVal2 ) );
	TASSERT( TMath::IsFinite( fVal ) );
	return fVal;
}
TFORCEINLINE TFLOAT Abs( TFLOAT fVal ) { return (TFLOAT)fabs( fVal ); }

// Returns 1, -1 or 0
// $Barnyard: FUNCTION 006b68f0
TFORCEINLINE TFLOAT Sign( TFLOAT a_fVal )
{
	if ( a_fVal > 0.0f ) return 1.0f;
	if ( a_fVal < 0.0f ) return -1.0f;
	return 0.0f;
}
TFORCEINLINE constexpr TFLOAT DegToRad( TFLOAT fDeg ) { return ( fDeg / 180.0f ) * PI; }

TFORCEINLINE constexpr TUINT IntLog2( TUINT32 a_uiValue )
{
	return std::bit_width( a_uiValue ) - 1;
}

inline constexpr TUINT RoundToNextPowerOfTwo( TUINT a_uiValue )
{
	a_uiValue--;
	a_uiValue |= a_uiValue >> 1;
	a_uiValue |= a_uiValue >> 2;
	a_uiValue |= a_uiValue >> 4;
	a_uiValue |= a_uiValue >> 8;
	a_uiValue |= a_uiValue >> 16;
	a_uiValue++;

	return a_uiValue;
}

TFORCEINLINE TFLOAT Sqrt( TFLOAT a_fX )
{
	// NOTE: retail inlines this as a plain fsqrt, so the square root of zero is valid (zero-length vectors etc.)
	TASSERT( a_fX >= 0.0f );
	TFLOAT fVal = sqrtf( a_fX );
	TASSERT( IsFinite( fVal ) );
	return fVal;
}

// $deBlob: FUNCTION 00673cb0
TFORCEINLINE TFLOAT OneOverSqrt( TFLOAT a_fX )
{
	TASSERT( a_fX != 0.0f );
	TFLOAT fVal = 1.0f / TFLOAT( sqrt( a_fX ) );
	TASSERT( TMath::IsFinite( fVal ) );
	return fVal;
}

TFORCEINLINE TINT Round( TFLOAT a_fVal ) { return (TINT)roundf( a_fVal ); }

// $deBlob: FUNCTION 00667200
// $Barnyard: FUNCTION 006b69d0
// NOTE: the original rounds with fist and then adds one if the difference is positive (compares the float's bits)
TFORCEINLINE TINT CeilToInt( TFLOAT a_fVal ) { return TINT( ceilf( a_fVal ) ); }

// $deBlob: FUNCTION 006671e0
// $Barnyard: FUNCTION 006b69b0
// NOTE: the original rounds with fist and then subtracts one if the difference is negative (compares the float's bits)
TFORCEINLINE TINT FloorToInt( TFLOAT a_fVal ) { return TINT( floorf( a_fVal ) ); }

TFORCEINLINE TBOOL IsNaN( TFLOAT fVal ) { return isnan( fVal ); }

TFORCEINLINE TINT FastMod( TINT a_iNum, TINT a_iModulus )
{
	TASSERT( a_iNum >= 0 );
	TASSERT( a_iModulus > 0 );
	TASSERT( 0 == ( a_iModulus & ( a_iModulus - 1 ) ) );
	return a_iNum & ( a_iModulus - 1 );
}

// $deBlob: FUNCTION 00666eb0
// $Barnyard: FUNCTION 006b7370
TFORCEINLINE void SinCos( TFLOAT fVal, TFLOAT& a_rSin, TFLOAT& a_rCos )
{
	a_rSin = TFLOAT( sin( fVal ) );
	a_rCos = TFLOAT( cos( fVal ) );
}

// $deBlob: FUNCTION 00667760
// $Barnyard: FUNCTION 006b6640
TFORCEINLINE TFLOAT LERP( TFLOAT a, TFLOAT b, TFLOAT t ) { return ( b - a ) * t + a; }

// $Barnyard: FUNCTION 006b6660
TFORCEINLINE TFLOAT LERPClamped( TFLOAT a, TFLOAT b, TFLOAT t )
{
	TFLOAT fResult = LERP( a, b, t );

	if ( ( a < b && b < fResult ) || ( b < a && fResult < b ) )
		return b;

	return fResult;
}

// $Barnyard: FUNCTION 006b6810
TFORCEINLINE void NormaliseAngle( TFLOAT& a_rfValue )
{
	if ( PI < a_rfValue )
		a_rfValue = fmodf( a_rfValue + PI, PI * 2 ) - PI;
	else if ( a_rfValue < -PI )
		a_rfValue = fmodf( a_rfValue - PI, PI * 2 ) + PI;
}

// Interpolates from the angle a_fFrom to a_fTo (radians) by a_fProgress, going the short way around
// $Barnyard: FUNCTION 006b8f40
inline TFLOAT LerpAngle( TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fProgress )
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

// Adds a_fAmount to a_rfValue without going above a_fMax (out of line in TMath.cpp)
void IncrMaxf( TFLOAT& a_rfValue, TFLOAT a_fAmount, TFLOAT a_fMax );

// Subtracts a_fAmount from a_rfValue without going below a_fMin (out of line in TMath.cpp)
void DecrMinf( TFLOAT& a_rfValue, TFLOAT a_fAmount, TFLOAT a_fMin );

// Moves a_rfValue towards a_fTarget by a_fStep without going past it
// NOTE: the name is unknown
// $Barnyard: FUNCTION 006b66b0
TFORCEINLINE void MoveTowards( TFLOAT& a_rfValue, TFLOAT a_fTarget, TFLOAT a_fStep )
{
	if ( a_rfValue < a_fTarget )
		IncrMaxf( a_rfValue, a_fStep, a_fTarget );
	else
		DecrMinf( a_rfValue, a_fStep, a_fTarget );
}

// Wraps a_rfValue to a_fMin when it goes above a_fMax and to a_fMax when it goes below a_fMin
// $deBlob: FUNCTION 006674b0
// $deBlob: FUNCTION 006674e0
// $Barnyard: FUNCTION 006b68c0
template <typename T>
TFORCEINLINE void Wrap( T& a_rfValue, T a_fMin, T a_fMax )
{
	if ( a_rfValue > a_fMax )
		a_rfValue = a_fMin;
	else if ( a_rfValue < a_fMin )
		a_rfValue = a_fMax;
}

// $deBlob: FUNCTION 00667460
// $deBlob: FUNCTION 00667490
// $Barnyard: FUNCTION 006b6870
// $Barnyard: FUNCTION 006b68a0
template <typename T>
TFORCEINLINE void Clip( T& rVal, const T& Min, const T& Max )
{
	// NOTE: else-if like the original: with Min > Max a value above Max ends up at Max, not at Min
	if ( Max < rVal ) { rVal = Max; }
	else if ( rVal < Min ) { rVal = Min; }
}

// Out of line instances: Min<TINT> 004024d0, Min<TFLOAT> 00438df0
// $Barnyard: FUNCTION 004024d0
// $Barnyard: FUNCTION 00438df0
template <typename T>
TFORCEINLINE const T& Min( const T& a, const T& b )
{
	// NOTE: returns a when both are equal, like the original
	return ( b < a ) ? b : a;
}

// Out of line instances: Max<TFLOAT> 00408660
// $Barnyard: FUNCTION 00408660
template <typename T>
TFORCEINLINE const T& Max( const T& a, const T& b )
{
	return ( a < b ) ? b : a;
}

// Perlin's bias: remaps a_fValue (0-1) so 0.5 becomes a_fBias (out of line in TMath.cpp)
TFLOAT Bias( TFLOAT a_fBias, TFLOAT a_fValue );

// Perlin's gain: an s-curve, a_fGain controls the steepness (out of line in TMath.cpp)
TFLOAT Gain( TFLOAT a_fGain, TFLOAT a_fValue );

// Same as Gain( 0.75f, a_fValue ) (out of line in TMath.cpp)
TFLOAT Gain075( TFLOAT a_fValue );

} // namespace TMath

TOSHI_NAMESPACE_END

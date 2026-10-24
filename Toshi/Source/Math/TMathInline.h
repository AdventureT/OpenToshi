#pragma once
#include "Toshi/Defines.h"
#include "Toshi/TDebug.h"

#include <cstdint>
#include <cfloat>
#include <cmath>

TOSHI_NAMESPACE_START

namespace TMath
{

TCONSTEXPR_VAR TINT8  TINT8_MAX  = INT8_MAX;
TCONSTEXPR_VAR TINT8  TINT8_MIN  = INT8_MIN;
TCONSTEXPR_VAR TUINT8 TUINT8_MAX = UINT8_MAX;

TCONSTEXPR_VAR TUINT TINT16_MAX  = INT16_MAX;
TCONSTEXPR_VAR TUINT TINT16_MIN  = INT16_MIN;
TCONSTEXPR_VAR TUINT TUINT16_MAX = UINT16_MAX;

TCONSTEXPR_VAR TINT  TINT32_MAX  = INT32_MAX;
TCONSTEXPR_VAR TINT  TINT32_MIN  = INT32_MIN;
TCONSTEXPR_VAR TUINT TUINT32_MAX = UINT32_MAX;

TCONSTEXPR_VAR TUINT16 MAXWCHAR = WCHAR_MAX;

TCONSTEXPR_VAR TUINTPTR MAXPTR = UINTPTR_MAX;

TCONSTEXPR_VAR TFLOAT MAXFLOAT       = FLT_MAX;
TCONSTEXPR_VAR TFLOAT MINFLOAT       = FLT_MIN;
TCONSTEXPR_VAR TFLOAT TFLOAT_EPSILON = FLT_EPSILON; // Renamed according to de Blob
TCONSTEXPR_VAR TFLOAT FLOATEPSILON   = FLT_EPSILON; // Both exist for some reason
TCONSTEXPR_VAR TFLOAT FLOAT_NAN      = NAN;

TCONSTEXPR_VAR TFLOAT PI               = 3.1415927f;
TCONSTEXPR_VAR TFLOAT TWO_PI           = PI * 2;
TCONSTEXPR_VAR TFLOAT HALF_PI          = PI / 2;
TCONSTEXPR_VAR TFLOAT ONEOVER_SQRT_TWO = 0.70710677f;
TCONSTEXPR_VAR TFLOAT ONEOVERTWO_PI    = 1 / TWO_PI;

TFORCEINLINE TBOOL IsFinite( TFLOAT fVal ) { return _finite( fVal ) != 0; }
// NOTE: de Blob calls the double precision CRT functions and rounds the results to float
TFORCEINLINE TFLOAT Sin( TFLOAT fVal ) { return TFLOAT( sin( fVal ) ); }
TFORCEINLINE TFLOAT Cos( TFLOAT fVal ) { return TFLOAT( cos( fVal ) ); }
TFORCEINLINE TFLOAT Tan( TFLOAT fVal ) { return TFLOAT( tan( fVal ) ); }

TFLOAT ASin( TFLOAT a_fX );
TFLOAT ACos( TFLOAT a_fX );
TFLOAT ATan( TFLOAT a_fX );
TFLOAT Exp( TFLOAT a_fX );
TFLOAT Log( TFLOAT a_fX );
TFLOAT Log10( TFLOAT a_fX );
TFLOAT Pow( TFLOAT a_fX, TFLOAT a_fY );

// $deBlob: FUNCTION 0058da50
inline TFLOAT ATan2( TFLOAT fVal1, TFLOAT fVal2 )
{
	TFLOAT fVal = TFLOAT( atan2( fVal1, fVal2 ) );
	TASSERT( TMath::IsFinite( fVal ) );
	return fVal;
}
TFORCEINLINE TFLOAT Abs( TFLOAT fVal ) { return (TFLOAT)fabs( fVal ); }

// Returns 1, -1 or 0
// $deBlob: FUNCTION 005d96e0
// $Barnyard: FUNCTION 006b68f0
inline TFLOAT Sign( TFLOAT a_fVal )
{
	TASSERT( !isnan( a_fVal ) );
	if ( a_fVal > 0.0f ) return 1.0f;
	if ( a_fVal < 0.0f ) return -1.0f;
	return 0.0f;
}
TFORCEINLINE TCONSTEXPR TFLOAT DegToRad( TFLOAT fDeg ) { return ( fDeg / 180.0f ) * PI; }

TFORCEINLINE TCONSTEXPR TUINT IntLog2( TUINT32 a_uiValue )
{
	return TBitWidth( a_uiValue ) - 1;
}

inline TCONSTEXPR TUINT RoundToNextPowerOfTwo( TUINT a_uiValue )
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
	// NOTE: retail has no asserts here (Barnyard asserts a_fX >= 0 and a finite result); the square root of zero is valid
	return sqrtf( a_fX );
}

// $deBlob: FUNCTION 00673cb0
inline TFLOAT OneOverSqrt( TFLOAT a_fX )
{
	TASSERT( a_fX != 0.0f );
	TFLOAT fVal = 1.0f / TFLOAT( sqrt( a_fX ) );
	TASSERT( TMath::IsFinite( fVal ) );
	return fVal;
}

TFORCEINLINE TINT Round( TFLOAT a_fVal ) { return (TINT)roundf( a_fVal ); }

// NOTE: the original rounds with fist and then adds one if the difference is positive (compares the float's bits)
TINT CeilToInt( TFLOAT a_fVal );

// the original rounds with fist and then moves the result one towards zero if it passed the value (compares the
// float's bits)
TINT TruncToInt( TFLOAT a_fVal );

// NOTE: the original rounds with fist and then subtracts one if the difference is negative (compares the float's bits)
TINT FloorToInt( TFLOAT a_fVal );

// Moves a_fFrom towards a_fTo by a_fT without passing it
TFLOAT LerpTowards( TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fT );

TFORCEINLINE TBOOL IsNaN( TFLOAT fVal ) { return isnan( fVal ); }

// $deBlob: FUNCTION 005f6fd0
inline TINT FastMod( TINT a_iNum, TINT a_iModulus )
{
	TASSERT( a_iNum >= 0 );
	TASSERT( a_iModulus > 0 );
	TASSERT( 0 == ( a_iModulus & ( a_iModulus - 1 ) ) );
	return a_iNum & ( a_iModulus - 1 );
}

void SinCos( TFLOAT fVal, TFLOAT& a_rSin, TFLOAT& a_rCos );

TFLOAT LERP( TFLOAT a, TFLOAT b, TFLOAT t );

// $Barnyard: FUNCTION 006b6660
TFORCEINLINE TFLOAT LERPClamped( TFLOAT a, TFLOAT b, TFLOAT t )
{
	TFLOAT fResult = LERP( a, b, t );

	if ( ( a < b && b < fResult ) || ( b < a && fResult < b ) )
		return b;

	return fResult;
}

// $deBlob: FUNCTION 0066d700
// $Barnyard: FUNCTION 006b6810
TFORCEINLINE void NormaliseAngle( TFLOAT& a_rfValue )
{
	if ( PI < a_rfValue )
		a_rfValue = fmodf( a_rfValue + PI, PI * 2 ) - PI;
	else if ( a_rfValue < -PI )
		a_rfValue = fmodf( a_rfValue - PI, PI * 2 ) + PI;
}

// Interpolates from the angle a_fFrom to a_fTo (radians) by a_fProgress, going the short way around
TFLOAT LerpAngle( TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fProgress );

// Adds a_fAmount to a_rfValue without going above a_fMax
void IncrMaxf( TFLOAT& a_rfValue, TFLOAT a_fAmount, TFLOAT a_fMax );

// Subtracts a_fAmount from a_rfValue without going below a_fMin
void DecrMinf( TFLOAT& a_rfValue, TFLOAT a_fAmount, TFLOAT a_fMin );

// Moves a_rfValue towards a_fTarget by a_fStep without going past it
// $Barnyard: FUNCTION 006b66b0
TFORCEINLINE void MoveTowards( TFLOAT& a_rfValue, TFLOAT a_fTarget, TFLOAT a_fStep )
{
	if ( a_rfValue < a_fTarget )
		IncrMaxf( a_rfValue, a_fStep, a_fTarget );
	else
		DecrMinf( a_rfValue, a_fStep, a_fTarget );
}

// Wraps a_rfValue to a_fMin when it goes above a_fMax and to a_fMax when it goes below a_fMin
// Explicitly instantiated for TFLOAT and TINT in TMath.cpp
template <typename T>
void Wrap( T& a_rfValue, T a_fMin, T a_fMax );

// Explicitly instantiated for TFLOAT and TINT in TMath.cpp
template <typename T>
void Clip( T& rVal, T Min, T Max );

// $Barnyard: FUNCTION 004024d0
// $Barnyard: FUNCTION 00438df0
template <typename T>
TFORCEINLINE const T& Min( const T& a, const T& b )
{
	// NOTE: returns a when both are equal, like the original
	return ( b < a ) ? b : a;
}

// $deBlob: FUNCTION 007200f0
// $Barnyard: FUNCTION 00408660
template <typename T>
TFORCEINLINE const T& Max( const T& a, const T& b )
{
	return ( a < b ) ? b : a;
}

// Perlin's bias: remaps a_fValue (0-1) so 0.5 becomes a_fBias
TFLOAT Bias( TFLOAT a_fBias, TFLOAT a_fValue );

// Perlin's gain: an s-curve, a_fGain controls the steepness
TFLOAT Gain( TFLOAT a_fGain, TFLOAT a_fValue );

// Same as Gain( 0.75f, a_fValue )
TFLOAT Gain075( TFLOAT a_fValue );

// $deBlob: FUNCTION 005f71e0
TFORCEINLINE TINT8 Int32ToInt8( TINT a_iX )
{
	TASSERT( ( a_iX >= TINT8_MIN ) && ( a_iX <= TINT8_MAX ) );
	return TINT8( a_iX );
}

// $deBlob: FUNCTION 00661ab0
TFORCEINLINE TUINT8 Int32ToUInt8( TINT a_iX )
{
	TASSERT( ( a_iX >= 0 ) && ( a_iX <= TINT( TUINT8_MAX ) ) );
	return TUINT8( a_iX );
}

// $deBlob: FUNCTION 00617d10
// $deBlob: FUNCTION 00617d70
inline TINT16 Int32ToInt16( TINT a_iX )
{
	TASSERT( ( a_iX >= TINT( TINT16_MIN ) ) && ( a_iX <= TINT( TINT16_MAX ) ) );
	return TINT16( a_iX );
}

// $deBlob: FUNCTION 007187e0
// $deBlob: FUNCTION 005f7230
// $deBlob: FUNCTION 00603440
inline TUINT16 Int32ToUInt16( TINT a_iX )
{
	TASSERT( ( a_iX >= 0 ) && ( a_iX <= TINT( TUINT16_MAX ) ) );
	return TUINT16( a_iX );
}

// $deBlob: FUNCTION 00712660
// $deBlob: FUNCTION 005f9f40
TFORCEINLINE TUINT16 UInt32ToUInt16( TUINT a_iX )
{
	TASSERT( a_iX <= TUINT16_MAX );
	return TUINT16( a_iX );
}

// $deBlob: FUNCTION 006eed30
TFORCEINLINE TINT32 UInt32ToInt32( TUINT32 a_iX )
{
	TASSERT( a_iX <= TUINT32( TINT32_MAX ) );
	return TINT32( a_iX );
}

} // namespace TMath

TOSHI_NAMESPACE_END

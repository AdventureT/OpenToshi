/**
 * @file TQuaternion.h
 * Quaternion implementation for the Toshi engine
 * 
 * This file provides quaternion functionality including:
 * - Quaternion operations (multiplication, normalization)
 * - Rotation representation and conversion
 * - Quaternion interpolation
 * - Quaternion to matrix conversion
 */

#pragma once
#include "Math/TVector3.h"
#include "Math/TVector4.h"

TOSHI_NAMESPACE_START

class TQuaternion
{
public:
	TQuaternion()
	    : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 1.0f )
	{}

	TQuaternion( const TQuaternion& a_pQuaternion )
	    : x( a_pQuaternion.x ), y( a_pQuaternion.y ), z( a_pQuaternion.z ), w( a_pQuaternion.w ) {}

	TQuaternion( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW )
	    : x( a_fX ), y( a_fY ), z( a_fZ ), w( a_fW ) {}

	void Set( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW )
	{
		x = a_fX;
		y = a_fY;
		z = a_fZ;
		w = a_fW;
	}

	void SetIdentity()
	{
		Set(
		    TQuaternion::IDENTITY.x,
		    TQuaternion::IDENTITY.y,
		    TQuaternion::IDENTITY.z,
		    TQuaternion::IDENTITY.w
		);
	}

	void SetFromEulerYX( const TFLOAT* fVal );
	void SetFromEulerRollPitchYaw( TFLOAT a_fRoll, TFLOAT a_fPitch, TFLOAT a_fYaw );

	// $deBlob: FUNCTION 00676320
	inline void SetConjugate() { AsVector4().Negate3(); }
	inline void SetInverse() { SetConjugate(); }
	void        SetRotation( const TVector3& a_rVec3, TFLOAT a_fVal );
	void        SetVectorDirection( const TVector3& a_rvDest, const TVector3& a_rvSource );

	static void RotateVector( TVector3& param_1, const TQuaternion& param_2, const TVector3 param_3 );
	static void RotateVector( TVector4& a_rOut, const TQuaternion& a_rcQuat, const TVector4& a_rcVec );
	void        RotateAroundAxis( const TVector3& param_1, TFLOAT param_2 );

	// $deBlob: FUNCTION 00674cc0
	void Negate() { Set( -x, -y, -z, -w ); }
	void GetEulerXYZ( TVector3& outVec ) const;

	TFLOAT Magnitude() const { return TMath::Sqrt( w * w + z * z + y * y + x * x ); }
	TFLOAT MagnitudeSq() const { return w * w + z * z + y * y + x * x; }

	// $deBlob: FUNCTION 00676280
	TFLOAT DotProduct( const TQuaternion& a_rcQuat ) const { return TVector4::DotProduct4( AsVector4(), a_rcQuat.AsVector4() ); }

	const TVector4& AsVector4() const { return reinterpret_cast<const TVector4&>( *this ); }
	TVector4&       AsVector4() { return reinterpret_cast<TVector4&>( *this ); }

	void         operator=( const TQuaternion& other ) { Set( other.x, other.y, other.z, other.w ); }
	TQuaternion& operator*=( const TQuaternion& a_rQuat );

	// $deBlob: FUNCTION 00674500
	TQuaternion& operator*=( TFLOAT a_fScalar )
	{
		x *= a_fScalar;
		y *= a_fScalar;
		z *= a_fScalar;
		w *= a_fScalar;
		return *this;
	}

	// $deBlob: FUNCTION 00674520
	TQuaternion& operator/=( const TQuaternion& a_rcQuat )
	{
		x /= a_rcQuat.x;
		y /= a_rcQuat.y;
		z /= a_rcQuat.z;
		w /= a_rcQuat.w;
		return *this;
	}

	// $deBlob: FUNCTION 00674570
	TQuaternion& operator/=( TFLOAT a_fScalar )
	{
		if ( a_fScalar != 0.0f )
		{
			TFLOAT fRatio = 1.0f / a_fScalar;
			x *= fRatio;
			y *= fRatio;
			z *= fRatio;
			w *= fRatio;
		}

		return *this;
	}

	// $deBlob: FUNCTION 006745b0
	TQuaternion& operator+=( const TQuaternion& a_rcQuat )
	{
		x = a_rcQuat.x + x;
		y = a_rcQuat.y + y;
		z = a_rcQuat.z + z;
		w = a_rcQuat.w + w;
		return *this;
	}

	// $deBlob: FUNCTION 00674600
	TQuaternion& operator-=( const TQuaternion& a_rcQuat )
	{
		x -= a_rcQuat.x;
		y -= a_rcQuat.y;
		z -= a_rcQuat.z;
		w -= a_rcQuat.w;
		return *this;
	}

	// $deBlob: FUNCTION 00674650
	TQuaternion operator+( const TQuaternion& a_rcQuat ) const
	{
		TQuaternion result = *this;
		result += a_rcQuat;
		return result;
	}

	// $deBlob: FUNCTION 006746a0
	TQuaternion operator-( const TQuaternion& a_rcQuat ) const
	{
		TQuaternion result = *this;
		result -= a_rcQuat;
		return result;
	}

	// $deBlob: FUNCTION 006746f0
	// $Barnyard: FUNCTION 006c9290
	TQuaternion operator*( const TQuaternion& a_rcQuat ) const
	{
		TQuaternion result = *this;
		result *= a_rcQuat;
		return result;
	}

	// $deBlob: FUNCTION 00674800
	TQuaternion operator*( TFLOAT a_fScalar ) const
	{
		TQuaternion result = *this;
		result *= a_fScalar;
		return result;
	}

	// $deBlob: FUNCTION 00674830
	TQuaternion operator/( const TQuaternion& a_rcQuat ) const
	{
		TQuaternion result = *this;
		result /= a_rcQuat;
		return result;
	}

	// $deBlob: FUNCTION 00674880
	TQuaternion operator/( TFLOAT a_fScalar ) const
	{
		TQuaternion result = *this;
		result /= a_fScalar;
		return result;
	}

	// $deBlob: FUNCTION 006748c0
	TBOOL operator==( const TQuaternion& a_rcQuat ) const { return x == a_rcQuat.x && y == a_rcQuat.y && z == a_rcQuat.z && w == a_rcQuat.w; }

	// $deBlob: FUNCTION 00674910
	TBOOL operator!=( const TQuaternion& a_rcQuat ) const { return !( *this == a_rcQuat ); }

	// $deBlob: FUNCTION 00674960
	TBOOL IsNormalised( TFLOAT a_fTolerance ) const
	{
		TFLOAT fMagSq = x * x + y * y + z * z + w * w;
		return ( 1.0f - a_fTolerance ) * ( 1.0f - a_fTolerance ) < fMagSq && fMagSq < ( a_fTolerance + 1.0f ) * ( a_fTolerance + 1.0f );
	}

	static void Add( TQuaternion& a_rOut, const TQuaternion& a_rcQuat1, const TQuaternion& a_rcQuat2 );
	static void Subtract( TQuaternion& a_rOut, const TQuaternion& a_rcQuat1, const TQuaternion& a_rcQuat2 );
	static void Multiply( TQuaternion& a_rOut, const TQuaternion& a_rcQuat1, const TQuaternion& a_rcQuat2 );
	static void Divide( TQuaternion& a_rOut, const TQuaternion& a_rcQuat1, const TQuaternion& a_rcQuat2 );

	static void Nlerp( TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, float a_fProgress );
	static void Slerp( TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, float a_fProgress );

	// Linear interpolation along the shortest path, the result is not normalised
	static void Lerp( TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress );

	// Like Nlerp, without the normalisation
	void Lerp( const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress );

public:
	static const TQuaternion       IDENTITY;
	static inline constexpr TFLOAT DELTA = 0.001f;

public:
	TFLOAT x, y, z, w;
};

TOSHI_NAMESPACE_END

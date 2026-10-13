/**
 * @file TMatrix44.h
 * 4x4 matrix implementation for the Toshi engine
 * 
 * This file provides 4x4 matrix functionality including:
 * - Matrix transformations (translation, rotation, scaling)
 * - Matrix operations (multiplication, inversion)
 * - View and projection matrix handling
 * - Matrix decomposition and composition
 */

#pragma once
#include "TQuaternion.h"
#include "TPlane.h"

#ifdef TRENDERINTERFACE_DX8
#  include <d3dx8math.h>
#endif

TOSHI_NAMESPACE_START

using BASISVECTOR = TINT;
enum BASISVECTOR_ : BASISVECTOR
{
	BASISVECTOR_RIGHT,
	BASISVECTOR_UP,
	BASISVECTOR_FORWARD,
	BASISVECTOR_TRANSLATION
};

class TMatrix44
{
public:
	constexpr TMatrix44() = default;

	constexpr TMatrix44( const TMatrix44& a_rMatrix )
	    : m_f11( a_rMatrix.m_f11 ), m_f12( a_rMatrix.m_f12 ), m_f13( a_rMatrix.m_f13 ), m_f14( a_rMatrix.m_f14 ), m_f21( a_rMatrix.m_f21 ), m_f22( a_rMatrix.m_f22 ), m_f23( a_rMatrix.m_f23 ), m_f24( a_rMatrix.m_f24 ), m_f31( a_rMatrix.m_f31 ), m_f32( a_rMatrix.m_f32 ), m_f33( a_rMatrix.m_f33 ), m_f34( a_rMatrix.m_f34 ), m_f41( a_rMatrix.m_f41 ), m_f42( a_rMatrix.m_f42 ), m_f43( a_rMatrix.m_f43 ), m_f44( a_rMatrix.m_f44 )
	{}

	// $Barnyard: FUNCTION 0041fab0
	constexpr TMatrix44(
	    TFLOAT a_f11,
	    TFLOAT a_f12,
	    TFLOAT a_f13,
	    TFLOAT a_f14,
	    TFLOAT a_f21,
	    TFLOAT a_f22,
	    TFLOAT a_f23,
	    TFLOAT a_f24,
	    TFLOAT a_f31,
	    TFLOAT a_f32,
	    TFLOAT a_f33,
	    TFLOAT a_f34,
	    TFLOAT a_f41,
	    TFLOAT a_f42,
	    TFLOAT a_f43,
	    TFLOAT a_f44
	)
	    : m_f11( a_f11 ), m_f12( a_f12 ), m_f13( a_f13 ), m_f14( a_f14 ), m_f21( a_f21 ), m_f22( a_f22 ), m_f23( a_f23 ), m_f24( a_f24 ), m_f31( a_f31 ), m_f32( a_f32 ), m_f33( a_f33 ), m_f34( a_f34 ), m_f41( a_f41 ), m_f42( a_f42 ), m_f43( a_f43 ), m_f44( a_f44 )
	{}

	// $Barnyard: FUNCTION 0041fa30
	constexpr void Set(
	    TFLOAT a_f11,
	    TFLOAT a_f12,
	    TFLOAT a_f13,
	    TFLOAT a_f14,
	    TFLOAT a_f21,
	    TFLOAT a_f22,
	    TFLOAT a_f23,
	    TFLOAT a_f24,
	    TFLOAT a_f31,
	    TFLOAT a_f32,
	    TFLOAT a_f33,
	    TFLOAT a_f34,
	    TFLOAT a_f41,
	    TFLOAT a_f42,
	    TFLOAT a_f43,
	    TFLOAT a_f44
	)
	{
		m_f11 = a_f11;
		m_f12 = a_f12;
		m_f13 = a_f13;
		m_f14 = a_f14;
		m_f21 = a_f21;
		m_f22 = a_f22;
		m_f23 = a_f23;
		m_f24 = a_f24;
		m_f31 = a_f31;
		m_f32 = a_f32;
		m_f33 = a_f33;
		m_f34 = a_f34;
		m_f41 = a_f41;
		m_f42 = a_f42;
		m_f43 = a_f43;
		m_f44 = a_f44;
	}

	constexpr void Set( const TMatrix44& a_rcMatrix )
	{
		Set(
		    a_rcMatrix.m_f11, a_rcMatrix.m_f12, a_rcMatrix.m_f13, a_rcMatrix.m_f14,
		    a_rcMatrix.m_f21, a_rcMatrix.m_f22, a_rcMatrix.m_f23, a_rcMatrix.m_f24,
		    a_rcMatrix.m_f31, a_rcMatrix.m_f32, a_rcMatrix.m_f33, a_rcMatrix.m_f34,
		    a_rcMatrix.m_f41, a_rcMatrix.m_f42, a_rcMatrix.m_f43, a_rcMatrix.m_f44
		);
	}

	// NOTE: 006c8520 copies a static that is never written (all zeros) instead of the identity; its callers
	// overwrite or ignore the result
	// $deBlob: FUNCTION 00663680
	// $Barnyard: FUNCTION 006c8500
	// $Barnyard: FUNCTION 006c8520
	constexpr void Identity()
	{
		Set( IDENTITY );
	}

	const TVector3& AsBasisVector3( BASISVECTOR a_iIndex ) const
	{
		return *TREINTERPRETCAST(
		    TVector3*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	TVector3& AsBasisVector3( BASISVECTOR a_iIndex )
	{
		return *TREINTERPRETCAST(
		    TVector3*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	const TVector4& AsBasisVector4( BASISVECTOR a_iIndex ) const
	{
		return *TREINTERPRETCAST(
		    TVector4*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	TVector4& AsBasisVector4( BASISVECTOR a_iIndex )
	{
		return *TREINTERPRETCAST(
		    TVector4*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	TVector3& GetTranslation3()
	{
		return AsBasisVector3( BASISVECTOR_TRANSLATION );
	}

	const TVector3& GetTranslation3() const
	{
		return AsBasisVector3( BASISVECTOR_TRANSLATION );
	}

	TVector4& GetTranslation()
	{
		return AsBasisVector4( BASISVECTOR_TRANSLATION );
	}

	const TVector4& GetTranslation() const
	{
		return AsBasisVector4( BASISVECTOR_TRANSLATION );
	}

	// NOTE: an instance of it lives in the game code
	// $Barnyard: FUNCTION 0045d950
	void SetBasisVector4( BASISVECTOR a_iIndex, const TVector4& a_rcVector )
	{
		AsBasisVector4( a_iIndex ) = a_rcVector;
	}

	// NOTE: an instance of it lives in the game code
	// $Barnyard: FUNCTION 00420450
	void GetTranslation( TVector4& a_rOutTranslation ) const
	{
		a_rOutTranslation = AsBasisVector4( BASISVECTOR_TRANSLATION );
	}

	// NOTE: an instance of it lives in the game code
	// $Barnyard: FUNCTION 00415a20
	void GetBasisVector4( BASISVECTOR a_iIndex, TVector4& a_rOutVector ) const
	{
		a_rOutVector = AsBasisVector4( a_iIndex );
	}

	void SetTranslation( const TVector4& a_rTranslation )
	{
		GetTranslation() = a_rTranslation;
	}

	void LookAtTarget( const TVector4& a_rTarget, const TVector4& a_rUp );
	void LookAtDirection( const TVector4& a_rVec, const TVector4& a_rVec2 );
	void LookAtDirection( const TVector3& a_rVec, const TVector3& a_rVec2 );

	TFLOAT Determinant() const;
	TFLOAT Determinant4() const;
	TBOOL  Invert4( const TMatrix44& a_rcMatrix );
	TBOOL  InvertArbitrary( const TMatrix44& a_rcMatrix );

	// this = a + b (all 16 elements)
	// $deBlob: FUNCTION 00664e60
	void Add( const TMatrix44& a_rcA, const TMatrix44& a_rcB )
	{
		for ( TINT i = 0; i < 4; i++ )
		{
			AsBasisVector4( i ).x = a_rcB.AsBasisVector4( i ).x + a_rcA.AsBasisVector4( i ).x;
			AsBasisVector4( i ).y = a_rcA.AsBasisVector4( i ).y + a_rcB.AsBasisVector4( i ).y;
			AsBasisVector4( i ).z = a_rcA.AsBasisVector4( i ).z + a_rcB.AsBasisVector4( i ).z;
			AsBasisVector4( i ).w = a_rcB.AsBasisVector4( i ).w + a_rcA.AsBasisVector4( i ).w;
		}
	}

	// this = a - b (all 16 elements)
	// $deBlob: FUNCTION 00664ef0
	void Subtract( const TMatrix44& a_rcA, const TMatrix44& a_rcB )
	{
		for ( TINT i = 0; i < 4; i++ )
		{
			AsBasisVector4( i ).x = a_rcA.AsBasisVector4( i ).x - a_rcB.AsBasisVector4( i ).x;
			AsBasisVector4( i ).y = a_rcA.AsBasisVector4( i ).y - a_rcB.AsBasisVector4( i ).y;
			AsBasisVector4( i ).z = a_rcA.AsBasisVector4( i ).z - a_rcB.AsBasisVector4( i ).z;
			AsBasisVector4( i ).w = a_rcA.AsBasisVector4( i ).w - a_rcB.AsBasisVector4( i ).w;
		}
	}

	// $deBlob: FUNCTION 006653a0
	TBOOL IsEqual( const TMatrix44& a_rcMatrix ) const
	{
		for ( TINT i = 0; i < 4; i++ )
			for ( TINT k = 0; k < 4; k++ )
				if ( AsBasisVector4( i ).AsArray()[ k ] != a_rcMatrix.AsBasisVector4( i ).AsArray()[ k ] ) return TFALSE;

		return TTRUE;
	}

	// The three basis vectors are perpendicular to each other
	// $deBlob: FUNCTION 006653f0
	TBOOL IsOrthogonal() const
	{
		TFLOAT fDot12 = m_f22 * m_f12 + m_f11 * m_f21 + m_f23 * m_f13;
		TFLOAT fDot13 = m_f32 * m_f12 + m_f31 * m_f11 + m_f33 * m_f13;
		TFLOAT fDot23 = m_f22 * m_f32 + m_f21 * m_f31 + m_f23 * m_f33;
		return fDot13 * fDot13 + fDot12 * fDot12 + fDot23 * fDot23 < 0.01f;
	}

	// $deBlob: FUNCTION 00665790
	void GetScale( TVector4& a_rOutScale ) const
	{
		TFLOAT fX = AsBasisVector4( 0 ).Magnitude();
		TFLOAT fY = AsBasisVector4( 1 ).Magnitude();
		TFLOAT fZ = AsBasisVector4( 2 ).Magnitude();
		a_rOutScale.Set( fX, fY, fZ, 1.0f );
	}

	void Lerp( const TMatrix44& a_rFrom, const TMatrix44& a_rTo, TFLOAT a_flProgress );

	// $deBlob: FUNCTION 00665660
	// $Barnyard: FUNCTION 006c86f0
	constexpr void Scale( TFLOAT a_fScalar1, TFLOAT a_fScalar2, TFLOAT a_fScalar3 )
	{
		m_f11 *= a_fScalar1;
		m_f12 *= a_fScalar1;
		m_f13 *= a_fScalar1;
		m_f21 *= a_fScalar2;
		m_f22 *= a_fScalar2;
		m_f23 *= a_fScalar2;
		m_f31 *= a_fScalar3;
		m_f32 *= a_fScalar3;
		m_f33 *= a_fScalar3;
	}

	// $deBlob: FUNCTION 00665700
	// $Barnyard: FUNCTION 006c8750
	constexpr void Scale( const TVector4& a_rScalars )
	{
		m_f11 *= a_rScalars.x;
		m_f12 *= a_rScalars.x;
		m_f13 *= a_rScalars.x;
		m_f21 *= a_rScalars.y;
		m_f22 *= a_rScalars.y;
		m_f23 *= a_rScalars.y;
		m_f31 *= a_rScalars.z;
		m_f32 *= a_rScalars.z;
		m_f33 *= a_rScalars.z;
	}

	// $deBlob: FUNCTION 006650d0
	// $Barnyard: FUNCTION 006c86a0
	void ScaleAll( const TMatrix44& a_rMatrix, TFLOAT a_fScalar )
	{
		for ( TINT i = 0; i < 4; i++ )
		{
			TVector4& basis = AsBasisVector4( i );

			basis.x = a_rMatrix.AsBasisVector4( i ).x * a_fScalar;
			basis.y = a_rMatrix.AsBasisVector4( i ).y * a_fScalar;
			basis.z = a_rMatrix.AsBasisVector4( i ).z * a_fScalar;
			basis.w = a_rMatrix.AsBasisVector4( i ).w * a_fScalar;
		}
	}

	constexpr void Scale( TFLOAT a_fScale )
	{
		Scale( a_fScale, a_fScale, a_fScale );
	}

	// $deBlob: FUNCTION 00665490
	constexpr TBOOL IsOrthonormal() const
	{
		TFLOAT fVar6 = m_f22 * m_f12 + m_f11 * m_f21 + m_f23 * m_f13;
		TFLOAT fVar4 = m_f32 * m_f12 + m_f11 * m_f31 + m_f33 * m_f13;
		TFLOAT fVar5 = ( m_f11 * m_f11 + m_f12 * m_f12 + m_f13 * m_f13 ) - 1.0f;
		TFLOAT fVar3 = m_f22 * m_f32 + m_f21 * m_f31 + m_f23 * m_f33;
		TFLOAT fVar2 = ( m_f22 * m_f22 + m_f21 * m_f21 + m_f23 * m_f23 ) - 1.0f;
		TFLOAT fVar1 = ( m_f32 * m_f32 + m_f31 * m_f31 + m_f33 * m_f33 ) - 1.0f;

		return fVar2 * fVar2 + fVar5 * fVar5 + fVar1 * fVar1 + fVar4 * fVar4 + fVar6 * fVar6 + fVar3 * fVar3 < 0.01f &&
		    m_f14 == 0.0f && m_f24 == 0.0f && m_f34 == 0.0f && m_f44 == 1.0f;
	}

	// $deBlob: FUNCTION 00664d60
	// $Barnyard: FUNCTION 006c8540
	void Transpose( const TMatrix44& a_rcMatrix )
	{
		TFLOAT f14 = a_rcMatrix.m_f41, f24 = a_rcMatrix.m_f42, f34 = a_rcMatrix.m_f43, f44 = a_rcMatrix.m_f44;
		TFLOAT f13 = a_rcMatrix.m_f31, f23 = a_rcMatrix.m_f32, f33 = a_rcMatrix.m_f33, f43 = a_rcMatrix.m_f34;
		TFLOAT f12 = a_rcMatrix.m_f21, f22 = a_rcMatrix.m_f22, f32 = a_rcMatrix.m_f23, f42 = a_rcMatrix.m_f24;
		TFLOAT f11 = a_rcMatrix.m_f11, f21 = a_rcMatrix.m_f12, f31 = a_rcMatrix.m_f13, f41 = a_rcMatrix.m_f14;
		Set( f11, f12, f13, f14, f21, f22, f23, f24, f31, f32, f33, f34, f41, f42, f43, f44 );
	}

	void Multiply( const TMatrix44& a_rLeft, const TMatrix44& a_rRight );
	void Multiply( const TMatrix44& a_rRight );

	// this = a_rcLeft * this
	// NOTE: the name is invented
	// $Barnyard: FUNCTION 00466830
	void PreMultiply( const TMatrix44& a_rcLeft )
	{
		TMatrix44 matCopy = *this;
		Multiply( a_rcLeft, matCopy );
	}

	// $Barnyard: FUNCTION 0046cca0
	TMatrix44 operator*( const TMatrix44& a_rcRight ) const
	{
		TMatrix44 matResult;
		matResult.Multiply( *this, a_rcRight );
		return matResult;
	}

	TBOOL Invert( const TMatrix44& a_rRight );

	void InvertOrthogonal();
	void InvertOrthogonal( const TMatrix44& a_rRight );
	void InvertOrthonormal();

	TMatrix44& SetFromQuaternion( const TQuaternion& a_rQuaternion );
	TMatrix44& PushQuaternion( const TQuaternion& a_rQuaternion, const TMatrix44& a_rMatrix, const TVector3& a_rOrigin );

	void RotateX( TFLOAT a_fAngle );
	void RotateY( TFLOAT a_fAngle );
	void RotateZ( TFLOAT a_fAngle );

	// $deBlob: FUNCTION 006657f0
	// $Barnyard: FUNCTION 006c9160
	void Rotate( TFLOAT a_fAngleX, TFLOAT a_fAngleY, TFLOAT a_fAngleZ )
	{
		RotateX( a_fAngleX );
		RotateY( a_fAngleY );
		RotateZ( a_fAngleZ );
	}

	void GetEulerXYZ( TVector3& a_rOutVec ) const;

	// $deBlob: FUNCTION 00666a60
	// $Barnyard: FUNCTION 006c8c20
	static void MatToQuat( TQuaternion& a_rOutQuat, const TMatrix44& a_rMatrix )
	{
		TFLOAT fVar1;
		TFLOAT fVar2;
		TFLOAT fVar3;
		TFLOAT fVar4;

		fVar1 = a_rMatrix.m_f11;
		fVar2 = a_rMatrix.m_f22;
		fVar3 = a_rMatrix.m_f33;

		// NOTE: de Blob's order of operations (Barnyard adds and subtracts in another order)
		fVar4 = fVar1 + fVar2 + fVar3;

		if ( 0.0f < fVar4 )
		{
			fVar1        = TMath::Sqrt( fVar4 + 1.0f ) * 0.5f;
			a_rOutQuat.w = fVar1;
			fVar1        = 0.25f / fVar1;
			a_rOutQuat.x = ( a_rMatrix.m_f23 - a_rMatrix.m_f32 ) * fVar1;
			a_rOutQuat.y = ( a_rMatrix.m_f31 - a_rMatrix.m_f13 ) * fVar1;
			a_rOutQuat.z = ( a_rMatrix.m_f12 - a_rMatrix.m_f21 ) * fVar1;
			return;
		}
		if ( fVar2 <= fVar1 )
		{
			if ( fVar3 < fVar1 )
			{
				fVar1        = TMath::Sqrt( ( ( fVar1 - fVar2 ) - fVar3 ) + 1.0f ) * 0.5f;
				a_rOutQuat.x = fVar1;
				fVar1        = 0.25f / fVar1;
				a_rOutQuat.w = ( a_rMatrix.m_f23 - a_rMatrix.m_f32 ) * fVar1;
				a_rOutQuat.y = ( a_rMatrix.m_f12 + a_rMatrix.m_f21 ) * fVar1;
				a_rOutQuat.z = ( a_rMatrix.m_f13 + a_rMatrix.m_f31 ) * fVar1;
				return;
			}
		}
		else if ( fVar3 <= fVar2 )
		{
			fVar1        = TMath::Sqrt( ( ( fVar2 - fVar3 ) - fVar1 ) + 1.0f ) * 0.5f;
			a_rOutQuat.y = fVar1;
			fVar1        = 0.25f / fVar1;
			a_rOutQuat.w = ( a_rMatrix.m_f31 - a_rMatrix.m_f13 ) * fVar1;
			a_rOutQuat.z = ( a_rMatrix.m_f23 + a_rMatrix.m_f32 ) * fVar1;
			a_rOutQuat.x = ( a_rMatrix.m_f21 + a_rMatrix.m_f12 ) * fVar1;
			return;
		}

		fVar1        = TMath::Sqrt( ( ( fVar3 - fVar1 ) - fVar2 ) + 1.0f ) * 0.5f;
		a_rOutQuat.z = fVar1;

		fVar1        = 0.25f / fVar1;
		a_rOutQuat.w = ( a_rMatrix.m_f12 - a_rMatrix.m_f21 ) * fVar1;
		a_rOutQuat.x = ( a_rMatrix.m_f31 + a_rMatrix.m_f13 ) * fVar1;
		a_rOutQuat.y = ( a_rMatrix.m_f32 + a_rMatrix.m_f23 ) * fVar1;
	}

	// NOTE: the vector/plane transforms below use de Blob's order of operations (Barnyard sums in another order)

	// $deBlob: FUNCTION 00666990
	// $Barnyard: FUNCTION 006c8bf0
	static void TransformPlaneOrthogonal( TPlane& a_rOutPlane, const TMatrix44& a_rMatrix, const TPlane& a_rcSourcePlane )
	{
		const TVector4& vNormal = a_rcSourcePlane.AsVector4();
		TFLOAT          fNX = vNormal.x, fNY = vNormal.y, fNZ = vNormal.z, fD = a_rcSourcePlane.GetD();

		TFLOAT fY = a_rMatrix.m_f12 * fNX + a_rMatrix.m_f22 * fNY + a_rMatrix.m_f32 * fNZ;
		TFLOAT fZ = a_rMatrix.m_f13 * fNX + a_rMatrix.m_f23 * fNY + a_rMatrix.m_f33 * fNZ;
		TFLOAT fX = a_rMatrix.m_f21 * fNY + fNX * a_rMatrix.m_f11 + a_rMatrix.m_f31 * fNZ;

		TVector4& vOut = a_rOutPlane.AsVector4();
		vOut.x         = fX;
		vOut.y         = fY;
		vOut.z         = fZ;
		a_rOutPlane.SetD( a_rMatrix.m_f42 * fY + fX * a_rMatrix.m_f41 + a_rMatrix.m_f43 * fZ + fD );
	}

	// Transforms a plane by any matrix (the inverse transpose is built from two vectors on the plane)
	static void TransformPlane( TPlane& a_rOutPlane, const TMatrix44& a_rMatrix, const TPlane& a_rcSourcePlane );

	// $deBlob: FUNCTION 00666330
	// $Barnyard: FUNCTION 006c8b20
	constexpr static void RotateVector( TVector3& a_rOutVector, const TMatrix44& a_rMatrix, const TVector3& a_rVector )
	{
		TFLOAT fX = a_rVector.x, fY = a_rVector.y, fZ = a_rVector.z;

		a_rOutVector.y = a_rMatrix.m_f12 * fX + a_rMatrix.m_f22 * fY + a_rMatrix.m_f32 * fZ;
		a_rOutVector.z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + a_rMatrix.m_f33 * fZ;
		a_rOutVector.x = a_rMatrix.m_f21 * fY + a_rMatrix.m_f11 * fX + a_rMatrix.m_f31 * fZ;
	}

	// $deBlob: FUNCTION 006663d0
	// $Barnyard: FUNCTION 006c8b80
	constexpr static void RotateVector( TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector )
	{
		TFLOAT fX = a_rVector.x, fY = a_rVector.y, fZ = a_rVector.z, fW = a_rVector.w;

		a_rOutVector.y = a_rMatrix.m_f12 * fX + a_rMatrix.m_f22 * fY + a_rMatrix.m_f32 * fZ;
		a_rOutVector.z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + a_rMatrix.m_f33 * fZ;
		a_rOutVector.w = fW;
		a_rOutVector.x = a_rMatrix.m_f21 * fY + a_rMatrix.m_f11 * fX + a_rMatrix.m_f31 * fZ;
	}

	// $deBlob: FUNCTION 00665e90
	// $Barnyard: FUNCTION 006c8a20
	constexpr static void TransformVector( TVector3& a_rOutVector, const TMatrix44& a_rMatrix, const TVector3& a_rVector )
	{
		TFLOAT fX = a_rVector.x, fY = a_rVector.y, fZ = a_rVector.z;

		a_rOutVector.x = a_rMatrix.m_f21 * fY + a_rMatrix.m_f11 * fX + a_rMatrix.m_f31 * fZ + a_rMatrix.m_f41;
		a_rOutVector.y = a_rMatrix.m_f12 * fX + a_rMatrix.m_f22 * fY + a_rMatrix.m_f32 * fZ + a_rMatrix.m_f42;
		a_rOutVector.z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + a_rMatrix.m_f33 * fZ + a_rMatrix.m_f43;
	}

	// $deBlob: FUNCTION 00666010
	// $Barnyard: FUNCTION 006c8a80
	constexpr static void TransformVector( TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector )
	{
		TFLOAT fX = a_rVector.x, fY = a_rVector.y, fZ = a_rVector.z, fW = a_rVector.w;

		a_rOutVector.z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + a_rMatrix.m_f33 * fZ + a_rMatrix.m_f43 * fW;
		a_rOutVector.w = a_rMatrix.m_f14 * fX + a_rMatrix.m_f24 * fY + a_rMatrix.m_f34 * fZ + a_rMatrix.m_f44 * fW;
		a_rOutVector.y = a_rMatrix.m_f12 * fX + a_rMatrix.m_f22 * fY + a_rMatrix.m_f32 * fZ + a_rMatrix.m_f42 * fW;
		a_rOutVector.x = a_rMatrix.m_f21 * fY + a_rMatrix.m_f11 * fX + a_rMatrix.m_f31 * fZ + a_rMatrix.m_f41 * fW;
	}

	// Batched versions (arrays of pointers / contiguous arrays)
	static void TransformVectorBatched( TVector3* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector3* const* a_ppVectors, TINT a_iBatchSize );
	static void TransformVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* const* a_ppVectors, TINT a_iBatchSize );
	static void TransformVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* a_pVectors, TINT a_iBatchSize );
	static void RotateVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* const* a_ppVectors, TINT a_BatchSize );
	static void RotateVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* a_pVectors, TINT a_BatchSize );

	// Out of line copy used by AGameTimeFX::operator=: 004928a0
	// $Barnyard: FUNCTION 004928a0
	constexpr TMatrix44& operator=( const TMatrix44& a_rcMatrix )
	{
		Set(
		    a_rcMatrix.m_f11, a_rcMatrix.m_f12, a_rcMatrix.m_f13, a_rcMatrix.m_f14,
		    a_rcMatrix.m_f21, a_rcMatrix.m_f22, a_rcMatrix.m_f23, a_rcMatrix.m_f24,
		    a_rcMatrix.m_f31, a_rcMatrix.m_f32, a_rcMatrix.m_f33, a_rcMatrix.m_f34,
		    a_rcMatrix.m_f41, a_rcMatrix.m_f42, a_rcMatrix.m_f43, a_rcMatrix.m_f44
		);

		return *this;
	}

#ifdef TRENDERINTERFACE_DX8
	operator D3DXMATRIX*()
	{
		return TREINTERPRETCAST( D3DXMATRIX*, this );
	}

	operator const D3DXMATRIX*() const
	{
		return TREINTERPRETCAST( const D3DXMATRIX*, this );
	}
#endif

public:
	static constinit TMatrix44 IDENTITY;

public:
	TFLOAT m_f11, m_f12, m_f13, m_f14;
	TFLOAT m_f21, m_f22, m_f23, m_f24;
	TFLOAT m_f31, m_f32, m_f33, m_f34;
	TFLOAT m_f41, m_f42, m_f43, m_f44;
};

TOSHI_NAMESPACE_END

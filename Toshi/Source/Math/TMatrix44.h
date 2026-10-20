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
	TCONSTEXPR TMatrix44() = default;

	// $deBlob: FUNCTION 0054ee50
	TMatrix44( const TMatrix44& a_rMatrix ) { memcpy( this, &a_rMatrix, sizeof( TMatrix44 ) ); }

	// $deBlob: FUNCTION 00572a40
	// $Barnyard: FUNCTION 0041fab0
	TCONSTEXPR TMatrix44(
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

	// $deBlob: FUNCTION 00550ed0
	// $Barnyard: FUNCTION 0041fa30
	TCONSTEXPR void Set(
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

	TCONSTEXPR void Set( const TMatrix44& a_rcMatrix )
	{
		Set(
		    a_rcMatrix.m_f11, a_rcMatrix.m_f12, a_rcMatrix.m_f13, a_rcMatrix.m_f14,
		    a_rcMatrix.m_f21, a_rcMatrix.m_f22, a_rcMatrix.m_f23, a_rcMatrix.m_f24,
		    a_rcMatrix.m_f31, a_rcMatrix.m_f32, a_rcMatrix.m_f33, a_rcMatrix.m_f34,
		    a_rcMatrix.m_f41, a_rcMatrix.m_f42, a_rcMatrix.m_f43, a_rcMatrix.m_f44
		);
	}

	// NOTE: retail copies a static that is never written (all zeros) instead of the identity; its callers
	// overwrite or ignore the result
	void Identity();

	// $deBlob: FUNCTION 00554b40
	const TVector3& AsBasisVector3( BASISVECTOR a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < 4 );
		return *TREINTERPRETCAST(
		    TVector3*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	// $deBlob: FUNCTION 00554bc0
	TVector3& AsBasisVector3( BASISVECTOR a_iIndex )
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < 4 );
		return *TREINTERPRETCAST(
		    TVector3*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	// $deBlob: FUNCTION 00555a50
	const TVector4& AsBasisVector4( BASISVECTOR a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < 4 );
		return *TREINTERPRETCAST(
		    TVector4*,
		    TREINTERPRETCAST( TUINTPTR, this ) + a_iIndex * sizeof( TVector4 )
		);
	}

	// $deBlob: FUNCTION 0055dcb0
	TVector4& AsBasisVector4( BASISVECTOR a_iIndex )
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < 4 );
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
	// $deBlob: FUNCTION 00555b30
	// $Barnyard: FUNCTION 0045d950
	void SetBasisVector4( BASISVECTOR a_iIndex, const TVector4& a_rcVector )
	{
		TASSERT( ( a_iIndex < 3 && a_rcVector.w == 0.0f ) || ( a_iIndex == 3 && a_rcVector.w == 1.0f ) );
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < 4 );
		AsBasisVector4( a_iIndex ) = a_rcVector;
	}

	// Sets the x, y and z of a column (w untouched)
	// $deBlob: FUNCTION 0076c250
	void SetBasisVector3( BASISVECTOR a_iIndex, const TVector3& a_rcVector )
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < 4 );
		AsBasisVector3( a_iIndex ) = a_rcVector;
	}

	// Sets a column, its w 1 for the translation and 0 for the axes
	// $deBlob: FUNCTION 00597dc0
	void SetBasisVector( BASISVECTOR a_iIndex, const TVector4& a_rcVector )
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < 4 );
		TVector4& rCol = AsBasisVector4( a_iIndex );
		rCol           = a_rcVector;
		rCol.w         = ( a_iIndex >= 3 ) ? 1.0f : 0.0f;
	}

	// NOTE: an instance of it lives in the game code
	// $Barnyard: FUNCTION 00420450
	void GetTranslation( TVector4& a_rOutTranslation ) const
	{
		a_rOutTranslation = AsBasisVector4( BASISVECTOR_TRANSLATION );
	}

	// NOTE: an instance of it lives in the game code
	// $deBlob: FUNCTION 00580360
	// $Barnyard: FUNCTION 00415a20
	void GetBasisVector4( BASISVECTOR a_iIndex, TVector4& a_rOutVector ) const
	{
		TASSERT( a_iIndex >= 0 );
		TASSERT( a_iIndex < 4 );
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

	// Whether the 3x3 part has a negative determinant (a mirroring matrix)
	// $deBlob: FUNCTION 006655f0
	TBOOL IsMirrored() const
	{
		return m_f32 * ( m_f13 * m_f21 - m_f11 * m_f23 ) + m_f31 * ( m_f12 * m_f23 - m_f13 * m_f22 ) + m_f33 * ( m_f11 * m_f22 - m_f12 * m_f21 ) < 0.0f;
	}
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

	TBOOL IsEqual( const TMatrix44& a_rcMatrix ) const;

	// The three basis vectors are perpendicular to each other
	TBOOL IsOrthogonal() const;

	// NOTE: retail returns a_rOutScale in EAX (callers chain on it, e.g. Magnitude of the scale)
	TVector4& GetScale( TVector4& a_rOutScale ) const;

	void Lerp( const TMatrix44& a_rFrom, const TMatrix44& a_rTo, TFLOAT a_flProgress );

	void Scale( TFLOAT a_fScalar1, TFLOAT a_fScalar2, TFLOAT a_fScalar3 );

	void Scale( const TVector4& a_rScalars );

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

	void Scale( TFLOAT a_fScale )
	{
		Scale( a_fScale, a_fScale, a_fScale );
	}

	TBOOL IsOrthonormal() const;

	void Transpose( const TMatrix44& a_rcMatrix );

	void Multiply( const TMatrix44& a_rLeft, const TMatrix44& a_rRight );
	void Multiply( const TMatrix44& a_rRight );

	// this = a_rcLeft * this
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

	void Rotate( TFLOAT a_fAngleX, TFLOAT a_fAngleY, TFLOAT a_fAngleZ );

	void GetEulerXYZ( TVector3& a_rOutVec ) const;

	static void MatToQuat( TQuaternion& a_rOutQuat, const TMatrix44& a_rMatrix );

	// NOTE: the vector/plane transforms below use de Blob's order of operations (Barnyard sums in another order)

	static void TransformPlaneOrthogonal( TPlane& a_rOutPlane, const TMatrix44& a_rMatrix, const TPlane& a_rcSourcePlane );

	// Transforms a plane by any matrix (the inverse transpose is built from two vectors on the plane)
	static void TransformPlane( TPlane& a_rOutPlane, const TMatrix44& a_rMatrix, const TPlane& a_rcSourcePlane );

	static void RotateVector( TVector3& a_rOutVector, const TMatrix44& a_rMatrix, const TVector3& a_rVector );

	static void RotateVector( TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector );

	static void TransformVector( TVector3& a_rOutVector, const TMatrix44& a_rMatrix, const TVector3& a_rVector );

	static void TransformVector( TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector );

	// Batched versions (arrays of pointers / contiguous arrays)
	static void TransformVectorBatched( TVector3* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector3* const* a_ppVectors, TINT a_iBatchSize );
	static void TransformVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* const* a_ppVectors, TINT a_iBatchSize );
	static void TransformVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* a_pVectors, TINT a_iBatchSize );
	static void RotateVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* const* a_ppVectors, TINT a_BatchSize );
	static void RotateVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* a_pVectors, TINT a_BatchSize );

	// $deBlob: FUNCTION 0057b850
	// $Barnyard: FUNCTION 004928a0
	// NOTE: retail returns nothing and copies the whole matrix at once (movdqu)
	void operator=( const TMatrix44& a_rcMatrix ) { memcpy( this, &a_rcMatrix, sizeof( TMatrix44 ) ); }

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
	static TCONSTINIT TMatrix44 IDENTITY;

public:
	TFLOAT m_f11, m_f12, m_f13, m_f14;
	TFLOAT m_f21, m_f22, m_f23, m_f24;
	TFLOAT m_f31, m_f32, m_f33, m_f34;
	TFLOAT m_f41, m_f42, m_f43, m_f44;
};

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TMatrix44.h"

TOSHI_NAMESPACE_START

TCONSTINIT TMatrix44 TMatrix44::IDENTITY(
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f
);

// $deBlob: FUNCTION 00663680
// $Barnyard: FUNCTION 006c8500
// $Barnyard: FUNCTION 006c8520
void TMatrix44::Identity()
{
	*this = IDENTITY;
}

// $deBlob: FUNCTION 00664d60
// $Barnyard: FUNCTION 006c8540
void TMatrix44::Transpose( const TMatrix44& a_rcMatrix )
{
	TFLOAT f14 = a_rcMatrix.m_f41, f24 = a_rcMatrix.m_f42, f34 = a_rcMatrix.m_f43, f44 = a_rcMatrix.m_f44;
	TFLOAT f13 = a_rcMatrix.m_f31, f23 = a_rcMatrix.m_f32, f33 = a_rcMatrix.m_f33, f43 = a_rcMatrix.m_f34;
	TFLOAT f12 = a_rcMatrix.m_f21, f22 = a_rcMatrix.m_f22, f32 = a_rcMatrix.m_f23, f42 = a_rcMatrix.m_f24;
	TFLOAT f11 = a_rcMatrix.m_f11, f21 = a_rcMatrix.m_f12, f31 = a_rcMatrix.m_f13, f41 = a_rcMatrix.m_f14;
	Set( f11, f12, f13, f14, f21, f22, f23, f24, f31, f32, f33, f34, f41, f42, f43, f44 );
}

// $deBlob: FUNCTION 006653a0
TBOOL TMatrix44::IsEqual( const TMatrix44& a_rcMatrix ) const
{
	const TFLOAT* pThis  = &m_f11;
	const TFLOAT* pOther = &a_rcMatrix.m_f11;

	for ( TINT i = 0; i < 4; i++ )
		for ( TINT k = 0; k < 4; k++ )
			if ( pThis[ i * 4 + k ] != pOther[ i * 4 + k ] ) return TFALSE;

	return TTRUE;
}

// The three basis vectors are perpendicular to each other
// $deBlob: FUNCTION 006653f0
TBOOL TMatrix44::IsOrthogonal() const
{
	TFLOAT fDot12 = m_f11 * m_f21 + m_f12 * m_f22 + m_f13 * m_f23;
	TFLOAT fDot13 = m_f11 * m_f31 + m_f12 * m_f32 + m_f13 * m_f33;
	TFLOAT fDot23 = m_f21 * m_f31 + m_f22 * m_f32 + m_f23 * m_f33;
	return fDot13 * fDot13 + fDot12 * fDot12 + fDot23 * fDot23 < 0.01f;
}

// $deBlob: FUNCTION 00665490
TBOOL TMatrix44::IsOrthonormal() const
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

// $deBlob: FUNCTION 00665660
// $Barnyard: FUNCTION 006c86f0
void TMatrix44::Scale( TFLOAT a_fScalar1, TFLOAT a_fScalar2, TFLOAT a_fScalar3 )
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
void TMatrix44::Scale( const TVector4& a_rScalars )
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

// $deBlob: FUNCTION 00665790
// NOTE: retail returns a_rOutScale in EAX (callers chain on it, e.g. Magnitude of the scale)
TVector4& TMatrix44::GetScale( TVector4& a_rOutScale ) const
{
	TFLOAT fX = AsBasisVector4( 0 ).Magnitude();
	TFLOAT fY = AsBasisVector4( 1 ).Magnitude();
	TFLOAT fZ = AsBasisVector4( 2 ).Magnitude();
	a_rOutScale.Set( fX, fY, fZ, 1.0f );
	return a_rOutScale;
}

// $deBlob: FUNCTION 006657f0
// $Barnyard: FUNCTION 006c9160
void TMatrix44::Rotate( TFLOAT a_fAngleX, TFLOAT a_fAngleY, TFLOAT a_fAngleZ )
{
	RotateX( a_fAngleX );
	RotateY( a_fAngleY );
	RotateZ( a_fAngleZ );
}

// $deBlob: FUNCTION 00665ca0
void TMatrix44::LookAtTarget( const TVector4& target, const TVector4& up )
{
	auto& forward = AsBasisVector3( 2 );

	forward = target.AsVector3() - up.AsVector3();
	forward.Normalize();

	AsBasisVector3( 0 ) = { m_f33, 0, -m_f31 };
	AsBasisVector3( 0 ).Normalize();

	m_f22 = m_f11 * m_f33 - m_f13 * m_f31;
	m_f23 = m_f12 * m_f31 - m_f11 * m_f32;
	m_f21 = m_f13 * m_f32 - m_f12 * m_f33;
}

// $deBlob: FUNCTION 00665e20
// $Barnyard: FUNCTION 006c8970
void TMatrix44::LookAtDirection( const Toshi::TVector4& a_rDirection, const Toshi::TVector4& a_rUpAxis )
{
	TVector4 vProjection;
	TVector4 vCrossProduct;

	vProjection.ProjectNormalToPlane( a_rUpAxis, a_rDirection );
	vProjection.Multiply( -1.0f );
	vCrossProduct.CrossProduct( vProjection, a_rDirection );

	AsBasisVector4( 0 )   = vCrossProduct;
	AsBasisVector4( 0 ).w = 0.0f;

	AsBasisVector4( 1 )   = vProjection;
	AsBasisVector4( 1 ).w = 0.0f;

	AsBasisVector4( 2 )   = a_rDirection;
	AsBasisVector4( 2 ).w = 0.0f;
}

// $deBlob: FUNCTION 00664f80
// $Barnyard: FUNCTION 006c85f0
void TMatrix44::Multiply( const TMatrix44& a_rLeft, const TMatrix44& a_rRight )
{
	TASSERT( ( this != &a_rLeft ) && ( this != &a_rRight ) );

	// NOTE: de Blob's order of operations (Barnyard sums w, z, y, x)
	for ( TINT i = 0; i < 4; i++ )
	{
		TVector4&       vOut = AsBasisVector4( i );
		const TVector4& vR   = a_rRight.AsBasisVector4( i );

		vOut.x = a_rLeft.m_f21 * vR.y + vR.x * a_rLeft.m_f11 + a_rLeft.m_f31 * vR.z + a_rLeft.m_f41 * vR.w;
		vOut.y = vR.x * a_rLeft.m_f12 + vR.y * a_rLeft.m_f22 + a_rLeft.m_f32 * vR.z + a_rLeft.m_f42 * vR.w;
		vOut.z = a_rLeft.m_f13 * vR.x + vR.y * a_rLeft.m_f23 + a_rLeft.m_f33 * vR.z + vR.w * a_rLeft.m_f43;
		vOut.w = a_rLeft.m_f24 * vR.y + vR.x * a_rLeft.m_f14 + a_rLeft.m_f34 * vR.z + a_rLeft.m_f44 * vR.w;
	}
}

// $deBlob: FUNCTION 005f0040
// $deBlob: FUNCTION 00572e80
// $Barnyard: FUNCTION 0041fb30
void TMatrix44::Multiply( const TMatrix44& a_rRight )
{
	TMatrix44 temp = *this;
	Multiply( temp, a_rRight );
}

// $deBlob: FUNCTION 006640b0
// $Barnyard: FUNCTION 006c8ef0
TBOOL TMatrix44::Invert( const TMatrix44& a_rRight )
{
	TASSERT( a_rRight.m_f14 == 0.0f );
	TASSERT( a_rRight.m_f24 == 0.0f );
	TASSERT( a_rRight.m_f34 == 0.0f );
	TASSERT( a_rRight.m_f44 == 1.0f );

	TFLOAT fVar1;
	TFLOAT fVar2;
	TFLOAT fVar3;
	TFLOAT fVar4;

	// NOTE: de Blob's order of operations
	fVar1 = a_rRight.m_f22 * a_rRight.m_f33 - a_rRight.m_f23 * a_rRight.m_f32;
	fVar4 = -( a_rRight.m_f12 * a_rRight.m_f33 - a_rRight.m_f13 * a_rRight.m_f32 );
	fVar3 = a_rRight.m_f12 * a_rRight.m_f23 - a_rRight.m_f13 * a_rRight.m_f22;
	fVar2 = a_rRight.m_f11 * fVar1 + a_rRight.m_f21 * fVar4 + a_rRight.m_f31 * fVar3;

	if ( fVar2 == 0.0f )
		return TFALSE;

	fVar2 = 1.0f / fVar2;

	TFLOAT f21 = -( ( a_rRight.m_f21 * a_rRight.m_f33 - a_rRight.m_f31 * a_rRight.m_f23 ) * fVar2 );
	TFLOAT f22 = ( a_rRight.m_f11 * a_rRight.m_f33 - a_rRight.m_f31 * a_rRight.m_f13 ) * fVar2;
	TFLOAT f23 = -( ( a_rRight.m_f11 * a_rRight.m_f23 - a_rRight.m_f21 * a_rRight.m_f13 ) * fVar2 );
	TFLOAT f31 = ( a_rRight.m_f21 * a_rRight.m_f32 - a_rRight.m_f31 * a_rRight.m_f22 ) * fVar2;
	TFLOAT f32 = -( ( a_rRight.m_f11 * a_rRight.m_f32 - a_rRight.m_f31 * a_rRight.m_f12 ) * fVar2 );
	TFLOAT f33 = ( a_rRight.m_f11 * a_rRight.m_f22 - a_rRight.m_f21 * a_rRight.m_f12 ) * fVar2;
	TFLOAT f11 = fVar2 * fVar1;
	TFLOAT f13 = fVar2 * fVar3;
	TFLOAT f12 = fVar2 * fVar4;
	TFLOAT f41 = -a_rRight.m_f41;
	TFLOAT f42 = -a_rRight.m_f42;
	TFLOAT f43 = -a_rRight.m_f43;

	Set( f11, f12, f13, a_rRight.m_f14, f21, f22, f23, a_rRight.m_f24, f31, f32, f33, a_rRight.m_f34, f41, f42, f43, a_rRight.m_f44 );

	m_f42 = f41 * f12 + f42 * f22 + f43 * f32;
	m_f43 = f41 * f13 + f42 * f23 + f43 * f33;
	m_f41 = f41 * f11 + f42 * f21 + f43 * f31;

	return TTRUE;
}

// $deBlob: FUNCTION 00664ae0
// $Barnyard: FUNCTION 006c9050
void TMatrix44::InvertOrthogonal( const TMatrix44& a_rRight )
{
	TASSERT( a_rRight.m_f14 == 0.0f );
	TASSERT( a_rRight.m_f24 == 0.0f );
	TASSERT( a_rRight.m_f34 == 0.0f );
	TASSERT( a_rRight.m_f44 == 1.0f );

	TFLOAT fVar7  = a_rRight.m_f13;
	TFLOAT fVar1  = a_rRight.m_f44;
	TFLOAT fVar2  = a_rRight.m_f43;
	TFLOAT fVar8  = a_rRight.m_f24;
	TFLOAT fVar3  = a_rRight.m_f42;
	TFLOAT fVar9  = a_rRight.m_f32;
	TFLOAT fVar4  = a_rRight.m_f41;
	TFLOAT fVar10 = a_rRight.m_f22;
	TFLOAT fVar5  = a_rRight.m_f34;
	TFLOAT fVar11 = a_rRight.m_f12;
	TFLOAT fVar6  = a_rRight.m_f33;
	TFLOAT fVar12 = a_rRight.m_f14;
	TFLOAT fVar13 = a_rRight.m_f31;
	TFLOAT fVar14 = a_rRight.m_f21;
	TFLOAT fVar15 = a_rRight.m_f11;

	m_f32 = a_rRight.m_f23;
	m_f11 = fVar15;
	m_f33 = fVar6;
	m_f13 = fVar13;
	m_f34 = fVar5;
	m_f21 = fVar11;
	m_f41 = -fVar4;
	m_f23 = fVar9;
	m_f42 = -fVar3;
	m_f31 = fVar7;
	m_f43 = -fVar2;
	m_f12 = fVar14;
	m_f44 = fVar1;
	m_f14 = fVar12;
	m_f22 = fVar10;
	m_f24 = fVar8;

	// NOTE: de Blob's order of operations
	TFLOAT f41 = -fVar4, f42 = -fVar3, f43 = -fVar2;
	m_f42 = f41 * fVar14 + f42 * fVar10 + f43 * m_f32;
	m_f43 = f41 * fVar13 + f42 * fVar9 + f43 * fVar6;
	m_f41 = f41 * fVar15 + f42 * fVar11 + f43 * fVar7;
}

// $Barnyard: FUNCTION 006c9110
void TMatrix44::InvertOrthogonal()
{
	InvertOrthogonal( *this );
}

// $deBlob: FUNCTION 00665140
void TMatrix44::InvertOrthonormal()
{
	TASSERT( IsOrthonormal() );

	std::swap( m_f12, m_f21 );
	std::swap( m_f13, m_f31 );
	std::swap( m_f23, m_f32 );

	TFLOAT f41 = -m_f41;
	TFLOAT f42 = -m_f42;
	TFLOAT f43 = -m_f43;

	// NOTE: Barnyard reused the already updated m_f42/m_f43 here, de Blob uses the negated translation for all three
	m_f42 = m_f22 * f42 + f41 * m_f12 + f43 * m_f32;
	m_f43 = f42 * m_f23 + f41 * m_f13 + m_f33 * f43;
	m_f41 = f42 * m_f21 + f41 * m_f11 + f43 * m_f31;
}

// $deBlob: FUNCTION 00665e90
// $Barnyard: FUNCTION 006c8a20
void TMatrix44::TransformVector( TVector3& a_rOutVector, const TMatrix44& a_rMatrix, const TVector3& a_rVector )
{
	TFLOAT fX = a_rVector.x, fY = a_rVector.y, fZ = a_rVector.z;

	a_rOutVector.x = a_rMatrix.m_f21 * fY + a_rMatrix.m_f11 * fX + a_rMatrix.m_f31 * fZ + a_rMatrix.m_f41;
	a_rOutVector.y = a_rMatrix.m_f12 * fX + a_rMatrix.m_f22 * fY + a_rMatrix.m_f32 * fZ + a_rMatrix.m_f42;
	a_rOutVector.z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + a_rMatrix.m_f33 * fZ + a_rMatrix.m_f43;
}

// $deBlob: FUNCTION 00666010
// $Barnyard: FUNCTION 006c8a80
void TMatrix44::TransformVector( TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector )
{
	TFLOAT fX = a_rVector.x, fY = a_rVector.y, fZ = a_rVector.z, fW = a_rVector.w;

	a_rOutVector.z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + a_rMatrix.m_f33 * fZ + a_rMatrix.m_f43 * fW;
	a_rOutVector.w = a_rMatrix.m_f14 * fX + a_rMatrix.m_f24 * fY + a_rMatrix.m_f34 * fZ + a_rMatrix.m_f44 * fW;
	a_rOutVector.y = a_rMatrix.m_f12 * fX + a_rMatrix.m_f22 * fY + a_rMatrix.m_f32 * fZ + a_rMatrix.m_f42 * fW;
	a_rOutVector.x = a_rMatrix.m_f21 * fY + a_rMatrix.m_f11 * fX + a_rMatrix.m_f31 * fZ + a_rMatrix.m_f41 * fW;
}

// $deBlob: FUNCTION 00666330
// $Barnyard: FUNCTION 006c8b20
void TMatrix44::RotateVector( TVector3& a_rOutVector, const TMatrix44& a_rMatrix, const TVector3& a_rVector )
{
	TFLOAT fY = a_rVector.y, fX = a_rVector.x, fZ = a_rVector.z;

	TFLOAT fOutX = a_rMatrix.m_f21 * fY + a_rMatrix.m_f11 * fX + a_rMatrix.m_f31 * fZ;
	TFLOAT fOutY = a_rMatrix.m_f12 * fX + a_rMatrix.m_f22 * fY + a_rMatrix.m_f32 * fZ;
	TFLOAT fOutZ = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + a_rMatrix.m_f33 * fZ;

	a_rOutVector.x = fOutX;
	a_rOutVector.y = fOutY;
	a_rOutVector.z = fOutZ;
}

// $deBlob: FUNCTION 006663d0
// $Barnyard: FUNCTION 006c8b80
void TMatrix44::RotateVector( TVector4& a_rOutVector, const TMatrix44& a_rMatrix, const TVector4& a_rVector )
{
	TFLOAT fX = a_rVector.x, fY = a_rVector.y, fZ = a_rVector.z, fW = a_rVector.w;

	a_rOutVector.y = a_rMatrix.m_f12 * fX + a_rMatrix.m_f22 * fY + a_rMatrix.m_f32 * fZ;
	a_rOutVector.z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + a_rMatrix.m_f33 * fZ;
	a_rOutVector.w = fW;
	a_rOutVector.x = a_rMatrix.m_f21 * fY + a_rMatrix.m_f11 * fX + a_rMatrix.m_f31 * fZ;
}

// $deBlob: FUNCTION 00666a60
// $Barnyard: FUNCTION 006c8c20
void TMatrix44::MatToQuat( TQuaternion& a_rOutQuat, const TMatrix44& a_rMatrix )
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

// $deBlob: FUNCTION 00666c80
// $Barnyard: FUNCTION 006c8d90
TMatrix44& TMatrix44::SetFromQuaternion( const TQuaternion& a_rQuaternion )
{
	TFLOAT fVal1 = a_rQuaternion.z * 2.0f;
	TFLOAT fVal2 = a_rQuaternion.y * 2.0f;
	TFLOAT fVal3 = a_rQuaternion.x * 2.0f * a_rQuaternion.w;
	TFLOAT fVal4 = a_rQuaternion.x * 2.0f * a_rQuaternion.x;
	TFLOAT fVal5 = fVal1 * a_rQuaternion.z;

	m_f14 = 0.0f;
	m_f24 = 0.0f;
	m_f34 = 0.0f;
	m_f41 = 0.0f;
	m_f42 = 0.0f;
	m_f43 = 0.0f;
	m_f44 = 1.0f;
	m_f11 = 1.0f - ( fVal5 + fVal2 * a_rQuaternion.y );
	m_f21 = fVal2 * a_rQuaternion.x - fVal1 * a_rQuaternion.w;
	m_f12 = fVal2 * a_rQuaternion.x + fVal1 * a_rQuaternion.w;
	m_f31 = fVal1 * a_rQuaternion.x + fVal2 * a_rQuaternion.w;
	m_f13 = fVal1 * a_rQuaternion.x - fVal2 * a_rQuaternion.w;
	m_f22 = 1.0f - ( fVal5 + fVal4 );
	m_f32 = fVal1 * a_rQuaternion.y - fVal3;
	m_f23 = fVal1 * a_rQuaternion.y + fVal3;
	m_f33 = 1.0f - ( fVal2 * a_rQuaternion.y + fVal4 );

	return *this;
}

// $deBlob: FUNCTION 00666dd0
// $Barnyard: FUNCTION 006c8ea0
TMatrix44& TMatrix44::PushQuaternion( const TQuaternion& a_rQuaternion, const TMatrix44& a_rMatrix, const TVector3& a_rOrigin )
{
	TMatrix44 matrix;

	matrix.SetFromQuaternion( a_rQuaternion );
	matrix.AsBasisVector3( 3 ) = a_rOrigin;
	Multiply( a_rMatrix, matrix );

	return *this;
}

// $deBlob: FUNCTION 00665a30
// $Barnyard: FUNCTION 006c8840
void TMatrix44::RotateX( TFLOAT angle )
{
	TFLOAT fVar1;
	TFLOAT fVar2;
	TFLOAT fCos;
	TFLOAT fSin;

	TMath::SinCos( angle, fSin, fCos );
	fVar1 = m_f31;
	m_f31 = fVar1 * fCos - m_f21 * fSin;
	fVar2 = m_f32;
	m_f21 = fVar1 * fSin + m_f21 * fCos;
	fVar1 = m_f23;
	m_f32 = fVar2 * fCos - m_f22 * fSin;
	m_f22 = fVar2 * fSin + m_f22 * fCos;
	m_f23 = m_f33 * fSin + fVar1 * fCos;
	m_f33 = m_f33 * fCos - fVar1 * fSin;
}

// $deBlob: FUNCTION 00665b00
// $Barnyard: FUNCTION 006c88e0
void TMatrix44::RotateY( TFLOAT angle )
{
	TFLOAT fVar1;
	TFLOAT fVar2;
	TFLOAT fSin;
	TFLOAT fCos;

	TMath::SinCos( angle, fSin, fCos );
	fVar1 = m_f31;
	m_f31 = fVar1 * fCos + m_f11 * fSin;
	fVar2 = m_f32;
	m_f11 = m_f11 * fCos - fVar1 * fSin;
	fVar1 = m_f13;
	m_f32 = fVar2 * fCos + m_f12 * fSin;
	m_f12 = m_f12 * fCos - fVar2 * fSin;
	m_f13 = fVar1 * fCos - m_f33 * fSin;
	m_f33 = m_f33 * fCos + fVar1 * fSin;
}

// $deBlob: FUNCTION 00665bd0
// $Barnyard: FUNCTION 006c87b0
void TMatrix44::RotateZ( TFLOAT angle )
{
	TFLOAT fVar1;
	TFLOAT fVar2;
	TFLOAT fCos;
	TFLOAT fSin;

	TMath::SinCos( angle, fSin, fCos );
	fVar1 = m_f21;
	m_f21 = fVar1 * fCos - m_f11 * fSin;
	fVar2 = m_f22;
	m_f11 = fVar1 * fSin + m_f11 * fCos;
	fVar1 = m_f13;
	m_f22 = fVar2 * fCos - m_f12 * fSin;
	m_f12 = fVar2 * fSin + m_f12 * fCos;
	m_f13 = m_f23 * fSin + fVar1 * fCos;
	m_f23 = m_f23 * fCos - fVar1 * fSin;
}

// $deBlob: FUNCTION 006d9cf0
// $Barnyard: FUNCTION 005e7bb0
void TMatrix44::GetEulerXYZ( TVector3& a_rOutVec ) const
{
	TFLOAT fVal1 = m_f31;
	TMath::Clip( fVal1, -1.0f, 1.0f );

	TFLOAT fVal2 = TMath::Sqrt( 1.0f - fVal1 * fVal1 );

	if ( m_f11 < 0 && m_f33 < 0 )
		fVal2 *= -1;

	if ( 0.001f < fVal2 || fVal2 < -0.001 )
	{
		TFLOAT fVal3 = 1.0f / fVal2;

		a_rOutVec.x = TMath::ATan2( m_f32 * fVal3, m_f33 * fVal3 );
		a_rOutVec.y = TMath::ATan2( fVal1, fVal2 );
		a_rOutVec.z = TMath::ATan2( m_f21 * fVal3, m_f11 * fVal3 );
	}
	else
	{
		a_rOutVec.x = TMath::ATan2( -m_f32, m_f22 );
		a_rOutVec.y = -TMath::ASin( fVal1 );
		a_rOutVec.z = 0.0f;
	}
}

// $Barnyard: FUNCTION 0042c510
void TMatrix44::Lerp( const TMatrix44& a_rFrom, const TMatrix44& a_rTo, TFLOAT a_flProgress )
{
	AsBasisVector4( 0 ).Lerp4( a_rFrom.AsBasisVector4( 0 ), a_rTo.AsBasisVector4( 0 ), a_flProgress );
	AsBasisVector4( 1 ).Lerp4( a_rFrom.AsBasisVector4( 1 ), a_rTo.AsBasisVector4( 1 ), a_flProgress );
	AsBasisVector4( 2 ).Lerp4( a_rFrom.AsBasisVector4( 2 ), a_rTo.AsBasisVector4( 2 ), a_flProgress );
	AsBasisVector4( 3 ).Lerp4( a_rFrom.AsBasisVector4( 3 ), a_rTo.AsBasisVector4( 3 ), a_flProgress );
}

// $deBlob: FUNCTION 006638a0
TFLOAT TMatrix44::Determinant() const
{
	return ( ( m_f22 * m_f33 - m_f23 * m_f32 ) * m_f11 - ( m_f12 * m_f33 - m_f13 * m_f32 ) * m_f21 ) + ( m_f12 * m_f23 - m_f13 * m_f22 ) * m_f31;
}

// $deBlob: FUNCTION 00663700
TFLOAT TMatrix44::Determinant4() const
{
	const TFLOAT* p = AsBasisVector4( 0 ).AsArray();

	TFLOAT fVar1 = p[ 0 ];
	TFLOAT fVar2 = p[ 1 ];
	TFLOAT fVar3 = p[ 4 ];
	TFLOAT fVar4 = p[ 15 ];
	TFLOAT fVar5 = p[ 14 ];
	TFLOAT fVar6 = p[ 12 ];
	TFLOAT fVar7 = p[ 3 ];

	TFLOAT fA = ( fVar1 * p[ 5 ] - fVar2 * fVar3 ) * ( p[ 10 ] * fVar4 - p[ 11 ] * fVar5 );
	TFLOAT fB = ( p[ 9 ] * fVar1 - p[ 8 ] * fVar2 ) * ( p[ 6 ] * fVar4 - p[ 7 ] * fVar5 );
	TFLOAT fC = ( p[ 13 ] * fVar1 - fVar6 * fVar2 ) * ( p[ 6 ] * p[ 11 ] - p[ 7 ] * p[ 10 ] );
	TFLOAT fD = ( p[ 2 ] * fVar4 - fVar7 * fVar5 ) * ( p[ 9 ] * fVar3 - p[ 8 ] * p[ 5 ] );
	TFLOAT fE = ( p[ 13 ] * fVar3 - fVar6 * p[ 5 ] ) * ( p[ 2 ] * p[ 11 ] - fVar7 * p[ 10 ] );
	TFLOAT fF = ( p[ 13 ] * p[ 8 ] - fVar6 * p[ 9 ] ) * ( p[ 2 ] * p[ 7 ] - fVar7 * p[ 6 ] );

	return ( ( ( fA - fB ) + fC + fD ) - fE ) + fF;
}

// Inverts any 4x4 matrix, returns TFALSE if it can't be inverted
// $deBlob: FUNCTION 00663910
TBOOL TMatrix44::InvertArbitrary( const TMatrix44& a_rcMatrix )
{
	const TFLOAT* param_1    = a_rcMatrix.AsBasisVector4( 0 ).AsArray();
	TFLOAT*       param_1_00 = AsBasisVector4( 0 ).AsArray();

	TFLOAT fVar1   = param_1[ 15 ];
	TFLOAT fVar2   = param_1[ 14 ];
	TFLOAT fVar3   = param_1[ 10 ];
	TFLOAT fVar4   = param_1[ 11 ];
	TFLOAT fVar74  = fVar3 * fVar1 - fVar4 * fVar2;
	TFLOAT fVar72  = param_1[ 1 ] * param_1[ 4 ];
	TFLOAT fVar75  = param_1[ 0 ] * param_1[ 5 ];
	TFLOAT fVar5   = param_1[ 7 ];
	TFLOAT fVar6   = param_1[ 10 ];
	TFLOAT fVar76  = fVar75 - fVar72;
	TFLOAT fVar7   = param_1[ 6 ];
	TFLOAT fVar8   = param_1[ 13 ];
	TFLOAT fVar9   = param_1[ 7 ];
	TFLOAT fVar10  = param_1[ 12 ];
	TFLOAT fVar95  = fVar7 * fVar1 - fVar9 * fVar2;
	TFLOAT fVar11  = param_1[ 8 ];
	TFLOAT fVar12  = param_1[ 1 ];
	TFLOAT fVar77  = fVar11 * param_1[ 1 ];
	TFLOAT fVar13  = param_1[ 9 ];
	TFLOAT fVar14  = param_1[ 0 ];
	TFLOAT fVar96  = fVar13 * fVar14 - fVar77;
	TFLOAT fVar15  = param_1[ 11 ];
	TFLOAT fVar16  = param_1[ 0 ];
	TFLOAT fVar78  = fVar7 * fVar15 - fVar5 * fVar6;
	TFLOAT fVar79  = fVar8 * fVar16 - fVar10 * fVar12;
	TFLOAT fVar17  = param_1[ 2 ];
	TFLOAT fVar18  = param_1[ 3 ];
	TFLOAT fVar19  = param_1[ 9 ];
	TFLOAT fVar20  = param_1[ 4 ];
	TFLOAT fVar85  = fVar17 * fVar1 - fVar18 * fVar2;
	TFLOAT fVar21  = param_1[ 8 ];
	TFLOAT fVar22  = param_1[ 5 ];
	TFLOAT fVar99  = fVar19 * fVar20 - fVar21 * fVar22;
	TFLOAT fVar23  = param_1[ 12 ];
	TFLOAT fVar24  = param_1[ 5 ];
	TFLOAT fVar25  = param_1[ 4 ];
	TFLOAT fVar26  = param_1[ 3 ];
	TFLOAT fVar100 = fVar8 * fVar25 - fVar23 * fVar24;
	TFLOAT fVar27  = param_1[ 10 ];
	TFLOAT fVar28  = param_1[ 11 ];
	TFLOAT fVar29  = param_1[ 8 ];
	TFLOAT fVar30  = param_1[ 7 ];
	TFLOAT fVar80  = fVar17 * fVar28 - fVar26 * fVar27;
	TFLOAT fVar31  = param_1[ 12 ];
	TFLOAT fVar32  = param_1[ 9 ];
	TFLOAT fVar97  = fVar8 * fVar29 - fVar31 * fVar32;
	TFLOAT fVar33  = param_1[ 3 ];
	TFLOAT fVar92  = fVar17 * fVar30 - fVar33 * fVar7;
	TFLOAT fVar81  = ( ( ( fVar76 * fVar74 - fVar96 * fVar95 ) + fVar79 * fVar78 + fVar99 * fVar85 ) - fVar80 * fVar100 ) + fVar92 * fVar97;

	if ( fVar81 == 0.0f )
		return TFALSE;

	fVar81         = 1.0f / fVar81;
	TFLOAT fVar34  = param_1[ 9 ];
	TFLOAT fVar35  = param_1[ 1 ];
	TFLOAT fVar36  = param_1[ 5 ];
	TFLOAT fVar37  = param_1[ 5 ];
	TFLOAT fVar38  = param_1[ 9 ];
	TFLOAT fVar98  = param_1[ 8 ] * param_1[ 15 ];
	TFLOAT fVar86  = param_1[ 8 ] * param_1[ 7 ];
	TFLOAT fVar103 = param_1[ 12 ] * param_1[ 7 ];
	TFLOAT fVar82  = param_1[ 12 ] * param_1[ 11 ];
	TFLOAT fVar101 = param_1[ 4 ] * param_1[ 15 ];
	TFLOAT fVar89  = param_1[ 4 ] * param_1[ 11 ];
	TFLOAT fVar39  = param_1[ 6 ];
	TFLOAT fVar40  = param_1[ 10 ];
	TFLOAT fVar41  = param_1[ 14 ];
	TFLOAT fVar104 = param_1[ 0 ] * param_1[ 11 ];
	TFLOAT fVar90  = param_1[ 0 ] * param_1[ 15 ];
	TFLOAT fVar93  = param_1[ 3 ] * param_1[ 8 ];
	TFLOAT fVar87  = param_1[ 3 ] * param_1[ 12 ];
	TFLOAT fVar42  = param_1[ 14 ];
	TFLOAT fVar43  = param_1[ 2 ];
	TFLOAT fVar44  = param_1[ 10 ];
	TFLOAT fVar45  = param_1[ 2 ];
	TFLOAT fVar46  = param_1[ 7 ];
	TFLOAT fVar47  = param_1[ 6 ];
	TFLOAT fVar48  = param_1[ 7 ];
	TFLOAT fVar49  = param_1[ 0 ];
	TFLOAT fVar50  = param_1[ 3 ];
	TFLOAT fVar51  = param_1[ 4 ];
	TFLOAT fVar52  = param_1[ 14 ];
	TFLOAT fVar53  = param_1[ 11 ];
	TFLOAT fVar54  = param_1[ 15 ];
	TFLOAT fVar55  = param_1[ 3 ];
	TFLOAT fVar102 = param_1[ 5 ] * param_1[ 14 ];
	TFLOAT fVar56  = param_1[ 13 ];
	TFLOAT fVar57  = param_1[ 10 ];
	TFLOAT fVar83  = param_1[ 9 ] * fVar47;
	TFLOAT fVar94  = param_1[ 9 ] * param_1[ 14 ];
	TFLOAT fVar105 = param_1[ 5 ] * param_1[ 10 ];
	TFLOAT fVar58  = param_1[ 8 ];
	TFLOAT fVar59  = param_1[ 4 ];
	TFLOAT fVar60  = param_1[ 0 ];
	TFLOAT fVar61  = param_1[ 12 ];
	TFLOAT fVar106 = param_1[ 2 ] * param_1[ 9 ];
	TFLOAT fVar88  = param_1[ 2 ] * fVar56;
	TFLOAT fVar91  = param_1[ 1 ] * param_1[ 10 ];
	TFLOAT fVar84  = param_1[ 1 ] * param_1[ 14 ];
	TFLOAT fVar62  = param_1[ 8 ];
	TFLOAT fVar63  = param_1[ 12 ];
	TFLOAT fVar73  = param_1[ 6 ] * param_1[ 1 ];
	TFLOAT fVar107 = param_1[ 2 ] * param_1[ 5 ];
	TFLOAT fVar64  = param_1[ 0 ];
	TFLOAT fVar65  = param_1[ 12 ];
	TFLOAT fVar66  = param_1[ 4 ];
	TFLOAT fVar67  = param_1[ 9 ];
	TFLOAT fVar68  = param_1[ 5 ];
	TFLOAT fVar69  = param_1[ 0 ];
	TFLOAT fVar70  = param_1[ 4 ];
	TFLOAT fVar71  = param_1[ 13 ];

	TFLOAT aResult[ 16 ];
	aResult[ 7 ]  = ( ( fVar104 - fVar93 ) * fVar47 + ( fVar86 - fVar89 ) * param_1[ 2 ] + ( fVar50 * fVar51 - fVar48 * fVar49 ) * param_1[ 10 ] ) * fVar81;
	aResult[ 1 ]  = ( ( fVar26 * fVar27 - fVar17 * fVar28 ) * fVar8 + fVar85 * fVar34 + ( fVar4 * fVar2 - fVar3 * fVar1 ) * fVar35 ) * fVar81;
	aResult[ 2 ]  = ( fVar92 * fVar8 + fVar95 * fVar35 + ( fVar18 * fVar2 - fVar17 * fVar1 ) * fVar37 ) * fVar81;
	aResult[ 3 ]  = ( ( fVar5 * fVar6 - fVar7 * fVar15 ) * fVar35 + fVar80 * fVar36 + ( fVar33 * fVar7 - fVar17 * fVar30 ) * fVar38 ) * fVar81;
	aResult[ 4 ]  = ( ( fVar98 - fVar82 ) * fVar39 + ( fVar103 - fVar101 ) * fVar40 + ( fVar89 - fVar86 ) * fVar41 ) * fVar81;
	aResult[ 5 ]  = ( ( fVar93 - fVar104 ) * fVar42 + ( fVar90 - fVar87 ) * fVar44 + ( fVar82 - fVar98 ) * fVar43 ) * fVar81;
	aResult[ 6 ]  = ( ( fVar48 * fVar49 - fVar50 * fVar51 ) * fVar52 + ( fVar101 - fVar103 ) * fVar45 + ( fVar87 - fVar90 ) * fVar47 ) * fVar81;
	aResult[ 8 ]  = ( ( fVar23 * fVar24 - fVar8 * fVar25 ) * fVar53 + fVar97 * fVar46 + fVar99 * fVar54 ) * fVar81;
	aResult[ 9 ]  = ( ( fVar77 - fVar13 * fVar14 ) * fVar54 + fVar79 * fVar53 + ( fVar31 * fVar32 - fVar8 * fVar29 ) * fVar55 ) * fVar81;
	aResult[ 11 ] = ( ( fVar21 * fVar22 - fVar19 * fVar20 ) * fVar55 + fVar96 * fVar46 + ( fVar72 - fVar75 ) * fVar53 ) * fVar81;
	aResult[ 0 ]  = ( ( fVar9 * fVar2 - fVar7 * fVar1 ) * fVar67 + fVar68 * fVar74 + fVar71 * fVar78 ) * fVar81;
	aResult[ 10 ] = ( fVar100 * fVar55 + fVar76 * fVar54 + ( fVar10 * fVar12 - fVar8 * fVar16 ) * fVar46 ) * fVar81;
	aResult[ 12 ] = ( ( fVar102 - fVar56 * fVar47 ) * fVar58 + ( fVar56 * fVar57 - fVar94 ) * fVar59 + ( fVar83 - fVar105 ) * fVar61 ) * fVar81;
	aResult[ 13 ] = ( ( fVar88 - fVar84 ) * fVar62 + ( fVar91 - fVar106 ) * fVar63 + ( fVar94 - fVar56 * fVar57 ) * fVar60 ) * fVar81;
	aResult[ 14 ] = ( ( fVar56 * fVar47 - fVar102 ) * fVar64 + ( fVar107 - fVar73 ) * fVar65 + ( fVar84 - fVar88 ) * fVar66 ) * fVar81;
	aResult[ 15 ] = ( ( fVar106 - fVar91 ) * fVar70 + ( fVar105 - fVar83 ) * fVar69 + ( fVar73 - fVar107 ) * fVar11 ) * fVar81;

	for ( TINT i = 0; i < 16; i++ )
		param_1_00[ i ] = aResult[ i ];

	return TTRUE;
}

// Inverts any 4x4 matrix with the cofactors, returns TFALSE if it can't be inverted
// $deBlob: FUNCTION 00664490
TBOOL TMatrix44::Invert4( const TMatrix44& a_rcMatrix )
{
	const TFLOAT* param_1    = a_rcMatrix.AsBasisVector4( 0 ).AsArray();
	TFLOAT*       param_1_00 = AsBasisVector4( 0 ).AsArray();

	TFLOAT fVar1  = param_1[ 13 ];
	TFLOAT fVar2  = param_1[ 5 ];
	TFLOAT fVar3  = param_1[ 0 ];
	TFLOAT fVar4  = param_1[ 4 ];
	TFLOAT fVar5  = param_1[ 1 ];
	TFLOAT fVar6  = param_1[ 8 ];
	TFLOAT fVar7  = param_1[ 9 ];
	TFLOAT fVar8  = param_1[ 12 ];
	TFLOAT fVar31 = fVar2 * fVar3 - fVar4 * fVar5;
	TFLOAT fVar33 = fVar7 * fVar3 - fVar6 * fVar5;
	TFLOAT fVar9  = param_1[ 2 ];
	TFLOAT fVar32 = fVar1 * fVar3 - fVar8 * fVar5;
	TFLOAT fVar30 = fVar7 * fVar4 - fVar6 * fVar2;
	fVar3         = param_1[ 10 ];
	TFLOAT fVar29 = fVar1 * fVar4 - fVar8 * fVar2;
	fVar2         = param_1[ 3 ];
	fVar4         = param_1[ 14 ];
	TFLOAT fVar21 = fVar1 * fVar6 - fVar8 * fVar7;
	fVar1         = param_1[ 15 ];
	fVar5         = param_1[ 6 ];
	fVar6         = param_1[ 7 ];
	fVar7         = param_1[ 11 ];
	fVar8         = param_1[ 7 ];
	TFLOAT fVar10 = param_1[ 11 ];
	TFLOAT fVar11 = param_1[ 11 ];
	TFLOAT fVar12 = param_1[ 7 ];
	TFLOAT fVar28 = fVar9 * fVar12 - fVar5 * fVar2;
	TFLOAT fVar27 = fVar9 * fVar11 - fVar3 * fVar2;
	TFLOAT fVar22 = fVar9 * fVar1 - fVar4 * fVar2;
	TFLOAT fVar26 = fVar5 * fVar11 - fVar3 * param_1[ 7 ];
	TFLOAT fVar13 = param_1[ 5 ];
	TFLOAT fVar25 = fVar5 * fVar1 - fVar4 * param_1[ 7 ];
	TFLOAT fVar14 = param_1[ 0 ];
	TFLOAT fVar34 = fVar3 * fVar1 - fVar4 * fVar11;
	TFLOAT fVar15 = param_1[ 4 ];
	TFLOAT fVar16 = param_1[ 9 ];
	TFLOAT fVar17 = param_1[ 8 ];
	TFLOAT fVar18 = param_1[ 1 ];
	TFLOAT fVar19 = param_1[ 12 ];
	TFLOAT fVar20 = param_1[ 13 ];
	TFLOAT fVar24 = ( fVar27 * fVar13 - fVar28 * fVar16 ) - fVar26 * fVar18;
	TFLOAT fVar23 = ( fVar25 * fVar18 - fVar22 * fVar13 ) + fVar28 * fVar20;
	fVar18        = ( fVar22 * fVar16 - fVar27 * fVar20 ) - fVar34 * fVar18;
	fVar16        = ( fVar34 * fVar13 - fVar25 * fVar16 ) + fVar26 * fVar20;
	fVar13        = fVar24 * fVar19 + fVar23 * fVar17 + fVar18 * fVar15 + fVar16 * fVar14;

	if ( fVar13 == 0.0f )
		return TFALSE;

	fVar13 = 1.0f / fVar13;

	TFLOAT aResult[ 16 ];
	aResult[ 0 ]  = fVar13 * fVar16;
	aResult[ 4 ]  = ( ( fVar25 * fVar17 - fVar26 * fVar19 ) - fVar34 * fVar15 ) * fVar13;
	aResult[ 1 ]  = fVar13 * fVar18;
	aResult[ 8 ]  = fVar13 * ( ( fVar21 * fVar12 - fVar29 * fVar11 ) + fVar30 * fVar1 );
	aResult[ 2 ]  = fVar13 * fVar23;
	aResult[ 12 ] = fVar13 * ( ( fVar3 * fVar29 - fVar4 * fVar30 ) - fVar5 * fVar21 );
	aResult[ 3 ]  = fVar13 * fVar24;
	aResult[ 5 ]  = ( ( fVar34 * fVar14 - fVar22 * fVar17 ) + fVar27 * fVar19 ) * fVar13;
	aResult[ 9 ]  = fVar13 * ( ( fVar32 * fVar10 - fVar33 * fVar1 ) - fVar21 * fVar2 );
	aResult[ 6 ]  = ( ( fVar22 * fVar15 - fVar28 * fVar19 ) - fVar25 * fVar14 ) * fVar13;
	aResult[ 13 ] = fVar13 * ( ( fVar9 * fVar21 - fVar3 * fVar32 ) + fVar4 * fVar33 );
	aResult[ 7 ]  = ( ( fVar26 * fVar14 - fVar27 * fVar15 ) + fVar28 * fVar17 ) * fVar13;
	aResult[ 10 ] = fVar13 * ( ( fVar29 * fVar2 - fVar32 * fVar6 ) + fVar31 * fVar1 );
	aResult[ 14 ] = fVar13 * ( ( fVar5 * fVar32 - fVar4 * fVar31 ) - fVar9 * fVar29 );
	aResult[ 11 ] = fVar13 * ( ( fVar33 * fVar8 - fVar31 * fVar7 ) - fVar30 * fVar2 );
	aResult[ 15 ] = fVar13 * ( ( fVar9 * fVar30 - fVar5 * fVar33 ) + fVar3 * fVar31 );

	for ( TINT i = 0; i < 16; i++ )
		param_1_00[ i ] = aResult[ i ];

	return TTRUE;
}

// $deBlob: FUNCTION 00665d60
void TMatrix44::LookAtDirection( const TVector3& a_rVec, const TVector3& a_rVec2 )
{
	TVector4 vDirection( a_rVec.x, a_rVec.y, a_rVec.z, 1.0f );
	TVector4 vUpAxis( a_rVec2.x, a_rVec2.y, a_rVec2.z, 1.0f );
	LookAtDirection( vDirection, vUpAxis );
}

// $deBlob: FUNCTION 00666660
void TMatrix44::TransformPlane( TPlane& a_rOutPlane, const TMatrix44& a_rMatrix, const TPlane& a_rcSourcePlane )
{
	const TVector4& vPlane = a_rcSourcePlane.AsVector4();
	TVector4&       vOut   = a_rOutPlane.AsVector4();

	// Two vectors on the plane...
	TVector4 v1;
	v1.w = 1.0f;

	if ( TMath::Abs( vPlane.y ) <= TMath::Abs( vPlane.x ) )
		v1.Set( -vPlane.z, 0.0f, vPlane.x, 1.0f );
	else
		v1.Set( 0.0f, vPlane.z, -vPlane.y, 1.0f );

	TVector4 v2;
	v2.CrossProduct( vPlane, v1 );
	v2.w = 1.0f;

	// ...and a point on it
	TVector4 vPoint;
	vPoint.Multiply( vPlane, vPlane.w );
	vPoint.w = 1.0f;

	const TFLOAT* m = a_rMatrix.AsBasisVector4( 0 ).AsArray();

	TFLOAT fV1Tmp1 = m[ 1 ] * v1.x;
	TFLOAT fV1Tmp3 = m[ 6 ] * v1.y;
	TFLOAT fV1Tmp2 = m[ 2 ] * v1.x;
	TFLOAT fV2Tmp5 = m[ 1 ] * v2.x;
	TFLOAT fV1X    = m[ 4 ] * v1.y + m[ 0 ] * v1.x + m[ 8 ] * v1.z;
	TFLOAT fV2Tmp4 = m[ 9 ] * v2.z;
	TFLOAT fV2Tmp6 = m[ 8 ] * v2.z;
	TFLOAT fV2Z    = m[ 2 ] * v2.x + m[ 6 ] * v2.y + m[ 10 ] * v2.z;
	TFLOAT fV2X    = m[ 0 ] * v2.x + m[ 4 ] * v2.y + fV2Tmp6;
	TFLOAT fV2Y    = fV2Tmp5 + m[ 5 ] * v2.y + fV2Tmp4;
	TFLOAT fV1Y    = m[ 5 ] * v1.y + fV1Tmp1 + m[ 9 ] * v1.z;
	TFLOAT fV1Z    = fV1Tmp3 + fV1Tmp2 + m[ 10 ] * v1.z;

	v1.x = fV1X;
	v1.y = fV1Y;
	v1.z = fV1Z;
	v2.x = fV2X;
	v2.y = fV2Y;
	v2.z = fV2Z;

	vOut.CrossProduct( v1, v2 );
	vOut.Normalise();

	vOut.w = vOut.y * ( m[ 5 ] * vPoint.y + m[ 1 ] * vPoint.x + m[ 9 ] * vPoint.z + m[ 13 ] * vPoint.w ) +
	    vOut.x * ( m[ 4 ] * vPoint.y + m[ 0 ] * vPoint.x + m[ 8 ] * vPoint.z + m[ 12 ] * vPoint.w ) +
	    vOut.z * ( m[ 6 ] * vPoint.y + m[ 2 ] * vPoint.x + vPoint.z * m[ 10 ] + m[ 14 ] * vPoint.w );

	// Mirroring matrices flip the plane
	if ( m[ 9 ] * ( m[ 2 ] * m[ 4 ] - m[ 0 ] * m[ 6 ] ) + m[ 8 ] * ( m[ 1 ] * m[ 6 ] - m[ 2 ] * m[ 5 ] ) + m[ 10 ] * ( m[ 0 ] * m[ 5 ] - m[ 1 ] * m[ 4 ] ) < 0.0f )
		vOut.Negate4();
}

// $deBlob: FUNCTION 00666990
// $Barnyard: FUNCTION 006c8bf0
void TMatrix44::TransformPlaneOrthogonal( TPlane& a_rOutPlane, const TMatrix44& a_rMatrix, const TPlane& a_rcSourcePlane )
{
	TFLOAT fNY = a_rcSourcePlane.AsNormal().y;
	TFLOAT fNX = a_rcSourcePlane.AsNormal().x;
	TFLOAT fNZ = a_rcSourcePlane.AsNormal().z;
	TFLOAT fM21 = a_rMatrix.m_f21;
	TFLOAT fM11 = a_rMatrix.m_f11;

	TFLOAT fY   = a_rMatrix.m_f12 * fNX + a_rMatrix.m_f22 * fNY + a_rMatrix.m_f32 * fNZ;
	TFLOAT fZ   = a_rMatrix.m_f13 * fNX + a_rMatrix.m_f23 * fNY + a_rMatrix.m_f33 * fNZ;
	TFLOAT fM31 = a_rMatrix.m_f31;

	TFLOAT fCopyD = a_rcSourcePlane.GetD();
	a_rOutPlane.SetD( fCopyD );
	a_rOutPlane.AsNormal().y  = fY;
	a_rOutPlane.AsNormal().z  = fZ;
	a_rOutPlane.AsNormal().x  = fM21 * fNY + fNX * fM11 + fM31 * fNZ;
	a_rOutPlane.SetD( a_rMatrix.m_f42 * fY + a_rOutPlane.AsNormal().x * a_rMatrix.m_f41 + a_rMatrix.m_f43 * fZ + a_rcSourcePlane.GetD() );
}

// $deBlob: FUNCTION 00665f40
void TMatrix44::TransformVectorBatched( TVector3* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector3* const* a_ppVectors, TINT a_iBatchSize )
{
	for ( TINT i = 0; i < a_iBatchSize; i++ )
	{
		const TVector3& v  = *a_ppVectors[ i ];
		TFLOAT          fX = v.x, fY = v.y, fZ = v.z;

		a_pOutVectors[ i ].y = a_rMatrix.m_f22 * fY + fX * a_rMatrix.m_f12 + fZ * a_rMatrix.m_f32 + a_rMatrix.m_f42;
		a_pOutVectors[ i ].z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + fZ * a_rMatrix.m_f33 + a_rMatrix.m_f43;
		a_pOutVectors[ i ].x = a_rMatrix.m_f11 * fX + a_rMatrix.m_f21 * fY + a_rMatrix.m_f31 * fZ + a_rMatrix.m_f41;
	}
}

// $deBlob: FUNCTION 00666110
void TMatrix44::TransformVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* const* a_ppVectors, TINT a_iBatchSize )
{
	for ( TINT i = 0; i < a_iBatchSize; i++ )
	{
		const TVector4& v  = *a_ppVectors[ i ];
		TFLOAT          fX = v.x, fY = v.y, fZ = v.z, fW = v.w;

		a_pOutVectors[ i ].z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + fZ * a_rMatrix.m_f33 + fW * a_rMatrix.m_f43;
		a_pOutVectors[ i ].w = a_rMatrix.m_f14 * fX + a_rMatrix.m_f24 * fY + a_rMatrix.m_f34 * fZ + fW * a_rMatrix.m_f44;
		a_pOutVectors[ i ].y = fX * a_rMatrix.m_f12 + a_rMatrix.m_f22 * fY + fZ * a_rMatrix.m_f32 + fW * a_rMatrix.m_f42;
		a_pOutVectors[ i ].x = a_rMatrix.m_f11 * fX + a_rMatrix.m_f21 * fY + a_rMatrix.m_f31 * fZ + fW * a_rMatrix.m_f41;
	}
}

// $deBlob: FUNCTION 00666230
void TMatrix44::TransformVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* a_pVectors, TINT a_iBatchSize )
{
	for ( TINT i = 0; i < a_iBatchSize; i++ )
	{
		TFLOAT fX = a_pVectors[ i ].x, fY = a_pVectors[ i ].y, fZ = a_pVectors[ i ].z, fW = a_pVectors[ i ].w;

		a_pOutVectors[ i ].y = fX * a_rMatrix.m_f12 + fY * a_rMatrix.m_f22 + fZ * a_rMatrix.m_f32 + fW * a_rMatrix.m_f42;
		a_pOutVectors[ i ].z = fX * a_rMatrix.m_f13 + fY * a_rMatrix.m_f23 + fZ * a_rMatrix.m_f33 + a_rMatrix.m_f43 * fW;
		a_pOutVectors[ i ].w = fX * a_rMatrix.m_f14 + fY * a_rMatrix.m_f24 + fZ * a_rMatrix.m_f34 + fW * a_rMatrix.m_f44;
		a_pOutVectors[ i ].x = fY * a_rMatrix.m_f21 + fX * a_rMatrix.m_f11 + fZ * a_rMatrix.m_f31 + fW * a_rMatrix.m_f41;
	}
}

// $deBlob: FUNCTION 00666470
void TMatrix44::RotateVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* const* a_ppVectors, TINT a_BatchSize )
{
	TASSERT( a_BatchSize > 0 );

	for ( TINT i = 0; i < a_BatchSize; i++ )
	{
		const TVector4& v  = *a_ppVectors[ i ];
		TFLOAT          fX = v.x, fY = v.y, fZ = v.z, fW = v.w;

		a_pOutVectors[ i ].y = a_rMatrix.m_f12 * fX + fY * a_rMatrix.m_f22 + a_rMatrix.m_f32 * fZ;
		a_pOutVectors[ i ].w = fW;
		a_pOutVectors[ i ].z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + a_rMatrix.m_f33 * fZ;
		a_pOutVectors[ i ].x = a_rMatrix.m_f11 * fX + fY * a_rMatrix.m_f21 + fZ * a_rMatrix.m_f31;
	}
}

// $deBlob: FUNCTION 00666560
void TMatrix44::RotateVectorBatched( TVector4* a_pOutVectors, const TMatrix44& a_rMatrix, const TVector4* a_pVectors, TINT a_BatchSize )
{
	TASSERT( a_BatchSize > 0 );

	for ( TINT i = 0; i < a_BatchSize; i++ )
	{
		TFLOAT fX = a_pVectors[ i ].x, fY = a_pVectors[ i ].y, fZ = a_pVectors[ i ].z, fW = a_pVectors[ i ].w;

		a_pOutVectors[ i ].y = fX * a_rMatrix.m_f12 + a_rMatrix.m_f22 * fY + fZ * a_rMatrix.m_f32;
		a_pOutVectors[ i ].w = fW;
		a_pOutVectors[ i ].z = a_rMatrix.m_f13 * fX + a_rMatrix.m_f23 * fY + fZ * a_rMatrix.m_f33;
		a_pOutVectors[ i ].x = fY * a_rMatrix.m_f21 + fX * a_rMatrix.m_f11 + fZ * a_rMatrix.m_f31;
	}
}

TOSHI_NAMESPACE_END

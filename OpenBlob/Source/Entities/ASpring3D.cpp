#include "pch.h"
#include "ASpring3D.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ASpring3D ) == 0x68 );

// $deBlob: FUNCTION 005ea830
ASpring3D::ASpring3D( const TVector4& a_rcPosition, TFLOAT a_fStiffness, TFLOAT a_fDamping )
{
	m_vPosition    = a_rcPosition;
	m_vRest        = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vVelocity    = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vForce       = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vStiffness.w = 1.0f;
	m_vStiffness.x = a_fStiffness;
	m_vStiffness.y = a_fStiffness;
	m_vStiffness.z = a_fStiffness;
	m_vDamping.x   = a_fDamping;
	m_vDamping.y   = a_fDamping;
	m_vDamping.z   = a_fDamping;
	m_vDamping.w   = 1.0f;
	m_fMass        = 1.0f;
	m_fInverseMass = 1.0f;
}

// $deBlob: FUNCTION 005ea8b0
ASpring3D::ASpring3D( const TVector4& a_rcPosition, const TVector4& a_rcStiffness, const TVector4& a_rcDamping )
{
	m_vPosition    = a_rcPosition;
	m_vRest        = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vVelocity    = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vForce       = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vStiffness   = a_rcStiffness;
	m_vDamping     = a_rcDamping;
	m_fMass        = 1.0f;
	m_fInverseMass = 1.0f;
}

// $deBlob: FUNCTION 005ea920
ASpring3D::~ASpring3D()
{
}

// $deBlob: FUNCTION 005ea930
void ASpring3D::Init( const TVector4& a_rcPosition, TFLOAT a_fStiffness, TFLOAT a_fDamping )
{
	m_vPosition    = a_rcPosition;
	m_vRest        = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vVelocity    = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_fMass        = 1.0f;
	m_fInverseMass = 1.0f;
	m_vStiffness   = TVector4( a_fStiffness, a_fStiffness, a_fStiffness, 1.0f );
	m_vDamping     = TVector4( a_fDamping, a_fDamping, a_fDamping, 1.0f );
}

// $deBlob: FUNCTION 005ea9c0
void ASpring3D::Init( const TVector4& a_rcPosition, const TVector4& a_rcStiffness, const TVector4& a_rcDamping )
{
	m_vPosition    = a_rcPosition;
	m_vRest        = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vVelocity    = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vForce       = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_vStiffness   = a_rcStiffness;
	m_fMass        = 1.0f;
	m_fInverseMass = 1.0f;
	m_vDamping     = a_rcDamping;
}

// $deBlob: FUNCTION 005eaa20
void ASpring3D::Tick( TFLOAT a_fDeltaTime )
{
	m_vForce.x = -( m_vDamping.x * m_vVelocity.x ) + -( m_vStiffness.x * ( m_vPosition.x - m_vRest.x ) );
	m_vForce.y = -( m_vDamping.y * m_vVelocity.y ) + -( m_vStiffness.y * ( m_vPosition.y - m_vRest.y ) );
	m_vForce.z = -( m_vDamping.z * m_vVelocity.z ) + -( m_vStiffness.z * ( m_vPosition.z - m_vRest.z ) );

	TVector4 vAcceleration;
	vAcceleration.Multiply( m_vForce, m_fInverseMass );

	TVector4 vHalf;
	vHalf.Multiply( vAcceleration, 0.5f );

	TVector4 vHalfDt;
	vHalfDt.Multiply( vHalf, a_fDeltaTime );

	TVector4 vFromAcceleration;
	vFromAcceleration.Multiply( vHalfDt, a_fDeltaTime );

	TVector4 vFromVelocity;
	vFromVelocity.Multiply( m_vVelocity, a_fDeltaTime );

	m_vPosition.x = m_vPosition.x + vFromVelocity.x + vFromAcceleration.x;
	m_vPosition.y = m_vPosition.y + vFromVelocity.y + vFromAcceleration.y;
	m_vPosition.z = m_vPosition.z + vFromVelocity.z + vFromAcceleration.z;

	TVector4 vDeltaVelocity;
	vDeltaVelocity.Multiply( vAcceleration, a_fDeltaTime );
	m_vVelocity.x = m_vVelocity.x + vDeltaVelocity.x;
	m_vVelocity.y = m_vVelocity.y + vDeltaVelocity.y;
	m_vVelocity.z = m_vVelocity.z + vDeltaVelocity.z;

	m_vForce = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
}

// $deBlob: FUNCTION 005eabc0
void ASpring3D::AddVelocity( const TVector4& a_rcVelocity )
{
	m_vVelocity.x = a_rcVelocity.x + m_vVelocity.x;
	m_vVelocity.y = a_rcVelocity.y + m_vVelocity.y;
	m_vVelocity.z = a_rcVelocity.z + m_vVelocity.z;
}

// $deBlob: FUNCTION 005eac00
void ASpring3D::AddForce( const TVector4& a_rcForce )
{
	m_vForce.x = m_vForce.x + a_rcForce.x;
	m_vForce.y = a_rcForce.y + m_vForce.y;
	m_vForce.z = a_rcForce.z + m_vForce.z;
}

// $deBlob: FUNCTION 005eac40
void ASpring3D::SetStiffness( TFLOAT a_fStiffness )
{
	m_vStiffness.Set( a_fStiffness, a_fStiffness, a_fStiffness, 1.0f );
}

// $deBlob: FUNCTION 005eac70
void ASpring3D::SetDampingRatio( TFLOAT a_fRatio )
{
	TFLOAT* pStiffness = &m_vStiffness.x;
	TFLOAT* pDamping   = &m_vDamping.x;

	for ( TINT i = 0; i < 3; i++ )
		pDamping[ i ] = TFLOAT( sqrt( TDOUBLE( pStiffness[ i ] * 4.0f * m_fMass ) ) ) * a_fRatio;
}

// $deBlob: FUNCTION 005ead00
void ASpring3D::SetCriticalDamping()
{
	TFLOAT* pStiffness = &m_vStiffness.x;
	TFLOAT* pDamping   = &m_vDamping.x;

	for ( TINT i = 0; i < 3; i++ )
		pDamping[ i ] = TFLOAT( sqrt( TDOUBLE( pStiffness[ i ] * 4.0f * m_fMass ) ) );
}

TSTATICASSERT( sizeof( ASpringQuaternion ) == 0x40 );

// $deBlob: FUNCTION 005eadb0
ASpringQuaternion::ASpringQuaternion( const TVector4& a_rcOrientation, TFLOAT a_fStiffness, TFLOAT a_fDamping )
{
	m_vOrientation = a_rcOrientation;
	m_vRest        = TVector4::VEC_ZERO; // TQuaternion::IDENTITY
	m_vVelocity    = TVector4::VEC_ZERO; // TQuaternion::IDENTITY
	m_fMass        = 1.0f;
	m_fStiffness   = a_fStiffness;
	m_fDamping     = a_fDamping;
	m_fInverseMass = 1.0f;
}

// $deBlob: FUNCTION 005eae10
void ASpringQuaternion::Init( const TVector4& a_rcOrientation, TFLOAT a_fStiffness, TFLOAT a_fDamping )
{
	m_vOrientation = a_rcOrientation;
	m_vRest        = TVector4::VEC_ZERO; // TQuaternion::IDENTITY
	m_vVelocity    = TVector4::VEC_ZERO; // TQuaternion::IDENTITY
	m_fInverseMass = 1.0f;
	m_fStiffness   = a_fStiffness;
	m_fDamping     = a_fDamping;
}

// $deBlob: FUNCTION 005eae60
void ASpringQuaternion::Tick( TFLOAT a_fDeltaTime )
{
	TVector4 vOrientation = m_vOrientation;
	TVector4 vVelocity    = m_vVelocity;

	// NOTE: the fourth component's force ignores the rest orientation
	TVector4 vForce;
	vForce.x = -( m_fStiffness * ( m_vOrientation.x - m_vRest.x ) ) + -( m_fDamping * m_vVelocity.x );
	vForce.y = -( m_fStiffness * ( m_vOrientation.y - m_vRest.y ) ) + -( m_fDamping * m_vVelocity.y );
	vForce.z = -( m_fStiffness * ( m_vOrientation.z - m_vRest.z ) ) + -( m_fDamping * m_vVelocity.z );
	vForce.w = -( m_fStiffness * m_vOrientation.w ) + -( m_fDamping * m_vVelocity.w );

	TVector4 vAcceleration;
	vAcceleration.Multiply( vForce, m_fInverseMass );

	TVector4 vHalf;
	vHalf.Multiply( vAcceleration, 0.5f );

	TVector4 vHalfDt;
	vHalfDt.Multiply( vHalf, a_fDeltaTime );

	TVector4 vFromAcceleration;
	vFromAcceleration.Multiply( vHalfDt, a_fDeltaTime );

	TVector4 vFromVelocity;
	vFromVelocity.Multiply( vVelocity, a_fDeltaTime );

	vOrientation.x = vFromAcceleration.x + vFromVelocity.x + vOrientation.x;
	vOrientation.y = vFromAcceleration.y + vFromVelocity.y + vOrientation.y;
	vOrientation.z = vFromAcceleration.z + vFromVelocity.z + vOrientation.z;

	TVector4 vDeltaVelocity;
	vDeltaVelocity.Multiply( vAcceleration, a_fDeltaTime );
	vVelocity.x = vDeltaVelocity.x + vVelocity.x;
	vVelocity.y = vDeltaVelocity.y + vVelocity.y;
	vVelocity.z = vDeltaVelocity.z + vVelocity.z;

	vOrientation.Normalise();
	vVelocity.Normalise();

	// NOTE: retail stores the results shifted by one component: the old w first, then x, y, z
	m_vOrientation = TVector4( vOrientation.w, vOrientation.x, vOrientation.y, vOrientation.z );
	m_vVelocity    = TVector4( vVelocity.w, vVelocity.x, vVelocity.y, vVelocity.z );
}

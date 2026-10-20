#include "pch.h"
#include "ASpring.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ASpring ) == 0x1C );

// $deBlob: FUNCTION 005ea470
// $deBlob: FUNCTION 005ea640
ASpring::ASpring( TFLOAT a_fPosition, TFLOAT a_fStiffness, TFLOAT a_fDamping )
{
	m_fPosition    = a_fPosition;
	m_fStiffness   = a_fStiffness;
	m_fRest        = 0.0f;
	m_fVelocity    = 0.0f;
	m_fDamping     = a_fDamping;
	m_fMass        = 1.0f;
	m_fInverseMass = 1.0f;
}

// $deBlob: FUNCTION 005ea4c0
// $deBlob: FUNCTION 0079a350
ASpring::~ASpring()
{
}

// $deBlob: FUNCTION 005ea4d0
void ASpring::Tick( TFLOAT a_fDeltaTime )
{
	TFLOAT fVelocity     = m_fVelocity;
	TFLOAT fAcceleration = ( -( ( m_fPosition - m_fRest ) * m_fStiffness ) + -( m_fDamping * fVelocity ) ) * m_fInverseMass;

	m_fVelocity = fAcceleration * a_fDeltaTime + fVelocity;
	m_fPosition = fAcceleration * 0.5f * a_fDeltaTime * a_fDeltaTime + fVelocity * a_fDeltaTime + m_fPosition;
}

// $deBlob: FUNCTION 005ea590
void ASpring::SetDampingRatio( TFLOAT a_fRatio )
{
	m_fDamping = TFLOAT( sqrt( m_fStiffness * 4.0f * m_fMass ) ) * a_fRatio;
}

// $deBlob: FUNCTION 005ea5d0
void ASpring::SetCriticalDamping()
{
	m_fDamping = TFLOAT( sqrt( m_fStiffness * 4.0f * m_fMass ) );
}

// $deBlob: FUNCTION 005ea610
void ASpring::SetMass( TFLOAT a_fMass )
{
	m_fMass        = a_fMass;
	m_fInverseMass = 1.0f / a_fMass;
}

// $deBlob: FUNCTION 005ea6a0
void ASpring::TickAngle( TFLOAT a_fDeltaTime )
{
	TFLOAT fOffset       = WrapAngle( m_fPosition - m_fRest );
	TFLOAT fVelocity     = m_fVelocity;
	TFLOAT fAcceleration = ( -( m_fDamping * fVelocity ) + -( m_fStiffness * fOffset ) ) * m_fInverseMass;

	m_fVelocity = fAcceleration * a_fDeltaTime + fVelocity;
	m_fPosition = fAcceleration * 0.5f * a_fDeltaTime * a_fDeltaTime + fVelocity * a_fDeltaTime + m_fPosition;
	m_fPosition = WrapAngle( m_fPosition );
}

// $deBlob: FUNCTION 005eb220
TFLOAT ASpring::WrapAngle( TFLOAT a_fX )
{
	const TFLOAT fRangeLimitInCompleteTurns = 100.0f;
	TASSERT( TMath::Abs( a_fX ) * TMath::ONEOVERTWO_PI < fRangeLimitInCompleteTurns );

	TFLOAT fTurns = TMath::ONEOVERTWO_PI * a_fX + 100.5f;
	return ( ( fTurns - TFLOAT( TINT( fTurns ) ) ) - 0.5f ) * TMath::TWO_PI;
}

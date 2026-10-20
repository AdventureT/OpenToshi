#include "pch.h"
#include "APlayerMoverComponent.h"
#include "APlayerOnlyHashFilter.h"
#include "AEntityHash.h"
#include "ABaseEntity.h"

#include <Toshi/TSystem.h>
#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007e59a0
APlayerMoverComponent::APlayerMoverComponent( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
{
	m_vPosition     = a_pEntity->GetTranslation();
	m_vPrevPosition = a_pEntity->GetTranslation();
	TMatrix44::MatToQuat( m_quatRotation, m_pEntity->GetMatrix() );
	m_quatPrevRotation = m_quatRotation;
}

// $deBlob: FUNCTION 007cd9c0
// $deBlob: FUNCTION 007cd910
APlayerMoverComponent::~APlayerMoverComponent()
{
}

// $deBlob: FUNCTION 007e5a30
void APlayerMoverComponent::Tick( TFLOAT a_fDeltaTime )
{
	ABaseEntity* pEntity = m_pEntity;

	m_vPrevPosition    = m_vPosition;
	m_vPosition        = pEntity->GetTranslation();
	m_quatPrevRotation = m_quatRotation;
	TMatrix44::MatToQuat( m_quatRotation, pEntity->GetMatrix() );

	if ( m_quatRotation != m_quatPrevRotation || !m_vPrevPosition.IsEqual( m_vPosition ) )
	{
		// NOTE: retail doesn't use the result of the query
		TINT                  iCount = 0;
		APlayerOnlyHashFilter oFilter;
		AEntityHash::GetSingleton()->Query( m_pEntity->GetBasePosition(), m_pEntity->GetBoundingSphereRadius(), &iCount, &oFilter, TTRUE, 500, TFALSE, TTRUE, TFALSE );
	}
}

// $deBlob: FUNCTION 007e5ae0
TVector4 APlayerMoverComponent::GetReactionForce( const TVector4& a_rvPoint, ABaseEntity* a_pEntity )
{
	TVector4 vForce    = TVector4::VEC_ZERO;
	TFLOAT   fInvDelta = 1.0f / TSystemManager::GetSingleton()->GetTimer().GetDelta();

	if ( m_quatRotation != m_quatPrevRotation )
	{
		// The shorter way between the rotations
		TQuaternion quatDiff   = m_quatRotation - m_quatPrevRotation;
		TFLOAT      fDiffSize = TFLOAT( sqrt( quatDiff.x * quatDiff.x + quatDiff.y * quatDiff.y + quatDiff.z * quatDiff.z + quatDiff.w * quatDiff.w ) );

		if ( fDiffSize > 1.99f && fDiffSize < 2.01f )
			m_quatPrevRotation.Negate();

		// The angular velocity: 2 * dq/dt * q^-1
		TQuaternion quatConjugate( -m_quatRotation.x, -m_quatRotation.y, -m_quatRotation.z, m_quatRotation.w );
		TQuaternion quatAngular = ( ( m_quatRotation - m_quatPrevRotation ) * ( fInvDelta + fInvDelta ) ) * quatConjugate;

		const TVector4& rvEntityPos = a_pEntity->GetTranslation();
		TVector4        vOffset( rvEntityPos.x - m_vPosition.x, rvEntityPos.y - m_vPosition.y, rvEntityPos.z - m_vPosition.z, rvEntityPos.w );

		vForce.CrossProduct( TVector4( quatAngular.x, quatAngular.y, quatAngular.z, quatAngular.w ), vOffset );
		vForce.w = 1.0f;
	}

	if ( !m_vPosition.IsEqual( m_vPrevPosition ) )
	{
		TVector4 vMove( m_vPosition.x - m_vPrevPosition.x, m_vPosition.y - m_vPrevPosition.y, m_vPosition.z - m_vPrevPosition.z, m_vPosition.w );

		TVector4 vVelocity;
		vVelocity.Multiply( vMove, fInvDelta );

		vForce.x += vVelocity.x;
		vForce.y += vVelocity.y;
		vForce.z += vVelocity.z;
	}

	return vForce;
}

// $deBlob: FUNCTION 007e5d10
TQuaternion APlayerMoverComponent::GetReactionRotation( ABaseEntity* a_pEntity )
{
	TQuaternion quatConjugate( -m_quatPrevRotation.x, -m_quatPrevRotation.y, -m_quatPrevRotation.z, m_quatPrevRotation.w );
	TQuaternion quatRotation = m_quatRotation * quatConjugate;

	quatRotation /= TFLOAT( sqrt( quatRotation.x * quatRotation.x + quatRotation.y * quatRotation.y + quatRotation.z * quatRotation.z + quatRotation.w * quatRotation.w ) );
	return quatRotation;
}

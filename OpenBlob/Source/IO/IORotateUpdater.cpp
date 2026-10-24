#include "pch.h"
#include "IORotateUpdater.h"
#include "IO/AIOEntity.h"

#include <Math/TMath.h>
#include <Math/TVector3.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TCONSTEXPR_VAR TFLOAT s_fRadToDeg = 57.295776f;
static TCONSTEXPR_VAR TFLOAT s_fDegToRad = 0.017453292f;

// The rotation of a matrix around a_iAxis (0: X, 1: Z, 2: Y)
static void RotateAroundAxis( TMatrix44& a_rMatrix, TINT a_iAxis, TFLOAT a_fAngle )
{
	if ( a_iAxis == 0 )
		a_rMatrix.RotateX( a_fAngle );
	else if ( a_iAxis == 1 )
		a_rMatrix.RotateZ( a_fAngle );
	else if ( a_iAxis == 2 )
		a_rMatrix.RotateY( a_fAngle );
}

void IORotateUpdater::ReadStartAngle( const Params* a_pParams )
{
	TMatrix44 matEntity = m_pOwner->GetMatrix();

	TVector3 vEuler;
	matEntity.GetEulerXYZ( vEuler );

	if ( a_pParams->iAxis == 0 )
		m_fStartAngle = vEuler.x * s_fRadToDeg;
	else if ( a_pParams->iAxis == 1 )
		m_fStartAngle = vEuler.z * s_fRadToDeg;
	else if ( a_pParams->iAxis == 2 )
		m_fStartAngle = vEuler.y * s_fRadToDeg;
}

// $deBlob: FUNCTION 007cae70
void IORotateUpdater::Activate()
{
	const Params* pParams = GetParams<Params>();

	m_fSpeed       = 0.0f;
	m_fTime        = 0.0f;
	m_bReverse     = TFALSE;
	m_bMessageSent = TFALSE;
	m_fRotated     = 0.0f;

	if ( pParams->fDuration == -1.0f )
	{
		ReadStartAngle( pParams );
		m_fTotalAngle = TMath::GetAngleDifference( m_fStartAngle * s_fDegToRad, pParams->fAngle * s_fDegToRad ) * s_fRadToDeg;
	}

	if ( pParams->iAbsolute == 0 )
	{
		// By the angle
		m_fTotalAngle = pParams->fAngle;
		m_bActive     = TTRUE;

		if ( pParams->iDirection == 1 )
			m_bReverse = TTRUE;

		return;
	}

	// To the angle
	ReadStartAngle( pParams );

	if ( pParams->iDirection == 1 )
	{
		if ( pParams->fAngle < m_fStartAngle )
		{
			m_bActive     = TTRUE;
			m_fTotalAngle = ( 360.0f - m_fStartAngle ) + pParams->fAngle;
			return;
		}

		m_bActive     = TTRUE;
		m_fTotalAngle = TMath::Abs( pParams->fAngle - m_fStartAngle );
	}
	else if ( pParams->iDirection == 2 )
	{
		if ( pParams->fAngle <= m_fStartAngle )
		{
			m_bReverse    = TTRUE;
			m_bActive     = TTRUE;
			m_fTotalAngle = TMath::Abs( m_fStartAngle - pParams->fAngle );
			return;
		}

		m_bReverse    = TTRUE;
		m_bActive     = TTRUE;
		m_fTotalAngle = ( 360.0f - pParams->fAngle ) + m_fStartAngle;
	}
	else
	{
		// The shorter way
		m_fTotalAngle = TMath::GetAngleDifference( m_fStartAngle * s_fDegToRad, pParams->fAngle * s_fDegToRad ) * s_fRadToDeg;

		if ( m_fStartAngle < 90.0f && pParams->fAngle < 270.0f )
		{
			m_bReverse = TTRUE;
			m_bActive  = TTRUE;
			return;
		}

		m_bActive = TTRUE;

		if ( pParams->fAngle < m_fStartAngle )
			m_bReverse = TTRUE;
	}
}

// $deBlob: FUNCTION 007cb130
void IORotateUpdater::Update( TFLOAT a_fDeltaTime )
{
	const Params* pParams = GetParams<Params>();

	if ( pParams->fDuration == -1.0f )
	{
		RotateUsingVelocity( a_fDeltaTime, pParams );
		return;
	}

	RotateUsingDuration( a_fDeltaTime, pParams );
}

// $deBlob: FUNCTION 007cb170
void IORotateUpdater::RotateUsingDuration( TFLOAT a_fDeltaTime, const Params* a_pParams )
{
	TFLOAT    fDuration = a_pParams->fDuration;
	TMatrix44 matEntity = m_pOwner->GetMatrix();

	TFLOAT fStep = a_fDeltaTime;

	if ( fDuration < m_fTime + a_fDeltaTime )
		fStep = fDuration - m_fTime;

	TFLOAT fAngle = ( m_fTotalAngle / fDuration ) * fStep;

	if ( m_bReverse )
		fAngle *= -1.0f;

	RotateAroundAxis( matEntity, a_pParams->iAxis, fAngle * s_fDegToRad );
	m_pOwner->SetMatrix( matEntity );

	m_fTime += a_fDeltaTime;

	if ( a_pParams->fDuration <= m_fTime )
	{
		if ( a_pParams->iLoop != 0 )
		{
			m_fTime = 0.0f;
			return;
		}

		m_bActive = TFALSE;

		if ( !m_bMessageSent && a_pParams->iFinishedMessage != 0 )
		{
			m_bMessageSent = TTRUE;
			SendFinishedMessage( a_pParams->iFinishedMessage );
		}
	}
}

// $deBlob: FUNCTION 007cb370
void IORotateUpdater::RotateUsingVelocity( TFLOAT a_fDeltaTime, const Params* a_pParams )
{
	// NOTE: the speed grows by the acceleration * dt but is capped to the maximum speed * dt
	m_fSpeed = a_pParams->fAcceleration * a_fDeltaTime + m_fSpeed;

	TFLOAT fMaxStep = a_pParams->fMaxSpeed * a_fDeltaTime;

	if ( fMaxStep < m_fSpeed )
		m_fSpeed = fMaxStep;

	TFLOAT    fStep     = m_fSpeed;
	TMatrix44 matEntity = m_pOwner->GetMatrix();

	if ( m_fTotalAngle < m_fRotated + fStep )
		fStep = m_fTotalAngle - m_fRotated;

	TFLOAT fSign = m_bReverse ? -1.0f : 1.0f;

	RotateAroundAxis( matEntity, a_pParams->iAxis, fStep * s_fDegToRad * fSign );
	m_pOwner->SetMatrix( matEntity );

	m_fRotated += fStep;

	if ( m_fTotalAngle <= m_fRotated )
	{
		if ( a_pParams->iLoop != 0 )
		{
			m_fRotated = 0.0f;
			return;
		}

		m_bActive = TFALSE;

		if ( !m_bMessageSent && a_pParams->iFinishedMessage != 0 )
		{
			m_bMessageSent = TTRUE;
			SendFinishedMessage( a_pParams->iFinishedMessage );
		}
	}
}

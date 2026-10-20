#include "pch.h"
#include "IOShakeUpdater.h"
#include "IO/AIOEntity.h"

#include <Math/TMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TCONSTEXPR_VAR TFLOAT s_fDegToRad = 0.017453292f;

// $deBlob: FUNCTION 00786830
IOShakeUpdater::IOShakeUpdater()
{
	m_pShaker       = TNULL;
	m_iTilt         = -1;
	m_fTiltAngle    = 0.0f;
	m_bTiltBack     = TFALSE;
	m_fBounce       = 0.0f;
	m_fBounceHeight = 0.1f;
	m_fBaseHeight   = 0.0f;
	m_iBounce       = -1;
	m_bBounceDown   = TFALSE;
}

// $deBlob: FUNCTION 007cbe60
void IOShakeUpdater::Activate()
{
	if ( m_bActive )
		return;

	const Params* pParams = GetParams<Params>();

	// The direction to the shaker
	TVector4 vDirection;

	if ( m_pShaker == TNULL )
	{
		vDirection = TVector4::VEC_POSX;
	}
	else
	{
		const TVector4& rvShaker = m_pShaker->GetTranslation();
		const TVector4& rvOwner  = m_pOwner->GetTranslation();
		vDirection               = TVector4( rvShaker.x - rvOwner.x, rvShaker.y - rvOwner.y, rvShaker.z - rvOwner.z, rvShaker.w );
	}

	vDirection.Normalise();
	TFLOAT fAngle = pParams->fTiltAngle * s_fDegToRad;

	// The closest of +X, -X, -Z and +Z
	TFLOAT fBest = TVector4::DotProduct4( vDirection, TVector4::VEC_POSX );
	TFLOAT fDot  = TVector4::DotProduct4( vDirection, TVector4::VEC_NEGX );

	TINT iDirection = fBest < fDot;

	if ( fBest < fDot )
		fBest = fDot;

	fDot = TVector4::DotProduct4( vDirection, TVector4::VEC_NEGZ );

	if ( fBest < fDot )
	{
		iDirection = 2;
		fBest      = fDot;
	}

	fDot = TVector4::DotProduct4( vDirection, TVector4::VEC_POSZ );

	if ( fBest < fDot )
		iDirection = 3;

	if ( ( iDirection & 0x80000001 ) == 1 )
		fAngle *= -1.0f;

	m_aTilts[ 0 ].fAngle     = fAngle;
	m_aTilts[ 0 ].iDirection = iDirection;
	m_aTilts[ 1 ].iDirection = iDirection;
	m_aTilts[ 1 ].fAngle     = -( pParams->fDamping * fAngle );
	m_aTilts[ 2 ].iDirection = iDirection;
	m_aTilts[ 2 ].fAngle     = -( pParams->fDamping * m_aTilts[ 1 ].fAngle );

	m_iTilt         = 0;
	m_fTiltAngle    = 0.0f;
	m_bTiltBack     = TFALSE;
	m_fBounceHeight = pParams->fBounceHeight;
	m_fBounce       = 0.0f;
	m_iBounce       = 0;
	m_bBounceDown   = TFALSE;
	m_fBaseHeight   = m_pOwner->GetTranslation().y;
	m_bActive       = TTRUE;
}

// $deBlob: FUNCTION 007cbbf0
void IOShakeUpdater::Update( TFLOAT a_fDeltaTime )
{
	const Params* pParams = GetParams<Params>();

	if ( m_iTilt >= 0 && m_iTilt < NUM_STAGES )
	{
		TFLOAT    fStep     = pParams->fTiltSpeed * a_fDeltaTime;
		TMatrix44 matEntity = m_pOwner->GetMatrix();

		TFLOAT fTarget = m_aTilts[ m_iTilt ].fAngle;

		if ( m_bTiltBack == TTRUE )
			fTarget *= -1.0f;

		if ( fTarget < 0.0f || m_fTiltAngle + fStep <= fTarget )
		{
			if ( fTarget < 0.0f )
			{
				if ( m_fTiltAngle - fStep < fTarget )
					fStep = ( fTarget - m_fTiltAngle ) * -1.0f;

				fStep = -fStep;
			}
		}
		else
		{
			fStep = fTarget - m_fTiltAngle;
		}

		TINT iDirection = m_aTilts[ m_iTilt ].iDirection;

		if ( iDirection == 0 || iDirection == 1 )
			matEntity.RotateZ( fStep );
		else
			matEntity.RotateX( fStep );

		m_pOwner->SetMatrix( matEntity );
		m_fTiltAngle += fStep;

		if ( ( fStep < 0.0f && m_fTiltAngle <= fTarget ) || ( 0.0f <= fStep && fTarget <= m_fTiltAngle ) )
		{
			m_fTiltAngle = 0.0f;

			if ( !m_bTiltBack )
			{
				m_bTiltBack = TTRUE;
			}
			else
			{
				m_iTilt++;
				m_bTiltBack = TFALSE;
			}
		}
	}

	if ( m_iBounce > -1 && m_iBounce < NUM_STAGES )
	{
		// NOTE: the side of the bounce follows the tilt (m_bTiltBack), not m_bBounceDown
		TVector4 vPosition = m_pOwner->GetTranslation();
		TFLOAT   fHeight   = TMath::LERP( 0.0f, m_fBounceHeight, m_fBounce );
		vPosition.y        = TFLOAT( m_bTiltBack ? 1 : -1 ) * fHeight + m_fBaseHeight;
		m_pOwner->SetPosition( vPosition );

		if ( !m_bBounceDown )
		{
			if ( 1.0f <= m_fBounce )
				m_bBounceDown = TTRUE;
		}
		else if ( m_fBounce <= 0.0f )
		{
			m_iBounce++;
			m_bBounceDown = TFALSE;
		}

		m_fBounce = TFLOAT( m_bBounceDown ? -1 : 1 ) * pParams->fBounceSpeed * a_fDeltaTime + m_fBounce;
		TMath::Clip( m_fBounce, 0.0f, 1.0f );
	}

	if ( ( m_iBounce < 0 || m_iBounce > 2 ) && ( m_iTilt < 0 || m_iTilt > 2 ) )
		m_bActive = TFALSE;
}

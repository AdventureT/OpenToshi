#include "pch.h"
#include "IOMoveUpdater.h"
#include "IO/AIOEntity.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007cb5b0
void IOMoveUpdater::Activate()
{
	m_vVelocity    = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_vStart       = m_pOwner->GetTranslation();
	m_bMessageSent = TFALSE;

	const Params* pParams = GetParams<Params>();

	if ( pParams->iToTarget != 1 )
	{
		// Z up to Y up
		m_vEnd = TVector4( pParams->fOffsetX, -pParams->fOffsetZ, pParams->fOffsetY, 1.0f );

		const TVector4& rvFrom = ( pParams->iFromStart == 0 ) ? m_vStart : m_pOwner->GetLocalMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
		m_vEnd.x += rvFrom.x;
		m_vEnd.y += rvFrom.y;
		m_vEnd.z += rvFrom.z;

		m_fTime   = 0.0f;
		m_bActive = TTRUE;
		return;
	}

	if ( m_pTarget != TNULL )
	{
		m_fTime   = 0.0f;
		m_bActive = TTRUE;
		m_vEnd    = m_pTarget->GetTranslation();
		return;
	}

	m_bActive = TFALSE;
}

void IOMoveUpdater::CheckMessageDistance( const Params* a_pParams, const TVector4& a_rvPosition )
{
	if ( m_bMessageSent || a_pParams->iFinishedMessage == 0 )
		return;

	TVector4 vToEnd( m_vEnd.x - a_rvPosition.x, m_vEnd.y - a_rvPosition.y, m_vEnd.z - a_rvPosition.z, m_vEnd.w );

	if ( vToEnd.MagnitudeSq() <= a_pParams->fMessageDistance * a_pParams->fMessageDistance )
	{
		m_bMessageSent = TTRUE;
		SendFinishedMessage( a_pParams->iFinishedMessage );
	}
}

// $deBlob: FUNCTION 007cb6a0
void IOMoveUpdater::Update( TFLOAT a_fDeltaTime )
{
	const Params* pParams = GetParams<Params>();

	if ( pParams->fDuration == -1.0f )
	{
		// Accelerates towards the end
		TVector4 vPosition = m_pOwner->GetTranslation();
		TVector4 vToEnd( m_vEnd.x - vPosition.x, m_vEnd.y - vPosition.y, m_vEnd.z - vPosition.z, m_vEnd.w );

		TFLOAT fDistanceSq = vToEnd.MagnitudeSq();
		vToEnd.Normalise();
		vToEnd.Multiply( pParams->fAcceleration );

		TVector4 vAcceleration;
		vAcceleration.Multiply( vToEnd, a_fDeltaTime );
		m_vVelocity.x += vAcceleration.x;
		m_vVelocity.y += vAcceleration.y;
		m_vVelocity.z += vAcceleration.z;

		m_vVelocity.ClampLength( pParams->fMaxSpeed * a_fDeltaTime );

		if ( fDistanceSq < m_vVelocity.MagnitudeSq() )
		{
			// Reached the end
			m_pOwner->SetPosition( m_vEnd );
			m_bActive = TFALSE;

			if ( !m_bMessageSent && pParams->iFinishedMessage != 0 )
			{
				m_bMessageSent = TTRUE;
				SendFinishedMessage( pParams->iFinishedMessage );
			}

			return;
		}

		vPosition.x += m_vVelocity.x;
		vPosition.y += m_vVelocity.y;
		vPosition.z += m_vVelocity.z;
		m_pOwner->SetPosition( vPosition );

		CheckMessageDistance( pParams, vPosition );
		return;
	}

	m_fTime += a_fDeltaTime;
	TFLOAT fProgress = m_fTime / pParams->fDuration;

	TVector4 vPosition;

	if ( fProgress < 1.0f )
	{
		vPosition.Lerp4( m_vStart, m_vEnd, fProgress );
	}
	else
	{
		vPosition = m_vEnd;
		m_bActive = TFALSE;

		if ( !m_bMessageSent && pParams->iFinishedMessage != 0 )
		{
			m_bMessageSent = TTRUE;
			SendFinishedMessage( pParams->iFinishedMessage );
		}
	}

	CheckMessageDistance( pParams, vPosition );
	m_pOwner->SetPosition( vPosition );
}

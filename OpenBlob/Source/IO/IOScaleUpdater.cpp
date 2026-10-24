#include "pch.h"
#include "IOScaleUpdater.h"
#include "IO/AIOEntity.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007cc0a0
void IOScaleUpdater::Update( TFLOAT a_fDeltaTime )
{
	m_fTime -= a_fDeltaTime;

	if ( m_fTime < 0.0f )
		m_fTime = 0.0f;

	const Params* pParams = GetParams<Params>();

	TVector4 vScale( pParams->fScaleX, pParams->fScaleY, pParams->fScaleZ, 1.0f );
	TVector4 vCurrent;
	vCurrent.Lerp3( m_vStartScale, vScale, 1.0f - m_fTime / pParams->fDuration );

	m_pOwner->SetScale( vCurrent, pParams->fCollision != 0.0f );

	if ( m_fTime == 0.0f )
		m_bActive = TFALSE;
}

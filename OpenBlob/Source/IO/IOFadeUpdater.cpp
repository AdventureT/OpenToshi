#include "pch.h"
#include "IOFadeUpdater.h"
#include "IO/AIOEntity.h"

#include <Math/TMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007cc010
void IOFadeUpdater::Update( TFLOAT a_fDeltaTime )
{
	m_fTime -= a_fDeltaTime;

	if ( m_fTime < 0.0f )
		m_fTime = 0.0f;

	const Params* pParams = GetParams<Params>();
	TFLOAT        fAlpha  = TMath::LERP( m_fStartAlpha, pParams->fAlpha, 1.0f - m_fTime / pParams->fDuration );

	m_pOwner->SetAlpha( fAlpha );

	if ( m_fTime == 0.0f )
		m_bActive = TFALSE;
}

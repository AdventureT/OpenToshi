#include "ToshiPCH.h"
#include "TPhysicsConstraintContainer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TSTATICASSERT( sizeof( TPhysicsConstraintContainer ) == 0x74 );

// $deBlob: FUNCTION 00730eb0
TPhysicsConstraintContainer::TPhysicsConstraintContainer()
{
	m_eType          = 0;
	m_uiFlags        = 0;
	m_iWorld         = 0;
	m_bInWorld       = TFALSE;
	m_bWantRuntime    = TFALSE;
	m_pBreakableData = TNULL;
	m_pBodyB         = TNULL;
	m_pBodyA         = TNULL;
	m_pData          = TNULL;
	m_pInstance      = TNULL;
	m_vPivotA        = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_vPivotB        = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_vSteeringAxis   = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_vSuspensionAxis = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_vLimits         = TVector4( 0.0f, 0.0f, 0.0f, 0.0f );
}

TOSHI_NAMESPACE_END

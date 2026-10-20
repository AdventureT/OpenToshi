#include "pch.h"
#include "AEntityComponent2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007c4270
AEntityComponent2::AEntityComponent2( ABaseEntity* a_pEntity )
{
	m_pEntity  = a_pEntity;
	m_bEnabled = TTRUE;
}

// $deBlob: FUNCTION 007c42a0
// $deBlob: FUNCTION 007c4330
AEntityComponent2::~AEntityComponent2()
{
}

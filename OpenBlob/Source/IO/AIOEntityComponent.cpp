#include "pch.h"
#include "AIOEntityComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007e5580
AIOEntityComponent::AIOEntityComponent( AIOEntity* a_pOwner, TINT a_iUnk )
{
	m_iListType = a_iUnk;
	m_pOwner    = a_pOwner;
	m_pNext     = TNULL;
	m_uiFlags   = 0;
}

// $deBlob: FUNCTION 007e55b0
// $deBlob: FUNCTION 007e55c0
AIOEntityComponent::~AIOEntityComponent()
{
}

// $deBlob: FUNCTION 007cc440
TBOOL AIOEntityComponent::ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther )
{
	return TFALSE;
}

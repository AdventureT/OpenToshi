#include "pch.h"
#include "AIOPopupComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007cc610
AIOPopupComponent::AIOPopupComponent( AIOEntity* a_pOwner )
    : AIOEntityComponent( a_pOwner, TYPE )
{
}

// $deBlob: FUNCTION 007cc6b0
// $deBlob: FUNCTION 007cc630
AIOPopupComponent::~AIOPopupComponent()
{
}

// $deBlob: FUNCTION 007cc640
void AIOPopupComponent::SetIOEntityData( void* a_pData, TTRB* a_pTRB )
{
	IOEntityData* pIOData = TSTATICCAST( IOEntityData, a_pData );

	TBOOL bCorrectType = pIOData->iType == TYPE;
	TASSERT( TTRUE == bCorrectType );

	m_iValue = *pIOData->pData;
}

// $deBlob: FUNCTION 007cc690
TBOOL AIOPopupComponent::ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther )
{
	if ( a_pMessage[ 0 ] == ACTION_Popup )
		return TTRUE;

	return TFALSE;
}

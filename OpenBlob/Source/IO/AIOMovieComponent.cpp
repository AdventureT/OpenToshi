#include "pch.h"
#include "AIOMovieComponent.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AMovieState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007cc6e0
AIOMovieComponent::AIOMovieComponent( AIOEntity* a_pOwner )
    : AIOEntityComponent( a_pOwner, TYPE )
{
}

// $deBlob: FUNCTION 007cc810
// $deBlob: FUNCTION 007cc700
AIOMovieComponent::~AIOMovieComponent()
{
}

// $deBlob: FUNCTION 007cc710
void AIOMovieComponent::SetIOEntityData( void* a_pData, TTRB* a_pTRB )
{
	IOEntityData* pIOData = TSTATICCAST( IOEntityData, a_pData );

	TBOOL bCorrectType = pIOData->iType == TYPE;
	TASSERT( TTRUE == bCorrectType );

	m_szMovie = *pIOData->pData;
}

// $deBlob: FUNCTION 007cc760
TBOOL AIOMovieComponent::ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther )
{
	if ( a_pMessage[ 0 ] != ACTION_PlayMovie )
		return TFALSE;

	AGameStateController::GetSingleton()->PushState( new AMovieState( m_szMovie, TTRUE, TFALSE, TNULL ) );

	return TTRUE;
}

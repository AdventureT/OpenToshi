#include "pch.h"
#include "APathfindingComponent.h"
#include "AAIMoveProbe.h"
#include "AAISteer.h"
#include "AAIPathFinder.h"
#include "AAINavigator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APathfindingComponent ) == 0x48 );

// $deBlob: FUNCTION 007e5280
APathfindingComponent::APathfindingComponent( ABaseEntity* a_pEntity, AEntityComponent2* a_pMotor )
    : AEntityComponent2( a_pEntity )
{
	m_pMotor          = a_pMotor;
	m_iCollisionFlags = 0;
	m_uiFlags         = 0;
	m_iUnk30          = 0;
	m_pMoveProbe      = new AAIMoveProbe( a_pEntity );
	m_pSteer          = new AAISteer( a_pEntity );
	m_pPathFinder     = new AAIPathFinder( a_pEntity );
	m_pNavigator      = new AAINavigator( a_pEntity );

	m_oObstruction.pObject = TNULL;
	m_oObstruction.vNormal = TVector4::VEC_ZERO;
}

// $deBlob: FUNCTION 007e54e0
// $deBlob: FUNCTION 007e53b0
APathfindingComponent::~APathfindingComponent()
{
	if ( m_pMoveProbe != TNULL )
		delete m_pMoveProbe;

	if ( m_pSteer != TNULL )
		delete m_pSteer;

	if ( m_pPathFinder != TNULL )
		delete m_pPathFinder;

	if ( m_pNavigator != TNULL )
		delete m_pNavigator;
}

// $deBlob: FUNCTION 007e5470
void APathfindingComponent::Tick( TFLOAT a_fDeltaTime )
{
	m_pMoveProbe->Tick( a_fDeltaTime );
	m_pSteer->Tick( a_fDeltaTime );
	m_pPathFinder->Tick( a_fDeltaTime );
	m_pNavigator->Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 007e5440
void APathfindingComponent::ResetComponent()
{
	m_pMoveProbe->ResetComponent();
	m_pSteer->ResetComponent();
	m_pPathFinder->ResetComponent();
	m_pNavigator->ResetComponent();
}

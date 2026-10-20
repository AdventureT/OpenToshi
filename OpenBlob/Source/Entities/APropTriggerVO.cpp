#include "pch.h"
#include "APropTriggerVO.h"
#include "AEntityParams.h"
#include "ACharEntity.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropTriggerVO ) == 0xC4 );

// $deBlob: FUNCTION 0079bfc0
// $deBlob: FUNCTION 0079bfd0
// $deBlob: FUNCTION 0079c050
// $deBlob: FUNCTION 0079c0d0
// $deBlob: FUNCTION 0079c0e0
// $deBlob: FUNCTION 00540da0
// $deBlob: FUNCTION 00859df0
TDEFINE_CLASS( APropTriggerVO );

// $deBlob: FUNCTION 0079c0f0
APropTriggerVO::APropTriggerVO()
{
}

// $deBlob: FUNCTION 0079c110
// $deBlob: FUNCTION 0079c21e
APropTriggerVO::~APropTriggerVO()
{
}

// $deBlob: FUNCTION 0079c130
// $deBlob: FUNCTION 0079c26c
void APropTriggerVO::ProcessParams( const AEntityParams& a_rParams )
{
	APropTrigger::ProcessParams( a_rParams );

	// NOTE: retail reads the VO into a local and drops it
	const TCHAR* szVO;
	a_rParams.GetValue( szVO, "VO_ID" );
}

// $deBlob: FUNCTION 0079c160
// $deBlob: FUNCTION 0079c261
void APropTriggerVO::PostCreate()
{
	APropTrigger::PostCreate();
}

// $deBlob: FUNCTION 0079c180
TBOOL APropTriggerVO::CanObjectTrigger( TObject* a_pObject )
{
	TBOOL bCanTrigger = APropTrigger::CanObjectTrigger( a_pObject );

	if ( bCanTrigger && a_pObject->IsA( &TGetClass( ACharEntity ) ) )
		return ( a_pObject->CastTo<ACharEntity>()->GetFlags() & 0x10 ) != 0;

	return bCanTrigger;
}

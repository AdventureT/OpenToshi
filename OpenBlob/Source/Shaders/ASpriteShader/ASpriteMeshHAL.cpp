#include "pch.h"
#include "ASpriteMeshHAL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00569460
// $deBlob: FUNCTION 00569470
// $deBlob: FUNCTION 005694e0
// $deBlob: FUNCTION 00569550
// $deBlob: FUNCTION 00569560
// $deBlob: FUNCTION 0053b5a0
// $deBlob: FUNCTION 00857f10
TDEFINE_CLASS( ASpriteMeshHAL );

// $deBlob: FUNCTION 00569570
ASpriteMeshHAL::ASpriteMeshHAL()
{
}

// $deBlob: FUNCTION 00569630
// $deBlob: FUNCTION 00569590
ASpriteMeshHAL::~ASpriteMeshHAL()
{
}

// $deBlob: FUNCTION 005695f0
TBOOL ASpriteMeshHAL::Render()
{
	return TTRUE;
}

// $deBlob: FUNCTION 005695a0
TBOOL ASpriteMeshHAL::Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices )
{
	TASSERT( !IsCreated() );

	SetFlags( State_Created, TTRUE );
	m_uiFlags = a_uiFlags;

	return TTRUE;
}

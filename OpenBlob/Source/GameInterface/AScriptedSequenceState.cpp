#include "pch.h"
#include "AScriptedSequenceState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AScriptedSequenceState ) == 0x3C );

// $deBlob: FUNCTION 005d2a10
// $deBlob: FUNCTION 005d2a20
// $deBlob: FUNCTION 005d2ab0
// $deBlob: FUNCTION 005d2b40
// $deBlob: FUNCTION 005d2b50
// $deBlob: FUNCTION 0053c770
// $deBlob: FUNCTION 00858520
TDEFINE_CLASS( AScriptedSequenceState );

// $deBlob: FUNCTION 005d2b60
AScriptedSequenceState::AScriptedSequenceState( const TCHAR* a_szName )
    : m_strName( a_szName )
{
	m_uiUnk38 = 0;
}

// $deBlob: FUNCTION 005d2bd0
// $deBlob: FUNCTION 005d2c90
AScriptedSequenceState::~AScriptedSequenceState()
{
}

// $deBlob: FUNCTION 005d2c30
AGameState::UpdateResult AScriptedSequenceState::OnUpdate( TFLOAT a_fDeltaTime )
{
	return AGameState::OnUpdate( a_fDeltaTime );
}

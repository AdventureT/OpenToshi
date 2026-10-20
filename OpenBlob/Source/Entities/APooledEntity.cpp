#include "pch.h"
#include "APooledEntity.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APooledEntity ) == 0x14 );

// $deBlob: FUNCTION 00791400
APooledEntity::APooledEntity()
    : m_pEntity( TNULL )
    , m_eType( -1 )
{
}

// $deBlob: FUNCTION 00791600
// $deBlob: FUNCTION 007914a0
APooledEntity::~APooledEntity()
{
	// The node's destructor asserts that the entry was removed from its list
}

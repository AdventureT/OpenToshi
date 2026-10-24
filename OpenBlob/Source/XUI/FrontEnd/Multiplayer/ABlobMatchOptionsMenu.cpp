#include "pch.h"
#include "ABlobMatchOptionsMenu.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobMatchOptionsMenu ) == 0x140 );

// $deBlob: FUNCTION 0063ed20
// $deBlob: FUNCTION 0063ed30
// $deBlob: FUNCTION 0063edb0
// $deBlob: FUNCTION 0063ee30
// $deBlob: FUNCTION 0063ee40
// $deBlob: FUNCTION 0053da90
// $deBlob: FUNCTION 00858a20
TDEFINE_CLASS( ABlobMatchOptionsMenu );

// $deBlob: FUNCTION 0063ee50
ABlobMatchOptionsMenu::ABlobMatchOptionsMenu()
    : ABlobMenu( -1 )
{
}

// $deBlob: FUNCTION 0063ee70
// $deBlob: FUNCTION 0063ee82
// $deBlob: FUNCTION 0063ee90
ABlobMatchOptionsMenu::~ABlobMatchOptionsMenu()
{
}

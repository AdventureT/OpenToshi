#include "pch.h"
#include "ABlobChoosePatternsMenu.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobChoosePatternsMenu ) == 0x140 );

// $deBlob: FUNCTION 0063dd20
// $deBlob: FUNCTION 0063dd30
// $deBlob: FUNCTION 0063ddb0
// $deBlob: FUNCTION 0063de30
// $deBlob: FUNCTION 0063de40
// $deBlob: FUNCTION 0053da10
// $deBlob: FUNCTION 00858a00
TDEFINE_CLASS( ABlobChoosePatternsMenu );

// $deBlob: FUNCTION 0063de50
ABlobChoosePatternsMenu::ABlobChoosePatternsMenu()
    : ABlobMenu( -1 )
{
}

// $deBlob: FUNCTION 0063de70
// $deBlob: FUNCTION 0063de82
// $deBlob: FUNCTION 0063de90
ABlobChoosePatternsMenu::~ABlobChoosePatternsMenu()
{
}

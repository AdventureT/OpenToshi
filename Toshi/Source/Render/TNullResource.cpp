#include "ToshiPCH.h"
#include "TNullResource.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006b5670
// $deBlob: FUNCTION 006b5680
// $deBlob: FUNCTION 006b56e0
// $deBlob: FUNCTION 006b5740
// $deBlob: FUNCTION 006b5750
// $deBlob: FUNCTION 0053ed30
// $deBlob: FUNCTION 00858ff0
TDEFINE_CLASS( TNullResource );

// The scalar deleting destructor has the empty destructor inlined
// $deBlob: FUNCTION 006b57c0
// $Barnyard: FUNCTION 006d8390
TNullResource::~TNullResource()
{
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0065d0f0
// $deBlob: FUNCTION 0065d100
// $deBlob: FUNCTION 0065d120
// $deBlob: FUNCTION 0065d140
// $deBlob: FUNCTION 0065d150
TDEFINE_CLASS_COMPILETIME( TObject, 0, 2 );

// $deBlob: FUNCTION 0065d170
TObject::TObject()
{
}

// $deBlob: FUNCTION 0065d160
// $Barnyard: FUNCTION 006bbee0
void TObject::Delete()
{
	delete this;
}

// $deBlob: FUNCTION 0065d210
// $deBlob: FUNCTION 0065d180
// $Barnyard: FUNCTION 006bbed0
// $Barnyard: FUNCTION 00570620
TObject::~TObject()
{
}

TOSHI_NAMESPACE_END

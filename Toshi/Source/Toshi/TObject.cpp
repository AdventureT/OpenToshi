#include "ToshiPCH.h"
#include "TObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: Barnyard: GetClass 006bbcc0, CreateObject 006bbef0, CreateObjectInPlace 006bbf10
// de Blob: GetClass 0065d0f0, CreateTObject 0065d100, CreateTObjectInPlace 0065d120, InitialiseClass 0065d140,
// DeinitialiseClass 0065d150
// $deBlob: FUNCTION 0065d0f0
// $deBlob: FUNCTION 0065d100
// $deBlob: FUNCTION 0065d120
// $deBlob: FUNCTION 0065d140
// $deBlob: FUNCTION 0065d150
TDEFINE_CLASS_COMPILETIME( TObject, 0, 2 );

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

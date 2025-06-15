#include "ToshiPCH.h"
#include "TObject.h"

TOSHI_NAMESPACE_START

constinit TClass TObject::s_Class = TClass("TObject", TMAKEVERSION(1, 0), TObject::CreateTObject, TObject::CreateTObjectInPlace, sizeof(TObject), alignof(TObject));

TOSHI_NAMESPACE_END

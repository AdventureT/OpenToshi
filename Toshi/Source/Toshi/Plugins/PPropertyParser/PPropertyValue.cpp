#include "ToshiPCH.h"
#include "PPropertyValue.h"
#include "PProperties.h"

const Toshi::TClass* PPropertyValue::TYPE_PROPS = TGetClass(PProperties);

TBOOL PPropertyValue::ChangeType(const Toshi::TClass* a_pClass)
{
	return TTRUE;
}

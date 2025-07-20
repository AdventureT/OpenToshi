#include "ToshiPCH.h"
#include "TXUIGroup.h"

TOSHI_NAMESPACE_START

TBOOL XURXUIGroupData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0) return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
	return TFALSE;
}

TBOOL XURXUIGroupData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0) return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	return TFALSE;
}

TUINT32 XURXUIGroupData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0) return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	return 4;
}

TBOOL XURXUIGroupData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	a_uiObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TBOOL XURXUIGroupData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex != 0) return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
	return TFALSE;
}

TBOOL XURXUIGroupData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);
	a_pData++;
	return TTRUE;
}

TOSHI_NAMESPACE_END

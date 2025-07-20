#include "ToshiPCH.h"
#include "TXUIVisual.h"

TOSHI_NAMESPACE_USING

TBOOL XURXUIVisualData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);
	a_pData++;
	return TTRUE;
}

TBOOL XURXUIVisualData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	return a_uiObjectIndex != 0 ? XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex) : TFALSE;
}

TBOOL XURXUIVisualData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	return XURXUIElementData::TranslateTimelineProp(name, ++a_uiObjectIndex, propType);
}

TUINT32 XURXUIVisualData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	return a_uiObjectIndex != 0 ? XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType) : 4;
}

TBOOL XURXUIVisualData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	return a_uiObjectIndex != 0 ? XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType) : TFALSE;
}

TBOOL XURXUIVisualData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	return a_uiObjectIndex != 0 ? XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType) : TFALSE;
}

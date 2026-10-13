#include "ToshiPCH.h"
#include "TXUIVisual.h"

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 006e17d0
TBOOL XURXUIVisualData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);
	a_pData++;
	return TTRUE;
}

// $deBlob: FUNCTION 006e1810
TBOOL XURXUIVisualData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	return a_uiObjectIndex != 0 ? XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex) : TFALSE;
}

// $deBlob: FUNCTION 006e1830
TBOOL XURXUIVisualData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	return XURXUIElementData::TranslateTimelineProp(name, ++a_uiObjectIndex, propType);
}

// $deBlob: FUNCTION 006e1850
TUINT32 XURXUIVisualData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	return a_uiObjectIndex != 0 ? XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType) : 4;
}

// $deBlob: FUNCTION 006e1870
TBOOL XURXUIVisualData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	return a_uiObjectIndex != 0 ? XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType) : TFALSE;
}

// $deBlob: FUNCTION 006e1890
TBOOL XURXUIVisualData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	return a_uiObjectIndex != 0 ? XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType) : TFALSE;
}

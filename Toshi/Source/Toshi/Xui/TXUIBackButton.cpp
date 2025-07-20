#include "ToshiPCH.h"
#include "TXUIButton.h"
#include "TXUIBackButton.h"

TOSHI_NAMESPACE_START

TBOOL XURXUIBackButtonData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0) return XURXUIButtonData::IsColourPropType(a_uiObjectIndex - 1, propType);
	return TFALSE;
}

TBOOL XURXUIBackButtonData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0) return XURXUIButtonData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	return TFALSE;
}

TUINT32 XURXUIBackButtonData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0) return XURXUIButtonData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	return 4;
}

TBOOL XURXUIBackButtonData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	a_uiObjectIndex++;
	return XURXUIButtonData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TBOOL XURXUIBackButtonData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex != 0) return XURXUIButtonData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
	return TFALSE;
}

TBOOL XURXUIBackButtonData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	TBOOL res = XURXUIButtonData::Load(resource, a_pData);
	a_pData++;

	return res;
}

TOSHI_NAMESPACE_END

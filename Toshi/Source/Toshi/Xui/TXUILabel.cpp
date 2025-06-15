#include "ToshiPCH.h"
#include "TXUILabel.h"

TOSHI_NAMESPACE_START

TBOOL XURXUILabelData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0) return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
	return TFALSE;
}

TBOOL XURXUILabelData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0) return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
	return TFALSE;
}

TUINT32 XURXUILabelData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0) return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, propType);
	return 4;
}

TBOOL XURXUILabelData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	a_uiObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TBOOL XURXUILabelData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex != 0) return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
	return TFALSE;
}

TBOOL XURXUILabelData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIControlData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		a_pData += 4;
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TXUICheckBox.h"
#include "XURReader.h"

TOSHI_NAMESPACE_START

TBOOL XURXUICheckBoxData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIControlData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_PressKey, m_uiPressKey);
	}

	return TTRUE;
}

TBOOL XURXUICheckBoxData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUICheckBoxData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, PressKey, propType);

	a_uiObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TUINT32 XURXUICheckBoxData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0)
	{
		return 4;
	}
	else
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
}

TBOOL XURXUICheckBoxData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUICheckBoxData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

TOSHI_NAMESPACE_END

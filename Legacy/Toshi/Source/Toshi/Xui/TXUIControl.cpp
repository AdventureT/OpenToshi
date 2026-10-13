#include "ToshiPCH.h"
#include "TXUIControl.h"
#include "XURReader.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006ca270
TBOOL XURXUIControlData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

// $deBlob: FUNCTION 006ca210
TBOOL XURXUIControlData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

// $deBlob: FUNCTION 006ca160
TUINT32 XURXUIControlData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}

	if (propType != 2 && propType != 3)
	{
		if (propType != 0 && propType != 1 && propType != 4 && propType != 5 && propType != 6 && propType != 7 && propType != 8 && propType != 10)
		{
			return 4;
		}

		return 2;
	}

	return 1;
}

TBOOL XURXUIControlData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, ClassOverride, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Visual, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Enabled, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, UnfocussedInput, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, NavLeft, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, NavRight, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, NavUp, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, NavDown, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Text, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, ImagePath, propType);

	a_uiObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

// $deBlob: FUNCTION 006c9f70
TBOOL XURXUIControlData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

// $deBlob: FUNCTION 006c9f10
TBOOL XURXUIControlData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_STRING>(PropType_ClassOverride, m_ClassOverride);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_Visual, m_Visual);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_Enabled, m_Enabled);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_UnfocussedInput, m_UnfocussedInput);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_NavLeft, m_NavLeft);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_NavRight, m_NavRight);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_NavUp, m_NavUp);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_NavDown, m_NavDown);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_Text, m_Text);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_ImagePath, m_ImagePath);
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END

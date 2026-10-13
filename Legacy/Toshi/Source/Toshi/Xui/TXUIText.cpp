#include "ToshiPCH.h"
#include "TXUIText.h"
#include "XURReader.h"

TBOOL Toshi::XURXUITextData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}
	if (propType == 1) return TTRUE; // propType 1 = TextColor
	return propType == 2;
}

TBOOL Toshi::XURXUITextData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TUINT32 Toshi::XURXUITextData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
	if (propType != 0 && propType != 4) return 4;
	return 2;
}

TBOOL Toshi::XURXUITextData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, Text, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, TextColor, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, DropShadowColor, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, PointSize, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Font, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, TextStyle, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, LineSpacingAdjust, propType);

	a_uiObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TBOOL Toshi::XURXUITextData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL Toshi::XURXUITextData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_STRING>(PropType_Text, m_Text);
		reader.ReadProperty<XUI_EPT_COLOR>(PropType_TextColor, m_TextColor);
		reader.ReadProperty<XUI_EPT_COLOR>(PropType_DropShadowColor, m_DropShadowColor);
		reader.ReadProperty<XUI_EPT_FLOAT>(PropType_PointSize, m_PointSize);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_Font, m_Font);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_TextStyle, m_TextStyle);
		reader.ReadProperty<XUI_EPT_INTEGER>(PropType_LineSpacingAdjust, m_LineSpacingAdjust);
	}

	return TTRUE;
}

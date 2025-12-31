#include "ToshiPCH.h"
#include "TXUITextPresenter.h"
#include "XURReader.h"

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 006dff40
TBOOL XURXUITextPresenterData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_COLOR>(PropType_TextColor, m_TextColor);
		reader.ReadProperty<XUI_EPT_COLOR>(PropType_DropShadowColor, m_DropShadowColor);
		reader.ReadProperty<XUI_EPT_FLOAT>(PropType_PointSize, m_PointSize);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_Font, m_Font);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_TextStyle, m_TextStyle);
		reader.ReadProperty<XUI_EPT_INTEGER>(PropType_LineSpacingAdjust, m_LineSpacingAdjust);
		reader.ReadProperty<XUI_EPT_INTEGER>(PropType_DataAssociation, m_DataAssociation);
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006dfc80
TBOOL XURXUITextPresenterData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

// $deBlob: FUNCTION 006dfcf0
TBOOL XURXUITextPresenterData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, TextColor, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, DropShadowColor, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, PointSize, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Font, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, TextStyle, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, LineSpacingAdjust, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, DataAssociation, propType);

	a_uiObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

// $deBlob: FUNCTION 006dfe00
TUINT32 XURXUITextPresenterData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
	if (propType != 3) return 4;
	return 2;
}

// $deBlob: FUNCTION 006dfe70
TBOOL XURXUITextPresenterData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

// $deBlob: FUNCTION 006dfed0
TBOOL XURXUITextPresenterData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}
	if (propType == 0) return TTRUE; // propType 0 = TextColor
	return propType == 1;
}

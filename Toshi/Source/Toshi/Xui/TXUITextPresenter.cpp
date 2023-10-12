#include "ToshiPCH.h"
#include "TXUITextPresenter.h"
#include "XURReader.h"
#include "Toshi/Xui/TXUIText.h"

TOSHI_NAMESPACE_USING

XURXUITextPresenterData::XURXUITextPresenterData() : XURXUIElementData()
{
	m_TextColor = 0xFF000000;
	m_DropShadowColor = 0x80000000;
	m_PointSize = XURXUITextData::sm_uiDefaultFontSize;
	m_Font = 0;
	m_TextStyle = 0x100;
	m_LineSpacingAdjust = 0;
	m_DataAssociation = 0;
	m_pClass = TClass::Find("TXUITextPresenter", TGetClass(TXUIElement));
}

TBOOL XURXUITextPresenterData::Load(TXUIResource& resource, uint8_t*& a_pData)
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

TBOOL XURXUITextPresenterData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUITextPresenterData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
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

uint32_t XURXUITextPresenterData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
	if (propType != 3) return 4;
	return 2;
}

TBOOL XURXUITextPresenterData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUITextPresenterData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
	}
	if (propType == 0) return TTRUE; // propType 0 = TextColor
	return propType == 1;
}

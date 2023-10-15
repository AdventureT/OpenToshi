#include "ToshiPCH.h"
#include "TXUIListItem.h"
#include "XURReader.h"

TOSHI_NAMESPACE_USING

TBOOL XURXUIListItemData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUICheckBoxData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_Layout, m_uiLayout);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_Checkable, m_bCheckable);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_SelectedSize, m_uiSelectedSize);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_KeepSizeUnfocused, m_bKeepSizeUnfocused);
		reader.ReadProperty<XUI_EPT_VECTOR>(PropType_InterItemSpacing, m_vInterItemSpacing);

	}

	return TTRUE;
}

TBOOL XURXUIListItemData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
{
	if (a_uiObjectIndex == 0)
		return a_uiPropIndex < PropType_NUMOF;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUIListItemData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, Layout, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Checkable, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, SelectedSize, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, KeepSizeUnfocused, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, InterItemSpacing, propType);

	a_uiObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

uint32_t XURXUIListItemData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0)
	{
		return propType != PropType_Checkable && propType != PropType_KeepSizeUnfocused ? 4 : 1;
	}
	else
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
}

TBOOL XURXUIListItemData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0)
		return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUIListItemData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0)
		return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

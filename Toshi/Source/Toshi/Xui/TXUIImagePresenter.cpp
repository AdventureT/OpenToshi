#include "ToshiPCH.h"
#include "TXUIImagePresenter.h"
#include "XURReader.h"

TOSHI_NAMESPACE_USING

TBOOL XURXUIImagePresenterData::Load(TXUIResource& resource, uint8_t*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_SizeMode, m_SizeMode);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_DataAssoc, m_DataAssoc);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_BrushFlags, m_BrushFlags);
	}

	return TTRUE;
}

TBOOL XURXUIImagePresenterData::ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUIImagePresenterData::TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, SizeMode, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, DataAssoc, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, BrushFlags, propType);

	a_uiObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

uint32_t XURXUIImagePresenterData::GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return 4;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUIImagePresenterData::IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUIImagePresenterData::IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

#include "ToshiPCH.h"
#include "TXUICommonList.h"
#include "XURReader.h"

TOSHI_NAMESPACE_BEGIN

TBOOL XURXUICommonListData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIListData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_STRING>(PropType_ItemsText, m_sItemsText);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_ItemsImage, m_sItemsImage);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_ItemsNavPath, m_sItemsNavPath);
	}

	return TTRUE;
}

TBOOL XURXUICommonListData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIListData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUICommonListData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, ItemsText, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, ItemsImage, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, ItemsNavPath, propType);

	a_uiObjectIndex++;
	return XURXUIListData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TUINT32 XURXUICommonListData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIListData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
	if (propType == 3)
	{
		return 2;
	}
	return 4;
}

TBOOL XURXUICommonListData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIListData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUICommonListData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIListData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

TOSHI_NAMESPACE_END

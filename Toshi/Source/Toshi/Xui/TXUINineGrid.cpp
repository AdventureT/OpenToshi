#include "ToshiPCH.h"
#include "TXUINineGrid.h"
#include "XURReader.h"

TOSHI_NAMESPACE_START

TBOOL XURXUINineGridData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_STRING>(PropType_TextureFileName, m_TextureFileName);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_LeftOffset, m_uiLeftOffset);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_TopOffset, m_uiTopOffset);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_RightOffset, m_uiRightOffset);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_BottomOffset, m_uiBottomOffset);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_NoCenter, m_bNoCenter);
	}

	return TFALSE;
}

TBOOL XURXUINineGridData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUINineGridData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, TextureFileName, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, LeftOffset, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, TopOffset, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, RightOffset, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, BottomOffset, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, NoCenter, propType);

	a_uiObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TUINT32 XURXUINineGridData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0)
	{
		if (propType == PropType_TextureFileName) return 2;

		if (propType == PropType_NoCenter) return 1;

		return 4;
	}
	else
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
}

TBOOL XURXUINineGridData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUINineGridData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

TOSHI_NAMESPACE_END

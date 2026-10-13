#include "ToshiPCH.h"
#include "TXUITabScene.h"
#include "XURReader.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006e79c0
TBOOL XURXUITabSceneData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUISceneData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_TabCount, m_uiTabCount);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_Wrap, m_bWrap);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_UserInterrupt, m_bUserInterrupt);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_VerticalTabs, m_bVerticalTabs);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_NoAutoHide, m_bNoAutoHide);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_DefaultTab, m_uiDefaultTab);
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e7a20
TBOOL XURXUITabSceneData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUISceneData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

// $deBlob: FUNCTION 006e7a90
TBOOL XURXUITabSceneData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, TabCount, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Wrap, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, UserInterrupt, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, VerticalTabs, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, NoAutoHide, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, DefaultTab, propType);

	a_uiObjectIndex++;
	return XURXUISceneData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

// $deBlob: FUNCTION 006e7b80
TUINT32 XURXUITabSceneData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0)
	{
		if (propType == PropType_Wrap || propType == PropType_UserInterrupt || propType == PropType_VerticalTabs || propType == PropType_NoAutoHide)
			return 1;
		else
			return 4;
	}
	else
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUISceneData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
}

// $deBlob: FUNCTION 006e7c00
TBOOL XURXUITabSceneData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUISceneData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

// $deBlob: FUNCTION 006e7c60
TBOOL XURXUITabSceneData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUISceneData::IsColourPropType(a_uiObjectIndex - 1, propType);
}


TOSHI_NAMESPACE_END

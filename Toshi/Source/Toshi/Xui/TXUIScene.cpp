#include "ToshiPCH.h"
#include "TXUIScene.h"
#include "XURReader.h"

TOSHI_NAMESPACE_START

TBOOL XURXUISceneData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUISceneData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TUINT32 XURXUISceneData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}

	if (propType == PropType_IgnorePresses)
	{
		return 1;
	}

	if (propType != PropType_DefaultFocus && propType != PropType_TransFrom && propType != PropType_TransTo && propType != PropType_TransBackFrom && propType != PropType_TransBackTo)
	{
		return 4;
	}

	return 2;
}

TBOOL XURXUISceneData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, DefaultFocus, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, TransFrom, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, TransTo, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, TransBackFrom, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, TransBackTo, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, InterruptTransitions, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, IgnorePresses, propType);

	a_uiObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TBOOL XURXUISceneData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUISceneData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIControlData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_STRING>(PropType_DefaultFocus, m_DefaultFocus);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_TransFrom, m_TransFrom);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_TransTo, m_TransTo);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_TransBackFrom, m_TransBackFrom);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_TransBackTo, m_TransBackTo);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_InterruptTransitions, m_InterruptTransitions);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_IgnorePresses, m_IgnorePresses);
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END

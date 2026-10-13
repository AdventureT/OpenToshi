#include "ToshiPCH.h"
#include "TXUINavButton.h"
#include "XURReader.h"

TOSHI_NAMESPACE_BEGIN

TBOOL XURXUINavButtonData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIButtonData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_STRING>(PropType_PressPath, m_sPressPath);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_StayVisible, m_bStayVisible);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_SrcTransIndex, m_uiSrcTransIndex);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_DestTransIndex, m_uiDestTransIndex);
	}

	return TTRUE;
}

TBOOL XURXUINavButtonData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUINavButtonData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, PressPath, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, StayVisible, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, SrcTransIndex, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, DestTransIndex, propType);

	a_uiObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TUINT32 XURXUINavButtonData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
	if (propType == 0)
	{
		return 2;
	}
	return propType == 1 ? 1 : 4;
}

TBOOL XURXUINavButtonData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIButtonData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUINavButtonData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIButtonData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

TOSHI_NAMESPACE_END
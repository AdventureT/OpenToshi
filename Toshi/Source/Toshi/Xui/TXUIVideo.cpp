#include "ToshiPCH.h"
#include "TXUIVideo.h"
#include "XURReader.h"

TOSHI_NAMESPACE_USING

TBOOL XURXUIVideoData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_OBJECT>(PropType_File, m_File);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_SizeMode, m_SizeMode);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_Pause, m_Pause);
		reader.ReadProperty<XUI_EPT_BOOL>(PropType_Loop, m_Loop);
		reader.ReadProperty<XUI_EPT_FLOAT>(PropType_Volume, m_Volume);
	}

	return TTRUE;
}

TBOOL XURXUIVideoData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUIVideoData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, File, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, SizeMode, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Pause, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Loop, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, Volume, propType);

	a_uiObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TUINT32 XURXUIVideoData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex != 0)
	{
		TASSERT(a_uiObjectIndex > 0);
		return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
	}
	if (propType == 2) return 1;
	if (propType != 0) return 4;
	return 2;
}

TBOOL XURXUIVideoData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return propType == 4;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUIVideoData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

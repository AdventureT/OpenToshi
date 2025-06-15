#include "ToshiPCH.h"
#include "TXUIButton.h"
#include "XURReader.h"

TOSHI_NAMESPACE_START

TBOOL TXUIButton::Create(TXUIResource& a_rRes, XURXUIElementData* a_pObjData, TBOOL a_bUnk)
{
	TXUIControl::Create(a_rRes, a_pObjData, a_bUnk);
	return TTRUE;
}


TBOOL XURXUIButtonData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);

	return XURXUIControlData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUIButtonData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TUINT32 XURXUIButtonData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return 4;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUIButtonData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, PressKey, propType);

	a_uiObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TBOOL XURXUIButtonData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex == 0;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIControlData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUIButtonData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIControlData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_PressKey, m_PressKey);
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TXUIImage.h"
#include "XURReader.h"

TOSHI_NAMESPACE_USING

TBOOL XURXUIImageData::IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUIImageData::IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

TUINT32 XURXUIImageData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType)
{
	if (a_uiObjectIndex == 0) return (propType != 1) * 2 + 2;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
}

TBOOL XURXUIImageData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP(name, SizeMode, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, ImagePath, propType);
	TXUI_TRANSLATE_TIMELINE_PROP(name, BrushFlags, propType);

	a_uiObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
}

TBOOL XURXUIImageData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;
	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

TBOOL XURXUIImageData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_SizeMode, m_SizeMode);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_ImagePath, m_ImagePath);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_BrushFlags, m_BrushFlags);
	}

	return TTRUE;
}

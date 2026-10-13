#include "ToshiPCH.h"
#include "TXUIFigure.h"
#include "XURReader.h"
#include <Toshi/Xui/TXUI.h>

TOSHI_NAMESPACE_START

TBOOL XURXUIFillData::IsColourPropType(TUINT32 propType)
{
	return propType == PropType_FillType;
}

TBOOL XURXUIFillData::IsFloatPropType(TUINT32 propType)
{
	return propType == PropType_FillRotation;
}

TUINT32 XURXUIFillData::GetTimelinePropSize(TUINT32 propType)
{
	return propType != PropType_FillTextureFileName ? 4 : 2;
}

TBOOL XURXUIFillData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.FillType", propType, PropType_FillType);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.FillColor", propType, PropType_FillColor);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.TextureFileName", propType, PropType_FillTextureFileName);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Translation", propType, PropType_FillTranslation);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Scale", propType, PropType_FillScale);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Rotation", propType, PropType_FillRotation);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.WrapX", propType, PropType_FillWrapX);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.WrapY", propType, PropType_FillWrapY);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.BrushFlags", propType, PropType_FillBrushFlags);

	return TFALSE;
}

TBOOL XURXUIFillData::ValidateTimelineProp(TUINT32 param_2)
{
	return param_2 < PropType_NUMOF;
}

// $deBlob: FUNCTION 006c5a00
TBOOL XURXUIFillData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIObjectData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_USHORT32>(PropType_FillType, m_FillType);
		reader.ReadProperty<XUI_EPT_COLOR>(PropType_FillColor, m_FillColor);
		reader.ReadProperty<XUI_EPT_STRING>(PropType_FillTextureFileName, m_FillTextureFileName);

		if (reader.ShouldReadThisProp(PropType_Gradient))
		{
			m_Gradient.Load(resource, a_pData);
		}

		reader.ReadProperty<XUI_EPT_VECTOR>(PropType_FillTranslation, m_FillTranslation);
		reader.ReadProperty<XUI_EPT_VECTOR>(PropType_FillScale, m_FillScale);
		reader.ReadProperty<XUI_EPT_FLOAT>(PropType_FillRotation, m_FillRotation);
		reader.ReadProperty<XUI_EPT_USHORT32>(PropType_FillWrapX, m_FillWrapX);
		reader.ReadProperty<XUI_EPT_USHORT32>(PropType_FillWrapY, m_FillWrapY);

		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_FillBrushFlags, m_FillBrushFlags);
		reader.ReadProperty<XUI_EPT_UNSIGNED>(PropType_Unknown, m_Unknown);
	}

	return TTRUE;
}

// StrokeData

TBOOL XURXUIStrokeData::IsColourPropType(TUINT32 propType)
{
	return propType == PropType_StrokeColor;
}

TBOOL XURXUIStrokeData::IsFloatPropType(TUINT32 propType)
{
	return propType == PropType_StrokeWidth;
}

TUINT32 XURXUIStrokeData::GetTimelinePropSize(TUINT32 propType)
{
	return 4;
}

TBOOL XURXUIStrokeData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Stroke.StrokeWidth", propType, PropType_StrokeWidth);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Stroke.StrokeColor", propType, PropType_StrokeColor);

	return TFALSE;
}

TBOOL XURXUIStrokeData::ValidateTimelineProp(TUINT32 param_2)
{
	return param_2 < PropType_NUMOF;
}

TBOOL XURXUIStrokeData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIObjectData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_FLOAT>(PropType_StrokeWidth, m_StrokeWidth);
		reader.ReadProperty<XUI_EPT_COLOR>(PropType_StrokeColor, m_StrokeColor);
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c5920
// GradientData

TBOOL XURXUIGradientData::IsColourPropType(TUINT32 propType)
{
	return propType == 2;
}

// $deBlob: FUNCTION 006c5910
TBOOL XURXUIGradientData::IsFloatPropType(TUINT32 propType)
{
	return propType == 3;
}

// $deBlob: FUNCTION 006c58f0
TUINT32 XURXUIGradientData::GetTimelinePropSize(TUINT32 propType)
{
	return propType == PropType_FillGradientRadial ? 1 : 4;
}

// $deBlob: FUNCTION 006c5850
TBOOL XURXUIGradientData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.Radial", propType, PropType_FillGradientRadial);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.NumStops", propType, PropType_FillGradientNumStops);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.StopPos", propType, PropType_FillGradientStopPos);
	TXUI_TRANSLATE_TIMELINE_PROP_MANUAL(name, "Fill.Gradient.StopColor", propType, PropType_FillGradientStopColor);

	return TFALSE;
}

// $deBlob: FUNCTION 006c5840
TBOOL XURXUIGradientData::ValidateTimelineProp(TUINT32 param_2)
{
	return param_2 < PropType_NUMOF;
}

// $deBlob: FUNCTION 006c57c0
TBOOL XURXUIGradientData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIObjectData::Load(resource, a_pData);
	XUIEPTUInt8 num;

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		reader.ReadProperty<XUI_EPT_BOOL>(PropType_FillGradientRadial, m_Radial);
		reader.ReadProperty<XUI_EPT_USHORT32>(PropType_FillGradientNumStops, m_NumStops);

		if (reader.ShouldReadThisProp(PropType_FillGradientStopPos))
		{
			num       = reader.ReadEPTUInt8();
			m_StopPos = new (TXUI::MemoryBlock()) XUIEPTFloat[num];

			for (XUIEPTUInt8 i = 0; i < num; i++)
			{
				m_StopPos[i] = reader.ReadEPTFloat();
			}
		}

		if (reader.ShouldReadThisProp(PropType_FillGradientStopColor))
		{
			num          = reader.ReadEPTUInt8();
			m_StopColors = new (TXUI::MemoryBlock()) XUIEPTColor[num];

			for (XUIEPTUInt8 i = 0; i < num; i++)
			{
				m_StopColors[i] = reader.ReadEPTColor();
			}
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c6aa0
TBOOL XURXUIFigureData::IsColourPropType(TUINT32 a_uiObjectIndex, PropType propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsColourPropType(a_uiObjectIndex - 1, propType);
}

// $deBlob: FUNCTION 006c6a40
TBOOL XURXUIFigureData::IsFloatPropType(TUINT32 a_uiObjectIndex, PropType propType)
{
	if (a_uiObjectIndex == 0) return TFALSE;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::IsFloatPropType(a_uiObjectIndex - 1, propType);
}

// $deBlob: FUNCTION 006c6990
TUINT32 XURXUIFigureData::GetTimelinePropSize(TUINT32 a_uiObjectIndex, PropType propType)
{
	if (a_uiObjectIndex == 0) return propType == PropType_Closed ? 1 : 4;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::GetTimelinePropSize(a_uiObjectIndex - 1, propType);
}

// $deBlob: FUNCTION 006c68f0
TBOOL XURXUIFigureData::TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType)
{
	if (!m_Stroke.TranslateTimelineProp(name, a_uiObjectIndex, propType))
	{
		if (!m_Fill.TranslateTimelineProp(name, a_uiObjectIndex, propType))
		{
			TXUI_TRANSLATE_TIMELINE_PROP(name, Closed, propType);
			TXUI_TRANSLATE_TIMELINE_PROP(name, Points, propType);

			a_uiObjectIndex++;
			return XURXUIElementData::TranslateTimelineProp(name, a_uiObjectIndex, propType);
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c6830
TBOOL XURXUIFigureData::ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0) return a_uiPropIndex < PropType_NUMOF;

	TASSERT(a_uiObjectIndex > 0);
	return XURXUIElementData::ValidateTimelineProp(a_uiObjectIndex - 1, a_uiPropIndex);
}

// $deBlob: FUNCTION 006c66c0
TBOOL XURXUIFigureData::Load(TXUIResource& resource, TUINT8*& a_pData)
{
	XURXUIElementData::Load(resource, a_pData);

	if (*a_pData++ != 0)
	{
		XURReader reader(a_pData);
		if (m_Index != 0) reader.ReadPropsInfo<PropType_NUMOF>();

		if (reader.ShouldReadThisProp(PropType_Stroke))
		{
			m_Stroke.Load(resource, a_pData);
		}

		if (reader.ShouldReadThisProp(PropType_Fill))
		{
			m_Fill.Load(resource, a_pData);
		}

		XUIEPTCustom points;
		TBOOL        hasClosed = reader.ReadProperty<XUI_EPT_BOOL>(PropType_Closed, m_Closed);
		TBOOL        hasPoints = reader.ReadProperty<XUI_EPT_CUSTOM>(PropType_Points, points);

		if (hasPoints)
		{
			auto pCustData = resource.GetCust(points);
			TASSERT(PARSEDWORD_BIG(pCustData) < (1<<16));
			TTODO("Load points");
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c68a0
TBOOL XURXUIFigureData::ValidateTimelineSubProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0)
		return m_Stroke.ValidateTimelineProp(a_uiPropIndex);
	else if (a_uiObjectIndex == 3)
		return m_Fill.m_Gradient.ValidateTimelineProp(a_uiPropIndex);
	else
		return m_Fill.ValidateTimelineProp(a_uiPropIndex);
}

// $deBlob: FUNCTION 006c6a00
TUINT32 XURXUIFigureData::GetTimelineSubPropSize(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0)
		return m_Stroke.GetTimelinePropSize(a_uiPropIndex);
	else if (a_uiObjectIndex == 3)
		return m_Fill.m_Gradient.GetTimelinePropSize(a_uiPropIndex);
	else
		return m_Fill.GetTimelinePropSize(a_uiPropIndex);
}

// $deBlob: FUNCTION 006c6b00
TBOOL XURXUIFigureData::IsFloatSubPropType(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0)
		return m_Stroke.IsFloatPropType(a_uiPropIndex);
	else if (a_uiObjectIndex == 3)
		return m_Fill.m_Gradient.IsFloatPropType(a_uiPropIndex);
	else
		return m_Fill.IsFloatPropType(a_uiPropIndex);
}

// $deBlob: FUNCTION 006c6b50
TBOOL XURXUIFigureData::IsColourSubPropType(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex)
{
	if (a_uiObjectIndex == 0)
		return m_Stroke.IsColourPropType(a_uiPropIndex);
	else if (a_uiObjectIndex == 3)
		return m_Fill.m_Gradient.IsColourPropType(a_uiPropIndex);
	else
		return m_Fill.IsColourPropType(a_uiPropIndex);
}

TOSHI_NAMESPACE_END

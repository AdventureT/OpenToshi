#pragma once
#include "TXUIResource.h"
#include "TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

static const TUINT32 g_uiDefaultColour = TCOLOR(0xF, 0xF, 0x80); // Ultramarine

class XURXUIGradientData : public XURXUIObjectData
{

	friend class XURXUIFillData;


	enum PropType_ : PropType
	{
		PropType_FillGradientRadial,
		PropType_FillGradientNumStops,
		PropType_FillGradientStopPos,
		PropType_FillGradientStopColor,
		PropType_NUMOF,
	};

public:
	XURXUIGradientData() {}


public:
	virtual TBOOL TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TBOOL Load(TXUIResource& resource, TUINT8*& a_pData) override;

	virtual TBOOL   ValidateTimelineProp(TUINT32 param_2);
	virtual TUINT32 GetTimelinePropSize(TUINT32 propType);
	virtual TBOOL   IsFloatPropType(TUINT32 propType);
	virtual TBOOL   IsColourPropType(TUINT32 propType);

protected:
	/* 0 */ XUIEPTBool     m_Radial;
	/* 1 */ XUIEPTUShort32 m_NumStops;
	/* 2 */ XUIEPTFloat*   m_StopPos;
	/* 3 */ XUIEPTColor*   m_StopColors;
};

class XURXUIFillData : public XURXUIObjectData
{
	friend class XURXUIFigureData;

	enum PropType_ : PropType
	{
		PropType_FillType,
		PropType_FillColor,
		PropType_FillTextureFileName,
		PropType_Gradient,
		PropType_FillTranslation,
		PropType_FillScale,
		PropType_FillRotation,
		PropType_FillWrapX,
		PropType_FillWrapY,
		PropType_FillBrushFlags,
		PropType_Unknown,
		PropType_NUMOF,
	};

public:
	// $deBlob: FUNCTION 006c5930
	XURXUIFillData() {}


public:
	virtual TBOOL TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TBOOL Load(TXUIResource& resource, TUINT8*& a_pData) override;

	virtual TBOOL   ValidateTimelineProp(TUINT32 param_2);
	virtual TUINT32 GetTimelinePropSize(TUINT32 propType);
	virtual TBOOL   IsFloatPropType(TUINT32 propType);
	virtual TBOOL   IsColourPropType(TUINT32 propType);

protected:
	/* 0 */ XUIEPTUnsigned     m_FillType;
	/* 1 */ XUIEPTColor        m_FillColor;
	/* 2 */ XUIEPTString       m_FillTextureFileName;
	/* 3 */ XURXUIGradientData m_Gradient;
	/* 4 */ XUIEPTVector       m_FillTranslation;
	/* 5 */ XUIEPTVector       m_FillScale;
	/* 6 */ XUIEPTFloat        m_FillRotation;
	/* 7 */ XUIEPTUnsigned     m_FillWrapX;
	/* 8 */ XUIEPTUnsigned     m_FillWrapY;
	/* 9 */ XUIEPTUnsigned     m_FillBrushFlags;
	/* 10 */ XUIEPTUnsigned    m_Unknown;
};

class XURXUIStrokeData : public XURXUIObjectData
{

	enum PropType_ : PropType
	{
		PropType_StrokeWidth,
		PropType_StrokeColor,
		PropType_NUMOF,
	};

public:
	XURXUIStrokeData() {}

public:
	virtual TBOOL TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TBOOL Load(TXUIResource& resource, TUINT8*& a_pData) override;

	virtual TBOOL   ValidateTimelineProp(TUINT32 param_2);
	virtual TUINT32 GetTimelinePropSize(TUINT32 propType);
	virtual TBOOL   IsFloatPropType(TUINT32 propType);
	virtual TBOOL   IsColourPropType(TUINT32 propType);

protected:
	/* 0 */ XUIEPTFloat m_StrokeWidth;
	/* 1 */ XUIEPTColor m_StrokeColor;
};


class TXUIFigure : public TGenericClassDerived<TXUIFigure, TXUIElement, "TXUIFigure", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUIFigureData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_FIGURE, XURXUIElementData)

	enum PropType_ : PropType
	{
		PropType_Stroke,
		PropType_Fill,
		PropType_Closed,
		PropType_Points,
		PropType_NUMOF,
	};

	struct XUIFigurePoint
	{
		XUIFigurePoint()
		{
			m_usId = 0;
			TUtil::MemClear(m_pPoints, sizeof(m_pPoints));
		}

		XUIEPTShort32 m_usId;
		XUIEPTShort32 m_pPoints[6];
	};

public:
	// $deBlob: FUNCTION 006c6050
	XURXUIFigureData()
	    : XURXUIElementData() { m_pClass = TFindClass(TXUIFigure, TXUIElement); }


public:
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, PropType propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, PropType propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, PropType propType) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineSubProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);
	virtual TUINT32 GetTimelineSubPropSize(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);
	virtual TBOOL   IsFloatSubPropType(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);
	virtual TBOOL   IsColourSubPropType(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);

private:
	XURXUIStrokeData m_Stroke;
	XURXUIFillData   m_Fill;
	XUIEPTBool       m_Closed;
	XUIFigurePoint*  m_Points;
};

TOSHI_NAMESPACE_END

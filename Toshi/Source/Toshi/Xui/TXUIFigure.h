#pragma once
#include "TXUIResource.h"
#include "TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

static const uint32_t g_uiDefaultColour = TCOLOR(0xF, 0xF, 0x80); // Ultramarine

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

	XURXUIGradientData()
	{

	}


public:
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;

	virtual TBOOL ValidateTimelineProp(uint32_t param_2);
	virtual uint32_t GetTimelinePropSize(uint32_t propType);
	virtual TBOOL IsFloatPropType(uint32_t propType);
	virtual TBOOL IsColourPropType(uint32_t propType);

protected:
	/* 0 */ XUIEPTBool m_Radial;
	/* 1 */ XUIEPTUShort32 m_NumStops;
	/* 2 */ XUIEPTFloat* m_StopPos;
	/* 3 */ XUIEPTColor* m_StopColors;
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

	XURXUIFillData()
	{
		
	}


public:
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;

	virtual TBOOL ValidateTimelineProp(uint32_t param_2);
	virtual uint32_t GetTimelinePropSize(uint32_t propType);
	virtual TBOOL IsFloatPropType(uint32_t propType);
	virtual TBOOL IsColourPropType(uint32_t propType);

protected:



	/* 0 */ XUIEPTUnsigned m_FillType;
	/* 1 */ XUIEPTColor m_FillColor;
	/* 2 */ XUIEPTString m_FillTextureFileName;
	/* 3 */ XURXUIGradientData m_Gradient;
	/* 4 */ XUIEPTVector m_FillTranslation;
	/* 5 */ XUIEPTVector m_FillScale;
	/* 6 */ XUIEPTFloat m_FillRotation;
	/* 7 */ XUIEPTUnsigned m_FillWrapX;
	/* 8 */ XUIEPTUnsigned m_FillWrapY;
	/* 9 */ XUIEPTUnsigned m_FillBrushFlags;
	/* 10 */ XUIEPTUnsigned m_Unknown;
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

	XURXUIStrokeData()
	{

	}

public:
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;

	virtual TBOOL ValidateTimelineProp(uint32_t param_2);
	virtual uint32_t GetTimelinePropSize(uint32_t propType);
	virtual TBOOL IsFloatPropType(uint32_t propType);
	virtual TBOOL IsColourPropType(uint32_t propType);

protected:
	/* 0 */ XUIEPTFloat m_StrokeWidth;
	/* 1 */ XUIEPTColor m_StrokeColor;
};


class TXUIFigure :
	public TGenericClassDerived<TXUIFigure, TXUIElement, "TXUIFigure", TMAKEVERSION(1, 0), TFALSE>
{

};

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

	XURXUIFigureData() : XURXUIElementData()
	{
		m_pClass = TFindClass(TXUIFigure, TXUIElement);
	}


public:
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, PropType propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, PropType propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, PropType propType) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineSubProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex);
	virtual uint32_t GetTimelineSubPropSize(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex);
	virtual TBOOL IsFloatSubPropType(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex);
	virtual TBOOL IsColourSubPropType(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex);

private:
	XURXUIStrokeData m_Stroke;
	XURXUIFillData m_Fill;
	XUIEPTBool m_Closed;
	XUIFigurePoint* m_Points;
};

TOSHI_NAMESPACE_END

#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUISliderBar :
	public TGenericClassDerived<TXUISliderBar, TXUIControl, "TXUISliderBar", TMAKEVERSION(1, 0), TFALSE>
{
};

class XURXUISliderBarData : public XURXUIControlData
{
	TXUI_TYPEINFO(XUI_CLASS_SLIDER, XURXUIControlData)

	enum PropType_ : PropType
	{
		PropType_RangeMin,
		PropType_RangeMax,
		PropType_Value,
		PropType_Step,
		PropType_Vertical,
		PropType_AccelInc,
		PropType_AccelTime,
		PropType_NUMOF,
	};

public:

	XURXUISliderBarData()
	{
		m_iRangeMin = 0;
		m_iRangeMax = 100;
		m_iValue = 0;
		m_iStep = 1;
		m_iAccelInc = 0;
		m_iAccelTime = 0;
		m_bVertical = TFALSE;
		m_pClass = TFindClass(TXUISliderBar, TXUIControl);
	}

public:

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;

private:
	XUIEPTInteger m_iRangeMin;
	XUIEPTInteger m_iRangeMax;
	XUIEPTInteger m_iValue;
	XUIEPTInteger m_iStep;
	XUIEPTInteger m_iAccelInc;
	XUIEPTInteger m_iAccelTime;
	XUIEPTBool m_bVertical;
};

TOSHI_NAMESPACE_END
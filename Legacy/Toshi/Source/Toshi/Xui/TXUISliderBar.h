#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUISliderBar : public TGenericClassDerived<TXUISliderBar, TXUIControl, "TXUISliderBar", TMAKEVERSION(1, 0), TFALSE>
{};

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
	// $deBlob: FUNCTION 006e5500
	XURXUISliderBarData()
	{
		m_iRangeMin  = 0;
		m_iRangeMax  = 100;
		m_iValue     = 0;
		m_iStep      = 1;
		m_iAccelInc  = 0;
		m_iAccelTime = 0;
		m_bVertical  = TFALSE;
		m_pClass     = TFindClass(TXUISliderBar, TXUIControl);
	}

public:
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	XUIEPTInteger m_iRangeMin;
	XUIEPTInteger m_iRangeMax;
	XUIEPTInteger m_iValue;
	XUIEPTInteger m_iStep;
	XUIEPTInteger m_iAccelInc;
	XUIEPTInteger m_iAccelTime;
	XUIEPTBool    m_bVertical;
};

TOSHI_NAMESPACE_END

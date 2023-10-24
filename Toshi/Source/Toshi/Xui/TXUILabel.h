#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUILabel :
	public TGenericClassDerived<TXUILabel, TXUIControl, "TXUILabel", TMAKEVERSION(1, 0), TFALSE>
{
};

class XURXUILabelData : public XURXUIControlData
{
	TXUI_TYPEINFO(XUI_CLASS_LABEL, XURXUIControlData)

public:

	XURXUILabelData() : XURXUIControlData()
	{
		m_pClass = TFindClass(TXUILabel, TXUIControl);
	}

public:

	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType);
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex);

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData);
};

TOSHI_NAMESPACE_END
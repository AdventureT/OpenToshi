#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUIScrollEnd :
	public TGenericClassDerived<TXUIScrollEnd, TXUIControl, "TXUIScrollEnd", TMAKEVERSION(1, 0), TFALSE>
{
};

class XURXUIScrollEndData : public XURXUIControlData
{
	TXUI_TYPEINFO(XUI_CLASS_SCROLLEND, XURXUIControlData)

	enum PropType_ : PropType
	{
		PropType_Direction,
		PropType_NUMOF,
	};

public:

	XURXUIScrollEndData() : XURXUIControlData()
	{
		m_uiDirection = 0;
		m_pClass = TFindClass(TXUIScrollEnd, TXUIControl);
	}

public:

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;

private:
	/* 0 */ XUIEPTUnsigned m_uiDirection;
};

TOSHI_NAMESPACE_END

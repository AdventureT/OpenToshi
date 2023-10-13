#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUIList :
	public TGenericClassDerived<TXUIList, TXUIControl, "TXUIList", TMAKEVERSION(1, 0), TFALSE>
{
};

class XURXUIListData : public XURXUIControlData
{

	TXUI_TYPEINFO(XURXUIListData)

	enum PropType_ : PropType
	{
		PropType_Wrap,
		PropType_NUMOF,
	};

public:

	XURXUIListData() : XURXUIControlData()
	{
		m_pClass = TFindClass(TXUIList, TXUIControl);
	}

public:
	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;

private:
	XUIEPTBool m_bWrap;
};

TOSHI_NAMESPACE_END
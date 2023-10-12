#pragma once

#include "TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIImagePresenter :
	public TGenericClassDerived<TXUIImagePresenter, TXUIElement, "TXUIImagePresenter", TMAKEVERSION(1, 0), TFALSE>
{

};

class XURXUIImagePresenterData : public XURXUIElementData
{
	TXUI_TYPEINFO(XURXUIImagePresenterData)

	enum PropType_ : PropType
	{
		PropType_SizeMode,
		PropType_DataAssoc,
		PropType_BrushFlags,
		PropType_NUMOF,
	};

public:

	XURXUIImagePresenterData() : XURXUIElementData()
	{
		m_pClass = TClass::Find("TXUIImagePresenter", TGetClass(TXUIElement));
	}

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
private:
	/* 0 */ XUIEPTUnsigned m_SizeMode;
	/* 1 */ XUIEPTUnsigned m_DataAssoc;
	/* 2 */ XUIEPTUnsigned m_BrushFlags;
};

TOSHI_NAMESPACE_END

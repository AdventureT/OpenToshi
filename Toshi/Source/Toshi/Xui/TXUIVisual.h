#pragma once

#include "Toshi/Xui/TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIVisual :
	public TGenericClassDerived<TXUIVisual, TXUIElement, "TXUIVisual", TMAKEVERSION(1, 0), TFALSE>
{

};

class XURXUIVisualData : public XURXUIElementData
{
	TXUI_TYPEINFO(XURXUIVisualData)

public:

	XURXUIVisualData() : XURXUIElementData()
	{
		m_pClass = TClass::Find("TXUIVisual", TGetClass(TXUIElement));
	}

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
};

TOSHI_NAMESPACE_END
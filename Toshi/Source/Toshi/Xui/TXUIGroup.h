#pragma once
#include <Toshi/Xui/TXUIResource.h>
#include "TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIGroup :
	public TGenericClassDerived<TXUIGroup, TXUIElement, "TXUIGroup", TMAKEVERSION(1, 0), TFALSE>
{
};

class XURXUIGroupData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_GROUP, XURXUIElementData)

public:

	XURXUIGroupData() : XURXUIElementData()
	{
		m_pClass = TFindClass(TXUIGroup, TXUIElement);
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
#pragma once
#include "Toshi/Xui/TXUIButton.h"

TOSHI_NAMESPACE_BEGIN

class TXUIBackButton :
	public TGenericClassDerived<TXUIBackButton, TXUIButton, "TXUIBackButton", TMAKEVERSION(1, 0), TFALSE>
{

};

class XURXUIBackButtonData : public XURXUIButtonData
{
	TXUI_TYPEINFO(XUI_CLASS_BACKBUTTON, XURXUIButtonData)

public:

	XURXUIBackButtonData() : XURXUIButtonData()
	{
		m_pClass = TFindClass(TXUIBackButton, TXUIButton);
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

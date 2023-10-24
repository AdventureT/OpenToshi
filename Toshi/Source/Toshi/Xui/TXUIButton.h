#pragma once
#include "Toshi/Xui/TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUIButton :
	public TGenericClassDerived<TXUIButton, TXUIControl, "TXUIButton", TMAKEVERSION(1, 0), TFALSE>
{
	TBOOL Create(TXUIResource& a_rRes, XURXUIElementData* a_pObjData, TBOOL a_bUnk);
};

class XURXUIButtonData : public XURXUIControlData
{
	TXUI_TYPEINFO(XUI_CLASS_BUTTON, XURXUIControlData)

	enum PropType_ : PropType
	{
		PropType_PressKey,
		PropType_NUMOF
	};

public:

	XURXUIButtonData() : XURXUIControlData()
	{
		m_pClass = TFindClass(TXUIButton, TXUIControl);
	}


public:
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType);
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex);

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData);

protected:
	/* 0 */ uint32_t m_PressKey;
};

TOSHI_NAMESPACE_END
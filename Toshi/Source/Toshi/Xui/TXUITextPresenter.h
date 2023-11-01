#pragma once

#include "Toshi/Xui/TXUIElement.h"
#include "TXUIText.h"

TOSHI_NAMESPACE_BEGIN

class TXUITextPresenter :
	public TGenericClassDerived<TXUITextPresenter, TXUIElement, "TXUITextPresenter", TMAKEVERSION(1, 0), TFALSE>
{

};

class XURXUITextPresenterData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_TEXT_PRESENTER, XURXUIElementData)

	enum PropType_ : PropType
	{
		PropType_TextColor,
		PropType_DropShadowColor,
		PropType_PointSize,
		PropType_Font,
		PropType_TextStyle,
		PropType_LineSpacingAdjust,
		PropType_DataAssociation,
		PropType_NUMOF,
	};

public:

	XURXUITextPresenterData()
	{
		m_TextColor = 0xFF000000;
		m_DropShadowColor = 0x80000000;
		m_PointSize = XURXUITextData::sm_uiDefaultFontSize;
		m_Font = 0;
		m_TextStyle = 0x100;
		m_LineSpacingAdjust = 0;
		m_DataAssociation = 0;
		m_pClass = TFindClass(TXUITextPresenter, TXUIElement);
	}

public:

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;

private:
	/* 0 */ XUIEPTColor m_TextColor;
	/* 1 */ XUIEPTColor m_DropShadowColor;
	/* 2 */ XUIEPTFloat m_PointSize;
	/* 3 */ XUIEPTString m_Font;
	/* 4 */ XUIEPTUnsigned m_TextStyle;
	/* 5 */ XUIEPTInteger m_LineSpacingAdjust;
	/* 6 */ XUIEPTInteger m_DataAssociation;
};

TOSHI_NAMESPACE_END

#pragma once

#include "Toshi/Xui/TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUITextPresenter
{

};

class XURXUITextPresenterData : public XURXUIElementData
{

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
	static constexpr const char* sm_sTypeInfo = "XURXUITextPresenterData";

public:

	XURXUITextPresenterData();

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual const char* GetTypeInfo() const { return sm_sTypeInfo; }

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

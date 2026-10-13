#pragma once

#include "Toshi/Xui/TXUIElement.h"
#include "TXUIText.h"

TOSHI_NAMESPACE_BEGIN

class TXUITextPresenter : public TGenericClassDerived<TXUITextPresenter, TXUIElement, "TXUITextPresenter", TMAKEVERSION(1, 0), TFALSE>
{};

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
	// $deBlob: FUNCTION 006dfbc0
	XURXUITextPresenterData()
	{
		m_TextColor         = 0xFF000000;
		m_DropShadowColor   = 0x80000000;
		m_PointSize         = XURXUITextData::sm_uiDefaultFontSize;
		m_Font              = 0;
		m_TextStyle         = 0x100;
		m_LineSpacingAdjust = 0;
		m_DataAssociation   = 0;
		m_pClass            = TFindClass(TXUITextPresenter, TXUIElement);
	}

public:
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	/* 0 */ XUIEPTColor    m_TextColor;
	/* 1 */ XUIEPTColor    m_DropShadowColor;
	/* 2 */ XUIEPTFloat    m_PointSize;
	/* 3 */ XUIEPTString   m_Font;
	/* 4 */ XUIEPTUnsigned m_TextStyle;
	/* 5 */ XUIEPTInteger  m_LineSpacingAdjust;
	/* 6 */ XUIEPTInteger  m_DataAssociation;
};

TOSHI_NAMESPACE_END

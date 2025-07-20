#pragma once

#include "Toshi/Xui/TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIText : public TGenericClassDerived<TXUIText, TXUIElement, "TXUIText", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUITextData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_TEXT, XURXUIElementData)

	enum PropType_ : PropType
	{
		PropType_Text,
		PropType_TextColor,
		PropType_DropShadowColor,
		PropType_PointSize,
		PropType_Font,
		PropType_TextStyle,
		PropType_LineSpacingAdjust,
		PropType_NUMOF,
	};

public:
	static constexpr TUINT32 sm_uiDefaultFontSize = 14;

	XURXUITextData()
	{
		m_Text              = 0;
		m_TextColor         = 0xFF000000;
		m_DropShadowColor   = 0x80000000;
		m_PointSize         = sm_uiDefaultFontSize;
		m_Font              = 0;
		m_TextStyle         = 0x110;
		m_LineSpacingAdjust = 0;
		m_pClass            = TFindClass(TXUIText, TXUIElement);
	}

public:
	virtual TBOOL IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType);
	virtual TBOOL ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);

	virtual TBOOL Load(TXUIResource& resource, TUINT8*& a_pData);

private:
	/* 0 */ XUIEPTString   m_Text;
	/* 1 */ XUIEPTColor    m_TextColor;
	/* 2 */ XUIEPTColor    m_DropShadowColor;
	/* 3 */ XUIEPTFloat    m_PointSize;
	/* 4 */ XUIEPTString   m_Font;
	/* 5 */ XUIEPTUnsigned m_TextStyle;
	/* 6 */ XUIEPTInteger  m_LineSpacingAdjust;
};

TOSHI_NAMESPACE_END

#pragma once
#include "Toshi/Xui/TXUIButton.h"

TOSHI_NAMESPACE_BEGIN

class TXUINavButton : public TGenericClassDerived<TXUINavButton, TXUIButton, "TXUIBackButton", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUINavButtonData : public XURXUIButtonData
{
	TXUI_TYPEINFO(XUI_CLASS_NAVBUTTON, XURXUINavButtonData)

	enum PropType_ : PropType
	{
		PropType_PressPath,
		PropType_StayVisible,
		PropType_SrcTransIndex,
		PropType_DestTransIndex,
		PropType_NUMOF
	};

public:
	XURXUINavButtonData()
	{
		m_sPressPath = 0;
		m_bStayVisible = TFALSE;
		m_uiSrcTransIndex = 0;
		m_uiDestTransIndex = 0;
		m_pClass = TFindClass(TXUINavButton, TXUIButton);
	}

public:
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	/* 0 */ XUIEPTString   m_sPressPath;
	/* 1 */ XUIEPTBool     m_bStayVisible;
	/* 2 */ XUIEPTUnsigned m_uiSrcTransIndex;
	/* 3 */ XUIEPTUnsigned m_uiDestTransIndex;
};

TOSHI_NAMESPACE_END
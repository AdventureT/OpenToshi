#pragma once
#include "Toshi/Xui/TXUIButton.h"

TOSHI_NAMESPACE_BEGIN

class TXUIBackButton : public TGenericClassDerived<TXUIBackButton, TXUIButton, "TXUIBackButton", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUIBackButtonData : public XURXUIButtonData
{
	TXUI_TYPEINFO(XUI_CLASS_BACKBUTTON, XURXUIButtonData)

public:
	XURXUIBackButtonData() { m_pClass = TFindClass(TXUIBackButton, TXUIButton); }

public:
	virtual TBOOL IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType);
	virtual TBOOL ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);

	virtual TBOOL Load(TXUIResource& resource, TUINT8*& a_pData);
};

TOSHI_NAMESPACE_END

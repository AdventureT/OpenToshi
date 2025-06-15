#pragma once
#include "Toshi/Xui/TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUIButton : public TGenericClassDerived<TXUIButton, TXUIControl, "TXUIButton", TMAKEVERSION(1, 0), TFALSE>
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
	XURXUIButtonData() { m_pClass = TFindClass(TXUIButton, TXUIControl); }


public:
	virtual TBOOL IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType);
	virtual TBOOL ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);

	virtual TBOOL Load(TXUIResource& resource, TUINT8*& a_pData);

protected:
	/* 0 */ TUINT32 m_PressKey;
};

TOSHI_NAMESPACE_END

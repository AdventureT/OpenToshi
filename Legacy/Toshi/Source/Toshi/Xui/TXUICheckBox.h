#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUICheckBox : public TGenericClassDerived<TXUICheckBox, TXUIControl, "TXUICheckBox", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUICheckBoxData : public XURXUIControlData
{
	TXUI_TYPEINFO(XUI_CLASS_CHECKBOX, XURXUIControlData)

	enum PropType_ : PropType
	{
		PropType_PressKey,
		PropType_NUMOF,
	};

public:
	XURXUICheckBoxData() { m_pClass = TFindClass(TXUICheckBox, TXUIControl); }

public:
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	XUIEPTUnsigned m_uiPressKey;
};

TOSHI_NAMESPACE_END

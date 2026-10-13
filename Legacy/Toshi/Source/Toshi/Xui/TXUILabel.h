#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUILabel : public TGenericClassDerived<TXUILabel, TXUIControl, "TXUILabel", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUILabelData : public XURXUIControlData
{
	TXUI_TYPEINFO(XUI_CLASS_LABEL, XURXUIControlData)

public:
	XURXUILabelData() { m_pClass = TFindClass(TXUILabel, TXUIControl); }

public:
	virtual TBOOL IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType);
	virtual TBOOL ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);

	virtual TBOOL Load(TXUIResource& resource, TUINT8*& a_pData);
};

TOSHI_NAMESPACE_END

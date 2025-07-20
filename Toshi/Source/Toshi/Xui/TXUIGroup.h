#pragma once
#include <Toshi/Xui/TXUIResource.h>
#include "TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIGroup : public TGenericClassDerived<TXUIGroup, TXUIElement, "TXUIGroup", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUIGroupData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_GROUP, XURXUIElementData)

public:
	XURXUIGroupData() { m_pClass = TFindClass(TXUIGroup, TXUIElement); }

public:
	virtual TBOOL IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType);
	virtual TBOOL ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);

	virtual TBOOL Load(TXUIResource& resource, TUINT8*& a_pData);
};

TOSHI_NAMESPACE_END

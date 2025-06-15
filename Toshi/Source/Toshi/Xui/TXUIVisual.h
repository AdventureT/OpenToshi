#pragma once

#include "Toshi/Xui/TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIVisual : public TGenericClassDerived<TXUIVisual, TXUIElement, "TXUIVisual", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUIVisualData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_VISUAL, XURXUIElementData)

public:
	XURXUIVisualData() { m_pClass = TFindClass(TXUIVisual, TXUIElement); }

	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
};

TOSHI_NAMESPACE_END

#pragma once
#include "Toshi/Xui/TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUICanvas : public TGenericClassDerived<TXUICanvas, TXUIElement, "TXUICanvas", TMAKEVERSION(1, 0), TFALSE>
{
public:
	virtual void SetDimensions(TFLOAT a_X, TFLOAT a_Y) override;
};

class XURXUICanvasData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_CANVAS, XURXUIElementData)

public:
	XURXUICanvasData() { m_pClass = TFindClass(TXUICanvas, TXUIElement); }

public:
	virtual ~XURXUICanvasData() = default;

	virtual TBOOL Load(TXUIResource& resource, TUINT8*& a_pData);
};

TOSHI_NAMESPACE_END

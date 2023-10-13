#pragma once
#include "Toshi/Xui/TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUICanvas :
	public TGenericClassDerived<TXUICanvas, TXUIElement, "TXUICanvas", TMAKEVERSION(1, 0), TFALSE>
{
public:
	virtual void SetDimensions(float a_X, float a_Y) override;
};

class XURXUICanvasData : public XURXUIElementData
{

	TXUI_TYPEINFO(XURXUICanvasData)

public:

	XURXUICanvasData()
	{
		m_pClass = TFindClass(TXUICanvas, TXUIElement);
	}

public:
	virtual ~XURXUICanvasData() = default;

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData);
};

TOSHI_NAMESPACE_END

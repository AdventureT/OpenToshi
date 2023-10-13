#pragma once
#include "TXUIElement.h"
#include "Toshi2/T2GUI/T2GUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIImage :
	public TGenericClassDerived<TXUIImage, TXUIElement, "TXUIImage", TMAKEVERSION(1, 0), TFALSE>
{
};

class XURXUIImageData : public XURXUIElementData
{
	TXUI_TYPEINFO(XURXUIImageData)

	enum PropType_ : PropType
	{
		PropType_SizeMode,
		PropType_ImagePath,
		PropType_BrushFlags,
		PropType_NUMOF,
	};

public:

	XURXUIImageData() : XURXUIElementData()
	{
		m_pClass = TFindClass(TXUIImage, TXUIElement);
	}

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;

private:
	/* 0 */ XUIEPTUnsigned m_SizeMode;
	/* 1 */ XUIEPTString m_ImagePath;
	/* 2 */ XUIEPTUnsigned m_BrushFlags;
};

TOSHI_NAMESPACE_END
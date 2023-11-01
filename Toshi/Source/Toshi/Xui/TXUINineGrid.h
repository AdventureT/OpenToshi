#pragma once
#include "Toshi/Xui/TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUINineGrid :
	public TGenericClassDerived<TXUINineGrid, TXUIElement, "TXUINineGrid", TMAKEVERSION(1, 0), TFALSE>
{

};

class XURXUINineGridData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_NINE_GRID, XURXUIElementData)

	enum PropType_ : PropType
	{
		PropType_TextureFileName,
		PropType_LeftOffset,
		PropType_TopOffset,
		PropType_RightOffset,
		PropType_BottomOffset,
		PropType_NoCenter,
		PropType_NUMOF
	};

public:

	XURXUINineGridData()
	{
		m_pClass = TFindClass(TXUINineGrid, TXUIElement);
	}

public:
	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
protected:
	XUIEPTString m_TextureFileName;
	XUIEPTUnsigned m_uiLeftOffset;
	XUIEPTUnsigned m_uiTopOffset;
	XUIEPTUnsigned m_uiRightOffset;
	XUIEPTUnsigned m_uiBottomOffset;
	XUIEPTBool m_bNoCenter;
};

TOSHI_NAMESPACE_END
#pragma once
#include "TXUICheckBox.h"

TOSHI_NAMESPACE_BEGIN

class TXUIListItem :
	public TGenericClassDerived<TXUIListItem, TXUICheckBox, "TXUIListItem", TMAKEVERSION(1, 0), TFALSE>
{
};

class XURXUIListItemData : public XURXUICheckBoxData
{
	TXUI_TYPEINFO(XUI_CLASS_LISTITEM, XURXUICheckBoxData)

	enum PropType_ : PropType
	{
		PropType_Layout,
		PropType_Checkable,
		PropType_SelectedSize,
		PropType_KeepSizeUnfocused,
		PropType_InterItemSpacing,
		PropType_NUMOF,
	};

public:

	XURXUIListItemData()
	{
		m_pClass = TFindClass(TXUIListItem, TXUIControl);
	}

public:
	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;

private:
	/* 0 */ XUIEPTUnsigned m_uiLayout;
	/* 1 */ XUIEPTBool m_bCheckable;
	/* 2 */ XUIEPTUnsigned m_uiSelectedSize;
	/* 3 */ XUIEPTBool m_bKeepSizeUnfocused;
	/* 4 */ XUIEPTVector m_vInterItemSpacing;
};

TOSHI_NAMESPACE_END

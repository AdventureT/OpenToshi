#pragma once
#include "TXUIList.h"

TOSHI_NAMESPACE_BEGIN

class TXUICommonList : public TGenericClassDerived<TXUICommonList, TXUIList, "TXUICommonList", TMAKEVERSION(1, 0), TFALSE>
{
};

class XURXUICommonListData : public XURXUIListData
{
	TXUI_TYPEINFO(XUI_CLASS_COMMONLIST, XURXUIListData)

	enum PropType_ : PropType
	{
		PropType_ItemsText,
		PropType_ItemsImage,
		PropType_ItemsNavPath,
		PropType_NUMOF
	};

public:
	XURXUICommonListData()
	{
		m_sItemsText = 0;
		m_sItemsImage = 0;
		m_sItemsNavPath = 0;
		m_pClass = TFindClass(TXUICommonList, TXUIList);
	}

public:
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	/* 0 */ XUIEPTString m_sItemsText;
	/* 1 */ XUIEPTString m_sItemsImage;
	/* 2 */ XUIEPTString m_sItemsNavPath;
};

TOSHI_NAMESPACE_END
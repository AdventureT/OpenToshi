#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUIList : public TGenericClassDerived<TXUIList, TXUIControl, "TXUIList", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUIListData : public XURXUIControlData
{
	TXUI_TYPEINFO(XUI_CLASS_LIST, XURXUIControlData)

	enum PropType_ : PropType
	{
		PropType_Wrap,
		PropType_NUMOF,
	};

public:
	XURXUIListData() { m_pClass = TFindClass(TXUIList, TXUIControl); }

public:
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	XUIEPTBool m_bWrap;
};

TOSHI_NAMESPACE_END

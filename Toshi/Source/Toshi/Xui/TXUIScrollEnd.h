#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUIScrollEnd : public TGenericClassDerived<TXUIScrollEnd, TXUIControl, "TXUIScrollEnd", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUIScrollEndData : public XURXUIControlData
{
	TXUI_TYPEINFO(XUI_CLASS_SCROLLEND, XURXUIControlData)

	enum PropType_ : PropType
	{
		PropType_Direction,
		PropType_NUMOF,
	};

public:
	XURXUIScrollEndData()
	{
		m_uiDirection = 0;
		m_pClass      = TFindClass(TXUIScrollEnd, TXUIControl);
	}

public:
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	/* 0 */ XUIEPTUnsigned m_uiDirection;
};

TOSHI_NAMESPACE_END

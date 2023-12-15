#pragma once
#include "Toshi/Xui/TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUIScene :
	public TGenericClassDerived<TXUIScene, TXUIControl, "TXUIScene", TMAKEVERSION(1, 0), TFALSE>
{
public:
	TXUIScene()
	{

	}
};

class XURXUISceneData : public XURXUIControlData
{
	TXUI_TYPEINFO(XUI_CLASS_SCENE, XURXUIControlData)

	enum PropType_ : PropType
	{
		PropType_DefaultFocus,
		PropType_TransFrom,
		PropType_TransTo,
		PropType_TransBackFrom,
		PropType_TransBackTo,
		PropType_InterruptTransitions,
		PropType_IgnorePresses,
		PropType_NUMOF,
	};

public:

	XURXUISceneData()
	{
		m_DefaultFocus = 0;
		m_TransFrom = 0;
		m_TransTo = 0;
		m_TransBackFrom = 0;
		m_TransBackTo = 0;
		m_InterruptTransitions = 0;
		m_IgnorePresses = TFALSE;
		m_pClass = TFindClass(TXUIScene, TXUIControl);
	}

public:

	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType);

	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType);

	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType);

	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType);
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex);

	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData);

private:
	/* 0 */ XUIEPTString m_DefaultFocus;
	/* 1 */ XUIEPTString m_TransFrom;
	/* 2 */ XUIEPTString m_TransTo;
	/* 3 */ XUIEPTString m_TransBackFrom;
	/* 4 */ XUIEPTString m_TransBackTo;
	/* 5 */ XUIEPTUnsigned m_InterruptTransitions;
	/* 6 */ XUIEPTBool m_IgnorePresses;
};

TOSHI_NAMESPACE_END

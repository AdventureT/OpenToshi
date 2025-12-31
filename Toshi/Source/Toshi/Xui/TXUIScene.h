#pragma once
#include "Toshi/Xui/TXUIControl.h"

TOSHI_NAMESPACE_BEGIN

class TXUIScene : public TGenericClassDerived<TXUIScene, TXUIControl, "TXUIScene", TMAKEVERSION(1, 0), TFALSE>
{
public:
	// $deBlob: FUNCTION 006cbd00
	TXUIScene() {}
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
	// $deBlob: FUNCTION 006cb800
	XURXUISceneData()
	{
		m_DefaultFocus         = 0;
		m_TransFrom            = 0;
		m_TransTo              = 0;
		m_TransBackFrom        = 0;
		m_TransBackTo          = 0;
		m_InterruptTransitions = 0;
		m_IgnorePresses        = TFALSE;
		m_pClass               = TFindClass(TXUIScene, TXUIControl);
	}

public:
	virtual TBOOL IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType);

	virtual TBOOL TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType);
	virtual TBOOL ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex);

	virtual TBOOL Load(TXUIResource& resource, TUINT8*& a_pData);

private:
	/* 0 */ XUIEPTString   m_DefaultFocus;
	/* 1 */ XUIEPTString   m_TransFrom;
	/* 2 */ XUIEPTString   m_TransTo;
	/* 3 */ XUIEPTString   m_TransBackFrom;
	/* 4 */ XUIEPTString   m_TransBackTo;
	/* 5 */ XUIEPTUnsigned m_InterruptTransitions;
	/* 6 */ XUIEPTBool     m_IgnorePresses;
};

TOSHI_NAMESPACE_END

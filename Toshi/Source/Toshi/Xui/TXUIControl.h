#pragma once
#include "Toshi/Xui/TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIControl : public TGenericClassDerived<TXUIControl, TXUIElement, "TXUIControl", TMAKEVERSION(1, 0), TFALSE>
{
};

class XURXUIControlData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_CONTROL, XURXUIElementData)

	enum PropType_ : PropType
	{
		PropType_ClassOverride,
		PropType_Visual,
		PropType_Enabled,
		PropType_UnfocussedInput,
		PropType_NavLeft,
		PropType_NavRight,
		PropType_NavUp,
		PropType_NavDown,
		PropType_Text,
		PropType_Unknown,
		PropType_ImagePath,
		PropType_NUMOF
	};

public:
	XURXUIControlData()
	{
		m_pClass = TFindClass(TXUIControl, TXUIElement);
	}


public:
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

protected:
	/* 0 */ XUIEPTString  m_ClassOverride;
	/* 1 */ XUIEPTString  m_Visual;
	/* 4 */ XUIEPTString  m_NavLeft;
	/* 5 */ XUIEPTString  m_NavRight;
	/* 6 */ XUIEPTString  m_NavUp;
	/* 7 */ XUIEPTString  m_NavDown;
	/* 8 */ XUIEPTString  m_Text;
	/* 10 */ XUIEPTString m_ImagePath;
	/* 2 */ XUIEPTBool    m_Enabled;
	/* 3 */ XUIEPTBool    m_UnfocussedInput;
};

TOSHI_NAMESPACE_END

#pragma once
#include "TXUIScene.h"

TOSHI_NAMESPACE_BEGIN

class TXUITabScene :
	public TGenericClassDerived<TXUITabScene, TXUIScene, "TXUITabScene", TMAKEVERSION(1, 0), TFALSE>
{
};

class XURXUITabSceneData : public XURXUISceneData
{
	TXUI_TYPEINFO(XUI_CLASS_TABSCENE, XURXUISceneData)

	enum PropType_ : PropType
	{
		PropType_TabCount,
		PropType_Wrap,
		PropType_UserInterrupt,
		PropType_VerticalTabs,
		PropType_NoAutoHide,
		PropType_DefaultTab,
		PropType_NUMOF,
	};

public:

	XURXUITabSceneData() : XURXUISceneData()
	{
		m_pClass = TFindClass(TXUITabScene, TXUIScene);
	}

public:
	virtual TBOOL Load(TXUIResource& resource, uint8_t*& a_pData) override;
	virtual TBOOL ValidateTimelineProp(uint32_t a_uiObjectIndex, uint32_t a_uiPropIndex) override;
	virtual TBOOL TranslateTimelineProp(const char* name, uint32_t& a_uiObjectIndex, PropType& propType) override;
	virtual uint32_t GetTimelinePropSize(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsFloatPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;
	virtual TBOOL IsColourPropType(uint32_t a_uiObjectIndex, uint32_t propType) override;

private:
	XUIEPTUnsigned m_uiTabCount;
	XUIEPTBool m_bWrap;
	XUIEPTBool m_bUserInterrupt;
	XUIEPTBool m_bVerticalTabs;
	XUIEPTBool m_bNoAutoHide;
	XUIEPTUnsigned m_uiDefaultTab;
};

TOSHI_NAMESPACE_END

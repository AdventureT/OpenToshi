#pragma once
#include "TXUIScene.h"

TOSHI_NAMESPACE_BEGIN

class TXUITabScene : public TGenericClassDerived<TXUITabScene, TXUIScene, "TXUITabScene", TMAKEVERSION(1, 0), TFALSE>
{};

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
	XURXUITabSceneData() { m_pClass = TFindClass(TXUITabScene, TXUIScene); }

public:
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	XUIEPTUnsigned m_uiTabCount;
	XUIEPTBool     m_bWrap;
	XUIEPTBool     m_bUserInterrupt;
	XUIEPTBool     m_bVerticalTabs;
	XUIEPTBool     m_bNoAutoHide;
	XUIEPTUnsigned m_uiDefaultTab;
};

TOSHI_NAMESPACE_END

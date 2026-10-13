#pragma once

#include "TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIImagePresenter : public TGenericClassDerived<TXUIImagePresenter, TXUIElement, "TXUIImagePresenter", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUIImagePresenterData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_IMAGE_PRESENTER, XURXUIElementData)

	enum PropType_ : PropType
	{
		PropType_SizeMode,
		PropType_DataAssoc,
		PropType_BrushFlags,
		PropType_NUMOF,
	};

public:
	XURXUIImagePresenterData() { m_pClass = TFindClass(TXUIImagePresenter, TXUIElement); }

	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	/* 0 */ XUIEPTUnsigned m_SizeMode;
	/* 1 */ XUIEPTUnsigned m_DataAssoc;
	/* 2 */ XUIEPTUnsigned m_BrushFlags;
};

TOSHI_NAMESPACE_END

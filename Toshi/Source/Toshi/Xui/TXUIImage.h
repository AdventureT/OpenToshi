#pragma once
#include "TXUIElement.h"
#include "Toshi2/T2GUI/T2GUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIImage : public TGenericClassDerived<TXUIImage, TXUIElement, "TXUIImage", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUIImageData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_IMAGE, XURXUIElementData)

	enum PropType_ : PropType
	{
		PropType_SizeMode,
		PropType_ImagePath,
		PropType_BrushFlags,
		PropType_NUMOF,
	};

public:
	// $deBlob: FUNCTION 006dd890
	XURXUIImageData() { m_pClass = TFindClass(TXUIImage, TXUIElement); }

	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	/* 0 */ XUIEPTUnsigned m_SizeMode;
	/* 1 */ XUIEPTString   m_ImagePath;
	/* 2 */ XUIEPTUnsigned m_BrushFlags;
};

TOSHI_NAMESPACE_END

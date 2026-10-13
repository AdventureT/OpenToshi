#pragma once
#include "Toshi/Xui/TXUIElement.h"

TOSHI_NAMESPACE_BEGIN

class TXUIVideo : public TGenericClassDerived<TXUIVideo, TXUIElement, "TXUIVideo", TMAKEVERSION(1, 0), TFALSE>
{};

class XURXUIVideoData : public XURXUIElementData
{
	TXUI_TYPEINFO(XUI_CLASS_VIDEO, XURXUIElementData)

	enum PropType_ : PropType
	{
		PropType_File,
		PropType_SizeMode,
		PropType_Pause,
		PropType_Loop,
		PropType_Volume,
		PropType_NUMOF
	};

public:
	XURXUIVideoData()
	{
		m_File     = 0;
		m_SizeMode = 0;
		m_Pause    = TFALSE;
		m_Loop     = TFALSE;
		m_Volume   = -12.0f;
		m_pClass   = TFindClass(TXUIVideo, TXUIElement);
	}

public:
	virtual TBOOL   Load(TXUIResource& resource, TUINT8*& a_pData) override;
	virtual TBOOL   ValidateTimelineProp(TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex) override;
	virtual TBOOL   TranslateTimelineProp(const TCHAR* name, TUINT32& a_uiObjectIndex, PropType& propType) override;
	virtual TUINT32 GetTimelinePropSize(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsFloatPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;
	virtual TBOOL   IsColourPropType(TUINT32 a_uiObjectIndex, TUINT32 propType) override;

private:
	/* 0 */ XUIEPTObject   m_File;
	/* 1 */ XUIEPTUnsigned m_SizeMode;
	/* 2 */ XUIEPTBool     m_Pause;
	/* 3 */ XUIEPTBool     m_Loop;
	/* 4 */ XUIEPTFloat	   m_Volume;
};

TOSHI_NAMESPACE_END

#pragma once
#include "TXUIButton.h"

TOSHI_NAMESPACE_START

class XURXUIMsgBoxButtonData : public XURXUIButtonData
{
public:
	enum Prop
	{
		Prop_MessageBoxCenter,
		Prop_MessageBoxVisual,
		Prop_MessageBoxTitle,
		Prop_MessageBoxText,
		Prop_MessageBoxButtons,
		Prop_NUMOF,
	};

public:
	XURXUIMsgBoxButtonData();

	// $deBlob: FUNCTION 006e3910
	// $deBlob: FUNCTION 006e3960
	~XURXUIMsgBoxButtonData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override;
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;
	virtual const TXUITypeInfo* GetTypeInfo() const override;

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;

public:
	TUINT16 m_uiMessageBoxVisual;  // 0x48 string index
	TUINT16 m_uiMessageBoxTitle;   // 0x4A string index
	TUINT16 m_uiMessageBoxText;    // 0x4C string index
	TUINT16 m_uiMessageBoxButtons; // 0x4E string index
	TBOOL   m_bMessageBoxCenter;   // 0x50
};

//-----------------------------------------------------------------------------
// TXUIMsgBoxButton (0x98): a button of a message box; the message box properties are only read (not used)
//-----------------------------------------------------------------------------
class TXUIMsgBoxButton : public TXUIButton
{
public:
	TDECLARE_CLASS( TXUIMsgBoxButton, TXUIButton );

public:
	TXUIMsgBoxButton();
	~TXUIMsgBoxButton();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80
};

TOSHI_NAMESPACE_END

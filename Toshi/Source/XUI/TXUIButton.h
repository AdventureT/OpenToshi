#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_START

class XURXUIButtonData : public XURXUIControlData
{
public:
	enum Prop
	{
		Prop_PressKey,
		Prop_NUMOF,
	};

public:
	XURXUIButtonData();

	// $deBlob: FUNCTION 006cffa0
	// $deBlob: FUNCTION 006cfff0
	~XURXUIButtonData() = default;

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
	TUINT32 m_uiPressKey; // 0x44
};

//-----------------------------------------------------------------------------
// TXUIButton (0x98): a control that can be pressed (button 0 of the default controller)
//-----------------------------------------------------------------------------
class TXUIButton : public TXUIControl
{
public:
	TDECLARE_CLASS( TXUIButton, TXUIControl );

public:
	TXUIButton();
	~TXUIButton();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL Press() override;                                            // +0xC0
	virtual TBOOL EndPress() override;                                         // +0xC4
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

protected:
	TUINT32 m_uiPressKey; // 0x94
};

TOSHI_NAMESPACE_END

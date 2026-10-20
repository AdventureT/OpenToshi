#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_START

class XURXUIRadioButtonData : public XURXUIControlData
{
public:
	enum Prop
	{
		Prop_PressKey,
		Prop_NUMOF,
	};

public:
	XURXUIRadioButtonData();

	// $deBlob: FUNCTION 006d13a0
	// $deBlob: FUNCTION 006d1350
	~XURXUIRadioButtonData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                         // +0x04
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                 // +0x08
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                      // +0x0C
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                       // +0x14
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                           // +0x18
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                          // +0x1C
	virtual const TXUITypeInfo* GetTypeInfo() const override;                                                                         // +0x20

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;

public:
	TUINT32 m_uiPressKey; // 0x44
};

//-----------------------------------------------------------------------------
// TXUIRadioButton (0x9C): a checkable button of a TXUIRadioGroup
//-----------------------------------------------------------------------------
class TXUIRadioButton : public TXUIControl
{
public:
	TDECLARE_CLASS( TXUIRadioButton, TXUIControl );

public:
	TXUIRadioButton();
	~TXUIRadioButton();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80
	virtual TBOOL IsChecked() override;                                                                                 // +0xA8

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL Press() override; // +0xC0

	//-----------------------------------------------------------------------------
	// TXUIRadioButton
	//-----------------------------------------------------------------------------
	virtual void Toggle( TBOOL a_bChecked ); // +0x110

	// $deBlob: FUNCTION 006e54e0
	TBOOL GetChecked() const { return m_bChecked; }

private:
	TBOOL   m_bChecked;   // 0x94
	TUINT32 m_uiPressKey; // 0x98
};

TOSHI_NAMESPACE_END

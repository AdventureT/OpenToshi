#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_START

class TXUIRadioButton;

// NOTE: XuiRadioGroup has no properties
class XURXUIRadioGroupData : public XURXUIControlData
{
public:
	XURXUIRadioGroupData();

	// $deBlob: FUNCTION 006e54b0
	// $deBlob: FUNCTION 006e5460
	~XURXUIRadioGroupData() = default;

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
};

//-----------------------------------------------------------------------------
// TXUIRadioGroup (0xA0): a group of TXUIRadioButtons (its children), one of them is checked
//-----------------------------------------------------------------------------
class TXUIRadioGroup : public TXUIControl
{
public:
	TDECLARE_CLASS( TXUIRadioGroup, TXUIControl );

public:
	TXUIRadioGroup();
	~TXUIRadioGroup();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL        Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;     // +0x5C
	virtual TXUIElement* RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController ) override;    // +0x64
	virtual void         UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void         UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override;                         // +0xF8
	virtual void  SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation ) override; // +0x108

	TXUIRadioButton* GetChecked();
	void             SetChecked( TXUIRadioButton* a_pButton );

private:
	TUINT32           m_uiNumButtons; // 0x94
	TXUIRadioButton*  m_pFocussed;    // 0x98
	TXUIRadioButton** m_apButtons;    // 0x9C
};

TOSHI_NAMESPACE_END

#pragma once
#include "TXUIButton.h"

TOSHI_NAMESPACE_START

// XuiBackButton has no properties of its own
class XURXUIBackButtonData : public XURXUIButtonData
{
public:
	XURXUIBackButtonData();

	// $deBlob: FUNCTION 006d0380
	// $deBlob: FUNCTION 006d03d0
	~XURXUIBackButtonData() = default;

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
};

//-----------------------------------------------------------------------------
// TXUIBackButton (0x98): a button that transitions back to the previous scene (button 1, or button 0 when focused)
//-----------------------------------------------------------------------------
class TXUIBackButton : public TXUIButton
{
public:
	TDECLARE_CLASS( TXUIBackButton, TXUIButton );

public:
	TXUIBackButton();
	~TXUIBackButton();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL EndPress() override;                                         // +0xC4
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8
};

TOSHI_NAMESPACE_END

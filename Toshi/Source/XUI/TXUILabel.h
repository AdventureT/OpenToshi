#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// XuiLabel (0x44 bytes, the size of XURXUIControlData): its only property (4 bytes) is skipped, not stored
//-----------------------------------------------------------------------------
class XURXUILabelData : public XURXUIControlData
{
public:
	XURXUILabelData();

	// $deBlob: FUNCTION 006d3d10
	// $deBlob: FUNCTION 006d3d00
	~XURXUILabelData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                // +0x04
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                        // +0x08
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;             // +0x0C
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;              // +0x14
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                  // +0x18
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                 // +0x1C
	virtual const TXUITypeInfo* GetTypeInfo() const override;                                                                // +0x20

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;
};

//-----------------------------------------------------------------------------
// TXUILabel (0x94): a control without members of its own
//-----------------------------------------------------------------------------
class TXUILabel : public TXUIControl
{
public:
	TDECLARE_CLASS( TXUILabel, TXUIControl );

public:
	// $deBlob: FUNCTION 0062d470
	TXUILabel() = default;

	// NOTE: emitted in game code
	// $deBlob: FUNCTION 0062d4b0
	// $deBlob: FUNCTION 0062d510
	// $deBlob: FUNCTION 0062d490
	// $deBlob: FUNCTION 0062d4ca
	~TXUILabel() = default;

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80
};

TOSHI_NAMESPACE_END

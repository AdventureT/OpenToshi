#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_START

class XURXUIScrollEndData : public XURXUIControlData
{
public:
	enum Prop
	{
		Prop_Direction,
		Prop_NUMOF,
	};

public:
	XURXUIScrollEndData();

	// $deBlob: FUNCTION 006e2c60
	// $deBlob: FUNCTION 006e2c10
	~XURXUIScrollEndData() = default;

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
	TUINT32 m_uiDirection; // 0x44 see TXUIList::ScrollEnd
};

//-----------------------------------------------------------------------------
// TXUIScrollEnd (0x98): the arrow of a TXUIList showing that the list can scroll that way (created by the list
// from the XuiScrollEnds of its visual)
//-----------------------------------------------------------------------------
class TXUIScrollEnd : public TXUIControl
{
public:
	TDECLARE_CLASS( TXUIScrollEnd, TXUIControl );

public:
	TXUIScrollEnd();
	~TXUIScrollEnd();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL Back() override; // +0xE0

private:
	TUINT32 m_uiDirection; // 0x94 see TXUIList::ScrollEnd
};

TOSHI_NAMESPACE_END

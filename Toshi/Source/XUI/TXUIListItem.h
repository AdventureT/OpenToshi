#pragma once
#include "TXUICheckBox.h"

TOSHI_NAMESPACE_START

class XURXUIListItemData : public XURXUICheckBoxData
{
public:
	enum Prop
	{
		Prop_Layout,
		Prop_Checkable,
		Prop_SelectedSize,
		Prop_KeepSizeUnfocused,
		Prop_InterItemSpacing,
		Prop_NUMOF,
	};

	// m_uiLayout: how TXUIList lays out its items
	enum Layout : TUINT32
	{
		Layout_Vertical0,   // 0
		Layout_Horizontal1, // 1
		Layout_Vertical2,   // 2
		Layout_Horizontal3, // 3
		Layout_GridRows,    // 4 left to right, then the next row
		Layout_GridColumns, // 5 top to bottom, then the next column
	};

public:
	XURXUIListItemData();

	// $deBlob: FUNCTION 006d0bd0
	// $deBlob: FUNCTION 006d0b70
	// $deBlob: FUNCTION 006d0b80
	~XURXUIListItemData() = default;

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
	TUINT32 m_uiLayout;           // 0x48 see Layout
	TUINT32 m_uiSelectedSize;     // 0x4C -1 if not set
	TUINT32 m_uiInterItemSpacing; // 0x50 index in the vector table, -1 if not set
	TBOOL   m_bCheckable;         // 0x54
	TBOOL   m_bKeepSizeUnfocused; // 0x55
};

//-----------------------------------------------------------------------------
// TXUIListItem (0x9C): an item of a TXUIList (created by the list from the XuiListItem of its visual)
//-----------------------------------------------------------------------------
class TXUIListItem : public TXUICheckBox
{
public:
	TDECLARE_CLASS( TXUIListItem, TXUICheckBox );

public:
	TXUIListItem();
	~TXUIListItem();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL Press() override;                                                                     // +0xC0
	virtual TBOOL EndPress() override;                                                                  // +0xC4
	virtual void  SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation ) override; // +0x108

	//-----------------------------------------------------------------------------
	// TXUICheckBox
	//-----------------------------------------------------------------------------
	virtual void SetChecked( TBOOL a_bChecked ) override; // +0x110

	// $deBlob: FUNCTION 006d0c10
	void SetCheckable( TBOOL a_bCheckable )
	{
		if ( a_bCheckable )
			m_uiControlFlags |= ControlFlags_Checkable;
		else
			m_uiControlFlags &= ~ControlFlags_Checkable;
	}

};

TOSHI_NAMESPACE_END

#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_START

class TXUIListItem;
class TXUIScrollEnd;

class XURXUIListData : public XURXUIControlData
{
public:
	enum Prop
	{
		Prop_Wrap,
		Prop_NUMOF,
	};

public:
	XURXUIListData();

	// $deBlob: FUNCTION 006d3a30
	// $deBlob: FUNCTION 006d3970
	~XURXUIListData() = default;

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
	TBOOL m_bWrap; // 0x44
};

//-----------------------------------------------------------------------------
// TXUIList (0xD8): a list of TXUIListItems created from the XuiListItem of the list's visual (one per item, the
// number of items is set by the derived classes, e.g. TXUICommonList), with up to 4 TXUIScrollEnds (also from the
// visual) shown while the list can scroll.
//-----------------------------------------------------------------------------
class TXUIList : public TXUIControl
{
public:
	TDECLARE_CLASS( TXUIList, TXUIControl );

public:
	// Directions of the scroll ends (XURXUIScrollEndData::m_uiDirection)
	enum ScrollEnd : TUINT32
	{
		ScrollEnd_Up,
		ScrollEnd_Down,
		ScrollEnd_Left,
		ScrollEnd_Right,
		ScrollEnd_NUMOF,
	};

public:
	TXUIList();
	~TXUIList();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL        Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;     // +0x5C
	virtual TXUIElement* RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController ) override;    // +0x64
	virtual void         UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void         UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80
	virtual void         OnCreated() override;                                                                                                                                               // +0x94

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override;                         // +0xF8
	virtual void  SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation ) override; // +0x108

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0062d090
	virtual void OnItemChecked( TXUIListItem* a_pItem ) {} // +0x110 (called by TXUIListItem::SetChecked)

	// $deBlob: FUNCTION 0062cf90
	virtual TINT Unknown114( TXUIListItem* a_pItem ) { return 0; } // +0x114

	virtual TBOOL         ScrollMoreLeft();                                                    // +0x118 items before the focussed one are hidden
	virtual TBOOL         ScrollMoreRight();                                                   // +0x11C items after the focussed one are hidden
	virtual void          UpdateScrollEnds( TUINT32 a_eScrollEnd, TBOOL a_bPressed );          // +0x120
	virtual const TWCHAR* GetListItemText( TINT a_iItemIndex );                                // +0x124
	virtual void          SetListItemText( TINT a_iItemIndex, const TWCHAR* a_pText );         // +0x128
	virtual const TWCHAR* GetListItemImage( TINT a_iItemIndex );                               // +0x12C
	virtual void          SetListItemImage( TINT a_iItemIndex, const TWCHAR* a_pImagePath );   // +0x130
	virtual TBOOL         GetListItemChecked( TINT a_iItemIndex );                             // +0x134
	virtual void          SetListItemChecked( TINT a_iItemIndex, TBOOL a_bChecked );           // +0x138
	virtual TBOOL         GetListItemEnabled( TINT a_iItemIndex );                             // +0x13C
	virtual void          SetListItemEnabled( TINT a_iItemIndex, TBOOL a_bEnabled );           // +0x140
	virtual void          SetListItemVisible( TINT a_iItemIndex, TBOOL a_bVisible );           // +0x144

	// $deBlob: FUNCTION 0062cf80
	virtual TINT GetFocussedItemIndex() { return m_uiFocussedItem; } // +0x148

	virtual void SetFocussedItemIndex( TINT a_iFocussedItem ); // +0x14C
	virtual TINT GetListItemIndex( TXUIListItem* a_pItem );    // +0x150

	// $deBlob: FUNCTION 0062cfb0
	virtual TUINT32 GetNumItems() { return m_uiNumItems; } // +0x154

	// $deBlob: FUNCTION 0062d080
	virtual void OnItemPressed( TXUIListItem* a_pItem ) {} // +0x158 (called by TXUIListItem::EndPress)

	void          SetFocussedItem( TUINT32 a_uiFocussedItem );
	void          Layout();
	TXUIListItem* GetListItem( TINT a_iItemIndex );

	// $deBlob: FUNCTION 006d3a60
	TXUIScrollEnd* GetScrollEndRight() const { return m_apScrollEnds[ ScrollEnd_Right ]; }

	// $deBlob: FUNCTION 006d3a70
	TXUIScrollEnd* GetScrollEndLeft() const { return m_apScrollEnds[ ScrollEnd_Left ]; }

	// NOTE: inlined (TXUIInput)
	// $deBlob: FUNCTION 006d7880
	TXUIListItem* GetRayTraceItem() const { return m_pRayTraceItem; }

private:
	// NOTE: not a retail function, see the .cpp
	void ScrollToFocussedItem( TUINT32 a_uiOldItem, TBOOL a_bVertical, TUINT32 a_eScrollEndOn, TUINT32 a_eScrollEndOff );

protected:
	TUINT32            m_uiNumItems;                          // 0x94
	TUINT32            m_uiFocussedItem;                      // 0x98
	XURXUIElementData* m_pItemData;                           // 0x9C the XuiListItem of the visual
	TXUIListItem*      m_pItems;                              // 0xA0 m_uiNumItems items
	XURXUIElementData* m_apScrollEndData[ ScrollEnd_NUMOF ];  // 0xA4 the XuiScrollEnds of the visual
	TXUIScrollEnd*     m_apScrollEnds[ ScrollEnd_NUMOF ];     // 0xB4
	TVector2           m_vInterItemSpacing;                   // 0xC4
	TXUIListItem*      m_pRayTraceItem;                       // 0xCC
	XURXUIObjectData*  m_pVisualData;                         // 0xD0
	TBOOL              m_bWrap;                               // 0xD4
	TBOOL              m_bScrollable;                         // 0xD5
	TBOOL              m_bSkin2;                              // 0xD6 the visual is in the second skin
};

TOSHI_NAMESPACE_END

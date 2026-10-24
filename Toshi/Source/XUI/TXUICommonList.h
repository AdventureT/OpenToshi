#pragma once
#include "TXUIList.h"

TOSHI_NAMESPACE_START

class XURXUICommonListData : public XURXUIListData
{
public:
	enum Prop
	{
		Prop_ItemsText,
		Prop_ItemsImage,
		Prop_ItemsNavPath,
		Prop_NUMOF,
	};

public:
	XURXUICommonListData();

	// $deBlob: FUNCTION 006d4890
	// $deBlob: FUNCTION 006d4840
	~XURXUICommonListData() = default;

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
	TUINT16 m_uiItemsText;    // 0x48 string index, the items separated by "\r\n"
	TUINT16 m_uiItemsImage;   // 0x4A string index, the items separated by "\r\n"
	TUINT16 m_uiItemsNavPath; // 0x4C string index, the items separated by "\r\n"
};

//-----------------------------------------------------------------------------
// TXUICommonList (0xF0): a TXUIList whose items (text, image and the .xur opened when the item is pressed) are
// set by the XUR data
//-----------------------------------------------------------------------------
class TXUICommonList : public TXUIList
{
public:
	TDECLARE_CLASS( TXUICommonList, TXUIList );

public:
	TXUICommonList();
	~TXUICommonList();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0063c690
	virtual const TWCHAR* GetListItemText( TINT a_iItemIndex ) override { return m_awszItemsText[ a_iItemIndex ]; } // +0x124

	// $deBlob: FUNCTION 0063c670
	virtual const TWCHAR* GetListItemImage( TINT a_iItemIndex ) override { return m_awszItemsImage[ a_iItemIndex ]; } // +0x12C

	virtual void OnItemPressed( TXUIListItem* a_pItem ) override; // +0x158

private:
	TWCHAR*  m_wszItemsText;       // 0xD8 copy of the XUR string
	TWCHAR*  m_wszItemsImage;      // 0xDC copy of the XUR string
	TWCHAR*  m_wszItemsNavPath;    // 0xE0 copy of the XUR string
	TWCHAR** m_awszItemsText;      // 0xE4 m_uiNumItems strings
	TWCHAR** m_awszItemsImage;     // 0xE8 m_uiNumItems strings
	TWCHAR** m_awszItemsNavPath;   // 0xEC m_uiNumItems strings
};

TOSHI_NAMESPACE_END

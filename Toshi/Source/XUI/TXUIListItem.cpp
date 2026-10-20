#include "ToshiPCH.h"
#include "TXUIListItem.h"
#include "TXUIList.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiListItem" type info
const TXUITypeInfo XURXUIListItemData::sm_sTypeInfo = { L"XuiListItem", &XURXUICheckBoxData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIListItemData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d0410
XURXUIListItemData::XURXUIListItemData()
{
	m_uiLayout           = 0;
	m_uiSelectedSize     = TUINT32( -1 );
	m_uiInterItemSpacing = TUINT32( -1 );
	m_bCheckable         = TFALSE;
	m_bKeepSizeUnfocused = TFALSE;

	m_pClass = TClass::Find( "TXUIListItem", &TGetClass( TXUICheckBox ) );
}

// $deBlob: FUNCTION 006d0450
TBOOL XURXUIListItemData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUICheckBoxData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006d0480
TUINT32 XURXUIListItemData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUICheckBoxData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006d04b0
TBOOL XURXUIListItemData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUICheckBoxData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d0510
TBOOL XURXUIListItemData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "Layout", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Layout;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Checkable", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Checkable;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "SelectedSize", -1 ) == 0 )
	{
		a_rPropIndex = Prop_SelectedSize;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "KeepSizeUnfocused", -1 ) == 0 )
	{
		a_rPropIndex = Prop_KeepSizeUnfocused;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "InterItemSpacing", -1 ) == 0 )
	{
		a_rPropIndex = Prop_InterItemSpacing;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUICheckBoxData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006d05e0
TUINT32 XURXUIListItemData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_Checkable || a_uiPropIndex == Prop_KeepSizeUnfocused )
			return 1;

		return 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUICheckBoxData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d0660
TBOOL XURXUIListItemData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUICheckBoxData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d06c0
TBOOL XURXUIListItemData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUICheckBoxData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d0720
TBOOL XURXUIListItemData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	if ( uiNumProps != 0 )
	{
		TUINT8 uiPropMask = 0;

		if ( m_uiPropLevelMask > 0 )
		{
			uiPropMask = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_Layout ) )
		{
			m_uiLayout = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Checkable ) )
		{
			m_bCheckable = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_SelectedSize ) )
		{
			m_uiSelectedSize = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_KeepSizeUnfocused ) )
		{
			m_bKeepSizeUnfocused = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_InterItemSpacing ) )
		{
			m_uiInterItemSpacing = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006d0400
const TXUITypeInfo* XURXUIListItemData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIListItem
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d07f0
// $deBlob: FUNCTION 006d0800
// $deBlob: FUNCTION 006d0840
// $deBlob: FUNCTION 006d0880
// $deBlob: FUNCTION 006d0890
// $deBlob: FUNCTION 0053f050
// $deBlob: FUNCTION 00859110
TXUI_DEFINE_CLASS( TXUIListItem );

// $deBlob: FUNCTION 006d08a0
TXUIListItem::TXUIListItem()
{
	m_uiControlFlags &= ~ControlFlags_Checkable;
}

// $deBlob: FUNCTION 006d08c0
// $deBlob: FUNCTION 006d0ba0
// $deBlob: FUNCTION 006d0b8b
// $deBlob: FUNCTION 006d3990
TXUIListItem::~TXUIListItem()
{
}

// $deBlob: FUNCTION 006d08e0
TBOOL TXUIListItem::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUICheckBox::Create( a_rResource, a_pData, a_bCreateChildren );

	if ( TSTATICCAST( XURXUIListItemData, a_pData )->m_bCheckable == TTRUE )
		m_uiControlFlags |= ControlFlags_Checkable;
	else
		m_uiControlFlags &= ~ControlFlags_Checkable;

	return TTRUE;
}

// $deBlob: FUNCTION 006d0920
void TXUIListItem::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	// NOTE: the own properties aren't animated
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUICheckBox::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006d0990
void TXUIListItem::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUICheckBox::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006d09d0
void TXUIListItem::SetChecked( TBOOL a_bChecked )
{
	if ( IsCheckable() )
	{
		TXUICheckBox::SetChecked( a_bChecked );

		TASSERT( GetParent()->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) );
		TXUIElement* pParent = TSTATICCAST( TXUIElement, GetParent() );
		TASSERT( pParent->IsA( &TGetClass( TXUIList ) ) );

		TSTATICCAST( TXUIList, pParent )->OnItemChecked( this );
	}
	else if ( GetTimelineState() != TimelineState_Press )
	{
		// Not checkable: show it pressed
		SetXUIStateBits( TimelineState_Press, 0xF, XUIState_TimelineStateShift );
		PlayVisual( TFALSE, TFALSE, TFALSE );
	}
}

// $deBlob: FUNCTION 006d0ab0
TBOOL TXUIListItem::Press()
{
	if ( !IsEnabled() )
		return TFALSE;

	if ( !IsCheckable() )
		TXUI::GetSingleton()->SetInputLocked( TTRUE );

	return TXUICheckBox::Press();
}

// $deBlob: FUNCTION 006d0af0
TBOOL TXUIListItem::EndPress()
{
	T2GUIElement* pParent = GetParent();
	TASSERT( pParent->GetFlag( FLAGS_XUIELEMENT ) );

	TSTATICCAST( TXUIList, pParent )->OnItemPressed( this );
	return TXUICheckBox::EndPress();
}

// $deBlob: FUNCTION 006d0b40
void TXUIListItem::SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation )
{
	// NOTE: retail calls IsCheckable and ignores the result
	IsCheckable();
	TXUIControl::SetFocus( a_bFocus, a_bNoAnimation, a_bFromNavigation );
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TXUIList.h"
#include "TXUITimelineControl.h"
#include "TXUIListItem.h"
#include "TXUIScrollEnd.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiList" type info
const TXUITypeInfo XURXUIListData::sm_sTypeInfo = { L"XuiList", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIListData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d13f0
XURXUIListData::XURXUIListData()
{
	m_bWrap  = TFALSE;
	m_pClass = TClass::Find( "TXUIList", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006d1420
TBOOL XURXUIListData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006d1470
TUINT32 XURXUIListData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006d14c0
TBOOL XURXUIListData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d1520
TBOOL XURXUIListData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "Wrap", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Wrap;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006d1570
TUINT32 XURXUIListData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex == Prop_Wrap ) ? 1 : 4;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d15e0
TBOOL XURXUIListData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d1640
TBOOL XURXUIListData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d16a0
TBOOL XURXUIListData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_Wrap ) )
		{
			m_bWrap = *a_rData;
			a_rData += 1;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006d13e0
const TXUITypeInfo* XURXUIListData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIList
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d16e0
// $deBlob: FUNCTION 006d16f0
// $deBlob: FUNCTION 006d1720
// $deBlob: FUNCTION 006d1740
// $deBlob: FUNCTION 006d1750
// $deBlob: FUNCTION 0053f0f0
// $deBlob: FUNCTION 00859150
TXUI_DEFINE_CLASS( TXUIList );

// $deBlob: FUNCTION 006d1760
TXUIList::TXUIList()
{
	m_uiNumItems        = 0;
	m_uiFocussedItem    = 0;
	m_pItemData         = TNULL;
	m_pItems            = TNULL;
	m_vInterItemSpacing = TVector2::VEC_ZERO;
	m_pRayTraceItem     = TNULL;
	m_pVisualData       = TNULL;
	m_bWrap             = TFALSE;
	m_bScrollable       = TFALSE;
	m_bSkin2            = TFALSE;

	for ( TINT i = 0; i < ScrollEnd_NUMOF; i++ )
	{
		m_apScrollEndData[ i ] = TNULL;
		m_apScrollEnds[ i ]    = TNULL;
	}
}

// $deBlob: FUNCTION 006d1830
// $deBlob: FUNCTION 006d3a00
// $deBlob: FUNCTION 006d397b
TXUIList::~TXUIList()
{
	if ( m_pItems != TNULL )
		delete[] m_pItems;

	for ( TINT i = 0; i < ScrollEnd_NUMOF; i++ )
	{
		if ( m_apScrollEnds[ i ] != TNULL )
			delete m_apScrollEnds[ i ];
	}
}

// $deBlob: FUNCTION 006d1890
TBOOL TXUIList::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, TTRUE );

	XURXUIListData* pListData = TSTATICCAST( XURXUIListData, a_pData );
	m_bWrap                   = pListData->m_bWrap == TTRUE;

	// Find the visual in the skins, it holds the XuiListItem and the XuiScrollEnds
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetSkin1Resource() != TNULL || pXUI->GetSkin2Resource() != TNULL )
	{
		if ( pListData->m_uiVisual != 0 )
		{
			const TWCHAR* wszVisual = a_rResource.GetString( pListData->m_uiVisual );
			m_pVisualData           = pXUI->GetSkin1Resource()->FindVisual( wszVisual );

			if ( m_pVisualData == TNULL )
			{
				// Skip "skin2"
				m_pVisualData = pXUI->GetSkin2Resource()->FindVisual( wszVisual + 5 );

				if ( m_pVisualData != TNULL )
					m_bSkin2 = TTRUE;
			}
		}
		else
		{
			// Use the default visual of the XUI class or of one of its parents
			const TXUITypeInfo* pTypeInfo = pListData->GetTypeInfo();
			TASSERT( 0 == TStringManager::String16Compare( pTypeInfo->GetXuiType(), a_rResource.GetString( pListData->m_uiTypeIndex ) ) );

			for ( ; pTypeInfo != TNULL; pTypeInfo = pTypeInfo->GetParent() )
			{
				const TWCHAR* wszXuiType = pTypeInfo->GetXuiType();
				m_pVisualData            = pXUI->GetSkin1Resource()->FindVisual( wszXuiType );

				if ( m_pVisualData == TNULL )
				{
					m_pVisualData = pXUI->GetSkin2Resource()->FindVisual( wszXuiType );

					if ( m_pVisualData != TNULL )
						m_bSkin2 = TTRUE;
				}

				if ( m_pVisualData != TNULL )
					break;
			}
		}
	}

	if ( m_pVisualData != TNULL )
	{
		for ( TUINT32 i = 0; i < m_pVisualData->GetNumChildren(); i++ )
		{
			XURXUIElementData* pChildData = m_pVisualData->GetChild( i );

			if ( pChildData->GetClass()->IsA( &TGetClass( TXUIListItem ) ) )
			{
				m_pItemData = pChildData;
			}
			else if ( pChildData->GetClass()->IsA( &TGetClass( TXUIScrollEnd ) ) )
			{
				TUINT32 uiDirection = TSTATICCAST( XURXUIScrollEndData, pChildData )->m_uiDirection;

				if ( m_apScrollEndData[ uiDirection ] == TNULL )
					m_apScrollEndData[ uiDirection ] = pChildData;
			}
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006d1a60
void TXUIList::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex == 0 )
	{
		TASSERT( a_uiPropIndex == XURXUIListData::Prop_Wrap );
		m_bWrap = a_uiValue1 == 1;
		return;
	}

	TASSERT( a_uiObjectIndex > 0 );
	TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006d1b10
void TXUIList::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

// Creates the items and the scroll ends and lays the items out
// $deBlob: FUNCTION 006d1b60
void TXUIList::OnCreated()
{
	if ( m_uiNumItems != 0 && m_pItemData != TNULL )
	{
		m_pItems = new ( TXUI::ms_pXUIMemoryBlock ) TXUIListItem[ m_uiNumItems ];

		TXUI*  pXUI = TXUI::GetSingleton();
		TFLOAT fX   = 0.0f;
		TFLOAT fY   = 0.0f;

		TXUIResource* pSkin = m_bSkin2 ? pXUI->GetSkin2Resource() : pXUI->GetSkin1Resource();
		m_vInterItemSpacing = *pSkin->GetVector( TSTATICCAST( XURXUIListItemData, m_pItemData )->m_uiInterItemSpacing );

		TUINT32 uiLayout    = TSTATICCAST( XURXUIListItemData, m_pItemData )->m_uiLayout;
		TBOOL   bVertical   = uiLayout == XURXUIListItemData::Layout_Vertical2 || uiLayout == XURXUIListItemData::Layout_Vertical0;
		TBOOL   bHorizontal = uiLayout == XURXUIListItemData::Layout_Horizontal3 || uiLayout == XURXUIListItemData::Layout_Horizontal1;

		for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
		{
			TXUIListItem* pItem = &m_pItems[ i ];

			TBOOL bRes = pItem->Create( *( m_bSkin2 ? pXUI->GetSkin2Resource() : pXUI->GetSkin1Resource() ), m_pItemData, TFALSE );
			pItem->SetFocus( TFALSE, TFALSE, TFALSE );
			TASSERT( TTRUE == bRes );

			AddChildTail( pItem );

			if ( pItem->GetVisual() == TNULL )
				continue;

			// Move the item to its place in the list
			TFLOAT fPosX = pItem->GetPositionX() + fX;
			TFLOAT fPosY = pItem->GetPositionY() + fY;
			pItem->SetPosition( fPosX, fPosY );

			pItem->SetText( GetListItemText( i ), -1 );
			pItem->SetImagePath( GetListItemImage( i ) );

			// Hide the items that don't fit, the list then scrolls
			if ( ( bVertical && TINT( i ) > 0 && pItem->GetHeight() + fPosY > GetHeight() ) ||
			     ( bHorizontal && TINT( i ) > 0 && pItem->GetWidth() + fPosX > GetWidth() ) )
			{
				pItem->SetVisible( TFALSE );
				m_bScrollable = TTRUE;
			}

			switch ( TSTATICCAST( XURXUIListItemData, m_pItemData )->m_uiLayout )
			{
				case XURXUIListItemData::Layout_Vertical0:
				case XURXUIListItemData::Layout_Vertical2:
					fY = pItem->GetHeight() + m_vInterItemSpacing.y + fY;
					break;
				case XURXUIListItemData::Layout_Horizontal1:
				case XURXUIListItemData::Layout_Horizontal3:
					fX = pItem->GetWidth() + m_vInterItemSpacing.x + fX;
					break;
				case XURXUIListItemData::Layout_GridRows:
					fX = pItem->GetWidth() + m_vInterItemSpacing.x + fX;

					if ( fX + 1.0f >= GetWidth() )
					{
						fX = 0.0f;
						fY = pItem->GetHeight() + m_vInterItemSpacing.y + fY;
					}

					break;
				case XURXUIListItemData::Layout_GridColumns:
					fY = pItem->GetHeight() + m_vInterItemSpacing.y + fY;

					if ( fY + 1.0f >= GetHeight() )
					{
						fY = 0.0f;
						fX = pItem->GetWidth() + m_vInterItemSpacing.x + fX;
					}

					break;
			}
		}

		m_uiFocussedItem = 0;
	}

	for ( TINT i = 0; i < ScrollEnd_NUMOF; i++ )
	{
		if ( m_apScrollEndData[ i ] != TNULL )
		{
			m_apScrollEnds[ i ] = new ( TXUI::ms_pXUIMemoryBlock ) TXUIScrollEnd();

			TXUI* pXUI = TXUI::GetSingleton();
			TBOOL bRes = m_apScrollEnds[ i ]->Create( *( m_bSkin2 ? pXUI->GetSkin2Resource() : pXUI->GetSkin1Resource() ), m_apScrollEndData[ i ], TFALSE );
			TASSERT( TTRUE == bRes );

			AddChildTail( m_apScrollEnds[ i ] );
		}
	}

	// Anchor the children to the size of the visual
	XURXUIElementData* pVisualData = TSTATICCAST( XURXUIElementData, m_pVisualData );

	TFLOAT fVisualWidth  = TFLOAT( pVisualData->m_iWidth ) * 0.16666667f;
	TFLOAT fVisualHeight = TFLOAT( pVisualData->m_iHeight ) * 0.16666667f;
	TFLOAT fWidth        = GetWidth();
	TFLOAT fHeight       = GetHeight();

	TVector2 vSizeDelta;
	vSizeDelta.x = fVisualWidth - fWidth;
	vSizeDelta.y = fVisualHeight - fHeight;
	UpdateAnchoring( vSizeDelta );
}

// $deBlob: FUNCTION 006d20d0
void TXUIList::SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation )
{
	if ( m_pItems != TNULL )
	{
		TASSERT( m_uiFocussedItem < m_uiNumItems );
		m_pItems[ m_uiFocussedItem ].SetFocus( a_bFocus, a_bNoAnimation, TTRUE );
	}

	if ( a_bFocus == TTRUE && m_pItems != TNULL && m_bScrollable == TTRUE )
	{
		for ( TINT i = 0; i < TINT( ScrollEnd_NUMOF ); i++ )
			UpdateScrollEnds( i, TFALSE );
	}

	TXUIControl::SetFocus( a_bFocus, a_bNoAnimation, a_bFromNavigation );
}

// $deBlob: FUNCTION 006d2190
TBOOL TXUIList::ScrollMoreLeft()
{
	TBOOL bMore = TFALSE;

	for ( TINT i = 0; i < TINT( m_uiFocussedItem ); i++ )
	{
		if ( !m_pItems[ i ].IsVisible() )
		{
			bMore = TTRUE;
			break;
		}
	}

	return bMore;
}

// $deBlob: FUNCTION 006d21d0
TBOOL TXUIList::ScrollMoreRight()
{
	for ( TUINT32 i = m_uiFocussedItem; i < m_uiNumItems; i++ )
	{
		if ( !m_pItems[ i ].IsVisible() )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006d2220
void TXUIList::UpdateScrollEnds( TUINT32 a_eScrollEnd, TBOOL a_bPressed )
{
	if ( m_apScrollEnds[ a_eScrollEnd ] == TNULL )
		return;

	TASSERT( TTRUE == m_bScrollable );

	if ( ( ( a_eScrollEnd == ScrollEnd_Down || a_eScrollEnd == ScrollEnd_Right ) && ScrollMoreRight() ) ||
	     ( ( a_eScrollEnd == ScrollEnd_Up || a_eScrollEnd == ScrollEnd_Left ) && ScrollMoreLeft() ) )
	{
		// The list can scroll this way
		if ( a_bPressed )
			m_apScrollEnds[ a_eScrollEnd ]->PlayAnimation( TXUITimelineControl::StdNamedFrame_Scrolling, TXUITimelineControl::StdNamedFrame_EndScrolling );
		else
			m_apScrollEnds[ a_eScrollEnd ]->PlayAnimation( TXUITimelineControl::StdNamedFrame_ScrollMore, TXUITimelineControl::StdNamedFrame_EndScrollMore );
	}
	else
	{
		m_apScrollEnds[ a_eScrollEnd ]->PlayAnimation( TXUITimelineControl::StdNamedFrame_Normal, TXUITimelineControl::StdNamedFrame_EndNormal );
	}
}

// $deBlob: FUNCTION 006d2300
TBOOL TXUIList::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( m_pItems == TNULL || a_iController != TXUI::GetSingleton()->GetDefaultController() )
		return TFALSE;

	TBOOL   bHandled = TFALSE;
	TUINT32 uiOld    = m_uiFocussedItem;

	switch ( a_eButton )
	{
		case XUIButton_Accept:
			return m_pItems[ uiOld ].HandleButton( XUIButton_Accept, a_iController );
		case XUIButton_Up:
		{
			// Up
			TUINT32 uiLayout = TSTATICCAST( XURXUIListItemData, m_pItemData )->m_uiLayout;

			if ( uiLayout == XURXUIListItemData::Layout_Vertical0 || uiLayout == XURXUIListItemData::Layout_Vertical2 )
			{
				if ( uiOld == 0 && m_bWrap == TTRUE )
				{
					m_uiFocussedItem = m_uiNumItems - 1;
					bHandled         = TTRUE;
				}
				else if ( TINT( uiOld ) > 0 )
				{
					m_uiFocussedItem = uiOld - 1;
					bHandled         = TTRUE;
				}
			}
			else if ( uiLayout == XURXUIListItemData::Layout_GridColumns || uiLayout == XURXUIListItemData::Layout_GridRows )
			{
				TINT iNumPerRow = TMath::Round( ( GetWidth() + m_vInterItemSpacing.x ) / ( m_pItems[ 0 ].GetWidth() + m_vInterItemSpacing.x ) );

				if ( m_uiFocussedItem > TUINT32( iNumPerRow - 1 ) )
				{
					m_uiFocussedItem -= iNumPerRow;
					bHandled = TTRUE;
				}
				else if ( m_bWrap == TTRUE )
				{
					m_uiFocussedItem = m_uiFocussedItem - iNumPerRow + m_uiNumItems;
					bHandled         = TTRUE;
				}
			}

			ScrollToFocussedItem( uiOld, TTRUE, ScrollEnd_Up, ScrollEnd_Down );
			return bHandled;
		}
		case XUIButton_Down:
		{
			// Down
			TUINT32 uiLayout = TSTATICCAST( XURXUIListItemData, m_pItemData )->m_uiLayout;

			if ( uiLayout == XURXUIListItemData::Layout_Vertical0 || uiLayout == XURXUIListItemData::Layout_Vertical2 )
			{
				if ( uiOld < m_uiNumItems - 1 )
				{
					m_uiFocussedItem = uiOld + 1;
					bHandled         = TTRUE;
				}
				else if ( m_bWrap == TTRUE )
				{
					m_uiFocussedItem = 0;
					bHandled         = TTRUE;
				}
			}
			else if ( uiLayout == XURXUIListItemData::Layout_GridColumns || uiLayout == XURXUIListItemData::Layout_GridRows )
			{
				TINT iNumPerRow = TMath::Round( ( GetWidth() + m_vInterItemSpacing.x ) / ( m_pItems[ 0 ].GetWidth() + m_vInterItemSpacing.x ) );

				if ( m_uiFocussedItem < m_uiNumItems - iNumPerRow )
				{
					m_uiFocussedItem = iNumPerRow + m_uiFocussedItem;
					bHandled         = TTRUE;
				}
				else if ( m_bWrap == TTRUE )
				{
					m_uiFocussedItem = iNumPerRow + m_uiFocussedItem - m_uiNumItems;
					bHandled         = TTRUE;
				}
			}

			ScrollToFocussedItem( uiOld, TTRUE, ScrollEnd_Down, ScrollEnd_Up );
			return bHandled;
		}
		case XUIButton_Left:
		{
			// Left
			TUINT32 uiLayout = TSTATICCAST( XURXUIListItemData, m_pItemData )->m_uiLayout;

			if ( uiLayout == XURXUIListItemData::Layout_Horizontal1 || uiLayout == XURXUIListItemData::Layout_Horizontal3 )
			{
				if ( uiOld == 0 && m_bWrap == TTRUE )
				{
					m_uiFocussedItem = m_uiNumItems - 1;
					bHandled         = TTRUE;
				}
				else if ( TINT( uiOld ) > 0 )
				{
					m_uiFocussedItem = uiOld - 1;
					bHandled         = TTRUE;
				}
			}
			else if ( uiLayout == XURXUIListItemData::Layout_GridColumns || uiLayout == XURXUIListItemData::Layout_GridRows )
			{
				TUINT32 uiNumPerRow = TMath::Round( ( GetWidth() + m_vInterItemSpacing.x ) / ( m_pItems[ 0 ].GetWidth() + m_vInterItemSpacing.x ) );

				if ( m_uiFocussedItem % uiNumPerRow != 0 )
				{
					m_uiFocussedItem = m_uiFocussedItem - 1;
					bHandled         = TTRUE;
				}
				else if ( m_bWrap == TTRUE )
				{
					m_uiFocussedItem = m_uiFocussedItem + uiNumPerRow - 1;
					bHandled         = TTRUE;
				}
			}

			ScrollToFocussedItem( uiOld, TFALSE, ScrollEnd_Left, ScrollEnd_Right );
			return bHandled;
		}
		case XUIButton_Right:
		{
			// Right
			TUINT32 uiLayout = TSTATICCAST( XURXUIListItemData, m_pItemData )->m_uiLayout;

			if ( uiLayout == XURXUIListItemData::Layout_Horizontal1 || uiLayout == XURXUIListItemData::Layout_Horizontal3 )
			{
				if ( uiOld < m_uiNumItems - 1 )
				{
					m_uiFocussedItem = uiOld + 1;
					bHandled         = TTRUE;
				}
				else if ( m_bWrap == TTRUE )
				{
					m_uiFocussedItem = 0;
					bHandled         = TTRUE;
				}
			}
			else if ( uiLayout == XURXUIListItemData::Layout_GridColumns || uiLayout == XURXUIListItemData::Layout_GridRows )
			{
				TUINT32 uiNumPerRow = TMath::Round( ( GetWidth() + m_vInterItemSpacing.x ) / ( m_pItems[ 0 ].GetWidth() + m_vInterItemSpacing.x ) );

				if ( m_uiFocussedItem % uiNumPerRow != uiNumPerRow - 1 )
				{
					m_uiFocussedItem = m_uiFocussedItem + 1;
					bHandled         = TTRUE;
				}
				else if ( m_bWrap == TTRUE )
				{
					m_uiFocussedItem = m_uiFocussedItem - uiNumPerRow + 1;
					bHandled         = TTRUE;
				}
			}

			ScrollToFocussedItem( uiOld, TFALSE, ScrollEnd_Right, ScrollEnd_Left );
			return bHandled;
		}
	}

	return TFALSE;
}

// NOTE: not a retail function: HandleButton's four inlined copies of the scrolling (one per direction)
void TXUIList::ScrollToFocussedItem( TUINT32 a_uiOldItem, TBOOL a_bVertical, TUINT32 a_eScrollEndOn, TUINT32 a_eScrollEndOff )
{
	TINT iFocussedItem = TINT( m_uiFocussedItem );
	TMath::Clip( iFocussedItem, 0, TINT( m_uiNumItems - 1 ) );
	m_uiFocussedItem = iFocussedItem;

	if ( m_uiFocussedItem == a_uiOldItem )
		return;

	TXUIListItem* pNewItem = &m_pItems[ m_uiFocussedItem ];

	if ( a_bVertical )
	{
		TFLOAT fNewY = pNewItem->GetPositionY();

		if ( m_bScrollable == TTRUE && ( fNewY < 0.0f || pNewItem->GetHeight() + fNewY > GetHeight() ) )
		{
			// Scroll the items by one item
			for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
			{
				TXUIListItem* pItem = &m_pItems[ i ];
				TFLOAT        fPosX = pItem->GetPositionX();
				TFLOAT        fPosY = pItem->GetPositionY();

				if ( fNewY < 0.0f )
					fPosY = pItem->GetHeight() + fPosY;
				else
					fPosY = fPosY - pItem->GetHeight();

				pItem->SetPosition( fPosX, fPosY );

				TBOOL bVisible = TFALSE;

				if ( fPosY >= 0.0f && GetHeight() > pItem->GetHeight() + fPosY )
					bVisible = TTRUE;

				pItem->SetVisible( bVisible );
			}

			m_pItems[ a_uiOldItem ].SetFocus( TFALSE, TFALSE, TFALSE );
			UpdateScrollEnds( a_eScrollEndOn, TTRUE );
			UpdateScrollEnds( a_eScrollEndOff, TFALSE );
		}
		else
		{
			m_pItems[ a_uiOldItem ].SetFocus( TFALSE, TFALSE, TTRUE );
		}
	}
	else
	{
		TFLOAT fNewX = pNewItem->GetPositionX();

		if ( m_bScrollable == TTRUE && ( fNewX < 0.0f || pNewItem->GetWidth() + fNewX > GetWidth() ) )
		{
			// Scroll the items by one item
			for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
			{
				TXUIListItem* pItem = &m_pItems[ i ];
				TFLOAT        fPosX = pItem->GetPositionX();
				TFLOAT        fPosY = pItem->GetPositionY();

				if ( fNewX < 0.0f )
					fPosX = pItem->GetWidth() + fPosX;
				else
					fPosX = fPosX - pItem->GetWidth();

				pItem->SetPosition( fPosX, fPosY );

				TBOOL bVisible = TFALSE;

				if ( fPosX >= 0.0f && GetWidth() > pItem->GetWidth() + fPosX )
					bVisible = TTRUE;

				pItem->SetVisible( bVisible );
			}

			m_pItems[ a_uiOldItem ].SetFocus( TFALSE, TFALSE, TFALSE );
			UpdateScrollEnds( a_eScrollEndOn, TTRUE );
			UpdateScrollEnds( a_eScrollEndOff, TFALSE );
		}
		else
		{
			m_pItems[ a_uiOldItem ].SetFocus( TFALSE, TFALSE, TTRUE );
		}
	}

	m_pItems[ m_uiFocussedItem ].SetFocus( TTRUE, TFALSE, TTRUE );
	m_pItems[ m_uiFocussedItem ].SetVisible( TTRUE );
}

// $deBlob: FUNCTION 006d2da0
TXUIElement* TXUIList::RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController )
{
	if ( !IsVisible() )
		return TNULL;

	TXUIElement* pHit = TXUIElement::RayTraceDeepest( a_rPoint, a_rScale, a_iController );

	if ( pHit == TNULL )
	{
		m_pRayTraceItem = TNULL;
		return TNULL;
	}

	TUINT32 uiOld   = m_uiFocussedItem;
	m_pRayTraceItem = pHit->IsA( &TGetClass( TXUIListItem ) ) ? TSTATICCAST( TXUIListItem, pHit ) : TNULL;

	// Focus the item under the cursor
	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		if ( &m_pItems[ i ] == pHit )
		{
			m_uiFocussedItem = i;

			if ( i != uiOld )
			{
				m_pItems[ uiOld ].SetFocus( TFALSE, TFALSE, TTRUE );
				m_pItems[ m_uiFocussedItem ].SetFocus( TTRUE, TFALSE, TTRUE );
			}
		}
	}

	return this;
}

// $deBlob: FUNCTION 006d2e90
const TWCHAR* TXUIList::GetListItemText( TINT a_iItemIndex )
{
	TASSERT( a_iItemIndex >= 0 && TUINT32( a_iItemIndex ) < m_uiNumItems );
	return m_pItems[ a_iItemIndex ].GetText();
}

// $deBlob: FUNCTION 006d2ef0
void TXUIList::SetListItemText( TINT a_iItemIndex, const TWCHAR* a_pText )
{
	TASSERT( a_iItemIndex >= 0 && TUINT32( a_iItemIndex ) < m_uiNumItems );
	TASSERT( TNULL != a_pText );
	m_pItems[ a_iItemIndex ].SetText( a_pText, -1 );
}

// $deBlob: FUNCTION 006d2f90
const TWCHAR* TXUIList::GetListItemImage( TINT a_iItemIndex )
{
	TASSERT( a_iItemIndex >= 0 && TUINT32( a_iItemIndex ) < m_uiNumItems );
	return m_pItems[ a_iItemIndex ].GetImagePath();
}

// $deBlob: FUNCTION 006d2ff0
void TXUIList::SetListItemImage( TINT a_iItemIndex, const TWCHAR* a_pImagePath )
{
	TASSERT( a_iItemIndex >= 0 && TUINT32( a_iItemIndex ) < m_uiNumItems );
	TASSERT( TNULL != a_pImagePath );
	m_pItems[ a_iItemIndex ].SetImagePath( a_pImagePath );
}

// $deBlob: FUNCTION 006d3090
TBOOL TXUIList::GetListItemChecked( TINT a_iItemIndex )
{
	TASSERT( a_iItemIndex >= 0 && TUINT32( a_iItemIndex ) < m_uiNumItems );
	return m_pItems[ a_iItemIndex ].IsChecked();
}

// $deBlob: FUNCTION 006d3100
void TXUIList::SetListItemChecked( TINT a_iItemIndex, TBOOL a_bChecked )
{
	TASSERT( a_iItemIndex >= 0 && TUINT32( a_iItemIndex ) < m_uiNumItems );
	m_pItems[ a_iItemIndex ].SetChecked( a_bChecked );
}

// $deBlob: FUNCTION 006d3170
TBOOL TXUIList::GetListItemEnabled( TINT a_iItemIndex )
{
	TASSERT( a_iItemIndex >= 0 && TUINT32( a_iItemIndex ) < m_uiNumItems );
	return m_pItems[ a_iItemIndex ].IsEnabled();
}

// $deBlob: FUNCTION 006d31e0
void TXUIList::SetListItemEnabled( TINT a_iItemIndex, TBOOL a_bEnabled )
{
	TASSERT( a_iItemIndex >= 0 && TUINT32( a_iItemIndex ) < m_uiNumItems );
	m_pItems[ a_iItemIndex ].SetEnabled( a_bEnabled );
}

// $deBlob: FUNCTION 006d3270
void TXUIList::SetListItemVisible( TINT a_iItemIndex, TBOOL a_bVisible )
{
	m_pItems[ a_iItemIndex ].SetVisible( a_bVisible );
}

// $deBlob: FUNCTION 006d3290
void TXUIList::SetFocussedItemIndex( TINT a_iFocussedItem )
{
	if ( m_pItems == TNULL )
		return;

	TASSERT( a_iFocussedItem >= 0 && TUINT32( a_iFocussedItem ) < m_uiNumItems );

	if ( TUINT32( a_iFocussedItem ) == m_uiFocussedItem )
		return;

	TXUIListItem* pNewItem = &m_pItems[ a_iFocussedItem ];
	TFLOAT        fNewX    = pNewItem->GetPositionX();

	if ( m_bScrollable == TTRUE && ( fNewX < 0.0f || pNewItem->GetWidth() + fNewX > GetWidth() ) )
	{
		// Scroll the items so the new one is shown
		for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
		{
			TXUIListItem* pItem = &m_pItems[ i ];
			TFLOAT        fPosX = pItem->GetPositionX();
			TFLOAT        fPosY = pItem->GetPositionY();

			fPosX = pItem->GetWidth() * TFLOAT( TINT( m_uiFocussedItem - a_iFocussedItem ) ) + fPosX;
			pItem->SetPosition( fPosX, fPosY );

			TBOOL bVisible = TFALSE;

			if ( fPosX >= 0.0f && GetWidth() > pItem->GetWidth() + fPosX )
				bVisible = TTRUE;

			pItem->SetVisible( bVisible );
		}

		m_pItems[ m_uiFocussedItem ].SetFocus( TFALSE, TFALSE, TFALSE );
	}
	else
	{
		m_pItems[ m_uiFocussedItem ].SetFocus( TFALSE, TFALSE, TTRUE );
	}

	m_uiFocussedItem = a_iFocussedItem;

	for ( TINT i = 0; i < TINT( ScrollEnd_NUMOF ); i++ )
		UpdateScrollEnds( i, TFALSE );
}

// $deBlob: FUNCTION 006d34d0
TINT TXUIList::GetListItemIndex( TXUIListItem* a_pItem )
{
	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		if ( &m_pItems[ i ] == a_pItem )
			return TINT( i );
	}

	return -1;
}

// $deBlob: FUNCTION 006d3510
void TXUIList::SetFocussedItem( TUINT32 a_uiFocussedItem )
{
	TASSERT( a_uiFocussedItem < m_uiNumItems );

	if ( a_uiFocussedItem != m_uiFocussedItem )
	{
		m_pItems[ m_uiFocussedItem ].SetFocus( TFALSE, TFALSE, TTRUE );
		m_pItems[ a_uiFocussedItem ].SetFocus( TTRUE, TFALSE, TTRUE );
		m_uiFocussedItem = a_uiFocussedItem;

		Layout();

		for ( TINT i = 0; i < TINT( ScrollEnd_NUMOF ); i++ )
			UpdateScrollEnds( i, TFALSE );
	}
}

// Scrolls the items so the focussed one is shown
// $deBlob: FUNCTION 006d35c0
void TXUIList::Layout()
{
	TXUIListItem* pFocussedItem = &m_pItems[ m_uiFocussedItem ];
	TFLOAT        fFocussedX    = pFocussedItem->GetPositionX();
	TFLOAT        fFocussedY    = pFocussedItem->GetPositionY();

	switch ( TSTATICCAST( XURXUIListItemData, m_pItemData )->m_uiLayout )
	{
		case XURXUIListItemData::Layout_Vertical0:
		case XURXUIListItemData::Layout_Vertical2:
		{
			TBOOL  bPositive = TFALSE;
			TFLOAT fOffset   = 0.0f;

			if ( fFocussedY >= 0.0f )
			{
				if ( pFocussedItem->GetHeight() + fFocussedY > GetHeight() )
				{
					bPositive = TTRUE;
					fOffset   = -fFocussedY;
				}
			}
			else
			{
				fOffset = fFocussedY;
			}

			for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
			{
				TXUIListItem* pItem = &m_pItems[ i ];
				TFLOAT        fPosY = ( bPositive ? 1.0f : -1.0f ) * fOffset + pItem->GetPositionY();
				pItem->SetPosition( pItem->GetPositionX(), fPosY );

				if ( fPosY < 0.0f || GetHeight() < pItem->GetHeight() + fPosY )
					pItem->SetVisible( TFALSE );
				else
					pItem->SetVisible( TTRUE );
			}

			break;
		}
		case XURXUIListItemData::Layout_Horizontal1:
		case XURXUIListItemData::Layout_Horizontal3:
		{
			TBOOL  bNegative = TFALSE;
			TFLOAT fOffset   = 0.0f;

			if ( fFocussedX >= 0.0f )
			{
				if ( pFocussedItem->GetWidth() + fFocussedX > GetWidth() )
					fOffset = -fFocussedX;
			}
			else
			{
				fOffset   = fFocussedX;
				bNegative = TTRUE;
			}

			for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
			{
				TXUIListItem* pItem = &m_pItems[ i ];
				TFLOAT        fPosY = pItem->GetPositionY();
				TFLOAT        fPosX = ( bNegative ? -1.0f : 1.0f ) * fOffset + pItem->GetPositionX();
				pItem->SetPosition( fPosX, fPosY );

				if ( fPosX < 0.0f || GetWidth() < pItem->GetWidth() + fPosX )
					pItem->SetVisible( TFALSE );
				else
					pItem->SetVisible( TTRUE );
			}

			break;
		}
	}
}

// $deBlob: FUNCTION 006d3940
TXUIListItem* TXUIList::GetListItem( TINT a_iItemIndex )
{
	if ( a_iItemIndex >= 0 && TUINT32( a_iItemIndex ) < m_uiNumItems )
		return &m_pItems[ a_iItemIndex ];

	return TNULL;
}

TOSHI_NAMESPACE_END

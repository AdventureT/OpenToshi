#include "ToshiPCH.h"
#include "TXUIRadioGroup.h"
#include "TXUIRadioButton.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiRadioGroup" type info
const TXUITypeInfo XURXUIRadioGroupData::sm_sTypeInfo = { L"XuiRadioGroup", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIRadioGroupData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e4ca0
XURXUIRadioGroupData::XURXUIRadioGroupData()
{
	m_pClass = TClass::Find( "TXUIRadioGroup", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006e4cd0
TBOOL XURXUIRadioGroupData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e4d00
TUINT32 XURXUIRadioGroupData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	a_rData += 1;
	return uiSize;
}

// $deBlob: FUNCTION 006e4d20
TBOOL XURXUIRadioGroupData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006e4d40
TBOOL XURXUIRadioGroupData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e4d60
TUINT32 XURXUIRadioGroupData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );

	return 4;
}

// $deBlob: FUNCTION 006e4d80
TBOOL XURXUIRadioGroupData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006e4da0
TBOOL XURXUIRadioGroupData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// NOTE: only skips the property count, returns whether it is 0
// $deBlob: FUNCTION 006e4dc0
TBOOL XURXUIRadioGroupData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	return uiNumProps == 0 ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006e4c90
const TXUITypeInfo* XURXUIRadioGroupData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIRadioGroup
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e4de0
// $deBlob: FUNCTION 006e4df0
// $deBlob: FUNCTION 006e4e50
// $deBlob: FUNCTION 006e4ea0
// $deBlob: FUNCTION 006e4eb0
// $deBlob: FUNCTION 0053f610
// $deBlob: FUNCTION 00859350
TXUI_DEFINE_CLASS( TXUIRadioGroup );

// $deBlob: FUNCTION 006e4ec0
TXUIRadioGroup::TXUIRadioGroup()
{
	m_uiNumButtons = 0;
	m_pFocussed    = TNULL;
	m_apButtons    = TNULL;
}

// $deBlob: FUNCTION 006e4f00
// $deBlob: FUNCTION 006e5480
// $deBlob: FUNCTION 006e546b
TXUIRadioGroup::~TXUIRadioGroup()
{
	for ( TUINT32 i = 0; i < m_uiNumButtons; i++ )
		m_apButtons[ i ] = TNULL;

	delete m_apButtons;
}

// $deBlob: FUNCTION 006e4f50
TBOOL TXUIRadioGroup::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, a_bCreateChildren );

	// Count the radio buttons
	for ( T2GUIElementNode* pNode = m_Children.GetNext(); pNode != &m_Children && pNode != TNULL; pNode = pNode->GetNext() )
	{
		T2GUIElement* pChild = TSTATICCAST( T2GUIElement, pNode );

		if ( pChild->GetFlag( FLAGS_XUIELEMENT ) && TSTATICCAST( TXUIElement, pChild )->IsA( &TGetClass( TXUIRadioButton ) ) )
			m_uiNumButtons++;
	}

	if ( m_uiNumButtons != 0 )
	{
		m_apButtons = new ( TXUI::ms_pXUIMemoryBlock ) TXUIRadioButton*[ m_uiNumButtons ];

		TUINT32 uiIndex = 0;

		for ( T2GUIElementNode* pNode = m_Children.GetNext(); pNode != &m_Children && pNode != TNULL; pNode = pNode->GetNext() )
		{
			T2GUIElement* pChild = TSTATICCAST( T2GUIElement, pNode );

			if ( pChild->GetFlag( FLAGS_XUIELEMENT ) && TSTATICCAST( TXUIElement, pChild )->IsA( &TGetClass( TXUIRadioButton ) ) )
				m_apButtons[ uiIndex++ ] = TSTATICCAST( TXUIRadioButton, pChild );
		}
	}

	for ( TUINT32 i = 0; i < m_uiNumButtons; i++ )
		m_apButtons[ i ]->ResolveNavigation( this );

	// NOTE: retail crashes on a group without radio buttons (m_apButtons is TNULL)
	m_pFocussed = m_apButtons[ 0 ];

	return TTRUE;
}

// $deBlob: FUNCTION 006e5070
void TXUIRadioGroup::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006e50a0
void TXUIRadioGroup::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006e50f0
void TXUIRadioGroup::SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation )
{
	if ( m_apButtons == TNULL )
		return;

	if ( m_pFocussed != TNULL )
		m_pFocussed->SetFocus( a_bFocus, a_bNoAnimation, TTRUE );

	// NOTE: retail passes the parameters shifted by one
	TXUIControl::SetFocus( a_bNoAnimation, a_bFromNavigation, TTRUE );
}

// $deBlob: FUNCTION 006e5130
TBOOL TXUIRadioGroup::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( m_apButtons == TNULL || a_iController != TXUI::GetSingleton()->GetDefaultController() )
		return TFALSE;

	if ( m_pFocussed == TNULL )
		return TFALSE;

	TXUIControl* pNewFocus;

	switch ( a_eButton )
	{
		case XUIButton_Accept:
			// Check the focussed button
			if ( !m_pFocussed->IsEnabled() )
				return TFALSE;

			for ( TUINT32 i = 0; i < m_uiNumButtons; i++ )
				m_apButtons[ i ]->Toggle( m_apButtons[ i ] == m_pFocussed );

			return TTRUE;
		case XUIButton_Up:
			pNewFocus = m_pFocussed->GetNav( Nav_Up );
			break;
		case XUIButton_Down:
			pNewFocus = m_pFocussed->GetNav( Nav_Down );
			break;
		case XUIButton_Left:
			pNewFocus = m_pFocussed->GetNav( Nav_Left );
			break;
		case XUIButton_Right:
			pNewFocus = m_pFocussed->GetNav( Nav_Right );
			break;
		default:
			return TFALSE;
	}

	if ( pNewFocus != TNULL && m_pFocussed->IsFocused() )
	{
		m_pFocussed->SetFocus( TFALSE, TFALSE, TTRUE );
		pNewFocus->SetFocus( TTRUE, TFALSE, TTRUE );

		TASSERT( pNewFocus->IsA( &TGetClass( TXUIRadioButton ) ) );
		m_pFocussed = TSTATICCAST( TXUIRadioButton, pNewFocus );

		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006e52a0
TXUIElement* TXUIRadioGroup::RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController )
{
	if ( !IsVisible() )
		return TNULL;

	TXUIElement* pHit = TXUIElement::RayTraceDeepest( a_rPoint, a_rScale, a_iController );

	if ( pHit == TNULL )
		return TNULL;

	// NOTE: the converted Id is unused (debug leftover)
	TCHAR         szID[ 256 ];
	const TWCHAR* wszID = pHit->GetID();
	TStringManager::StringUnicodeToChar( szID, wszID, -1 );

	TXUIRadioButton* pOldFocus = m_pFocussed;

	// Focus the button under the cursor
	for ( TUINT32 i = 0; i < m_uiNumButtons; i++ )
	{
		TXUIRadioButton* pButton = m_apButtons[ i ];

		if ( pButton == pHit )
		{
			m_pFocussed = pButton;

			if ( pButton != pOldFocus )
			{
				pOldFocus->SetFocus( TFALSE, TFALSE, TTRUE );
				m_pFocussed->SetFocus( TTRUE, TFALSE, TTRUE );
			}
		}
	}

	return this;
}

// $deBlob: FUNCTION 006e53a0
TXUIRadioButton* TXUIRadioGroup::GetChecked()
{
	for ( TUINT32 i = 0; i < m_uiNumButtons; i++ )
	{
		if ( m_apButtons[ i ]->GetChecked() )
			return m_apButtons[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 006e53e0
void TXUIRadioGroup::SetChecked( TXUIRadioButton* a_pButton )
{
	TUINT32 i = 0;

	for ( ; i < m_uiNumButtons; i++ )
	{
		if ( m_apButtons[ i ] == a_pButton )
			break;
	}

	if ( i >= m_uiNumButtons )
		return;

	for ( TUINT32 j = 0; j < m_uiNumButtons; j++ )
	{
		if ( m_apButtons[ j ] != a_pButton )
			m_apButtons[ j ]->Toggle( TFALSE );
	}

	a_pButton->Toggle( TTRUE );
}

TOSHI_NAMESPACE_END

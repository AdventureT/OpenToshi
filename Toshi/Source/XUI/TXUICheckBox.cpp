#include "ToshiPCH.h"
#include "TXUICheckBox.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiCheckbox" type info
const TXUITypeInfo XURXUICheckBoxData::sm_sTypeInfo = { L"XuiCheckbox", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUICheckBoxData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e1eb0
XURXUICheckBoxData::XURXUICheckBoxData()
{
	m_uiPressKey = 0;
	m_pClass     = TClass::Find( "TXUICheckBox", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006e1ee0
TBOOL XURXUICheckBoxData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e1f40
TUINT32 XURXUICheckBoxData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e1fb0
TBOOL XURXUICheckBoxData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e2010
TBOOL XURXUICheckBoxData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "PressKey", -1 ) == 0 )
	{
		a_rPropIndex = Prop_PressKey;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e2060
TUINT32 XURXUICheckBoxData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return 4;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e20c0
TBOOL XURXUICheckBoxData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e2120
TBOOL XURXUICheckBoxData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e2180
TBOOL XURXUICheckBoxData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_PressKey ) )
		{
			m_uiPressKey = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e1ea0
const TXUITypeInfo* XURXUICheckBoxData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUICheckBox
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e21e0
// $deBlob: FUNCTION 006e21f0
// $deBlob: FUNCTION 006e2240
// $deBlob: FUNCTION 006e2290
// $deBlob: FUNCTION 006e22a0
// $deBlob: FUNCTION 0053f430
// $deBlob: FUNCTION 00859290
TXUI_DEFINE_CLASS( TXUICheckBox );

// $deBlob: FUNCTION 006e22b0
TXUICheckBox::TXUICheckBox()
{
	m_uiControlFlags |= ControlFlags_Checkable;
	m_bChecked   = TFALSE;
	m_uiPressKey = 0;
}

// $deBlob: FUNCTION 006e22f0
// $deBlob: FUNCTION 006e25e0
// $deBlob: FUNCTION 006e25c9
TXUICheckBox::~TXUICheckBox()
{
}

// $deBlob: FUNCTION 006e2310
TBOOL TXUICheckBox::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, a_bCreateChildren );
	m_bChecked = TFALSE;

	return TTRUE;
}

// $deBlob: FUNCTION 006e2340
void TXUICheckBox::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	TASSERT( a_uiPropIndex < XURXUICheckBoxData::Prop_NUMOF );

	// Prop_PressKey
	TFLOAT fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

	if ( fScalar == 0.0f )
	{
		m_uiPressKey = a_uiValue1;
		return;
	}

	TASSERT( a_fScalar == 1.0f );
	m_uiPressKey = a_uiValue2;
}

// NOTE: retail inlines TXUIControl's float version, calling TXUIElement's with a_uiObjectIndex - 2
// $deBlob: FUNCTION 006e2460
void TXUICheckBox::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006d0c00
TBOOL TXUICheckBox::IsChecked()
{
	return m_bChecked;
}

// $deBlob: FUNCTION 006e24b0
void TXUICheckBox::SetChecked( TBOOL a_bChecked )
{
	if ( m_bChecked == a_bChecked )
		return;

	m_bChecked = a_bChecked;
	PlayVisual( TFALSE, TFALSE, TFALSE );

	TASSERT( TTRUE == GetParent()->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) );

	// Notify the parents (+0xB8) until one handles it
	TXUIElement* pParent = TSTATICCAST( TXUIElement, m_pParent );

	while ( pParent != TNULL )
	{
		TBOOL bHandled = pParent->OnNotifyKillFocus( this );
		pParent        = TSTATICCAST( TXUIElement, pParent->GetParent() );

		if ( pParent == TNULL || !pParent->GetFlag( FLAGS_XUIELEMENT ) || bHandled )
			break;
	}
}

// $deBlob: FUNCTION 006e2540
TBOOL TXUICheckBox::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton == XUIButton_Accept && IsEnabled() == TTRUE )
	{
		m_iPressController = TINT8( a_iController );
		return Press();
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006e2580
TBOOL TXUICheckBox::Press()
{
	if ( !IsEnabled() )
		return TFALSE;

	SetChecked( m_bChecked == TFALSE );
	return TTRUE;
}

// $deBlob: FUNCTION 006e25b0
TBOOL TXUICheckBox::EndPress()
{
	SetFocus( TTRUE, TFALSE, TTRUE );
	return TXUIControl::EndPress();
}

TOSHI_NAMESPACE_END

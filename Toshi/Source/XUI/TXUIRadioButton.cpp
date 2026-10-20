#include "ToshiPCH.h"
#include "TXUIRadioButton.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiRadioButton" type info
const TXUITypeInfo XURXUIRadioButtonData::sm_sTypeInfo = { L"XuiRadioButton", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIRadioButtonData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d0c40
XURXUIRadioButtonData::XURXUIRadioButtonData()
{
	m_uiPressKey = 0;
	m_pClass     = TClass::Find( "TXUIRadioButton", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006d0c70
TBOOL XURXUIRadioButtonData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006d0cd0
TUINT32 XURXUIRadioButtonData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006d0d40
TBOOL XURXUIRadioButtonData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d0da0
TBOOL XURXUIRadioButtonData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "PressKey", -1 ) == 0 )
	{
		a_rPropIndex = Prop_PressKey;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006d0df0
TUINT32 XURXUIRadioButtonData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return 4;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d0e50
TBOOL XURXUIRadioButtonData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d0eb0
TBOOL XURXUIRadioButtonData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006d0f10
TBOOL XURXUIRadioButtonData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

// $deBlob: FUNCTION 006d0c30
const TXUITypeInfo* XURXUIRadioButtonData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIRadioButton
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d0f70
// $deBlob: FUNCTION 006d0f80
// $deBlob: FUNCTION 006d0fd0
// $deBlob: FUNCTION 006d1020
// $deBlob: FUNCTION 006d1030
// $deBlob: FUNCTION 0053f0a0
// $deBlob: FUNCTION 00859130
TXUI_DEFINE_CLASS( TXUIRadioButton );

// $deBlob: FUNCTION 006d1040
TXUIRadioButton::TXUIRadioButton()
{
	m_uiControlFlags |= ControlFlags_Checkable;
	m_bChecked   = TFALSE;
	m_uiPressKey = 0;
}

// $deBlob: FUNCTION 006d1080
// $deBlob: FUNCTION 006d1370
// $deBlob: FUNCTION 006d135b
TXUIRadioButton::~TXUIRadioButton()
{
}

// $deBlob: FUNCTION 006d10a0
TBOOL TXUIRadioButton::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, a_bCreateChildren );
	return TTRUE;
}

// $deBlob: FUNCTION 006d10c0
void TXUIRadioButton::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	TASSERT( a_uiPropIndex < XURXUIRadioButtonData::Prop_NUMOF );

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

// $deBlob: FUNCTION 006d11e0
void TXUIRadioButton::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006d13d0
TBOOL TXUIRadioButton::IsChecked()
{
	return m_bChecked;
}

// $deBlob: FUNCTION 006d1230
TBOOL TXUIRadioButton::Press()
{
	if ( GetTimelineState() == TimelineState_Press || IsEnabled() != TTRUE )
		return TFALSE;

	TXUI::GetSingleton()->SetInputLocked( TTRUE );
	Toggle( TTRUE );

	return TTRUE;
}

// $deBlob: FUNCTION 006d1270
void TXUIRadioButton::Toggle( TBOOL a_bChecked )
{
	if ( m_bChecked == a_bChecked || !IsEnabled() )
		return;

	m_bChecked = a_bChecked;

	TINT iFrame;

	if ( GetTimelineState() == TimelineState_Focus )
		iFrame = ( a_bChecked == TTRUE ) ? 0x18 : 5;
	else
		iFrame = ( a_bChecked == TTRUE ) ? 0x16 : 1;

	PlayAnimation( iFrame, iFrame );

	if ( m_bChecked )
	{
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
}

TOSHI_NAMESPACE_END

#include "ToshiPCH.h"
#include "TXUIButton.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiButton" type info
const TXUITypeInfo XURXUIButtonData::sm_sTypeInfo = { L"XuiButton", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIButtonData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006cf900
XURXUIButtonData::XURXUIButtonData()
{
	m_uiPressKey = 0;
	m_pClass     = TClass::Find( "TXUIButton", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006cf930
TBOOL XURXUIButtonData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006cf990
TUINT32 XURXUIButtonData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006cfa00
TBOOL XURXUIButtonData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006cfa60
TBOOL XURXUIButtonData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "PressKey", -1 ) == 0 )
	{
		a_rPropIndex = Prop_PressKey;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006cfab0
TUINT32 XURXUIButtonData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return 4;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006cfb10
TBOOL XURXUIButtonData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006cfb70
TBOOL XURXUIButtonData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006cfbd0
TBOOL XURXUIButtonData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

// $deBlob: FUNCTION 006cf8f0
const TXUITypeInfo* XURXUIButtonData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIButton
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006cfc30
// $deBlob: FUNCTION 006cfc40
// $deBlob: FUNCTION 006cfc80
// $deBlob: FUNCTION 006cfcc0
// $deBlob: FUNCTION 006cfcd0
// $deBlob: FUNCTION 0053efa0
// $deBlob: FUNCTION 008590d0
TXUI_DEFINE_CLASS( TXUIButton );

// $deBlob: FUNCTION 006cfce0
TXUIButton::TXUIButton()
{
	m_uiPressKey = 0;
}

// $deBlob: FUNCTION 006cfd10
// $deBlob: FUNCTION 006cffc0
// $deBlob: FUNCTION 006cffab
TXUIButton::~TXUIButton()
{
}

// $deBlob: FUNCTION 006cfd30
TBOOL TXUIButton::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, a_bCreateChildren );
	return TTRUE;
}

// $deBlob: FUNCTION 006cfd50
TBOOL TXUIButton::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( m_iPressController == -1 && a_eButton == XUIButton_Accept )
	{
		TASSERT( a_iController < 0x7F );
		m_iPressController = TINT8( a_iController );
		return Press();
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006cfdc0
void TXUIButton::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	TASSERT( a_uiPropIndex < XURXUIButtonData::Prop_NUMOF );

	// Prop_PressKey
	a_fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

	if ( a_fScalar == 0.0f )
	{
		m_uiPressKey = a_uiValue1;
		return;
	}

	TASSERT( a_fScalar == 1.0f );
	m_uiPressKey = a_uiValue2;
}

// $deBlob: FUNCTION 006cfee0
void TXUIButton::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006cff30
TBOOL TXUIButton::Press()
{
	if ( GetTimelineState() != TimelineState_Press && IsEnabled() == TTRUE )
	{
		TXUI::GetSingleton()->SetInputLocked( TTRUE );
		SetXUIStateBits( TimelineState_Press, 0xF, XUIState_TimelineStateShift );
		PlayVisual( TFALSE, TFALSE, TFALSE );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006cff80
TBOOL TXUIButton::EndPress()
{
	SetFocus( TTRUE, TFALSE, TTRUE );
	return TXUIControl::EndPress();
}

TOSHI_NAMESPACE_END

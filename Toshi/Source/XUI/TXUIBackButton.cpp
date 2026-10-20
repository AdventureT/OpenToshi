#include "ToshiPCH.h"
#include "TXUIBackButton.h"
#include "TXUI.h"
#include "TXUIScene.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiBackButton" type info
const TXUITypeInfo XURXUIBackButtonData::sm_sTypeInfo = { L"XuiBackButton", &XURXUIButtonData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIBackButtonData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d0030
XURXUIBackButtonData::XURXUIBackButtonData()
{
	m_pClass = TClass::Find( "TXUIBackButton", &TGetClass( TXUIButton ) );
}

// $deBlob: FUNCTION 006d0060
TBOOL XURXUIBackButtonData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIButtonData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006d0090
TUINT32 XURXUIBackButtonData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIButtonData::CalcPileSize( a_rResource, a_rData );
	a_rData += 1;
	return uiSize;
}

// $deBlob: FUNCTION 006d00b0
TBOOL XURXUIBackButtonData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIButtonData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006d00d0
TBOOL XURXUIBackButtonData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	a_rObjectIndex++;
	return XURXUIButtonData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006d00f0
TUINT32 XURXUIBackButtonData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIButtonData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );

	return 4;
}

// $deBlob: FUNCTION 006d0110
TBOOL XURXUIBackButtonData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIButtonData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006d0130
TBOOL XURXUIBackButtonData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIButtonData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// NOTE: no properties: only the property count, TFALSE if it isn't 0
// $deBlob: FUNCTION 006d0150
TBOOL XURXUIBackButtonData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	return uiNumProps == 0 ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006d0020
const TXUITypeInfo* XURXUIBackButtonData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIBackButton
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d0170
// $deBlob: FUNCTION 006d0180
// $deBlob: FUNCTION 006d01c0
// $deBlob: FUNCTION 006d01f0
// $deBlob: FUNCTION 006d0200
// $deBlob: FUNCTION 0053f000
// $deBlob: FUNCTION 008590f0
TXUI_DEFINE_CLASS( TXUIBackButton );

// $deBlob: FUNCTION 006d0210
TXUIBackButton::TXUIBackButton()
{
}

// $deBlob: FUNCTION 006d0230
// $deBlob: FUNCTION 006d03a0
// $deBlob: FUNCTION 006d038b
TXUIBackButton::~TXUIBackButton()
{
}

// $deBlob: FUNCTION 006d0250
TBOOL TXUIBackButton::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIButton::Create( a_rResource, a_pData, a_bCreateChildren );
	return TTRUE;
}

// $deBlob: FUNCTION 006d0270
void TXUIBackButton::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIButton::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006d02a0
void TXUIBackButton::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIButton::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006d02e0
TBOOL TXUIBackButton::EndPress()
{
	TXUI::GetSingleton()->TransitionBack( FindParentScene() );
	TXUIButton::EndPress();
	SetFocus( TFALSE, TFALSE, TFALSE );

	return TTRUE;
}

// $deBlob: FUNCTION 006d0310
TBOOL TXUIBackButton::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_iController == TXUI::GetSingleton()->GetDefaultController() &&
	     ( a_eButton == XUIButton_Back || ( GetTimelineState() == TimelineState_Focus && a_eButton == XUIButton_Accept ) ) )
	{
		// Nothing to go back to
		if ( FindParentScene()->GetPrevScene() == TNULL )
			return TFALSE;

		return Press();
	}

	return TXUIButton::HandleButton( a_eButton, a_iController );
}

TOSHI_NAMESPACE_END

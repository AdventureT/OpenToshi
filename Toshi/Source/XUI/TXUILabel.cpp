#include "ToshiPCH.h"
#include "TXUILabel.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiLabel" type info
const TXUITypeInfo XURXUILabelData::sm_sTypeInfo = { L"XuiLabel", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUILabelData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d3a90
XURXUILabelData::XURXUILabelData()
{
	m_pClass = TClass::Find( "TXUILabel", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006d3ac0
TBOOL XURXUILabelData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006d3af0
TUINT32 XURXUILabelData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006d3b20
TBOOL XURXUILabelData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006d3b40
TBOOL XURXUILabelData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006d3b60
TUINT32 XURXUILabelData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );

	return 4;
}

// $deBlob: FUNCTION 006d3b80
TBOOL XURXUILabelData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006d3ba0
TBOOL XURXUILabelData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// NOTE: retail skips the label's property data (the mask and one 4 byte property) without reading it
// $deBlob: FUNCTION 006d3bc0
TBOOL XURXUILabelData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	if ( uiNumProps != 0 )
		a_rData += 4;

	return TTRUE;
}

// $deBlob: FUNCTION 006d3a80
const TXUITypeInfo* XURXUILabelData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUILabel
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006d3be0
// $deBlob: FUNCTION 006d3bf0
// $deBlob: FUNCTION 006d3c30
// $deBlob: FUNCTION 006d3c60
// $deBlob: FUNCTION 006d3c70
// $deBlob: FUNCTION 0053f140
// $deBlob: FUNCTION 00859170
TXUI_DEFINE_CLASS( TXUILabel );

// $deBlob: FUNCTION 006d3c80
void TXUILabel::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006d3cb0
void TXUILabel::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

TOSHI_NAMESPACE_END

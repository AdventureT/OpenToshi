#include "ToshiPCH.h"
#include "TXUIHTMLControl.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiHtmlControl" type info
const TXUITypeInfo XURXUIHTMLControlData::sm_sTypeInfo = { L"XuiHtmlControl", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIHTMLControlData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006eacc0
const TXUITypeInfo* XURXUIHTMLControlData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// $deBlob: FUNCTION 006eacd0
XURXUIHTMLControlData::XURXUIHTMLControlData()
{
	m_uiUnknown44 = 0;
	m_pClass      = TClass::Find( "TXUIHTMLControl", &TGetClass( TXUIControl ) );
}

// NOTE: LoadData is inlined
// $deBlob: FUNCTION 006ead00
TBOOL XURXUIHTMLControlData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// NOTE: LoadData is inlined
// $deBlob: FUNCTION 006ead60
TUINT32 XURXUIHTMLControlData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006eadd0
TBOOL XURXUIHTMLControlData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006eadf0
TBOOL XURXUIHTMLControlData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006eae10
TUINT32 XURXUIHTMLControlData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );

	return 4;
}

// $deBlob: FUNCTION 006eae30
TBOOL XURXUIHTMLControlData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006eae50
TBOOL XURXUIHTMLControlData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006eae70
TBOOL XURXUIHTMLControlData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( 0 ) )
		{
			m_uiUnknown44 = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// TXUIHTMLControl
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006eaed0
// $deBlob: FUNCTION 006eaee0
// $deBlob: FUNCTION 006eaf20
// $deBlob: FUNCTION 006eaf50
// $deBlob: FUNCTION 006eaf60
// $deBlob: FUNCTION 0053f8a0
// $deBlob: FUNCTION 00859460
TXUI_DEFINE_CLASS( TXUIHTMLControl );

// $deBlob: FUNCTION 006eaf70
TXUIHTMLControl::TXUIHTMLControl()
{
}

// $deBlob: FUNCTION 006eaf90
// $deBlob: FUNCTION 006eb070
// $deBlob: FUNCTION 006eb05b
TXUIHTMLControl::~TXUIHTMLControl()
{
}

// $deBlob: FUNCTION 006eafb0
TBOOL TXUIHTMLControl::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, a_bCreateChildren );
	return TTRUE;
}

// $deBlob: FUNCTION 006eb020
void TXUIHTMLControl::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// NOTE: TXUIControl's float UpdateTimelineProp is inlined
// $deBlob: FUNCTION 006eafd0
void TXUIHTMLControl::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

TOSHI_NAMESPACE_END

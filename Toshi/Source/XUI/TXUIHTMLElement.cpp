#include "ToshiPCH.h"
#include "TXUIHTMLElement.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiHtmlElement" type info
const TXUITypeInfo XURXUIHTMLElementData::sm_sTypeInfo = { L"XuiHtmlElement", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIHTMLElementData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006ea770
const TXUITypeInfo* XURXUIHTMLElementData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// $deBlob: FUNCTION 006ea780
XURXUIHTMLElementData::XURXUIHTMLElementData()
{
	m_uiText = 0;
	m_pClass = TClass::Find( "TXUIHTMLElement", &TGetClass( TXUIElement ) );
}

// NOTE: LoadData is inlined
// $deBlob: FUNCTION 006ea7c0
TBOOL XURXUIHTMLElementData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// NOTE: LoadData is inlined
// $deBlob: FUNCTION 006ea810
TUINT32 XURXUIHTMLElementData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006ea870
TBOOL XURXUIHTMLElementData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex == Prop_Text ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ea8d0
TBOOL XURXUIHTMLElementData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "Text", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Text;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006ea920
TUINT32 XURXUIHTMLElementData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex != Prop_Text ) ? 4 : 2;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ea990
TBOOL XURXUIHTMLElementData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ea9f0
TBOOL XURXUIHTMLElementData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006eaa50
TBOOL XURXUIHTMLElementData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_Text ) )
		{
			m_uiText = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// TXUIHTMLElement
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006eaaa0
// $deBlob: FUNCTION 006eaab0
// $deBlob: FUNCTION 006eaaf0
// $deBlob: FUNCTION 006eab20
// $deBlob: FUNCTION 006eab30
// $deBlob: FUNCTION 0053f850
// $deBlob: FUNCTION 00859440
TXUI_DEFINE_CLASS( TXUIHTMLElement );

// $deBlob: FUNCTION 006eab40
TXUIHTMLElement::TXUIHTMLElement()
{
}

// $deBlob: FUNCTION 006eab60
// $deBlob: FUNCTION 006eac60
// $deBlob: FUNCTION 006eac49
TXUIHTMLElement::~TXUIHTMLElement()
{
}

// $deBlob: FUNCTION 006eab80
TBOOL TXUIHTMLElement::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, a_bCreateChildren );
	return TTRUE;
}

// $deBlob: FUNCTION 006eabe0
void TXUIHTMLElement::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006eaba0
void TXUIHTMLElement::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

TOSHI_NAMESPACE_END

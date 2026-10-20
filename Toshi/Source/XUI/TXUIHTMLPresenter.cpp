#include "ToshiPCH.h"
#include "TXUIHTMLPresenter.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiHtmlPresenter" type info
const TXUITypeInfo XURXUIHTMLPresenterData::sm_sTypeInfo = { L"XuiHtmlPresenter", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIHTMLPresenterData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006ea200
const TXUITypeInfo* XURXUIHTMLPresenterData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// $deBlob: FUNCTION 006ea210
XURXUIHTMLPresenterData::XURXUIHTMLPresenterData()
{
	m_uiDataAssociation = 0;
	m_pClass            = TClass::Find( "TXUIHTMLPresenter", &TGetClass( TXUIElement ) );
}

// NOTE: LoadData is inlined
// $deBlob: FUNCTION 006ea240
TBOOL XURXUIHTMLPresenterData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// NOTE: LoadData is inlined
// $deBlob: FUNCTION 006ea2a0
TUINT32 XURXUIHTMLPresenterData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006ea310
TBOOL XURXUIHTMLPresenterData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex == Prop_DataAssociation ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ea370
TBOOL XURXUIHTMLPresenterData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "DataAssociation", -1 ) == 0 )
	{
		a_rPropIndex = Prop_DataAssociation;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006ea3c0
TUINT32 XURXUIHTMLPresenterData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return 4;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ea420
TBOOL XURXUIHTMLPresenterData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ea480
TBOOL XURXUIHTMLPresenterData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006ea4e0
TBOOL XURXUIHTMLPresenterData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_DataAssociation ) )
		{
			m_uiDataAssociation = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// TXUIHTMLPresenter
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006ea540
// $deBlob: FUNCTION 006ea550
// $deBlob: FUNCTION 006ea590
// $deBlob: FUNCTION 006ea5c0
// $deBlob: FUNCTION 006ea5d0
// $deBlob: FUNCTION 0053f800
// $deBlob: FUNCTION 00859420
TXUI_DEFINE_CLASS( TXUIHTMLPresenter );

// $deBlob: FUNCTION 006ea5e0
TXUIHTMLPresenter::TXUIHTMLPresenter()
{
}

// $deBlob: FUNCTION 006ea600
// $deBlob: FUNCTION 006ea710
// $deBlob: FUNCTION 006ea6fb
TXUIHTMLPresenter::~TXUIHTMLPresenter()
{
}

// $deBlob: FUNCTION 006ea620
TBOOL TXUIHTMLPresenter::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, a_bCreateChildren );
	return TTRUE;
}

// $deBlob: FUNCTION 006ea640
void TXUIHTMLPresenter::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006ea6b0
void TXUIHTMLPresenter::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

TOSHI_NAMESPACE_END

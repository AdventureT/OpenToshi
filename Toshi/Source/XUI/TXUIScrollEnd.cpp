#include "ToshiPCH.h"
#include "TXUIScrollEnd.h"
#include "TXUIList.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiScrollEnd" type info
const TXUITypeInfo XURXUIScrollEndData::sm_sTypeInfo = { L"XuiScrollEnd", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIScrollEndData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e2650
XURXUIScrollEndData::XURXUIScrollEndData()
{
	m_uiDirection = 0;
	m_pClass      = TClass::Find( "TXUIScrollEnd", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006e2680
TBOOL XURXUIScrollEndData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e26e0
TUINT32 XURXUIScrollEndData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e2750
TBOOL XURXUIScrollEndData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e27b0
TBOOL XURXUIScrollEndData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "Direction", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Direction;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e2800
TUINT32 XURXUIScrollEndData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return 4;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e2860
TBOOL XURXUIScrollEndData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e28c0
TBOOL XURXUIScrollEndData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e2920
TBOOL XURXUIScrollEndData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_Direction ) )
		{
			m_uiDirection = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e2640
const TXUITypeInfo* XURXUIScrollEndData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIScrollEnd
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e2980
// $deBlob: FUNCTION 006e2990
// $deBlob: FUNCTION 006e29d0
// $deBlob: FUNCTION 006e2a10
// $deBlob: FUNCTION 006e2a20
// $deBlob: FUNCTION 0053f480
// $deBlob: FUNCTION 008592b0
TXUI_DEFINE_CLASS( TXUIScrollEnd );

// $deBlob: FUNCTION 006e2a30
TXUIScrollEnd::TXUIScrollEnd()
{
	m_uiDirection = 0;
}

// $deBlob: FUNCTION 006e2a60
// $deBlob: FUNCTION 006e2c30
// $deBlob: FUNCTION 006e2c1b
TXUIScrollEnd::~TXUIScrollEnd()
{
}

// $deBlob: FUNCTION 006e2a80
TBOOL TXUIScrollEnd::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, a_bCreateChildren );
	m_uiDirection = TSTATICCAST( XURXUIScrollEndData, a_pData )->m_uiDirection;

	return TTRUE;
}

// $deBlob: FUNCTION 006e2ab0
void TXUIScrollEnd::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	// NOTE: the direction isn't animated
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006e2b20
void TXUIScrollEnd::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006e2b70
TBOOL TXUIScrollEnd::Back()
{
	TASSERT( GetParent()->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) );
	TXUIElement* pParent = TSTATICCAST( TXUIElement, GetParent() );
	TASSERT( pParent->IsA( &TGetClass( TXUIList ) ) );

	TSTATICCAST( TXUIList, pParent )->UpdateScrollEnds( m_uiDirection, TFALSE );
	return TTRUE;
}

TOSHI_NAMESPACE_END

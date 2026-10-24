#include "ToshiPCH.h"
#include "TXUICaret.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiCaret" type info
const TXUITypeInfo XURXUICaretData::sm_sTypeInfo = { L"XuiCaret", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUICaretData
//-----------------------------------------------------------------------------

// NOTE: retail doesn't set the TClass: it stays the one of XURXUIControlData ("TXUIControl")
// $deBlob: FUNCTION 006d9c60
XURXUICaretData::XURXUICaretData()
{
}

// $deBlob: FUNCTION 006e9f40
TBOOL XURXUICaretData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e9f70
TUINT32 XURXUICaretData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	a_rData += 1;
	return uiSize;
}

// $deBlob: FUNCTION 006e9f90
TBOOL XURXUICaretData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006e9fb0
TBOOL XURXUICaretData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e9fd0
TUINT32 XURXUICaretData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );

	return 4;
}

// $deBlob: FUNCTION 006e9ff0
TBOOL XURXUICaretData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// $deBlob: FUNCTION 006ea010
TBOOL XURXUICaretData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex != 0 )
		return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );

	return TFALSE;
}

// NOTE: no properties: only the property count, TFALSE if it isn't 0
// $deBlob: FUNCTION 006ea030
TBOOL XURXUICaretData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	return uiNumProps == 0 ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 006e9f30
const TXUITypeInfo* XURXUICaretData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUICaret
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006ea050
// $deBlob: FUNCTION 006ea060
// $deBlob: FUNCTION 006ea0a0
// $deBlob: FUNCTION 006ea0d0
// $deBlob: FUNCTION 006ea0e0
// $deBlob: FUNCTION 0053f7b0
// $deBlob: FUNCTION 00859400
TXUI_DEFINE_CLASS( TXUICaret );

// $deBlob: FUNCTION 006ea0f0
TXUICaret::TXUICaret()
{
}

// $deBlob: FUNCTION 006ea110
// $deBlob: FUNCTION 006ea1d0
// $deBlob: FUNCTION 006ea1c1
TXUICaret::~TXUICaret()
{
}

// $deBlob: FUNCTION 006ea130
TBOOL TXUICaret::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, TTRUE );
	return TTRUE;
}

// $deBlob: FUNCTION 006ea150
void TXUICaret::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006ea180
void TXUICaret::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

TOSHI_NAMESPACE_END

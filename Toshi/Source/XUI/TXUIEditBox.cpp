#include "ToshiPCH.h"
#include "TXUIEditBox.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiEdit" type info
const TXUITypeInfo XURXUIEditData::sm_sTypeInfo = { L"XuiEdit", &XURXUIControlData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIEditData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e2ca0
XURXUIEditData::XURXUIEditData()
{
	m_pClass = TClass::Find( "TXUIEditBox", &TGetClass( TXUIControl ) );
}

// $deBlob: FUNCTION 006e2cd0
TBOOL XURXUIEditData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIControlData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e2d00
TUINT32 XURXUIEditData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIControlData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e2d30
TBOOL XURXUIEditData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e2da0
TBOOL XURXUIEditData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "TextLimit", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TextLimit;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "AllowedChars", -1 ) == 0 )
	{
		a_rPropIndex = Prop_AllowedChars;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "PasswordChar", -1 ) == 0 )
	{
		a_rPropIndex = Prop_PasswordChar;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "ReadOnly", -1 ) == 0 )
	{
		a_rPropIndex = Prop_ReadOnly;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Multiline", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Multiline;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "SmoothScroll", -1 ) == 0 )
	{
		a_rPropIndex = Prop_SmoothScroll;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIControlData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e2e90
TUINT32 XURXUIEditData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_AllowedChars || a_uiPropIndex == Prop_PasswordChar )
			return 2;

		if ( a_uiPropIndex == Prop_ReadOnly || a_uiPropIndex == Prop_Multiline || a_uiPropIndex == Prop_SmoothScroll )
			return 1;

		return 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e2f20
TBOOL XURXUIEditData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e2f80
TBOOL XURXUIEditData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIControlData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e2fe0
TBOOL XURXUIEditData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_TextLimit ) )
		{
			m_uiTextLimit = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_AllowedChars ) )
		{
			m_uiAllowedChars = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_PasswordChar ) )
		{
			m_uiPasswordChar = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_ReadOnly ) )
		{
			m_uiReadOnly = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_Multiline ) )
		{
			TASSERT( m_uiReadOnly );
			m_uiMultiline = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_SmoothScroll ) )
		{
			TASSERT( m_uiReadOnly && m_uiMultiline );
			m_uiSmoothScroll = *a_rData;
			a_rData += 1;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e2c90
const TXUITypeInfo* XURXUIEditData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIEditBox
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e3110
// $deBlob: FUNCTION 006e3120
// $deBlob: FUNCTION 006e3160
// $deBlob: FUNCTION 006e3190
// $deBlob: FUNCTION 006e31a0
// $deBlob: FUNCTION 0053f4d0
// $deBlob: FUNCTION 008592d0
TXUI_DEFINE_CLASS( TXUIEditBox );

// $deBlob: FUNCTION 006e31b0
TXUIEditBox::TXUIEditBox()
{
}

// $deBlob: FUNCTION 006e31d0
// $deBlob: FUNCTION 006e3330
// $deBlob: FUNCTION 006e331b
TXUIEditBox::~TXUIEditBox()
{
}

// $deBlob: FUNCTION 006e31f0
TBOOL TXUIEditBox::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIControl::Create( a_rResource, a_pData, TTRUE );
	return TTRUE;
}

// $deBlob: FUNCTION 006e3210
void TXUIEditBox::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex == 0 )
	{
		// NOTE: the edit properties aren't animated
		TASSERT( a_uiPropIndex < XURXUIEditData::Prop_NUMOF );
		return;
	}

	TASSERT( a_uiObjectIndex > 0 );
	TXUIControl::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
}

// $deBlob: FUNCTION 006e32c0
void TXUIEditBox::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		a_uiObjectIndex--;

		if ( a_uiObjectIndex != 0 )
			TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
	}
}

TOSHI_NAMESPACE_END

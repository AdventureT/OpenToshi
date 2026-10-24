#include "ToshiPCH.h"
#include "TXUIMsgBoxButton.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiMessageBoxButton" type info
const TXUITypeInfo XURXUIMsgBoxButtonData::sm_sTypeInfo = { L"XuiMessageBoxButton", &XURXUIButtonData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIMsgBoxButtonData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e33a0
XURXUIMsgBoxButtonData::XURXUIMsgBoxButtonData()
{
	m_uiMessageBoxVisual  = 0;
	m_uiMessageBoxTitle   = 0;
	m_uiMessageBoxText    = 0;
	m_uiMessageBoxButtons = 0;
	m_bMessageBoxCenter   = TFALSE;
	m_pClass              = TClass::Find( "TXUIMsgBoxButton", &TGetClass( TXUIButton ) );
}

// $deBlob: FUNCTION 006e33d0
TBOOL XURXUIMsgBoxButtonData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIButtonData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e3400
TUINT32 XURXUIMsgBoxButtonData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIButtonData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e3430
TBOOL XURXUIMsgBoxButtonData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIButtonData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e3490
TBOOL XURXUIMsgBoxButtonData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "MessageBoxCenter", -1 ) == 0 )
	{
		a_rPropIndex = Prop_MessageBoxCenter;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "MessageBoxVisual", -1 ) == 0 )
	{
		a_rPropIndex = Prop_MessageBoxVisual;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "MessageBoxTitle", -1 ) == 0 )
	{
		a_rPropIndex = Prop_MessageBoxTitle;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "MessageBoxText", -1 ) == 0 )
	{
		a_rPropIndex = Prop_MessageBoxText;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "MessageBoxButtons", -1 ) == 0 )
	{
		a_rPropIndex = Prop_MessageBoxButtons;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIButtonData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e3560
TUINT32 XURXUIMsgBoxButtonData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_MessageBoxVisual || a_uiPropIndex == Prop_MessageBoxTitle || a_uiPropIndex == Prop_MessageBoxText ||
		     a_uiPropIndex == Prop_MessageBoxButtons )
			return 2;

		return ( a_uiPropIndex == Prop_MessageBoxCenter ) ? 1 : 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIButtonData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e35f0
TBOOL XURXUIMsgBoxButtonData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIButtonData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e3650
TBOOL XURXUIMsgBoxButtonData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIButtonData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e36b0
TBOOL XURXUIMsgBoxButtonData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_MessageBoxCenter ) )
		{
			m_bMessageBoxCenter = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_MessageBoxVisual ) )
		{
			m_uiMessageBoxVisual = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_MessageBoxTitle ) )
		{
			m_uiMessageBoxTitle = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_MessageBoxText ) )
		{
			m_uiMessageBoxText = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_MessageBoxButtons ) )
		{
			m_uiMessageBoxButtons = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e3390
const TXUITypeInfo* XURXUIMsgBoxButtonData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIMsgBoxButton
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e3760
// $deBlob: FUNCTION 006e3770
// $deBlob: FUNCTION 006e37b0
// $deBlob: FUNCTION 006e37e0
// $deBlob: FUNCTION 006e37f0
// $deBlob: FUNCTION 0053f520
// $deBlob: FUNCTION 008592f0
TXUI_DEFINE_CLASS( TXUIMsgBoxButton );

// $deBlob: FUNCTION 006e3800
TXUIMsgBoxButton::TXUIMsgBoxButton()
{
}

// $deBlob: FUNCTION 006e3820
// $deBlob: FUNCTION 006e3930
// $deBlob: FUNCTION 006e391b
TXUIMsgBoxButton::~TXUIMsgBoxButton()
{
}

// $deBlob: FUNCTION 006e3840
TBOOL TXUIMsgBoxButton::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIButton::Create( a_rResource, a_pData, TTRUE );
	return TTRUE;
}

// $deBlob: FUNCTION 006e3860
void TXUIMsgBoxButton::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	// NOTE: the message box properties aren't animated
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIButton::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
	}
}

// $deBlob: FUNCTION 006e38d0
void TXUIMsgBoxButton::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIButton::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

TOSHI_NAMESPACE_END

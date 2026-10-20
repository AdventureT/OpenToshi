#include "ToshiPCH.h"
#include "TXUITextPresenter.h"
#include "TXUIText.h"
#include "TXUIScene.h"
#include "TXUI.h"
#include "Toshi/TString8.h"
#include "GUI/T2GUIFont.h"
#include "GUI/T2GUIFontManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiTextPresenter" type info
const TXUITypeInfo XURXUITextPresenterData::sm_sTypeInfo = { L"XuiTextPresenter", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUITextPresenterData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006dfbc0
XURXUITextPresenterData::XURXUITextPresenterData()
{
	m_uiDataAssociation  = 0;
	m_uiTextColour       = 0xFF000000;
	m_uiDropShadowColour = 0x80000000;
	m_uiPointSize        = XURXUITextData::sm_uiDefaultFontSize;
	m_uiFont             = 0;
	m_uiTextStyle        = XURXUITextData::TextStyle_JustifyLeft;
	m_iLineSpacingAdjust = 0;

	m_pClass = TClass::Find( "TXUITextPresenter", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006dfc20
TBOOL XURXUITextPresenterData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006dfc50
TUINT32 XURXUITextPresenterData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006dfc80
TBOOL XURXUITextPresenterData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006dfcf0
TBOOL XURXUITextPresenterData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "TextColor", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TextColor;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "DropShadowColor", -1 ) == 0 )
	{
		a_rPropIndex = Prop_DropShadowColor;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "PointSize", -1 ) == 0 )
	{
		a_rPropIndex = Prop_PointSize;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Font", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Font;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "TextStyle", -1 ) == 0 )
	{
		a_rPropIndex = Prop_TextStyle;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "LineSpacingAdjust", -1 ) == 0 )
	{
		a_rPropIndex = Prop_LineSpacingAdjust;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "DataAssociation", -1 ) == 0 )
	{
		a_rPropIndex = Prop_DataAssociation;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006dfe00
TUINT32 XURXUITextPresenterData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex != Prop_Font ) ? 4 : 2;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006dfe70
TBOOL XURXUITextPresenterData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006dfed0
TBOOL XURXUITextPresenterData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_TextColor )
			return TTRUE;

		return a_uiPropIndex == Prop_DropShadowColor ? TTRUE : TFALSE;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006dff40
TBOOL XURXUITextPresenterData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_TextColor ) )
		{
			m_uiTextColour = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_DropShadowColor ) )
		{
			m_uiDropShadowColour = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_PointSize ) )
		{
			m_uiPointSize = TUINT32( PARSEFLOAT_BIG( a_rData ) );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Font ) )
		{
			m_uiFont = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_TextStyle ) )
		{
			m_uiTextStyle = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_LineSpacingAdjust ) )
		{
			m_iLineSpacingAdjust = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_DataAssociation ) )
		{
			m_uiDataAssociation = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006dfbb0
const TXUITypeInfo* XURXUITextPresenterData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUITextPresenter
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e00a0
// $deBlob: FUNCTION 006e00b0
// $deBlob: FUNCTION 006e00e0
// $deBlob: FUNCTION 006e0100
// $deBlob: FUNCTION 006e0110
// $deBlob: FUNCTION 0053f340
// $deBlob: FUNCTION 00859230
TXUI_DEFINE_CLASS( TXUITextPresenter );

// $deBlob: FUNCTION 006e0120
TXUITextPresenter::TXUITextPresenter()
{
	m_pFont              = TNULL;
	m_wszText            = TNULL;
	m_pColourIndices     = TNULL;
	m_pColours           = TNULL;
	m_iJustify           = 0;
	m_uiDropShadowColour = 0x80000000;
	m_iLineSpacingAdjust = 0;
	m_uiDataAssociation  = 0;
	m_iParam             = -1;
	m_bDropShadow        = TFALSE;
	m_bWordWrap          = TFALSE;
	m_bVerticalCentre    = TFALSE;
}

// $deBlob: FUNCTION 006e0190
// $deBlob: FUNCTION 006e0980
// $deBlob: FUNCTION 006e096b
TXUITextPresenter::~TXUITextPresenter()
{
	T2GUIFontManager::ReleaseFont( m_pFont );
}

// $deBlob: FUNCTION 006e01b0
TBOOL TXUITextPresenter::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, a_bCreateChildren );
	XURXUITextPresenterData* pTextPresenterData = TSTATICCAST( XURXUITextPresenterData, a_pData );

	// The font is "<font name>_<point size>", '-' and ' ' replaced by '_'
	TString8 sFontName;

	if ( pTextPresenterData->m_uiFont != 0 )
	{
		sFontName = a_rResource.GetString( pTextPresenterData->m_uiFont );

		for ( TINT i = 0; i < sFontName.Length(); i++ )
		{
			if ( sFontName[ i ] == '-' || sFontName[ i ] == ' ' )
				sFontName[ i ] = '_';
		}
	}
	else
	{
		sFontName = TXUI::GetSingleton()->GetDefaultFontName();
	}

	TString8 sPointSize;
	TASSERT( pTextPresenterData->m_uiPointSize < 100 );
	sPointSize.Format( "_%d", pTextPresenterData->m_uiPointSize );
	sFontName.Concat( sPointSize, -1 );

	m_pFont = T2GUIFontManager::GetFont( sFontName.GetString() );

	if ( m_pFont == TNULL )
		m_pFont = TXUI::GetSingleton()->GetDefaultFont();

	SetColour( pTextPresenterData->m_uiTextColour );
	m_uiDropShadowColour = pTextPresenterData->m_uiDropShadowColour;
	m_iJustify           = TXUIText::GetJustify( pTextPresenterData->m_uiTextStyle );
	m_iLineSpacingAdjust = pTextPresenterData->m_iLineSpacingAdjust;
	m_bDropShadow        = ( pTextPresenterData->m_uiTextStyle & XURXUITextData::TextStyle_DropShadow ) != 0;
	m_bWordWrap          = ( pTextPresenterData->m_uiTextStyle & XURXUITextData::TextStyle_NoWordWrap ) == 0;
	m_uiDataAssociation  = pTextPresenterData->m_uiDataAssociation;
	m_bVerticalCentre    = ( pTextPresenterData->m_uiTextStyle & XURXUITextData::TextStyle_VerticalCentre ) != 0;

	return TTRUE;
}

// $deBlob: FUNCTION 006e03a0
void TXUITextPresenter::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	switch ( a_uiPropIndex )
	{
		case XURXUITextPresenterData::Prop_TextColor:
			SetColour( InterpColour( a_uiValue1, a_uiValue2, a_fScalar ) );
			return;
		case XURXUITextPresenterData::Prop_DropShadowColor:
			m_uiDropShadowColour = InterpColour( a_uiValue1, a_uiValue2, a_fScalar );
			return;
		case XURXUITextPresenterData::Prop_LineSpacingAdjust:
			// NOTE: retail rounds with fistp (round half to even)
			m_iLineSpacingAdjust = TMath::Round( TMath::LERP( TFLOAT( a_uiValue1 ), TFLOAT( a_uiValue2 ), a_fScalar ) );
			return;
		case XURXUITextPresenterData::Prop_DataAssociation:
			if ( TFLOAT( TMath::FloorToInt( a_fScalar ) ) == 0.0f )
			{
				m_uiDataAssociation = a_uiValue1;
				return;
			}

			TASSERT( a_fScalar == 1.0f );
			m_uiDataAssociation = a_uiValue2;
			return;
	}
}

// $deBlob: FUNCTION 006e0590
void TXUITextPresenter::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006e05d0
void TXUITextPresenter::Render()
{
	if ( m_vScale.GetX() == 0.0f || m_vScale.GetY() == 0.0f )
		return;

	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );

	TFLOAT fClipExtra;

	if ( TXUIText::IsClipExtended() )
	{
		fClipExtra = m_pFont->GetHeight() * 0.5f;
	}
	else
	{
		fClipExtra = 0.0f;
	}

	PushClip( fWidth, fHeight + fClipExtra );

	TFLOAT        fX    = fWidth * -0.5f;
	TFLOAT        fY    = fHeight * -0.5f;
	const TWCHAR* pText = TNULL;
	TWCHAR        wszText[ 4096 ];

	if ( m_wszText != TNULL )
	{
		T2String16::Copy( wszText, m_wszText );
		TXUIScene::RemapPlatformGlyphs( wszText );
		pText = wszText;
	}

	if ( m_bVerticalCentre == TTRUE )
	{
		TFLOAT fTextHeight = m_pFont->GetTextBoxHeight( pText, fWidth, 1.0f, m_bWordWrap, m_iLineSpacingAdjust, TNULL );

		if ( fHeight > fTextHeight )
			fY += ( fHeight - fTextHeight ) * 0.5f;
	}

	if ( m_bDropShadow == TTRUE )
	{
		TFLOAT  fAlpha             = GetAlpha();
		TUINT32 uiDropShadowColour = ( m_uiDropShadowColour & 0xFFFFFF ) | ( TUINT32( ( TFLOAT( m_uiDropShadowColour >> 24 ) / 255.0f ) * fAlpha * 255.0f ) << 24 );

		m_pFont->DrawTextBox( pText, fX + 1.0f, fY + 1.0f, fWidth, fHeight, uiDropShadowColour, 1.0f, m_iJustify, m_iParam, m_bWordWrap, m_iLineSpacingAdjust, TTRUE, TNULL, TNULL, TNULL );
	}

	if ( m_pColours == TNULL )
	{
		m_pFont->DrawTextBox( pText, fX, fY, fWidth, fHeight, m_uiColour, 1.0f, m_iJustify, m_iParam, m_bWordWrap, m_iLineSpacingAdjust, TTRUE, TNULL, TNULL, TNULL );
	}
	else
	{
		TUINT32 uiOldColour = *m_pColours;
		*m_pColours         = m_uiColour | 0xFF000000;

		TFLOAT fAlpha = GetAlpha();
		m_pFont->DrawTextBoxMultiColoured( pText, fX, fY, fWidth, fHeight, TSTATICCAST( const TUINT8, m_pColourIndices ), m_pColours, 1.0f, m_iJustify, m_iParam, m_bWordWrap, m_iLineSpacingAdjust, TTRUE, &fAlpha );

		*m_pColours = uiOldColour;
	}

	T2GUIElement::Render();
	PopClip();
}

TOSHI_NAMESPACE_END

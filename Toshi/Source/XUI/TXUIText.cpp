#include "ToshiPCH.h"
#include "TXUIText.h"
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

// "XuiText" type info
const TXUITypeInfo XURXUITextData::sm_sTypeInfo = { L"XuiText", &XURXUIElementData::sm_sTypeInfo };

const TUINT32 XURXUITextData::sm_uiDefaultFontSize = 14;

TBOOL TXUIText::ms_bExtendClip = TFALSE;

//-----------------------------------------------------------------------------
// XURXUITextData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006c2cc0
XURXUITextData::XURXUITextData()
{
	m_uiText             = 0;
	m_uiTextColour       = 0xFF000000;
	m_uiDropShadowColour = 0x80000000;
	m_uiPointSize        = sm_uiDefaultFontSize;
	m_uiFont             = 0;
	m_uiTextStyle        = TextStyle_JustifyLeft | TextStyle_NoWordWrap;
	m_iLineSpacingAdjust = 0;

	m_pClass = TClass::Find( "TXUIText", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006c2d20
TBOOL XURXUITextData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006c2d50
TUINT32 XURXUITextData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006c2d80
TBOOL XURXUITextData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006c2df0
TBOOL XURXUITextData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "Text", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Text;
		return TTRUE;
	}

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

	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006c2f00
TUINT32 XURXUITextData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex != Prop_Text && a_uiPropIndex != Prop_Font )
			return 4;

		return 2;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006c2f80
TBOOL XURXUITextData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006c2fe0
TBOOL XURXUITextData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
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

// $deBlob: FUNCTION 006c3060
TBOOL XURXUITextData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006c2cb0
const TXUITypeInfo* XURXUITextData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

//-----------------------------------------------------------------------------
// TXUIText
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006c31b0
// $deBlob: FUNCTION 006c31c0
// $deBlob: FUNCTION 006c31f0
// $deBlob: FUNCTION 006c3210
// $deBlob: FUNCTION 006c3220
// $deBlob: FUNCTION 0053ee50
// $deBlob: FUNCTION 00859050
TXUI_DEFINE_CLASS( TXUIText );

// $deBlob: FUNCTION 006c3230
TXUIText::TXUIText()
{
	m_pFont              = TNULL;
	m_wszText            = TNULL;
	m_pColourIndices     = TNULL;
	m_pColours           = TNULL;
	m_iJustify           = 0;
	m_uiDropShadowColour = 0x80000000;
	m_iLineSpacingAdjust = 0;
	m_bDropShadow        = TFALSE;
	m_bWordWrap          = TFALSE;
	m_bVerticalCentre    = TFALSE;
	m_bReleaseFont       = TTRUE;
}

// $deBlob: FUNCTION 006c3290
// $deBlob: FUNCTION 006c3a10
// $deBlob: FUNCTION 006c39fd
TXUIText::~TXUIText()
{
	if ( m_bReleaseFont )
		T2GUIFontManager::ReleaseFont( m_pFont );
}

// $deBlob: FUNCTION 006c32c0
TBOOL TXUIText::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, a_bCreateChildren );
	XURXUITextData* pData = TSTATICCAST( XURXUITextData, a_pData );

	// The font is "<font name>_<point size>", '-' and ' ' replaced by '_'
	TString8 sFontName;

	if ( pData->m_uiFont != 0 )
	{
		sFontName = a_rResource.GetString( pData->m_uiFont );

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
	sPointSize.Format( "_%d", pData->m_uiPointSize );
	sFontName.Concat( sPointSize, -1 );

	TASSERT( TFALSE == sFontName.IsEmpty() );
	m_pFont = T2GUIFontManager::GetFont( sFontName.GetString() );

	if ( m_pFont == TNULL )
		m_pFont = TXUI::GetSingleton()->GetDefaultFont();

	if ( pData->m_uiText != 0 )
	{
		a_rResource.PushID( L"Text" );
		m_wszText = a_rResource.LookupStringTable();

		if ( m_wszText == TNULL )
			m_wszText = a_rResource.GetString( pData->m_uiText );

		a_rResource.PopID();
	}

	SetColour( pData->m_uiTextColour );
	m_uiDropShadowColour = pData->m_uiDropShadowColour;
	m_iJustify           = GetJustify( pData->m_uiTextStyle );
	m_iLineSpacingAdjust = pData->m_iLineSpacingAdjust;
	m_bDropShadow        = ( pData->m_uiTextStyle & XURXUITextData::TextStyle_DropShadow ) != 0;
	m_bWordWrap          = ( pData->m_uiTextStyle & XURXUITextData::TextStyle_NoWordWrap ) == 0;
	m_bVerticalCentre    = ( pData->m_uiTextStyle & XURXUITextData::TextStyle_VerticalCentre ) != 0;

	return TTRUE;
}

// $deBlob: FUNCTION 006c34e0
void TXUIText::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	switch ( a_uiPropIndex )
	{
		case XURXUITextData::Prop_Text:
			if ( TFLOAT( TMath::FloorToInt( a_fScalar ) ) == 0.0f )
			{
				m_wszText = a_pResource->GetString( TUINT16( a_uiValue1 ) );
				return;
			}

			TASSERT( a_fScalar == 1.0f );
			m_wszText = a_pResource->GetString( TUINT16( a_uiValue2 ) );
			return;
		case XURXUITextData::Prop_TextColor:
			SetColour( InterpColour( a_uiValue1, a_uiValue2, a_fScalar ) );
			return;
		case XURXUITextData::Prop_DropShadowColor:
			m_uiDropShadowColour = InterpColour( a_uiValue1, a_uiValue2, a_fScalar );
			return;
		case XURXUITextData::Prop_LineSpacingAdjust:
			// NOTE: retail rounds with fistp (round half to even)
			m_iLineSpacingAdjust = TMath::Round( TMath::LERP( TFLOAT( a_uiValue1 ), TFLOAT( a_uiValue2 ), a_fScalar ) );
			return;
	}
}

// $deBlob: FUNCTION 006c36e0
void TXUIText::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
}

// $deBlob: FUNCTION 006c3720
void TXUIText::Render()
{
	if ( m_vScale.GetX() == 0.0f || m_vScale.GetY() == 0.0f )
		return;

	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );

	TFLOAT fClipExtra;

	if ( ms_bExtendClip )
	{
		fClipExtra = m_pFont->GetHeight() * 0.5f;
	}
	else
	{
		fClipExtra = 0.0f;
	}

	PushClip( fWidth, fHeight + fClipExtra );

	TFLOAT fX = fWidth * -0.5f;
	TFLOAT fY = fHeight * -0.5f;

	if ( m_bVerticalCentre == TTRUE )
	{
		TFLOAT fTextHeight = m_pFont->GetTextBoxHeight( m_wszText, fWidth, 1.0f, m_bWordWrap, m_iLineSpacingAdjust, TNULL );

		if ( fHeight > fTextHeight )
			fY += ( fHeight - fTextHeight ) * 0.5f;
	}

	if ( m_bDropShadow == TTRUE )
	{
		TFLOAT  fAlpha             = GetAlpha();
		TUINT32 uiDropShadowColour = ( m_uiDropShadowColour & 0xFFFFFF ) | ( TUINT32( ( TFLOAT( m_uiDropShadowColour >> 24 ) / 255.0f ) * fAlpha * 255.0f ) << 24 );

		m_pFont->DrawTextBox( m_wszText, fX + 1.0f, fY + 1.0f, fWidth, fHeight, uiDropShadowColour, 1.0f, m_iJustify, -1, m_bWordWrap, m_iLineSpacingAdjust, TTRUE, TNULL, TNULL, TNULL );
	}

	if ( m_pColours != TNULL )
	{
		TUINT32 uiOldColour = *m_pColours;
		*m_pColours         = m_uiColour | 0xFF000000;

		TFLOAT fAlpha = GetAlpha();
		m_pFont->DrawTextBoxMultiColoured( m_wszText, fX, fY, fWidth, fHeight, TSTATICCAST( const TUINT8, m_pColourIndices ), m_pColours, 1.0f, m_iJustify, -1, m_bWordWrap, m_iLineSpacingAdjust, TTRUE, &fAlpha );

		*m_pColours = uiOldColour;
	}
	else
	{
		m_pFont->DrawTextBox( m_wszText, fX, fY, fWidth, fHeight, m_uiColour, 1.0f, m_iJustify, -1, m_bWordWrap, m_iLineSpacingAdjust, TTRUE, TNULL, TNULL, TNULL );
	}

	T2GUIElement::Render();
	PopClip();
}

TOSHI_NAMESPACE_END

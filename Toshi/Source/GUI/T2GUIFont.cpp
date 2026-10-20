#include "ToshiPCH.h"
#include "T2GUIFont.h"
#include "T2GUIFontManager.h"
#include "T2GUI.h"
#include "XUI/TXUIScene.h"

#include "File/TTRB.h"
#include "Toshi/T2ResourceManager.h"

#include <cwctype>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

T2GUIFont::t_IsLanguage T2GUIFont::ms_fnIsJapaneseLanguage = TNULL;
T2GUIFont::t_IsLanguage T2GUIFont::ms_fnIsChineseLanguage  = TNULL;
TBitField<128>          T2GUIFont::s_aBitField;

T2GUIFont::t_CanBreakBetweenChars T2GUIFont::s_CanBreakBetweenChars = CanBreakBetweenChars_Default;

// $deBlob: FUNCTION 006f75b0
T2GUIFont::T2GUIFont()
{
	m_font        = TNULL;
	m_font2       = TNULL;
	m_materials   = TNULL;
	m_bOwnsFont   = TFALSE;
	m_iLineHeight = 0;
}

// $deBlob: FUNCTION 006f75e0
// $deBlob: FUNCTION 006f5b20
T2GUIFont::~T2GUIFont()
{
	Destroy();
}

// $deBlob: FUNCTION 006f75f0
void T2GUIFont::SetRegion()
{
	// NOTE: retail calls SteamManager::IsJapaneseLanguage and IsChineseLanguage (game code) directly
	if ( ms_fnIsJapaneseLanguage && ms_fnIsJapaneseLanguage() )
	{
		s_CanBreakBetweenChars = CanBreakBetweenChars_Japanese;
		return;
	}

	s_CanBreakBetweenChars = CanBreakBetweenChars_Default;

	if ( ms_fnIsChineseLanguage && ms_fnIsChineseLanguage() )
		s_CanBreakBetweenChars = CanBreakBetweenChars_Chinese;
}

// $deBlob: FUNCTION 006f7620
TBOOL T2GUIFont::Create( const TCHAR* a_szSymbolName, TTRB* a_pTRB )
{
	return Create( TSTATICCAST( T2GUIFontDef, a_pTRB->GetSymbolAddress( a_szSymbolName ) ) );
}

// $deBlob: FUNCTION 006f7640
TBOOL T2GUIFont::Create( T2GUIFontDef* a_pFontDef )
{
	m_font = a_pFontDef;

	TUINT uiNumMaterials = a_pFontDef->uiNumMaterials;
	m_materials          = TSTATICCAST( T2GUIMaterial*, TMemalign( 16, uiNumMaterials * sizeof( T2GUIMaterial* ) ) );

	const TCHAR* szTextureName = a_pFontDef->szTextureNames;

	for ( TUINT i = 0; i < uiNumMaterials; i++ )
	{
		m_materials[ i ] = T2GUI::CreateMaterial( szTextureName );
		szTextureName += TStringManager::String8Length( szTextureName ) + 1;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f76c0
TBOOL T2GUIFont::Create( T2GUIFontDef2* a_pFontDef )
{
	m_font2 = a_pFontDef;

	TUINT uiNumMaterials = a_pFontDef->GetDef()->uiNumMaterials;
	m_materials          = TSTATICCAST( T2GUIMaterial*, TMemalign( 16, uiNumMaterials * sizeof( T2GUIMaterial* ) ) );

	const TCHAR* szTextureName = a_pFontDef->GetDef()->szTextureNames;

	for ( TUINT i = 0; i < uiNumMaterials; i++ )
	{
		m_materials[ i ] = T2GUI::CreateMaterial( szTextureName );
		szTextureName += TStringManager::String8Length( szTextureName ) + 1;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f7740
void T2GUIFont::Destroy()
{
	if ( m_bOwnsFont )
	{
		// NOTE: an owned font keeps the T2Resource of its TRB at m_font2 (the ID is its first member)
		operator delete( m_font );
		TINT* pResource = TREINTERPRETCAST( TINT*, m_font2 );
		m_font          = TNULL;

		if ( pResource != TNULL )
		{
			T2ResourceManager::GetSingleton()->DestroyResource( *pResource );
			operator delete( pResource );
		}

		m_font2 = TNULL;
		return;
	}

	if ( m_font == TNULL )
	{
		if ( m_font2 != TNULL )
		{
			TUINT uiNumMaterials = m_font2->GetDef()->uiNumMaterials;

			for ( TUINT i = 0; i < uiNumMaterials; i++ )
				T2GUI::DestroyMaterial( m_materials[ i ] );

			operator delete( m_materials );
			m_font2     = TNULL;
			m_materials = TNULL;
		}

		return;
	}

	TUINT uiNumMaterials = m_font->uiNumMaterials;

	for ( TUINT i = 0; i < uiNumMaterials; i++ )
		T2GUI::DestroyMaterial( m_materials[ i ] );

	operator delete( m_materials );
	m_font      = TNULL;
	m_materials = TNULL;
}

// $deBlob: FUNCTION 006f7820
TFLOAT T2GUIFont::GetHeight() const
{
	if ( m_font != TNULL ) return TFLOAT( m_font->uiHeight );
	if ( m_font2 != TNULL ) return TFLOAT( m_font2->GetDef()->uiHeight );

	return 0.0f;
}

// $deBlob: FUNCTION 006f7860
TFLOAT T2GUIFont::GetLineSpacing() const
{
	if ( m_font != TNULL ) return TFLOAT( m_font->iLineSpacing );
	if ( m_font2 != TNULL ) return TFLOAT( m_font2->GetDef()->iLineSpacing );

	return 0.0f;
}

// $deBlob: FUNCTION 006f78a0
TFLOAT T2GUIFont::GetCharWidth( TWCHAR a_wcChar, TFLOAT a_fScale, t_GetCharWidth a_fnGetCharWidth ) const
{
	if ( !IsCreated() )
		return 0.0f;

	if ( a_fnGetCharWidth != TNULL )
	{
		TFLOAT fWidth = a_fnGetCharWidth( a_wcChar );

		if ( fWidth != -1.0f )
			return fWidth * a_fScale;
	}

	const T2GUIFontGlyph* pGlyph = GetGlyph( a_wcChar );

	if ( pGlyph == TNULL )
		return 0.0f * a_fScale;

	return TFLOAT( pGlyph->iRightPadding + pGlyph->iLeftPadding + pGlyph->uiWidth ) * a_fScale;
}

// $deBlob: FUNCTION 006f7920
TFLOAT T2GUIFont::GetStringWidth( const TWCHAR* a_wszString, TFLOAT a_fScale, t_GetCharWidth a_fnGetCharWidth ) const
{
	if ( a_wszString != TNULL && IsCreated() )
		return GetStringWidth_Imp( a_wszString, wcslen( a_wszString ), a_fScale, a_fnGetCharWidth );

	return 0.0f;
}

// $deBlob: FUNCTION 006f7980
TFLOAT T2GUIFont::GetLongestNonBreakableSubStringWidth( const TWCHAR* a_wszString, TFLOAT a_fScale, t_GetCharWidth a_fnGetCharWidth ) const
{
	const TWCHAR* pWordStart = TNULL;
	TFLOAT        fLongest   = 0.0f;

	while ( TTRUE )
	{
		TWCHAR wcChar = *a_wszString;

		if ( pWordStart == TNULL )
		{
			if ( ( !iswspace( wcChar ) || *a_wszString == CHAR_NOBREAKSPACE ) && *a_wszString != CHAR_IDEOGRAPHICSPACE )
				pWordStart = a_wszString;
		}
		else if ( wcChar == L'\0' || s_CanBreakBetweenChars( a_wszString[ -1 ], wcChar ) )
		{
			const T2GUIFontDef* pDef   = GetFontDef();
			TINT                iCount = TINT( a_wszString - pWordStart );
			TFLOAT              fWidth = 0.0f;

			for ( ; iCount > 0; iCount-- )
			{
				fWidth = TFLOAT( pDef->iCharSpacing ) + GetGlyphWidth( *pWordStart, a_fnGetCharWidth ) + fWidth;
				pWordStart++;
			}

			if ( fLongest < fWidth * a_fScale )
				fLongest = fWidth * a_fScale;

			pWordStart = TNULL;
		}

		if ( *a_wszString == L'\0' )
			return fLongest;

		a_wszString++;
	}
}

// $deBlob: FUNCTION 006f7af0
TFLOAT T2GUIFont::GetTextBoxHeight( const TWCHAR* a_wszString, TFLOAT a_fWidth, TFLOAT a_fScale, TBOOL a_bWordWrap, TUINT a_uiLineSpacing, t_GetCharWidth a_fnGetCharWidth ) const
{
	TFLOAT fHeight = 0.0f;

	if ( a_wszString != TNULL && IsCreated() )
	{
		TFLOAT        fLineHeight = GetLineHeight( a_wszString, a_uiLineSpacing ) * a_fScale;
		const TWCHAR* pString     = a_wszString;
		TFLOAT        fLineWidth;

		do
		{
			GetNextTextBoxLine( pString, a_fWidth, a_fScale, fLineWidth, a_bWordWrap, a_fnGetCharWidth );
			fHeight = fLineHeight + fHeight;
		} while ( *pString != L'\0' );

		return fHeight;
	}

	return 0.0f;
}

// $deBlob: FUNCTION 006f7b90
void T2GUIFont::DrawString( const TWCHAR* a_wszString, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale )
{
	if ( a_wszString != TNULL && *a_wszString != L'\0' && IsCreated() )
		DrawString_Imp( a_wszString, wcslen( a_wszString ), a_fX, a_fY, a_uiColour, a_fScale, TNULL, TNULL, TNULL );
}

// $deBlob: FUNCTION 006f7c00
void T2GUIFont::DrawStringMultiColoured( const TWCHAR* a_wszString, TFLOAT a_fX, TFLOAT a_fY, const TUINT8* a_pColourIndices, const TUINT32* a_pColours, TFLOAT a_fScale )
{
	if ( a_wszString != TNULL && *a_wszString != L'\0' && IsCreated() )
		DrawStringMultiColoured_Imp( a_wszString, wcslen( a_wszString ), a_fX, a_fY, a_pColourIndices, a_pColours, a_fScale, TNULL );
}

// $deBlob: FUNCTION 006f7c70
void T2GUIFont::DrawTextBox( const TWCHAR* a_wszString, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TUINT32 a_uiColour, TFLOAT a_fScale, JUSTIFY a_eJustify, TINT a_iMaxChars, TBOOL a_bWordWrap, TUINT a_uiLineSpacing, TBOOL a_bLastLineFits, t_CharCallback a_fnCharCallback, t_GetCharWidth a_fnGetCharWidth, t_RenderChar a_fnRenderChar )
{
	TFLOAT fStartY = a_fY;

	if ( a_wszString == TNULL || *a_wszString == L'\0' || !IsCreated() )
		return;

	GetFontDef();
	TFLOAT fLineHeight = GetLineHeight( a_wszString, a_uiLineSpacing ) * a_fScale;
	m_iLineHeight      = TINT( fLineHeight );
	m_bFirstLine       = TTRUE;
	m_bMoreLines       = TFALSE;

	const TWCHAR* pString = a_wszString;
	TFLOAT        fLineWidth;
	GetNextTextBoxLine( pString, a_fWidth, a_fScale, fLineWidth, a_bWordWrap, a_fnGetCharWidth );

	if ( *pString != L'\0' )
		m_bMoreLines = TTRUE;

	pString                = a_wszString;
	const TWCHAR* pNextLine = a_wszString;

	if ( !a_bLastLineFits )
		a_fHeight -= fLineHeight;

	while ( TTRUE )
	{
		const TWCHAR* pLine     = pNextLine;
		TINT          iNumChars = GetNextTextBoxLine( pString, a_fWidth, a_fScale, fLineWidth, a_bWordWrap, a_fnGetCharWidth );
		TFLOAT        fX        = a_fX;

		if ( a_eJustify != JUSTIFY_LEFT )
		{
			if ( a_eJustify == JUSTIFY_CENTRE )
				fX = ( a_fWidth - fLineWidth ) * 0.5f + a_fX;
			else if ( a_eJustify == JUSTIFY_RIGHT )
				fX = ( a_fWidth - fLineWidth ) + a_fX;
			else
				TASSERT( 0 );
		}

		TINT iDrawChars = iNumChars;

		if ( a_iMaxChars != -1 && a_iMaxChars <= TINT( pString - a_wszString ) )
		{
			pString         = a_wszString + a_iMaxChars;
			TINT iToMaxChar = TINT( pString - pLine );
			iDrawChars      = TMath::Min( iNumChars, iToMaxChar );
		}

		pNextLine = pString;
		DrawString_Imp( pLine, iDrawChars, fX, a_fY, a_uiColour, a_fScale, a_fnCharCallback, a_fnGetCharWidth, a_fnRenderChar );

		if ( a_iMaxChars != -1 && a_wszString + a_iMaxChars <= pNextLine )
			return;

		a_fY         = fLineHeight + a_fY;
		m_bFirstLine = TFALSE;

		if ( *pNextLine == L'\0' )
			return;

		if ( a_fHeight < a_fY - fStartY )
			return;
	}
}

// $deBlob: FUNCTION 006f7ea0
void T2GUIFont::DrawTextBoxMultiColoured( const TWCHAR* a_wszString, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, const TUINT8* a_pColourIndices, const TUINT32* a_pColours, TFLOAT a_fScale, JUSTIFY a_eJustify, TINT a_iMaxChars, TBOOL a_bWordWrap, TUINT a_uiLineSpacing, TBOOL a_bLastLineFits, const TFLOAT* a_pfAlpha )
{
	TFLOAT fStartY = a_fY;

	if ( a_wszString == TNULL || *a_wszString == L'\0' || !IsCreated() )
		return;

	GetFontDef();
	TFLOAT fLineHeight = GetLineHeight( a_wszString, a_uiLineSpacing ) * a_fScale;
	m_bFirstLine       = TTRUE;
	m_bMoreLines       = TFALSE;
	m_iLineHeight      = TINT( fLineHeight );

	const TWCHAR* pString = a_wszString;
	TFLOAT        fLineWidth;
	GetNextTextBoxLine( pString, a_fWidth, a_fScale, fLineWidth, a_bWordWrap, TNULL );

	if ( *pString != L'\0' )
		m_bMoreLines = TTRUE;

	pString                 = a_wszString;
	const TWCHAR* pNextLine = a_wszString;

	if ( !a_bLastLineFits )
		a_fHeight -= fLineHeight;

	while ( TTRUE )
	{
		const TWCHAR* pLine     = pNextLine;
		TINT          iNumChars = GetNextTextBoxLine( pString, a_fWidth, a_fScale, fLineWidth, a_bWordWrap, TNULL );
		TFLOAT        fX        = a_fX;

		if ( a_eJustify != JUSTIFY_LEFT )
		{
			if ( a_eJustify == JUSTIFY_CENTRE )
				fX = ( a_fWidth - fLineWidth ) * 0.5f + a_fX;
			else if ( a_eJustify == JUSTIFY_RIGHT )
				fX = ( a_fWidth - fLineWidth ) + a_fX;
			else
				TASSERT( 0 );
		}

		TINT iDrawChars = iNumChars;

		if ( a_iMaxChars != -1 && a_iMaxChars <= TINT( pString - a_wszString ) )
		{
			pString         = a_wszString + a_iMaxChars;
			TINT iToMaxChar = TINT( pString - pLine );
			iDrawChars      = TMath::Min( iNumChars, iToMaxChar );
		}

		pNextLine = pString;
		DrawStringMultiColoured_Imp( pLine, iDrawChars, fX, a_fY, a_pColourIndices + ( pLine - a_wszString ), a_pColours, a_fScale, a_pfAlpha );

		if ( a_iMaxChars != -1 && a_wszString + a_iMaxChars <= pNextLine )
			return;

		a_fY         = fLineHeight + a_fY;
		m_bFirstLine = TFALSE;

		if ( *pNextLine == L'\0' )
			return;

		if ( a_fHeight < a_fY - fStartY )
			return;
	}
}

// $deBlob: FUNCTION 006f80c0
const T2GUIFontGlyph* T2GUIFont::GetGlyph( TWCHAR a_wcChar ) const
{
	const T2GUIFontDef* pDef    = GetFontDef();
	TINT32**            ppPages = GetPages();
	T2GUIFontGlyph*     pGlyphs = GetGlyphs();

	TUINT    uiPage = ( a_wcChar >> 8 ) & 0xFF;
	TINT32*  pPage  = ppPages[ uiPage ];

	if ( pPage != TNULL )
	{
		const T2GUIFontPageRange& rRange = pDef->pPageRanges[ uiPage ];

		if ( rRange.uiMin <= a_wcChar && a_wcChar <= rRange.uiMax )
		{
			TINT32 iGlyph = pPage[ a_wcChar - rRange.uiMin ];

			if ( iGlyph >= 0 )
				return &pGlyphs[ iGlyph ];
		}
	}

	return TNULL;
}

// Draws the characters material by material: each pass draws the glyphs of the material of the last glyph not
// drawn yet
// $deBlob: FUNCTION 006f81f0
void T2GUIFont::DrawString_Imp( const TWCHAR* a_wszString, TINT a_iLength, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, t_CharCallback a_fnCharCallback, t_GetCharWidth a_fnGetCharWidth, t_RenderChar a_fnRenderChar )
{
	TFLOAT              fOffsetX  = 0.0f;
	TFLOAT              fOffsetY  = 0.0f;
	T2GUIRenderer*      pRenderer = T2GUI::GetSingleton()->GetRenderer();
	const T2GUIFontDef* pDef      = GetFontDef();

	TINT iLast = -1;
	s_aBitField.Clear();

	if ( a_fnRenderChar == TNULL )
	{
		for ( iLast = a_iLength - 1; iLast >= 0 && GetGlyph( a_wszString[ iLast ] ) == TNULL; iLast-- )
			;
	}
	else
	{
		// The special characters are drawn first, the last glyph of the font starts the passes
		TFLOAT fX = a_fX;

		for ( TINT i = 0; i < a_iLength; i++ )
		{
			if ( a_fnCharCallback != TNULL )
				a_fnCharCallback( i, fOffsetX, fOffsetY );

			TFLOAT fCharWidth = 0.0f;

			if ( !a_fnRenderChar( pRenderer, a_wszString[ i ], TFLOAT( pDef->uiHeight ), fX + fOffsetX, TFLOAT( pDef->uiHeight ) + a_fY + fOffsetY, fCharWidth ) )
			{
				const T2GUIFontGlyph* pGlyph = GetGlyph( a_wszString[ i ] );

				if ( pGlyph != TNULL )
				{
					iLast = TMath::Max( iLast, i );
					fX    = TFLOAT( pGlyph->iRightPadding + pGlyph->iLeftPadding + pGlyph->uiWidth + pDef->iCharSpacing ) * a_fScale + fX;
				}
			}
			else
			{
				fX = ( TFLOAT( pDef->iCharSpacing ) + fCharWidth ) * a_fScale + fX;
			}
		}
	}

	while ( iLast >= 0 )
	{
		TFLOAT                fX       = a_fX;
		const T2GUIFontGlyph* pGlyph   = GetGlyph( a_wszString[ iLast ] );
		TUINT                 uiPassMat = pGlyph->GetMaterial();

		s_aBitField.Set( uiPassMat, TTRUE );

		T2GUIMaterial* pMaterial = m_materials[ uiPassMat ];
		pRenderer->SetMaterial( pMaterial );
		TFLOAT fInvWidth  = 1.0f / TFLOAT( TINT( T2GUI::GetWidth( pMaterial ) ) );
		TFLOAT fInvHeight = 1.0f / TFLOAT( TINT( T2GUI::GetHeight( pMaterial ) ) );

		TINT iNextLast = -1;
		TINT iEnd      = iLast;

		for ( TINT i = 0; i <= iEnd; i++ )
		{
			const TWCHAR wcChar = a_wszString[ i ];

			if ( a_fnGetCharWidth != TNULL )
			{
				TFLOAT fCharWidth = a_fnGetCharWidth( wcChar );

				if ( fCharWidth > 0.0f )
				{
					fX = ( TFLOAT( pDef->iCharSpacing ) + fCharWidth ) * a_fScale + fX;
					continue;
				}
			}

			const T2GUIFontGlyph* pCharGlyph = GetGlyph( wcChar );

			if ( pCharGlyph == TNULL )
				continue;

			// NOTE: retail keeps the scale of the first call ever in a function static
			static TFLOAT s_fScale = a_fScale;
			a_fScale               = s_fScale;

			// The glyphs of the platform buttons are scaled down to the line height
			TFLOAT fIconOffsetY = 0.0f;

			if ( TXUIScene::IsPlatformGlyph( wcChar ) )
			{
				TFLOAT fMaxHeight = ( !m_bMoreLines && i == 0 && m_bFirstLine ) ? TFLOAT( m_iLineHeight ) * 1.5f : TFLOAT( m_iLineHeight );
				TFLOAT fHeight    = TFLOAT( pCharGlyph->uiHeight );

				if ( fMaxHeight < fHeight )
					a_fScale = fMaxHeight / fHeight;

				fIconOffsetY = ( fHeight - fHeight * a_fScale ) * 0.5f;
			}

			TFLOAT fGlyphX = TFLOAT( pCharGlyph->iLeftPadding ) * a_fScale + fX;

			if ( pCharGlyph->uiWidth != 0 && pCharGlyph->uiHeight != 0 )
			{
				TUINT uiMat = pCharGlyph->GetMaterial();

				if ( uiMat == uiPassMat )
				{
					TUINT  uiHeight = pCharGlyph->uiHeight;
					TFLOAT fU1      = TFLOAT( pCharGlyph->uiPosX ) * fInvWidth;
					TFLOAT fV1      = TFLOAT( pCharGlyph->uiPosY ) * fInvHeight;
					TFLOAT fU2      = TFLOAT( pCharGlyph->uiPosX + 1 + pCharGlyph->uiWidth ) * fInvWidth;
					TFLOAT fV2      = TFLOAT( uiHeight + 1 + pCharGlyph->uiPosY ) * fInvHeight;
					TFLOAT fX2      = TFLOAT( pCharGlyph->uiWidth + 1 ) * a_fScale + fGlyphX;
					TFLOAT fTop     = TFLOAT( TINT( ( pCharGlyph->iYOffset + TUINT( pDef->uiHeight ) ) - uiHeight ) ) * a_fScale + a_fY;
					TFLOAT fY1      = fTop + fIconOffsetY;
					TFLOAT fY2      = TFLOAT( uiHeight + 1 ) * a_fScale + fTop + fIconOffsetY;

					if ( a_fnCharCallback != TNULL )
						a_fnCharCallback( i, fOffsetX, fOffsetY );

					if ( !pCharGlyph->IsWhite() )
						pRenderer->SetColour( a_uiColour );
					else
						pRenderer->SetColour( a_uiColour | 0xFFFFFF );

					TVector2 vUV2( fU2, fV2 );
					TVector2 vUV1( fU1, fV1 );
					TVector2 vB( fX2 + fOffsetX, fY2 + fOffsetY );
					TVector2 vA( fGlyphX + fOffsetX, fY1 + fOffsetY );
					pRenderer->RenderRectangle( vA, vB, vUV1, vUV2 );
				}
				else if ( !s_aBitField.IsSet( uiMat ) && iNextLast < i )
				{
					iNextLast = i;
				}
			}

			fX = TFLOAT( pCharGlyph->iRightPadding + pDef->iCharSpacing + pCharGlyph->uiWidth ) * a_fScale + fGlyphX;
		}

		iLast = iNextLast;
	}
}

// $deBlob: FUNCTION 006f8810
void T2GUIFont::DrawStringMultiColoured_Imp( const TWCHAR* a_wszString, TINT a_iLength, TFLOAT a_fX, TFLOAT a_fY, const TUINT8* a_pColourIndices, const TUINT32* a_pColours, TFLOAT a_fScale, const TFLOAT* a_pfAlpha )
{
	const T2GUIFontDef* pDef      = GetFontDef();
	T2GUIRenderer*      pRenderer = T2GUI::GetSingleton()->GetRenderer();
	TINT                iLast     = a_iLength - 1;
	s_aBitField.Clear();

	if ( iLast < 0 )
		return;

	while ( GetGlyph( a_wszString[ iLast ] ) == TNULL )
	{
		if ( --iLast < 0 )
			return;
	}

	while ( iLast >= 0 )
	{
		TFLOAT                fX        = a_fX;
		const T2GUIFontGlyph* pGlyph    = GetGlyph( a_wszString[ iLast ] );
		TUINT                 uiPassMat = pGlyph->GetMaterial();

		s_aBitField.Set( uiPassMat, TTRUE );

		T2GUIMaterial* pMaterial = m_materials[ uiPassMat ];
		pRenderer->SetMaterial( pMaterial );
		TFLOAT fInvWidth  = 1.0f / TFLOAT( TINT( T2GUI::GetWidth( pMaterial ) ) );
		TFLOAT fInvHeight = 1.0f / TFLOAT( TINT( T2GUI::GetHeight( pMaterial ) ) );

		TINT iNextLast = -1;
		TINT iEnd      = iLast;

		for ( TINT i = 0; i <= iEnd; i++ )
		{
			const TWCHAR          wcChar     = a_wszString[ i ];
			const T2GUIFontGlyph* pCharGlyph = GetGlyph( wcChar );

			if ( pCharGlyph == TNULL )
				continue;

			// NOTE: retail keeps the scale of the first call ever in a function static
			static TFLOAT s_fScale = a_fScale;
			a_fScale               = s_fScale;

			TFLOAT fIconOffsetY = 0.0f;

			if ( TXUIScene::IsPlatformGlyph( wcChar ) )
			{
				TFLOAT fMaxHeight = ( !m_bMoreLines && i == 0 && m_bFirstLine ) ? TFLOAT( m_iLineHeight ) * 1.5f : TFLOAT( m_iLineHeight );
				TFLOAT fHeight    = TFLOAT( pCharGlyph->uiHeight );

				if ( fMaxHeight < fHeight )
					a_fScale = fMaxHeight / fHeight;

				fIconOffsetY = ( fHeight - fHeight * a_fScale ) * 0.5f;
			}

			fX = TFLOAT( pCharGlyph->iLeftPadding ) * a_fScale + fX;

			if ( pCharGlyph->uiWidth != 0 && pCharGlyph->uiHeight != 0 )
			{
				TUINT uiMat = pCharGlyph->GetMaterial();

				if ( uiMat == uiPassMat )
				{
					TUINT32 uiColour = a_pColours[ a_pColourIndices[ i ] ];

					if ( a_pfAlpha == TNULL )
					{
						pRenderer->SetColour( uiColour );
					}
					else
					{
						TFLOAT fAlpha = TFLOAT( uiColour >> 24 ) / 255.0f;
						TASSERT( fAlpha >= 0 && fAlpha <= 1.0f );
						pRenderer->SetColour( ( TUINT32( *a_pfAlpha * fAlpha * 255.0f ) << 24 ) | ( uiColour & 0xFFFFFF ) );
					}

					TUINT  uiHeight = pCharGlyph->uiHeight;
					TFLOAT fU2      = TFLOAT( pCharGlyph->uiWidth + 1 + pCharGlyph->uiPosX ) * fInvWidth;
					TFLOAT fV2      = TFLOAT( uiHeight + 1 + pCharGlyph->uiPosY ) * fInvHeight;
					TFLOAT fTop     = TFLOAT( TINT( ( pCharGlyph->iYOffset + TUINT( pDef->uiHeight ) ) - uiHeight ) ) * a_fScale + a_fY;
					TFLOAT fU1      = TFLOAT( pCharGlyph->uiPosX ) * fInvWidth;
					TFLOAT fV1      = TFLOAT( pCharGlyph->uiPosY ) * fInvHeight;
					TFLOAT fX2      = TFLOAT( pCharGlyph->uiWidth + 1 ) * a_fScale + fX;

					TVector2 vUV2( fU2, fV2 );
					TVector2 vUV1( fU1, fV1 );
					TVector2 vB( fX2, TFLOAT( uiHeight + 1 ) * a_fScale + fTop + fIconOffsetY );
					TVector2 vA( fX, fTop + fIconOffsetY );
					pRenderer->RenderRectangle( vA, vB, vUV1, vUV2 );
				}
				else if ( !s_aBitField.IsSet( uiMat ) && iNextLast < i )
				{
					iNextLast = i;
				}
			}

			fX = TFLOAT( pCharGlyph->iRightPadding + pDef->iCharSpacing + pCharGlyph->uiWidth ) * a_fScale + fX;
		}

		iLast = iNextLast;
	}
}

// $deBlob: FUNCTION 006f8d10
TFLOAT T2GUIFont::GetStringWidth_Imp( const TWCHAR* a_wszString, TINT a_iLength, TFLOAT a_fScale, t_GetCharWidth a_fnGetCharWidth ) const
{
	TFLOAT              fWidth = 0.0f;
	const T2GUIFontDef* pDef   = GetFontDef();

	for ( ; a_iLength > 0; a_iLength-- )
	{
		fWidth = TFLOAT( pDef->iCharSpacing ) + GetGlyphWidth( *a_wszString, a_fnGetCharWidth ) + fWidth;
		a_wszString++;
	}

	return fWidth * a_fScale;
}

// $deBlob: FUNCTION 006f8df0
TINT T2GUIFont::GetNextTextBoxLine( const TWCHAR*& a_rpString, TFLOAT a_fWidth, TFLOAT a_fScale, TFLOAT& a_rfOutWidth, TBOOL a_bWordWrap, t_GetCharWidth a_fnGetCharWidth ) const
{
	const T2GUIFontDef* pDef = GetFontDef();
	const TWCHAR*       pChar = a_rpString;
	a_rfOutWidth             = 0.0f;

	if ( !a_bWordWrap )
	{
		// Up to the new line or as many characters as fit, the rest of the line is skipped
		do
		{
			TWCHAR wcChar = *pChar;

			if ( wcChar != L'\r' )
			{
				if ( wcChar == L'\n' )
				{
					pChar++;
					break;
				}

				TFLOAT fWidth = a_rfOutWidth + ( TFLOAT( pDef->iCharSpacing ) + GetGlyphWidth( wcChar, a_fnGetCharWidth ) ) * a_fScale;

				if ( a_fWidth < fWidth )
					break;

				a_rfOutWidth = fWidth;
			}

			pChar++;
		} while ( *pChar != L'\0' );

		TINT iNumChars = TINT( pChar - a_rpString );

		if ( ( pChar == a_rpString || pChar[ -1 ] != L'\n' ) && *pChar != L'\0' )
		{
			while ( *pChar != L'\n' )
			{
				pChar++;

				if ( *pChar == L'\0' )
				{
					a_rpString = pChar;
					return iNumChars;
				}
			}

			if ( *pChar != L'\0' )
				pChar++;
		}

		a_rpString = pChar;
		return iNumChars;
	}

	// Word wrap: the line ends after the last place it can be broken at
	const TWCHAR* pBreak      = TNULL;
	TFLOAT        fBreakWidth = 0.0f;
	const TWCHAR* pWordStart  = TNULL;

	do
	{
		TWCHAR wcChar = *pChar;

		if ( wcChar != L'\r' )
		{
			if ( wcChar == L'\n' )
			{
				fBreakWidth = a_rfOutWidth;
				pBreak      = pChar + 1;
				pChar       = pBreak;
			}
			else
			{
				if ( pWordStart == TNULL )
					fBreakWidth = a_rfOutWidth;

				TFLOAT fWidth = ( TFLOAT( pDef->iCharSpacing ) + GetGlyphWidth( wcChar, a_fnGetCharWidth ) ) * a_fScale + a_rfOutWidth;
				a_rfOutWidth  = fWidth;

				if ( fWidth <= a_fWidth )
				{
					if ( s_CanBreakBetweenChars( pChar[ 0 ], pChar[ 1 ] ) )
					{
						fBreakWidth = a_rfOutWidth;
						pBreak      = pChar + 1;
					}

					pWordStart = pBreak;
					pChar++;
					continue;
				}
			}

			if ( *pChar == L'\0' )
				fBreakWidth = a_rfOutWidth;
			else if ( pBreak != TNULL )
				pChar = pBreak;

			goto LINE_END;
		}

		pChar++;
	} while ( *pChar != L'\0' );

	fBreakWidth = a_rfOutWidth;

LINE_END:
	TINT iNumChars = TINT( pChar - a_rpString );

	if ( iNumChars != 0 )
	{
		// Skips the spaces the next line would start with
		if ( a_fWidth <= a_rfOutWidth )
		{
			while ( *pChar != L'\0' && ( ( iswspace( *pChar ) && *pChar != CHAR_NOBREAKSPACE ) || *pChar == CHAR_IDEOGRAPHICSPACE ) && *pChar != L'\n' )
				pChar++;
		}

		a_rpString   = pChar;
		a_rfOutWidth = fBreakWidth;
		return iNumChars;
	}

	a_rpString   = pChar + 1;
	a_rfOutWidth = fBreakWidth;
	return 0;
}

// The characters a Japanese line can't end with (opening brackets...) and start with (closing punctuation, small
// kana...), sorted
static const TWCHAR s_aJapaneseBeforeChars[] = {
	0x24, 0x28, 0x5B, 0x7B, 0xA3, 0xA5, 0x2018, 0x201B, 0x201C, 0x201F, 0x2116,
	0x3008, 0x300A, 0x300C, 0x300E, 0x3010, 0x3014, 0x3016, 0x3018, 0x301A, 0x301D
};

static const TWCHAR s_aJapaneseAfterChars[] = {
	0x21, 0x25, 0x29, 0x2C, 0x2E, 0x3A, 0x3B, 0x3F, 0x5D, 0x7D, 0xA2, 0xB0,
	0x2014, 0x2019, 0x201A, 0x201D, 0x201E, 0x2024, 0x2025, 0x2026, 0x2030, 0x2031, 0x2032, 0x2033,
	0x203C, 0x2044, 0x3001, 0x3002, 0x3009, 0x300B, 0x300D, 0x300F, 0x3011, 0x3015, 0x3017, 0x3019,
	0x301B, 0x301C, 0x301E, 0x301F, 0x3041, 0x3043, 0x3045, 0x3047, 0x3049, 0x3063, 0x3083, 0x3085,
	0x3087, 0x308E, 0x309D, 0x309E, 0x30A1, 0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30C3, 0x30E3, 0x30E5,
	0x30E7, 0x30EE, 0x30F5, 0x30F6, 0x30FB, 0x30FC, 0x30FD, 0x30FE
};

// The characters a Chinese line can't start with and end with
static const TWCHAR s_aChineseAfterChars[] = {
	0x21, 0x25, 0x29, 0x2C, 0x2E, 0x3A, 0x3B, 0x3F, 0x5D, 0x7D, 0xA2, 0xB0, 0xB7, 0x2019,
	0x2020, 0x2021, 0x203A, 0x2103, 0x2236, 0x3001, 0x3002, 0x3003, 0x3006, 0x3015, 0x3017, 0x301E,
	0xFE5A, 0xFE5C, 0xFF01, 0xFF02, 0xFF05, 0xFF07, 0xFF09, 0xFF0C, 0xFF0E, 0xFF1A, 0xFF1B, 0xFF1F,
	0xFF01, 0xFF3D, 0xFF5D, 0xFF5E
};

static const TWCHAR s_aChineseBeforeChars[] = {
	0x24, 0x28, 0xA3, 0xA5, 0xB7, 0x2018, 0x3008, 0x300A, 0x300C, 0x300E, 0x3010, 0x3014,
	0x3016, 0x301D, 0xFE59, 0xFE5B, 0xFF04, 0xFF08, 0xFF0E, 0xFF3B, 0xFF5B, 0xFFE1, 0xFFE5
};

static TBOOL BinarySearchChar( const TWCHAR* a_pChars, TINT a_iNumChars, TWCHAR a_wcChar )
{
	TINT iLow  = 0;
	TINT iHigh = a_iNumChars;

	do
	{
		TINT iMid = ( iHigh - iLow ) / 2 + iLow;

		if ( a_wcChar < a_pChars[ iMid ] )
			iHigh = iMid;
		else if ( a_wcChar > a_pChars[ iMid ] )
			iLow = iMid + 1;
		else
			return TTRUE;
	} while ( iLow < iHigh );

	return TFALSE;
}

// $deBlob: FUNCTION 006f90f0
TBOOL CanBreakBetweenChars_Japanese( TWCHAR a_wcChar, TWCHAR a_wcNextChar )
{
	if ( iswalnum( a_wcChar ) && iswalnum( a_wcNextChar ) )
		return TFALSE;

	if ( BinarySearchChar( s_aJapaneseBeforeChars, TARRAYSIZE( s_aJapaneseBeforeChars ), a_wcChar ) )
		return TFALSE;

	if ( BinarySearchChar( s_aJapaneseAfterChars, TARRAYSIZE( s_aJapaneseAfterChars ), a_wcNextChar ) )
		return TFALSE;

	return TTRUE;
}

// $deBlob: FUNCTION 006f9190
TBOOL CanBreakBetweenChars_Chinese( TWCHAR a_wcChar, TWCHAR a_wcNextChar )
{
	if ( iswdigit( a_wcChar ) && iswdigit( a_wcNextChar ) )
		return TFALSE;

	for ( TINT i = 0; i < TARRAYSIZE( s_aChineseAfterChars ); i++ )
	{
		if ( a_wcNextChar == s_aChineseAfterChars[ i ] )
			return TFALSE;
	}

	for ( TINT i = 0; i < TARRAYSIZE( s_aChineseBeforeChars ); i++ )
	{
		if ( a_wcChar == s_aChineseBeforeChars[ i ] )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f9200
TBOOL CanBreakBetweenChars_Default( TWCHAR a_wcChar, TWCHAR a_wcNextChar )
{
	return ( iswspace( a_wcNextChar ) && T2GUIFont::CHAR_NOBREAKSPACE != a_wcNextChar ) || a_wcNextChar == T2GUIFont::CHAR_IDEOGRAPHICSPACE;
}

//-----------------------------------------------------------------------------
// T2GUIFontDef2
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006f98d0
void T2GUIFontDef2::Init()
{
	T2GUIFontManager::CreateFont( this );
}

// $deBlob: FUNCTION 006f98e0
void T2GUIFontDef2::Deinit()
{
	T2GUIFontManager::DestroyFont( this );
}

TOSHI_NAMESPACE_END

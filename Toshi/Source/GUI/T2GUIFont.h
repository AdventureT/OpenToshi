#pragma once
#include "T2GUIMaterial.h"
#include "Toshi/TBitField.h"

TOSHI_NAMESPACE_START

class T2GUIRenderer;
class TTRB;

//-----------------------------------------------------------------------------
// A glyph of a font (0xC bytes, TRB data)
//-----------------------------------------------------------------------------
struct T2GUIFontGlyph
{
	TUINT16 uiUnk00;       // 0x00
	TUINT16 uiPosX;        // 0x02 in the texture, in pixels
	TUINT16 uiPosY;        // 0x04
	TUINT8  uiWidth;       // 0x06
	TUINT8  uiHeight;      // 0x07
	TINT8   iLeftPadding;  // 0x08
	TINT8   iRightPadding; // 0x09
	TINT8   iYOffset;      // 0x0A
	TUINT8  uiMaterial;    // 0x0B bits 0-6: the material, bit 7: drawn white (only the alpha of the colour is used)

	// $deBlob: FUNCTION 006f9550
	TINT GetMaterial() const { return uiMaterial & 0x7F; }

	// $deBlob: FUNCTION 006f9560
	TINT IsWhite() const { return uiMaterial >> 7; }
};

// The characters of a page (the characters that share their high byte)
struct T2GUIFontPageRange
{
	TUINT16 uiMin; // 0x00
	TUINT16 uiMax; // 0x02
};

//-----------------------------------------------------------------------------
// A font definition (TRB data, 0x1C bytes). The glyphs are looked up by page (high byte of the character).
//-----------------------------------------------------------------------------
struct T2GUIFontDef
{
	const TCHAR*        szName;         // 0x00
	const TCHAR*        szTextureNames; // 0x04 the textures of the materials, one string after the other
	TINT8               iCharSpacing;   // 0x08 added to the advance of every character
	TINT8               iLineSpacing;   // 0x09
	TUINT8              uiHeight;       // 0x0A
	TUINT8              uiNumMaterials; // 0x0B
	T2GUIFontPageRange* pPageRanges;    // 0x0C [ 256 ]
	TUINT32             uiUnk10;        // 0x10
	TINT32**            ppPages;        // 0x14 [ 256 ]: the glyph index of each character of the page (-1: none)
	T2GUIFontGlyph*     pGlyphs;        // 0x18
};

//-----------------------------------------------------------------------------
// The "tfnt" symbol of a font TRB (TAssetInit::InitFont creates its T2GUIFont)
//-----------------------------------------------------------------------------
class T2GUIFontDef2
{
public:
	// Creates the font (T2GUIFontManager::CreateFont)
	void Init();

	// Destroys the font (T2GUIFontManager::DestroyFont)
	void Deinit();

	const T2GUIFontDef* GetDef() const { return &m_oDef; }
	T2GUIFontDef*       GetDef() { return &m_oDef; }

	const TCHAR* GetName() const { return m_oDef.szName; }

public:
	TUINT32      m_uiUnk00; // 0x00
	T2GUIFontDef m_oDef;    // 0x04
};

//-----------------------------------------------------------------------------
// de Blob's T2GUIFont: draws strings and text boxes with the glyphs of a T2GUIFontDef (or T2GUIFontDef2), one material
// per texture of the font.
//-----------------------------------------------------------------------------
class T2GUIFont
{
public:
	// The spaces that don't break a line
	enum CHAR_ : TWCHAR
	{
		CHAR_NOBREAKSPACE     = 0x00A0,
		CHAR_IDEOGRAPHICSPACE = 0x3000,
	};

	using JUSTIFY = TINT;
	enum JUSTIFY_ : JUSTIFY
	{
		JUSTIFY_LEFT   = 0,
		JUSTIFY_CENTRE = 1,
		JUSTIFY_RIGHT  = 2,
	};

	// Called for every drawn character with its index in the line, may move it (and the rest of the line)
	using t_CharCallback = void( __stdcall* )( TINT a_iCharIndex, TFLOAT& a_rfOffsetX, TFLOAT& a_rfOffsetY );

	// The width of a special character (an icon), -1.0f (or <= 0.0f when drawing) for a glyph of the font
	using t_GetCharWidth = TFLOAT( __stdcall* )( TWCHAR a_wcChar );

	// Draws a special character (an icon); TFALSE if it's a glyph of the font
	using t_RenderChar = TBOOL( __stdcall* )( T2GUIRenderer* a_pRenderer, TWCHAR a_wcChar, TFLOAT a_fHeight, TFLOAT a_fX, TFLOAT a_fY, TFLOAT& a_rfOutWidth );

	using t_CanBreakBetweenChars = TBOOL ( * )( TWCHAR a_wcChar, TWCHAR a_wcNextChar );

	// NOTE: SetRegion calls the game's IsJapaneseLanguage/IsChineseLanguage (SteamManager), the game sets these
	using t_IsLanguage = TBOOL ( * )();

public:
	T2GUIFont();
	~T2GUIFont();

	// Selects the line breaking of the language
	static void SetRegion();

	TBOOL Create( const TCHAR* a_szSymbolName, TTRB* a_pTRB );
	TBOOL Create( T2GUIFontDef* a_pFontDef );
	TBOOL Create( T2GUIFontDef2* a_pFontDef );
	void  Destroy();

	TFLOAT GetHeight() const;
	TFLOAT GetLineSpacing() const;

	TFLOAT GetCharWidth( TWCHAR a_wcChar, TFLOAT a_fScale, t_GetCharWidth a_fnGetCharWidth ) const;
	TFLOAT GetStringWidth( const TWCHAR* a_wszString, TFLOAT a_fScale, t_GetCharWidth a_fnGetCharWidth ) const;
	TFLOAT GetLongestNonBreakableSubStringWidth( const TWCHAR* a_wszString, TFLOAT a_fScale, t_GetCharWidth a_fnGetCharWidth ) const;
	TFLOAT GetTextBoxHeight( const TWCHAR* a_wszString, TFLOAT a_fWidth, TFLOAT a_fScale, TBOOL a_bWordWrap, TUINT a_uiLineSpacing, t_GetCharWidth a_fnGetCharWidth ) const;

	void DrawString( const TWCHAR* a_wszString, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale );
	void DrawStringMultiColoured( const TWCHAR* a_wszString, TFLOAT a_fX, TFLOAT a_fY, const TUINT8* a_pColourIndices, const TUINT32* a_pColours, TFLOAT a_fScale );

	void DrawTextBox( const TWCHAR* a_wszString, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TUINT32 a_uiColour, TFLOAT a_fScale, JUSTIFY a_eJustify, TINT a_iMaxChars, TBOOL a_bWordWrap, TUINT a_uiLineSpacing, TBOOL a_bLastLineFits, t_CharCallback a_fnCharCallback, t_GetCharWidth a_fnGetCharWidth, t_RenderChar a_fnRenderChar );
	void DrawTextBoxMultiColoured( const TWCHAR* a_wszString, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, const TUINT8* a_pColourIndices, const TUINT32* a_pColours, TFLOAT a_fScale, JUSTIFY a_eJustify, TINT a_iMaxChars, TBOOL a_bWordWrap, TUINT a_uiLineSpacing, TBOOL a_bLastLineFits, const TFLOAT* a_pfAlpha );

	const T2GUIFontGlyph* GetGlyph( TWCHAR a_wcChar ) const;

	// The definition of the font (m_font or the one of m_font2)
	// $deBlob: FUNCTION 006f9430
	const T2GUIFontDef* GetFontDef() const
	{
		if ( m_font ) return m_font;
		TASSERT( TNULL != m_font2 );
		return m_font2->GetDef();
	}

	// $deBlob: FUNCTION 006f9500
	TINT32** GetPages() const
	{
		if ( m_font ) return m_font->ppPages;
		TASSERT( TNULL != m_font2 );
		return m_font2->GetDef()->ppPages;
	}

	// $deBlob: FUNCTION 006f93e0
	T2GUIFontGlyph* GetGlyphs() const
	{
		if ( m_font ) return m_font->pGlyphs;
		TASSERT( TNULL != m_font2 );
		return m_font2->GetDef()->pGlyphs;
	}

	// The unscaled width of a character (a_fnGetCharWidth's or the glyph's)
	// $deBlob: FUNCTION 006f9370
	TFLOAT GetGlyphWidth( TWCHAR a_wcChar, t_GetCharWidth a_fnGetCharWidth ) const
	{
		if ( a_fnGetCharWidth != TNULL )
		{
			TFLOAT fWidth = a_fnGetCharWidth( a_wcChar );
			if ( fWidth != -1.0f ) return fWidth;
		}

		const T2GUIFontGlyph* pGlyph = GetGlyph( a_wcChar );
		if ( pGlyph != TNULL ) return TFLOAT( pGlyph->iRightPadding + pGlyph->iLeftPadding + pGlyph->uiWidth );

		return 0.0f;
	}

	// The unscaled height of a line of a text box
	// $deBlob: FUNCTION 006f9480
	TFLOAT GetLineHeight( const TWCHAR* a_wszString, TUINT a_uiLineSpacing ) const
	{
		const T2GUIFontDef* pDef = GetFontDef();
		return TFLOAT( TUINT( pDef->iLineSpacing + pDef->uiHeight + a_uiLineSpacing ) );
	}

	TBOOL IsCreated() const { return m_font != TNULL || m_font2 != TNULL; }

private:
	void DrawString_Imp( const TWCHAR* a_wszString, TINT a_iLength, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour, TFLOAT a_fScale, t_CharCallback a_fnCharCallback, t_GetCharWidth a_fnGetCharWidth, t_RenderChar a_fnRenderChar );
	void DrawStringMultiColoured_Imp( const TWCHAR* a_wszString, TINT a_iLength, TFLOAT a_fX, TFLOAT a_fY, const TUINT8* a_pColourIndices, const TUINT32* a_pColours, TFLOAT a_fScale, const TFLOAT* a_pfAlpha );
	TFLOAT GetStringWidth_Imp( const TWCHAR* a_wszString, TINT a_iLength, TFLOAT a_fScale, t_GetCharWidth a_fnGetCharWidth ) const;

	// Returns the number of characters of the line that starts at a_rpString (and its width) and moves a_rpString to
	// the next line
	TINT GetNextTextBoxLine( const TWCHAR*& a_rpString, TFLOAT a_fWidth, TFLOAT a_fScale, TFLOAT& a_rfOutWidth, TBOOL a_bWordWrap, t_GetCharWidth a_fnGetCharWidth ) const;

public:
	static t_CanBreakBetweenChars s_CanBreakBetweenChars; // (CanBreakBetweenChars_Default)

	static t_IsLanguage ms_fnIsJapaneseLanguage;
	static t_IsLanguage ms_fnIsChineseLanguage;

private:
	// The materials a DrawString pass has drawn
	static TBitField<128> s_aBitField;

private:
	T2GUIFontDef*   m_font;        // 0x00
	T2GUIFontDef2*  m_font2;       // 0x04
	T2GUIMaterial** m_materials;   // 0x08 one per texture of the font
	TBOOL           m_bOwnsFont;   // 0x0C Destroy frees m_font and the resource m_font2 points to
	TINT            m_iLineHeight; // 0x10 of the text box being drawn (the platform glyphs are scaled to it)
	TBOOL           m_bFirstLine;  // 0x14 drawing the first line of the text box
	TBOOL           m_bMoreLines;  // 0x15 the text box has more than one line
};

// The line breaking of the languages
TBOOL CanBreakBetweenChars_Japanese( TWCHAR a_wcChar, TWCHAR a_wcNextChar );
TBOOL CanBreakBetweenChars_Chinese( TWCHAR a_wcChar, TWCHAR a_wcNextChar );
TBOOL CanBreakBetweenChars_Default( TWCHAR a_wcChar, TWCHAR a_wcNextChar );

TOSHI_NAMESPACE_END

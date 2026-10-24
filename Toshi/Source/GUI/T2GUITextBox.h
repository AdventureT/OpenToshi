#pragma once
#include "T2GUIElement.h"
#include "T2GUIFont.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// de Blob's T2GUITextBox: draws a wide string with a T2GUIFont word wrapped into its width, optionally with a drop
// shadow. The height is the one of the wrapped text unless it was set by SetDimensions( w, h, TTRUE ).
//-----------------------------------------------------------------------------
class T2GUITextBox : public T2GUIElement
{
public:
	T2GUITextBox();

	// $deBlob: FUNCTION 006f6e60
	// $deBlob: FUNCTION 006f7180
	~T2GUITextBox() {}

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void   Render() override;                                                // +0x14
	virtual void   GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight ) override;    // +0x1C
	virtual TFLOAT GetWidth() override;                                              // +0x20
	virtual TFLOAT GetHeight() override;                                             // +0x24
	virtual void   SetWidth( TFLOAT a_fWidth ) override;                             // +0x28
	virtual void   SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) override;      // +0x30, DOES_NOTHING

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Sets the font and the width of the box (the height follows the text)
	void Create( T2GUIFont* a_pFont, TFLOAT a_fWidth );

	// a_bFixedHeight: use a_fHeight instead of the height of the wrapped text
	void SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight, TBOOL a_bFixedHeight );

	TFLOAT GetFontHeight();

	// Draws the text (and its shadow), a_fnCharCallback is called by T2GUIFont::DrawTextBox for every character
	void Render( T2GUIFont::t_CharCallback a_fnCharCallback );

	// Sets the alpha of the shadow
	void SetShadowAlpha( TFLOAT a_fAlpha );

	// Both return the previous callback (they are not used by Render)
	T2GUIFont::t_RenderChar  SetGlyphRenderCallback( T2GUIFont::t_RenderChar a_fnRenderChar );
	T2GUIFont::t_GetCharWidth SetCharWidthCallback( T2GUIFont::t_GetCharWidth a_fnGetCharWidth );

	// NOTE: retail's callers write the following members directly (they are not functions)
	void SetShadow( TBOOL a_bShadow, TFLOAT a_fOffsetX, TFLOAT a_fOffsetY )
	{
		m_bShadow        = a_bShadow;
		m_fShadowOffsetX = a_fOffsetX;
		m_fShadowOffsetY = a_fOffsetY;
	}

	void SetShadowColour( TUINT32 a_uiColour ) { m_uiShadowColour = a_uiColour; }

	void SetText( const TWCHAR* a_wszText )
	{
		m_wszText = a_wszText;
		m_bDirty  = TTRUE;
	}

	void SetScale( TFLOAT a_fScale )
	{
		m_fScale = a_fScale;
		m_bDirty = TTRUE;
	}

	void SetFont( T2GUIFont* a_pFont )
	{
		m_pFont  = a_pFont;
		m_bDirty = TTRUE;
	}

	void SetJustify( T2GUIFont::JUSTIFY a_eJustify ) { m_eJustify = TUINT8( a_eJustify ); }
	void SetMaxChars( TINT a_iMaxChars ) { m_iMaxChars = a_iMaxChars; }

	const TWCHAR* GetText() const { return m_wszText; }
	T2GUIFont*    GetFont() const { return m_pFont; }

protected:
	TBOOL                     m_bShadow;        // 0x3C draw the shadow first
	TFLOAT                    m_fShadowOffsetX; // 0x40
	TFLOAT                    m_fShadowOffsetY; // 0x44
	TUINT32                   m_uiShadowColour; // 0x48
	const TWCHAR*             m_wszText;        // 0x4C
	T2GUIFont*                m_pFont;          // 0x50
	TFLOAT                    m_fScale;         // 0x54
	TINT                      m_iMaxChars;      // 0x58
	TUINT8                    m_eJustify;       // 0x5C T2GUIFont::JUSTIFY
	TBOOL                     m_bDirty;         // 0x5D the height has to be recalculated
	TBOOL                     m_bFixedHeight;   // 0x5E the height was set by SetDimensions
	T2GUIFont::t_RenderChar   m_fnRenderChar;   // 0x60
	T2GUIFont::t_GetCharWidth m_fnGetCharWidth; // 0x64
};

TSTATICASSERT( sizeof( T2GUITextBox ) == 0x68 );

TOSHI_NAMESPACE_END

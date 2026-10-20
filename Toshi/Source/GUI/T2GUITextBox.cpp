#include "ToshiPCH.h"
#include "T2GUITextBox.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006f6e00
T2GUITextBox::T2GUITextBox()
{
	m_bShadow        = TFALSE;
	m_fShadowOffsetX = 5.0f;
	m_fShadowOffsetY = 5.0f;
	m_uiShadowColour = 0x80000000;
	m_wszText        = TNULL;
	m_pFont          = TNULL;
	m_fScale         = 1.0f;
	m_iMaxChars      = -1;
	m_eJustify       = T2GUIFont::JUSTIFY_CENTRE;
	m_bFixedHeight   = TFALSE;
	m_fnRenderChar   = TNULL;
	m_fnGetCharWidth = TNULL;

#ifdef DEBLOB_COMMUNITY_PATCH
	m_bDirty = TTRUE;
#else
	// NOTE: retail leaves m_bDirty uninitialised until Create or SetWidth
#endif
}

// $deBlob: FUNCTION 006f6e70
void T2GUITextBox::Create( T2GUIFont* a_pFont, TFLOAT a_fWidth )
{
	m_pFont        = a_pFont;
	m_bDirty       = TTRUE;
	m_bFixedHeight = TFALSE;
	m_iWidth       = PackFloat( a_fWidth );
}

// $deBlob: FUNCTION 006f70e0
void T2GUITextBox::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight, TBOOL a_bFixedHeight )
{
	m_bFixedHeight = a_bFixedHeight;
	m_iWidth       = PackFloat( a_fWidth );

	if ( a_bFixedHeight )
	{
		m_iHeight = PackFloat( a_fHeight );
		m_bDirty  = TFALSE;
	}
	else
	{
		m_bDirty = TTRUE;
	}
}

// $deBlob: FUNCTION 006f6eb0
void T2GUITextBox::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	if ( !m_bFixedHeight && m_bDirty )
	{
		if ( m_wszText != TNULL && m_pFont != TNULL )
		{
			TFLOAT fHeight = m_pFont->GetTextBoxHeight( m_wszText, UnpackFloat( m_iWidth ), m_fScale, TTRUE, 0, TNULL );
			m_iHeight      = PackFloat( fHeight );
		}
		else
		{
			// NOTE: clears the width and the height
			m_iWidth  = 0;
			m_iHeight = 0;
		}

		m_bDirty = TFALSE;
	}

	a_rWidth  = UnpackFloat( m_iWidth );
	a_rHeight = UnpackFloat( m_iHeight );
}

// $deBlob: FUNCTION 006f6f60
TFLOAT T2GUITextBox::GetWidth()
{
	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );

	return fWidth;
}

// $deBlob: FUNCTION 006f6f80
TFLOAT T2GUITextBox::GetHeight()
{
	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );

	return fHeight;
}

// $deBlob: FUNCTION 006f6ea0
TFLOAT T2GUITextBox::GetFontHeight()
{
	if ( m_pFont != TNULL )
		return m_pFont->GetHeight();

	return 0.0f;
}

// $deBlob: FUNCTION 006f6fa0
void T2GUITextBox::Render( T2GUIFont::t_CharCallback a_fnCharCallback )
{
	if ( m_pFont != TNULL && m_wszText != TNULL )
	{
		TFLOAT fWidth, fHeight;
		GetDimensions( fWidth, fHeight );

		TFLOAT fX = fWidth * -0.5f;
		TFLOAT fY = fHeight * -0.5f;

		if ( m_bShadow == TTRUE )
		{
			m_pFont->DrawTextBox( m_wszText, m_fShadowOffsetX + fX, m_fShadowOffsetY + fY, fWidth, fHeight, m_uiShadowColour, m_fScale, m_eJustify, m_iMaxChars, TTRUE, 0, TFALSE, a_fnCharCallback, TNULL, TNULL );
		}

		m_pFont->DrawTextBox( m_wszText, fX, fY, fWidth, fHeight, m_uiColour, m_fScale, m_eJustify, m_iMaxChars, TTRUE, 0, TFALSE, a_fnCharCallback, TNULL, TNULL );
	}
}

// $deBlob: FUNCTION 006f70c0
void T2GUITextBox::Render()
{
	Render( TNULL );
	T2GUIElement::Render();
}

// $deBlob: FUNCTION 006f7130
void T2GUITextBox::SetShadowAlpha( TFLOAT a_fAlpha )
{
	m_uiShadowColour &= 0x00FFFFFF;
	m_uiShadowColour |= TUINT32( a_fAlpha * 255.0f ) << 24;
}

// $deBlob: FUNCTION 006f7160
T2GUIFont::t_RenderChar T2GUITextBox::SetGlyphRenderCallback( T2GUIFont::t_RenderChar a_fnRenderChar )
{
	T2GUIFont::t_RenderChar fnOld = m_fnRenderChar;
	m_fnRenderChar                = a_fnRenderChar;

	return fnOld;
}

// $deBlob: FUNCTION 006f7170
T2GUIFont::t_GetCharWidth T2GUITextBox::SetCharWidthCallback( T2GUIFont::t_GetCharWidth a_fnGetCharWidth )
{
	T2GUIFont::t_GetCharWidth fnOld = m_fnGetCharWidth;
	m_fnGetCharWidth                = a_fnGetCharWidth;

	return fnOld;
}

// $deBlob: FUNCTION 006f71c0
void T2GUITextBox::SetWidth( TFLOAT a_fWidth )
{
	m_iWidth = PackFloat( a_fWidth );
	m_bDirty = TTRUE;
}

// $deBlob: FUNCTION 006f71b0
void T2GUITextBox::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
}

TOSHI_NAMESPACE_END

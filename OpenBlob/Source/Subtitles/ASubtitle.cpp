#include "pch.h"
#include "ASubtitle.h"
#include "GUI/AFontManager.h"

#include <GUI/T2GUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0061adb0
ASubtitle::ASubtitle()
{
	m_pSubHeader = TNULL;
	m_iCurLine   = 0;
	m_fTime      = 0.0f;
	m_fSpeed     = 1.0f;
	m_wszText    = TNULL;
}

// $deBlob: FUNCTION 0061b1b0
// $deBlob: FUNCTION 0061adf0
ASubtitle::~ASubtitle()
{
}

// $deBlob: FUNCTION 0061ae00
void ASubtitle::Reset()
{
	m_pSubHeader = TNULL;
	m_iCurLine   = 0;
	m_fTime      = 0.0f;
	m_wszText    = TNULL;
	m_fSpeed     = 1.0f;
	SetTextBoxText( TNULL );
	m_oTextBox.RemoveSelf();
}

// $deBlob: FUNCTION 0061ae60
void ASubtitle::SetSubtitle( SubHeader* a_pSubHeader )
{
	Reset();
	m_pSubHeader = a_pSubHeader;

	if ( 0 < a_pSubHeader->m_iNumLines && TFLOAT( a_pSubHeader->m_aLines[ 0 ].m_iStart ) <= m_fTime * 0.001f )
		m_wszText = a_pSubHeader->m_aLines[ 0 ].m_wszText;

	SetDefaultRect();
}

// $deBlob: FUNCTION 0061af00
void ASubtitle::AdjustLengthF( TFLOAT a_fLength )
{
	if ( m_pSubHeader != TNULL )
		m_fSpeed = a_fLength / ( TFLOAT( m_pSubHeader->m_aLines[ m_pSubHeader->m_iNumLines - 1 ].m_iEnd ) * 0.001f );
}

// $deBlob: FUNCTION 0061af40
void ASubtitle::SetTimeF( TFLOAT a_fTime )
{
	SubHeader* pHeader = m_pSubHeader;

	if ( pHeader == TNULL )
		return;

	TINT iLine     = ( m_fTime <= a_fTime ) ? m_iCurLine : 0;
	TINT iNumLines = pHeader->m_iNumLines;

	while ( iLine < iNumLines && TFLOAT( pHeader->m_aLines[ iLine ].m_iEnd ) * m_fSpeed * 0.001f <= a_fTime )
		iLine++;

	if ( iLine < iNumLines )
	{
		m_iCurLine = iLine;
		m_fTime    = a_fTime;

		if ( TFLOAT( pHeader->m_aLines[ iLine ].m_iStart ) * m_fSpeed * 0.001f <= a_fTime &&
		     a_fTime < TFLOAT( pHeader->m_aLines[ iLine ].m_iEnd ) * m_fSpeed * 0.001f )
		{
			m_wszText = pHeader->m_aLines[ iLine ].m_wszText;
		}
		else
		{
			m_wszText = TNULL;
		}
	}
	else
	{
		m_wszText = TNULL;
	}

	SetTextBoxText( m_wszText );
}

// Returns TFALSE once there's no text to show, the manager stops the subtitle then
// NOTE: so the subtitle is stopped in the first gap between two lines (no subtitle is ever played on PC)
// $deBlob: FUNCTION 0061b020
TBOOL ASubtitle::Update( TFLOAT a_fDeltaTime )
{
	SetTimeF( m_fTime + a_fDeltaTime );
	return m_wszText != TNULL;
}

// $deBlob: FUNCTION 0061b060
void ASubtitle::SetLine( TINT a_iLine )
{
	if ( m_pSubHeader != TNULL )
	{
		TASSERT( a_iLine < m_pSubHeader->m_iNumLines );

		m_iCurLine = a_iLine;
		m_fTime    = TFLOAT( m_pSubHeader->m_aLines[ a_iLine ].m_iStart ) * m_fSpeed * 0.001f;
		m_wszText  = m_pSubHeader->m_aLines[ a_iLine ].m_wszText;
	}
}

// $deBlob: FUNCTION 0061b0e0
void ASubtitle::SetDefaultRect()
{
	T2GUIFont*    pFont = AFontManager::sm_pMsgFont;
	T2GUIElement* pRoot = T2GUI::GetSingleton()->GetRootElement();

	TFLOAT fWidth;
	TFLOAT fHeight;
	pRoot->GetDimensions( fWidth, fHeight );

	m_oTextBox.RemoveSelf();
	m_oTextBox.Create( pFont, fWidth );
	m_oTextBox.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oTextBox.SetJustify( T2GUIFont::JUSTIFY_CENTRE );
	m_oTextBox.SetDimensions( fWidth - 80.0f, 60.0f, TTRUE );
	SetTextBoxText( m_wszText );
	m_oTextBox.SetVisible( TTRUE );
	m_oTextBox.SetState( T2GUIElement::STATE_SUBTITLES );
	pRoot->AddChildTail( &m_oTextBox );
}

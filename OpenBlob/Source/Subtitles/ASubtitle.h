#pragma once
#include <Toshi/T2SList.h>
#include <GUI/T2GUITextBox.h>

//-----------------------------------------------------------------------------
// Purpose: one subtitle being shown: a list of timed lines from the subtitles TRB shown in a text box at the bottom of
// the screen. Owned by ASubtitleManager.
// NOTE: in de Blob PC ASubtitleManager::PlaySubtitle always returns TNULL, so no subtitle is ever shown
//-----------------------------------------------------------------------------
class ASubtitle
    : public Toshi::T2SList<ASubtitle>::Node
{
public:
	struct Line
	{
		TINT          m_iStart;  // 0x00 in ms
		TINT          m_iEnd;    // 0x04 in ms
		const TWCHAR* m_wszText; // 0x08
	};

	struct SubHeader
	{
		TINT m_iNumLines;   // 0x00
		Line m_aLines[ 1 ]; // 0x04 m_iNumLines entries
	};

public:
	ASubtitle();
	virtual ~ASubtitle(); // +0x00

	void  Reset();
	void  SetSubtitle( SubHeader* a_pSubHeader );
	void  AdjustLengthF( TFLOAT a_fLength );
	void  SetTimeF( TFLOAT a_fTime );
	TBOOL Update( TFLOAT a_fDeltaTime );
	void  SetLine( TINT a_iLine );
	void  SetDefaultRect();

	// $deBlob: FUNCTION 0061b050
	TINT GetNumLines() const { return m_pSubHeader ? m_pSubHeader->m_iNumLines : 0; }

	// $deBlob: FUNCTION 0061b1e0
	TFLOAT GetTime() const { return m_fTime; }

private:
	void SetTextBoxText( const TWCHAR* a_wszText )
	{
		m_oTextBox.SetText( a_wszText );
		m_oTextBox.SetMaxChars( -1 );
	}

private:
	SubHeader*          m_pSubHeader; // 0x08
	TINT                m_iCurLine;   // 0x0C
	TFLOAT              m_fTime;      // 0x10 in seconds
	const TWCHAR*       m_wszText;    // 0x14 the text of the current line, TNULL between lines
	Toshi::T2GUITextBox m_oTextBox;   // 0x18
	TFLOAT              m_fSpeed;     // 0x80 scale of the line times (AdjustLengthF)
};

TSTATICASSERT( sizeof( ASubtitle ) == 0x84 );

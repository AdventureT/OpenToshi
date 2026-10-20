#pragma once
#include <XUI/TXUILabel.h>

//-----------------------------------------------------------------------------
// Purpose: a label that reveals its text one character after the other, used
// by the help texts (ABlobHelpScene's control 5, ABlobHelpFullScreen).
//-----------------------------------------------------------------------------
class ABlobTextReveal : public Toshi::TXUILabel
{
public:
	TDECLARE_CLASS( ABlobTextReveal, Toshi::TXUILabel );

public:
	ABlobTextReveal();
	~ABlobTextReveal();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual void SetText( const TWCHAR* a_wszText, TINT a_iParam ) override; // +0xE8

	// Ends the reveal: shows all the text (a_bShowAll) or none of it
	void FinishReveal( TBOOL a_bShowAll );

	// Reveals the current text again from its start
	void RestartReveal();

	// The text is still being revealed
	TBOOL IsRevealing() const { return m_bRevealing && m_iNumRevealed < m_iLength; }

private:
	TFLOAT m_fRevealInterval; // 0x94 0.025 s between two characters
	TFLOAT m_fRevealTimer;    // 0x98
	TINT   m_iNumRevealed;    // 0x9C
	TINT   m_iLength;         // 0xA0
	TBOOL  m_bRevealing;      // 0xA4
};

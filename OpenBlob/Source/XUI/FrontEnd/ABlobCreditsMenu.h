#pragma once
#include "XUI/ABlobMenu.h"
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Purpose: the scrolling credits: Data\credits.trb's
// "Blocks" are titles with lines (string IDs, localised on load); every line scrolls up in one of 32 "NameScene"s, the
// titles in one of 8 "TitleScene"s of credits.xur. The menu closes once the last line has scrolled away.
//-----------------------------------------------------------------------------
class ABlobCreditsMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobCreditsMenu, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_NAME_SCENES  = 32;
	static TCONSTEXPR_VAR TINT NUM_TITLE_SCENES = 8;

	// A title and its lines in credits.trb (0xC bytes)
	struct Block
	{
		union
		{
			TINT          m_iTitleId; // 0x0 in the TRB...
			const TWCHAR* m_wszTitle; // 0x0 ...replaced by OnCreated
		};
		union
		{
			TINT*          m_piLineIds; // 0x4 in the TRB...
			const TWCHAR** m_pwszLines; // 0x4 ...replaced by OnCreated
		};
		TINT m_iNumLines; // 0x8
	};

	// The "Blocks" symbol of credits.trb (0x18 bytes)
	struct Blocks
	{
		Block** m_ppBlocks;       // 0x00
		TINT    m_iNumBlocks;     // 0x04
		TFLOAT  m_fTitleDelay;    // 0x08 after a title
		TFLOAT  m_fLastLineDelay; // 0x0C after the line before the last of a block
		TFLOAT  m_fLineDelay;     // 0x10 after the other lines
		TFLOAT  m_fEndDelay;      // 0x14 once the last line has scrolled away
	};

public:
	ABlobCreditsMenu();
	~ABlobCreditsMenu();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;    // +0x114
	virtual void EndTransTo() override; // +0x124

private:
	// Scrolls a_wszText in the next title or name scene
	void AddLine( const TWCHAR* a_wszText, TBOOL a_bTitle );

	// Waits for the next line or title
	void UpdateLines( TFLOAT a_fDeltaTime );

private:
	Toshi::TTRB       m_oTRB;                         // 0x140
	Blocks*           m_pBlocks;                      // 0x154
	TINT              m_iBlock;                       // 0x158
	TINT              m_iLine;                        // 0x15C
	TINT              m_iNextName;                    // 0x160
	TINT              m_iNextTitle;                   // 0x164
	Toshi::TXUIScene* m_apNames[ NUM_NAME_SCENES ];   // 0x168
	Toshi::TXUIScene* m_apTitles[ NUM_TITLE_SCENES ]; // 0x1E8
	TFLOAT            m_fTimer;                       // 0x208
	TBOOL             m_bEnding;                      // 0x20C every line has scrolled away
	TBOOL             m_bClosed;                      // 0x20D
};

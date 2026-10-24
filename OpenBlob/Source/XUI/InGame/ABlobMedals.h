#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the medals screen: the medals of the level and the ones
// the player has.
//-----------------------------------------------------------------------------
class ABlobMedals : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobMedals, ABlobMenu );

	static TCONSTEXPR_VAR TINT MAXACHIEVEMENTS = 10; // "Achievement1".."Achievement10" controls

public:
	ABlobMedals();
	~ABlobMedals();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam ) override;           // +0x78
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0
	virtual TBOOL OnNotifySetFocus( Toshi::TXUIElement* a_pSource ) override;                  // +0xB4
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override;                 // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override; // +0x114

private:
	// The next/previous of the 10 titles (with "Next"/"Prev"), unless the scene's timeline plays
	void NextTitle();
	void PrevTitle();

	// Shows the medals and awards of world m_iTitleIndex ("Repopulate" message)
	void Repopulate();

private:
	// 0x140 the button (id 4) that opens the medal list
	Toshi::TXUIControl* m_pButton4;
	// 0x144 the button (id 3) that closes the medals
	Toshi::TXUIControl* m_pButton3;
	// 0x148 the achievement controls "Achievement1".."Achievement10"
	Toshi::TXUIControl* m_apAchievements[ MAXACHIEVEMENTS ];
	// 0x170 the title text control (id 9), its text is the locale string m_iTitleIndex
	Toshi::TXUIControl* m_pTitle;
	// 0x174 the count text control (id 7), its text is m_wszCount
	Toshi::TXUIControl* m_pCount;
	// 0x178 the "%d/%d" count text
	TWCHAR m_wszCount[ 0x20 ];
	// 0x1B8 the index of the title's locale string
	TINT m_iTitleIndex;
};

TSTATICASSERT( sizeof( ABlobMedals ) == 0x1BC );

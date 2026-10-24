#pragma once
#include "XUI/ABlobMenu.h"
#include "GUI/A2GUIBackground.h"

//-----------------------------------------------------------------------------
// Purpose: the concept art menu: pressing a button named "bik_<movie>" plays that movie full screen,
// any other button shows the picture of its name as a full screen background; back (button 1) closes it.
//-----------------------------------------------------------------------------
class ABlobConceptsMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobConceptsMenu, ABlobMenu );

public:
	ABlobConceptsMenu();
	~ABlobConceptsMenu();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

private:
	A2GUIBackground     m_oBackground;   // 0x140
	Toshi::TXUIControl* m_pPrevFocus;    // 0x218 the focus before the picture or the movie
	TBOOL               m_bPlayingMovie; // 0x21C
};

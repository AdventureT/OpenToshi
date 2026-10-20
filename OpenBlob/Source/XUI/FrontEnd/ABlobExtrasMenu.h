#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the extras menu: its three buttons (cutscenes, movies, gallery) clear
// their "New" flags when pressed; coming back from the music mixer's "Quit" it destroys the track selection and
// goes back to the main menu.
//-----------------------------------------------------------------------------
class ABlobExtrasMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobExtrasMenu, ABlobMenu );

public:
	ABlobExtrasMenu();
	~ABlobExtrasMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;       // +0x114
	virtual void TransFrom() override;     // +0x118
	virtual void TransBackTo() override;   // +0x11C
	virtual void TransBackFrom() override; // +0x120
	virtual void EndTransTo() override;    // +0x124

	// The next TransTo destroys the track selection and goes back to the main menu (ABlobPopupMusicMixer)
	// $deBlob: FUNCTION 0063a830
	void SetReturnToMainMenu() { m_bReturnToMainMenu = TTRUE; }

private:
	Toshi::TXUIControl* m_pCutscenes;        // 0x140 control 7
	Toshi::TXUIControl* m_pMovies;           // 0x144 control 6
	Toshi::TXUIControl* m_pGallery;          // 0x148 control 5
	TBOOL               m_bReturnToMainMenu; // 0x14C
	Toshi::TXUIControl* m_pCutscenesNew;     // 0x150 control 9
	Toshi::TXUIControl* m_pMoviesNew;        // 0x154 control 10
	Toshi::TXUIControl* m_pGalleryNew;       // 0x158 control 11
};

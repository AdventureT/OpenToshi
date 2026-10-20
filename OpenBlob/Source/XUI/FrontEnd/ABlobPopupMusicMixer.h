#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the popup of the music mixer (no members
// of its own): "btnContinue" resumes the music, "btnChangeTune" goes back to the track selection ("MusicSelectMenu"),
// "btnQuit" to the extras ("ExtrasScene").
//-----------------------------------------------------------------------------
class ABlobPopupMusicMixer : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobPopupMusicMixer, ABlobMenu );

public:
	ABlobPopupMusicMixer();
	~ABlobPopupMusicMixer();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0
};

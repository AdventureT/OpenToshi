#pragma once
#include "XUI/ABlobMenu.h"

class ABlobCarouselList;

//-----------------------------------------------------------------------------
// Purpose: the movie list of the extras: a carousel of the extra
// movies, the unlocked ones (EnSaveData::SlotData::m_iUnlockedVideos) play full screen after a fade to black; the movies whose
// name ends with "_M" keep the music paused.
//-----------------------------------------------------------------------------
class ABlobMovies : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobMovies, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_MOVIES = 10; // AGameStatus::MovieUnlock_NUMOF

	// An extra movie (8 bytes)
	struct Movie
	{
		const TCHAR* m_szName;   // 0x0 the file in data\movies
		TINT         m_iTitleId; // 0x4 its title
	};

	static const Movie sm_aMovies[ NUM_MOVIES ];

public:
	ABlobMovies();
	~ABlobMovies();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override; // +0x114

private:
	TBOOL               m_bMoviePlaying;  // 0x140
	TBOOL               m_bStartMovie;    // 0x141 the movie starts once the fade to black is over
	TFLOAT              m_fMusicTime;     // 0x144 -1: the music isn't paused for the movie
	TCHAR               m_szMovie[ 128 ]; // 0x148
	ABlobCarouselList*  m_pCarousel;      // 0x1C8 control 4
	Toshi::TXUIControl* m_pSelect;        // 0x1CC control 5
	Toshi::TXUIControl* m_pLeft;          // 0x1D0 control 7
	Toshi::TXUIControl* m_pRight;         // 0x1D4 control 6
	Toshi::TXUIControl* m_pTitle;         // 0x1D8 control 8
};

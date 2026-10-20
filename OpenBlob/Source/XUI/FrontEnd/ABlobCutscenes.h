#pragma once
#include "XUI/ABlobMenu.h"

class ABlobCarouselList;

//-----------------------------------------------------------------------------
// Purpose: the cutscene list of the extras: a carousel of the game's
// cutscenes, the unlocked ones (EnSaveData::SlotData::m_iUnlockedMovies) play full screen.
//-----------------------------------------------------------------------------
class ABlobCutscenes : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobCutscenes, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_CUTSCENES = 12; // AGameStatus::CutsceneUnlock_NUMOF

	// The cutscenes' movies
	static const TCHAR* sm_aszCutscenes[ NUM_CUTSCENES ];

public:
	ABlobCutscenes();
	~ABlobCutscenes();

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
	TBOOL               m_bMoviePlaying; // 0x140
	ABlobCarouselList*  m_pCarousel;     // 0x144 control 4
	Toshi::TXUIControl* m_pSelect;       // 0x148 control 5
	Toshi::TXUIControl* m_pLeft;         // 0x14C control 7
	Toshi::TXUIControl* m_pRight;        // 0x150 control 6
	Toshi::TXUIControl* m_pTitle;        // 0x154 control 8
};

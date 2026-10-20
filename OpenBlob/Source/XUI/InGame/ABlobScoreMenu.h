#pragma once
#include "XUI/ABlobMenu.h"

class ABlobRewardScreen;

//-----------------------------------------------------------------------------
// Purpose: the level results screen: shown when a level is
// completed, the score and the medals of the level, then the awards and the reward screen (rewardscreen.xur).
//-----------------------------------------------------------------------------
class ABlobScoreMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobScoreMenu, ABlobMenu );

	static TCONSTEXPR_VAR TINT MAXAWARDS       = 10; // "Award1".."Award10" controls and their awarded flags
	static TCONSTEXPR_VAR TINT MAXACHIEVEMENTS = 10; // the achievement controls at 0x18C

public:
	ABlobScoreMenu();
	~ABlobScoreMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                        // +0x04
	virtual void  OnCreated() override;                                        // +0x94
	virtual TBOOL OnNotifySetFocus( Toshi::TXUIElement* a_pSource ) override;  // +0xB4
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void EndTransBackTo() override; // +0x12C

	// The results screen of the current level, cleared by the dtor
	static ABlobScoreMenu* sm_pInstance;

private:
	// The medals and the awards of the level (the "Medal<name>On/Off" visuals and their texts)
	void SetupMedals();

	// Formats a time as "m:ss" (a_iUnused is not used)
	static void __stdcall FormatTime( TINT a_iMinutes, TINT a_iSeconds, TWCHAR* a_wszBuffer, TINT a_iSize, TINT a_iUnused );

private:
	// 0x140 the control (id 0x27) whose opacity (XUIState) reaches 127 once the screen has faded in
	Toshi::TXUIControl* m_pTransitionControl;
	// 0x144 the score text "Points" (the locale string 0xBE)
	TWCHAR m_wszScore[ 0x10 ];
	// 0x164 the time text
	TWCHAR m_wszTime[ 0x10 ];
	// 0x184 the control (id 0x10) shown when no medal was reached
	Toshi::TXUIControl* m_pNoMedal;
	// 0x188 the control (id 0x0E) shown for a new record time
	Toshi::TXUIControl* m_pNewRecord;
	// 0x18C the achievement controls (ids 0x31 down to 0x28)
	Toshi::TXUIControl* m_apAchievements[ MAXACHIEVEMENTS ];
	// 0x1B4 the counter texts of the awards: 0 Challenge, 1 the paint percentage, 2 Trees, 3 Raydian,
	// 4 Transform, 5 Bill, 6 Landmarks, 7 Styles, 8 Vista, 9 the target time
	TWCHAR m_awszAwardTexts[ 10 ][ 0x10 ];
	// 0x2F4 the TransFrom animation of m_pTransElement has been started
	TUINT8 m_bTransOutRequested;
	// 0x2F5 the results are being shown
	TUINT8 m_bResultsBusy;
	// 0x2F6 the results are shown, the player may continue
	TUINT8 m_bResultsShown;
	// 0x2F8 the element that plays the TransFrom / EndTransFrom animation (id 2)
	Toshi::TXUIElement* m_pTransElement;
	// 0x2FC the reward screen (rewardscreen.xur) the results lead to
	ABlobRewardScreen* m_pRewardScreen;
	// 0x300 the screen is leaving (the fade to black is running)
	TUINT8 m_bLeaving;
	// 0x301 a fade is running: the fade in or, with m_bLeaving, the fade out that ends
	// the level
	TUINT8 m_bWaitForFade;
	// 0x304 the award controls "Award1".."Award10"
	Toshi::TXUIControl* m_apAwards[ MAXAWARDS ];
	// 0x32C the awarded flag of each award (the award's sound plays on focus when set)
	TUINT8 m_abAwarded[ MAXAWARDS ];
	// 0x336
	TUINT8 m_aUnk336[ 0x338 - 0x336 ];
};

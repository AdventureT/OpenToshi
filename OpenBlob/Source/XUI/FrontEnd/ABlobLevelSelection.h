#pragma once
#include "XUI/ABlobMenu.h"

class ABlobTrackList;

//-----------------------------------------------------------------------------
// Purpose: the level selection of Story and Free Paint (levelselection.xur): the ten worlds; pressing one shows
// its panel (Story: the level and its two
// sprint challenges with stats and medals) and then the music panel (ABlobTrackList) whose tracks start the level.
//-----------------------------------------------------------------------------
class ABlobLevelSelection : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobLevelSelection, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_WORLDS = 10;

public:
	ABlobLevelSelection();
	~ABlobLevelSelection();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TXUIElement* RayTraceDeepest( const Toshi::TVector2& a_rPoint, const Toshi::TVector2& a_rScale, TINT a_iController ) override; // +0x64
	virtual TBOOL        SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam ) override;                                                // +0x78
	virtual void         OnCreated() override;                                                                                             // +0x94
	virtual TBOOL        OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override;                                             // +0xB0
	virtual TBOOL        OnNotifySetFocus( TXUIElement* a_pSource ) override;                                                              // +0xB4

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void OnMouseOverChange( TXUIElement* a_pElement, TINT a_iController ) override; // +0x110
	virtual void TransTo() override;                                                        // +0x114
	virtual void TransBackFrom() override;                                                  // +0x120
	virtual void EndTransTo() override;                                                     // +0x124
	virtual void EndTransBackFrom() override;                                               // +0x130

private:
	// The music panel of the selected world (a_bFreePaint: the free paint mood)
	void ShowMusicPanel( TINT a_iController, TBOOL a_bFreePaint );

	// The stats of a sprint challenge slot
	void ShowSlotStats( TINT a_iSlot );

	// The time of the stats ("m:ss", a_bTenths: ".t"), -1: none
	void SetStatsTime( TFLOAT a_fTime, TBOOL a_bTenths );

	// The track list was scrolled
	void OnTrackScrolledLeft();
	void OnTrackScrolledRight();

private:
	Toshi::TXUIControl* m_apWorlds[ NUM_WORLDS ];   // 0x140 the world buttons
	TUINT32             m_uiUnk168;                 // 0x168
	Toshi::TXUIScene*   m_pWorldPanel;              // 0x16C UID 15
	Toshi::TXUIControl* m_pPrevFocus;               // 0x170 the focus before the world panel
	Toshi::TXUIControl* m_pLevel;                   // 0x174 UID 34, the world's level
	Toshi::TXUIControl* m_apSlots[ 2 ];             // 0x178 UID 33/32, the world's sprint challenges
	Toshi::TXUIControl* m_pTitle;                   // 0x180 UID 20
	Toshi::TXUIScene*   m_pLevelPanel;              // 0x184 UID 35
	Toshi::TXUIScene*   m_pMusicPanel;              // 0x188 UID 56
	TBOOL               m_bWorldSelected;           // 0x18C the world panel is shown
	ABlobTrackList*     m_pTrackList;               // 0x190 UID 58
	Toshi::TXUIControl* m_apWorldNew[ NUM_WORLDS ]; // 0x194 UID 111+, the "New" labels
	Toshi::TXUIControl* m_pTime;                    // 0x1BC UID 23
	Toshi::TXUIControl* m_pScore;                   // 0x1C0 UID 25
	TWCHAR              m_wszTime[ 128 ];           // 0x1C4
	TUINT8              m_auiUnk2C4[ 0x200 ];       // 0x2C4
	TWCHAR              m_wszScore[ 128 ];          // 0x4C4
	Toshi::TXUIControl* m_apSlotNew[ 2 ];           // 0x5C4 UID 107/108
	Toshi::TXUIControl* m_pTrackDescription;        // 0x5CC UID 59
	Toshi::TXUIControl* m_pTrackLeft;               // 0x5D0 UID 62, the left arrow
	Toshi::TXUIControl* m_pTrackRight;              // 0x5D4 UID 61, the right arrow
	Toshi::TXUIControl* m_pTrackName;               // 0x5D8 UID 60
	Toshi::TXUIElement* m_pMedals;                  // 0x5DC "Medals" of UID 24
	Toshi::TXUIElement* m_apTrackStatic[ 2 ];       // 0x5E0 UID 63/85
	TBOOL               m_bStatsShown;              // 0x5E8
	TBOOL               m_bEnableInput;             // 0x5E9 enables the XUI input once faded in
};

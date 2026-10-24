#pragma once
#include "XUI/ABlobMenu.h"

#include <XUI/TXUICheckBox.h>
#include <XUI/TXUISliderBar.h>
#include <XUI/TXUITabScene.h>

//-----------------------------------------------------------------------------
// Purpose: the options menu of the front end: the music and
// SFX volume sliders, the camera speed slider, the camera invert and help checkboxes and three tabs. The sliders play
// a sound while they change and apply the volumes through AOptions::UpdateVolumes; leaving the menu saves the options
// (SaveLoad, the m_bSaving state).
//-----------------------------------------------------------------------------
class ABlobOptionsMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobOptionsMenu, ABlobMenu );

public:
	ABlobOptionsMenu();
	~ABlobOptionsMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                        // +0x04
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0
	virtual TBOOL OnNotifyValueChanged( Toshi::TXUIElement* a_pSource ) override;              // +0xBC

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;        // +0x114
	virtual void TransBackFrom() override;  // +0x120
	virtual void EndTransTo() override;     // +0x124
	virtual void EndTransBackTo() override; // +0x12C

private:
	// Sets the sliders and checkboxes from the options
	void SyncFromOptions();
	// Closes the save: m_bSaving off, the back button pressed and the options applied
	void FinishSaving( TINT a_iError ); // a_iError: SaveLoad's result (unused)

public:
	Toshi::TXUISliderBar* m_pMusicSlider;     // 0x140 UID 7, the music volume (0..10)
	Toshi::TXUISliderBar* m_pSFXSlider;       // 0x144 UID 8, the SFX volume (0..10)
	Toshi::TXUISliderBar* m_pCameraSpeed;     // 0x148 UID 12, the camera speed
	Toshi::TXUICheckBox*  m_pCameraInvert;    // 0x14C UID 15, the camera invert checkbox
	Toshi::TXUICheckBox*  m_pHelp;            // 0x150 UID 20, the help checkbox (checked when the help is off)
	TUINT32               m_uiUnk154;         // 0x154 not set by retail's OnCreated; NOTE: zeroed here
	Toshi::TXUIControl*   m_pUID24;           // 0x158 UID 24, the Apply button (OnNotifyPress saves the options)
	TBOOL                 m_bSaving;          // 0x15C the options are being saved (SaveLoad), set by the save
	TUINT8                m_uiUnk15D;         // 0x15D
	TBOOL                 m_bUnk15E;          // 0x15E
	TINT                  m_iLastSFX;         // 0x160 the SFX volume the last time it was applied
	TINT                  m_iLastMusic;       // 0x164 the music volume the last time it was applied
	TINT                  m_iLastCameraSpeed; // 0x168 the camera speed the last time it was applied
	Toshi::TXUITabScene*  m_pTabs;            // 0x16C UID 3, the tab scene of the three tabs
	Toshi::TXUIControl*   m_pTabButton1;      // 0x170 UID 21, the first tab button
	Toshi::TXUIControl*   m_pTabButton2;      // 0x174 UID 22, the second tab button
	Toshi::TXUIControl*   m_pTabButton3;      // 0x178 UID 23, the third tab button
};

TSTATICASSERT( sizeof( ABlobOptionsMenu ) == 0x17C );

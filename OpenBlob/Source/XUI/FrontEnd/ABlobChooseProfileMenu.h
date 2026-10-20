#pragma once
#include "XUI/ABlobMenu.h"
#include "MemoryCard/EnSaveData.h"

//-----------------------------------------------------------------------------
// Purpose: the profile selection: the buttons
// "Profile1" to "Profile4" show "Game <n>" for the used save slots; pressing a used one makes it the current profile
// and goes to the main menu, pressing an empty one creates a profile named "Profile_<n>" in the current save slot,
// saves it and then goes on as if it was pressed again.
//-----------------------------------------------------------------------------
class ABlobChooseProfileMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobChooseProfileMenu, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_PROFILES = 4;

	// The texts of the used profiles' buttons, "Game <n>" (also used by ABlobDeleteProfileMenu)
	static TWCHAR sm_awszGameNames[ NUM_PROFILES ][ 12 ];

public:
	ABlobChooseProfileMenu();
	~ABlobChooseProfileMenu();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;        // +0x114
	virtual void TransFrom() override;      // +0x118
	virtual void TransBackTo() override;    // +0x11C
	virtual void EndTransBackTo() override; // +0x12C

	//-----------------------------------------------------------------------------
	// ABlobMenu
	//-----------------------------------------------------------------------------
	virtual void OnConfirmationClosed( TBOOL a_bYes ) override; // +0x138

	// SaveLoad's result of the save of the new profile
	void OnProfileSaved( TINT a_iResult );

private:
	// Finds the buttons and sets their texts
	void UpdateProfileButtons();

	// Creates the profile a_iProfile in the current save slot and saves it
	void CreateProfile( TINT a_iProfile );

private:
	TBOOL                    m_bSaving;                                // 0x140
	TBOOL                    m_bProfileCreated;                        // 0x141 presses the selected button in Tick
	EnSaveData::SaveGameSlot m_oSlotBackup;                            // 0x144 the slot before the new profile
	TINT                     m_iSelectedProfile;                       // 0x45C -1: none
	TWCHAR                   m_awszProfileNames[ NUM_PROFILES ][ 12 ]; // 0x460 "Profile_<n>"
	Toshi::TXUIControl*      m_apProfileButtons[ NUM_PROFILES ];       // 0x4C0 "Profile1" to "Profile4"
	TINT                     m_iUnk4D0;                                // 0x4D0 -1, never changed (yes quits the game if 0)
};

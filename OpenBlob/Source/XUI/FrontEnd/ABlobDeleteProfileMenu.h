#pragma once
#include "XUI/ABlobMenu.h"
#include "MemoryCard/EnSaveData.h"

//-----------------------------------------------------------------------------
// Purpose: the profile deletion: the buttons
// "Profile1" to "Profile4" show "Game <n>" for the used save slots; pressing a used one asks for a confirmation
// (confirmation.xur), then marks the slot unused and saves.
//-----------------------------------------------------------------------------
class ABlobDeleteProfileMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobDeleteProfileMenu, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_PROFILES = 4;

public:
	ABlobDeleteProfileMenu();
	~ABlobDeleteProfileMenu();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;     // +0x114
	virtual void TransBackTo() override; // +0x11C

	//-----------------------------------------------------------------------------
	// ABlobMenu
	//-----------------------------------------------------------------------------
	virtual void OnConfirmationClosed( TBOOL a_bYes ) override; // +0x138

	// SaveLoad's result of the save after the deletion
	void OnProfileSaved( TINT a_iResult );

private:
	// Finds the buttons and sets their texts
	void UpdateProfileButtons();

private:
	Toshi::TXUIControl*      m_apProfileButtons[ NUM_PROFILES ]; // 0x140 "Profile1" to "Profile4"
	TINT                     m_iSelectedProfile;                 // 0x150 -1: none
	EnSaveData::SaveGameSlot m_oSlotBackup;                      // 0x154 the slot after the deletion (restored if the save fails)
	TBOOL                    m_bSaving;                          // 0x46C
};

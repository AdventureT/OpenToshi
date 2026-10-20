#pragma once
#include "XUI/ABlobMenu.h"
#include "MemoryCard/EnSaveData.h"

//-----------------------------------------------------------------------------
// Purpose: the base of the profile name entries (no TClass
// of its own): ABlobGermanicNameEntry, ABlobJPNNameEntryScene and ABlobKONameSelect edit m_wszName, then create the
// profile of the current save slot with it, save it and go to the main menu (mainmenu.xur).
// NOTE: the name entries aren't ported (PC creates the profile in ABlobSplash)
//-----------------------------------------------------------------------------
class ABlobNameEntryCommon : public ABlobMenu
{
public:
	ABlobNameEntryCommon();
	~ABlobNameEntryCommon();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;        // +0x114
	virtual void TransFrom() override;      // +0x118
	virtual void TransBackFrom() override;  // +0x120
	virtual void EndTransBackTo() override; // +0x12C

	//-----------------------------------------------------------------------------
	// ABlobNameEntryCommon
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 006328e0
	virtual TINT GetMaxNameLength() { return 12; } // +0x144

	void SetName( const TWCHAR* a_wszName );
	void AddChar( TWCHAR a_wcChar );
	void RemoveChar();

	// Empties m_wszName
	void ClearName();

	// m_wszName isn't empty
	TBOOL HasName();

	// $deBlob: FUNCTION 00632150
	TWCHAR* GetName() { return m_wszName; }

	// The profile's name is the entered one
	void ApplyName();

	// Creates the profile of the current save slot and saves it
	void CreateProfile();

	// SaveLoad's result of the save of the new profile
	void OnProfileSaved( TINT a_iResult );

protected:
	// The profile is created: the main menu follows
	void GoToMainMenu();

protected:
	TWCHAR                   m_wszName[ 12 ];   // 0x140
	TINT                     m_iUnk158;         // 0x158 0
	TBOOL                    m_bSaving;         // 0x15C
	TBOOL                    m_bProfileCreated; // 0x15D
	EnSaveData::SaveGameSlot m_oSlotBackup;     // 0x160 the slot before the new profile
};

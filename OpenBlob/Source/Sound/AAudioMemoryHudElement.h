#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUITextBox.h>

//-----------------------------------------------------------------------------
// Purpose: a debug HUD of the audio memory (T2GUIElement): a line "Current usage = %d Available %d
// Percentage used %d" in the safe area and a list of up to 30 entries (name, size, share of the usage) chained below it.
// NOTE: retail never creates one (dead code).
//-----------------------------------------------------------------------------
class AAudioMemoryHudElement
    : public Toshi::T2GUIElement
{
public:
	static TCONSTEXPR_VAR TINT MAX_NUM_ENTRIES = 30;

	// One entry (0x43C bytes)
	struct Entry
	{
		// $deBlob: FUNCTION 005b17f0
		Entry() {}
		// $deBlob: FUNCTION 005b18e0
		~Entry() {}

		TWCHAR              m_wszName[ 256 ];      // 0x000
		TWCHAR              m_wszSize[ 64 ];       // 0x200 "Size %d"
		TWCHAR              m_wszPercentage[ 64 ]; // 0x280 " %f Percentage of used"
		Toshi::T2GUITextBox m_oNameText;           // 0x300
		Toshi::T2GUITextBox m_oSizeText;           // 0x368
		Toshi::T2GUITextBox m_oPercentageText;     // 0x3D0
		TINT                m_iSize;               // 0x438
	};

public:
	AAudioMemoryHudElement();
	virtual ~AAudioMemoryHudElement() override; // +0x00

	// Adds an entry a_szName below the last one: its size is a_iUsage minus the previous usage, or a_iSize if positive;
	// a_iUsage becomes the current usage
	void AddEntry( const TCHAR* a_szName, TINT a_iUsage, TINT a_iSize );
	// Removes all entries and the usage line (created again, hidden)
	void Reset();
	// Shows or hides the usage line and the entries
	void SetHudVisible( TBOOL a_bVisible );

private:
	void UpdateUsageText();
	void CreateUsageText();
	void UpdatePercentages();

private:
	TBOOL               m_bVisible;                    // 0x3C
	TWCHAR              m_wszUsage[ 256 ];             // 0x3E
	Toshi::T2GUITextBox m_oUsageText;                  // 0x240
	Entry               m_aEntries[ MAX_NUM_ENTRIES ]; // 0x2A8
	TINT                m_iUsage;                      // 0x81B0
	TINT                m_iNumEntries;                 // 0x81B4
	TINT                m_iAvailable;                  // 0x81B8 0xB00000
};

TSTATICASSERT( sizeof( AAudioMemoryHudElement ) == 0x81BC );

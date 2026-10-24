#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/TString16.h>
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUITextBox.h>
#include <GUI/T2GUIRectangle.h>
#include <Core/TMemory.h>

//-----------------------------------------------------------------------------
// Purpose: a debug screen of the memory pools (a T2GUIElement singleton): one line per pool (AMemory's two pools, the
// global heap "Mem1" and Havok's memory "PHYSICS") with a bar of the used (blue, red above 90%) and the free (green)
// part, refreshed every 0.1 s while shown.
// NOTE: retail never creates it (nothing references the constructor nor the toggle callbacks), so it's dead code kept as
// retail has it.
//-----------------------------------------------------------------------------
class AMemoryScreen
    : public Toshi::T2GUIElement
    , public Toshi::TSingleton<AMemoryScreen>
{
public:
	static TCONSTEXPR_VAR TINT NUM_POOLS = 4;

	// A line of the screen (0x194 bytes)
	struct Panel
	{
		Panel();
		~Panel();

		Toshi::T2GUITextBox   oText;    // 0x000
		Toshi::TString16      strText;  // 0x068
		Toshi::T2GUIRectangle oUsedBar; // 0x074 (below the text)
		Toshi::T2GUIRectangle oFreeBar; // 0x0D4 (right of the used bar)
		Toshi::T2GUIRectangle oUnk134;  // 0x134 (created but never shown)
	};

public:
	AMemoryScreen();
	virtual ~AMemoryScreen();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x04

	// Fills the safe area and creates the lines
	void Create();

	// Sets where the lines are (a T2GUIATTACHMENT: 0, 2, 4, 6 or 8)
	void SetMode( TINT a_iMode );

	// Shows/hides the screen
	void Toggle();

	// The callbacks of the debug menu (the singleton; return TTRUE)
	static TBOOL __stdcall OnToggle( void* a_pUnused );
	static TBOOL __stdcall OnTopLeft( void* a_pUnused );
	static TBOOL __stdcall OnTopRight( void* a_pUnused );
	static TBOOL __stdcall OnBottomLeft( void* a_pUnused );
	static TBOOL __stdcall OnBottomRight( void* a_pUnused );
	static TBOOL __stdcall OnCentre( void* a_pUnused );

private:
	void ClearTexts();
	void UpdatePools();
	void UpdateTextWidths();
	void UpdateLayout();
	void SetPoolInfo( TINT a_iIndex, const TCHAR* a_szName, const TMemoryHeapInfo& a_rInfo );

	Panel& GetPanel( TINT a_iIndex );

private:
	TFLOAT m_fUpdateTimer; // 0x3C
	TINT   m_eFont;        // 0x40 (AFontManager::FONTS)
	TINT   m_iMode;        // 0x44
	TBOOL  m_bVisible;     // 0x48
	// NOTE: retail's is a T2Array<Panel, 4> (constructed in place)
	Panel m_aPanels[ NUM_POOLS ]; // 0x4C
};                                // 0x69C

#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/TString16.h>
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUITextBox.h>
#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: a debug screen of the level's memory budgets (a T2GUIElement singleton): eight lines (asset and placed sizes
// of the instances, entities, FX and Blob, the numbers of instances and entities) with a bar of the used part of each
// budget (blue, red above 90%) and the rest (green).
// NOTE: retail never creates it and nothing fills ms_oStats (only the constructor and ResetStats clear it), so it's dead
// code kept as retail has it.
//-----------------------------------------------------------------------------
class ALevelMemStatsScreen
    : public Toshi::T2GUIElement
    , public Toshi::TSingleton<ALevelMemStatsScreen>
{
public:
	static TCONSTEXPR_VAR TINT NUM_LINES = 8;

	// The sizes the lines show (0x44 bytes); each size is the sum of two parts
	struct Stats
	{
		TINT iInstancesAsset[ 2 ];  // 0x00
		TINT iEntitiesAsset[ 2 ];   // 0x08
		TINT iFXAsset[ 2 ];         // 0x10
		TINT iBlobAsset[ 2 ];       // 0x18
		TINT iNumInstances;         // 0x20
		TINT iNumLandmarks;         // 0x24
		TINT iNumTrees;             // 0x28
		TINT iNumBuildings;         // 0x2C
		TINT iInstancesPlaced[ 2 ]; // 0x30
		TINT iNumEntities;          // 0x38
		TINT iEntitiesPlaced[ 2 ];  // 0x3C
	};

	// A line of the screen (0x134 bytes)
	struct Panel
	{
		Panel();
		~Panel();

		Toshi::T2GUITextBox   oText;    // 0x000
		Toshi::TString16      strText;  // 0x068
		Toshi::T2GUIRectangle oUsedBar; // 0x074 (below the text)
		Toshi::T2GUIRectangle oFreeBar; // 0x0D4 (right of the used bar)
	};

public:
	ALevelMemStatsScreen();
	virtual ~ALevelMemStatsScreen();

	// Shows/hides the screen
	void Toggle();

	// Updates the texts and the bars from ms_oStats
	void Refresh();

	static void ResetStats();

	// The callback of the debug menu (the singleton if any; returns TTRUE)
	static TBOOL __stdcall OnToggle( void* a_pUnused );

private:
	// Fills the safe area and creates the lines
	void Create();

	// Sets the bars of a line to a_iValue out of a_iMax
	void SetBar( TINT a_iIndex, TINT a_iValue, TINT a_iMax );

	Panel& GetPanel( TINT a_iIndex );

public:
	static Stats ms_oStats;

private:
	TINT  m_iMode;    // 0x3C (the T2GUIATTACHMENT of the lines)
	TBOOL m_bVisible; // 0x40
	// NOTE: retail's is a T2Array<Panel, 8> (constructed in place)
	Panel m_aPanels[ NUM_LINES ]; // 0x44
};                                // 0x9E4

#pragma once
#include "EnSaveData.h"
#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Purpose: the autosave of the consoles (created by
// AAppInitState, deleted by AApplication::OnDestroy). On PC it's a leftover: Save only backs the current save slot up
// and copies AApplication's default profile (m_pSaveData2) over it in memory, Update does nothing, so the state never
// goes back to idle and nothing is written (the PC saves through SaveLoad from the menus: ABlobScoreMenu,
// ABlobSprintSummaryMenu...). Save is never reached on PC: its only caller is the IO autosave action and no IO entity
// gets IO data (AIOEntity::SetIOEntityData has no callers).
//-----------------------------------------------------------------------------
class AAutosaveManager
    : public Toshi::TSingleton<AAutosaveManager>
{
public:
	enum STATE : TINT
	{
		STATE_Idle,
		STATE_Saving,
	};

public:
	AAutosaveManager();
	virtual ~AAutosaveManager(); // +0x00

	// Does nothing on PC (the console version runs the save and its icon)
	// $deBlob: FUNCTION 005e2730
	void Update( TFLOAT a_fDeltaTime ) {} // DOES_NOTHING

	// Starts an autosave (the level scripts' autosave action): backs the current slot up and copies
	// AApplication's default profile (m_pSaveData2) over it
	void Save();

	// The result of the save (a_iResult: 1 success, -8...); the slot is restored unless it succeeded. NOTE: unused
	// on PC
	void OnSaveFinished( TINT a_iResult );

	// Puts the backup back into the current slot
	void RestoreSlot();

private:
	STATE                    m_eState;         // 0x004
	TINT                     m_iUnk8;          // 0x008 3 (the console version's icon state)
	TBOOL                    m_bEnabled;       // 0x00C TTRUE
	EnSaveData::SaveGameSlot m_oBackup;        // 0x010 the current slot before the save
	void*                    m_pAutosaveState; // 0x328 TNULL (the console version's stall state)
	TBOOL                    m_bUnk32C;        // 0x32C
};

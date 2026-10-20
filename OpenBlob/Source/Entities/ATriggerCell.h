#pragma once
#include "Entities/ABaseEntity.h"

namespace TerrainTypes
{
struct Cell;
}

//-----------------------------------------------------------------------------
// Purpose: hides or shows terrain cells when triggered (ATriggerCell\ATriggerCell.cpp): MESSAGE_TRIGGER shows ("State"
// set) or hides (unloading them) the cells of "TriggerCellList" of the current region, or all the other cells with
// "InvertCells"; once only until MESSAGE_RESET resets it.
//-----------------------------------------------------------------------------
class ATriggerCell
    : public ABaseEntity
{
public:
	TDECLARE_CLASS( ATriggerCell, ABaseEntity );

	enum CELLFLAG : TUINT8
	{
		CELLFLAG_Show      = 0x1, // "State": the cells are shown, else hidden
		CELLFLAG_Invert    = 0x4, // "InvertCells": every cell but the listed ones
		CELLFLAG_Triggered = 0x8,
	};

public:
	ATriggerCell();
	virtual ~ATriggerCell() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8

private:
	// Shows a_pCell, or hides it: a loaded (or queued) cell is unloaded, a loading one cancelled
	static void SetCellShown( TerrainTypes::Cell* a_pCell, TBOOL a_bShow );

private:
	TINT*  m_cells;       // 0x80
	TUINT  m_numCells;    // 0x84
	TUINT8 m_uiCellFlags; // 0x88 CELLFLAG
};

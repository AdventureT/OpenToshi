#include "pch.h"
#include "ATriggerCell.h"
#include "AEntityParams.h"
#include "AEntityIOPacket.h"
#include "Terrain2/ATerrain2.h"
#include "Terrain2/ATerrain2Interface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ATriggerCell ) == 0x8C );

// $deBlob: FUNCTION 007a6230
// $deBlob: FUNCTION 007a6240
// $deBlob: FUNCTION 007a62f0
// $deBlob: FUNCTION 007a63a0
// $deBlob: FUNCTION 007a63b0
// $deBlob: FUNCTION 00541120
// $deBlob: FUNCTION 00859ed0
TDEFINE_CLASS( ATriggerCell );

// $deBlob: FUNCTION 007a63c0
ATriggerCell::ATriggerCell()
{
	m_cells       = TNULL;
	m_numCells    = 0;
	m_uiCellFlags = 0;
	SetHash( TFALSE );
	SetTicking( TFALSE );
	m_Flags &= ~FLAG_DontTick;
}

// $deBlob: FUNCTION 007a6450
// $deBlob: FUNCTION 007a67f0
ATriggerCell::~ATriggerCell()
{
	delete[] m_cells;
}

// $deBlob: FUNCTION 007a6470
void ATriggerCell::ProcessParams( const AEntityParams& a_rParams )
{
	ABaseEntity::ProcessParams( a_rParams );

	TUINT32 uiState = 0;
	a_rParams.GetUInt( uiState, "State" );

	if ( uiState != 0 )
		m_uiCellFlags |= CELLFLAG_Show;

	TBOOL bInvert = TFALSE;
	a_rParams.GetBool( bInvert, "InvertCells" );

	if ( bInvert )
		m_uiCellFlags |= CELLFLAG_Invert;

	TINT* pCells = TNULL;
	m_numCells   = a_rParams.GetArray( pCells, "TriggerCellList" );
	TASSERT( m_numCells > 0 );

	m_cells = new TINT[ m_numCells ];
	TUtil::MemCopy( m_cells, pCells, m_numCells * sizeof( TINT ) );
}

// $deBlob: FUNCTION 007a6560
void ATriggerCell::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TRIGGER && !( m_uiCellFlags & CELLFLAG_Triggered ) )
	{
		ATerrain2* terrain = ATerrain2::GetInstance();
		TINT       region  = ATerrain2Interface::GetInstance()->GetCurrentRegion();

		if ( m_uiCellFlags & CELLFLAG_Invert )
		{
			TINT iNumCells = terrain->GetNumCells( region );

			for ( TINT i = 0; i < iNumCells; i++ )
			{
				TUINT j;

				for ( j = 0; j < TUINT( m_numCells ); j++ )
				{
					TASSERT( m_cells[ j ] >= 0 && m_cells[ j ] < terrain->GetNumCells( region ) );

					if ( i == m_cells[ j ] )
						break;
				}

				if ( j >= TUINT( m_numCells ) )
					SetCellShown( terrain->GetCell( region, i ), m_uiCellFlags & CELLFLAG_Show );
			}
		}
		else
		{
			for ( TUINT i = 0; i < TUINT( m_numCells ); i++ )
			{
				TASSERT( m_cells[ i ] >= 0 && m_cells[ i ] < terrain->GetNumCells( region ) );
				SetCellShown( terrain->GetCell( region, m_cells[ i ] ), m_uiCellFlags & CELLFLAG_Show );
			}
		}

		m_uiCellFlags |= CELLFLAG_Triggered;
		return;
	}

	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_RESET )
		Reset( TNULL );
}

// $deBlob: FUNCTION 007a6760
void ATriggerCell::Reset( ABaseEntity* a_pEntity )
{
	m_uiCellFlags &= ~CELLFLAG_Triggered;
	ABaseEntity::Reset( a_pEntity );
}

// $deBlob: FUNCTION 007a6780
void ATriggerCell::SetCellShown( TerrainTypes::Cell* a_pCell, TBOOL a_bShow )
{
	if ( a_bShow )
	{
		a_pCell->m_uiFlags &= ~TerrainTypes::CellRuntimeFlag_Hidden;
		return;
	}

	TINT region = ATerrain2Interface::GetInstance()->GetCurrentRegion();
	a_pCell->m_uiFlags |= TerrainTypes::CellRuntimeFlag_Hidden;

	if ( a_pCell->m_uiFlags & ( TerrainTypes::CellRuntimeFlag_Loaded | TerrainTypes::CellRuntimeFlag_QueuedLoad ) )
	{
		ATerrain2* pTerrain = ATerrain2::GetInstance();
		pTerrain->RequestCell( region, pTerrain->GetCellIndex( region, a_pCell ), TFALSE, TFALSE );
	}
	else if ( a_pCell->m_uiFlags & TerrainTypes::CellRuntimeFlag_Loading )
	{
		a_pCell->m_uiFlags |= TerrainTypes::CellRuntimeFlag_CancelLoad;
	}
}

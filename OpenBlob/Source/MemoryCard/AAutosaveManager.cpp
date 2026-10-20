#include "pch.h"
#include "AAutosaveManager.h"
#include "AApplication.h"
#include "Movie/AMoviePlayer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AAutosaveManager ) == 0x330 );

// $deBlob: FUNCTION 005e25e0
AAutosaveManager::AAutosaveManager()
{
	m_eState         = STATE_Idle;
	m_iUnk8          = 3;
	m_bEnabled       = TTRUE;
	m_pAutosaveState = TNULL;
	m_bUnk32C        = TFALSE;
}

// $deBlob: FUNCTION 005e26a0
// $deBlob: FUNCTION 005e2a10
AAutosaveManager::~AAutosaveManager()
{
	TASSERT( STATE_Idle == m_eState );
}

// $deBlob: FUNCTION 005e2740
void AAutosaveManager::Save()
{
	TASSERT( TFALSE == AMoviePlayer::GetSingleton()->IsMoviePlaying(), "Autosave triggered while movie is playing!" );
	TASSERT( STATE_Idle == m_eState, "Autosave already in progress! Separate autosave triggers more." );
	TASSERT( TNULL == m_pAutosaveState );

	if ( m_bEnabled && !AMoviePlayer::GetSingleton()->IsMoviePlaying() && m_eState == STATE_Idle )
	{
		m_eState = STATE_Saving;

		EnSaveData*               pSaveData = TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData );
		EnSaveData::SaveGameSlot& rSlot     = pSaveData->GetSlot( g_oTheApp.m_iCurrentSaveSlot );

		TUtil::MemCopy( &m_oBackup, &rSlot, sizeof( EnSaveData::SaveGameSlot ) );
		rSlot.m_bUsedSlot = TTRUE;

		// NOTE: AApplication's m_pSaveData2 is a default profile nothing ever writes (AApplication::CreateSaveData): this
		// would reset the current profile in memory, and the next save would write it. Unreachable on PC: the only
		// caller is the IO autosave action, and no IO entity gets IO data
		TUtil::MemCopy( &rSlot.m_oData, g_oTheApp.m_pSaveData2, sizeof( EnSaveData::SlotData ) );
	}
}

// $deBlob: FUNCTION 005e2860
void AAutosaveManager::OnSaveFinished( TINT a_iResult )
{
	m_eState = STATE_Idle;

	if ( a_iResult != -8 )
	{
		if ( a_iResult != 1 )
			RestoreSlot();
	}
	else
	{
		RestoreSlot();
		m_eState = STATE_Idle;
	}
}

// $deBlob: FUNCTION 005e28e0
void AAutosaveManager::RestoreSlot()
{
	EnSaveData* pSaveData = TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData );
	TUtil::MemCopy( &pSaveData->GetSlot( g_oTheApp.m_iCurrentSaveSlot ), &m_oBackup, sizeof( EnSaveData::SaveGameSlot ) );
}

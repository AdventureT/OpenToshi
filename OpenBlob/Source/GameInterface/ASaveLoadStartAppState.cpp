#include "pch.h"
#include "ASaveLoadStartAppState.h"
#include "GUI/AGUIManager.h"
#include "AFrontEndMovieState.h"
#include "AGameStateController.h"
#include "AApplication.h"
#include "APlayerProfileManager.h"
#include "Memory/AMemory.h"
#include "Options/AOptions.h"
#include "MemoryCard/SaveLoad.h"

#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005d5560
// $deBlob: FUNCTION 005d5570
// $deBlob: FUNCTION 005d55f0
// $deBlob: FUNCTION 005d5670
// $deBlob: FUNCTION 005d5680
// $deBlob: FUNCTION 0053c910
// $deBlob: FUNCTION 008585b0
TDEFINE_CLASS( ASaveLoadStartAppState );

// $deBlob: FUNCTION 005d4460
ASaveLoadStartAppState::ASaveLoadStartAppState()
{
}

// $deBlob: FUNCTION 005d4520
// $deBlob: FUNCTION 005d44c0
ASaveLoadStartAppState::~ASaveLoadStartAppState()
{
}

// $deBlob: FUNCTION 005d5720
AGameState::UpdateResult ASaveLoadStartAppState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( SaveLoad::GetInstance()->GetError() == SaveLoad::ERROR_NONE )
		return AGameState::OnUpdate( a_fDeltaTime );

	// Loading failed: the save data the application had before
	if ( SaveLoad::GetInstance()->GetError() != SaveLoad::ERROR_OK )
		TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData )->Copy( &m_oSaveData );

	EnSaveData::SaveData* pSaveData = TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData )->GetSaveData();

	for ( TINT i = 0; i < EnSaveData::SAVEGAME_SLOT_COUNT; i++ )
	{
		TASSERT( i >= 0 && i < EnSaveData::SAVEGAME_SLOT_COUNT );

		if ( !pSaveData->m_aSaveGameSlots[ i ].m_bUsedSlot )
			APlayerProfileManager::GetSingleton()->SetProfileUnused( i );
	}

	AOptions::GetSingleton()->Apply( TFALSE );

	TXUI::GetSingleton()->Reset();
	return UpdateResult_Remove;
}

// $deBlob: FUNCTION 005d5690
void ASaveLoadStartAppState::OnInsertion()
{
	TXUI* pXUI = TXUI::GetSingleton();
	pXUI->CreateScene( pXUI->FindResource( "black.xur" ), TUINT32( 0 ) );

	m_oSaveData.Copy( TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData ) );

	SaveLoad::GetInstance()->StartupLoad( 1 );

	AGUIManager::GetSingleton()->SetCursorEnabled( TFALSE );
}

// $deBlob: FUNCTION 005d56e0
void ASaveLoadStartAppState::OnRemoval()
{
	AGameState::OnRemoval();

	AFrontEndMovieState* pState = new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) AFrontEndMovieState();
	pState->SetState( AFrontEndMovieState::STATE_LEGAL );
	AGameStateController::GetSingleton()->PushState( pState );
}

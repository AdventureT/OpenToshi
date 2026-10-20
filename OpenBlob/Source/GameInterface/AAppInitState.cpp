#include "pch.h"
#include "AModelViewState.h"
#include "Console/AConsole.h"
#include "Console/AStaticCVars.h"
#include "Sound/ASoundTextureSwapMap.h"
#include "Cheats/ACheatManager.h"
#include "Render/Decal/ADecalManager.h"
#include "Sound/APositionalSoundManager.h"
#include "Entities/APickupManager.h"
#include <Physics/Havok/TPhysicsManager.h>
#include "Collision/ACollisionManager2.h"
#include "AAppInitState.h"
#include "Sound/AStreamingSoundPlayer.h"
#include "AGameStateController.h"
#include "AApplication.h"
#include "AAssetStreaming.h"
#include "Render/ARenderer.h"
#include "AXUIState.h"
#include "APlayerProfileManager.h"
#include "APlayerManager.h"
#include "ALevelInformation.h"
#include "GameController/AGameStatus.h"
#include "AFrontEndMovieState.h"
#include "Movie/AMoviePlayer_DX11.h"
#include "Memory/AMemory.h"
#include "Render/ModelLoading/AModelLoader.h"
#include "Animation/AModelFactory.h"
#include "Buildings/ABuildingManager.h"
#include "Platform/DX11/AJellyShader_DX11.h"
#include "Options/AOptions.h"
#include "MemoryCard/SaveLoad_Win.h"
#include "ARandom.h"
#include "Subtitles/ASubtitleManager.h"
#include <GUI/T2GUITextureSectionManager.h>
#include <T2Locale/T2Locale.h>
#include "AAchievements.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundLoadHelper.h"
#include "Sound/AMusicManager.h"
#include "Challenges/AChallengeManager.h"

#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "FXComposition/AFXCompositionManager.h"

#include "Render/Material/AMaterialManager.h"
#include <TParticleSystem/TParticleSystem.h>
#include "MemoryCard/AAutosaveManager.h"
#include "Input/AVibrationManager.h"
#include "Sound/APhysicsSoundManager.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005c1550
// $deBlob: FUNCTION 005c1560
// $deBlob: FUNCTION 005c15d0
// $deBlob: FUNCTION 005c1640
// $deBlob: FUNCTION 005c1650
// $deBlob: FUNCTION 0053c500
// $deBlob: FUNCTION 00858490
TDEFINE_CLASS( AAppInitState );

// $deBlob: FUNCTION 005c1660
AAppInitState::AAppInitState()
{
}

// $deBlob: FUNCTION 005c1680
// $deBlob: FUNCTION 005c1c60
AAppInitState::~AAppInitState()
{
}

// $deBlob: FUNCTION 005c1690
AGameState::UpdateResult AAppInitState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( AAssetStreaming::GetSingleton()->HasActiveJobs() )
	{
		ARenderer::GetSingleton()->SetBackgroundColour( 0, 0, 0 );
		return AGameState::OnUpdate( a_fDeltaTime );
	}

	AConsole::RegisterCommands();

	TString8 strLevelInformation;
	strLevelInformation.Format( "data/%s.trb", g_oTheApp.GetLevelPropertiesFile() );

	ALevelInformation* pLevelInformation = new ALevelInformation();
	pLevelInformation->Create( strLevelInformation.GetString() );

	g_oTheApp.CreateSaveData();

	new SaveLoadSKU();

	new AOptions();

	new ARandom();

	new AVibrationManager();

	if ( AVibrationManager::GetSingleton() != TNULL )
		AVibrationManager::GetSingleton()->EnableAllVibrators( TTRUE );

	new AFXCompositionManager();

	new APickupManager();

	new AGameStatus();
	new APlayerManager();

	new APlayerProfileManager();

	new ACheatManager();

	new AAchievements();

	new ADecalManager();

	AStaticCVars::RegisterStaticCVars();

	if ( TSound::GetSingleton() != TNULL )
		ASoundInterface::LoadEventSystem();

	new AMusicManager();

	new ASoundLoadHelper();

	ADX11MoviePlayer::Create();
	AMoviePlayer::GetSingleton()->OnCreate();

	new ASubtitleManager();
	ASubtitleManager::GetSingleton()->LoadSubtitles( T2Locale::GetSingleton()->GetLanguage() );
	T2GUITextureSectionManager::UpdateMaterials();

	TParticleSystem::s_oCreateData.m_iNumSystems = 80;
	new TParticleSystem( TParticleSystem::s_oCreateData );

	new AAutosaveManager();

	Toshi::TPhysicsManager::Create();

	new ACollisionManager2();

	new AMaterialManager();

	new AModelLoader();

	new AModelFactory();

	new AJellyShader_DX11();

	new ABuildingManager();
	new AChallengeManager();

	new APositionalSoundManager();

	new APhysicsSoundManager();

	new AStreamingSoundPlayer();

	new ASoundTextureSwapMap();

	return UpdateResult_Remove;
}

// $deBlob: FUNCTION 005c1bd0
void AAppInitState::OnInsertion()
{
	TMemoryHeap* pOldTRBHeap = AMemory::SetTRBHeap( TXUI::ms_pXUIMemoryBlock );
	AMemory::SetTRBHeap( pOldTRBHeap );

	AGameState::OnInsertion();
}

// $deBlob: FUNCTION 005c1bf0
void AAppInitState::OnRemoval()
{
	AGameState::OnRemoval();

	if ( g_oTheApp.ShouldLoadModelViewState() == TFALSE )
	{
		AFrontEndMovieState::StartBlobFrontendMenu();
	}
	else
	{
		AGameStateController::GetSingleton()->PushState( new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) AModelViewState() );
	}
}

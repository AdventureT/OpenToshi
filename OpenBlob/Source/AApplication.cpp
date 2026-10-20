#include "pch.h"
#include "Animation/AAnimationPoseFactory.h"
#include "Console/AStaticCVars.h"
#include "Cheats/ACheatManager.h"
#include "Render/Decal/ADecalManager.h"
#include "Sound/APositionalSoundManager.h"
#include "Entities/APickupManager.h"
#include "Buildings/ABuildingManager.h"
#include "Platform/DX11/AJellyShader_DX11.h"
#include "AApplication.h"
#include "Collision/ACollisionManager2.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Animation/AModelFactory.h"
#include "ALevelInformation.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "SteamManager.h"
#include "AAchievements.h"
#include "AAssetStreaming.h"
#include "Memory/AMemory.h"
#include <Toshi/TScheduler.h>
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include "Cameras/ACameraManager.h"
#include "Input/AInputManager2.h"
#include "Input/AVibrationManager.h"
#include "XUI/InGame/ABlobPauseMenu.h"
#include "Entities/AEntityFactory.h"
#include <Platform/Windows/Input/TInputDeviceKeyboard_Win.h>
#include <XUI/TXUIInput.h>
#include <XUI/TXUICanvas.h>
#include "Sound/APhysicsSoundManager.h"
#include "Options/AOptions.h"
#include "GUI/AGUIManager.h"
#include "GUI/AFontManager.h"
#include "Console/AConsole.h"
#include "Sound/ASoundTextureSwapMap.h"
#include "Locale/ALocaleManager.h"
#include "GameInterface/AGameStateController.h"
#include "MemoryCard/EnSaveData.h"
#include "MemoryCard/SaveLoad.h"
#include "Movie/AMoviePlayer.h"
#include "ARandom.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundLoadHelper.h"
#include "Sound/AMusicManager.h"
#include "Sound/AStreamingSoundPlayer.h"

#include <GUI/T2GUI.h>
#include "Challenges/AChallengeManager.h"
#include "Render/ModelLoading/AModelLoader.h"
#include "Entities/AEntityData.h"
#include "GameInterface/AXUIState.h"
#include "GameInterface/AGameState.h"
#include "GameInterface/APlayerProfileManager.h"
#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <GUI/T2GUIRenderer.h>
#include <XUI/TXUI.h>
#include <Sound/TSound.h>

#include <Toshi/TSystem.h>
#include <File/TFile.h>
#include <File/TIniFile.h>

#ifdef TRENDERINTERFACE_DX11
#  include <Platform/DX11/TRender_DX11.h>
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "FXComposition/AFXCompositionManager.h"
#include "Render/Material/AMaterialManager.h"
#include <TParticleSystem/TParticleSystem.h>
#include "GameInterface/AInGameState.h"
#include "MemoryCard/AAutosaveManager.h"
#include "Subtitles/ASubtitleManager.h"
#include <Physics/Havok/TPhysicsManager.h>

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TCHAR* const AApplication::scm_sDefaultLevelPropertiesFileName = "Levels";
const TCHAR*       AApplication::sm_ForcedLocale;

// The sounds are paused while the system is (OnUpdate)
static TBOOL  s_bSoundsPaused    = TFALSE;
static TFLOAT s_fInputPauseTime  = 0.0f; // the XUI input is disabled while > 0 (the window lost the focus)
static TBOOL  s_bInputWasEnabled = TTRUE;

// $deBlob: FUNCTION 00857f30
// $deBlob: FUNCTION 0053b620
AApplication g_oTheApp;

// $deBlob: FUNCTION 0056a990
AApplication::AApplication()
{
	m_uiUnk30                   = 0;
	m_pRenderer                 = TNULL;
	m_pGameStateController      = TNULL;
	m_pSaveData                 = TNULL;
	m_pSaveData2                = TNULL;
	m_iCurrentSaveSlot          = 0;
	m_bRenderWorld              = TFALSE;
	m_bGamePaused               = TFALSE;
	m_bInitialised              = TFALSE;
	m_fMaxDeltaTime             = 0.25f;
	m_bUnk54                    = TFALSE;
	m_iWidth                    = -1;
	m_iHeight                   = -1;
	m_bWindowed                 = TFALSE;
	m_bExclusiveFullscreen      = TFALSE;
	m_bUnk62                    = TFALSE;
	m_bLoadModelViewState       = TFALSE;
	m_bFXView                   = TFALSE;
	m_bSkipFrontend             = TFALSE;
	// Windowed when started from a debugger
	m_bWindowed = IsDebuggerPresent();

	m_szLevelPropertiesFileName = scm_sDefaultLevelPropertiesFileName;
	SetVerbose( TFALSE );
}

// $deBlob: FUNCTION 0056be00
// $deBlob: FUNCTION 0056aa20
AApplication::~AApplication()
{
}

// $deBlob: FUNCTION 0056aa30
TBOOL AApplication::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( a_fDeltaTime > m_fMaxDeltaTime )
		a_fDeltaTime = m_fMaxDeltaTime;

#ifdef TRENDERINTERFACE_DX11
	TRenderDX11::Interface()->GetMSWindow()->Update();
#endif

	if ( SaveLoad::GetInstance() != TNULL )
		SaveLoad::GetInstance()->Update( a_fDeltaTime );

	SteamManager::GetInstance();
	SteamAPI_RunCallbacks();

	if ( AAchievements::GetSingleton() != TNULL )
		AAchievements::GetSingleton()->Update( a_fDeltaTime );

	m_fTotalTime = TSystemManager::GetSingleton()->GetScheduler()->GetTotalTime();

	a_fDeltaTime      = AStaticCVars::cvSlowTime.GetFloat() * a_fDeltaTime;
	TFLOAT fDeltaTime = a_fDeltaTime;

	if ( TSystemManager::GetSingleton()->IsPaused() )
	{
		if ( TSound::GetSingleton() != TNULL && !s_bSoundsPaused )
		{
			ASoundInterface::SetCategoryPausedAll( TTRUE );

			if ( AStreamingSoundPlayer::GetSingleton() != TNULL )
			{
				AStreamingSoundPlayer::GetSingleton()->StopAll();
				AStreamingSoundPlayer::GetSingleton()->SetPaused( TTRUE, TFALSE );
			}

			if ( AMusicManager::GetSingleton() != TNULL )
				AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.5f );

			FMOD::ChannelGroup* pMasterChannelGroup;
			FMOD::System*       pSystem = TSound::GetSingleton()->GetSystem();
			s_bSoundsPaused             = TTRUE;
			pSystem->getMasterChannelGroup( &pMasterChannelGroup );
			pMasterChannelGroup->setPaused( true );

			// The XUI input is disabled for a while
			if ( s_fInputPauseTime <= 0.0f )
			{
				s_fInputPauseTime  = 0.3f;
				s_bInputWasEnabled = TXUI::GetSingleton()->GetInput()->IsEnabled();
				TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );
				TUtil::Log( "Pause Input (WasEnabled  =%s)", s_bInputWasEnabled ? "true" : "false" );
			}
		}
		UpdateSound( fDeltaTime );
		return TTRUE;
	}

	if ( s_bSoundsPaused )
	{
		s_bSoundsPaused = TFALSE;
		ASoundInterface::SetCategoryPausedAll( TFALSE );

		FMOD::ChannelGroup* pMasterChannelGroup;
		TSound::GetSingleton()->GetSystem()->getMasterChannelGroup( &pMasterChannelGroup );
		pMasterChannelGroup->setPaused( false );

		if ( AStreamingSoundPlayer::GetSingleton() != TNULL )
			AStreamingSoundPlayer::GetSingleton()->SetPaused( TFALSE, TFALSE );

		if ( AMusicManager::GetSingleton() != TNULL )
		{
			AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.5f );
			AMusicManager::GetSingleton()->ResetPitch( 0.5f );
		}

		// The pause menu (if open) is paused, the keys are released
		TXUIControl* pPauseMenu = TXUI::GetSingleton()->GetCanvas()->FindChildXUIControl( L"PauseMenu" );

		if ( pPauseMenu != TNULL )
			TSTATICCAST( ABlobPauseMenu, pPauseMenu )->Pause();

		TSTATICCAST( TInputDXDeviceKeyboard, TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 ) )->ClearAllKeys();
	}

	// The XUI input gets its state back once the pause time is over
	if ( 0.0f < s_fInputPauseTime )
	{
		s_fInputPauseTime -= a_fDeltaTime;

		if ( s_fInputPauseTime <= 0.0f )
		{
			s_fInputPauseTime = 0.0f;
			TUtil::Log( "Enable Input (%s)", s_bInputWasEnabled ? "true" : "false" );
			TXUI::GetSingleton()->GetInput()->SetEnabled( s_bInputWasEnabled );
		}
	}

	if ( !TApplication::OnUpdate( fDeltaTime ) )
		return TFALSE;

	if ( T2GUI::GetSingleton() != TNULL )
		T2GUI::GetSingleton()->Tick( fDeltaTime );

	if ( TXUI::GetSingleton() != TNULL )
		TXUI::GetSingleton()->Tick( fDeltaTime );

	if ( AAssetStreaming::GetSingleton() )
		AAssetStreaming::GetSingleton()->Update( fDeltaTime );

	if ( AAnimationPoseFactory::GetSingleton() )
		AAnimationPoseFactory::GetSingleton()->Update();

	if ( AInputManager2::GetSingleton() )
		AInputManager2::GetSingleton()->Update( fDeltaTime );

	if ( AVibrationManager::GetSingleton() != TNULL )
		AVibrationManager::GetSingleton()->Update( fDeltaTime );

	if ( ACameraManager::GetSingleton() != TNULL )
		ACameraManager::GetSingleton()->Update( fDeltaTime );

	if ( AGUIManager::GetSingleton() != TNULL )
		AGUIManager::GetSingleton()->Tick( fDeltaTime );

	if ( AFXCompositionManager::GetSingleton() != TNULL )
		AFXCompositionManager::GetSingleton()->Update( a_fDeltaTime );

	if ( AMoviePlayer::GetSingleton() != TNULL )
		AMoviePlayer::GetSingleton()->OnUpdate( a_fDeltaTime );

	if ( ASubtitleManager::GetSingleton() != TNULL )
		ASubtitleManager::GetSingleton()->Update( a_fDeltaTime );

	if ( AAutosaveManager::GetSingleton() != TNULL )
		AAutosaveManager::GetSingleton()->Update( a_fDeltaTime );

	if ( ATerrain2Interface::GetInstance() != TNULL )
		ATerrain2Interface::GetInstance()->Update( a_fDeltaTime );

	if ( !m_bGamePaused && m_bInitialised )
	{
		if ( TParticleSystem::GetSingleton() != TNULL )
		{
			AGameState* pCurrentState = m_pGameStateController->GetCurrentGameState();

			if ( !pCurrentState->IsA( &TGetClass( AInGameState ) ) || !TSTATICCAST( AInGameState, pCurrentState )->IsPaused() )
				TParticleSystem::GetSingleton()->Update( a_fDeltaTime );
		}

		if ( m_pGameStateController )
			m_pGameStateController->Update( fDeltaTime );

		UpdateSound( fDeltaTime );
	}
	else
	{
		AGameState* pState = m_pGameStateController->GetCurrentGameState();

		if ( pState && pState->WasControllerRemoved() )
		{
			m_pGameStateController->Update( fDeltaTime );
			UpdateSound( fDeltaTime );

			if ( AEntityFactory::GetSingleton() != TNULL && m_bRenderWorld == TTRUE )
				AEntityFactory::GetSingleton()->Tick( a_fDeltaTime, 1 );
		}
	}

	TPhysicsManager::GetSingleton()->UpdateBatches( a_fDeltaTime );

	m_pRenderer->Update( fDeltaTime );
	return TTRUE;
}

// $deBlob: FUNCTION 0056b010
TBOOL AApplication::OnDestroy()
{
	m_bInitialised = TFALSE;

	if ( !TApplication::OnDestroy() )
		return TFALSE;

	if ( AAutosaveManager::GetSingleton() != TNULL )
		delete AAutosaveManager::GetSingleton();

	if ( TRender::GetSingleton() != TNULL )
		TRender::GetSingleton()->FlushDyingResources();

	if ( m_pGameStateController )
	{
		m_pGameStateController->ResetStack();

		m_pGameStateController->Destroy();

		delete m_pGameStateController;
		m_pGameStateController = TNULL;
	}

	if ( ADecalManager::GetSingleton() != TNULL )
		delete ADecalManager::GetSingleton();

	if ( AJellyShader_DX11::GetSingleton() != TNULL )
		delete AJellyShader_DX11::GetSingleton();

	AMusicManager::GetSingleton()->UnloadMusicData();

	if ( AMusicManager::GetSingleton() != TNULL )
		delete AMusicManager::GetSingleton();

	if ( ASoundLoadHelper::GetSingleton() != TNULL )
		delete ASoundLoadHelper::GetSingleton();

	if ( ASubtitleManager::GetSingleton() != TNULL )
		delete ASubtitleManager::GetSingleton();

	AMoviePlayer::GetSingleton()->OnDestroy();

	if ( AMoviePlayer::GetSingleton() != TNULL )
		delete AMoviePlayer::GetSingleton();

	if ( SaveLoad::GetInstance() != TNULL )
		delete SaveLoad::GetInstance();

	if ( m_pSaveData != TNULL )
		delete TSTATICCAST( EnSaveData, m_pSaveData );

	m_pSaveData = TNULL;
	delete TSTATICCAST( EnSaveData::SlotData, m_pSaveData2 );
	m_pSaveData2 = TNULL;

	if ( APlayerProfileManager::GetSingleton() != TNULL )
		delete APlayerProfileManager::GetSingleton();

	if ( APlayerManager::GetSingleton() != TNULL )
		delete APlayerManager::GetSingleton();

	if ( APickupManager::GetSingleton() != TNULL )
		delete APickupManager::GetSingleton();

	if ( ACollisionManager2::GetSingleton() != TNULL )
		delete ACollisionManager2::GetSingleton();

	if ( TPhysicsManager::GetSingleton() != TNULL )
		delete TPhysicsManager::GetSingleton();

	if ( ACameraManager::GetSingleton() != TNULL )
		delete ACameraManager::GetSingleton();

	if ( AModelFactory::GetSingleton() != TNULL )
		delete AModelFactory::GetSingleton();

	if ( ABuildingManager::GetSingleton() != TNULL )
		delete ABuildingManager::GetSingleton();

	if ( AChallengeManager::GetSingleton() != TNULL )
		delete AChallengeManager::GetSingleton();

	if ( APhysicsSoundManager::GetSingleton() != TNULL )
		delete APhysicsSoundManager::GetSingleton();

	if ( ASoundTextureSwapMap::GetSingleton() != TNULL )
		delete ASoundTextureSwapMap::GetSingleton();

	if ( AModelLoader::GetSingleton() != TNULL )
		delete AModelLoader::GetSingleton();

	if ( AMaterialManager::GetSingleton() != TNULL )
		delete AMaterialManager::GetSingleton();

	if ( AEntityData::GetSingleton() != TNULL )
	{
		AEntityData::GetSingleton()->Unload();

		if ( AEntityData::GetSingleton() != TNULL )
			delete AEntityData::GetSingleton();
	}

	if ( AGameStatus::GetSingleton() != TNULL )
		delete AGameStatus::GetSingleton();

	AGameState::DestroyLoadIcon();
	T2GUITextureSectionManager::Close();

	if ( APositionalSoundManager::GetSingleton() != TNULL )
		delete APositionalSoundManager::GetSingleton();

	if ( AStreamingSoundPlayer::GetSingleton() != TNULL )
		delete AStreamingSoundPlayer::GetSingleton();

	if ( ACheatManager::GetSingleton() != TNULL )
		delete ACheatManager::GetSingleton();

	if ( AAchievements::GetSingleton() != TNULL )
		delete AAchievements::GetSingleton();

	if ( AGUIManager::GetSingleton() != TNULL )
		delete AGUIManager::GetSingleton();

	if ( AOptions::GetSingleton() != TNULL )
		delete AOptions::GetSingleton();

	if ( AFXCompositionManager::GetSingleton() != TNULL )
		delete AFXCompositionManager::GetSingleton();

	if ( TParticleSystem::GetSingleton() != TNULL )
		delete TParticleSystem::GetSingleton();

	AFontManager::Destroy();

	TXUI::GetSingleton()->Reset();
	AXUIState::UnloadSkin();
	AXUIState::UnloadInGameSkin();
	AXUIState::UnloadCommonSkin();
	AXUIState::UnloadFontSkin();

	if ( TXUI::GetSingleton() != TNULL )
		delete TXUI::GetSingleton();

	if ( TSound::GetSingleton() != TNULL )
	{
		TSound::GetSingleton()->Destroy();
		delete TSound::GetSingleton();
	}

	if ( m_pSoundMempool != TNULL )
		TMemoryHeap_Free( TMemory_GetGlobalHeap(), m_pSoundMempool );

	T2GUIRenderer* pGUIRenderer = T2GUI::GetSingleton()->GetRenderer();

	if ( pGUIRenderer != TNULL )
		delete pGUIRenderer;

	if ( ARandom::GetSingleton() != TNULL )
		delete ARandom::GetSingleton();

	if ( AInputManager2::GetSingleton() != TNULL )
		delete AInputManager2::GetSingleton();

	if ( AVibrationManager::GetSingleton() != TNULL )
		delete AVibrationManager::GetSingleton();

	TSystemManager::GetSingleton()->GetScheduler()->DestroyAllTasks();

	T2GUIFontManager::UnloadFontTRB( "data/gui/fonts.trb" );
	T2GUIFontManager::Close();
	T2GUI::Close();

	TASSERT( !AAssetStreaming::GetSingleton()->HasActiveJobs() );

	if ( m_pRenderer != TNULL )
	{
		m_pRenderer->Destroy();

		delete m_pRenderer;
		m_pRenderer = TNULL;
	}

	if ( ALevelInformation::GetSingleton() != TNULL )
		delete ALevelInformation::GetSingleton();

	AConsole::UnregisterCommands();

	ALocaleManager::Destroy();

	if ( AAssetStreaming::GetSingleton() != TNULL )
		delete AAssetStreaming::GetSingleton();

	return TTRUE;
}

// $deBlob: FUNCTION 0056b530
void AApplication::SetRenderWorld( TBOOL a_bRenderWorld )
{
	m_bRenderWorld = a_bRenderWorld;
}

// $deBlob: FUNCTION 0056b540
void AApplication::SetGamePaused( TBOOL a_bPaused )
{
	m_bGamePaused = a_bPaused;

	if ( AMaterialManager::GetSingleton() != TNULL )
		AMaterialManager::GetSingleton()->SetPaused( a_bPaused );

	AMusicManager::GetSingleton()->PauseMusic( a_bPaused, 0.0f );
}

// $deBlob: FUNCTION 0056b580
const TCHAR* AApplication::GetLevelPropertiesFile() const
{
	return m_szLevelPropertiesFileName;
}

// $deBlob: FUNCTION 0056b590
void AApplication::CreateSaveData()
{
	// NOTE: a default profile (SlotData's constructor resets it) that nothing else ever writes: AAutosaveManager::Save
	// copies it over the current slot
	m_pSaveData2 = new EnSaveData::SlotData();
	m_pSaveData  = new EnSaveData();
}

// $deBlob: FUNCTION 0056b630
void AApplication::SetCurrentSaveSlot( TINT a_iCurrentSaveSlot )
{
	TASSERT( 0 <= a_iCurrentSaveSlot && a_iCurrentSaveSlot < 4 /*EnSaveData::SAVEGAME_SLOT_COUNT*/ );
	m_iCurrentSaveSlot = a_iCurrentSaveSlot;
}

// $deBlob: FUNCTION 0056b690
void AApplication::InitialiseFreeLists()
{
}

// NOTE: the loop goes over every argument from argv[0] (the executable); the value of -width... is the next one
// $deBlob: FUNCTION 0056b6a0
void AApplication::ParseCommandLineParams( TINT argc, TCHAR** argv )
{
	TBOOL bExclusiveFullscreenSwitch = TFALSE;

	for ( TINT i = 1; i <= argc; i++ )
	{
		const TCHAR* szArg = argv[ i - 1 ];

		if ( TStringManager::String8CompareNoCase( szArg, "-width" ) == 0 && i < argc )
			m_iWidth = TStringManager::String8ToInt( argv[ i ] );
		else if ( TStringManager::String8CompareNoCase( szArg, "-height" ) == 0 && i < argc )
			m_iHeight = TStringManager::String8ToInt( argv[ i ] );
		else if ( TStringManager::String8CompareNoCase( szArg, "-w" ) == 0 && i < argc )
			m_iWidth = TStringManager::String8ToInt( argv[ i ] );
		else if ( TStringManager::String8CompareNoCase( szArg, "-h" ) == 0 && i < argc )
			m_iHeight = TStringManager::String8ToInt( argv[ i ] );
		else if ( TStringManager::String8CompareNoCase( szArg, "-exclusivefullscreen" ) == 0 )
		{
			m_bWindowed                = TFALSE;
			m_bExclusiveFullscreen     = TTRUE;
			bExclusiveFullscreenSwitch = TTRUE;
		}
		else if ( TStringManager::String8CompareNoCase( szArg, "-windowed" ) == 0 )
			m_bWindowed = TTRUE;
		else if ( TStringManager::String8CompareNoCase( szArg, "-sw" ) == 0 )
			m_bWindowed = TTRUE;
		else if ( TStringManager::String8CompareNoCase( szArg, "-locale" ) == 0 )
		{
			// NOTE: no check of i < argc, a last -locale reads argv[ argc ] (TNULL)
			sm_ForcedLocale = argv[ i ];
		}
	}

	TNativeFileInfo oNativeInfo;

	if ( TFile::ToNative( "setup.ini", oNativeInfo ) )
	{
		TIniFile oIniFile;
		oIniFile.Open( oNativeInfo.InternalPath );

		TINT iValue;

		if ( oIniFile.ReadLong( "video", "width", &iValue, 0 ) == TIniFile::RESULT_OK )
			m_iWidth = iValue;

		if ( oIniFile.ReadLong( "video", "height", &iValue, 0 ) == TIniFile::RESULT_OK )
			m_iHeight = iValue;

		if ( oIniFile.ReadLong( "video", "windowed", &iValue, 0 ) == TIniFile::RESULT_OK )
		{
			// "-exclusivefullscreen" wins
			if ( bExclusiveFullscreenSwitch )
				iValue = 2;

			if ( iValue == 0 )
			{
				m_bWindowed            = TFALSE;
				m_bExclusiveFullscreen = TFALSE;
			}
			else if ( iValue == 1 )
			{
				m_bWindowed            = TTRUE;
				m_bExclusiveFullscreen = TFALSE;
			}
			else if ( iValue == 2 )
			{
				m_bWindowed            = TFALSE;
				m_bExclusiveFullscreen = TTRUE;
			}
		}

		if ( oIniFile.ReadLong( "video", "antialias", &iValue, 0 ) == TIniFile::RESULT_OK )
			ARenderer::s_iAntialiasLevel = iValue;

		if ( oIniFile.ReadLong( "video", "quality", &iValue, 0 ) == TIniFile::RESULT_OK )
			TRenderDX11::s_iQualityLevel = iValue;
	}

	if ( m_bWindowed )
		m_bExclusiveFullscreen = TFALSE;
}

// $deBlob: FUNCTION 0056b990
TBOOL __stdcall AApplication::OnPauseEvent( AApplication* a_pApp, TSystemManager* a_pSystemManager, TBOOL* a_pPause )
{
	a_pApp->m_bInitialised = !*a_pPause;
	return TTRUE;
}

// $deBlob: FUNCTION 0056b9b0
void AApplication::UpdateSound( TFLOAT a_fDeltaTime )
{
	if ( AMusicManager::GetSingleton() != TNULL )
		AMusicManager::GetSingleton()->Update( a_fDeltaTime );

	TSound* pSound = TSound::GetSingleton();

	if ( pSound != TNULL && pSound->IsInitialised() )
	{
		FMOD::EventSystem* pEventSystem = pSound->GetEventSystem();
		TINT               iListener    = 0;

		if ( ACameraManager::GetSingleton() != TNULL )
		{
			AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();

			for ( TINT i = AXYZViewportManager::VIEWPORT_Count; i != 0; )
			{
				i--;
				ACamera* pCamera = pViewportManager->GetCamera( i );

				if ( pCamera != TNULL && pViewportManager->IsViewportUsed( i ) )
				{
					const TMatrix44& rMatrix = pCamera->GetMatrix();

					TVector4 vPosition = rMatrix.GetTranslation();
					TVector4 vForward( rMatrix.m_f31, rMatrix.m_f32, rMatrix.m_f33, 1.0f );
					TVector4 vUp       = pCamera->GetUp();
					TVector4 vVelocity = TVector4::VEC_ZERO;

					FMOD_VECTOR vFMODPosition = ASoundInterface::GetFMODVector( vPosition );
					FMOD_VECTOR vFMODForward  = ASoundInterface::GetFMODVector( vForward );
					FMOD_VECTOR vFMODUp       = ASoundInterface::GetFMODVector( vUp );
					FMOD_VECTOR vFMODVelocity = ASoundInterface::GetFMODVector( vVelocity );

					TSound::ErrorCheck( pEventSystem->set3DListenerAttributes( iListener++, &vFMODPosition, &vFMODVelocity, &vFMODForward, &vFMODUp ) );
				}

				if ( ACameraManager::GetSingleton() == TNULL )
					break;
			}
		}

		// NOTE: FMOD refuses 0 listeners; retail sets at least one
		TSound::ErrorCheck( pEventSystem->set3DNumListeners( iListener < 1 ? 1 : iListener ) );

		pSound->Update();
	}
}

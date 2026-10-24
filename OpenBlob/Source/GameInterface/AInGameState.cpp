#include "pch.h"
#include <T2Locale/T2Locale.h>
#include "Render/Decal/ADecalManager.h"
#include "Console/AStaticCVars.h"
#include "Platform/DX11/AStencilShadow_DX11.h"
#include "Sound/APositionalSoundManager.h"
#include "Entities/APickupManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/AGndMotorManager.h"
#include "AInGameState.h"
#include "Entities/AStreetSpawner/AStreetSpawner.h"
#include "GameController/AGameControllerHelper.h"
#include "Sound/AMusicManager.h"
#include "Input/ABlobInputMap.h"
#include "Cameras/ACameraShakeFunction.h"
#include "Entities/Enemies/AEnemyManager.h"
#include "AGameStateController.h"
#include "APlayerManager.h"
#include "AApplication.h"
#include "Cameras/ACameraManager.h"
#include "Entities/AEntityFactory.h"
#include "Entities/APooledEntityManager.h"
#include "Entities/ASpawnedCivManager.h"
#include "GameController/AGameStatus.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Buildings/ABuildingManager.h"
#include "Platform/DX11/AJellyShader_DX11.h"
#include "GUI/AGUIManager.h"
#include "Input/AInputManager2.h"
#include "Input/AVibrationManager.h"
#include "Sound/APhysicsSoundManager.h"
#include "XUI/ABlobControllerPopup.h"
#include "XUI/InGame/ABlobInGame.h"
#include "XUI/InGame/ABlobHelpFullScreen.h"
#include "Cheats/ACheatManager.h"
#include "Sound/ASoundInterface.h"
#include <TParticleSystem/TParticleEffectManager.h>
#include "Fx/Shadows/AShadow.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "XUI/InGame/HUD/ABlobLetterBoxHud.h"
#include "Entities/APropTriggerHelp.h"
#include "AAssetStreaming.h"
#include <Toshi/TStringManager.h>
#include "XUI/InGame/HUD/ABlobTimerHud.h"
#include "ALevelInformation.h"
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Platform/DX11/TRender_DX11.h>
#include "Entities/Projectiles/AProjectileManager.h"
#include "Entities/APlatformManager.h"

#include <Physics/Havok/TPhysicsManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "FXComposition/AFXCompositionManager.h"
#include "JobSystem/AJobQueue.h"
#include "Fx/AEnvironmentFX.h"
#include "Render/PaintParticleSystem/APaintParticleSystem.h"

#include "Entities/AEntityFramework.h"
#include "Entities/ANavMarkers.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Entities/APropWaterHydrant.h"
#include "Shaders/AShaderEffects.h"
#include "Challenges/AChallengeManager.h"
#include "Challenges/AChallenge.h"
#include "Render/Lighting/ALightingData.h"
#include "ACurrentLevelInformation.h"
#include "ARegionManager.h"
#include <Render/TRender.h>
#include <XUI/TXUI.h>
#include <TParticleSystem/TParticleSystem.h>
#include "ParticleSystem/AEnvParticles.h"
#include "Sound/AStreamingSoundPlayer.h"
#include "Console/AConsoleVar.h"
#include "Render/ARenderer.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Entities/AZJumpData.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The streamed job of OnUpdate (StartStreamedJob): its user data once finished (OnStreamedJobFinished), whether it
// finished without user data while APropTriggerHelp's 0x9b720d was clear, and whether it finished
static TUINT32 s_uiStreamedJobResult = 0;
static TBOOL   s_bStreamedJobFailed  = TFALSE;
static TBOOL   s_bStreamedJobDone    = TFALSE;

TSTATICASSERT( sizeof( AInGameState ) == 400 );

// $deBlob: FUNCTION 005c69a0
// $deBlob: FUNCTION 005c69b0
// $deBlob: FUNCTION 005c6a20
// $deBlob: FUNCTION 005c6a80
// $deBlob: FUNCTION 005c6a90
// $deBlob: FUNCTION 0053c630
// $deBlob: FUNCTION 008584d0
TDEFINE_CLASS( AInGameState );

// $deBlob: FUNCTION 005c6aa0
AInGameState::AInGameState()
    : m_bLevelEndAcknowledged( TFALSE )
    , m_bLevelEnded( TFALSE )
    , m_bUnk2E( TFALSE )
    , m_iReadyState( 0 )
    , m_oHelper( this )
{
	m_fEndTimer             = -1.0f;
	m_bPaused               = TFALSE;
	m_bUnk6D                = TFALSE;
	m_pMultiplayerHud       = TNULL;
	m_uiNextLevel           = 0;
	m_uiNextLevelName       = 0;
	m_iNextLevelIndex       = -1;
	m_bLoadNextLevel        = TFALSE;
	m_bWaitForFade          = TFALSE;
	m_pTRB                  = TNULL;
	m_pTRBUserData          = TNULL;
	m_pControllerPopup      = TNULL;
	m_bWaitingToStart       = TFALSE;
}

// $deBlob: FUNCTION 005c8710
// $deBlob: FUNCTION 005c6b60
AInGameState::~AInGameState()
{
}

// $deBlob: FUNCTION 005c8540
void AInGameState::HandleMenuRequestToCancel()
{
	if ( g_oTheApp.ShouldRenderWorld() )
		APlayerManager::GetSingleton()->Render( 0.0f );
}

// Whether the game is paused by the console variable pause_game
// $deBlob: FUNCTION 005c6be0
TBOOL AInGameState::WasControllerRemoved()
{
	return AStaticCVars::cvPauseGame.GetBool();
}

// $deBlob: FUNCTION 005c7110
void AInGameState::OnFadeFinished( TINT a_iViewport, TBOOL a_bFromBlack )
{
	if ( m_pMultiplayerHud != TNULL && m_bUnk6D )
	{
		m_bUnk6D = TFALSE;
		m_pMultiplayerHud->StartReadySetGo();
	}
}

// $deBlob: FUNCTION 005c7130
void AInGameState::SetNextLevel( const TCHAR* a_szLevel, TINT a_iTitleId, TINT a_iLevel )
{
	m_uiNextLevel = TUINT32( a_szLevel );

	if ( a_iTitleId == -1 )
	{
		m_uiNextLevelName = TUINT32( L"" );
		m_iNextLevelIndex = a_iLevel;
	}
	else
	{
		m_uiNextLevelName = TUINT32( T2Locale::GetSingleton()->GetString( a_iTitleId ) );
		m_iNextLevelIndex = a_iLevel;
	}
}

// $deBlob: FUNCTION 005c6c20
void AInGameState::RestartLevel()
{
	AGameStateController* pController = AGameStateController::GetSingleton();
	AGameStatus*          pGameStatus = AGameStatus::GetSingleton();

	if ( pGameStatus->GetSPLevel() == 9 )
	{
		// The last level fades through black instead of the level's colour
		TFLOAT fFade = pController->GetFade( 0 );
		pController->SetFadeColour( 0xFF000000, 0 );
		pController->SetFade( fFade, 0 );
	}

	pGameStatus->ClearNextPaintableProjections();
	TPhysicsManager::GetSingleton()->SetBatchCommitDisabled( TFALSE, 10 );
	TRenderDX11::Interface()->WaitForEndOfRender();
	m_fEndTimer = -1.0f;

	ALevelInformation::LevelProperties* pLevelProps = &ALevelInformation::GetSingleton()->GetLevelHeader()->m_pLevelProps[ pGameStatus->GetCurrentLevel() ];
	pGameStatus->InitLevelData( pLevelProps );

	AGameStatus::LevelData& rLevelData = pGameStatus->GetLevelData();

	ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID_INGAME, TFALSE );
	APositionalSoundManager::GetSingleton()->ResetEmitters();
	APositionalSoundManager::GetSingleton()->SetPaused( TFALSE );
	AJellyShader_DX11::GetSingleton()->Reset();
	AShaderEffects::GetSingleton()->ReleaseAll();
	ABuildingManager::GetSingleton()->Reset();
	ADecalManager::GetSingleton()->Reset();
	APaintParticleSystem::s_Instance->Reset();
	ATerrain2Interface::GetInstance()->ResetAllCells();
	pGameStatus->GetPointData().ResetUnk20();
	pGameStatus->GetPointData().SetUnk20( 0, TTRUE );

	AStreetSpawner::ms_iNumSpawned = 0;

	if ( AEnemyManager::GetSingleton() != TNULL )
	{
		for ( TINT i = 0; i < 4; i++ )
			AEnemyManager::GetSingleton()->ReleaseActiveEnemies( AEnemyManager::EnemyType( i ) );
	}

	ASpawnedCivManager::GetSingleton()->ReleaseAll();
	APooledEntityManager::GetSingleton()->ReleaseAll();
	AProjectileManager::GetSingleton()->KillAllActive();
	AEntityFactory::GetSingleton()->ReactivateAllEntities();
	ACameraManager::GetSingleton()->ResetLevelCameras();
	APlayerManager::GetSingleton()->RestartPlayers();
	ACameraManager::GetSingleton()->SuspendCameras();
	ACameraManager::GetSingleton()->Activate();
	APlayerManager::GetSingleton()->ResetUnk2E8();
	AEnvironmentFX::GetSingleton()->Reset();

	AEnvParticles::GetSingleton()->Reset();

	TParticleSystem::GetSingleton()->StopAllParticles();
	AChallengeManager::GetSingleton()->Reset();
	pGameStatus->ResetLevel();

	ABlobInputMap* pInputMap = ABlobInputMap::GetSingleton();
	pInputMap->ClearPlayers();

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();

	if ( pPlayerManager->GetNumPlayers() == 1 )
		pInputMap->Enable();
	else
		pInputMap->DisableFor( -1.0f );

	pController->FadeToBlack( 0.0f, 0 );
	pController->SetFade( 1.0f, 0 );

	if ( ( pPlayerManager->GetNumPlayers() == 1 && !pPlayerManager->GetBlobData( 0 )->IsKeyFrameCameraActive() ) || pPlayerManager->GetNumPlayers() > 1 )
	{
		m_bWaitingToStart = TTRUE;
		m_fWaitTime       = 0.2f;
	}

	AMusicManager* pMusicManager = AMusicManager::GetSingleton();
	pMusicManager->StartLevelMusic();
	pMusicManager->PauseMusic( TTRUE, 0.0f );

	if ( pGameStatus->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
		pMusicManager->SetPauseLocked( TTRUE );

	AStreamingSoundPlayer::GetSingleton()->StopAll();
	AStreamingSoundPlayer::GetSingleton()->SetPaused( TFALSE, TFALSE );

	m_bPaused = TFALSE;

	// The goal of the goal HUD: the sprint level's challenge count, else the level's goal of the level mode
	// NOTE: LevelProperties 0x38 (the percentage of the challenges) and 0x5C/0x60 (the goals) are unnamed
	TINT  iLevelMode = pGameStatus->GetLevelMode();
	TBOOL bSetGoal   = TTRUE;
	TINT  iGoal      = -1;

	if ( pLevelProps->m_sprintLevel != 0 )
	{
		TASSERT( APlayerManager::GetSingleton()->GetNumPlayers() == 1 );

		AChallengeManager* pChallengeManager = AChallengeManager::GetSingleton();
		pChallengeManager->StartNearestChallenge();

		if ( pChallengeManager->GetCurrentChallenge()->GetData()->m_uiNumLinkedChallenges == 0 )
			iGoal = TMath::CeilToInt( TFLOAT( pChallengeManager->GetCounterTotal() ) * ( TFLOAT( TINT( rLevelData.m_pLevelProps->m_auiMedalLimits[ 1 ] ) ) / 100.0f ) );
		else
			iGoal = -1;
	}
	else
	{
		TINT* piGoal = TNULL;

		if ( iLevelMode == -1 || iLevelMode == 0 || iLevelMode == 3 )
			piGoal = TREINTERPRETCAST( TINT*, rLevelData.m_pLevelProps->m_auiGoals[ 0 ] );

		if ( piGoal == TNULL && iLevelMode == 2 )
			piGoal = TREINTERPRETCAST( TINT*, rLevelData.m_pLevelProps->m_auiGoals[ 1 ] );

		if ( piGoal != TNULL )
			iGoal = *piGoal;
		else
			bSetGoal = TFALSE;
	}

	if ( bSetGoal )
		rLevelData.SetGoal( iGoal, 0 );

	if ( ABlobHud::GetSingleton() != TNULL )
		ABlobHud::GetSingleton()->Reset();

	s_uiStreamedJobResult = 0;
	s_bStreamedJobFailed  = TFALSE;
	s_bStreamedJobDone    = TFALSE;

	pController->SetRenderFadesWhenPaused( TFALSE );

	TXUIScene* pFocussedScene = TXUI::GetSingleton()->GetActiveScene();
	TASSERT( pFocussedScene->IsA( &TGetClass( ABlobInGame ) ) );
	TSTATICCAST( ABlobInGame, pFocussedScene )->CloseScenesToHud();
}

// $deBlob: FUNCTION 005c71c0
void AInGameState::RequestStreamedTRB( TTRB* a_pTRB, const TCHAR* a_szFileName, void* a_pUserData )
{
	TASSERT( m_pTRB == TNULL );
	m_pTRB = a_pTRB;
	TStringManager::String8Copy( m_szTRBFileName, a_szFileName, sizeof( m_szTRBFileName ) );
	m_pTRBUserData = a_pUserData;
}

// $deBlob: FUNCTION 005c7230
void AInGameState::StartStreamedJob( TTRB* a_pTRB, const TCHAR* a_szFileName, void* a_pUserData )
{
	AAssetStreaming* pAssetStreaming = AAssetStreaming::GetSingleton();
	ATRBLoaderJob*   pJob            = pAssetStreaming->GetFreeJob();
	pJob->Init( a_pTRB, a_szFileName );
	pJob->m_fnOnFinished        = OnStreamedJobFinished;
	pJob->m_pOnFinishedUserData = a_pUserData;
	pAssetStreaming->AddMainThreadJob2( pJob );
}

// $deBlob: FUNCTION 005c7280
void AInGameState::OnStreamedJobFinished( ATRBLoaderJob* a_pJob, void* a_pUserData )
{
	s_uiStreamedJobResult = TUINT32( TREINTERPRETCAST( TUINTPTR, a_pUserData ) );

	if ( a_pUserData == TNULL && !APropTriggerHelp::IsUnk9B720D() )
		s_bStreamedJobFailed = TTRUE;

	AAssetStreaming::GetSingleton()->ReleaseJob( a_pJob );
	s_bStreamedJobDone = TTRUE;
}

// $deBlob: FUNCTION 005c72d0
AGameState::UpdateResult AInGameState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bWaitForFade )
	{
		if ( !AGameStateController::GetSingleton()->IsFading( 0 ) )
		{
			m_bWaitForFade = TFALSE;
			m_bLevelEnded  = TTRUE;
			m_bUnk2E       = TTRUE;
		}

		return UpdateResult_OK;
	}

	// The boss grass bomb sequence has ended: the level is complete and the boss end sequence follows
	if ( AGameStatus::GetSingleton()->GetUnk00C() )
	{
		AGameStatus::GetSingleton()->OnLevelComplete();
		AGameControllerHelper::StartBossEndSequence();
		return UpdateResult_OK;
	}

	TXUI*      pXUI         = TXUI::GetSingleton();
	TXUIScene* pActiveScene = pXUI->GetActiveScene();

	if ( m_pTRB != TNULL )
	{
		// A TRB to stream (the full screen help's): once the in-game scene, the HUD or the letter box HUD is shown and
		// no transition runs, the game pauses for the help and the TRB is streamed (behind the load transition on the
		// HUDs)
		if ( pActiveScene != TNULL && pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() )
		{
			if ( pActiveScene->IsA( &TGetClass( ABlobInGame ) ) || pActiveScene->IsA( &TGetClass( ABlobHud ) ) || pActiveScene->IsA( &TGetClass( ABlobLetterBoxHud ) ) )
			{
				ABlobHelpFullScreen::Pause();

				if ( pActiveScene->IsA( &TGetClass( ABlobHud ) ) || pActiveScene->IsA( &TGetClass( ABlobLetterBoxHud ) ) )
				{
					TXUIScene* pTransition = pXUI->CreateScene( pXUI->FindResource( "loadtransition.xur" ), TUINT32( 0 ) );
					pXUI->TransitionTo( TNULL, pTransition );
					ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x4B ), FMOD_EVENT_DEFAULT, TTRUE );

					TXUIElement* pElement = pTransition->FindChildXUIElement( 1 );
					pElement->SetVisible( pActiveScene->IsA( &TGetClass( ABlobLetterBoxHud ) ) );
				}

				StartStreamedJob( m_pTRB, m_szTRBFileName, m_pTRBUserData );
				m_pTRB         = TNULL;
				m_pTRBUserData = TNULL;
			}
		}
	}
	else if ( s_bStreamedJobDone && ( s_uiStreamedJobResult != 0 || s_bStreamedJobFailed || APropTriggerHelp::IsUnk9B720D() ) )
	{
		// The TRB is streamed: the full screen help opens once the load transition is shown (at once without the user
		// data)
		TBOOL bPopupHidden = TTRUE;

		if ( m_pControllerPopup != TNULL )
			bPopupHidden = TSTATICCAST( ABlobControllerPopup, m_pControllerPopup )->GetState() == ABlobControllerPopup::STATE_Hidden;

		if ( pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() && bPopupHidden )
		{
			if ( s_bStreamedJobFailed ||
			     ( pActiveScene != TNULL && TStringManager::String16Compare( pActiveScene->GetID(), L"Transition", -1 ) == 0 ) )
			{
				APropTriggerHelp::OpenFullscreenHelp( TREINTERPRETCAST( APropTriggerHelp*, TUINTPTR( s_uiStreamedJobResult ) ) );
				s_uiStreamedJobResult = 0;
				s_bStreamedJobFailed  = TFALSE;
				s_bStreamedJobDone    = TFALSE;
			}
		}
	}

	AStreamingSoundPlayer::GetSingleton()->Tick( a_fDeltaTime );

	if ( m_bWaitingToStart )
	{
		m_fWaitTime -= a_fDeltaTime;

		if ( m_fWaitTime <= 0.0f && !ATerrain2Interface::GetInstance()->IsStreaming() )
		{
			TPhysicsManager::GetSingleton()->SetBatchCommitDisabled( TTRUE, 10 );
			m_bWaitingToStart = TFALSE;

			if ( AGameStateController::GetSingleton()->GetFade( 0 ) >= 0.98f )
			{
				m_bUnk6D = TTRUE;
				OnResume();
				AGameStateController::GetSingleton()->FadeFromBlack( 1.0f, 0 );
			}
		}
	}

	TINT iPlayerState = APlayerManager::GetSingleton()->GetVistaState();

	if ( iPlayerState != 8 && iPlayerState != 0 )
		TPhysicsManager::GetSingleton()->SetBatchCommitDisabled( TFALSE, 10 );

	if ( m_bLevelEnded == TTRUE )
	{
		if ( m_bLevelEndAcknowledged )
		{
			AGameStateController::GetSingleton()->SetRenderFadesWhenPaused( TFALSE );
			AGameControllerHelper::ReturnToFrontEnd( m_bUnk2E );
			return UpdateResult_OK;
		}

		m_bLevelEndAcknowledged = TTRUE;
		return UpdateResult_OK;
	}

	if ( m_bPaused == TTRUE )
	{
		TXUI::GetSingleton()->TickInput( a_fDeltaTime );

		// The pause menu chose another level (SetNextLevel)
		if ( m_uiNextLevel != 0 && m_bLoadNextLevel )
		{
			m_bLoadNextLevel = TFALSE;
			AGameControllerHelper::LoadLevel( TREINTERPRETCAST( const TCHAR*, m_uiNextLevel ), TREINTERPRETCAST( const TWCHAR*, m_uiNextLevelName ) );
			return UpdateResult_OK;
		}

		if ( m_pControllerPopup != TNULL )
			TSTATICCAST( ABlobControllerPopup, m_pControllerPopup )->UpdateInGame( a_fDeltaTime );
	}
	else if ( !m_bWaitingToStart )
	{
		if ( m_pControllerPopup != TNULL && !TSTATICCAST( ABlobControllerPopup, m_pControllerPopup )->UpdateInGame( a_fDeltaTime ) )
			return UpdateResult_OK;

		ACheatManager::GetSingleton()->Update( a_fDeltaTime );

		if ( !AStaticCVars::cvPauseGame.GetBool() && ATerrain2Interface::GetInstance()->IsCurrentCellReady() )
		{
			ABuildingManager::GetSingleton()->Update( a_fDeltaTime );
			AChallengeManager::GetSingleton()->Update( a_fDeltaTime );

			AGameStatus* pGameStatus = AGameStatus::GetSingleton();

			pGameStatus->Update( a_fDeltaTime );

			if ( pGameStatus->IsLevelComplete() )
			{
				if ( m_fEndTimer == -1.0f )
				{
					// The level has just ended: the results follow after a delay
					m_fEndTimer = pGameStatus->IsMultiplayer() ? 3.5f : 2.0f;
				}
				else if ( m_fEndTimer <= 0.0f || pGameStatus->IsLevelWon() != TTRUE )
				{
					m_fEndTimer = -1.0f;
					APositionalSoundManager::GetSingleton()->SetPaused( TTRUE );

					if ( pGameStatus->GetLevelMode() != AGameStatus::Mode_FreePaint )
					{
						// The results screen in the InGame scene: the score, the multiplayer score, the sprint summary or
						// the time up menu
						TXUI*        pXUI    = TXUI::GetSingleton();
						ABlobInGame* pInGame = TSTATICCAST( ABlobInGame, pXUI->CreateScene( pXUI->FindResource( "ingame.xur" ), L"InGame", TFALSE ) );
						const TCHAR* szMenu;

						if ( pGameStatus->IsLevelWon() == TTRUE )
						{
							if ( pGameStatus->IsMultiplayer() )
							{
								szMenu = "multiplayerscoremenu.xur";
							}
							else if ( pGameStatus->GetLevelProperties()->m_sprintLevel != 0 )
							{
								szMenu = "sprintsummarymenu.xur";
							}
							else
							{
								AGameStatus::CalculateScoreSummary( &AGameStatus::s_oScoreSummary );
								szMenu = "scoremenu.xur";
							}
						}
						else
						{
							szMenu = "timeupmenu.xur";
						}

						TXUIScene* pMenu = pXUI->CreateScene( pXUI->FindResource( szMenu ), 0 );
						pMenu->SetFocus( TTRUE, TFALSE, TTRUE );
						pInGame->SetUsage( ABlobInGame::Usage_Results );
						pInGame->SetFocus( pMenu, 0 );
						pInGame->AddChildTail( pMenu );
						pXUI->TransitionTo( TNULL, pInGame );
						return UpdateResult_OK;
					}

					// Free paint: fades out and leaves the level
					AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
					m_bWaitForFade = TTRUE;
					return UpdateResult_OK;
				}
				else
				{
					TXUI::GetSingleton()->TickInput( a_fDeltaTime );

					TFLOAT fOldTimer = m_fEndTimer;
					m_fEndTimer      = fOldTimer - a_fDeltaTime;

					if ( 1.0f < fOldTimer && m_fEndTimer < 1.0f )
					{
						AGameStateController::GetSingleton()->SetRenderFadesFirst( TFALSE );
						AGameStateController::GetSingleton()->FadeToBlack( 0.5f, 0 );
					}
				}
			}

			APositionalSoundManager::GetSingleton()->Update();

			APhysicsSoundManager::GetSingleton()->Update( a_fDeltaTime );

			if ( m_iReadyState == 0 )
			{
				m_iReadyState = 1;
			}
			else if ( m_iReadyState == 1 )
			{
				if ( UpdateReady( a_fDeltaTime ) == 1 )
					return UpdateResult_OK;
			}
			else if ( m_iReadyState == 2 )
			{
				return UpdateResult_Remove;
			}

			AEnemyManager::GetSingleton()->Tick( a_fDeltaTime );

			// The paint fades and its particles fly
			APaintParticleSystem::s_Instance->FadePaint( a_fDeltaTime );

			ASpawnedCivManager::GetSingleton()->Update( a_fDeltaTime );

			AEntityFactory::GetSingleton()->Tick( a_fDeltaTime, 0 );
			TPhysicsManager::GetSingleton()->Update( a_fDeltaTime );

			AStencilShadow::GetSingleton()->Update( a_fDeltaTime );

			AProjectileManager::GetSingleton()->Tick( a_fDeltaTime );

			APickupManager::GetSingleton()->UpdateRotations( a_fDeltaTime );

			APlayerManager::GetSingleton()->Update( a_fDeltaTime );

			AGndMotorManager::GetSingleton()->Update( a_fDeltaTime );

			AEnvironmentFX::GetSingleton()->Update( a_fDeltaTime );

			AEnvParticles::GetSingleton()->Update( a_fDeltaTime );

			AJobQueue::GetInstance()->Update( a_fDeltaTime );

			AShaderEffects::GetSingleton()->Update( a_fDeltaTime );
		}
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $deBlob: FUNCTION 005c7ad0
void AInGameState::OnInsertion()
{
	TPhysicsManager::GetSingleton()->SetBatchCommitDisabled( TTRUE, 10 );

	// The controller popup is cached (taken off the canvas until it's shown)
	m_pControllerPopup = TXUI::GetSingleton()->FindCachedScene( L"ControllerPopup" );
	TSTATICCAST( TXUIScene, m_pControllerPopup )->RemoveSelf();

	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
		ATerrain2Interface::GetInstance()->SetPositionCallback( GetTerrainStreamingPosition );

	AGameState::OnInsertion();
	m_oHelper.OnInsertion();

	ACheatManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );

	AGameStatus::GetSingleton()->SetCurrentCheckpoint( 0 );
	AGameStatus::GetSingleton()->StartAtCheckpoint();

	AGameStatus::GetSingleton()->GetPointData().ResetUnk20();
	AGameStatus::GetSingleton()->GetPointData().SetUnk20( 0, TTRUE );

	new ABlobInputMap();

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	pPlayerManager->CreatePlayers();
	pPlayerManager->AddSprintPatterns();
	pPlayerManager->SetupViewports();

	AGameStateController::GetSingleton()->FadeToBlack( 0.0f, 0 );
	AGameStateController::GetSingleton()->SetFade( 1.0f, 0 );

	if ( ( pPlayerManager->GetNumPlayers() == 1 && !pPlayerManager->GetBlobData( 0 )->IsKeyFrameCameraActive() ) || pPlayerManager->GetNumPlayers() > 1 )
	{
		m_bWaitingToStart = TTRUE;
		m_fWaitTime       = 0.2f;
	}

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
	{
		AMusicManager::GetSingleton()->StartMusic( TTRUE );
	}
	else
	{
		AMusicManager::GetSingleton()->SetPauseLocked( TFALSE );
		AMusicManager::GetSingleton()->StartRainbow();
	}

	AMusicManager::GetSingleton()->SetPercentage( 0.0f );

	// A sprint level starts at its nearest challenge
	if ( ALevelInformation::GetSingleton()->GetLevelHeader()->m_pLevelProps[ AGameStatus::GetSingleton()->GetCurrentLevel() ].m_sprintLevel != 0 )
	{
		TASSERT( APlayerManager::GetSingleton()->GetNumPlayers() == 1 );
		AChallengeManager::GetSingleton()->StartNearestChallenge();
		AChallengeManager::GetSingleton()->SetMarkersVisible( TTRUE );
	}

	// The in-game scene and the pause menu are created and cached (taken off the canvas until they're shown)
	TXUI* pXUI = TXUI::GetSingleton();
	pXUI->CreateScene( pXUI->FindResource( "ingame.xur" ), L"InGame", TTRUE )->RemoveSelf();
	pXUI->CreateScene( pXUI->FindResource( AGameStatus::GetSingleton()->IsMultiplayer() ? "mppausemenu.xur" : "pausemenu.xur" ), L"PauseMenu", TTRUE )->RemoveSelf();

	// The level's goal: the paint goal of the level mode, or on a sprint level a percentage of the challenges' counter
	// (none when the current challenge has linked challenges)
	{
		AGameStatus*                        pStatus = AGameStatus::GetSingleton();
		ALevelInformation::LevelProperties* pProps  = pStatus->GetLevelProperties();
		TINT                                iGoal;
		TBOOL                               bSetGoal = TTRUE;

		if ( pProps->m_sprintLevel == 0 )
		{
			TINT  eLevelMode = pStatus->GetLevelMode();
			TINT* piGoalA    = TREINTERPRETCAST( TINT*, pProps->m_auiGoals[ 0 ] );
			TINT* piGoalB    = TREINTERPRETCAST( TINT*, pProps->m_auiGoals[ 1 ] );

			if ( ( eLevelMode == -1 || eLevelMode == 0 || eLevelMode == 3 ) && piGoalA != TNULL )
				iGoal = *piGoalA;
			else if ( eLevelMode == 2 && piGoalB != TNULL )
				iGoal = *piGoalB;
			else
				bSetGoal = TFALSE;
		}
		else
		{
			AChallengeManager* pChallengeManager = AChallengeManager::GetSingleton();

			if ( pChallengeManager->GetCurrentChallenge()->GetData()->m_uiNumLinkedChallenges == 0 )
				iGoal = TINT( TFLOAT( TMath::CeilToInt( TFLOAT( pChallengeManager->GetCounterTotal() ) * ( TFLOAT( TINT( pProps->m_auiMedalLimits[ 1 ] ) ) / 100.0f ) ) ) );
			else
				iGoal = -1;
		}

		if ( bSetGoal )
			pStatus->GetLevelData().SetGoal( iGoal, 0 );
	}

	AChallengeManager::CreateEffect();

	ACameraManager::GetSingleton()->SuspendCameras();

	APlayerManager::GetSingleton()->ResetUnk2E8();
	AGameStatus::GetSingleton()->ResetLevel();

	APositionalSoundManager::GetSingleton()->OnLevelStarted();

	APositionalSoundManager::GetSingleton()->SetPaused( TFALSE );

	AStreetSpawner::ms_iNumSpawned = 0;
}

// $deBlob: FUNCTION 005c7ea0
void AInGameState::OnRemoval()
{
	// The level mode (AGameStatus+0xA28), read first; decides whether the challenges are destroyed
	TINT iLevelMode = AGameStatus::GetSingleton()->GetLevelMode();

	TPhysicsManager::GetSingleton()->SetBatchCommitDisabled( TFALSE, 10 );

	TRenderDX11::Interface()->WaitForEndOfRender();

	AStreamingSoundPlayer::GetSingleton()->ResetAll();

	m_pControllerPopup = TNULL;
	AMusicManager::GetSingleton()->SetPauseLocked( TFALSE );
	AMusicManager::GetSingleton()->ClearMusicOverride();

	AStreamingSoundPlayer::GetSingleton()->StopAll();
	AStreamingSoundPlayer::GetSingleton()->SetPaused( TFALSE, TFALSE );

	TXUI::GetSingleton()->Reset();

	APositionalSoundManager::GetSingleton()->Reset();

	APhysicsSoundManager::GetSingleton()->StopAll();

	ABuildingManager::GetSingleton()->Reset();

	APaintParticleSystem::s_Instance->Reset();

	if ( TSound::GetSingleton()->IsInitialised() )
		ASoundInterface::GetEventCategory( ASoundMap::CATEGORYID( 0 ) )->stopAllEvents();

	// The civilians spawned by the level are given back to the pools
	ASpawnedCivManager::GetSingleton()->ReleaseAll();

	// The blobs and cameras of the players go, and their BlobData is reset (patterns collected in the level...)
	APlayerManager::GetSingleton()->DestroyPlayers();

	ACameraShakeFunction::UnloadShakeTypes();

	ABlobEntity::UnloadProperties();

	if ( AZJumpData::GetSingleton() != TNULL )
	{
		AZJumpData::GetSingleton()->DestroyPointEffects();
		delete AZJumpData::GetSingleton();
	}

	if ( ANavMarkers::GetSingleton() )
		delete ANavMarkers::GetSingleton();

	ACameraManager::GetSingleton()->Reset();

	if ( ABlobInputMap::GetSingleton() )
		delete ABlobInputMap::GetSingleton();

	AJellyShader_DX11::GetSingleton()->Reset();

	TBOOL bRes = AJellyShader_DX11::GetSingleton()->UnloadSolids();
	TASSERT( bRes );

	TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() )->DeinitWaterTexture();

	AGameStatus::GetSingleton()->ClearCheckPoints();

	if ( AGndMotorManager::GetSingleton() )
		delete AGndMotorManager::GetSingleton();

	AEnemyManager::GetSingleton()->DestroyPools();

	APooledEntityManager::GetSingleton()->DestroyPools();

	APlatformManager::GetSingleton()->ClearGroups();

	if ( AEntityFactory::GetSingleton() )
		AEntityFactory::GetSingleton()->DestroyAllEntities();

	APlayerManager::GetSingleton()->UnloadPatternTextures();

	if ( ATerrain2Interface::GetInstance() )
		delete ATerrain2Interface::GetInstance();

	ABuildingManager::GetSingleton()->Deinit();

	APickupManager::GetSingleton()->CheckEmpty();

	AVibrationManager::GetSingleton()->ResetAll();

	AGameStateController::GetSingleton()->ResetFades();

	if ( TSound::GetSingleton()->IsInitialised() )
		AStreamingSoundPlayer::GetSingleton()->ClearVOMapping();

	AMusicManager::GetSingleton()->UnloadMusicData();

	if ( AShaderEffects::IsSingletonCreated() )
		AShaderEffects::DestroySingleton();

	ACheatManager::GetSingleton()->SetEnabled( TFALSE, TTRUE );

	AMusicManager::GetSingleton()->PauseMusicNow();

	if ( iLevelMode == AGameStatus::Mode_Story || iLevelMode == -1 || iLevelMode == 3 )
		AChallengeManager::GetSingleton()->Destroy();

	if ( AEnvParticles::GetSingleton() != TNULL )
		delete AEnvParticles::GetSingleton();

	if ( AEnvironmentFX::GetSingleton() != TNULL )
		delete AEnvironmentFX::GetSingleton();

	// NOTE: the retail calls it unconditionally; the manager isn't created yet (AEntityFramework's init is not ported)
	if ( AProjectileManager::IsSingletonCreated() )
		AProjectileManager::GetSingleton()->DeletePools();

	if ( AStencilShadow::IsSingletonCreated() )
		delete AStencilShadow::GetSingleton();

	ADecalManager::GetSingleton()->Deinitialise();

	if ( ARegionManager::GetSingleton() )
	{
		ARegionManager::GetSingleton()->UnloadAllRegions();
		delete ARegionManager::GetSingleton();
	}

	AFXCompositionManager::GetSingleton()->UnLoad();

	AEntityFramework::Shutdown();

	AShadow::ResetShadowCount();

	TPhysicsManager::GetSingleton()->Unitialise();
	AGameState::OnRemoval();

	ALeechBot2::ClearEffects();

	ABlobEntity::ClearEffects();

	APlayerManager::ClearEffects();

	APickupManager::ClearEffects();

	AChallengeManager::DestroyEffect();

	APropWaterHydrant::ClearEffects();

	TSTATICCAST( ASkinShaderHAL, ASkinShader::GetSingleton() )->ResetFakeOpaqueFades();

	if ( ALightingData::GetSingleton() )
		delete ALightingData::GetSingleton();

	TParticleSystem::GetSingleton()->StopAllParticles();
	TParticleSystem::GetSingleton()->Destroy();

	TASSERT( TParticleSystem::GetSingleton()->GetEffectManager()->GetNumEffects() == 0 );

	if ( ACurrentLevelInformation::GetSingleton() )
	{
		ACurrentLevelInformation::GetSingleton()->Destroy();
		delete ACurrentLevelInformation::GetSingleton();
	}

	TRender::GetSingleton()->FlushDyingResources();
	TRender::GetSingleton()->FlushDyingResources();
	TRender::GetSingleton()->FlushDyingResources();
	TRender::GetSingleton()->FlushDyingResources();
	TRender::GetSingleton()->FlushDyingResources();
	TRender::GetSingleton()->FlushDyingResources();
	TRender::GetSingleton()->FlushDyingResources();

	ARenderer* pRenderer = ARenderer::GetSingleton();
	pRenderer->SetBackgroundColour( 0, 0, 0 );
	pRenderer->SetFogEnabled( TFALSE );
	g_oTheApp.SetGamePaused( TFALSE );
	m_oHelper.OnRemoval();

	// The level's references are kept after the last level's completion (the end sequence)
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	if ( pGameStatus->GetSPLevel() != 9 || !pGameStatus->IsLevelWon() )
		pGameStatus->ResetLevelReferences();

	AMusicManager::GetSingleton()->PauseMusicNow();
	AGameStateController::GetSingleton()->ResetFades();
}

// $deBlob: FUNCTION 005c8370
void AInGameState::OnActivate()
{
	APlayerManager::GetSingleton()->SetupViewports();
	ACameraManager::GetSingleton()->Activate();
	AGUIManager::GetSingleton()->SetCursorEnabled( TFALSE );
	AInputManager2::GetSingleton()->SetContext( AInputMap::INPUTCONTEXT_InGame );
	ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID( 2 ), TFALSE );
	AGUIManager::GetSingleton()->SetGUIMask( T2GUIElement::STATE_INGAME | T2GUIElement::STATE_UNK3 | T2GUIElement::STATE_SUBTITLES );
	m_oHelper.ResetJustDown();
	g_oTheApp.SetRenderWorld( TTRUE );
	g_oTheApp.SetGamePaused( AStaticCVars::cvPauseGame.GetBool() );

	AGameState::OnActivate();

	ABlobInputMap::GetSingleton()->ClearPlayers();

	// The buttons of the blobs are cleared
	for ( TINT i = 0; i < APlayerManager::MAX_PLAYERS; i++ )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( i ) ) );

		if ( pBlob != TNULL )
			pBlob->ClearButtons();
	}
}

// $deBlob: FUNCTION 005c8460
void AInGameState::OnDeactivate()
{
	ACameraManager::GetSingleton()->Deactivate();
	ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID( 2 ), TTRUE );
	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );
	AInputManager2::GetSingleton()->RemoveCurrentContext();
	AGUIManager::GetSingleton()->RemoveCurrentGUIMask();
	AVibrationManager::GetSingleton()->ResetAll();
	AGameState::OnDeactivate();
}

// $deBlob: FUNCTION 005c70a0
void AInGameState::UpdateMusicPause()
{
	ASoundInterface::SetCategoryPausedAll( TFALSE );
	ASoundInterface::SetCategoryPaused( ASoundMap::CATEGORYID( 2 ), TFALSE );

	ALevelInformation::LevelProperties* pProperties = &ALevelInformation::GetSingleton()->GetLevelHeader()->m_pLevelProps[ AGameStatus::GetSingleton()->GetCurrentLevel() ];

	if ( pProperties->m_sprintLevel != 0 )
	{
		AMusicManager::GetSingleton()->ResetPitch( 0.0f );
		AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.5f );
	}
	else
	{
		AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.0f );
		AMusicManager::GetSingleton()->ResetPitch( 0.5f );
	}
}

// $deBlob: FUNCTION 005c84c0
TINT AInGameState::UpdateReady( TFLOAT a_fDeltaTime )
{
	if ( AInputManager2::GetSingleton()->GetButtonDown( AInputMap::INPUTBUTTON( 10 ) ) )
		AEntityFramework::ReloadResources();

	if ( m_oHelper.OnUpdate( a_fDeltaTime ) != 1 && m_bLevelEnded != TTRUE )
	{
		AEntityFramework::ReadyTick( a_fDeltaTime );

		ABlobInputMap::GetSingleton()->Update( a_fDeltaTime );
		return 0;
	}

	return 1;
}

// $deBlob: FUNCTION 005c8560
void AInGameState::OnResume()
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	if ( pGameStatus->GetLevelData().m_bMultiplayer && ABlobHud::GetSingleton() != TNULL && ABlobHud::GetSingleton()->GetLevelTimer() != TNULL )
	{
		TFLOAT fTimeLeft = pGameStatus->GetLevelData().m_fTimeLeft;
		TFLOAT fSeconds  = TFLOAT( fmod( TDOUBLE( fTimeLeft ), 60.0 ) );
		ABlobHud::GetSingleton()->GetLevelTimer()->SetTime( TINT( fTimeLeft / 60.0f ), TINT( fSeconds ), -1 );
	}

	UpdateMusicPause();
}

// $deBlob: FUNCTION 005c85e0
void __stdcall AInGameState::GetTerrainStreamingPosition( TVector4* a_pOutPosition )
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	// The vista camera, else the blob of the current player when the camera follows it
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();

	if ( pPlayerManager->GetVistaCamera() != TNULL )
	{
		*a_pOutPosition = pPlayerManager->GetVistaCamera()->m_vPosition;
		return;
	}

	if ( ACameraManager::GetSingleton()->GetCurrentCameraType() == ACamera::CT_BLOBFOLLOW )
	{
		*a_pOutPosition = TSTATICCAST( ABlobEntity, pPlayerManager->GetCurrentPlayerBlob() )->GetBodyPosition();
		return;
	}

	*a_pOutPosition = pCamera->GetMatrix().GetTranslation();
}

// $deBlob: FUNCTION 005c8870
TBOOL AInGameState::CheckForControllerRemoval()
{
	return TTRUE;
}

// $deBlob: FUNCTION 005c89a0
TBOOL AInGameState::PauseOnControllerReinserted()
{
	return TTRUE;
}

// $deBlob: FUNCTION 005c7170
void AInGameState::OnLevelStartFinished()
{
	AMusicManager::GetSingleton()->SetPauseLocked( TFALSE );
	AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.0f );
}

#include "pch.h"
#include "Sound/ASoundTextureSwapMap.h"
#include "Render/Decal/ADecalManager.h"
#include "Platform/DX11/AStencilShadow_DX11.h"
#include "Sound/APositionalSoundManager.h"
#include "Entities/APickupManager.h"
#include "Cameras/ACameraShakeFunction.h"
#include "Render/Lighting/ALightingData.h"
#include "ALoadLevelState.h"
#include "AGameStateController.h"
#include "AXUIState.h"
#include "APlayerManager.h"
#include "Shaders/AShaderEffects.h"
#include "Cameras/ACameraManager.h"
#include "ACurrentLevelInformation.h"
#include "AApplication.h"
#include "AAssetStreaming.h"
#include "ARegionManager.h"
#include "GameController/AGameStatus.h"
#include "GUI/AGUIManager.h"
#include "Locale/ALocaleManager.h"
#include "Memory/AMemory.h"
#include "Render/ARenderer.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundLoadHelper.h"
#include "Sound/AStreamingSoundPlayer.h"
#include "XUI/ABlobControllerPopup.h"
#include "XUI/InGame/ABlobLoadingScreen.h"
#include "Entities/AEntityLoader.h"
#include "Entities/AEntityFactory.h"
#include "Entities/AEntityFramework.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Entities/APropWaterHydrant.h"
#include "Terrain2/ATerrain2.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Platform/DX11/AJellyShader_DX11.h"

#include <XUI/TXUI.h>
#include <Render/TAssetInit.h>
#include <Sound/TSound.h>
#include <Toshi/TStringManager.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Physics/Havok/TPhysicsManager.h>
#include <TParticleSystem/TParticleSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "FXComposition/AFXCompositionManager.h"
#include "Entities/AWaypointData.h"
#include "Entities/AZJumpData.h"
#include "Entities/ANavMarkers.h"
#include "Entities/PathFinding/AAINetwork.h"
#include "Fx/AEnvironmentFX.h"
#include "ParticleSystem/AEnvParticles.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ALoadLevelState* ALoadLevelState::s_pInstance;

// Loads the properties of the entities (Blob,...) and their common sound events
// $deBlob: FUNCTION 005ca1d0
static void LoadEntityProperties()
{
	ACameraShakeFunction::LoadShakeTypes();
	ABlobEntity::LoadProperties( TFALSE );
	ALeechBot2::LoadEffects();
	ABlobEntity::LoadEffects();
	APlayerManager::LoadEffects();
	APickupManager::LoadEffects();
	APropWaterHydrant::LoadEffects();
}

// The length of the loading screen's path
static TCONSTEXPR_VAR TINT s_iLen = 128;

// The loading screens of the SPLevels
static const TCHAR* s_aLoadingScreenTRBs[ AGameStatus::SPLevel_NUMOF ] = {
	"loadingscreen.trb",
	"loadingscreen_uptown.trb",
	"loadingscreen_downtown.trb",
	"loadingscreen_chromadam.trb",
	"loadingscreen_guggentraz.trb",
	"loadingscreen_thedocklands.trb",
	"loadingscreen_hanging_gardens.trb",
	"loadingscreen_chromacity.trb",
	"loadingscreen_ministryofink.trb",
	"loadingscreen_lakeraydia.trb",
};

static const TCHAR* s_aLoadingScreenXURs[ AGameStatus::SPLevel_NUMOF ] = {
	"loadingscreen.xur",
	"loadingscreen_uptown.xur",
	"loadingscreen_downtown.xur",
	"loadingscreen_chromadam.xur",
	"loadingscreen_guggentraz.xur",
	"loadingscreen_thedocklands.xur",
	"loadingscreen_hanging_gardens.xur",
	"loadingscreen_chromacity.xur",
	"loadingscreen_ministryofink.xur",
	"loadingscreen_lakeraydia.xur",
};

// $deBlob: FUNCTION 005c9040
// $deBlob: FUNCTION 005c9050
// $deBlob: FUNCTION 005c90d0
// $deBlob: FUNCTION 005c9150
// $deBlob: FUNCTION 005c9160
// $deBlob: FUNCTION 0053c670
// $deBlob: FUNCTION 008584e0
TDEFINE_CLASS( ALoadLevelState );

// $deBlob: FUNCTION 005c9170
ALoadLevelState::ALoadLevelState( AGameState* a_pNextState, ALevelInformation::LevelProperties* a_pLevelProperties, const TWCHAR* a_wszLevelName )
{
	m_wszLevelName     = a_wszLevelName;
	m_pNextState       = a_pNextState;
	m_State            = STATE_WaitForJobs;
	m_pEntityLoader    = TNULL;
	m_levelProperties  = a_pLevelProperties;
	m_bFadeIn          = TTRUE;
	m_pControllerPopup = TNULL;

	s_pInstance = this;
	AMusicManager::GetSingleton()->PauseMusicNow();

	AGameStatus*       pGameStatus       = AGameStatus::GetSingleton();
	ALevelInformation* pLevelInformation = ALevelInformation::GetSingleton();
	pGameStatus->SetCurrentLevel( pLevelInformation->GetLevelIndex( a_pLevelProperties ) );
	pGameStatus->InitLevelData( &pLevelInformation->GetLevelHeader()->m_pLevelProps[ pGameStatus->GetCurrentLevel() ] );

	TASSERT( TFALSE == AShaderEffects::IsSingletonCreated() );
	AShaderEffects::CreateSingleton();

	SetState( STATE_WaitForJobs );
}

// $deBlob: FUNCTION 005ca650
// $deBlob: FUNCTION 005c92b0
ALoadLevelState::~ALoadLevelState()
{
	s_pInstance = TNULL;
}

// $deBlob: FUNCTION 005ca160
static void CreateRegionManager()
{
	TTRB* pTRB = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_RegionRuntimeData );
	new ARegionManager( TSTATICCAST( ARegionManager::RegionRuntimeData, pTRB->GetSymbolAddress( "RegionRuntimeData" ) ) );
}

// Loads the AI network, the zjump data, the nav markers and the waypoint sets
// $deBlob: FUNCTION 005ca330
static TBOOL LoadNetwork()
{
	TBOOL bLoaded = AAINetwork::GetSingleton()->Load();

	if ( !bLoaded )
		return bLoaded;

	ACurrentLevelInformation* pLevelInfo = ACurrentLevelInformation::GetSingleton();

	new AZJumpData( pLevelInfo->Stream( ACurrentLevelInformation::DataBlock_ZJumpData ) );
	new ANavMarkers( pLevelInfo->Stream( ACurrentLevelInformation::DataBlock_NavMarkers ) );
	new AWaypointData( pLevelInfo->Stream( ACurrentLevelInformation::DataBlock_Waypoints ) );

	return TTRUE;
}

// $deBlob: FUNCTION 005c9320
AGameState::UpdateResult ALoadLevelState::OnUpdate( TFLOAT a_fDeltaTime )
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( m_bFadeIn && !pController->IsFading( 0 ) && pController->GetFade( 0 ) == 1.0f )
	{
		m_bFadeIn = TFALSE;
		pController->FadeFromBlack( 0.5f, 0 );
	}

	TXUI::GetSingleton()->TickInput( a_fDeltaTime );

	if ( m_pControllerPopup != TNULL )
		m_pControllerPopup->UpdateFrontEnd( a_fDeltaTime );

	switch ( m_State )
	{
		case STATE_WaitForJobs:
			if ( !AAssetStreaming::GetSingleton()->HasActiveJobs() )
				SetState( STATE_LoadingScreen );
			break;
		case STATE_LoadingScreen:
			if ( TSound::GetSingleton()->IsInitialised() )
			{
				SetState( STATE_Music );
				return UpdateResult_OK;
			}

			SetState( STATE_LevelData );
			return UpdateResult_OK;
		case STATE_LevelData:
			if ( !ACurrentLevelInformation::GetSingleton()->IsStreaming() )
			{
				TTRB* pPointData = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_PointData );
				AGameStatus::GetSingleton()->GetPointData().Load( pPointData );
				ASoundLoadHelper::GetSingleton()->AddLoadingLoopSounds();
				ASoundLoadHelper::GetSingleton()->SetPlayLoadingLoop( TTRUE );
				APositionalSoundManager::GetSingleton()->LoadEmitters( m_levelProperties );
				ASoundTextureSwapMap::GetSingleton()->AddSoundGroupsToLoad();

				if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() != AGameStatus::GAMETYPE_MP_NONE )
					ASoundLoadHelper::GetSingleton()->LoadGroupID( ASoundMap::GROUPID_LEVELS_MULTIPLAYER );

				SetState( STATE_RegionSounds );
				return UpdateResult_OK;
			}
			break;
		case STATE_Music:
			if ( !AAssetStreaming::GetSingleton()->HasActiveJobs() )
			{
				SetState( STATE_LevelData );
				return UpdateResult_OK;
			}
			break;
		case STATE_Entities:
			ACameraManager::GetSingleton()->LoadCameras();
			SetState( STATE_LoadEntities );
			return UpdateResult_OK;
		case STATE_RegionSounds:
			CreateRegionManager();

			if ( TSound::GetSingleton()->IsInitialised() )
			{
				ARegionManager::GetSingleton()->LoadRegionSounds( 0 );
				SetState( STATE_RegionSoundsLoading );
				return UpdateResult_OK;
			}

			SetState( STATE_FXParticles );
			return UpdateResult_OK;
		case STATE_RegionSoundsLoading:
		{
			if ( ARegionManager::GetSingleton()->IsLoadingSounds() )
			{
				ARegionManager::GetSingleton()->UpdateSoundLoading();
				return UpdateResult_OK;
			}

			SetState( STATE_ChallengeVoices );
			return UpdateResult_OK;
		}
		case STATE_ChallengeVoices:
		{
			// The streamed sounds (the challenge voices of LoadChallengeVO)
			if ( AStreamingSoundPlayer::GetSingleton()->AreStreamsReady() == TTRUE )
			{
				AStreamingSoundPlayer::GetSingleton()->AreStreamsOK();
				SetState( STATE_FXParticles );
				return UpdateResult_OK;
			}

			AStreamingSoundPlayer::GetSingleton()->Tick( 0.0f );
			return UpdateResult_OK;
		}
		case STATE_FXParticles:
		{
			TTRB* pFXParticles = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_FXParticles );
			TParticleSystem::GetSingleton()->LoadTRB( pFXParticles, "data/Blob_FX/FXParticles", TFALSE );
			SetState( STATE_EntityProperties );
			return UpdateResult_OK;
		}
		case STATE_EntityProperties:
			LoadEntityProperties();
			SetState( STATE_Terrain );
			return UpdateResult_OK;
		case STATE_Terrain:
		{
			if ( ARegionManager::GetSingleton()->IsLoading() )
			{
				ARegionManager::GetSingleton()->UpdateLoading();
				return UpdateResult_OK;
			}

			LoadGUI();
			ASoundTextureSwapMap::GetSingleton()->ResolveTextures();
			AEntityFramework::InitialiseEntityHash();
			SetState( STATE_EnvParticles );
			return UpdateResult_OK;
		}
		case STATE_EnvParticles:
		{
			if ( AEnvParticles::GetSingleton()->IsLoaded() )
			{
				SetState( STATE_AINetwork );
				return UpdateResult_OK;
			}

			break;
		}
		case STATE_AINetwork:
		{
			TBOOL bLoaded = LoadNetwork();

			if ( bLoaded )
			{
				SetState( STATE_FXCompositions );
				return UpdateResult_OK;
			}

			break;
		}
		case STATE_FXCompositions:
		{
			if ( AFXCompositionManager::GetSingleton()->IsDoneLoading() )
			{
				SetState( STATE_TerrainInstances );
				return UpdateResult_OK;
			}

			break;
		}
		case STATE_TerrainInstances:
		{
			if ( !ATerrain2Interface::GetInstance()->UpdateInstanceCreation( TTRUE ) )
			{
				AZJumpData::GetSingleton()->CreatePointEffects();
				SetState( STATE_TerrainStreaming );
				return UpdateResult_OK;
			}

			break;
		}
		case STATE_TerrainStreaming:
		{
			if ( !ATerrain2Interface::GetInstance()->IsStreaming() )
			{
				SetState( STATE_EnvironmentFX );
				return UpdateResult_OK;
			}

			break;
		}
		case STATE_EnvironmentFX:
		{
			if ( AEnvironmentFX::GetSingleton()->UpdateLoading() )
			{
				SetState( STATE_Unk11 );
				return UpdateResult_OK;
			}

			break;
		}
		case STATE_Unk11:
			SetState( STATE_Entities );
			return UpdateResult_OK;
		case STATE_LoadEntities:
			if ( !m_pEntityLoader->Update( a_fDeltaTime ) )
			{
				OnEntitiesLoaded();
				SetState( STATE_WaitForOK );
				return UpdateResult_OK;
			}

			break;
		case STATE_Done:
			ASoundLoadHelper::GetSingleton()->StopLoadingLoop();
			ASoundLoadHelper::GetSingleton()->UnloadLoadingLoopSounds();
			AMusicManager::GetSingleton()->CreateMusicStream();
			AMusicManager::GetSingleton()->SetPauseLocked( TTRUE );
			return UpdateResult_Remove;
	}

	return UpdateResult_OK;
}

// $deBlob: FUNCTION 005c97d0
void ALoadLevelState::OnInsertion()
{
	g_oTheApp.SetRenderWorld( TFALSE );
	ARenderer::GetSingleton()->SetLoadingScreen( TTRUE );
	AXUIState::SetSkin2( "skin.trb", "skin.xur" );
	AXUIState::SetInGameSkin( "ingame.trb" );
	AXUIState::SetSkinWithFont( AGameStatus::GetSingleton()->IsMultiplayer() ? "ingame_mp.trb" : "ingame_sp.trb", TTRUE );
	AGUIManager::GetSingleton()->SetCursorVisibleCount( APlayerManager::GetSingleton()->GetNumPlayersInline() );
}

// $deBlob: FUNCTION 005c9840
void ALoadLevelState::OnRemoval()
{
	TXUIScene* pScene = TXUI::GetSingleton()->GetActiveScene();
	TASSERT( TNULL != pScene || ( m_State != STATE_Done ) );

	while ( pScene != TNULL )
	{
		TXUIScene* pPrevScene = pScene->GetPrevScene();
		delete pScene;
		pScene = pPrevScene;
	}

	if ( m_oLoadingScreenTRB.GetSymbolTable() != TNULL )
	{
		TAssetInit::DeinitAssets( m_oLoadingScreenTRB );
		m_oLoadingScreenTRB.Close();
	}

	ACameraManager::GetSingleton()->GetCurrentCamera()->Lock( TFALSE );

	if ( m_State != STATE_Done )
	{
		if ( m_pNextState != TNULL )
			delete m_pNextState;

		return;
	}

	AGameStateController::GetSingleton()->PushState( m_pNextState );
}

// $deBlob: FUNCTION 005c9900
void ALoadLevelState::SetState( TINT a_eState )
{
	m_State = a_eState;

	switch ( a_eState )
	{
		case STATE_LoadingScreen:
		{
			AEntityFramework::Startup();
			TASSERT( TStringManager::String8Length( m_levelProperties->m_directory ) );

			m_oLoadingScreenTRB.SetMemoryFunctions( AMemory::TemporaryTRBAllocator, AMemory::TemporaryTRBDeallocator, TNULL );
			TMemoryHeap* pOldTRBHeap = AMemory::SetTRBHeap( TXUI::ms_pXUIMemoryBlock );

			TCHAR* szPath = TStringManager::GetTempString8();
			TStringManager::String8Copy( szPath, "data/xui/", s_iLen );
			TINT iLen = TStringManager::String8Length( szPath );
			TStringManager::String8Copy( szPath + iLen, TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() )->GetCurLocaleCode(), s_iLen - iLen );
			iLen = TStringManager::String8Length( szPath );
			TASSERT( iLen < s_iLen );
			szPath[ iLen ] = '/';

			AGameStatus*                        pGameStatus = AGameStatus::GetSingleton();
			ALevelInformation::LevelProperties* pProperties = pGameStatus->GetLevelProperties();
			const TCHAR*                        szTRB;
			const TCHAR*                        szXUR;

			if ( pProperties->m_sprintLevel == 0 )
			{
				if ( pGameStatus->GetSPLevel() == AGameStatus::SPLevel_Invalid )
					szTRB = pGameStatus->IsMultiplayer() ? "loadingscreen_multi.trb" : s_aLoadingScreenTRBs[ 0 ];
				else if ( pGameStatus->GetLevelMode() == AGameStatus::Mode_FreePaint )
					szTRB = "loadingsprint.trb";
				else
					szTRB = s_aLoadingScreenTRBs[ pGameStatus->GetSPLevel() ];
			}
			else
			{
				szTRB = "loadingsprint.trb";
			}

			TStringManager::String8Copy( szPath + iLen + 1, szTRB, s_iLen - ( iLen + 1 ) );
			TTRB::ERROR res = m_oLoadingScreenTRB.Load( szPath );
			TASSERT( res == TTRB::ERROR_OK );
			TAssetInit::InitAssets( m_oLoadingScreenTRB, TTRUE, TFALSE );
			AMemory::SetTRBHeap( pOldTRBHeap );

			if ( pProperties->m_sprintLevel == 0 )
			{
				if ( pGameStatus->GetSPLevel() == AGameStatus::SPLevel_Invalid )
					szXUR = pGameStatus->IsMultiplayer() ? "loadingscreen_multi.xur" : s_aLoadingScreenXURs[ 0 ];
				else if ( pGameStatus->GetLevelMode() == AGameStatus::Mode_FreePaint )
					szXUR = "loadingsprint.xur";
				else
					szXUR = s_aLoadingScreenXURs[ pGameStatus->GetSPLevel() ];
			}
			else
			{
				szXUR = "loadingsprint.xur";
			}

			TXUIScene* pScene = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( szXUR ), TUINT32( 0 ) );
			TASSERT( pScene->IsA( &TGetClass( ABlobLoadingScreen ) ) );
			TSTATICCAST( ABlobLoadingScreen, pScene )->SetLevelName( m_wszLevelName );
			TXUI::GetSingleton()->TransitionTo( TNULL, pScene );

			TXUIScene* pPopup = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "controllerscreen.xur" ), L"ControllerPopup", TTRUE );
			TASSERT( pPopup == TNULL || pPopup->IsA( &TGetClass( ABlobControllerPopup ) ) );
			m_pControllerPopup = TSTATICCAST( ABlobControllerPopup, pPopup );
			m_pControllerPopup->RemoveSelf();
			return;
		}
		case STATE_LevelData:
		{
			TINT                      iMultiPlayerGameType     = AGameStatus::GetSingleton()->GetMultiPlayerGameType();
			ACurrentLevelInformation* pCurrentLevelInformation = new ACurrentLevelInformation( m_levelProperties );
			pCurrentLevelInformation->Create( iMultiPlayerGameType > -1 );

			ADecalManager::GetSingleton()->Initialise();
			return;
		}
		case STATE_Music:
			break;
		case STATE_ChallengeVoices:
			AStreamingSoundPlayer::GetSingleton()->LoadChallengeVO( TINT( TREINTERPRETCAST( TUINTPTR, m_levelProperties ) ) );
			return;
		case STATE_Terrain:
		{
			ATerrain2* pTerrain    = new ATerrain2();
			TTRB*      pTerrainTRB = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_Terrain );
			pTerrain->Load( pTerrainTRB, m_levelProperties->m_iTerrainStreaming == 1, 14, 28 );

			if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() > -1 )
			{
				TTRB* pMPData = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_MPData );
				pTerrain->LoadMPData( pMPData );
			}

			// NOTE: retail allocates 0x18 bytes for the 4 bytes ALightingData
			new ALightingData();
			ALightingData::GetSingleton()->Load( pTerrainTRB );

			ARegionManager::GetSingleton()->LoadRegion( 0 );
			InitialisePhysics();
			return;
		}
		case STATE_EnvParticles:
			new AEnvParticles( 500 );
			AEnvParticles::GetSingleton()->Load( "Data/EnvFX/EnvParticles.trb" );
			return;
		case STATE_AINetwork:
			AAINetwork::GetSingleton()->InitLoad();
			return;
		case STATE_FXCompositions:
			AFXCompositionManager::GetSingleton()->LoadFromLevel( 0x16 );
			return;
		case STATE_TerrainInstances:
			( new ATerrain2Interface( 0 ) )->StartInstanceCreation();
			return;
		case STATE_TerrainStreaming:
		{
			ATerrain2Interface* pTerrainInterface = ATerrain2Interface::GetInstance();

			if ( m_levelProperties->m_iTerrainStreaming != 0 )
			{
				APointDataFunction::Point* pStartPoint = AGameStatus::GetSingleton()->GetPointData().GetPoint( 1 );
				TVector4                   vStartPos( 0.0f, 0.0f, 0.0f, 1.0f ); // TVector4::X0Y0Z0W1

				if ( pStartPoint != TNULL )
					vStartPos = pStartPoint->m_vPosition;

				vStartPos.y -= 10.0f;

				TVector4 vDirection = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD );
				vDirection.w        = 1.0f;

				pTerrainInterface->Init( &vStartPos, &vDirection );
				return;
			}

			pTerrainInterface->InitAllCells();
			return;
		}
		case STATE_EnvironmentFX:
		{
			new AEnvironmentFX();
			AEnvironmentFX::GetSingleton()->CreateSky();

			const TCHAR* szEnvFX = m_levelProperties->m_szEnvFX;

			if ( TStringManager::String8CompareNoCase( "default", szEnvFX ) != 0 && TStringManager::String8Length( szEnvFX ) != 0 )
			{
				TCHAR* szFileName = TStringManager::GetTempString8();
				T2String8::Format( szFileName, 0x80, "data/envfx/%s.trb", szEnvFX );
				AEnvironmentFX::GetSingleton()->LoadSettings( szFileName, TFALSE );
				return;
			}

			if ( AGameStatus::GetSingleton()->IsMultiplayer() )
			{
				AEnvironmentFX::GetSingleton()->LoadSettings( ACurrentLevelInformation::GetDataBlockFileName( ACurrentLevelInformation::DataBlock_EnvFXDefaultMP ), TTRUE );
				return;
			}

			AEnvironmentFX::GetSingleton()->LoadSettings( ACurrentLevelInformation::GetDataBlockFileName( ACurrentLevelInformation::DataBlock_EnvFXDefault ), TTRUE );
		}
			return;
		case STATE_LoadEntities:
			m_pEntityLoader = new AEntityLoader();
			return;
		default:
			return;
	}

	AMusicManager* pMusicManager = AMusicManager::GetSingleton();
	pMusicManager->SetMaxScore( m_levelProperties->m_iMaxScore );

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() != AGameStatus::GAMETYPE_MP_NONE )
		pMusicManager->SetRandomMusicOverride();

	if ( pMusicManager->UsingMusicOverride() == TTRUE && m_levelProperties->m_sprintLevel == 0 )
	{
		pMusicManager->LoadMusicOverride( TTRUE );
		return;
	}

	if ( TStringManager::String8Length( m_levelProperties->m_szMusic ) == 0 )
	{
		pMusicManager->LoadMusicData( "default", TTRUE, TTRUE );
		return;
	}

	pMusicManager->LoadMusicData( m_levelProperties->m_szMusic, TTRUE, TTRUE );
}

// $deBlob: FUNCTION 005ca420
void ALoadLevelState::InitialisePhysics()
{
	TPhysicsManager*       pPhysicsManager = TPhysicsManager::GetSingleton();
	ATerrain2::RegionInfo* pRegion         = ATerrain2::GetInstance()->GetRegion( 0 )->m_pInfo;

	TVector4 vMin( pRegion->m_fX - 150.0f, -500.0f, pRegion->m_fZ - 150.0f, 1.0f );
	TVector4 vMax( TFLOAT( pRegion->m_uiSizeX + 2 ) * 150.0f + vMin.x, 500.0f, TFLOAT( pRegion->m_uiSizeZ + 2 ) * 150.0f + vMin.z, 1.0f );

	if ( TStringManager::String8CompareNoCase( "spaceship", m_levelProperties->m_szName, -1 ) == 0 )
	{
		vMin.y = -1400.0f;
		vMax.y = 1100.0f;
	}

	TPhysicsManagerInitData oInitData;
	oInitData.m_fTimeStep      = 1.0f / 60.0f;
	oInitData.m_vBroadPhaseMin = vMin;
	oInitData.m_vBroadPhaseMax = vMax;
	vMax.Multiply( TVector4::VEC_POSY, 80.0f );
	oInitData.m_vGravity                      = vMax;
	oInitData.m_iUnk34                        = 5500;
	oInitData.m_iMaxCustomMovers              = 800;
	oInitData.m_iMaxConstraints               = 200;
	oInitData.m_iMaxCollisionCallbacks        = 400;
	oInitData.m_iMaxTriggerCallbacks          = 100;
	oInitData.m_iMaxConstraintBrokenCallbacks = 100;
	pPhysicsManager->Initialise( &oInitData );
}

// $deBlob: FUNCTION 005ca200
void ALoadLevelState::LoadGUI()
{
	TASSERT( !AAssetStreaming::GetSingleton()->HasActiveJobs() );

	TBOOL bRes = AJellyShader_DX11::GetSingleton()->LoadSolids();
	TASSERT( TTRUE == bRes );

	new AStencilShadow();

	TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() )->InitWaterTexture();
}

// $deBlob: FUNCTION 005ca2f0
void ALoadLevelState::OnEntitiesLoaded()
{
	if ( m_pEntityLoader != TNULL )
		delete m_pEntityLoader;

	m_pEntityLoader = TNULL;
	AEntityFactory::GetSingleton()->UpdateEntitiesLOD();
	APlayerManager::GetSingleton()->LoadPatternTextures();
}

#include "pch.h"
#include "Entities/ABoss/ABoss.h"
#include "Entities/APickupBonus.h"
#include "APlayerManager.h"
#include <TParticleSystem/TParticleSystem.h>
#include "Entities/AEntityFactory.h"
#include "GameController/AGameStatus.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "ARandom.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "ALevelInformation.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Sound/AMusicManager.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ABlobFollowCamera.h"
#include "Buildings/ABuildingManager.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "Cameras/AScriptedCamera.h"
#include "Cameras/AKeyFrameCamera.h"
#include "Challenges/AChallengeManager.h"
#include "Entities/APropTriggerHelp.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "Options/AOptions.h"
#include "Terrain2/ATerrain2.h"
#include "XUI/InGame/HUD/ABlobLetterBoxHud.h"
#include "XUI/InGame/HUD/ABlobLifeHud.h"
#include "Render/ARenderer.h"
#include <XUI/TXUI.h>
#include "Render/AXYZViewportManager.h"
#include "ARegionManager.h"
#include "Terrain2/ATerrain2Interface.h"
#include <Math/TQuaternion.h>
#include "Buildings/ALandmarkInstance.h"
#include "Buildings/AGrassInstance.h"
#include "Buildings/ATreeInstance.h"
#include "Sound/APositionalSoundManager.h"
#include "XUI/InGame/HUD/ABlobVistaHud.h"
#include "GUI/AFontManager.h"
#include "Locale/ALocaleManager.h"
#include <GUI/T2GUI.h>

#include <Platform/DX11/TTexture_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APlayerManager::BlobData ) == 0xB8 );

TSTATICASSERT( sizeof( APlayerManager ) == 0x1E74 );

// $deBlob: FUNCTION 005cb610
APlayerManager::BlobData::BlobData()
{
	m_bKeyFrameCameraActive = TFALSE;
	m_auiUnk09[ 0 ]         = 0;
	m_auiUnk09[ 1 ]         = 0;
	m_pBlob                 = TNULL;
	m_pBlobCamera           = TNULL;
	m_pKeyFrameCamera       = TNULL;
	m_pScriptedCamera       = TNULL;
	m_pRespawnCamera        = TNULL;
	m_pBossCamera           = TNULL;
	m_uiLifeHud             = 0;
	m_pSSCamTrigger         = TNULL;
	m_iColour               = 0;

	Reset();
}

// Counts down the power-up timer (m_fPowerUpTimer); once it runs out the power-up (m_iPowerUpType) ends
// $deBlob: FUNCTION 005cb780
void APlayerManager::BlobData::UpdatePowerUpTimer( TFLOAT a_fDeltaTime )
{
	if ( m_fPowerUpTimer > 0.0f )
	{
		m_fPowerUpTimer -= a_fDeltaTime;

		if ( m_fPowerUpTimer <= 0.0f )
			m_iPowerUpType = -1;
	}
}

// $deBlob: FUNCTION 005cb7b0
void APlayerManager::BlobData::Reset()
{
	m_fPowerUpTimer       = 0.0f;
	m_uiStencilsCollected = 0;
	m_fPowerUpTime        = 0.0f;
	m_fSSCamTriggerTime   = 0.0f;
	m_uiUnk38             = 0;
	m_bUnk60              = TFALSE;
	m_uiUnk64             = 0;
	m_bUnk68              = TFALSE;
	m_auiUnk09[ 0 ]       = 0;
	m_auiUnk09[ 1 ]       = 0;
	m_iCameraState        = 5;
	m_iNextCameraMode     = -1;
	m_eCameraMode         = -1;
	// NOTE: retail writes 0xFFFF to both bytes as a TINT16
	m_iPowerUpType     = -1;
	m_iUnk4D           = -1;
	m_iPattern         = -1;
	m_iQueuedAnimState = -1;
	m_uiPatternMask    = 0;

	for ( TINT i = 8; i != 0; i-- )
		m_auiBonusCollected[ i - 1 ] = 0;

	m_uiUnk58               = 0;
	m_pSSCamTrigger         = TNULL;
	m_iHelpTopic            = -1;
	m_iHelpString           = -1;
	m_bKeyFrameCameraActive = TFALSE;
}

// $deBlob: FUNCTION 005cba40
APlayerManager::APlayerManager()
{
	m_iNumPlayers          = 0;
	m_uiCurrentPlayer      = 0xFF;
	m_pVistaCam            = TNULL;
	m_eVistaState          = VistaState_Start;
	m_uiSSCameraDelay      = 0;
	m_uiNumQueuedSSCameras = 0;
}

// $deBlob: FUNCTION 005cbb40
// $deBlob: FUNCTION 0056c1f0
APlayerManager::~APlayerManager()
{
	RemovePlayers();
}

// Updates the blobs of the players after the physics step (AInGameState::OnUpdate)
// $deBlob: FUNCTION 005cc140
void APlayerManager::Update( TFLOAT a_fDeltaTime )
{
	for ( TINT i = MAX_PLAYERS; i != 0; )
	{
		i--;
		BlobData& rData = m_aBlobData[ i ];

		if ( rData.m_pBlob != TNULL )
		{
			if ( 0.0f < rData.m_fPowerUpTimer )
			{
				rData.m_fPowerUpTimer -= a_fDeltaTime;

				if ( rData.m_fPowerUpTimer <= 0.0f )
					rData.m_iPowerUpType = -1;
			}

			TSTATICCAST( ABlobEntity, rData.m_pBlob )->PostPhysicsUpdate( a_fDeltaTime, rData.m_iController );

			if ( rData.m_iPowerUpType == -1 && rData.m_iUnk4D != -1 )
				rData.m_iUnk4D = -1;

			if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
				UpdateCameraRegions( a_fDeltaTime, i, TFALSE );
		}
	}

	UpdateCameraStates( a_fDeltaTime );

	// Single player with a key frame camera on the HUD: the letterbox comes up
	if ( m_iNumPlayers == 1 && m_aBlobData[ 0 ].m_eCameraMode == 4 )
	{
		TXUI*      pXUI         = TXUI::GetSingleton();
		TXUIScene* pActiveScene = pXUI->GetActiveScene();

		if ( pActiveScene != TNULL && pActiveScene->IsA( &TGetClass( ABlobHud ) ) && pXUI->GetTransToScene() == TNULL && pXUI->GetTransBackFromScene() == TNULL && !pXUI->IsInputLocked() )
		{
			TXUIResource* pResource = pXUI->FindResource( "letterbox.xur" );
			pXUI->TransitionTo( TNULL, pXUI->CreateScene( pResource, 0 ) );
		}
	}
}

// Draws the blobs of the players (AInGameState::HandleMenuRequestToCancel)
// $deBlob: FUNCTION 005ccaf0
void APlayerManager::Render( TFLOAT a_fDeltaTime )
{
	for ( TINT i = MAX_PLAYERS; i != 0; )
	{
		i--;
		if ( m_aBlobData[ i ].m_pBlob != TNULL )
			TSTATICCAST( ABlobEntity, m_aBlobData[ i ].m_pBlob )->Render();
	}
}

// Creates the blobs, cameras and HUD of the players (AInGameState::OnInsertion)
// $deBlob: FUNCTION 005ccb20
void APlayerManager::CreatePlayers()
{
	ABlobEntity::ms_bControlsLocked = TFALSE;

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
		AddPlayer( TUINT8( i ), TINT8( i ), TNULL );

	if ( !AGameStatus::GetSingleton()->IsMultiplayer() || m_iNumPlayers != 1 )
	{
		const TCHAR*  szResource;
		const TWCHAR* wszScene;

		if ( m_iNumPlayers == 1 )
		{
			szResource = "hud.xur";
			wszScene   = L"HUD";
		}
		else if ( m_iNumPlayers == 2 )
		{
			szResource = "hud_2p.xur";
			wszScene   = L"HUD_2P";
		}
		else if ( m_iNumPlayers == 3 )
		{
			szResource = "hud_3p.xur";
			wszScene   = L"HUD_3P";
		}
		else
		{
			szResource = "hud_4p.xur";
			wszScene   = L"HUD_4P";
		}

		TXUI* pXUI = TXUI::GetSingleton();
		pXUI->SetFocussedScene( pXUI->CreateScene( pXUI->FindResource( szResource ), wszScene, TFALSE ) );
	}

	TINT iGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

	if ( iGameType >= 0 && iGameType < 3 )
	{
		// The multiplayer game types 0-2 start at random start points
		AssignMultiplayerPlayers();
		SetupColours();
		return;
	}

	if ( iGameType < 0 || iGameType == 3 )
		AssignPlayers( TFALSE );

	SetupColours();
}

// Puts the blobs at the level's start points (point types 1-4)
// $deBlob: FUNCTION 005cce80
void APlayerManager::AssignPlayers( TBOOL a_bUnused )
{
	TINT iTotalPlayers = m_iNumPlayers;
	TASSERT( iTotalPlayers <= MAX_PLAYERS );

	TBOOL abDropOnto[ MAX_PLAYERS ];
	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
		abDropOnto[ i ] = TFALSE;

	TVector4 avStartPos[ MAX_PLAYERS ];
	TINT     iTotalStartPos = 0;

	for ( TINT iType = 1; iType < 5; iType++ )
	{
		APointDataFunction::Point* pPoint = AGameStatus::GetSingleton()->GetPointData().GetPoint( iType );

		if ( pPoint != TNULL )
		{
			avStartPos[ iTotalStartPos ] = pPoint->m_vPosition;
			abDropOnto[ iTotalStartPos ] = pPoint->m_uiFlags & 1;
			iTotalStartPos++;
		}
	}

	TASSERT( iTotalStartPos >= iTotalPlayers );

	for ( TINT i = 0; i < iTotalPlayers; i++ )
	{
		ABaseEntity* pBlob = GetNthBlob( i );

		if ( pBlob != TNULL )
		{
			if ( !abDropOnto[ i ] )
				avStartPos[ i ].y -= ABlobEntity::GetProperties( ABlobEntity::BLOB_SIZE_ABS_MAX )->m_fRadius + 0.5f;

			TSTATICCAST( ABlobEntity, pBlob )->SetStartPosition( avStartPos[ i ], TTRUE, TFALSE );

			if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint ) // != AGameStatus::Mode_FreePaint
			{
				UpdateCameraRegions( 0.0f, i, TTRUE );
			}
		}
	}

	for ( TINT i = 0; i < iTotalPlayers; i++ )
	{
		TASSERT( GetNthBlob( i ) == m_aBlobData[ i ].m_pBlob );

		m_aBlobData[ i ].m_iQueuedAnimState = abDropOnto[ i ] ? ABlobAnimationComponent::STATE_LEVEL_START : ABlobAnimationComponent::STATE_BALL;

		if ( m_aBlobData[ i ].m_eCameraMode == 4 || m_aBlobData[ i ].m_iNextCameraMode == 4 )
		{
			TASSERT( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint );
			TSTATICCAST( ABlobEntity, m_aBlobData[ i ].m_pBlob )->SetLevelStartAttached( TTRUE );
		}
		else
		{
			StartQueuedAnimState( i );
		}
	}
}

// $deBlob: FUNCTION 005ccc10
void APlayerManager::AssignMultiplayerPlayers()
{
	TINT     iTotalPlayers  = m_iNumPlayers;
	TINT     iTotalStartPos = 0;
	TVector4 avStartPos[ MAX_START_POSITIONS ];

	// Every start point of the four types
	for ( TINT iNth = 1; iNth < 5; iNth++ )
	{
		for ( TINT iType = 1; iType < 5; iType++ )
		{
			APointDataFunction::Point* pPoint = AGameStatus::GetSingleton()->GetPointData().GetNthPoint( iType, iNth );

			if ( pPoint != TNULL )
			{
				TASSERT( iTotalStartPos < MAX_START_POSITIONS );
				avStartPos[ iTotalStartPos++ ] = pPoint->m_vPosition;
			}
		}
	}

	TASSERT( iTotalStartPos >= iTotalPlayers );

	TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

	// Each blob takes a random one (lifted by the biggest blob's radius)
	for ( TINT i = 0; i < iTotalPlayers; i++ )
	{
		ABaseEntity* pBlob = GetNthBlob( i );

		if ( pBlob == TNULL )
			continue;

		TASSERT( iTotalStartPos > 0 );
		TINT iStartPos = rRandom.GetInt( iTotalStartPos );

		avStartPos[ iStartPos ].y -= ABlobEntity::GetProperties( ABlobEntity::BLOB_SIZE_ABS_MAX )->m_fRadius + 0.5f;
		TSTATICCAST( ABlobEntity, pBlob )->SetStartPosition( avStartPos[ iStartPos ], TTRUE, TFALSE );

		for ( TINT j = iStartPos; j < iTotalStartPos - 1; j++ )
			avStartPos[ j ] = avStartPos[ j + 1 ];

		iTotalStartPos--;
	}
}

// The a_iIndex-th blob of the players that have one, TNULL if there are fewer
// $deBlob: FUNCTION 005d0690
ABaseEntity* APlayerManager::GetActiveBlob( TINT a_iIndex ) const
{
	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		if ( m_aBlobData[ i ].m_pBlob != TNULL && --a_iIndex < 0 )
			return m_aBlobData[ i ].m_pBlob;
	}

	return TNULL;
}

// $deBlob: FUNCTION 005d06d0
void APlayerManager::AssignStartPositions( TBOOL a_bUnk )
{
	TINT iGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

	if ( iGameType < 0 )
	{
		AssignPlayers( a_bUnk );
		return;
	}

	if ( iGameType < 3 )
		AssignMultiplayerPlayers();
	else if ( iGameType == 3 )
		AssignPlayers( TFALSE );
}

// $deBlob: FUNCTION 005cf950
void APlayerManager::GetTeamColours( TUINT32& a_riColor1, TUINT32& a_riColor2 )
{
	TASSERT( 4 == GetNumPlayers() );

	TUINT32 uiColour0 = TSTATICCAST( ABlobEntity, m_aBlobData[ 0 ].m_pBlob )->GetColourComponent()->GetUnk20();
	TUINT32 uiColour2 = TSTATICCAST( ABlobEntity, m_aBlobData[ 2 ].m_pBlob )->GetColourComponent()->GetUnk20();

	a_riColor1 = ABlobColourComponent::MixColours( uiColour0, TSTATICCAST( ABlobEntity, m_aBlobData[ 1 ].m_pBlob )->GetColourComponent()->GetUnk20() );
	a_riColor2 = ABlobColourComponent::MixColours( uiColour2, TSTATICCAST( ABlobEntity, m_aBlobData[ 3 ].m_pBlob )->GetColourComponent()->GetUnk20() );

	// The mixes are the same: the teams keep their first players' colours
	if ( a_riColor1 == a_riColor2 )
	{
		a_riColor1 = uiColour0;
		a_riColor2 = uiColour2;
	}

	TASSERT( a_riColor1 != a_riColor2 );
}

// Starts the queued animation state of a player's blob: the level start (0xF, the blob attached to the level start
// gate) or the normal start (the music plays again)
// $deBlob: FUNCTION 005d0b00
void APlayerManager::StartQueuedAnimState( TINT a_iPlayer )
{
	TASSERT( GetNthBlob( a_iPlayer ) == m_aBlobData[ a_iPlayer ].m_pBlob );
	TASSERT( -1 != m_aBlobData[ a_iPlayer ].m_iQueuedAnimState );

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, m_aBlobData[ a_iPlayer ].m_pBlob );
	pBlob->GetAnimationComponent()->SetState( m_aBlobData[ a_iPlayer ].m_iQueuedAnimState );

	if ( m_aBlobData[ a_iPlayer ].m_iQueuedAnimState == ABlobAnimationComponent::STATE_LEVEL_START )
	{
		pBlob->SetLevelStartAttached( TTRUE );
		AGameStatus::GetSingleton()->SetLevelStart( TTRUE, TFALSE );
		m_aBlobData[ a_iPlayer ].m_iQueuedAnimState = -1;
		return;
	}

	pBlob->SetLevelStartAttached( TFALSE );
	AMusicManager::GetSingleton()->SetPauseLocked( TFALSE );
	AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.25f );
	m_aBlobData[ a_iPlayer ].m_iQueuedAnimState = -1;
}

// $deBlob: FUNCTION 005ce7a0
TINT8 APlayerManager::GetPlayerIndex( ABaseEntity* a_pBlob ) const
{
	if ( a_pBlob == TNULL )
		return -1;

	for ( TINT8 i = MAX_PLAYERS; i != 0; )
	{
		i--;

		if ( a_pBlob == m_aBlobData[ i ].m_pBlob )
			return i;
	}

	return -1;
}

// Creates the blob of player a_uiPlayer (with controller a_iController) and its cameras, at a_pPosition if not TNULL;
// returns the blob.
// Removes the blob and the cameras of a player (AddPlayer first frees the slot)
// $deBlob: FUNCTION 005ce7d0
void APlayerManager::RemovePlayer( TUINT8 a_uiPlayer )
{
	TASSERT( a_uiPlayer >= 0 && a_uiPlayer < MAX_PLAYERS );
	BlobData& rData = m_aBlobData[ a_uiPlayer ];

	if ( rData.m_pBlob != TNULL )
	{
		ACamera** apCameras[ 4 ] = {
			TREINTERPRETCAST( ACamera**, &rData.m_pBlobCamera ),
			TREINTERPRETCAST( ACamera**, &rData.m_pKeyFrameCamera ),
			TREINTERPRETCAST( ACamera**, &rData.m_pRespawnCamera ),
			TREINTERPRETCAST( ACamera**, &rData.m_pBossCamera ),
		};

		for ( TINT i = 0; i < 4; i++ )
		{
			if ( *apCameras[ i ] != TNULL )
			{
				TASSERT( ARenderer::IsSingletonCreated() );
				ARenderer::GetSingleton()->GetViewportManager()->RemoveCamera( *apCameras[ i ] );
				ACameraManager::GetSingleton()->DestroyCamera( *apCameras[ i ] );
				*apCameras[ i ] = TNULL;
			}
		}

		rData.m_pScriptedCamera = TNULL;
		rData.m_eCameraMode     = -1;
		AEntityFactory::GetSingleton()->DestroyEntity( rData.m_pBlob );
		rData.Reset();
	}

	rData.m_pBlob       = TNULL;
	rData.m_iController = -1;

	// The current player moves to the first player left
	if ( TUINT8( m_uiCurrentPlayer & 0xFF ) == a_uiPlayer )
	{
		*TREINTERPRETCAST( TUINT8*, &m_uiCurrentPlayer ) = 0xFF;

		for ( TINT i = MAX_PLAYERS; i != 0; )
		{
			i--;
			if ( m_aBlobData[ i ].m_pBlob != TNULL )
				*TREINTERPRETCAST( TUINT8*, &m_uiCurrentPlayer ) = TUINT8( i );
		}
	}
}

// $deBlob: FUNCTION 005cea70
void APlayerManager::RemovePlayers()
{
	for ( TINT i = MAX_PLAYERS - 1; i >= 0; i-- )
		RemovePlayer( TUINT8( i ) );
}

// $deBlob: FUNCTION 005cf120
void APlayerManager::DestroyPlayers()
{
	RemovePlayers();
	m_uiSSCameraDelay      = 0;
	m_uiNumQueuedSSCameras = 0;
}

// $deBlob: FUNCTION 005cd120
ABaseEntity* APlayerManager::AddPlayer( TUINT8 a_uiPlayer, TINT8 a_iController, const Toshi::TVector4* a_pPosition )
{
	TASSERT( a_uiPlayer >= 0 && a_uiPlayer < MAX_PLAYERS );

	RemovePlayer( a_uiPlayer );

	BlobData& rData = m_aBlobData[ a_uiPlayer ];

	ABaseEntity* pBlob = AEntityFactory::GetSingleton()->CreateEntity( "ABlobEntity" );
	TASSERT( ( pBlob == TNULL ) || pBlob->IsA( &TGetClass( ABlobEntity ) ) );
	rData.m_pBlob = pBlob;

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();

	ACamera* pCamera = pCameraManager->CreateCamera( ACamera::CT_BLOBFOLLOW );
	TASSERT( ( pCamera == TNULL ) || pCamera->IsA( &TGetClass( ABlobFollowCamera ) ) );
	rData.m_pBlobCamera = TSTATICCAST( ABlobFollowCamera, pCamera );

	pCamera = pCameraManager->CreateCamera( ACamera::CT_KEYFRAME );
	TASSERT( ( pCamera == TNULL ) || pCamera->IsA( &TGetClass( AKeyFrameCamera ) ) );
	rData.m_pKeyFrameCamera = TSTATICCAST( AKeyFrameCamera, pCamera );

	pCamera = pCameraManager->CreateCamera( ACamera::CT_SCRIPTED );
	TASSERT( ( pCamera == TNULL ) || pCamera->IsA( &TGetClass( AScriptedCamera ) ) );
	rData.m_pRespawnCamera = TSTATICCAST( AScriptedCamera, pCamera );
	rData.m_pRespawnCamera->InitialiseActor( "Blob_Respawn_Camera.trb", "camera_Loc", TNULL );
	rData.m_pScriptedCamera = rData.m_pRespawnCamera;

	// The boss camera (the spaceship level)
	if ( ABoss::GetSingleton() != TNULL || AGameStatus::GetSingleton()->GetSPLevel() == 9 )
	{
		pCamera = pCameraManager->CreateCamera( ACamera::CT_SCRIPTED );
		TASSERT( ( pCamera == TNULL ) || pCamera->IsA( &TGetClass( AScriptedCamera ) ) );
		rData.m_pBossCamera = TSTATICCAST( AScriptedCamera, pCamera );
		rData.m_pBossCamera->InitialiseActor( "Bossmech_camera.trb", "Bossmech_LB_Camera", TNULL );
	}

	rData.m_pBlobCamera->SetTarget( TSTATICCAST( ABlobEntity, rData.m_pBlob ) );
	rData.m_eCameraMode = 1;
	rData.ResetLives();

	// NOTE: retail reads and writes the low byte of m_uiCurrentPlayer
	if ( *TREINTERPRETCAST( TINT8*, &m_uiCurrentPlayer ) == -1 )
		*TREINTERPRETCAST( TUINT8*, &m_uiCurrentPlayer ) = a_uiPlayer;

	rData.m_iController = a_iController;

	// The start position (10 lower)
	if ( a_pPosition != TNULL )
	{
		TVector4 vPosition( a_pPosition->x, a_pPosition->y - 10.0f, a_pPosition->z, a_pPosition->w );
		TSTATICCAST( ABlobEntity, rData.m_pBlob )->SetStartPosition( vPosition, TTRUE, TFALSE );
	}

	return rData.m_pBlob;
}

// $deBlob: FUNCTION 005cd850
ACamera* APlayerManager::GetPlayerCamera( TUINT8 a_uiPlayer )
{
	TASSERT( a_uiPlayer >= 0 && a_uiPlayer < MAX_PLAYERS );

	BlobData& rData = m_aBlobData[ a_uiPlayer ];

	if ( rData.m_eCameraMode == 4 )
	{
		TASSERT( ( rData.m_pKeyFrameCamera == TNULL ) || rData.m_pKeyFrameCamera->IsA( &TGetClass( ACamera ) ) );
		return rData.m_pKeyFrameCamera;
	}

	ACamera* pCamera;

	if ( rData.m_eCameraMode == 2 )
		pCamera = rData.m_pScriptedCamera;
	else
		pCamera = rData.m_pBlobCamera;

	TASSERT( ( pCamera == TNULL ) || pCamera->IsA( &TGetClass( ACamera ) ) );
	return pCamera;
}

// $deBlob: FUNCTION 005cd920
ACamera* APlayerManager::GetBlobCamera( ABaseEntity* a_pBlob )
{
	if ( a_pBlob != TNULL )
	{
		for ( TINT8 i = MAX_PLAYERS; i != 0; )
		{
			i--;
			if ( a_pBlob == m_aBlobData[ i ].m_pBlob )
				return GetPlayerCamera( TUINT8( i ) );
		}
	}

	return GetPlayerCamera( 0xFF );
}

// $deBlob: FUNCTION 005cda50
void APlayerManager::PlaySSCamera( TUINT8 a_uiPlayer, TINT a_iCamera )
{
	TASSERT( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint );
	TASSERT( a_uiPlayer >= 0 && a_uiPlayer < MAX_PLAYERS );

	if ( a_iCamera < 0 )
		return;

	BlobData& rData = m_aBlobData[ a_uiPlayer ];

	TBOOL bOk = rData.m_pKeyFrameCamera->SetCamera( a_iCamera );
	TASSERT( bOk );

	const ASSCameraFunction::Camera* pCamera = rData.m_pKeyFrameCamera->GetSSCamera();

	if ( pCamera != TNULL )
	{
		rData.m_iHelpTopic      = TINT16( pCamera->m_iHelpTopic );
		rData.m_iHelpString     = pCamera->m_iHelpString;
		rData.m_iNextCameraMode = 4; // the key frame camera

		if ( m_iNumPlayers == 1 )
		{
			AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TTRUE;
			AMusicManager::GetSingleton()->OnEmptyEvent1();
		}

		rData.m_iCameraState  = 0;
		rData.m_auiUnk09[ 0 ] = 0;
		rData.m_auiUnk09[ 1 ] = 0;

		UpdateCameraStates( 0.0f );
	}
}

// $deBlob: FUNCTION 005cdce0
void APlayerManager::OnSSCameraChanged( const ASSCameraFunction::Camera* a_pPrev, const ASSCameraFunction::Camera* a_pNext )
{
	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		BlobData& rData = m_aBlobData[ i ];

		if ( ( rData.m_bKeyFrameCameraActive || rData.m_eCameraMode == 4 || rData.m_iNextCameraMode == 4 ) && rData.m_pKeyFrameCamera != TNULL &&
		     a_pNext->m_iHelpTopic != -1 && a_pNext->m_iHelpTopic != 0 )
		{
			rData.m_iHelpTopic  = TINT16( a_pNext->m_iHelpTopic );
			rData.m_iHelpString = a_pNext->m_iHelpString;
		}
	}
}

// $deBlob: FUNCTION 005cd610
ABlobFollowCamera* APlayerManager::GetBlobFollowCamera( ABaseEntity* a_pBlob )
{
	TUINT8 uiPlayer = 0xFF;

	if ( a_pBlob != TNULL )
	{
		for ( TINT8 i = MAX_PLAYERS; i != 0; )
		{
			i--;
			if ( a_pBlob == m_aBlobData[ i ].m_pBlob )
			{
				uiPlayer = TUINT8( i );
				break;
			}
		}
	}

	TASSERT( uiPlayer >= 0 && uiPlayer < MAX_PLAYERS );

	if ( uiPlayer >= MAX_PLAYERS )
		return TNULL;

	return m_aBlobData[ uiPlayer ].m_pBlobCamera;
}

// $deBlob: FUNCTION 005cdd30
void APlayerManager::StartRespawnCamera( TINT8 a_iPlayer, const TVector4& a_rPosition, const TVector4* a_pRotation, TBOOL a_bHandOver, TFLOAT a_fBlendOutStart, TFLOAT a_fBlendOutEnd )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );

	BlobData&        rData   = m_aBlobData[ a_iPlayer ];
	AScriptedCamera* pCamera = rData.m_pRespawnCamera;
	rData.m_pScriptedCamera  = pCamera;

	TMatrix44 oMatrix = TMatrix44::IDENTITY;

	if ( a_pRotation != TNULL )
	{
		// Facing the blob: its rotation turned around the up axis
		TQuaternion oRotation( a_pRotation->x, a_pRotation->y, a_pRotation->z, a_pRotation->w );
		// NOTE: retail passes TVector4::VEC_NEGY
		oRotation.RotateAroundAxis( TVector3::VEC_NEGY, TMath::PI );
		oMatrix.SetFromQuaternion( oRotation );
	}

	oMatrix.SetTranslation( a_rPosition );
	pCamera->SetTransform( oMatrix );
	pCamera->StopAnimation();
	pCamera->PlayAnimation( "respawn_loop" );
	pCamera->SetBlendOut( a_fBlendOutStart, a_fBlendOutEnd );

	rData.m_iNextCameraMode = 2;
	rData.m_iCameraState    = 0;
	rData.m_auiUnk09[ 0 ]   = 1;
	rData.m_auiUnk09[ 1 ]   = a_bHandOver;

	if ( m_iNumPlayers == 1 )
		AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TTRUE;

	UpdateCameraStates( 0.0f );
}

// The boss camera of player a_iPlayer at a_rcTransform plays "Intro" (a_iAnim 0) or "Death" (1) (ABoss)
// $deBlob: FUNCTION 005cde80
void APlayerManager::StartBossCamera( TINT8 a_iPlayer, const TMatrix44& a_rcTransform, TINT8 a_iAnim )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );

	BlobData& rData         = m_aBlobData[ a_iPlayer ];
	rData.m_pScriptedCamera = rData.m_pBossCamera;
	rData.m_pScriptedCamera->SetTransform( a_rcTransform );
	rData.m_pScriptedCamera->StopAnimation();

	if ( a_iAnim == 0 )
		rData.m_pScriptedCamera->PlayAnimation( "Intro" );
	else if ( a_iAnim == 1 )
		rData.m_pScriptedCamera->PlayAnimation( "Death" );

	rData.m_iNextCameraMode = 2;
	rData.m_iCameraState    = 0;
	rData.m_auiUnk09[ 0 ]   = 0;
	rData.m_auiUnk09[ 1 ]   = 0;

	if ( m_iNumPlayers == 1 )
		AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TTRUE;

	UpdateCameraStates( 0.0f );
}

// $deBlob: FUNCTION 005cdf50
void APlayerManager::ResetCameraState( TINT8 a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );

	BlobData& rData         = m_aBlobData[ a_iPlayer ];
	rData.m_iCameraState    = 0;
	rData.m_iNextCameraMode = 1;
	rData.m_auiUnk09[ 0 ]   = 0;
	rData.m_auiUnk09[ 1 ]   = 0;
}

// $deBlob: FUNCTION 005cf7e0
TBOOL APlayerManager::IsBlobLockedOn( ABaseEntity* a_pBlob )
{
	for ( TINT i = m_iNumPlayers; i != 0; )
	{
		ABaseEntity* pBlob = m_aBlobData[ --i ].m_pBlob;

		if ( pBlob != a_pBlob && TSTATICCAST( ABlobEntity, pBlob )->GetZJumpComponent()->IsLockingOn( a_pBlob ) )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005cfba0
TBOOL APlayerManager::IsBlobInState15( ABaseEntity* a_pBlob )
{
	TINT8 iPlayer = -1;

	if ( a_pBlob != TNULL )
	{
		for ( TINT8 i = MAX_PLAYERS; i != 0; )
		{
			i--;
			if ( a_pBlob == m_aBlobData[ i ].m_pBlob )
			{
				iPlayer = i;
				break;
			}
		}
	}

	if ( iPlayer < 0 )
		return TFALSE;

	return m_aBlobData[ iPlayer ].m_iQueuedAnimState == ABlobAnimationComponent::STATE_LEVEL_START || TSTATICCAST( ABlobEntity, a_pBlob )->GetAnimationComponent()->GetState() == ABlobAnimationComponent::STATE_LEVEL_START;
}

// $deBlob: FUNCTION 005cfc00
TFLOAT APlayerManager::GetFollowCameraUnk14C( ABaseEntity* a_pBlob )
{
	TINT iPlayer = -1;

	if ( a_pBlob != TNULL )
	{
		for ( TINT i = MAX_PLAYERS; i != 0; )
		{
			i--;
			if ( a_pBlob == m_aBlobData[ i ].m_pBlob )
			{
				iPlayer = i;
				break;
			}
		}
	}

	if ( iPlayer >= 0 && m_aBlobData[ iPlayer ].m_eCameraMode == 1 )
		return *TREINTERPRETCAST( const TFLOAT*, &m_aBlobData[ iPlayer ].m_pBlobCamera->m_uiRegionData );

	return 0.0f;
}

// $deBlob: FUNCTION 005d0a80
TBOOL APlayerManager::UpdateRespawnCamera( TINT8 a_iPlayer, TFLOAT a_fDeltaTime )
{
	BlobData& rData = m_aBlobData[ a_iPlayer ];

	if ( TSTATICCAST( ABlobEntity, rData.m_pBlob )->GetAnimationComponent()->GetStateAnim() != TNULL )
		return TFALSE;

	TASSERT( rData.m_pScriptedCamera == rData.m_pRespawnCamera );

	rData.m_pScriptedCamera->StopAnimation();
	rData.m_pScriptedCamera->PlayAnimation( "respawn_end" );

	return TTRUE;
}

// $deBlob: FUNCTION 005cc260
void APlayerManager::UpdateCameraStates( TFLOAT a_fDeltaTime )
{
	AGameStateController* pController      = AGameStateController::GetSingleton();
	AXYZViewportManager*  pViewportManager = ARenderer::GetSingleton()->GetViewportManager();

	for ( TINT i = MAX_PLAYERS - 1; i >= 0; i-- )
	{
		BlobData& rData = m_aBlobData[ i ];

		if ( rData.m_pBlob == TNULL )
			continue;

		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, rData.m_pBlob );

		switch ( rData.m_iCameraState )
		{
			case 0:
			{
				// Fades out the current camera
				if ( rData.m_eCameraMode == 1 )
				{
					rData.m_pBlobCamera->m_iUnk3B0 &= ~1;
					rData.m_pBlobCamera->m_uiControls = 0;
				}

				TBOOL bAttach;

				if ( rData.m_iNextCameraMode == 2 )
				{
					if ( rData.m_auiUnk09[ 0 ] )
					{
						rData.m_iCameraState = 1;
						break;
					}

					bAttach = rData.m_pBossCamera == rData.m_pScriptedCamera;
				}
				else
				{
					bAttach = rData.m_iNextCameraMode == 4;
				}

				if ( bAttach )
					pBlob->SetLevelStartAttached( TTRUE );

				TINT iViewport = pViewportManager->GetCameraViewport( GetPlayerCamera( TUINT8( i ) ) );

				if ( iViewport >= 0 && !rData.m_auiUnk09[ 0 ] )
					pController->FadeToBlack( 1.1f, iViewport );

				rData.m_iCameraState = 1;
				break;
			}
			case 1:
			{
				// Switches the camera once faded out
				if ( rData.m_iNextCameraMode == 2 && rData.m_pScriptedCamera == rData.m_pBossCamera )
					rData.m_pScriptedCamera->Update( a_fDeltaTime );

				TINT  iViewport = pViewportManager->GetCameraViewport( GetPlayerCamera( TUINT8( i ) ) );
				TBOOL bFading   = TFALSE;

				if ( iViewport >= 0 && !rData.m_auiUnk09[ 0 ] )
					bFading = pController->IsFading( iViewport );

				AGameState* pState = pController->GetCurrentGameState();

				if ( rData.m_eCameraMode == 4 && pState != TNULL && pState->IsA( &TGetClass( AInGameState ) ) && TSTATICCAST( AInGameState, pState )->IsPaused() )
					break;

				if ( bFading && !( IsBlobInState15( rData.m_pBlob ) && rData.m_iNextCameraMode == 4 ) )
					break;

				if ( rData.m_eCameraMode == 1 )
				{
					rData.m_pBlobCamera->OnDeactivate();
				}
				else if ( rData.m_eCameraMode == 2 )
				{
					rData.m_pScriptedCamera->StopAnimation();

					if ( rData.m_pBossCamera == rData.m_pScriptedCamera )
						rData.m_pBlob->SetVisible( TTRUE );
				}
				else if ( rData.m_eCameraMode == 4 )
				{
					AMusicManager::GetSingleton()->OnEmptyEvent2();

					// The help of the key frame camera
					if ( rData.m_iHelpTopic != -1 && ( rData.m_iHelpTopic != 0 || rData.m_iHelpString != -1 ) && AOptions::GetSingleton()->IsHelpEnabled() )
					{
						APropTriggerHelp::SetUnk9B720D( TTRUE );
						APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_FullScreen, rData.m_iHelpString, TINT8( rData.m_iHelpTopic ), TTRUE, 4.0f, 0, TNULL, TTRUE );
						rData.m_iCameraState = 4;
						return;
					}
				}

				rData.m_eCameraMode = rData.m_iNextCameraMode;
				SetupViewports();

				if ( rData.m_iNextCameraMode == 1 )
				{
					rData.m_pBlobCamera->Invalidate();
					rData.m_pBlobCamera->Update( 0.0f );
					rData.m_pBlobCamera->OnDeactivate();
				}
				else if ( rData.m_iNextCameraMode == 2 )
				{
					rData.m_pScriptedCamera->Update( 0.0f );

					if ( rData.m_auiUnk09[ 0 ] )
					{
						rData.m_iCameraState = 2;
						break;
					}

					if ( rData.m_pBossCamera == rData.m_pScriptedCamera )
						rData.m_pBlob->SetVisible( TFALSE );
				}
				else if ( rData.m_iNextCameraMode == 4 )
				{
					// The first frame of the key frame camera under the black screen
					rData.m_pKeyFrameCamera->Play();
					rData.m_pKeyFrameCamera->Update( 0.0f );
					rData.m_pKeyFrameCamera->Pause();

					if ( iViewport >= 0 )
						pController->SetFade( 1.0f, iViewport );
				}

				rData.m_iCameraState = 3;
				break;
			}
			case 2:
			{
				// The respawn camera
				if ( !UpdateRespawnCamera( TINT8( i ), a_fDeltaTime ) )
					break;

				if ( !rData.m_auiUnk09[ 1 ] )
				{
					rData.m_eCameraMode = rData.m_iNextCameraMode;
					SetupViewports();
					rData.m_pScriptedCamera->Update( 0.0f );
					rData.m_iCameraState = 3;
				}
				else
				{
					// Hands over to the follow camera
					rData.m_pScriptedCamera->Update( 0.0f );
					rData.m_iNextCameraMode = 1;
					rData.m_eCameraMode     = 1;
					SetupViewports();

					if ( rData.m_pBlobCamera != TNULL && rData.m_pScriptedCamera != TNULL )
						rData.m_pBlobCamera->Reset( TFALSE, TTRUE );

					// The respawn camera blends out into the follow camera
					rData.m_pBlobCamera->SetOverrideCamera( rData.m_pScriptedCamera );
					rData.m_pBlobCamera->Invalidate();
					rData.m_pBlobCamera->Update( 1.0f / 60.0f );
					rData.m_pBlobCamera->OnDeactivate();
					rData.m_iCameraState = 3;
				}

				break;
			}
			case 3:
			{
				// Fades in once the cells around are loaded
				if ( rData.m_eCameraMode == 2 && rData.m_pScriptedCamera == rData.m_pBossCamera )
					rData.m_pScriptedCamera->Update( a_fDeltaTime );

				TXUI* pXUI = TXUI::GetSingleton();

				if ( !ATerrain2Interface::GetInstance()->IsCurrentCellAreaReady() || ATerrain2::GetInstance()->IsStreaming() ||
				     pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked() )
					break;

				TINT iViewport = pViewportManager->GetCameraViewport( GetPlayerCamera( TUINT8( i ) ) );

				if ( iViewport >= 0 && !rData.m_auiUnk09[ 0 ] )
					pController->FadeFromBlack( 1.1f, iViewport );

				TXUIScene* pScene = pXUI->GetActiveScene();

				if ( rData.m_eCameraMode == 1 )
				{
					pBlob->SetLevelStartAttached( TFALSE );

					if ( pBlob->GetBlobState() == ABlobEntity::STATE_LMARK )
						pBlob->ExitLandmark( TTRUE );

					// Closes the letterbox, then shows the help kept by the key frame camera
					if ( m_iNumPlayers == 1 && pScene != TNULL && TStringManager::String16Compare( pScene->GetID(), L"LetterBox", -1 ) == 0 &&
					     !AChallengeManager::GetSingleton()->IsEnding() )
					{
						pXUI->TransitionBack( pScene );

						if ( rData.m_iHelpTopic == -1 && rData.m_iHelpString != -1 )
							APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_Hud, rData.m_iHelpString, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );
					}

					if ( rData.m_auiUnk09[ 0 ] )
					{
						pBlob->FinishRespawn();
						rData.m_auiUnk09[ 0 ] = 0;
						rData.m_auiUnk09[ 1 ] = 0;

						if ( m_iNumPlayers == 1 )
							AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
					}

					rData.m_iHelpString = -1;
					rData.m_iHelpTopic  = -1;
				}
				else if ( rData.m_eCameraMode == 2 )
				{
					if ( m_iNumPlayers == 1 && pScene != TNULL && TStringManager::String16Compare( pScene->GetID(), L"LetterBox", -1 ) == 0 &&
					     !AChallengeManager::GetSingleton()->IsEnding() )
					{
						pXUI->TransitionBack( pScene );

						if ( rData.m_iHelpTopic == -1 && rData.m_iHelpString != -1 )
						{
							APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_Hud, rData.m_iHelpString, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );
							rData.m_iCameraState = 5;
							break;
						}
					}
				}
				else if ( rData.m_eCameraMode == 4 )
				{
					rData.m_pKeyFrameCamera->Play();
					rData.m_auiUnk09[ 0 ] = 0;
					rData.m_auiUnk09[ 1 ] = 0;

					if ( ABlobLetterBoxHud::GetInstance() != TNULL )
					{
						ABlobLetterBoxHud::GetInstance()->SetBarsVisible( TTRUE );
						rData.m_iCameraState = 5;
						break;
					}
				}

				rData.m_iCameraState = 5;
				break;
			}
			case 4:
			{
				// Waits for the help of the key frame camera
				if ( APropTriggerHelp::IsHelpShown() )
					break;

				TINT iViewport = pViewportManager->GetCameraViewport( GetPlayerCamera( TUINT8( i ) ) );
				pController->SetFade( 1.0f, iViewport );

				rData.m_eCameraMode = rData.m_iNextCameraMode;
				SetupViewports();

				if ( rData.m_iNextCameraMode == 1 )
				{
					rData.m_pBlobCamera->Invalidate();
					rData.m_pBlobCamera->Update( 0.0f );
					rData.m_pBlobCamera->OnDeactivate();
				}
				else if ( rData.m_iNextCameraMode == 4 )
				{
					rData.m_pKeyFrameCamera->Play();
					rData.m_pKeyFrameCamera->Update( 0.0f );
					rData.m_pKeyFrameCamera->Pause();

					if ( iViewport >= 0 )
						pController->SetFade( 1.0f, iViewport );
				}

				rData.m_iCameraState = 3;
				break;
			}
			case 5:
			{
				// The camera plays
				if ( rData.m_eCameraMode == 1 )
				{
					rData.m_pBlobCamera->m_iUnk3B0 |= 1;
				}
				else if ( rData.m_eCameraMode == 2 )
				{
					// Back to the follow camera once the animation is over
					if ( !rData.m_pScriptedCamera->IsAnimating() )
					{
						TBOOL bUnk09 = rData.m_auiUnk09[ 0 ];
						ResetCameraState( TINT8( i ) );

						if ( rData.m_pBlobCamera != TNULL && rData.m_pScriptedCamera != TNULL && rData.m_pScriptedCamera == rData.m_pRespawnCamera )
						{
							rData.m_pBlobCamera->SetFromCamera( rData.m_pScriptedCamera );
							rData.m_pBlobCamera->StartRotationBlend();
							rData.m_pBlobCamera->Update( 1.0f / 60.0f );
						}

						if ( bUnk09 )
							rData.m_auiUnk09[ 0 ] = 1;

						if ( m_iNumPlayers == 1 )
							AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;
					}
				}
				else if ( rData.m_eCameraMode == 4 )
				{
					TASSERT( m_aBlobData[ i ].m_pKeyFrameCamera->IsActive() );

					// Back to the follow camera once it's done or skipped
					if ( !rData.m_pKeyFrameCamera->IsPlaying() || rData.m_pKeyFrameCamera->IsSkipped() == TTRUE )
					{
						ResetCameraState( TINT8( i ) );

						if ( m_iNumPlayers == 1 )
							AGameStatus::GetSingleton()->GetLevelData().m_bUnk00C = TFALSE;

						if ( ABlobLetterBoxHud::GetInstance() != TNULL )
							ABlobLetterBoxHud::GetInstance()->SetBarsVisible( TFALSE );
					}
				}

				break;
			}
			default:
				break;
		}
	}
}

// $deBlob: FUNCTION 005cdfc0
void APlayerManager::SetupViewports()
{
	TASSERT( ARenderer::IsSingletonCreated() );

	AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
	pViewportManager->DisableAllViewports();

	TINT iTotalPlayers = 0;

	for ( TINT i = MAX_PLAYERS; i != 0; )
	{
		i--;
		if ( m_aBlobData[ i ].m_pBlob == TNULL )
			continue;

		iTotalPlayers++;

		TASSERT( i >= 0 && i < MAX_PLAYERS );
		pViewportManager->RemoveCamera( m_aBlobData[ i ].m_pBlobCamera );
		TASSERT( i >= 0 && i < MAX_PLAYERS );
		pViewportManager->RemoveCamera( m_aBlobData[ i ].m_pKeyFrameCamera );
		TASSERT( i >= 0 && i < MAX_PLAYERS );
		pViewportManager->RemoveCamera( m_aBlobData[ i ].m_pScriptedCamera );
	}

	ACameraManager::GetSingleton()->SetCurrentCamera( GetPlayerCamera( GetNthPlayer( 0 ) ), TTRUE );

	for ( TINT i = 1; i < iTotalPlayers; i++ )
		ACameraManager::GetSingleton()->SetCameraActive( GetPlayerCamera( GetNthPlayer( i ) ), TTRUE );

	TASSERT( iTotalPlayers <= 4 );

	// The particle budget per viewport
	// NOTE: SetDetail overwrites it with the detail level's (10000), so it has no effect
	TParticleSystem::ACreateData oCreateData = TParticleSystem::s_oCreateData;
	oCreateData.m_iMaxParticles              = 1200 / iTotalPlayers;
	TParticleSystem::GetSingleton()->SetDetail( 2, &oCreateData );

	switch ( iTotalPlayers )
	{
		case 1:
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_FullScreen, GetBlobCamera( GetNthBlob( 0 ) ) );
			return;
		case 2:
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_Left, GetBlobCamera( GetNthBlob( 0 ) ) );
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_Right, GetBlobCamera( GetNthBlob( 1 ) ) );
			return;
		case 3:
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_TopLeft, GetBlobCamera( GetNthBlob( 0 ) ) );
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_TopRight, GetBlobCamera( GetNthBlob( 1 ) ) );
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_BottomLeft, GetBlobCamera( GetNthBlob( 2 ) ) );
			return;
		case 4:
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_TopLeft, GetBlobCamera( GetNthBlob( 0 ) ) );
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_TopRight, GetBlobCamera( GetNthBlob( 1 ) ) );
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_BottomLeft, GetBlobCamera( GetNthBlob( 2 ) ) );
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_BottomRight, GetBlobCamera( GetNthBlob( 3 ) ) );
			return;
		default:
			pViewportManager->EnableViewport( AXYZViewportManager::VIEWPORT_FullScreen, GetPlayerCamera( GetNthPlayer( 0 ) ) );
			return;
	}
}

// The a_iIndex-th player that has a blob (0xFF if none)
// $deBlob: FUNCTION 005cf000
TINT8 APlayerManager::GetClosestPlayer( const Toshi::TVector4& a_rvPosition )
{
	TASSERT( m_iNumPlayers > 0, "GetNumPlayers()>0" );

	if ( m_iNumPlayers == 1 )
		return 0;

	// NOTE: retail also keeps the index in a local for an "iBlobRet>=0" assert; it can't fail
	TINT8  iClosest       = 0;
	TFLOAT fClosestDistSq = 3.402823466e+38f;

	for ( TINT i = MAX_PLAYERS; i != 0; )
	{
		i--;
		ABaseEntity* pBlob = m_aBlobData[ i ].m_pBlob;
		if ( pBlob == TNULL )
			continue;

		TFLOAT fDistSq = Toshi::TVector4::DistanceSq( a_rvPosition, pBlob->GetTranslation() );
		if ( fClosestDistSq > fDistSq )
		{
			fClosestDistSq = fDistSq;
			iClosest       = TINT8( i );
		}
	}

	return iClosest;
}

TUINT8 APlayerManager::GetNthPlayer( TINT a_iIndex )
{
	ABaseEntity* pBlob = GetNthBlob( a_iIndex );

	if ( pBlob != TNULL )
	{
		for ( TINT i = MAX_PLAYERS; i != 0; )
		{
			i--;
			if ( m_aBlobData[ i ].m_pBlob == pBlob )
				return TUINT8( i );
		}
	}

	return 0xFF;
}

// The blob of the a_iIndex-th player that has one
ABaseEntity* APlayerManager::GetNthBlob( TINT a_iIndex )
{
	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		if ( m_aBlobData[ i ].m_pBlob != TNULL && --a_iIndex < 0 )
			return m_aBlobData[ i ].m_pBlob;
	}

	return TNULL;
}

// $deBlob: FUNCTION 005cb850
void APlayerManager::BlobData::ResetLives()
{
	AGameStatus*                   pGameStatus = AGameStatus::GetSingleton();
	ABlobEntity::CommonProperties* pProps      = ABlobEntity::GetCommonProperties();

	// NOTE: the start lives are integers at 0x128 (story), 0x130 (mode 3), 0x134 (sprint), 0x138 (mode 1), 0x13C (mode 2) of
	// BlobEntityCommonProperties, mapped as m_afUnk128
	const TINT* pLives = TREINTERPRETCAST( const TINT*, pProps->m_afUnk128 );

	switch ( pGameStatus->GetLevelMode() )
	{
		case -1:
		case AGameStatus::Mode_Story:
			m_uiLives = ( pGameStatus->GetLevelProperties()->m_sprintLevel != 0 ) ? pLives[ 3 ] : pLives[ 0 ];
			break;
		case AGameStatus::Mode_FreePaint:
			m_uiLives = pLives[ 4 ];
			break;
		case 2:
			m_uiLives = pLives[ 5 ];
			break;
		case 3:
			m_uiLives = pLives[ 2 ];
			break;
		default:
			break;
	}

	if ( m_uiLifeHud != 0 )
		TREINTERPRETCAST( ABlobLifeHud*, m_uiLifeHud )->SetLives( m_uiLives, TTRUE );
}

// The bonus pickups collected in all the sets
// $deBlob: FUNCTION 005cb940
TINT APlayerManager::BlobData::GetNumBonusesCollected() const
{
	TINT iCount = 0;

	for ( TINT i = 0; i < TARRAYSIZE( m_auiBonusCollected ); i++ )
	{
		TUINT8 uiBits = m_auiBonusCollected[ i ];

		if ( uiBits & 0x01 ) iCount++;
		if ( uiBits & 0x02 ) iCount++;
		if ( uiBits & 0x04 ) iCount++;
		if ( uiBits & 0x08 ) iCount++;
		if ( uiBits & 0x10 ) iCount++;
	}

	return iCount;
}

// $deBlob: FUNCTION 005cb980
void APlayerManager::BlobData::LoseLife( TBOOL a_bNoAnim )
{
	m_uiLives = ( TINT( m_uiLives ) > 0 ) ? m_uiLives - 1 : 0;

	if ( m_uiLifeHud != 0 )
		TREINTERPRETCAST( ABlobLifeHud*, m_uiLifeHud )->SetLives( m_uiLives, !a_bNoAnim );
}

// $deBlob: FUNCTION 005cb9b0
void APlayerManager::BlobData::AddLife()
{
	m_uiLives++;

	TINT& riLives = TREINTERPRETCAST( TINT&, m_uiLives );
	Toshi::TMath::Clip( riLives, 0, 10 );

	if ( m_uiLifeHud != 0 )
		TREINTERPRETCAST( ABlobLifeHud*, m_uiLifeHud )->SetLives( m_uiLives, TTRUE );
}

// $deBlob: FUNCTION 005cb9e0
TBOOL APlayerManager::BlobData::IsKeyFrameCameraActive() const
{
	return m_bKeyFrameCameraActive || m_eCameraMode == 4 || m_iNextCameraMode == 4;
}

// $deBlob: FUNCTION 005cb8e0
TBOOL APlayerManager::BlobData::IsBonusSetComplete( TINT a_iSet ) const
{
	TASSERT( a_iSet >= 0 && a_iSet < APickupBonus::PT_TOTAL_BONUS_SETS );
	return m_auiBonusCollected[ a_iSet ] == 4;
}

// $deBlob: FUNCTION 005cf140
void APlayerManager::UpdateCameraRegions( TFLOAT a_fDeltaTime, TINT a_iPlayer, TBOOL a_bUnk4 )
{
	TASSERT( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint );

	BlobData& rData = m_aBlobData[ a_iPlayer ];

	if ( rData.m_eCameraMode != 4 )
	{
		if ( rData.m_iNextCameraMode != 4 )
		{
			ANedCameraFunction&               rNedCameras = ACameraManager::GetSingleton()->GetNedCameras();
			ABlobEntity*                      pBlob       = TSTATICCAST( ABlobEntity, rData.m_pBlob );
			const TVector4&                   rvPos       = pBlob->GetBodyPosition();
			TBOOL                             bPlay       = TTRUE;
			ANedCameraFunction::SSCamTrigger* pTrigger    = TNULL;

			if ( a_bUnk4 )
				pTrigger = rNedCameras.FindSSCamTriggerAt( rvPos, 4 );

			if ( pTrigger == TNULL && ABuildingManager::GetSingleton()->IsBlobOnLandmark() )
				pTrigger = rNedCameras.FindSSCamTriggerAt( rvPos, 2 );

			if ( pTrigger == TNULL )
			{
				TINT iZJumpState = pBlob->GetZJumpComponent()->GetState();

				if ( iZJumpState == ABlobZJumpComponent::STATE_POINT || iZJumpState == ABlobZJumpComponent::STATE_AIMNODE || iZJumpState == ABlobZJumpComponent::STATE_AIM )
					pTrigger = rNedCameras.FindSSCamTriggerAt( rvPos, pBlob->GetZJumpComponent()->IsUnk120() ? 0x20 : 0x10 );
			}

			if ( pTrigger == TNULL && rData.m_bUnk60 )
				pTrigger = rNedCameras.FindSSCamTriggerAt( rvPos, 0x40 );

			if ( pTrigger == TNULL )
				pTrigger = rNedCameras.FindCurrentSSCamTrigger();

			if ( pTrigger == TNULL )
			{
				pTrigger = rNedCameras.FindSSCamTriggerAt( rvPos, 1 );
				bPlay    = TFALSE;
			}

			if ( pTrigger != TNULL )
			{
				if ( pTrigger != rData.m_pSSCamTrigger )
				{
					rData.m_pSSCamTrigger = pTrigger;
					pTrigger->m_iCount++;

					if ( bPlay )
						PlaySSCamera( TUINT8( a_iPlayer ), TINT( pTrigger->m_iSSCam ) );
					else
						QueueSSCamera( TUINT8( a_iPlayer ), TINT( pTrigger->m_iSSCam ) );
				}

				rData.m_fSSCamTriggerTime = 10.0f;
			}
			else if ( rData.m_fSSCamTriggerTime <= 0.0f )
			{
				rData.m_pSSCamTrigger = TNULL;
			}
			else
			{
				rData.m_fSSCamTriggerTime -= a_fDeltaTime;
			}
		}

		if ( rData.m_eCameraMode != 4 && rData.m_iNextCameraMode != 4 && rData.m_iQueuedAnimState != -1 )
			StartQueuedAnimState( a_iPlayer );
	}

	// The queued SS cameras play once the follow camera of player 0 is free
	ACamera* pCam = ACameraManager::GetSingleton()->GetCurrentCamera();
	TXUI*    pXUI = TXUI::GetSingleton();

	if ( TINT( m_uiNumQueuedSSCameras ) > 0 && TINT( m_uiSSCameraDelay ) <= 0 && pCam != TNULL && pCam->GetCameraType() == ACamera::CT_BLOBFOLLOW && a_iPlayer == 0 &&
	     pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() && m_aBlobData[ 0 ].m_eCameraMode == 1 &&
	     ( m_aBlobData[ 0 ].m_iNextCameraMode == 1 || m_aBlobData[ 0 ].m_iNextCameraMode == -1 ) )
	{
		TASSERT( pCam == TSTATICCAST( ACamera, m_aBlobData[ a_iPlayer ].m_pBlobCamera ) );

		ABlobFollowCamera*            pFollowCamera = m_aBlobData[ 0 ].m_pBlobCamera;
		ABlobFollowCamera::POS_HANDLE ePosType      = pFollowCamera->m_iPosType;

		if ( ePosType != ABlobFollowCamera::POS_HANDLE_FOLLOW && ePosType != ABlobFollowCamera::POS_HANDLE_3 )
		{
			TUINT32 uiFlags;

			if ( ePosType == ABlobFollowCamera::POS_HANDLE_NEDFIXED )
			{
				TASSERT( m_aBlobData[ a_iPlayer ].m_pBlobCamera->m_pNedFixedCam );
				uiFlags = m_aBlobData[ 0 ].m_pBlobCamera->m_uiFlags;
			}
			else
			{
				if ( ePosType != ABlobFollowCamera::POS_HANDLE_SPLINE )
					return;

				TASSERT( m_aBlobData[ a_iPlayer ].m_pBlobCamera->m_pCurrSpline );
				uiFlags = m_aBlobData[ 0 ].m_pBlobCamera->m_uiSplineFlags;
			}

			if ( uiFlags != 1 )
				return;
		}

		TINT iBlobState = TSTATICCAST( ABlobEntity, m_aBlobData[ 0 ].m_pBlob )->GetBlobState();

		if ( ( iBlobState == 0 || iBlobState == 6 ) && AChallengeManager::GetSingleton()->IsEnding() != TTRUE &&
		     ( ABlobHud::GetSingleton() == TNULL || ABlobHud::GetSingleton()->GetBannerQueueSize() <= 0 ) )
		{
			TINT* piQueue = TREINTERPRETCAST( TINT*, m_auiQueuedSSCameras );
			PlaySSCamera( 0, piQueue[ 0 ] );
			m_uiNumQueuedSSCameras--;

			for ( TINT i = 0; i < TINT( m_uiNumQueuedSSCameras ); i++ )
				piQueue[ i ] = piQueue[ i + 1 ];
		}
	}
}

// $deBlob: FUNCTION 005ce5e0
APlayerManager::BlobData* APlayerManager::GetBlobData( TUINT8 a_uiPlayer )
{
	TASSERT( a_uiPlayer >= 0 && a_uiPlayer < MAX_PLAYERS );

	if ( a_uiPlayer < MAX_PLAYERS )
		return &m_aBlobData[ a_uiPlayer ];

	return TNULL;
}

// $deBlob: FUNCTION 005cb6f0
TINT APlayerManager::BlobData::GetNextPattern( TINT a_iPattern )
{
	APlayerManager* pPM    = APlayerManager::GetSingleton();
	TINT            iStart = a_iPattern + 1;

	if ( iStart < 0 || iStart >= 16 )
		iStart = 0;

	for ( TINT i = iStart; i < 16; i++ )
	{
		if ( pPM->GetPatternTexture( i ) != TNULL && ( ( 1 << i ) & m_uiPatternMask ) )
			return i;
	}

	for ( TINT i = 0; i < iStart; i++ )
	{
		if ( pPM->GetPatternTexture( i ) != TNULL && ( ( 1 << i ) & m_uiPatternMask ) )
			return i;
	}

	return -1;
}

// The patterns AddSprintPatterns gives
static const TINT s_iMaxSprintPatterns = 10;

// $deBlob: FUNCTION 005cb680
void APlayerManager::BlobData::AddSprintPatterns()
{
	if ( AGameStatus::GetSingleton()->GetLevelData().m_pLevelProps->m_sprintLevel == 0 )
		return;

	TINT  iNumPatterns = 0;
	TUINT uiBit        = 1;

	for ( TINT i = 0; i < 16; i++ )
	{
		if ( APlayerManager::GetSingleton()->GetPatternTexture( i ) != TNULL )
		{
			if ( iNumPatterns == s_iMaxSprintPatterns )
				return;

			m_uiPatternMask |= uiBit;
			iNumPatterns++;

			if ( m_iPattern == -1 )
				m_iPattern = i;
		}

		uiBit = ( uiBit << 1 ) | ( uiBit >> 31 );
	}
}

// $deBlob: FUNCTION 005cc0b0
void APlayerManager::AddSprintPatterns()
{
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
		m_aBlobData[ i ].AddSprintPatterns();
}

// $deBlob: FUNCTION 005cece0
void APlayerManager::RestartPlayers()
{
	m_uiNumQueuedSSCameras = 0;
	m_uiSSCameraDelay      = 0;

	ABlobEntity::ms_bControlsLocked = TFALSE;

	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		BlobData& rData = m_aBlobData[ i ];

		if ( rData.m_pBlob == TNULL )
			continue;

		rData.Reset();
		rData.ResetLives();
		rData.AddSprintPatterns();
		rData.m_pBlob->Reset( TNULL );
		TSTATICCAST( ABlobEntity, rData.m_pBlob )->SetLevelStartAttached( TFALSE );
		rData.m_eCameraMode     = 1;
		rData.m_iNextCameraMode = 1;
		rData.m_iCameraState    = 5;
		rData.m_auiUnk09[ 0 ]   = 0;
		rData.m_auiUnk09[ 1 ]   = 0;
		rData.m_pBlobCamera->OnSuspend();
	}

	TINT iGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

	if ( iGameType < 0 )
	{
		AssignPlayers( TTRUE );
	}
	else if ( iGameType < 3 )
	{
		AssignMultiplayerPlayers();
	}
	else if ( iGameType == 3 )
	{
		AssignPlayers( TFALSE );
	}

	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		BlobData& rData = m_aBlobData[ i ];

		if ( rData.m_pBlob == TNULL )
			continue;

		rData.m_pBlobCamera->OnSuspend();

		if ( rData.m_iNextCameraMode != 4 && rData.m_eCameraMode == 4 )
		{
			rData.m_eCameraMode     = 1;
			rData.m_iNextCameraMode = 1;
			rData.m_iCameraState    = 5;
		}
	}

	SetupViewports();

	SetupColours();

	RemoveVistaScoreTexts();

	if ( !AGameStatus::GetSingleton()->IsLevelComplete() )
		m_pVistaHud = TNULL;
}

// $deBlob: FUNCTION 005d0710
void APlayerManager::SetupColours()
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	TINT         iGameType   = pGameStatus->GetMultiPlayerGameType();
	TINT         iNumPlayers = m_iNumPlayers;

	if ( iGameType < 0 )
	{
		TASSERT( m_iNumPlayers == 1 );

		// A sprint level starts in its colour (its LevelProperties 0x54)
		BlobData& rData = m_aBlobData[ 0 ];
		rData.m_iColour = 0;

		ALevelInformation::LevelProperties* pLevelProps = &ALevelInformation::GetSingleton()->GetLevelHeader()->m_pLevelProps[ pGameStatus->GetCurrentLevel() ];

		if ( pLevelProps->m_sprintLevel != 0 )
		{
			TUINT32 uiColour = 0;

			switch ( pLevelProps->m_auiMedalLimits[ 8 ] )
			{
				case 0: uiColour = 0; break;
				case 1: uiColour = 1; break;
				case 2: uiColour = 2; break;
				case 3: uiColour = 4; break;
				case 4: uiColour = 16; break;
				case 5: uiColour = 6; break;
				case 6: uiColour = 5; break;
				case 7: uiColour = 3; break;
				case 8: uiColour = 7; break;
				default: break;
			}

			rData.m_iColour = uiColour;
		}

		ABlobColourComponent* pColour = TSTATICCAST( ABlobEntity, rData.m_pBlob )->GetColourComponent();
		pColour->SetStartColour( rData.m_iColour );
		pColour->SetColour( rData.m_iColour );
		pColour->SetUnk20( rData.m_iColour );
		return;
	}

	for ( TINT i = MAX_PLAYERS; i != 0; )
	{
		i--;
		BlobData& rData = m_aBlobData[ i ];

		if ( rData.m_pBlob != TNULL )
		{
			ABlobColourComponent* pColour = TSTATICCAST( ABlobEntity, rData.m_pBlob )->GetColourComponent();
			pColour->SetStartColour( rData.m_iColour );
			pColour->SetUnk20( rData.m_iColour );
		}
	}

	switch ( iGameType )
	{
		case 0:
		case 2:
			for ( TINT i = MAX_PLAYERS; i != 0; )
			{
				i--;
				if ( m_aBlobData[ i ].m_pBlob != TNULL )
				{
					ABlobColourComponent* pColour = TSTATICCAST( ABlobEntity, m_aBlobData[ i ].m_pBlob )->GetColourComponent();
					pColour->SetColour( pColour->GetStartColour() );
				}
			}
			break;
		case 1:
			for ( TINT i = MAX_PLAYERS; i != 0; )
			{
				i--;
				if ( m_aBlobData[ i ].m_pBlob != TNULL )
				{
					TSTATICCAST( ABlobEntity, m_aBlobData[ i ].m_pBlob )->GetColourComponent()->SetColour( 0 );
					TSTATICCAST( ABlobEntity, m_aBlobData[ i ].m_pBlob )->SetPaintPoints( 20 );
				}
			}
			break;
		case 3:
			for ( TINT i = MAX_PLAYERS; i != 0; )
			{
				i--;
				if ( m_aBlobData[ i ].m_pBlob != TNULL )
				{
					ABlobColourComponent* pColour = TSTATICCAST( ABlobEntity, m_aBlobData[ i ].m_pBlob )->GetColourComponent();
					pColour->SetColour( pColour->GetStartColour() );
				}
			}

			if ( iNumPlayers == 4 )
			{
				// The two teams' colours (players 0 and 1, 2 and 3)
				TUINT32 uiTeamColour1;
				TUINT32 uiTeamColour2;
				GetTeamColours( uiTeamColour1, uiTeamColour2 );

				for ( TINT i = 0; i < MAX_PLAYERS; i++ )
				{
					TUINT32               uiColour = ( i < 2 ) ? uiTeamColour1 : uiTeamColour2;
					ABlobColourComponent* pColour  = TSTATICCAST( ABlobEntity, m_aBlobData[ i ].m_pBlob )->GetColourComponent();
					pColour->SetStartColour( uiColour );
					pColour->SetUnk20( uiColour );
					pColour->SetColour( uiColour );
				}
			}
			break;
		default:
			break;
	}
}

// $deBlob: FUNCTION 005cee10
TINT APlayerManager::GetNumPlayers() const
{
	return m_iNumPlayers;
}

// $deBlob: FUNCTION 005cee40
void APlayerManager::LoadPatternTextures()
{
	ARegionManager*  pRegionManager = ARegionManager::GetSingleton();
	TINT             iRegion        = ATerrain2Interface::GetInstance()->GetCurrentRegion();
	TTextureManager* pTM            = TTextureManager::GetSingleton();

	for ( TINT i = 0; i < pRegionManager->GetNumPatternTextures( iRegion ); i++ )
	{
		ARegionManager::PatternTexture* patternTex = pRegionManager->GetPatternTexture( iRegion, i );
		patternTex->m_pTex                         = pTM->FindTexture( patternTex->m_szTexture );
		TASSERT( !( patternTex->m_pTex == pTM->GetInvalidTexture() ) );

		// NOTE: retail toggles it on every load (the region's data stays loaded between levels)
		patternTex->m_eBlendType = ( patternTex->m_eBlendType == 1 ) ? 2 : 1;
	}

	m_oBarcodePattern.m_eBlendType = 1;
	m_oBarcodePattern.m_pTex       = pTM->FindTexture( "pattern_barcode.tga" );
}

// $deBlob: FUNCTION 005cef60
void APlayerManager::UnloadPatternTextures()
{
	ARegionManager* pRegionManager = ARegionManager::GetSingleton();
	TINT            iRegion        = ATerrain2Interface::GetInstance()->GetCurrentRegion();

	for ( TINT i = 0; i < pRegionManager->GetNumPatternTextures( iRegion ); i++ )
		pRegionManager->GetPatternTexture( iRegion, i )->m_pTex = TNULL;

	m_oBarcodePattern.m_pTex = TNULL;
}

// $deBlob: FUNCTION 005cf840
ARegionManager::PatternTexture* APlayerManager::GetPatternTexture( TINT a_iIndex )
{
	TASSERT( a_iIndex < 17 );

	if ( a_iIndex == 16 )
		return &m_oBarcodePattern;

	ARegionManager* pRegionManager = ARegionManager::GetSingleton();
	TINT            iRegion        = ATerrain2Interface::GetInstance()->GetCurrentRegion();

	for ( TINT i = 0; i < pRegionManager->GetNumPatternTextures( iRegion ); i++ )
	{
		ARegionManager::PatternTexture* pPattern = pRegionManager->GetPatternTexture( iRegion, i );

		if ( pPattern->m_iIndex == a_iIndex )
			return pPattern;
	}

	return TNULL;
}

// $deBlob: FUNCTION 005cf920
TINT APlayerManager::CyclePattern( TINT a_iPlayer )
{
	TINT iPattern                       = m_aBlobData[ a_iPlayer ].m_iPattern;
	m_aBlobData[ a_iPlayer ].m_iPattern = m_aBlobData[ a_iPlayer ].GetNextPattern( iPattern );

	return ( TUINT32( iPattern ) <= 15 ) ? iPattern : -1;
}

// $deBlob: FUNCTION 005cf750
TBOOL APlayerManager::IsNoBlobPainted() const
{
	for ( TINT i = MAX_PLAYERS; i != 0; )
	{
		i--;
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, m_aBlobData[ i ].m_pBlob );

		if ( pBlob != TNULL )
		{
			TUINT32 uiColour = pBlob->GetPaintColour();

			if ( uiColour != 0 && ( uiColour & 0x10 ) == 0 )
				return TFALSE;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005cf790
ABaseEntity* APlayerManager::GetPaintedBlob() const
{
	for ( TINT i = MAX_PLAYERS; i != 0; )
	{
		i--;
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, m_aBlobData[ i ].m_pBlob );

		if ( pBlob != TNULL )
		{
			TUINT32 uiColour = pBlob->GetPaintColour();

			if ( uiColour != 0 && ( uiColour & 0x10 ) == 0 )
				return pBlob;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 005cba00
TBOOL APlayerManager::BlobData::IsSSCameraPlaying( TINT a_iCamera ) const
{
	return ( m_bKeyFrameCameraActive || m_eCameraMode == 4 || m_iNextCameraMode == 4 ) && a_iCamera == m_pKeyFrameCamera->GetSSCamera()->m_iID;
}

// $deBlob: FUNCTION 005cdb90
void APlayerManager::QueueSSCamera( TUINT8 a_uiPlayer, TINT a_iCamera )
{
	TASSERT( 0 == a_uiPlayer );
	TASSERT( TINT( m_uiNumQueuedSSCameras ) < 10 );

	if ( TINT( m_uiNumQueuedSSCameras ) < 10 )
	{
		TREINTERPRETCAST( TINT*, m_auiQueuedSSCameras )
		[m_uiNumQueuedSSCameras] = a_iCamera;
		m_uiNumQueuedSSCameras++;
	}
}

// $deBlob: FUNCTION 005cdc30
TBOOL APlayerManager::IsSSCameraQueued( TUINT8 a_uiPlayer, TINT a_iCamera ) const
{
	TASSERT( 0 == a_uiPlayer );
	TASSERT( TINT( m_uiNumQueuedSSCameras ) < 10 );

	for ( TINT i = TINT( m_uiNumQueuedSSCameras ); i != 0; i-- )
	{
		if ( a_iCamera == TREINTERPRETCAST( const TINT*, m_auiQueuedSSCameras )[ i - 1 ] )
			return TTRUE;
	}

	return TFALSE;
}

APlayerManager::VistaScore APlayerManager::m_soTempVistaScore;

// $deBlob: FUNCTION 005cf700
APlayerManager::VistaScore* APlayerManager::GetVistaScore( TINT a_iPlayer )
{
	TASSERT( a_iPlayer == m_soTempVistaScore.m_iPlayer );
	return &m_soTempVistaScore;
}

// $deBlob: FUNCTION 005cbe60
void APlayerManager::CreateVistaFrustum( TPlane* a_pOutPlanes, const TMatrix44& a_rMatrix, TFLOAT a_fFOV, TFLOAT a_fNear, TFLOAT a_fFar )
{
	TASSERT( a_rMatrix.IsOrthogonal() );

	TPlane aPlanes[ 6 ];

	TFLOAT fHalfFOVNeg = a_fFOV * -0.5f;
	aPlanes[ 0 ]       = TPlane( -1.0f, 0.0f, 0.0f, 0.0f );
	aPlanes[ 0 ].AsNormal().RotateY( fHalfFOVNeg );

	TFLOAT fHalfFOV = a_fFOV * 0.5f;
	aPlanes[ 1 ]    = TPlane( 1.0f, 0.0f, 0.0f, 0.0f );
	aPlanes[ 1 ].AsNormal().RotateY( fHalfFOV );

	aPlanes[ 2 ] = TPlane( 0.0f, -1.0f, 0.0f, 0.0f );
	aPlanes[ 2 ].AsNormal().RotateX( fHalfFOV );

	aPlanes[ 3 ] = TPlane( 0.0f, 1.0f, 0.0f, 0.0f );
	aPlanes[ 3 ].AsNormal().RotateX( fHalfFOVNeg );

	TASSERT( aPlanes[ 0 ].AsNormal().z <= 0.0f );
	TASSERT( aPlanes[ 1 ].AsNormal().z <= 0.0f );
	TASSERT( aPlanes[ 2 ].AsNormal().z <= 0.0f );
	TASSERT( aPlanes[ 3 ].AsNormal().z <= 0.0f );

	aPlanes[ 4 ] = TPlane( 0.0f, 0.0f, -1.0f, -a_fNear );
	aPlanes[ 5 ] = TPlane( 0.0f, 0.0f, 1.0f, a_fFar );

	for ( TINT i = 0; i < 6; i++ )
		TMatrix44::TransformPlaneOrthogonal( a_pOutPlanes[ i ], a_rMatrix, aPlanes[ i ] );
}

// Adds a painted tree/building of a_uiColour (ABlobColourComponent::COLOUR) to the colour counts of a_rScore
static void AddVistaColour( APlayerManager::VistaScore& a_rScore, TUINT a_uiColour )
{
	switch ( a_uiColour )
	{
		case 0: a_rScore.m_aiColourCounts[ 0 ]++; break;
		case 1: a_rScore.m_aiColourCounts[ 1 ]++; break;
		case 2: a_rScore.m_aiColourCounts[ 2 ]++; break;
		case 3: a_rScore.m_aiColourCounts[ 7 ]++; break;
		case 4: a_rScore.m_aiColourCounts[ 3 ]++; break;
		case 5: a_rScore.m_aiColourCounts[ 6 ]++; break;
		case 6: a_rScore.m_aiColourCounts[ 5 ]++; break;
		case 7: a_rScore.m_aiColourCounts[ 8 ]++; break;
		case ABlobColourComponent::COLOUR_BLACK: a_rScore.m_aiColourCounts[ 4 ]++; break;
	}
}

// $deBlob: FUNCTION 005cbc10
void APlayerManager::CountVistaInstance( void* a_pInstance, void* a_pVistaScore )
{
	TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, a_pInstance );
	VistaScore&                 rScore    = *TSTATICCAST( VistaScore, a_pVistaScore );

	TUINT uiType = pInstance->m_uiFlags & 7;

	if ( uiType == TerrainTypes::InstanceType_Tree )
	{
		ATreeInstance* pTree = TSTATICCAST( ATreeInstance, pInstance->m_pModelInstance2 );
		rScore.m_iNumTrees++;

		if ( pTree->m_bTransformed )
		{
			rScore.m_iNumTreesPainted++;
			AddVistaColour( rScore, pTree->m_iColour );
		}
	}
	else if ( uiType == TerrainTypes::InstanceType_Landmark )
	{
		ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, pInstance->m_pModelInstance2 );

		if ( !pLandmark->IsABillBoard() )
		{
			rScore.m_iNumLandmarks++;

			if ( pLandmark->m_uiFlags & ALandmarkInstance::Flags_Completed )
				rScore.m_iNumLandmarksPainted++;
		}
		else
		{
			rScore.m_iNumBillBoards++;

			if ( pLandmark->m_uiFlags & ALandmarkInstance::Flags_Completed )
				rScore.m_iNumBillBoardsPainted++;
		}
	}
	else if ( uiType == TerrainTypes::InstanceType_Grass )
	{
		AGrassInstance* pGrass = TSTATICCAST( AGrassInstance, pInstance->m_pModelInstance2 );
		rScore.m_iNumPublicSpaces++;

		if ( pGrass->m_bPainted )
			rScore.m_iNumPublicSpacesPainted++;
	}
	else if ( uiType == TerrainTypes::InstanceType_Building )
	{
		ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 );
		rScore.m_iNumBuildings++;

		TUINT uiColour = pBuilding->m_uiCurrentColour;

		// NOTE: ink (COLOUR_BLACK) and grey buildings aren't painted, but every building counts into the colours
		if ( uiColour != ABlobColourComponent::COLOUR_BLACK && uiColour != ABlobColourComponent::COLOUR_WHITE )
		{
			rScore.m_iNumBuildingsPainted++;

			TINT8 iTexIndex = TINT8( pBuilding->m_uiUnkC6 );

			if ( iTexIndex != -1 )
			{
				TASSERT( iTexIndex >= 0 && iTexIndex < TINT( TARRAYSIZE( rScore.m_aiPatternCounts ) ) );
				rScore.m_aiPatternCounts[ iTexIndex ]++;
			}
		}

		AddVistaColour( rScore, uiColour );
	}
}

// $deBlob: FUNCTION 005cfcf0
void APlayerManager::CalculateVistaScore( TINT a_iPlayer, const ANedCameraFunction::FixedCam* a_pCamera )
{
	VistaScore& rScore = m_soTempVistaScore;

	rScore.m_fLPBPercent             = 0.0f;
	rScore.m_fBTPercent              = 0.0f;
	rScore.m_fColourSpread           = 0.0f;
	rScore.m_fScore                  = 0.0f;
	rScore.m_iMessage                = -1;
	rScore.m_iRank                   = 0x53;
	rScore.m_iMaxColour              = 0;
	rScore.m_iMinColour              = 0;
	rScore.m_iPlayer                 = a_iPlayer;
	rScore.m_iNumColoursUsed         = 0;
	rScore.m_iTotalPatternsUsed      = 0;
	rScore.m_iNumLandmarks           = 0;
	rScore.m_iNumLandmarksPainted    = 0;
	rScore.m_iNumPublicSpaces        = 0;
	rScore.m_iNumPublicSpacesPainted = 0;
	rScore.m_iNumBillBoards          = 0;
	rScore.m_iNumBillBoardsPainted   = 0;
	rScore.m_iNumTrees               = 0;
	rScore.m_iNumTreesPainted        = 0;
	rScore.m_iNumBuildings           = 0;
	rScore.m_iNumBuildingsPainted    = 0;

	for ( TINT i = TARRAYSIZE( rScore.m_aiColourCounts ) - 1; i >= 0; i-- )
		rScore.m_aiColourCounts[ i ] = 0;

	for ( TINT i = TARRAYSIZE( rScore.m_aiPatternCounts ) - 1; i >= 0; i-- )
		rScore.m_aiPatternCounts[ i ] = 0;

	ATerrain2Interface* pInterface = ATerrain2Interface::GetInstance();
	TBoxTree*           pTrees     = pInterface->GetPerCellInstancesVisTrees();

	if ( a_pCamera->m_iNumInstances > 0 )
	{
		// The instances the camera lists
		for ( TINT i = a_pCamera->m_iNumInstances; i != 0; )
		{
			i--;
			CountVistaInstance( a_pCamera->m_ppInstances[ i ], &rScore );
		}
	}
	else
	{
		// The instances in the frustum of the camera
		TMatrix44 oMatrix;
		oMatrix.SetFromQuaternion( a_pCamera->m_qOrientation );
		TASSERT( a_pCamera->m_vPosition.w == 1.0f );
		oMatrix.SetTranslation( a_pCamera->m_vPosition );

		TPlane aPlanes[ 6 ];
		CreateVistaFrustum( aPlanes, oMatrix, a_pCamera->m_fFOV * 0.8f, 5.0f, 300.0f );

		for ( TINT i = ATerrain2::GetInstance()->GetNumCells( pInterface->GetCurrentRegion() ); i != 0; )
		{
			i--;
			TBoxTree& rTree = pTrees[ i ];

			const TPlane*              pOldPlanes   = rTree.GetPlanes();
			TBoxTree::t_RenderFunction fnOldRender  = rTree.GetRenderFunction();
			void*                      pOldUserData = rTree.GetUserData();

			rTree.SetPlanes( aPlanes );
			rTree.SetRenderFunction( CountVistaInstance );
			rTree.SetUserData( &rScore );
			rTree.Render();
			rTree.SetRenderFunction( fnOldRender );
			rTree.SetPlanes( pOldPlanes );
			rTree.SetUserData( pOldUserData );
		}
	}

	// Landmarks, public spaces and billboards
	TINT iLPBTotalPainted = rScore.m_iNumLandmarksPainted + rScore.m_iNumPublicSpacesPainted + rScore.m_iNumBillBoardsPainted;
	TINT iLPBTotal        = rScore.m_iNumLandmarks + rScore.m_iNumPublicSpaces + rScore.m_iNumBillBoards;

	if ( iLPBTotal != 0 )
	{
		TASSERT( iLPBTotalPainted <= iLPBTotal );
		rScore.m_fLPBPercent = ( TFLOAT( iLPBTotalPainted ) * 100.0f ) / TFLOAT( iLPBTotal );
	}
	else
	{
		rScore.m_fLPBPercent = 100.0f;
	}

	// Buildings and trees
	TINT iBTTotalPainted = rScore.m_iNumTreesPainted + rScore.m_iNumBuildingsPainted;
	TINT iBTTotal        = rScore.m_iNumTrees + rScore.m_iNumBuildings;

	if ( iBTTotal != 0 )
	{
		TASSERT( iBTTotalPainted <= iBTTotal );
		rScore.m_fBTPercent = ( TFLOAT( iBTTotalPainted ) * 100.0f ) / TFLOAT( iBTTotal );
	}
	else
	{
		rScore.m_fBTPercent = 100.0f;
	}

	// The colours (without 0, white, and 4, ink)
	TFLOAT fTotal = 0.0f;
	TINT   iMax   = 0;
	TINT   iMin   = 9999;

	for ( TINT i = 8; i != 0; i-- )
	{
		if ( i == 4 )
			continue;

		if ( rScore.m_aiColourCounts[ i ] > iMax )
		{
			iMax                = rScore.m_aiColourCounts[ i ];
			rScore.m_iMaxColour = i;
		}

		if ( rScore.m_aiColourCounts[ i ] < iMin )
		{
			iMin                = rScore.m_aiColourCounts[ i ];
			rScore.m_iMinColour = i;
		}

		fTotal += TFLOAT( rScore.m_aiColourCounts[ i ] );

		if ( rScore.m_aiColourCounts[ i ] > 0 )
			rScore.m_iNumColoursUsed++;
	}

	TFLOAT fAverage = fTotal / 7.0f;
	TMath::Clip( fAverage, 1.0f, fTotal );

	for ( TINT i = 8; i != 0; i-- )
	{
		if ( i == 4 )
			continue;

		TFLOAT fShare = TFLOAT( rScore.m_aiColourCounts[ i ] ) / fAverage;
		TMath::Clip( fShare, 0.0f, 1.0f );
		rScore.m_fColourSpread += fShare;
	}

	rScore.m_fColourSpread *= 14.285714f;
	TMath::Clip( rScore.m_fColourSpread, 0.0f, 100.0f );

	// The patterns (without the barcode)
	rScore.m_iTotalPatternsUsed = 0;

	if ( ARegionManager::GetSingleton()->GetNumPatternTextures( pInterface->GetCurrentRegion() ) > 0 )
	{
		for ( TINT i = 16; i != 0; )
		{
			i--;

			if ( rScore.m_aiPatternCounts[ i ] > 0 )
				rScore.m_iTotalPatternsUsed++;
		}
	}

	// The rank
	rScore.m_iRank  = 0x53;
	rScore.m_fScore = 0.0f;

	if ( rScore.m_fLPBPercent >= 100.0f && rScore.m_fBTPercent >= 85.0f && rScore.m_fColourSpread >= 80.0f && rScore.m_iNumColoursUsed >= 7 && rScore.m_iTotalPatternsUsed >= 2 )
	{
		rScore.m_iRank    = 0x54;
		rScore.m_fScore   = 25000.0f;
		rScore.m_iMessage = -1;
		return;
	}

	if ( rScore.m_fLPBPercent >= 100.0f && rScore.m_fBTPercent >= 70.0f && rScore.m_fColourSpread >= 60.0f && rScore.m_iNumColoursUsed >= 5 && rScore.m_iTotalPatternsUsed >= 1 )
	{
		rScore.m_iRank  = 0x55;
		rScore.m_fScore = 10000.0f;
	}
	else if ( rScore.m_fLPBPercent >= 0.0f && rScore.m_fBTPercent >= 50.0f && rScore.m_fColourSpread >= 40.0f && rScore.m_iNumColoursUsed >= 3 && rScore.m_iTotalPatternsUsed >= 1 )
	{
		rScore.m_iRank  = 0x56;
		rScore.m_fScore = 5000.0f;
	}
	else if ( rScore.m_fLPBPercent >= 0.0f && rScore.m_fBTPercent >= 10.0f && rScore.m_fColourSpread >= 0.0f && rScore.m_iNumColoursUsed >= 1 && rScore.m_iTotalPatternsUsed >= 0 )
	{
		rScore.m_iRank  = 0x57;
		rScore.m_fScore = 1000.0f;
	}

	// The thresholds of the next rank
	TFLOAT fLPB;
	TFLOAT fBT;
	TINT   iColours;
	TFLOAT fSpread;
	TINT   iPattern;

	if ( rScore.m_iRank == 0x55 )
	{
		fLPB     = 100.0f;
		fBT      = 85.0f;
		iColours = 7;
		fSpread  = 80.0f;
		iPattern = 2;
	}
	else if ( rScore.m_iRank == 0x56 )
	{
		fLPB     = 100.0f;
		fBT      = 70.0f;
		iColours = 5;
		fSpread  = 60.0f;
		iPattern = 1;
	}
	else if ( rScore.m_iRank == 0x57 )
	{
		fLPB     = 0.0f;
		fBT      = 50.0f;
		iColours = 3;
		fSpread  = 40.0f;
		iPattern = 1;
	}
	else
	{
		fLPB     = 0.0f;
		fBT      = 10.0f;
		iColours = 1;
		fSpread  = 0.0f;
		iPattern = 0;
	}

	// The message: what's missing for the next rank
	rScore.m_iMessage = -1;

	if ( rScore.m_fLPBPercent < fLPB )
	{
		TFLOAT fLandmarks    = 100.0f;
		TFLOAT fPublicSpaces = 100.0f;
		TFLOAT fBillBoards   = 100.0f;

		if ( rScore.m_iNumLandmarks > 0 )
			fLandmarks = ( TFLOAT( rScore.m_iNumLandmarksPainted ) / TFLOAT( rScore.m_iNumLandmarks ) ) * 100.0f;

		if ( rScore.m_iNumPublicSpaces > 0 )
			fPublicSpaces = ( TFLOAT( rScore.m_iNumPublicSpacesPainted ) / TFLOAT( rScore.m_iNumPublicSpaces ) ) * 100.0f;

		if ( rScore.m_iNumBillBoards > 0 )
			fBillBoards = ( TFLOAT( rScore.m_iNumBillBoardsPainted ) / TFLOAT( rScore.m_iNumBillBoards ) ) * 100.0f;

		rScore.m_iMessage = 0x58;

		if ( fPublicSpaces <= fLandmarks && fPublicSpaces <= fBillBoards )
			rScore.m_iMessage = 0x5A;

		if ( fBillBoards <= fLandmarks && fBillBoards <= fPublicSpaces )
			rScore.m_iMessage = 0x59;
	}
	else if ( rScore.m_fBTPercent < fBT )
	{
		TFLOAT fBuildings = 100.0f;
		TFLOAT fTrees     = 100.0f;

		if ( rScore.m_iNumBuildings > 0 )
			fBuildings = ( TFLOAT( rScore.m_iNumBuildingsPainted ) / TFLOAT( rScore.m_iNumBuildings ) ) * 100.0f;

		if ( rScore.m_iNumTrees > 0 )
			fTrees = ( TFLOAT( rScore.m_iNumTreesPainted ) / TFLOAT( rScore.m_iNumTrees ) ) * 100.0f;

		rScore.m_iMessage = ( fTrees < fBuildings ) ? 0x5C : 0x5B;
	}
	else if ( rScore.m_iNumColoursUsed < iColours )
	{
		rScore.m_iMessage = 0x5D;

		if ( rScore.m_iMinColour == 0 )
			rScore.m_iMinColour = 6;
	}
	else if ( rScore.m_fColourSpread < fSpread )
	{
		rScore.m_iMessage = 0x5E;

		if ( rScore.m_iMinColour == 0 )
			rScore.m_iMinColour = 6;
	}
	else
	{
		TASSERT( rScore.m_iTotalPatternsUsed < iPattern );
		rScore.m_iMessage = 0x5F;
	}
}

// The locale string of a colour of VistaScore::m_aiColourCounts, -1: none
static TINT GetVistaColourString( TINT a_iColour )
{
	switch ( a_iColour )
	{
		case 0: return 0x2E;
		case 1: return 0x30;
		case 2: return 0x32;
		case 3: return 0x31;
		case 4: return 0x2F;
		case 5: return 0x34;
		case 6: return 0x33;
		case 7: return 0x35;
		case 8: return 0x36;
	}

	return -1;
}

// $deBlob: FUNCTION 005d0c10
void APlayerManager::CreateVistaScoreTexts()
{
	RemoveVistaScoreTexts();

	TFLOAT fWidth;
	TFLOAT fHeight;
	T2GUI::GetSingleton()->GetRootElement()->GetDimensions( fWidth, fHeight );

	m_oRectangle.Create( fWidth, fHeight );
	m_oRectangle.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oRectangle.SetAlpha( 0.0f );
	m_oRectangle.GetTransform().m_Pos.x = fWidth * 0.3f;
	m_oRectangle.GetTransform().m_Pos.y = fHeight * 0.15f;

	for ( TINT i = 0; i < NUM_VISTA_SCORE_TEXTS; i++ )
	{
		VistaScoreText& rText    = m_aVistaScoreTexts[ i ];
		T2GUITextBox&   rTextBox = rText.m_oTextBox;
		TFLOAT          fLineY   = AFontManager::sm_pMsgFont->GetHeight() * 1.1f * TFLOAT( i );

		rTextBox.GetTransform().SetIdentity();
		rTextBox.Create( AFontManager::sm_pMsgFont, fWidth );
		rTextBox.SetText( rText.m_wszText );
		rTextBox.SetMaxChars( -1 );
		rTextBox.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
		rTextBox.GetTransform().m_Pos.x = 0.0f;
		rTextBox.GetTransform().m_Pos.y = fLineY;
		rTextBox.SetColour( 0xFFFFFFFF );
		rTextBox.SetJustify( T2GUIFont::JUSTIFY( 0 ) );
		m_oRectangle.AddChildTail( &rTextBox );
	}

	T2GUI::GetSingleton()->GetRootElement()->AddChildTail( &m_oRectangle );

	VistaScore& rScore = m_soTempVistaScore;

	TStringManager::String16Format( m_aVistaScoreTexts[ 0 ].m_wszText, 0x100, L"> LandMark      [%d / %d] \t %.2f %%", rScore.m_iNumLandmarksPainted, rScore.m_iNumLandmarks, rScore.m_fLPBPercent );
	TStringManager::String16Format( m_aVistaScoreTexts[ 1 ].m_wszText, 0x100, L"> Public Spaces [%d / %d] \t %.2f %%", rScore.m_iNumPublicSpacesPainted, rScore.m_iNumPublicSpaces, rScore.m_fLPBPercent );
	TStringManager::String16Format( m_aVistaScoreTexts[ 2 ].m_wszText, 0x100, L"> BillBoard     [%d / %d] \t %.2f %%", rScore.m_iNumBillBoardsPainted, rScore.m_iNumBillBoards, rScore.m_fLPBPercent );
	TStringManager::String16Format( m_aVistaScoreTexts[ 3 ].m_wszText, 0x100, L"> Buildings     [%d / %d] \t %.2f %%", rScore.m_iNumBuildingsPainted, rScore.m_iNumBuildings, rScore.m_fBTPercent );
	TStringManager::String16Format( m_aVistaScoreTexts[ 4 ].m_wszText, 0x100, L"> Trees         [%d / %d] \t %.2f %%", rScore.m_iNumTreesPainted, rScore.m_iNumTrees, rScore.m_fBTPercent );
	TStringManager::String16Format( m_aVistaScoreTexts[ 5 ].m_wszText, 0x100, L"> Patterns Used [%d ] ", rScore.m_iTotalPatternsUsed );
	TStringManager::String16Format( m_aVistaScoreTexts[ 6 ].m_wszText, 0x100, L"> Colors Used   [%d ]", rScore.m_iNumColoursUsed );
	TStringManager::String16Format( m_aVistaScoreTexts[ 7 ].m_wszText, 0x100, L"> Colors Spread  \t \t %.2f %%", rScore.m_fColourSpread );
	TStringManager::String16Format( m_aVistaScoreTexts[ 8 ].m_wszText, 0x100, L"> OverAll Score  \t \t %.2f %%", rScore.m_fScore );
	TStringManager::String16Format( m_aVistaScoreTexts[ 9 ].m_wszText, 0x100, L"> Rank\t [ %ls ] ", T2Locale::GetSingleton()->GetString( rScore.m_iRank ) );

	TWCHAR* wszMessage = m_aVistaScoreTexts[ 10 ].m_wszText;

	if ( rScore.m_iMessage == -1 )
	{
		TStringManager::String16Format( wszMessage, 0x100, L"> Message [ No Message] " );
	}
	else
	{
		TINT iMinColorLocale = GetVistaColourString( rScore.m_iMinColour );
		TINT iMAxColorLocale = GetVistaColourString( rScore.m_iMaxColour );

		if ( rScore.m_iMessage == 0x5D )
		{
			TASSERT( iMinColorLocale != -1 );
			const TWCHAR* wszMinColour = T2Locale::GetSingleton()->GetString( iMinColorLocale );
			const TWCHAR* wszFormat    = T2Locale::GetSingleton()->GetString( rScore.m_iMessage );
			TStringManager::String16Format( wszMessage, 0x100, wszFormat, wszMinColour );
		}
		else if ( rScore.m_iMessage == 0x5E )
		{
			TASSERT( iMinColorLocale != -1 && iMAxColorLocale != -1 );
			const TWCHAR* wszMinColour = T2Locale::GetSingleton()->GetString( iMinColorLocale );
			const TWCHAR* wszMaxColour = T2Locale::GetSingleton()->GetString( iMAxColorLocale );
			const TWCHAR* wszFormat    = T2Locale::GetSingleton()->GetString( rScore.m_iMessage );
			TStringManager::String16Format( wszMessage, 0x100, wszFormat, wszMaxColour, wszMinColour );
		}
		else
		{
			TStringManager::String16Format( wszMessage, 0x100, L"> Message [ %ls ] ", T2Locale::GetSingleton()->GetString( rScore.m_iMessage ) );
		}
	}

	TStringManager::String16Copy( rScore.m_wszMessage, wszMessage, 0x100 );
	m_oRectangle.SetVisible( TFALSE );
}

// $deBlob: FUNCTION 005d1170
void APlayerManager::RemoveVistaScoreTexts()
{
	for ( TINT i = NUM_VISTA_SCORE_TEXTS; i != 0; )
	{
		i--;
		T2GUITextBox& rTextBox = m_aVistaScoreTexts[ i ].m_oTextBox;
		rTextBox.SetText( TNULL );
		rTextBox.SetMaxChars( -1 );
		rTextBox.RemoveSelf();
	}

	m_oRectangle.RemoveSelf();
}

// $deBlob: FUNCTION 005d0570
void APlayerManager::ShowVistaScore( TBOOL a_bShow )
{
	if ( a_bShow )
	{
		CreateVistaScoreTexts();

		if ( m_pVistaHud == TNULL )
		{
			// ABlobVistaHud::OnCreated sets m_pVistaHud
			TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "vista.xur" ), 0 );
			m_pVistaHud->ShowUnk130( TTRUE );
			m_pVistaHud->ShowUnk140( TFALSE );
			TXUI::GetSingleton()->TransitionTo( TNULL, m_pVistaHud );
		}
	}
	else
	{
		RemoveVistaScoreTexts();

		if ( !AGameStatus::GetSingleton()->IsLevelComplete() )
			m_pVistaHud = TNULL;
	}
}

// $deBlob: FUNCTION 005cf4c0
TBOOL APlayerManager::UpdateVista( TFLOAT a_fDeltaTime )
{
	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() >= 0 )
	{
		m_pVistaCam = TNULL;
		return TFALSE;
	}

	switch ( m_eVistaState )
	{
		case VistaState_Start:
			APositionalSoundManager::GetSingleton()->SetPaused( TTRUE );
			m_pVistaCam = ACameraManager::GetSingleton()->GetNedCameras().FindVistaCam( m_iVista );

			if ( m_pVistaHud == TNULL )
			{
				// ABlobVistaHud::OnCreated sets m_pVistaHud
				TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "vista.xur" ), 0 );
				m_pVistaHud->ShowUnk130( TFALSE );
				m_pVistaHud->ShowUnk140( TTRUE );
				TXUI::GetSingleton()->TransitionTo( TNULL, m_pVistaHud );
			}

			m_eVistaState = VistaState_Intro;
			m_oVistaCam   = *m_pVistaCam;
			m_iVista++;
			m_oVistaCam.m_iFLAGS &= ~ANedCameraFunction::RS_BLEND;
			m_oVistaCam.m_fStartDelay = 0.0f;
			m_oVistaCam.m_fHoldTime   = 0.0f;

			if ( m_pVistaHud != TNULL )
				m_pVistaHud->GetTimelineControl().PlayAnimation( L"EndToStatic", L"EndToStatic" );

			return TTRUE;
		case VistaState_Intro:
			if ( m_pVistaHud->GetTimelineControl().IsPlaying() )
				return TTRUE;

			m_eVistaState = VistaState_Streaming;
			return TTRUE;
		case VistaState_Streaming:
			if ( !ATerrain2Interface::GetInstance()->IsCurrentCellReady() || ATerrain2::GetInstance()->IsStreaming() )
				return TTRUE;

			m_pVistaHud->GetTimelineControl().PlayAnimation( L"Normal", L"EndNormal" );
			m_eVistaState = VistaState_Score;
			return TTRUE;
		case VistaState_Score:
			CalculateVistaScore( 0, m_pVistaCam );
			m_pVistaHud->SetScore( TINT( m_soTempVistaScore.m_fScore ) );
			ShowVistaScore( TTRUE );
			m_eVistaState = VistaState_Shown;
			return TTRUE;
		case VistaState_End:
			if ( m_pVistaHud == TNULL )
				return TFALSE;

			return m_pVistaHud->GetTimelineControl().IsPlaying();
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005cd510
ABaseEntity* APlayerManager::SetCurrentPlayer( TUINT8 a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );
	m_uiCurrentPlayer = ( m_uiCurrentPlayer & 0xFF00 ) | a_iPlayer;
	return m_aBlobData[ a_iPlayer ].m_pBlob;
}

// Makes the player of a_pBlob the current one (-1 if it isn't a player's blob)
// $deBlob: FUNCTION 005cd570
void APlayerManager::SetCurrentPlayerBlob( ABaseEntity* a_pBlob )
{
	TINT8 iPlayer = -1;

	if ( a_pBlob != TNULL )
	{
		for ( TINT8 i = MAX_PLAYERS; i != 0; )
		{
			i--;
			if ( m_aBlobData[ i ].m_pBlob == a_pBlob )
			{
				iPlayer = i;
				break;
			}
		}
	}

	*TREINTERPRETCAST( TINT8*, &m_uiCurrentPlayer ) = iPlayer;
}

// $deBlob: FUNCTION 005cd5b0
ABlobFollowCamera* APlayerManager::GetFollowCamera( TUINT8 a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );

	if ( a_iPlayer >= MAX_PLAYERS )
		return TNULL;

	return m_aBlobData[ a_iPlayer ].m_pBlobCamera;
}

// $deBlob: FUNCTION 005cd690
AKeyFrameCamera* APlayerManager::GetKeyFrameCamera( TUINT8 a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );

	if ( a_iPlayer >= MAX_PLAYERS )
		return TNULL;

	return m_aBlobData[ a_iPlayer ].m_pKeyFrameCamera;
}

// $deBlob: FUNCTION 005cd6f0
AKeyFrameCamera* APlayerManager::GetBlobKeyFrameCamera( ABaseEntity* a_pBlob )
{
	return GetKeyFrameCamera( TUINT8( GetPlayerIndex( a_pBlob ) ) );
}

// $deBlob: FUNCTION 005cd770
AScriptedCamera* APlayerManager::GetScriptedCamera( TUINT8 a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );

	if ( a_iPlayer >= MAX_PLAYERS )
		return TNULL;

	return m_aBlobData[ a_iPlayer ].m_pScriptedCamera;
}

// $deBlob: FUNCTION 005cd7d0
AScriptedCamera* APlayerManager::GetBlobScriptedCamera( ABaseEntity* a_pBlob )
{
	return GetScriptedCamera( TUINT8( GetPlayerIndex( a_pBlob ) ) );
}

// $deBlob: FUNCTION 005cd9e0
TINT APlayerManager::GetCameraMode( TUINT8 a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < MAX_PLAYERS );

	if ( m_aBlobData[ a_iPlayer ].m_pBlob != TNULL )
		return m_aBlobData[ a_iPlayer ].m_eCameraMode;

	return -1;
}

// $deBlob: FUNCTION 005cfa90
TBOOL APlayerManager::IsUnk64( TINT a_iPlayer )
{
	if ( a_iPlayer >= 0 )
	{
		TASSERT( a_iPlayer < MAX_PLAYERS );
		return m_aBlobData[ a_iPlayer ].m_uiUnk64 != 0;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005cfaf0
TBOOL APlayerManager::IsVistaScoreShown() const
{
	return m_eVistaState == VistaState_Shown;
}

// $deBlob: FUNCTION 005cfb00
TINT APlayerManager::GetFixedCamUnk3E( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < m_iNumPlayers );

	BlobData& rData = m_aBlobData[ a_iPlayer ];

	if ( rData.m_eCameraMode == 1 )
	{
		ANedCameraFunction::FixedCam* pFixedCam = rData.m_pBlobCamera->GetNedFixedCam();

		if ( pFixedCam != TNULL )
			return pFixedCam->m_iUnk3E;
	}

	return 0;
}

// $deBlob: FUNCTION 005cfc60
void APlayerManager::ResetPlayerUnk( TUINT a_uiPlayer )
{
	// NOTE: retail's assert allows a_uiPlayer == MAX_PLAYERS (one past the players)
	TASSERT( a_uiPlayer <= MAX_PLAYERS );

	BlobData& rData = m_aBlobData[ a_uiPlayer ];
	rData.m_bUnk69  = TFALSE;
	rData.m_bUnk60  = TFALSE;
	TSTATICCAST( ABlobEntity, rData.m_pBlob )->SetLevelStartAttached( TFALSE );
	rData.m_bUnk68 = TFALSE;
	RemoveVistaScoreTexts();

	if ( !AGameStatus::GetSingleton()->IsLevelComplete() )
		m_pVistaHud = TNULL;
}

// $deBlob: FUNCTION 005cea20
void APlayerManager::RemovePlayerOf( ABaseEntity* a_pBlob )
{
	if ( a_pBlob != TNULL )
	{
		for ( TINT8 i = MAX_PLAYERS; i != 0; )
		{
			i--;
			if ( m_aBlobData[ i ].m_pBlob == a_pBlob )
			{
				RemovePlayer( TUINT8( i ) );
				return;
			}
		}
	}

	RemovePlayer( 0xFF );
}

// $deBlob: FUNCTION 005d0600
void APlayerManager::UpdatePlayer( TFLOAT a_fDeltaTime, TINT a_iPlayer )
{
	BlobData& rData = m_aBlobData[ a_iPlayer ];

	if ( 0.0f < rData.m_fPowerUpTimer )
	{
		rData.m_fPowerUpTimer -= a_fDeltaTime;

		if ( rData.m_fPowerUpTimer <= 0.0f )
			rData.m_iPowerUpType = -1;
	}

	TSTATICCAST( ABlobEntity, rData.m_pBlob )->PostPhysicsUpdate( a_fDeltaTime, rData.m_iController );

	if ( rData.m_iPowerUpType == -1 && rData.m_iUnk4D != -1 )
		rData.m_iUnk4D = -1;

	if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
		UpdateCameraRegions( a_fDeltaTime, a_iPlayer, TFALSE );
}

// $deBlob: FUNCTION 005d0a10
void APlayerManager::CreateHUD()
{
	const TCHAR*  szResource;
	const TWCHAR* wszScene;

	switch ( m_iNumPlayers )
	{
		case 1:
			szResource = "hud.xur";
			wszScene   = L"HUD";
			break;
		case 2:
			szResource = "hud_2p.xur";
			wszScene   = L"HUD_2P";
			break;
		case 3:
			szResource = "hud_3p.xur";
			wszScene   = L"HUD_3P";
			break;
		default:
			szResource = "hud_4p.xur";
			wszScene   = L"HUD_4P";
			break;
	}

	TXUI* pXUI = TXUI::GetSingleton();
	pXUI->SetFocussedScene( pXUI->CreateScene( pXUI->FindResource( szResource ), wszScene, TFALSE ) );
}

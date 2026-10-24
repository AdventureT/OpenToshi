#include "pch.h"
#include "AChallengeManager.h"
#include "APaintChallenge.h"
#include "AZJumpChallenge.h"
#include "ACombatChallenge.h"
#include "ARaceChallenge.h"
#include "ALandmarkChallenge.h"
#include "ACivilianChallenge.h"
#include "ACarChallenge.h"
#include "ANullChallenge.h"
#include "AUberChallenge.h"
#include "ARevolutionary/ARevolutionary.h"
#include "Entities/AEntityFactory.h"
#include "Entities/APropWaterHydrant.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "ACurrentLevelInformation.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Buildings/ALandmarkInstance.h"
#include "Buildings/ABuildingManager.h"
#include "Entities/APropTriggerHelp.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "FXComposition/AFXCompositionInstance.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/AEntityHashFilter.h"
#include "Entities/AEntityHash.h"
#include "Entities/ACiv.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "XUI/InGame/ABlobChallengeScene.h"
#include "Input/ABlobInputMap.h"
#include "XUI/InGame/HUD/ABlobTimerHud.h"
#include "XUI/InGame/HUD/ABlobChallengeStatusHud.h"
#include "XUI/InGame/HUD/ABlobChallengeEndHud.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Render/TRenderContext.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AFXCompositionInstance* AChallengeManager::s_pEffect;

// Whether a sprint level is played (no challenge HUD, no challenge music)
static TBOOL AChallengeManager_IsSprintLevel()
{
	return AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel != 0;
}

// Whether XUI isn't in a transition and no control is pressed
static TBOOL AChallengeManager_IsXUIIdle()
{
	TXUI* pXUI = TXUI::GetSingleton();
	return pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked();
}

// Builds a_uiCount challenges of type T (each 0x174+ bytes) from their data (a_uiDataSize bytes each), adds them to
// m_ppChallenges from a_rIndex on and returns the array (TNULL if a_uiCount is 0)
template <class T>
static T* AChallengeManager_CreateChallenges( TUINT32 a_uiCount, void* a_pData, TUINT32 a_uiDataSize, AChallenge** a_ppChallenges, TINT& a_rIndex, TINT a_iTotal )
{
	if ( a_uiCount == 0 )
		return TNULL;

	T* pChallenges = new T[ a_uiCount ];

	for ( TUINT32 i = 0; i < a_uiCount; i++ )
	{
		pChallenges[ i ].Create( TREINTERPRETCAST( typename T::TypeData*, TREINTERPRETCAST( TUINT8*, a_pData ) + i * a_uiDataSize ) );
		TASSERT( a_rIndex < a_iTotal );
		a_ppChallenges[ a_rIndex++ ] = &pChallenges[ i ];
	}

	return pChallenges;
}

// $deBlob: FUNCTION 00599b50
AChallengeManager::AChallengeManager()
{
	m_pChallengesData     = TNULL;
	m_ppChallenges        = TNULL;
	m_pPaintChallenges    = TNULL;
	m_pZJumpChallenges    = TNULL;
	m_pCombatChallenges   = TNULL;
	m_pRaceChallenges     = TNULL;
	m_pLandmarkChallenges = TNULL;
	m_pCivilianChallenges = TNULL;
	m_pCarChallenges      = TNULL;
	m_pNullChallenges     = TNULL;
	m_pUberChallenges     = TNULL;
	m_pNearbyChallenge    = TNULL;
	m_pCurrentChallenge   = TNULL;
	m_pChallengeScene     = TNULL;
	m_pHudElement         = TNULL;
	m_uiUnk3C             = 0;
	m_bWaitingForPrompt   = TFALSE;

	m_iTotalNumChallenges       = 0;
	m_iTotalCompletedChallenges = 0;
	m_iNumSubChallenges         = 0;
	m_bPromptShown              = TFALSE;
	m_iLastScore                = 0;
	m_iCounter                  = 0;
	m_iCounterTotal             = 0;

	m_fContextHelpTimer    = 0.0f;
	m_bEnding              = TFALSE;
	m_bFailed              = TFALSE;
	m_uiLastNumLinked      = 0;
	m_bShowChallengeScreen = TFALSE;
	m_bShowCompletedHelp   = TFALSE;
	m_completedChallenge   = TNULL;

	m_oRestartInfo.m_pChallenge      = TNULL;
	m_oRestartInfo.m_uiBlobColour    = 0;
	m_oRestartInfo.m_vBlobPosition   = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_oRestartInfo.m_vBlobPosition.w = 1.0f;
	m_oRestartInfo.m_iBlobPoints     = 0;
}

// $deBlob: FUNCTION 00599cb0
// $deBlob: FUNCTION 0056bec0
AChallengeManager::~AChallengeManager()
{
	Destroy();
}

// $deBlob: FUNCTION 00599b00
void AChallengeManager::SetRestartInfo( const RestartInfo& a_rInfo )
{
	m_oRestartInfo = a_rInfo;
}

// $deBlob: FUNCTION 00599d40
void AChallengeManager::Update( TFLOAT a_fDeltaTime )
{
	TXUI* pXUI = TXUI::GetSingleton();

	// The help of the completed challenge, once Blob may move again
	if ( m_bShowCompletedHelp )
	{
		TASSERT( m_completedChallenge != TNULL );

		if ( APlayerManager::GetSingleton()->GetBlobData( 0 )->m_iCameraState == 5 )
		{
			TINT   iStringID   = -1;
			TUINT8 uiNumLinked = m_completedChallenge->GetData()->m_uiNumLinkedChallenges;

			if ( uiNumLinked != 0 )
				iStringID = ( uiNumLinked != 1 ) + 0xC5;

			APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_LetterBox, iStringID, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );
			m_completedChallenge = TNULL;
			m_bShowCompletedHelp = TFALSE;
		}
	}

	// Blob touched a marker
	if ( m_bShowChallengeScreen && AChallengeManager_IsXUIIdle() && pXUI->GetActiveScene() != TNULL &&
	     pXUI->GetActiveScene()->IsA( &TGetClass( ABlobHud ) ) )
	{
		ShowChallengeScreen();
		m_bShowChallengeScreen = TFALSE;
	}

	AChallenge* pCurrent = m_pCurrentChallenge;

	if ( pCurrent == TNULL )
	{
		// The context help about challenges after 3 minutes without one
		if ( 0 < m_iTotalNumChallenges && !AChallengeManager_IsSprintLevel() )
		{
			m_fContextHelpTimer += a_fDeltaTime;

			if ( 180.0f <= m_fContextHelpTimer )
			{
				APropTriggerHelp::ShowContextHelp( 0x800 );
				m_fContextHelpTimer = 0.0f;
			}
		}
	}
	else if ( !( pCurrent->GetFlags() & AChallenge::Flags_Starting ) )
	{
		if ( !m_bEnding )
		{
			pCurrent->Update( a_fDeltaTime );

			if ( !AChallengeManager_IsSprintLevel() && m_pCurrentChallenge != TNULL )
			{
				AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();
				TFLOAT                  fTime;

				// The challenge's time, at most the level's
				if ( m_pCurrentChallenge->GetData()->m_iTimeLimit == -1 )
					fTime = rLevelData.m_fTimeLeft;
				else
					fTime = TMath::Min( m_pCurrentChallenge->GetTimeLeft(), rLevelData.m_fTimeLeft );

				ABlobHud* pHud = ABlobHud::GetSingleton();

				if ( pHud != TNULL && pHud->GetChallengeTimerControl() != TNULL )
				{
					TFLOAT fSeconds = fmodf( fTime, 60.0f );
					TFLOAT fTenths  = fmodf( fSeconds, 1.0f );
					pHud->GetChallengeTimer()->SetTime( TINT( fTime / 60.0f ), TINT( fSeconds ), TINT( fTenths * 10.0f ) );
				}
			}
		}
		else
		{
			// The challenge is over: challengeend.xur once the HUD (or the letterbox) is up and Blob rolls
			TXUIScene*   pActiveScene = pXUI->GetActiveScene();
			ABlobEntity* pBlob        = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
			TBOOL        bLetterBox   = pActiveScene != TNULL && TStringManager::String16Compare( pActiveScene->GetID(), L"LetterBox", -1 ) == 0;

			if ( pActiveScene != TNULL && ( bLetterBox || pActiveScene->IsA( &TGetClass( ABlobHud ) ) ) && AChallengeManager_IsXUIIdle() &&
			     pBlob->m_eState == ABlobEntity::STATE_ROLL )
			{
				pBlob->ResetMotion();

				TXUIScene* pEndScene = pXUI->CreateScene( pXUI->FindResource( "challengeend.xur" ), 0u );
				TSTATICCAST( ABlobChallengeEndHud, pEndScene )->SetChallenge( m_pCurrentChallenge, m_bFailed );

				if ( !bLetterBox )
				{
					pXUI->TransitionTo( TNULL, pEndScene );
					return;
				}

#ifdef DEBLOB_COMMUNITY_PATCH
				// NOTE: retail reads the letterbox's previous scene after deleting it
				TXUIScene* pPrevScene = pActiveScene->GetPrevScene();
				delete pActiveScene;
				pEndScene->SetPrevScene( pPrevScene );
#else
				delete pActiveScene;
				pEndScene->SetPrevScene( pActiveScene->GetPrevScene() );
#endif
				pXUI->SetFocussedScene( pEndScene );
			}
		}
	}
	else
	{
		// Starting: the challenge begins once the camera is back and nothing fades
		APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobData( 0 );

		if ( !pBlobData->IsKeyFrameCameraActive() && pBlobData->m_iCameraState == 5 && AChallengeManager_IsXUIIdle() &&
		     !AGameStateController::GetSingleton()->IsFading( 0 ) )
		{
			OnChallengeCompleted( m_pCurrentChallenge, TFALSE );
		}
	}
}

// $deBlob: FUNCTION 0059a150
void AChallengeManager::LoadChallenges()
{
	Destroy();

	TTRB* pTRB        = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_ChallengeData );
	m_pChallengesData = TSTATICCAST( ChallengesData, pTRB->GetSymbolAddress( "Challenges" ) );

	ChallengesData* pData = m_pChallengesData;

	m_iTotalNumChallenges = pData->m_uiNumPaint + pData->m_uiNumUber + pData->m_uiNumNull + pData->m_uiNumCar +
	    pData->m_uiNumCivilian + pData->m_uiNumLandmark + pData->m_uiNumRace + pData->m_uiNumCombat +
	    pData->m_uiNumZJump;

	m_ppChallenges = TSTATICCAST( AChallenge*, TMemalign( 16, m_iTotalNumChallenges * sizeof( AChallenge* ) ) );

	TINT iIndex           = 0;
	m_pPaintChallenges    = AChallengeManager_CreateChallenges<APaintChallenge>( pData->m_uiNumPaint, pData->m_pPaint, 0x40, m_ppChallenges, iIndex, m_iTotalNumChallenges );
	m_pZJumpChallenges    = AChallengeManager_CreateChallenges<AZJumpChallenge>( pData->m_uiNumZJump, pData->m_pZJump, 0x40, m_ppChallenges, iIndex, m_iTotalNumChallenges );
	m_pCombatChallenges   = AChallengeManager_CreateChallenges<ACombatChallenge>( pData->m_uiNumCombat, pData->m_pCombat, 0x40, m_ppChallenges, iIndex, m_iTotalNumChallenges );
	m_pRaceChallenges     = AChallengeManager_CreateChallenges<ARaceChallenge>( pData->m_uiNumRace, pData->m_pRace, 0x48, m_ppChallenges, iIndex, m_iTotalNumChallenges );
	m_pLandmarkChallenges = AChallengeManager_CreateChallenges<ALandmarkChallenge>( pData->m_uiNumLandmark, pData->m_pLandmark, 0x40, m_ppChallenges, iIndex, m_iTotalNumChallenges );
	m_pCivilianChallenges = AChallengeManager_CreateChallenges<ACivilianChallenge>( pData->m_uiNumCivilian, pData->m_pCivilian, 0x40, m_ppChallenges, iIndex, m_iTotalNumChallenges );
	m_pCarChallenges      = AChallengeManager_CreateChallenges<ACarChallenge>( pData->m_uiNumCar, pData->m_pCar, 0x40, m_ppChallenges, iIndex, m_iTotalNumChallenges );
	m_pNullChallenges     = AChallengeManager_CreateChallenges<ANullChallenge>( pData->m_uiNumNull, pData->m_pNull, 0x3C, m_ppChallenges, iIndex, m_iTotalNumChallenges );
	m_pUberChallenges     = AChallengeManager_CreateChallenges<AUberChallenge>( pData->m_uiNumUber, pData->m_pUber, 0x3C, m_ppChallenges, iIndex, m_iTotalNumChallenges );

	TASSERT( iIndex == m_iTotalNumChallenges );

	for ( TINT i = 0; i < m_iTotalNumChallenges; i++ )
		m_ppChallenges[ i ]->LinkChallenges();

	for ( TUINT32 i = 0; i < m_pChallengesData->m_uiNumUber; i++ )
		m_pUberChallenges[ i ].LinkSubChallenges();

	for ( TINT i = 0; i < m_iTotalNumChallenges; i++ )
	{
		if ( m_ppChallenges[ i ]->GetData()->m_uiFlags & AChallenge::Data::Flags_SubChallenge )
			m_iNumSubChallenges++;
	}

	Reset();
}

// Deletes an array made by AChallengeManager_CreateChallenges
template <class T>
static void AChallengeManager_DeleteChallenges( T*& a_rpChallenges )
{
	if ( a_rpChallenges != TNULL )
		delete[] a_rpChallenges;

	a_rpChallenges = TNULL;
}

// $deBlob: FUNCTION 0059ab40
void AChallengeManager::Destroy()
{
	AChallengeManager_DeleteChallenges( m_pPaintChallenges );
	AChallengeManager_DeleteChallenges( m_pZJumpChallenges );
	AChallengeManager_DeleteChallenges( m_pCombatChallenges );
	AChallengeManager_DeleteChallenges( m_pRaceChallenges );
	AChallengeManager_DeleteChallenges( m_pLandmarkChallenges );
	AChallengeManager_DeleteChallenges( m_pCivilianChallenges );
	AChallengeManager_DeleteChallenges( m_pNullChallenges );
	AChallengeManager_DeleteChallenges( m_pUberChallenges );
	AChallengeManager_DeleteChallenges( m_pCarChallenges );

	TFree( m_ppChallenges );
	m_ppChallenges = TNULL;

	m_iTotalNumChallenges       = 0;
	m_iTotalCompletedChallenges = 0;
	m_iNumSubChallenges         = 0;
	m_pNearbyChallenge          = TNULL;
	m_pCurrentChallenge         = TNULL;
	m_bWaitingForPrompt         = TFALSE;
	m_bPromptShown              = TFALSE;
	m_iLastScore                = 0;
}

// $deBlob: FUNCTION 0059ad00
void AChallengeManager::DisableAllChallengeCameras()
{
	TTRB*           pTRB  = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_ChallengeData );
	ChallengesData* pData = TSTATICCAST( ChallengesData, pTRB->GetSymbolAddress( "Challenges" ) );

	struct
	{
		TUINT32 uiCount;
		void*   pData;
		TUINT32 uiSize;
	} aTypes[] = {
		{ pData->m_uiNumPaint, pData->m_pPaint, 0x40 },
		{ pData->m_uiNumZJump, pData->m_pZJump, 0x40 },
		{ pData->m_uiNumCombat, pData->m_pCombat, 0x40 },
		{ pData->m_uiNumRace, pData->m_pRace, 0x48 },
		{ pData->m_uiNumLandmark, pData->m_pLandmark, 0x40 },
		{ pData->m_uiNumCivilian, pData->m_pCivilian, 0x40 },
		{ pData->m_uiNumCar, pData->m_pCar, 0x40 },
		{ pData->m_uiNumNull, pData->m_pNull, 0x3C },
		{ pData->m_uiNumUber, pData->m_pUber, 0x3C },
	};

	for ( TINT iType = 0; iType < TARRAYSIZE( aTypes ); iType++ )
	{
		for ( TUINT32 i = 0; i < aTypes[ iType ].uiCount; i++ )
			DisableChallengeCameras( TREINTERPRETCAST( AChallenge::Data*, TSTATICCAST( TUINT8, aTypes[ iType ].pData ) + i * aTypes[ iType ].uiSize ) );
	}
}

// $deBlob: FUNCTION 0059b060
void AChallengeManager::OnPaintEvent( void* a_pBuilding )
{
	if ( m_pCurrentChallenge != TNULL )
		m_pCurrentChallenge->OnPaintEvent( a_pBuilding );
}

// $deBlob: FUNCTION 0059b080
void AChallengeManager::OnPaintEvent2( void* a_pTree )
{
	if ( m_pCurrentChallenge != TNULL )
		m_pCurrentChallenge->OnPaintEvent2( a_pTree );
}

// $deBlob: FUNCTION 0059b0a0
void AChallengeManager::OnZJumpEvent( void* a_pSet )
{
	if ( m_pCurrentChallenge != TNULL )
		m_pCurrentChallenge->OnZJumpEvent( a_pSet );
}

// $deBlob: FUNCTION 0059b0c0
void AChallengeManager::OnCombatEvent( void* a_pEntityID )
{
	if ( m_pCurrentChallenge != TNULL )
		m_pCurrentChallenge->OnCombatEvent( a_pEntityID );
}

// $deBlob: FUNCTION 0059b0e0
void AChallengeManager::OnCivilianEvent( void* a_pEntityID )
{
	if ( m_pCurrentChallenge != TNULL )
		m_pCurrentChallenge->OnCivilianEvent( a_pEntityID );
}

// $deBlob: FUNCTION 0059b100
void AChallengeManager::OnCarEvent( void* a_pEntityID )
{
	if ( m_pCurrentChallenge != TNULL )
		m_pCurrentChallenge->OnCarEvent( a_pEntityID );
}

// $deBlob: FUNCTION 0059b120
void AChallengeManager::OnLandmarkEvent( void* a_pLandmark )
{
	if ( m_pCurrentChallenge != TNULL )
		m_pCurrentChallenge->OnLandmarkEvent( a_pLandmark );
}

// $deBlob: FUNCTION 0059b140
void AChallengeManager::OnLandmarkEvent2( void* a_pLandmarkInstance )
{
	if ( m_pCurrentChallenge != TNULL )
		m_pCurrentChallenge->OnLandmarkEvent2( a_pLandmarkInstance );
}

// $deBlob: FUNCTION 0059b160
void AChallengeManager::OnBlobDied()
{
	AChallenge* pCurrent = m_pCurrentChallenge;

	if ( pCurrent == TNULL || AChallengeManager_IsSprintLevel() )
		return;

	if ( !( pCurrent->GetFlags() & AChallenge::Flags_Starting ) && ( !m_bEnding || m_bFailed ) )
	{
		if ( pCurrent->GetData()->m_uiFlags & 1 )
			TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) )->m_uiUnkB68 = TREINTERPRETCAST( TUINT32, m_pCurrentChallenge );
	}
	else
	{
		EndChallenge( pCurrent, TFALSE, TFALSE );
	}
}

// Completes the current challenge without the help
// $deBlob: FUNCTION 0059b1d0
void AChallengeManager::CompleteCurrentChallenge()
{
	OnChallengeCompleted( m_pCurrentChallenge, TFALSE );
}

// $deBlob: FUNCTION 0059b1e0
void AChallengeManager::OnChallengeCompleted( AChallenge* a_pChallenge, TBOOL a_bShowHelp )
{
	m_bShowCompletedHelp = TFALSE;
	m_completedChallenge = TNULL;

	if ( TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) )->m_bInBlobVision != 0 )
		return;

	TUINT32 uiFlags = a_pChallenge->GetData()->m_uiFlags;

	// A sub challenge: the uber challenge goes on
	if ( uiFlags & AChallenge::Data::Flags_SubChallenge )
	{
		a_pChallenge->Complete();
		TASSERT( m_pCurrentChallenge->GetType() == AChallenge::Type_Uber );
		TSTATICCAST( AUberChallenge, m_pCurrentChallenge )->OnSubChallengeCompleted( a_pChallenge );
		return;
	}

	if ( ( !( uiFlags & AChallenge::Data::Flags_CompletedCameraOnRequest ) || a_bShowHelp ) && a_pChallenge->GetData()->m_iCompletedSSCamera != -1 )
	{
		m_bShowCompletedHelp = a_bShowHelp;

		if ( a_bShowHelp )
			m_completedChallenge = a_pChallenge;

		// First the camera of the completed challenge (Update calls this again once it's over)
		if ( !( a_pChallenge->GetFlags() & AChallenge::Flags_Starting ) )
		{
			APlayerManager::GetSingleton()->PlaySSCamera( 0, a_pChallenge->GetData()->m_iCompletedSSCamera );
			a_pChallenge->SetFlags( a_pChallenge->GetFlags() | AChallenge::Flags_Starting );
			return;
		}

		a_pChallenge->SetFlags( a_pChallenge->GetFlags() & ~AChallenge::Flags_Starting );
	}

	if ( !AChallengeManager_IsSprintLevel() )
	{
		m_bEnding = TTRUE;
		m_bFailed = TFALSE;
	}
	else
	{
		EndChallenge( m_pCurrentChallenge, TFALSE, TFALSE );
	}

	if ( a_pChallenge == m_pNearbyChallenge )
		m_pNearbyChallenge = TNULL;
}

// $deBlob: FUNCTION 0059b310
void AChallengeManager::OnChallengeTimeOut( AChallenge* a_pChallenge )
{
	if ( a_pChallenge->GetData()->m_uiFlags & AChallenge::Data::Flags_SubChallenge )
	{
		TASSERT( a_pChallenge != m_pCurrentChallenge );

		AChallenge* pCurrChallenge = AChallengeManager::GetSingleton()->m_pCurrentChallenge;
		TASSERT( TNULL != pCurrChallenge && pCurrChallenge->GetType() == AChallenge::Type_Uber );

		TSTATICCAST( AUberChallenge, pCurrChallenge )->OnSubChallengeTimeOut( a_pChallenge );
	}
	else if ( a_pChallenge == m_pCurrentChallenge )
	{
		FailCurrentChallenge();
	}
}

// $deBlob: FUNCTION 0059b3c0
void AChallengeManager::FailCurrentChallenge()
{
	TASSERT( m_pCurrentChallenge );

	if ( m_pCurrentChallenge == TNULL )
		return;

	ABlobEntity*            pBlob      = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();

	if ( pBlob->m_eState == ABlobEntity::STATE_DEAD || AChallengeManager_IsSprintLevel() || rLevelData.m_fTimeLeft <= 0.0f )
	{
		EndChallenge( m_pCurrentChallenge, TTRUE, TFALSE );
		return;
	}

	m_bEnding = TTRUE;
	m_bFailed = TTRUE;

	if ( pBlob->m_eState == ABlobEntity::STATE_LMARK )
	{
		pBlob->ExitLandmark( TFALSE );
	}
	else if ( pBlob->m_eState == ABlobEntity::STATE_BOSS )
	{
		pBlob->ExitBoss( TFALSE );
	}
}

// $deBlob: FUNCTION 0059b470
void AChallengeManager::QuitChallenge( TBOOL a_bForce )
{
	if ( m_pCurrentChallenge == TNULL || ( !( m_pCurrentChallenge->GetData()->m_uiFlags & AChallenge::Data::Flags_CanQuit ) && !a_bForce ) )
		return;

	m_pCurrentChallenge->Quit();
	m_pCurrentChallenge = TNULL;

	ABlobHud* pHud = ABlobHud::GetSingleton();

	if ( pHud != TNULL && pHud->GetChallengeTimerControl() != TNULL )
		pHud->GetChallengeTimer()->ResetWarning();

	// Blob's body back in the simulation
	ABlobEntity*                    pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	Toshi::TPhysicsObjectContainer* pBody = pBlob->m_pBody;

	TPhysicsManager::GetSingleton()->RemoveFromSimulation( pBody, TFALSE );
	TPhysicsManager::GetSingleton()->AddToSimulation( pBody, TFALSE );

	if ( pBody != TNULL && ( pBody->m_flags & 0x1000 ) )
	{
		pBody->m_body->activate();
	}

	if ( pHud != TNULL && !AChallengeManager_IsSprintLevel() )
		pHud->FindChildXUIElement( 1 )->PlayAnimation( L"EndChallengeToNormal", L"EndChallengeToNormal" );

	m_bEnding = TFALSE;
}

// $deBlob: FUNCTION 0059b550
void AChallengeManager::StartChallenge()
{
	AChallenge* pChallenge = m_pNearbyChallenge;

	if ( pChallenge == TNULL )
		return;

	m_pCurrentChallenge = pChallenge;
	m_pNearbyChallenge  = TNULL;
	pChallenge->Start();

	ABlobHud* pHud = ABlobHud::GetSingleton();

	if ( pHud != TNULL && !AChallengeManager_IsSprintLevel() )
		pHud->FindChildXUIElement( 1 )->PlayAnimation( L"NormalToChallenge", L"EndNormalToChallenge" );
}

// $deBlob: FUNCTION 0059b5a0
void AChallengeManager::HideNearbyPrompt()
{
	if ( m_pNearbyChallenge != TNULL )
		m_pNearbyChallenge->HideStartPrompt();
}

// $deBlob: FUNCTION 0059b5b0
void AChallengeManager::ShowChallengeScreen()
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();

	if ( !pState->IsA( &TGetClass( AInGameState ) ) || TSTATICCAST( AInGameState, pState )->IsPaused() )
		return;

	if ( m_pNearbyChallenge == TNULL || m_pNearbyChallenge->IsCompleted() )
		return;

	TXUI*        pXUI         = TXUI::GetSingleton();
	TXUIScene*   pActiveScene = pXUI->GetActiveScene();
	ABlobEntity* pBlob        = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

	if ( 0 < pBlob->m_iLeechbotsAttached )
		return;

	// Blob is inky: only the challenges with a camera may start
	TUINT32 uiColour = pBlob->m_pColourComponent->GetCurrentColour();

	if ( ( ( uiColour & 0x10 ) || uiColour == 0x10 ) && !( m_pNearbyChallenge->GetData()->m_uiFlags & AChallenge::Data::Flags_HasCamera ) )
	{
		if ( !APropTriggerHelp::IsHelpHudShown() )
			APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_Hud, 0x2C, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );

		return;
	}

	if ( !AChallengeManager_IsXUIIdle() || pActiveScene == TNULL || !pActiveScene->IsA( &TGetClass( ABlobHud ) ) )
		return;

	m_pNearbyChallenge->ShowStartPrompt();
	TPhysicsManager::StopBody( pBlob->m_pBody );

	ABlobInputMap::GetSingleton()->ClearPlayers();

	ABuildingManager::GetSingleton()->SetLandmarkTargetScenesVisible( TFALSE );

	TXUIScene* pScene = pXUI->CreateScene( pXUI->FindResource( "challenge.xur" ), 0u );

	if ( m_pNearbyChallenge->GetData()->m_uiFlags & AChallenge::Data::Flags_MirrorLayout )
		TSTATICCAST( ABlobChallengeScene, pScene )->MirrorLayout();

	pXUI->TransitionTo( TNULL, pScene );
}

// $deBlob: FUNCTION 0059b750
void AChallengeManager::RestartChallenge()
{
	AChallenge* pChallenge = m_oRestartInfo.m_pChallenge;

	if ( pChallenge == TNULL )
		return;

	m_pNearbyChallenge  = TNULL;
	m_pCurrentChallenge = pChallenge;
	pChallenge->Reset( TTRUE );
	m_pCurrentChallenge->SetRestartedFlag();
	ABlobHud::GetSingleton()->GetChallengeTimer()->ResetWarning();
	m_pCurrentChallenge->Start();
	m_pCurrentChallenge->GetRevolutionary()->HideMarker();
}

// $deBlob: FUNCTION 0059b7a0
AChallenge* AChallengeManager::FindChallengeByGUID( TINT a_iGUID ) const
{
	for ( TINT i = 0; i < m_iTotalNumChallenges; i++ )
	{
		if ( m_ppChallenges[ i ]->GetData()->m_iGUID == a_iGUID )
			return m_ppChallenges[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 0059b7f0
void AChallengeManager::Reset()
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	TINT         eLevelMode  = pGameStatus->GetLevelMode();

	if ( pGameStatus->GetLevelData().m_bMultiplayer || ( eLevelMode != 0 && eLevelMode != -1 && eLevelMode != 3 ) )
		DisableAllChallengeCameras();

	ABlobHud* pHud = ABlobHud::GetSingleton();

	if ( pHud != TNULL )
	{
		TBOOL        bSprint = AChallengeManager_IsSprintLevel();
		TXUIElement* pMain   = pHud->FindChildXUIElement( 1 );

		if ( m_pCurrentChallenge != TNULL && !bSprint )
			pMain->PlayAnimation( L"EndChallengeToNormal", L"EndChallengeToNormal" );

		if ( m_pCurrentChallenge != TNULL || !bSprint )
			pMain->PlayAnimation( L"EndNormalToChallenge", L"EndNormalToChallenge" );
	}

	m_pCurrentChallenge = TNULL;
	m_pNearbyChallenge  = TNULL;
	ResetChallenges();

	m_iTotalCompletedChallenges = 0;
	m_iCounter                  = 0;
	m_iCounterTotal             = 0;
	m_fContextHelpTimer         = 0.0f;
	m_bEnding                   = TFALSE;
	m_bFailed                   = TFALSE;
	m_uiLastNumLinked           = 0;
	m_bShowChallengeScreen      = TFALSE;

	if ( pHud != TNULL && pHud->GetChallengeTimerControl() != TNULL )
		pHud->GetChallengeTimer()->ResetWarning();

	m_iLastScore                     = 0;
	m_oRestartInfo.m_pChallenge      = TNULL;
	m_oRestartInfo.m_uiBlobColour    = 0;
	m_oRestartInfo.m_vBlobPosition   = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	m_oRestartInfo.m_vBlobPosition.w = 1.0f;
	m_oRestartInfo.m_iBlobPoints     = 0;
}

// $deBlob: FUNCTION 0059b920
void AChallengeManager::ResetChallenges()
{
	for ( TINT i = 0; i < m_iTotalNumChallenges; i++ )
	{
		m_ppChallenges[ i ]->Reset( TFALSE );
		m_ppChallenges[ i ]->SetAvailable( m_ppChallenges[ i ]->m_oLinkedFrom.Size() < 1, TFALSE );
	}
}

// $deBlob: FUNCTION 0059b980
void AChallengeManager::SetCounter( TINT a_iCount, TINT a_iTotal )
{
	m_iCounter      = a_iCount;
	m_iCounterTotal = a_iTotal;

	if ( m_pHudElement == TNULL || a_iCount == -1 )
		return;

	TINT eType = m_pCurrentChallenge->GetType();

	if ( eType != AChallenge::Type_Paint && eType != AChallenge::Type_Landmark )
	{
		TStringManager::String16Format( m_wszCounter, TARRAYSIZE( m_wszCounter ), L"%d", a_iTotal - a_iCount );
		TSTATICCAST( ABlobChallengeStatusHud, m_pHudElement )->SetCounterText( m_wszCounter );
		return;
	}

	TSTATICCAST( ABlobChallengeStatusHud, m_pHudElement )->SetCounterText( TNULL );
}

// $deBlob: FUNCTION 0059ba00
AZJumpChallenge* AChallengeManager::FindZJumpChallenge( TINT a_iWPSetID ) const
{
	for ( TUINT32 i = 0; i < m_pChallengesData->m_uiNumZJump; i++ )
	{
		if ( m_pZJumpChallenges[ i ].UsesWPSet( a_iWPSetID ) )
			return &m_pZJumpChallenges[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 0059ba80
void AChallengeManager::CreateEffect()
{
	TASSERT( TNULL == s_pEffect );
	s_pEffect = new AFXCompositionInstance();
	s_pEffect->Initialize( "Blob_Powerup_med" );
}

// $deBlob: FUNCTION 0059bb30
void AChallengeManager::DestroyEffect()
{
	s_pEffect->Destroy();

	if ( s_pEffect != TNULL )
		delete s_pEffect;

	s_pEffect = TNULL;
}

// $deBlob: FUNCTION 0059bb70
void AChallengeManager::TriggerEffect( const TMatrix44& a_rTransform )
{
	s_pEffect->TriggerEffect( a_rTransform );
}

// $deBlob: FUNCTION 0059bb80
TFLOAT AChallengeManager::GetNearestChallengePosition( const TVector4& a_rvPos, TVector4& a_rvOutPos ) const
{
	TFLOAT fNearest = TMath::MAXFLOAT;
	a_rvOutPos      = TVector4::VEC_ZERO; // TVector4::X0Y0Z0W1
	a_rvOutPos.w    = 1.0f;

	for ( TINT i = 0; i < m_iTotalNumChallenges; i++ )
	{
		const TVector4& rPos    = m_ppChallenges[ i ]->GetData()->m_pMatrix->GetTranslation();
		TFLOAT          fX      = a_rvPos.x - rPos.x;
		TFLOAT          fZ      = a_rvPos.z - rPos.z;
		TFLOAT          fDistSq = fX * fX + fZ * fZ;

		if ( fDistSq < fNearest )
		{
			a_rvOutPos = rPos;
			fNearest   = fDistSq;
		}
	}

	// NOTE: retail returns an uninitialised local
	return fNearest;
}

// $deBlob: FUNCTION 0059bc10
AChallenge* AChallengeManager::FindNearestChallengeToBlob( TBOOL a_bVisible )
{
	const TVector4& rPos = APlayerManager::GetSingleton()->GetPlayerBlob( 0 )->GetMatrix().GetTranslation();

	if ( a_bVisible )
		return AChallengeManager::GetSingleton()->FindNearestVisibleChallenge( rPos );

	return AChallengeManager::GetSingleton()->FindNearestAvailableChallenge( rPos );
}

// $deBlob: FUNCTION 0059bc50
AChallenge* AChallengeManager::FindNearestAvailableChallenge( const TVector4& a_rvPos ) const
{
	TFLOAT      fNearest = TMath::MAXFLOAT;
	AChallenge* pNearest = TNULL;

	for ( TINT i = 0; i < m_iTotalNumChallenges; i++ )
	{
		AChallenge* pChallenge = m_ppChallenges[ i ];

		if ( pChallenge->IsAvailable() && !pChallenge->IsCompleted() )
		{
			const TVector4& rPos    = pChallenge->GetData()->m_pMatrix->GetTranslation();
			TFLOAT          fX      = a_rvPos.x - rPos.x;
			TFLOAT          fZ      = a_rvPos.z - rPos.z;
			TFLOAT          fDistSq = fX * fX + fZ * fZ;

			if ( fDistSq < fNearest )
			{
				pNearest = pChallenge;
				fNearest = fDistSq;
			}
		}
	}

	return pNearest;
}

// $deBlob: FUNCTION 0059bcd0
AChallenge* AChallengeManager::FindNearestVisibleChallenge( const TVector4& a_rvPos ) const
{
	TFLOAT      fNearest = TMath::MAXFLOAT;
	AChallenge* pNearest = TNULL;

	for ( TINT i = 0; i < m_iTotalNumChallenges; i++ )
	{
		AChallenge* pChallenge = m_ppChallenges[ i ];

		if ( pChallenge->IsAvailable() && !pChallenge->IsCompleted() )
		{
			TSphere oSphere( pChallenge->GetData()->m_pMatrix->GetTranslation().AsVector3(), 0.01f );

			if ( !TRenderContext::CullSphereToFrustumSimple( oSphere, ACameraManager::GetSingleton()->GetCurrentCamera()->GetViewPlanes(), 6 ) )
				continue;

			const TVector4& rPos    = pChallenge->GetData()->m_pMatrix->GetTranslation();
			TFLOAT          fX      = a_rvPos.x - rPos.x;
			TFLOAT          fZ      = a_rvPos.z - rPos.z;
			TFLOAT          fDistSq = fZ * fZ + fX * fX;

			if ( fDistSq < fNearest )
			{
				pNearest = pChallenge;
				fNearest = fDistSq;
			}
		}
	}

	return pNearest;
}

// $deBlob: FUNCTION 0059bdc0
TINT AChallengeManager::GetPaintChallengeIndex( TINT a_iID ) const
{
	AChallenge* pChallenge = m_pCurrentChallenge;

	if ( pChallenge == TNULL || a_iID == 0 )
		return -1;

	// Down to three levels of uber challenges
	for ( TINT i = 0; i < 3 && pChallenge->GetType() == AChallenge::Type_Uber; i++ )
	{
		pChallenge = TSTATICCAST( AUberChallenge, pChallenge )->GetCurrentSubChallenge();

		if ( pChallenge == TNULL )
			return -1;
	}

	if ( pChallenge->GetType() != AChallenge::Type_Paint )
		return -1;

	return TSTATICCAST( APaintChallenge, pChallenge )->GetTargetIndex( a_iID );
}

// $deBlob: FUNCTION 0059be50
TBOOL AChallengeManager::CanStartNearbyChallenge() const
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

	return m_pNearbyChallenge != TNULL && !( m_pNearbyChallenge->GetData()->m_uiFlags & AChallenge::Data::Flags_HasCamera ) &&
	    pBlob->m_iLeechbotsAttached < 1 && m_bPromptShown != TTRUE;
}

// $deBlob: FUNCTION 0059be90
void AChallengeManager::StartNearestChallenge()
{
	AChallenge* pChallenge = FindNearestAvailableChallenge( APlayerManager::GetSingleton()->GetPlayerBlob( 0 )->GetMatrix().GetTranslation() );

	if ( pChallenge != TNULL )
	{
		m_pNearbyChallenge = pChallenge;
		StartChallenge();
	}
}

// $deBlob: FUNCTION 0059bec0
void AChallengeManager::ResetEntities()
{
	T2DList<ACellPair>& rEntities = AEntityFactory::GetSingleton()->GetAlwaysTickEntities();

	for ( auto it = rEntities.Begin(); it != rEntities.End(); it++ )
	{
		ABaseEntity* pEntity = it->m_pEntity;

		if ( ( pEntity->GetFlags() & 0xE00 ) == ABaseEntity::FLAG_LowLOD )
			continue;

		if ( pEntity->IsA( &TGetClass( ALeechBot2 ) ) )
		{
			ALeechBot2* pLeechBot = TSTATICCAST( ALeechBot2, pEntity );

			if ( ALeechBot2::IsFixedColourType( pLeechBot->GetLeechbotType() ) )
				pLeechBot->SetActive( TTRUE, TFALSE );
		}
		else if ( pEntity->IsA( &TGetClass( APropWaterHydrant ) ) )
		{
			pEntity->Reset( TNULL );
			pEntity->SetActive( TTRUE );
		}
	}
}

// $deBlob: FUNCTION 0059c0a0
void AChallengeManager::EndChallenge( AChallenge* a_pChallenge, TBOOL a_bFailed, TBOOL a_bQuiet )
{
	ABlobHud* pHud = ABlobHud::GetSingleton();

	if ( !a_bFailed )
	{
		if ( a_pChallenge->GetType() != AChallenge::Type_Null )
		{
			m_iTotalCompletedChallenges++;
			m_iLastScore = a_pChallenge->GetScore();
		}

		TASSERT( m_iTotalCompletedChallenges <= m_iTotalNumChallenges );

		a_pChallenge->Complete();
		m_uiLastNumLinked = a_pChallenge->GetData()->m_uiNumLinkedChallenges;

		if ( !AChallengeManager_IsSprintLevel() )
		{
			if ( a_pChallenge == m_pCurrentChallenge )
			{
				m_pCurrentChallenge = TNULL;

				if ( pHud != TNULL && pHud->GetChallengeTimerControl() != TNULL )
					pHud->GetChallengeTimer()->ResetWarning();

				// The laughing civilians around Blob stop
				ABaseEntity*                  pBlob = APlayerManager::GetSingleton()->GetPlayerBlob( 0 );
				AEntityHashFilterOnlyThisType oFilter( &TGetClass( ACiv ) );
				TINT                          iNumCivs;
				ABaseEntity**                 ppCivs = AEntityHash::GetSingleton()->Query( TSTATICCAST( ABlobEntity, pBlob )->GetBodyPosition(), 100.0f, &iNumCivs, &oFilter, TFALSE, 500, TFALSE, TTRUE, TFALSE );

				for ( TUINT i = 0; i < TUINT( iNumCivs ); i++ )
				{
					ACiv* pCiv = TSTATICCAST( ACiv, ppCivs[ i ] );

					if ( pCiv->IsTransformed() )
						pCiv->StartLaughTimer();
				}
			}
		}
		else if ( a_pChallenge->GetData()->m_uiNumLinkedChallenges == 0 )
		{
			// The sprint is over
			if ( a_pChallenge == m_pCurrentChallenge )
			{
				m_pCurrentChallenge = TNULL;

				if ( pHud != TNULL && pHud->GetChallengeTimerControl() != TNULL )
					pHud->GetChallengeTimer()->ResetWarning();

				AGameStatus::GetSingleton()->OnSprintChallengesCompleted();
			}
		}
		else
		{
			// The sprint goes on with the next challenge
			m_pNearbyChallenge = a_pChallenge->GetData()->m_ppLinkedChallenges[ 0 ];
			StartChallenge();

			if ( m_pCurrentChallenge != TNULL && m_pCurrentChallenge->GetData()->m_uiNumLinkedChallenges == 0 )
			{
				TINT iTime = TMath::CeilToInt( TFLOAT( m_iCounterTotal ) * ( TFLOAT( *TREINTERPRETCAST( TINT*, TREINTERPRETCAST( TUINT8*, AGameStatus::GetSingleton()->GetLevelProperties() ) + 0x38 ) ) / 100.0f ) );
				AGameStatus::GetSingleton()->GetLevelData().SetGoal( iTime, 0 );
			}
		}
	}
	else
	{
		if ( a_bQuiet )
		{
			m_bEnding = TFALSE;
			m_bFailed = TFALSE;
			return;
		}

		a_pChallenge->Fail();

		if ( a_pChallenge == m_pCurrentChallenge )
			m_pCurrentChallenge = TNULL;

		if ( pHud == TNULL )
		{
			m_bEnding = TFALSE;
			m_bFailed = TFALSE;
			return;
		}

		if ( pHud->GetChallengeTimerControl() != TNULL )
			pHud->GetChallengeTimer()->ResetWarning();
	}

	if ( pHud != TNULL && a_pChallenge->GetType() != AChallenge::Type_Null && !AChallengeManager_IsSprintLevel() && !a_bQuiet &&
	     ( m_pCurrentChallenge == TNULL || a_pChallenge == m_pCurrentChallenge ) )
	{
		pHud->FindChildXUIElement( 1 )->PlayAnimation( L"EndChallengeToNormal", L"EndChallengeToNormal" );
	}

	m_bEnding = TFALSE;
	m_bFailed = TFALSE;
}

// $deBlob: FUNCTION 0059c3a0
void AChallengeManager::SendSkipToCurrentChallenge()
{
	if ( m_pCurrentChallenge != TNULL )
		m_pCurrentChallenge->SendSkipToEntities();
}

// $deBlob: FUNCTION 0059c3b0
void AChallengeManager::SetMarkersVisible( TBOOL a_bVisible )
{
	for ( TINT i = 0; i < m_iTotalNumChallenges; i++ )
	{
		ARevolutionary* pRevolutionary = m_ppChallenges[ i ]->GetRevolutionary();

		if ( pRevolutionary != TNULL )
		{
			// NOTE: retail passes !a_bVisible
			pRevolutionary->SetVisible( !a_bVisible );
			pRevolutionary->HideMarker();
		}
	}
}

// $deBlob: FUNCTION 0059c400
void AChallengeManager::DisableChallengeCameras( AChallenge::Data* a_pData )
{
	ANedCameraFunction& rNed = ACameraManager::GetSingleton()->GetNedCameras();

	for ( TINT i = 0; i < a_pData->m_iNumCameraRegions; i++ )
		rNed.SetRegionEnabled( a_pData->m_pCameraRegions[ i ], TFALSE );
}

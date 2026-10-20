#include "pch.h"
#include "ABossEndSeqState.h"
#include "AGameStateController.h"
#include "AXUIState.h"
#include "GameController/AGameControllerHelper.h"
#include "GameController/AGameStatus.h"
#include "Input/ABlobInputMap.h"
#include "Movie/AMoviePlayer.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundLoadHelper.h"
#include "XUI/ABlobControllerPopup.h"
#include <XUI/TXUI.h>
#include <XUI/TXUICanvas.h>
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABossEndSeqState ) == 0x34 );

// $deBlob: FUNCTION 005c1cf0
// $deBlob: FUNCTION 005c1d00
// $deBlob: FUNCTION 005c1d80
// $deBlob: FUNCTION 005c1e00
// $deBlob: FUNCTION 005c1e10
// $deBlob: FUNCTION 0053c550
// $deBlob: FUNCTION 008584a0
TDEFINE_CLASS( ABossEndSeqState );

// $deBlob: FUNCTION 005c1e20
ABossEndSeqState::ABossEndSeqState()
{
	m_eState           = STATE_Loading;
	m_pControllerPopup = TNULL;
}

// $deBlob: FUNCTION 005c1e40
// $deBlob: FUNCTION 005c24b0
ABossEndSeqState::~ABossEndSeqState()
{
}

// $deBlob: FUNCTION 005c1e50
void ABossEndSeqState::Finish()
{
	m_eState = STATE_Finished;
	ABlobInputMap::GetSingleton()->ClearPlayers();
}

// $deBlob: FUNCTION 005c1e70
AGameState::UpdateResult ABossEndSeqState::OnUpdate( TFLOAT a_fDeltaTime )
{
	ABlobInputMap* pInputMap = ABlobInputMap::GetSingleton();
	pInputMap->Update( a_fDeltaTime );

	TXUI*         pXUI   = TXUI::GetSingleton();
	AMoviePlayer* pMovie = AMoviePlayer::GetSingleton();

	switch ( m_eState )
	{
		case STATE_Loading:
			if ( ASoundLoadHelper::GetSingleton()->GetState() == ASoundLoadHelper::LOADED )
			{
				AMusicManager::GetSingleton()->CreateMusicStream();
				AMusicManager::GetSingleton()->PlaySnapshot( "vistacam" );
				pXUI->GetCanvas()->SetVisible( TFALSE );
				AGameStateController::GetSingleton()->SetFadeColour( 0xFF000000, 0 );
				AGameStateController::GetSingleton()->ResetFades();
				AGameStateController::GetSingleton()->SetRenderFadesFirst( TFALSE );

				// The letterbox is gone, the movie plays
				TXUIScene* pFocScene = pXUI->GetActiveScene();
				TASSERT( pFocScene && 0 == TStringManager::String16Compare( pFocScene->GetID(), L"LetterBox" ) );

				if ( pFocScene != TNULL )
					delete pFocScene;

				pMovie->PlayMovie( "11_Triumph", 0, AMoviePlayer::PlayFlags_None );
				pInputMap->ClearPlayers();
				m_eState = STATE_Movie;
			}
			else
			{
				ASoundLoadHelper::GetSingleton()->Update();
			}
			break;
		case STATE_LetterBox:
		{
			TXUIScene* pScene = pXUI->GetActiveScene();

			if ( TStringManager::String16Compare( pScene->GetID(), L"LetterBox" ) == 0 && !pScene->GetTimelineControl().IsPlaying() )
				m_eState = STATE_Loading;
			break;
		}
		case STATE_Movie:
		{
			// Skippable (the button 0x10000 of the first player)
			const ABlobInputMap::PlayerInput& rInput = pInputMap->GetPlayerInput( 0 );

			if ( ( rInput.m_uiButtons & ABlobInputMap::BUTTON_SKIP & ( rInput.m_uiButtons ^ rInput.m_uiPrevButtons ) ) && pMovie->IsMoviePlaying() )
				pMovie->StopMovie();

			if ( !pMovie->IsMoviePlaying() )
			{
				m_eState = STATE_ScoreMenu;
				AMusicManager::GetSingleton()->StartMusic( TFALSE );
				AMusicManager::GetSingleton()->AddScore( 100 );
				pInputMap->ClearPlayers();
			}
			break;
		}
		case STATE_ScoreMenu:
		{
			pXUI->GetCanvas()->SetVisible( TTRUE );
			TXUIScene* pScoreMenu = pXUI->CreateScene( pXUI->FindResource( "scoremenu.xur" ), TUINT32( 0 ) );
			pXUI->TransitionTo( TNULL, pScoreMenu );
			m_eState = STATE_InScoreMenu;
			pInputMap->ClearPlayers();
			break;
		}
		case STATE_InScoreMenu:
			m_pControllerPopup->UpdateFrontEnd( a_fDeltaTime );
			break;
		case STATE_Finished:
			pXUI->Reset();
			pXUI->GetCanvas()->SetVisible( TTRUE );
			AGameControllerHelper::ReturnToFrontEnd( TFALSE );
			return UpdateResult_OK;
		default:
			break;
	}

	if ( !AGameStateController::GetSingleton()->IsFading( 0 ) && !pMovie->IsMoviePlaying() )
		pXUI->TickInput( a_fDeltaTime );

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $deBlob: FUNCTION 005c2120
void ABossEndSeqState::OnInsertion()
{
	AGameState::OnInsertion();

	new ABlobInputMap();

	TASSERT( AMusicManager::IsSingletonCreated() );
	TASSERT( ASoundLoadHelper::IsSingletonCreated() );

	AMusicManager::GetSingleton()->LoadMusicData( "M12_Boss", TTRUE, TFALSE );
	ASoundLoadHelper::GetSingleton()->LoadFrontEndSounds();
	ASoundLoadHelper::GetSingleton()->SetState( ASoundLoadHelper::LOADGROUP );

	AXUIState::SetSkin2( "frontendskin.trb", "frontendskin.xur" );
	AXUIState::SetSkinWithFont( "endgame.trb", TFALSE );

	TXUI*      pXUI       = TXUI::GetSingleton();
	TXUIScene* pLetterBox = pXUI->CreateScene( pXUI->FindResource( "letterboxbossend.xur" ), TUINT32( 0 ) );

	TASSERT( TXUI::GetSingleton()->GetActiveScene() == TNULL );
	pXUI->SetFocussedScene( pLetterBox );
	pLetterBox->PlayAnimation( L"169", L"End169" );
	m_eState = STATE_LetterBox;

	AGameStateController::GetSingleton()->SetFade( 1.0f, 0 );

	TXUIScene* pPopup = pXUI->CreateScene( pXUI->FindResource( "controllerscreen.xur" ), L"ControllerPopup", TTRUE );
	TASSERT( pPopup == TNULL || pPopup->IsA( &TGetClass( ABlobControllerPopup ) ) );
	m_pControllerPopup = TSTATICCAST( ABlobControllerPopup, pPopup );
	m_pControllerPopup->RemoveSelf();
}

// $deBlob: FUNCTION 005c2370
void ABossEndSeqState::OnRemoval()
{
	AMusicManager::GetSingleton()->UnloadMusicData();
	ASoundLoadHelper::GetSingleton()->UnloadSoundEventGroups();
	ASoundLoadHelper::GetSingleton()->UnloadEventProjects();

	AXUIState::ResetXUI();
	AXUIState::UnloadSkin();
	AXUIState::UnloadInGameSkin();
	TXUI::GetSingleton()->UnloadSkin2();

	if ( ABlobInputMap::GetSingleton() != TNULL )
		delete ABlobInputMap::GetSingleton();

	AGameStateController::GetSingleton()->FadeFromBlack( 0.5f, 0 );
	AGameState::OnRemoval();
	ASoundInterface::UnloadEventSystem();
	AGameStatus::GetSingleton()->ResetLevelReferences(); // the level start gates, the boss grass bomb and m_apNextBuildingPaintable
}

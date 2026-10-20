#include "pch.h"
#include "Sound/AStreamingSoundPlayer.h"
#include "GameInterface/AInGameState.h"
#include "Memory/AMemory.h"
#include "AGameControllerHelper.h"
#include "AGameStatus.h"
#include "ALevelInformation.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ALoadLevelState.h"
#include "GameInterface/AMovieState.h"
#include "IO/AIOEntity.h"
#include "GameInterface/AXUIState.h"
#include "GameInterface/APlayerManager.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "Render/ARenderer.h"
#include "GameInterface/ABossEndSeqState.h"

#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005b96a0
AIOEntity* AGameControllerHelper::FindIOEntity( TINT a_iID )
{
	if ( a_iID == -1 )
		return TNULL;

	ABaseEntity* pEntity = ABaseEntity::FindIDEntity( a_iID );

	if ( pEntity != TNULL && pEntity->IsA( &TGetClass( AIOEntity ) ) == TTRUE )
		return pEntity->CastTo<AIOEntity>();

	return TNULL;
}

// $deBlob: FUNCTION 005b9650
AIOEntity* AGameControllerHelper::GetCheckPointStartEntity()
{
	TINT iID = ABaseEntity::GetIDName( "CheckPoint1" );

	if ( iID == -1 )
		return TNULL;

	ABaseEntity* pEntity = ABaseEntity::FindIDEntity( iID );

	if ( pEntity != TNULL && pEntity->IsA( &TGetClass( AIOEntity ) ) == TTRUE )
		return pEntity->CastTo<AIOEntity>();

	return TNULL;
}

// $deBlob: FUNCTION 005b96f0
void AGameControllerHelper::LoadLevel( const TCHAR* a_szLevelName, const TWCHAR* a_wszDisplayName )
{
	TINT iLevelIndex = ALevelInformation::GetSingleton()->GetLevelIndex( a_szLevelName );
	TASSERT( iLevelIndex >= 0 );

	LoadLevel( iLevelIndex, a_wszDisplayName );
}

// $deBlob: FUNCTION 005b9750
void AGameControllerHelper::LoadLevel( TINT a_iLevelIndex, const TWCHAR* a_wszDisplayName )
{
	// Stops the sound events of the frontend
	AStreamingSoundPlayer::GetSingleton()->StopAll();
	AStreamingSoundPlayer::GetSingleton()->ResetAll();

	AGameStateController::GetSingleton()->ResetStack();
	AXUIState::ResetXUI();
	AXUIState::UnloadSkin();
	AXUIState::UnloadInGameSkin();
	TXUI::GetSingleton()->UnloadSkin2();
	ARenderer::FlushRender();

	ALevelInformation*                  pLevelInformation = ALevelInformation::GetSingleton();
	ALevelInformation::LevelProperties* pProperties       = pLevelInformation->GetLevelProperties( a_iLevelIndex );

	// The state the loading state pushes once the level is loaded
	AGameState* pInGameState = new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) AInGameState();

	AGameState* pState = new ALoadLevelState( pInGameState, &pLevelInformation->GetLevelHeader()->m_pLevelProps[ a_iLevelIndex ], a_wszDisplayName );

	const TCHAR* szMovie    = pProperties->m_szMovie;
	TINT         eLevelMode = AGameStatus::GetSingleton()->GetLevelMode();

	if ( szMovie != TNULL && TStringManager::String8Compare( szMovie, "" ) != 0 && ( eLevelMode == -1 || eLevelMode == AGameStatus::Mode_Story ) )
	{
		// The level's movie plays before the loading state
		pState = new AMovieState( szMovie, TFALSE, TFALSE, pState );
	}

	AGameStateController::GetSingleton()->PushState( pState );
}

// Loads the first level
// $deBlob: FUNCTION 005b98a0
void AGameControllerHelper::LoadFirstLevel()
{
	LoadLevel( 0, TNULL );
}

// $deBlob: FUNCTION 005b98b0
void AGameControllerHelper::ReturnToFrontEnd( TBOOL a_bLevelSelect )
{
	AGameStateController::GetSingleton()->ResetStack();
	AXUIState::ResetXUI();
	AXUIState::UnloadSkin();
	AXUIState::UnloadInGameSkin();
	TXUI::GetSingleton()->UnloadSkin2();
	ARenderer::FlushRender();
	AXUIState::LoadFrontendCarousel();
	AGameStateController::GetSingleton()->SetFade( 1.0f, 0 );
	AXUIState::SetSkin2( "frontendskin.trb", "frontendskin.xur" );
	AXUIState::SetSkinWithFont( "frontend.trb", TFALSE );

	TXUI*      pXUI      = TXUI::GetSingleton();
	TXUIScene* pFrontEnd = pXUI->CreateScene( pXUI->FindResource( "frontend.xur" ), TUINT32( 0 ) );
	TASSERT( pFrontEnd->IsA( &TGetClass( ABlobFrontEnd ) ) );
	TSTATICCAST( ABlobFrontEnd, pFrontEnd )->SetCursorTrailsVisible( TFALSE );

	// The players are reset (their colours) and the game has one player again
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	for ( TINT i = 0; i < APlayerManager::MAX_PLAYERS; i++ )
		pPlayerManager->SetPlayerColour( i, 0 );
	pPlayerManager->SetNumPlayers( 1 );

	AGameStatus::GetSingleton()->SetSPLevel( -1 );
	AGameStatus::GetSingleton()->SetSelectedSlot( -1 );

	TXUIScene* pSplash = pXUI->CreateScene( pXUI->FindResource( "splash.xur" ), TUINT32( 0 ) );
	pSplash->SetXUIStateBits( TXUIScene::State_Hidden, 0xF, TXUIElement::XUIState_TimelineStateShift );
	pSplash->SetVisible( TFALSE );
	pFrontEnd->AddChildTail( pSplash );

	AXUIState::Assets oAssets;
	if ( !a_bLevelSelect )
	{
		AGameStatus::GetSingleton()->SetLevelMode( AGameStatus::Mode_Story );
		oAssets.m_sResourceName    = "mainmenu.xur";
		oAssets.m_wszSceneName     = TNULL;
		oAssets.m_iTransitionIndex = 6;
	}
	else
	{
		// The main menu scene is created now (not streamed) under the splash: its previous scene is the splash and its
		// transition index 5 (retail 0x5b9aa5..0x5b9aed); the level selection is streamed below
		TXUIScene* pMainMenu = pXUI->CreateScene( pXUI->FindResource( "mainmenu.xur" ), TUINT32( 0 ) );
		pMainMenu->SetXUIStateBits( TXUIScene::State_Hidden, 0xF, TXUIElement::XUIState_TimelineStateShift );
		pMainMenu->SetVisible( TFALSE );
		pMainMenu->SetPrevScene( pSplash );
		pMainMenu->SetTransitionIndex( 5 );
		pFrontEnd->AddChildTail( pMainMenu );

		oAssets.m_sResourceName = "levelselection.xur";
		oAssets.m_wszSceneName  = L"LevelSelection";
	}

	oAssets.m_fSoundVolume   = 0.5f;
	oAssets.m_pTargetParent  = pFrontEnd;
	oAssets.m_bDelayed       = TTRUE;
	oAssets.m_bBackFromLevel = TTRUE;

	AGameStateController::GetSingleton()->PushState( new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) AXUIState( oAssets ) );
}

// $deBlob: FUNCTION 005b9c60
void AGameControllerHelper::StartBossEndSequence()
{
	AGameStatus::CalculateScoreSummary( &AGameStatus::s_oScoreSummary );
	AGameStateController::GetSingleton()->ResetStack();
	AXUIState::ResetXUI();
	AXUIState::UnloadSkin();
	AXUIState::UnloadInGameSkin();
	TXUI::GetSingleton()->UnloadSkin2();
	ARenderer::FlushRender();

	AGameStateController::GetSingleton()->PushState( new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) ABossEndSeqState() );
}

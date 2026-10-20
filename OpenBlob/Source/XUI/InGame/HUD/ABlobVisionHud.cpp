#include "pch.h"
#include "ABlobVisionHud.h"
#include "GUI/AGUICursor.h"
#include "GUI/AGUIManager.h"
#include "GameInterface/APlayerManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Cameras/ABlobFollowCamera.h"
#include "Input/ABlobInputMap.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"

#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0065b620
// $deBlob: FUNCTION 0065b630
// $deBlob: FUNCTION 0065b6c0
// $deBlob: FUNCTION 0065b750
// $deBlob: FUNCTION 0065b760
// $deBlob: FUNCTION 0053e600
// $deBlob: FUNCTION 00858cf0
TDEFINE_CLASS( ABlobVisionHud );

// $deBlob: FUNCTION 0065bb90
ABlobVisionHud::ABlobVisionHud()
    : m_bUnk130( TFALSE )
    , m_uiVisionColour( 0 )
{
}

// $deBlob: FUNCTION 0065b770
// $deBlob: FUNCTION 0065bbd0
// $deBlob: FUNCTION 0065bbba
ABlobVisionHud::~ABlobVisionHud()
{
	AGUIManager::GetSingleton()->GetCursor()->SetBlobVision( TFALSE );
	AGUIManager::GetSingleton()->SetCursorEnabled( TFALSE );
}

// $deBlob: FUNCTION 0065b7f0
void ABlobVisionHud::OnCreated()
{
	m_uiSceneFlags &= ~SceneFlags_AcceptsInput;

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	pBlob->GetAnimationComponent()->SetTargetAlpha( 0.0f, 10.0f );

	m_oTimelineControl.PlayAnimation( L"Normal", L"EndNormal" );

	UpdateColour( pBlob );

	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );
}

// $deBlob: FUNCTION 0065b9b0
void ABlobVisionHud::EndTransBackTo()
{
	AGUIManager::GetSingleton()->GetCursor()->SetBlobVision( TTRUE );
	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );
	TXUIScene::EndTransBackTo();
}

// $deBlob: FUNCTION 0065b8d0
void ABlobVisionHud::EndTransBackFrom()
{
	ABlobEntity* pBlob     = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	pBlob->m_bInBlobVision = 0;

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );
	TSTATICCAST( AInGameState, pState )->SetPaused( TFALSE );

	pBlob->GetAnimationComponent()->SetTargetAlpha( 1.0f, 10.0f );

	ACamera* pCam = pBlob->GetViewCamera();
	TASSERT( pCam->IsA( &TGetClass( ABlobFollowCamera ) ) );
	TSTATICCAST( ABlobFollowCamera, pCam )->SetBlobVision( TTRUE );

	TXUIScene::EndTransBackFrom();
}

// $deBlob: FUNCTION 0065b9e0
void ABlobVisionHud::TransFrom()
{
	AGUIManager::GetSingleton()->GetCursor()->SetBlobVision( TFALSE );
	AGUIManager::GetSingleton()->SetCursorVisible( TFALSE, TTRUE );
	TXUIScene::TransFrom();
}

// $deBlob: FUNCTION 0065ba10
void ABlobVisionHud::TransBackTo()
{
	AGUIManager::GetSingleton()->GetCursor()->SetBlobVision( TTRUE );
	AGUIManager::GetSingleton()->SetCursorVisible( TTRUE, TTRUE );
	TXUIScene::TransBackTo();
}

// $deBlob: FUNCTION 0065ba40
void ABlobVisionHud::Tick( TFLOAT a_fDeltaTime )
{
	ABlobEntity* pBlob  = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	AGameState*  pState = AGameStateController::GetSingleton()->GetCurrentGameState();
	TASSERT( TTRUE == pState->IsA( &TGetClass( AInGameState ) ) );

	// Closed when the button is let go (or SetUnk130), unless the game is paused
	if ( !TSTATICCAST( AInGameState, pState )->IsPaused() && ( !( pBlob->m_uiButtons & ABlobInputMap::BUTTON_BLOBVISION ) || m_bUnk130 ) && TXUI::GetSingleton()->GetActiveScene() == this )
		TXUI::GetSingleton()->TransitionBack( this );

	TUINT32 uiColour = pBlob->GetColourComponent()->GetCurrentColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;

	if ( uiColour != m_uiVisionColour )
		UpdateColour( pBlob );

	TXUIScene::Tick( a_fDeltaTime );
}

// The background (element 1) takes Blob's colour, the cursor shows the vision
void ABlobVisionHud::UpdateColour( ABlobEntity* a_pBlob )
{
	TUINT32 uiColour = a_pBlob->GetColourComponent()->GetCurrentColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;

	m_uiVisionColour = uiColour;

	// NOTE: retail splits the colour into the components of a static colour and packs it again
	FindChildXUIElement( 1 )->SetColour( ABlobColourComponent::ms_auiColours[ uiColour ] );
	AGUIManager::GetSingleton()->GetCursor()->SetBlobVision( TTRUE );
}

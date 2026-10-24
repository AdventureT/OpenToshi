#include "pch.h"
#include "AFlythroughState.h"
#include "AGameStateController.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AScriptedCamera.h"
#include "Entities/AEntityFactory.h"
#include "Entities/APickupManager.h"
#include "Entities/Enemies/AEnemyManager.h"
#include "Entities/Projectiles/AProjectileManager.h"
#include "GUI/AGUIManager.h"
#include "Input/AInputManager2.h"
#include "JobSystem/AJobQueue.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AFlythroughState ) == 0x40 );

// $deBlob: FUNCTION 005c2960
// $deBlob: FUNCTION 005c2970
// $deBlob: FUNCTION 005c2a00
// $deBlob: FUNCTION 005c2a90
// $deBlob: FUNCTION 005c2aa0
// $deBlob: FUNCTION 0053c590
// $deBlob: FUNCTION 008584b0
TDEFINE_CLASS( AFlythroughState );

// $deBlob: FUNCTION 005c2ab0
AFlythroughState::AFlythroughState( const TCHAR* a_szCamera )
    : m_strCamera( a_szCamera )
{
	m_eState         = STATE_Playing;
	m_bSkipRequested = TFALSE;
}

// $deBlob: FUNCTION 005c2b20
// $deBlob: FUNCTION 005c2e50
AFlythroughState::~AFlythroughState()
{
}

// $deBlob: FUNCTION 005c2b80
AGameState::UpdateResult AFlythroughState::OnUpdate( TFLOAT a_fDeltaTime )
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( m_eState == STATE_Playing )
	{
		AScriptedCamera* pCamera = TSTATICCAST( AScriptedCamera, ACameraManager::GetSingleton()->GetCamera( ACamera::CT_SCRIPTED, 0 ) );

		if ( !pCamera->IsAnimating() )
		{
			m_eState = STATE_FadingOut;
			pController->FadeToBlack( 1.0f, 0 );
		}
	}
	else if ( m_eState == STATE_FadingOut )
	{
		if ( !pController->IsFading( 0 ) )
		{
			m_eState = STATE_FadingIn;
			pController->FadeFromBlack( 0.5f, 0 );
			return 1;
		}
	}
	else if ( m_eState == STATE_FadingIn )
	{
		return 1;
	}

	// NOTE: button 0x23 skips the fly-through
	TBOOL bSkip;

	if ( AInputManager2::GetSingleton()->GetButtonDown( 0x23 ) == TTRUE && !pController->IsFading( 0 ) )
		bSkip = TTRUE;
	else
		bSkip = m_eState != STATE_FadingOut && m_bSkipRequested == TTRUE;

	if ( bSkip )
	{
		TSTATICCAST( AScriptedCamera, ACameraManager::GetSingleton()->GetCamera( ACamera::CT_SCRIPTED, 0 ) )->StopAnimation();
		pController->FadeToBlack( 1.0f, 0 );
		m_eState         = STATE_FadingOut;
		m_bSkipRequested = TFALSE;
	}

	AEntityFactory::GetSingleton()->Tick( a_fDeltaTime, 0 );
	AProjectileManager::GetSingleton()->Tick( a_fDeltaTime );
	AEnemyManager::GetSingleton()->Tick( a_fDeltaTime );
	APickupManager::GetSingleton()->UpdateRotations( a_fDeltaTime );
	AJobQueue::GetInstance()->Update( a_fDeltaTime );

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $deBlob: FUNCTION 005c2d00
void AFlythroughState::OnInsertion()
{
	ACameraManager::GetSingleton()->SetCurrentCamera( ACamera::CT_SCRIPTED, 0, TTRUE );
	AScriptedCamera* pCamera = TSTATICCAST( AScriptedCamera, ACameraManager::GetSingleton()->GetCurrentCamera() );
	pCamera->PlayAnimation( m_strCamera );

	AGameStateController::GetSingleton()->FadeFromBlack( 1.0f, 0 );
	AEnemyManager::GetSingleton()->PauseForCutscene( TTRUE );
	AInputManager2::GetSingleton()->SetContext( AInputMap::INPUTCONTEXT_Movie );

	AGameStateController::GetSingleton()->SetWidescreen( TTRUE, 0.0f );

	AGameState::OnInsertion();
}

// $deBlob: FUNCTION 005c2dc0
void AFlythroughState::OnRemoval()
{
	ACameraManager::GetSingleton()->SetCurrentCamera( ACamera::CT_BLOBFOLLOW, 0, TTRUE );
	AEnemyManager::GetSingleton()->PauseForCutscene( TFALSE );
	AInputManager2::GetSingleton()->RemoveCurrentContext();

	AGameStateController::GetSingleton()->SetWidescreen( TFALSE, 0.0f );

	AGameState::OnRemoval();
}

// $deBlob: FUNCTION 005c2e10
void AFlythroughState::OnActivate()
{
	AGUIManager::GetSingleton()->SetGUIMask( T2GUIElement::STATE_UNK3 | T2GUIElement::STATE_SUBTITLES );
	AGameState::OnActivate();
}

// $deBlob: FUNCTION 005c2e30
void AFlythroughState::OnDeactivate()
{
	AGUIManager::GetSingleton()->RemoveCurrentGUIMask();
	AGameState::OnDeactivate();
}

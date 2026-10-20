#include "pch.h"
#include "Console/AConsoleState.h"
#include "ACameraManager.h"
#include "AFreeCamera.h"
#include "AKeyFrameCamera.h"
#include "AScriptedCamera.h"
#include "ABlobFollowCamera.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0058dc10
ACameraManager::ACameraManager()
{
	m_bActive = TFALSE;

	ABlobFollowCamera::InitialiseSettings();

	for ( TINT i = MAX_CAMERA - 1; i >= 0; i-- )
		m_apCameras[ i ] = TNULL;

	m_iNumActive     = 0;
	m_iCurrentCamera = -1;

	CreateCamera( ACamera::CT_FREE );
	CreateCamera( ACamera::CT_SCRIPTED );

	AConsoleState::RegisterCommand( "camera_free_cam", Command_FreeCam );
	AConsoleState::RegisterCommand( "camera_follow_cam", Command_FollowCam );
}

// $deBlob: FUNCTION 0058de10
// $deBlob: FUNCTION 0056be90
ACameraManager::~ACameraManager()
{
	AConsoleState::UnregisterCommand( Command_FreeCam );
	AConsoleState::UnregisterCommand( Command_FollowCam );

	m_oNedCameras.Clear();
	m_oSSCameras.Clear();

	m_oShakeFunction.Clear();

	for ( TINT i = 0; i < MAX_CAMERA; i++ )
	{
		if ( m_apCameras[ i ] != TNULL )
		{
			if ( ARenderer::GetSingleton() != TNULL )
				ARenderer::GetSingleton()->GetViewportManager()->RemoveCamera( m_apCameras[ i ] );

			if ( m_apCameras[ i ] != TNULL )
				delete m_apCameras[ i ];
		}

		m_apCameras[ i ] = TNULL;
	}

	m_iCurrentCamera = -1;
	m_iNumActive     = 0;
}

// $deBlob: FUNCTION 0058df40
void ACameraManager::SuspendCameras()
{
	for ( TINT i = MAX_CAMERA; i != 0; )
	{
		i--;
		ACamera* pCamera = m_apCameras[ i ];

		if ( pCamera != TNULL && pCamera->IsActive() )
			pCamera->OnSuspend();
	}
}

// $deBlob: FUNCTION 0058df70
void ACameraManager::Activate()
{
	m_bActive = TTRUE;

	for ( TINT i = MAX_CAMERA; i != 0; )
	{
		i--;
		ACamera* pCamera = m_apCameras[ i ];

		if ( pCamera != TNULL && pCamera->IsActive() )
			pCamera->Invalidate();
	}
}

// $deBlob: FUNCTION 0058dfa0
void ACameraManager::Deactivate()
{
	m_bActive = TFALSE;

	for ( TINT i = MAX_CAMERA; i != 0; )
	{
		i--;
		ACamera* pCamera = m_apCameras[ i ];

		if ( pCamera != TNULL && pCamera->IsActive() )
			pCamera->OnDeactivate();
	}
}

// Called by AInGameState::OnRemoval
// $deBlob: FUNCTION 0058dfd0
void ACameraManager::Reset()
{
	m_bActive = TFALSE;
	m_oNedCameras.Clear();
	m_oSSCameras.Clear();

	m_oShakeFunction.Clear();

	DisableFreeCameraInput();
}

// $deBlob: FUNCTION 0058e810
void ACameraManager::DisableFreeCameraInput()
{
	AFreeCamera* pFreeCamera = TSTATICCAST( AFreeCamera, FindCamera( ACamera::CT_FREE, 0 ) );
	TASSERT( ( pFreeCamera == TNULL ) || pFreeCamera->IsA( &TGetClass( AFreeCamera ) ) );
	pFreeCamera->SetInputEnabled( TFALSE );
}

// $deBlob: FUNCTION 0058e6f0
void ACameraManager::OnModelInstanceUnloaded( AModelInstance2* a_pInstance )
{
	for ( TINT i = MAX_CAMERA; i != 0; )
	{
		i--;
		if ( m_apCameras[ i ] != TNULL && m_apCameras[ i ]->GetCameraType() == ACamera::CT_BLOBFOLLOW )
		{
			ABlobFollowCamera* pCamera = TSTATICCAST( ABlobFollowCamera, m_apCameras[ i ] );
			TASSERT( ( pCamera == TNULL ) || pCamera->IsA( &TGetClass( ABlobFollowCamera ) ) );
			pCamera->ForgetAlphaInstance( a_pInstance );
		}
	}
}

// The level restart (AInGameState)
// $deBlob: FUNCTION 0058e000
void ACameraManager::ResetLevelCameras()
{
	m_oNedCameras.ResetUses();
	m_oNedCameras.SetAllRegionsEnabled( TTRUE );
	m_oShakeFunction.ResetShakes();
}

// $deBlob: FUNCTION 0058e020
void ACameraManager::Update( TFLOAT a_fDeltaTime )
{
	TBOOL       bPaused = TFALSE;
	AGameState* pState  = AGameStateController::GetSingleton()->GetCurrentGameState();

	if ( pState->IsA( &TGetClass( AInGameState ) ) )
		bPaused = TSTATICCAST( AInGameState, pState )->IsPaused();

	if ( m_bActive )
	{
		for ( TINT i = MAX_CAMERA; i != 0; )
		{
			i--;
			ACamera* pCamera = m_apCameras[ i ];

			// Only the follow cameras keep moving while the game is paused
			if ( pCamera != TNULL && pCamera->IsActive() && ( !bPaused || pCamera->GetCameraType() == ACamera::CT_BLOBFOLLOW ) )
				pCamera->Update( a_fDeltaTime );
		}

		UpdateCameraEffects( a_fDeltaTime );
	}
}

// $deBlob: FUNCTION 0058e0c0
ACamera* ACameraManager::CreateCamera( ACamera::CameraType a_eCamType )
{
	TINT iSlot = 0;

	while ( m_apCameras[ iSlot ] != TNULL )
	{
		if ( ++iSlot >= MAX_CAMERA )
			return TNULL;
	}

	switch ( a_eCamType )
	{
		case ACamera::CT_DEFAULT:
			m_apCameras[ iSlot ] = new ACamera();
			break;
		case ACamera::CT_BLOBFOLLOW:
			m_apCameras[ iSlot ] = new ABlobFollowCamera();
			break;
		case ACamera::CT_SCRIPTED:
			m_apCameras[ iSlot ] = new AScriptedCamera();
			break;
		case ACamera::CT_FREE:
			m_apCameras[ iSlot ] = new AFreeCamera();
			break;
		case ACamera::CT_KEYFRAME:
			m_apCameras[ iSlot ] = new AKeyFrameCamera();
			break;
		default:
			break;
	}

	if ( m_iCurrentCamera == -1 )
		m_iCurrentCamera = TINT8( iSlot );

	ACamera* pCamera = m_apCameras[ iSlot ];

	if ( pCamera != TNULL )
	{
		if ( pCamera->IsActive() )
			m_iNumActive--;

		pCamera->SetActive( TFALSE );
	}

	return m_apCameras[ iSlot ];
}

// $deBlob: FUNCTION 0058e250
void ACameraManager::DestroyCamera( ACamera* a_pCamera )
{
	if ( a_pCamera == TNULL )
		return;

	for ( TINT i = MAX_CAMERA - 1; i >= 0; i-- )
	{
		if ( m_apCameras[ i ] != a_pCamera )
			continue;

		m_apCameras[ i ]->OnDestroy();

		if ( ARenderer::GetSingleton() != TNULL )
			ARenderer::GetSingleton()->GetViewportManager()->RemoveCamera( m_apCameras[ i ] );

		if ( m_apCameras[ i ] != TNULL )
		{
			if ( m_apCameras[ i ]->IsActive() )
				m_iNumActive--;

			m_apCameras[ i ]->SetActive( TFALSE );
		}

		delete a_pCamera;
		m_apCameras[ i ] = TNULL;

		if ( m_iCurrentCamera == i )
			SetCurrentCamera( ACamera::CT_FREE, 0, TTRUE );

		return;
	}
}

// $deBlob: FUNCTION 0058e7c0
void ACameraManager::SetCameraTypeActive( TINT a_eType, TBOOL a_bActive )
{
	ACamera* pCamera = m_apCameras[ a_eType ];

	if ( pCamera != TNULL )
	{
		if ( a_bActive )
		{
			if ( !pCamera->IsActive() )
				m_iNumActive++;
		}
		else if ( pCamera->IsActive() )
		{
			m_iNumActive--;
		}

		pCamera->SetActive( a_bActive );
	}
}

// $deBlob: FUNCTION 0058e300
void ACameraManager::SetCameraActive( ACamera* a_pCamera, TBOOL a_bActive )
{
	for ( TINT i = MAX_CAMERA - 1; i >= 0; i-- )
	{
		if ( m_apCameras[ i ] != a_pCamera )
			continue;

		ACamera* pCamera = m_apCameras[ i ];

		if ( pCamera != TNULL )
		{
			if ( !a_bActive )
			{
				if ( pCamera->IsActive() )
					m_iNumActive--;
			}
			else if ( !pCamera->IsActive() )
			{
				m_iNumActive++;
			}

			pCamera->SetActive( a_bActive );
		}

		return;
	}
}

// $deBlob: FUNCTION 0058e370
ACamera* ACameraManager::GetCurrentCamera() const
{
	if ( m_iCurrentCamera >= 0 )
	{
		TASSERT( m_iCurrentCamera >= 0 && m_iCurrentCamera < MAX_CAMERA );
		return m_apCameras[ m_iCurrentCamera ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 0058e3d0
ACamera::CameraType ACameraManager::GetCurrentCameraType() const
{
	if ( m_iCurrentCamera < 0 )
		return ACamera::CameraType( TUINT8( 0xFF ) );

	TASSERT( m_iCurrentCamera >= 0 && m_iCurrentCamera < MAX_CAMERA );

	// NOTE: retail returns the low byte of the type
	return ACamera::CameraType( TUINT8( m_apCameras[ m_iCurrentCamera ]->GetCameraType() ) );
}

// $deBlob: FUNCTION 0058e430
ACamera* ACameraManager::SetCurrentCamera( ACamera::CameraType a_eType, TINT8 a_iNo, TBOOL a_bDeactivateOthers )
{
	TASSERT( a_eType >= 0 && a_eType < ACamera::CT_TYPECOUNT );
	TASSERT( a_iNo >= 0 && a_iNo < MAX_CAMERA );

	if ( a_bDeactivateOthers )
	{
		for ( TINT i = MAX_CAMERA; i != 0; )
		{
			i--;
			if ( m_apCameras[ i ] != TNULL )
			{
				if ( m_apCameras[ i ]->IsActive() )
					m_iNumActive--;

				m_apCameras[ i ]->SetActive( TFALSE );
			}
		}
	}

	TINT8 iNo = a_iNo;

	for ( TINT i = 0; i < MAX_CAMERA; i++ )
	{
		if ( m_apCameras[ i ] == TNULL || m_apCameras[ i ]->GetCameraType() != a_eType || --iNo >= 0 )
			continue;

		if ( m_apCameras[ i ] != TNULL )
		{
			if ( !m_apCameras[ i ]->IsActive() )
				m_iNumActive++;

			m_apCameras[ i ]->SetActive( TTRUE );
		}

		m_iCurrentCamera = TINT8( i );
		return m_apCameras[ i ];
	}

	m_iCurrentCamera = -1;
	return TNULL;
}

// $deBlob: FUNCTION 0058e550
void ACameraManager::SetCurrentCamera( ACamera* a_pCamera, TBOOL a_bDeactivateOthers )
{
	if ( a_bDeactivateOthers )
	{
		for ( TINT i = MAX_CAMERA; i != 0; )
		{
			i--;
			if ( m_apCameras[ i ] != TNULL )
			{
				if ( m_apCameras[ i ]->IsActive() )
					m_iNumActive--;

				m_apCameras[ i ]->SetActive( TFALSE );
			}
		}
	}

	for ( TINT i = 0; i < MAX_CAMERA; i++ )
	{
		if ( m_apCameras[ i ] != a_pCamera )
			continue;

		if ( m_apCameras[ i ] != TNULL )
		{
			if ( !m_apCameras[ i ]->IsActive() )
				m_iNumActive++;

			m_apCameras[ i ]->SetActive( TTRUE );
		}

		m_iCurrentCamera = TINT8( i );
		return;
	}

	m_iCurrentCamera = -1;
}

ACamera* ACameraManager::FindCamera( ACamera::CameraType a_eType, TINT8 a_iNo ) const
{
	TINT8 iNo = a_iNo;

	for ( TINT8 i = 0; i < MAX_CAMERA; i++ )
	{
		if ( m_apCameras[ i ] != TNULL && m_apCameras[ i ]->GetCameraType() == a_eType && --iNo < 0 )
			return m_apCameras[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 0058e6d0
void ACameraManager::LoadCameras()
{
	m_oSSCameras.Load();
	m_oNedCameras.Load();
	m_oShakeFunction.LoadLevelShakes();
}

// The effects of the active cameras (shakes...) after their update
// $deBlob: FUNCTION 0058e770
void ACameraManager::UpdateCameraEffects( TFLOAT a_fDeltaTime )
{
	m_oShakeFunction.Update( a_fDeltaTime );

	for ( TINT i = MAX_CAMERA; i != 0; )
	{
		i--;
		if ( m_apCameras[ i ] != TNULL && m_apCameras[ i ]->IsActive() )
			m_oShakeFunction.ApplyShakes( m_apCameras[ i ] );
	}
}

// The console command camera_free_cam: the free camera takes the current camera's place
// $deBlob: FUNCTION 0058e8b0
void __stdcall ACameraManager::Command_FreeCam( Toshi::TString8* a_pArgs, TINT a_iNumArgs )
{
	ACameraManager* pManager = ACameraManager::GetSingleton();
	TMatrix44       mMatrix  = pManager->GetCurrentCamera()->GetMatrix();

	pManager->FindCamera( ACamera::CT_FREE, 0 )->SetMatrix( mMatrix );

	AFreeCamera* pFreeCamera = TSTATICCAST( AFreeCamera, pManager->FindCamera( ACamera::CT_FREE, 0 ) );
	TASSERT( ( pFreeCamera == TNULL ) || pFreeCamera->IsA( &TGetClass( AFreeCamera ) ) );
	pFreeCamera->SetFollowedObject( TNULL );
	pFreeCamera->SetInputEnabled( TFALSE );

	pManager->SetCurrentCamera( ACamera::CT_FREE, 0, TTRUE );
}

// The console command camera_follow_cam
// $deBlob: FUNCTION 0058ea30
void __stdcall ACameraManager::Command_FollowCam( Toshi::TString8* a_pArgs, TINT a_iNumArgs )
{
	ACameraManager::GetSingleton()->SetCurrentCamera( ACamera::CT_BLOBFOLLOW, 0, TTRUE );
}

// $deBlob: FUNCTION 0058e5e0
ACamera* ACameraManager::GetCamera( ACamera::CameraType a_eType, TINT8 a_iNo ) const
{
	TASSERT( a_eType >= 0 && a_eType < ACamera::CT_TYPECOUNT );
	TASSERT( a_iNo >= 0 && a_iNo < MAX_CAMERA );

	TINT8 iNo = a_iNo;

	for ( TINT8 i = 0; i < MAX_CAMERA; i++ )
	{
		if ( m_apCameras[ i ] != TNULL && m_apCameras[ i ]->GetCameraType() == a_eType && --iNo < 0 )
			return m_apCameras[ i ];
	}

	return TNULL;
}

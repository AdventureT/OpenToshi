#pragma once
#include "ACamera.h"
#include "ACameraShakeFunction.h"
#include "ANedCameraFunction.h"
#include "ASSCameraFunction.h"

#include <Toshi/TSingleton.h>

class AModelInstance2;

//-----------------------------------------------------------------------------
// Purpose: owns the cameras. The active cameras are updated every frame; the current one is what the viewports without
// a camera of their own render.
//-----------------------------------------------------------------------------
class ACameraManager : public Toshi::TSingleton<ACameraManager>
{
public:
	static TCONSTEXPR_VAR TINT MAX_CAMERA = 14;

public:
	ACameraManager();
	~ACameraManager();

	void Activate();
	void Deactivate();
	void SuspendCameras();
	void Reset();

	// The follow cameras forget a_pInstance (AModelInstance2::OnUnloaded)
	void OnModelInstanceUnloaded( AModelInstance2* a_pInstance );

	// The free camera ignores the input (Reset)
	void DisableFreeCameraInput();
	void ResetLevelCameras();
	void Update( TFLOAT a_fDeltaTime );

	ACamera* CreateCamera( ACamera::CameraType a_eCamType );
	void     DestroyCamera( ACamera* a_pCamera );
	void     SetCameraActive( ACamera* a_pCamera, TBOOL a_bActive );
	// The same for the camera of the type a_eType
	void     SetCameraTypeActive( TINT a_eType, TBOOL a_bActive );
	ACamera* GetCurrentCamera() const;

	ACameraShakeFunction& GetShakeFunction() { return m_oShakeFunction; }
	TINT                  GetNumActiveCameras() const { return m_iNumActive; }
	ACamera::CameraType   GetCurrentCameraType() const;
	ACamera*              SetCurrentCamera( ACamera::CameraType a_eType, TINT8 a_iNo, TBOOL a_bDeactivateOthers );

	// The a_iNo-th camera of a_eType, TNULL if there's none
	ACamera* GetCamera( ACamera::CameraType a_eType, TINT8 a_iNo ) const;
	void     SetCurrentCamera( ACamera* a_pCamera, TBOOL a_bDeactivateOthers );

	// The a_iNo-th camera of type a_eType, TNULL if there's none
	ACamera* FindCamera( ACamera::CameraType a_eType, TINT8 a_iNo ) const;

	// Reads the camera data of the level (ALoadLevelState): the regions, the scripted sequence cameras and the shakes
	void LoadCameras();

	ANedCameraFunction& GetNedCameras() { return m_oNedCameras; }
	ASSCameraFunction&  GetSSCameras() { return m_oSSCameras; }

	TBOOL IsActive() const { return m_bActive; }

private:
	void UpdateCameraEffects( TFLOAT a_fDeltaTime );

	static void __stdcall Command_FreeCam( Toshi::TString8* a_pArgs, TINT a_iNumArgs );
	static void __stdcall Command_FollowCam( Toshi::TString8* a_pArgs, TINT a_iNumArgs );

private:
	TBOOL                m_bActive;                 // 0x000 cameras are only updated while active (AInGameState)
	ANedCameraFunction   m_oNedCameras;             // 0x004
	ASSCameraFunction    m_oSSCameras;              // 0x00C
	ACameraShakeFunction m_oShakeFunction;          // 0x010
	ACamera*             m_apCameras[ MAX_CAMERA ]; // 0x340
	TINT8                m_iCurrentCamera;          // 0x378 -1: none
	TINT                 m_iNumActive;              // 0x37C the cameras with m_bActive
};

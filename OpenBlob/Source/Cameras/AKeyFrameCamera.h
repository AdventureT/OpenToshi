#pragma once
#include "ACamera.h"
#include "ASSCameraFunction.h"

#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Purpose: the key frame camera of a player: plays a scripted sequence camera of the level (ASSCameraFunction), the
// fly-by of a completed block and other events (APlayerManager::PlaySSCamera), chained cameras one after the other; the
// player can skip it.
//-----------------------------------------------------------------------------
class AKeyFrameCamera : public ACamera
{
public:
	TDECLARE_CLASS( AKeyFrameCamera, ACamera );

public:
	AKeyFrameCamera();
	~AKeyFrameCamera();

	virtual void Update( TFLOAT a_fDeltaTime ) override; // +0x0C

	// Plays the SS camera a_iCamera (ACameraManager's ASSCameraFunction, -1: none); TFALSE if it has no key frames
	TBOOL SetCamera( TINT a_iCamera );
	TBOOL SetCamera( const ASSCameraFunction::Camera* a_pCam );

	// Starts the camera from its beginning
	void Play();

	// Stops playing and clears the key frames and time
	void Stop();

	// Stops playing (the camera stays where it is)
	void Pause();

	// Plays on from the current time
	void Resume();

	// Stop() without the help timer reset
	void Rewind();

	// Jumps to key frame a_iKeyFrame
	void SetKeyFrame( TINT a_iKeyFrame );

	// Moves the camera's key frame to a_fTime (0..m_fTotalTime)
	void SetTime( TFLOAT a_fTime );

	// $deBlob: FUNCTION 005930e0
	TFLOAT GetTime() const { return m_fTime; }
	// $deBlob: FUNCTION 005930f0
	TFLOAT GetTotalTime() const { return m_fTotalTime; }
	// $deBlob: FUNCTION 00593100
	TBOOL IsPlaying() const { return m_bPlaying; }

	// $deBlob: FUNCTION 00593110
	TBOOL IsSkipped() const { return m_bSkipped; }

	// $deBlob: FUNCTION 005d1570
	const ASSCameraFunction::Camera* GetSSCamera() const { return m_pSSCam; }

private:
	// Follows the key frames (not a path camera)
	void UpdateKeyFrame( TFLOAT a_fDeltaTime );

	// Follows the path (ASSCameraFunction::CF_PATH)
	void UpdatePath( TFLOAT a_fDeltaTime );

	// Sends the letterbox helps (APropTriggerHelp) around the camera their trigger message
	void UpdateEntityHash( TFLOAT a_fDeltaTime );

	// The player pressed skip: skips this camera and the ones chained after it
	void UpdateSkip();

	// Skips this camera and the ones chained after it
	void Skip();

	// The fade of the key frame
	void UpdateFade( TBOOL a_bAreaReady );

private:
	ASSCameraFunction::KeyFrame        m_oKeyFrame;   // 0x104 the camera's interpolated key frame
	const ASSCameraFunction::Camera*   m_pSSCam;      // 0x154
	const ASSCameraFunction::KeyFrame* m_pCurrentKF;  // 0x158
	const ASSCameraFunction::KeyFrame* m_pNextKF;     // 0x15C
	const ASSCameraFunction::KeyFrame* m_pNextNextKF; // 0x160
	Toshi::TQuaternion                 m_qOrient;     // 0x164 the smoothed orientation (UpdateKeyFrame)
	TFLOAT                             m_fTime;       // 0x174
	TFLOAT                             m_fTotalTime;  // 0x178
	TBOOL                              m_bPlaying;    // 0x17C
	TBOOL                              m_bSkipped;    // 0x17D
	TBOOL                              m_bSnap;       // 0x17E the next update jumps to the key frame
	TBOOL                              m_bPath;       // 0x17F ASSCameraFunction::CF_PATH
	TFLOAT                             m_fHelpTimer;  // 0x180 UpdateEntityHash's interval
};

#pragma once
#include "ACamera.h"
#include "Input/AInputManager2.h"

#include <Toshi/T2DList.h>
#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Purpose: the debug free camera, moved with the camera sticks or following an object.
//-----------------------------------------------------------------------------
class AFreeCamera : public ACamera
{
public:
	TDECLARE_CLASS( AFreeCamera, ACamera );

public:
	// A camera of StatsLog.txt (the profiling positions of a level: "Level:", "Cam Pos: (x, y, z)" and
	// "Cam Rot: (x, y, z, w)" lines, 0x2C bytes)
	class ProfileCamData
	    : public Toshi::T2DList<ProfileCamData>::Node
	{
	public:
		// $deBlob: FUNCTION 00591770
		// $deBlob: FUNCTION 00591610
		virtual ~ProfileCamData() {} // +0x00

	public:
		Toshi::TVector4    m_vPosition; // 0x0C
		Toshi::TQuaternion m_qRotation; // 0x1C
	};

public:
	AFreeCamera();
	~AFreeCamera();

	virtual void  Update( TFLOAT a_fDeltaTime ) override;                             // +0x0C
	virtual void  Lock( TBOOL a_bLock ) override;                                     // +0x10
	virtual TBOOL IsLocked() override;                                                // +0x14
	virtual void  GetController( AInputManager2::AInputDeviceHandle& a_rController ); // +0x28

	void PushForward( TFLOAT a_fDistance );
	void PushBackward( TFLOAT a_fDistance );
	void PushRight( TFLOAT a_fDistance );
	void PushLeft( TFLOAT a_fDistance );
	void SetFreeCameraSpeed( TFLOAT a_fFreeCamSpeed );
	void SetInputEnabled( TBOOL a_bEnabled );

	// Turns the camera (the negated angle: RotateAroundWorldUp / RotateAroundRight)
	void Yaw( TFLOAT a_fAngle );
	void Pitch( TFLOAT a_fAngle );

	// Reads the cameras of the current level from StatsLog.txt (once, until ClearProfileCams)
	void LoadProfileCams();
	void ClearProfileCams();

	// Moves a_iOffset cameras on in the list (clamped) and puts the camera there
	void SelectProfileCam( TINT a_iOffset );

	// Reloads the cameras when it changes while the input is enabled
	void SetUnk116( TBOOL a_bUnk );

	// The camera copies the transform at +0x10 of a_pObject every frame (TNULL: free)
	// $deBlob: FUNCTION 0058ebb0
	void SetFollowedObject( void* a_pObject ) { m_pFollowedObject = a_pObject; }

private:
	TFLOAT                             m_fFreeCamSpeed;   // 0x104 60
	AInputManager2::AInputDeviceHandle m_oController;     // 0x108 the first controller of AInputManager2
	void*                              m_pFollowedObject; // 0x110
	TBOOL                              m_bLocked;         // 0x114
	TBOOL                              m_bInputEnabled;   // 0x115
	TBOOL                              m_bUnk116;         // 0x116 TTRUE
	TINT                               m_iProfileCam;     // 0x118
	TINT                               m_iNumProfileCams; // 0x11C
	Toshi::T2DList<ProfileCamData>*    m_pProfileCams;    // 0x120
};
